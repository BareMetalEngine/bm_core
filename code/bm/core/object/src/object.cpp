/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "object.h"
#include "objectGlobalRegistry.h"
#include "objectBinarySaver.h"
#include "objectBinaryLoader.h"
#include "serializationErrorReporter.h"

#include "rttiTypeSystem.h"
#include "rttiNativeClassType.h"
#include "rttiTypeSystem.h"
#include "rttiNativeClassType.h"
#include "rttiMetadata.h"
#include "rttiDataView.h"
#include "rttiProperty.h"
#include "rttiArrayType.h"
#include "rttiHandleType.h"
#include "globalEventDispatch.h"

#include "dataView.h"
#include "dataViewNative.h"

#include "bm/core/file/include/fileReader.h"
#include "bm/core/file/include/fileView.h"
#include "bm/core/containers/include/hashMap.h"
#include "bm/core/file/include/fileMemoryWriter.h"
#include "bm/core/memory/include/public.h"

BEGIN_INFERNO_NAMESPACE()

//--

std::atomic<uint32_t> GObjectIDAllocator = 0;

ObjectID IObject::AllocUniqueObjectID()
{
    return ++GObjectIDAllocator;
}

ObjectPtr IObject::FindUniqueObjectById(ObjectID id)
{
    return ObjectGlobalRegistry::GetInstance().findObject(id);
}

//--

static std::function<void(IObject*)> GScriptObjectDispose;
static std::function<void(IObject*)> GScriptObjectResurrect;

struct ObjectCloneContext
{
    const IObject* rootObject = nullptr;
    HashMap<const IObject*, ObjectPtr> objectTranslation;

    INLINE bool shouldClone(const IObject* obj) const
    {
        while (obj)
        {
            if (obj == rootObject)
                return true;
            obj = obj->parent();
        }

        return false;
    }

    ObjectPtr cloneObject(const IObject* object)
    {
        ObjectPtr ret = nullptr;
        if (!objectTranslation.find(object, ret))
        {
            ret = object->cloneSelf(*this);
            objectTranslation[object] = ret;
        }
        return ret;
    }

    void cloneProperties(ClassType cls, void* dest, const void* src)
    {
        for (const auto* prop : cls->allProperties().values())
        {
            const void* srcData = prop->offsetPtr(src);
            void* destData = prop->offsetPtr(dest);
            cloneData(prop->type(), destData, srcData);
        }
    }

    void cloneData(Type type, void* dest, const void* src)
    {
        switch (type.metaType())
        {
        case MetaType::Array:
        {
            const auto* arrayType = static_cast<const IArrayType*>(type.ptr());
            const auto arrayInnerType = arrayType->innerType();
            const auto size = arrayType->arraySize(src);
            for (auto i : IndexRange(size))
            {
                const void* srcElement = arrayType->arrayElementData(src, i);

                arrayType->createArrayElement(dest, i);
                if (void* destElement = arrayType->arrayElementData(dest, i))
                {
                    cloneData(arrayInnerType, destElement, srcElement);
                }
            }
            break;
        }

        case MetaType::StrongHandle:
        {
            const auto* refType = static_cast<const IHandleType*>(type.ptr());
            if (refType->pointedClass().is<IObject>())
            {
                const auto& srcPtr = *(const RefPtr<IObject>*)src;
                auto& destPtr = *(RefPtr<IObject>*)dest;

                if (shouldClone(srcPtr))
                    destPtr = cloneObject(srcPtr);
            }

            break;
        }

        case MetaType::WeakHandle:
        {
            const auto* refType = static_cast<const IHandleType*>(type.ptr());
            if (refType->pointedClass().is<IObject>())
            {
                const auto& srcWeakPtr = *(const RefWeakPtr<IObject>*)src;
                if (auto srcPtr = srcWeakPtr.lock())
                {
                    auto& destPtr = *(RefWeakPtr<IObject>*)dest;

                    if (shouldClone(srcPtr))
                        destPtr = cloneObject(srcPtr);
                }
            }

            break;
        }

        case MetaType::Class:
        {
            const auto* classType = static_cast<const IClassType*>(type.ptr());
            cloneProperties(classType, dest, src);
            break;
        }

        default:
            // simple copy is enough
            type->copy(dest, src);
        }

    }
};

void IObject::cloneProperties(ObjectCloneContext& ctx, IObject* target) const
{
    ctx.cloneProperties(cls(), target, this);
}

ObjectPtr IObject::cloneSelf(ObjectCloneContext& ctx) const
{   
    auto ret = cls()->create<IObject>();
    cloneProperties(ctx, ret);
    return ret;
}

ObjectPtr IObject::clone() const
{
    ObjectCloneContext ctx;
    ctx.rootObject = this;
    return ctx.cloneObject(this);
}

//--

ObjectSavingContext::ObjectSavingContext()
{
	pool = &MainPool();
    pagedPool = &LocalPagePool();
}

ObjectLoadingContext::ObjectLoadingContext()
{
    pool = &MainPool();
    collectedErrors = &ISerializationErrorReporter::DefaultErrorReporter();
 }


//--

void IObject::RegisterScriptObjetctDispose(const std::function<void(IObject*)>& func)
{
    GScriptObjectDispose = func;
}

void IObject::RegisterScriptObjetctResurrect(const std::function<void(IObject*)>& func)
{
    GScriptObjectResurrect = func;
}

//--

ITemplatePropertyBuilder::~ITemplatePropertyBuilder()
{}

//--

ITemplatePropertyValueContainer::~ITemplatePropertyValueContainer()
{}

//--

void* PatchClassPointer(void* ptr, const IClassType* cls)
{
    DEBUG_CHECK_EX(!cls->is<IObject>(), "Object class with no allocator");
    if (cls->is<IObject>())
    {
        void** classPtr = OffsetPtr<void*>(ptr, 24);
        *classPtr = (void*)cls;
    }

    return ptr;
}

//--

// TODO: get rid of this crap
TYPE_TLS HashMap<void*, const IClassType*>* GPendingClasses = nullptr;

static const IClassType* FetchClassType(void* ptr)
{
    DEBUG_CHECK_RETURN_V(GPendingClasses, nullptr);
    
    const IClassType* val = nullptr;
    DEBUG_CHECK_RETURN_V(GPendingClasses->find(ptr, val), nullptr);
    GPendingClasses->remove(ptr);

    DEBUG_CHECK(val);
    return val;
}

static void PushClassType(void* ptr, const IClassType* cls)
{
    DEBUG_CHECK_RETURN_EX(cls, "Trying to patch invalid class");

	if (!GPendingClasses)
		GPendingClasses = new HashMap<void*, const IClassType*>;
    (*GPendingClasses)[ptr] = cls;
    DEBUG_CHECK(GPendingClasses->size() < 16);
}

IObject::IObject()
{
    static_assert(offsetof(IObject, m_class) == 24, "Invalid class offset");

    m_class = FetchClassType(this);

    if (!IsDefaultObjectCreation())
    {
        m_id = AllocUniqueObjectID();
        m_eventKey = MakeUniqueEventKey();
        ObjectGlobalRegistry::GetInstance().registerObject(m_id, this);
    }
}

void* IObject::PatchClassPointer(void* ptr, const IClassType* cls)
{
    PushClassType(ptr, cls);
    return ptr;
}

IObject::~IObject()
{
    if (m_id)
    {
        ObjectGlobalRegistry::GetInstance().unregisterObject(m_id, this);
        m_id = 0;
    }
}

void IObject::parent(const IObject* parentObject)
{
    m_parent = nullptr;

    if (parentObject)
    {
        DEBUG_CHECK_EX(!parentObject->hasParent(this), "Recursive parenting");
        if (!parentObject->hasParent(this))
            m_parent = const_cast<IObject*>(parentObject);
    }
}

bool IObject::hasParent(const IObject* parentObject) const
{
    auto parent = m_parent.unsafe();
    while (parent)
    {
        if (parent == parentObject)
            return true;
        parent = parent->m_parent.unsafe();
    }

    return false;
}

IObject* IObject::findParent(ClassType parentObjectClass) const
{
    auto* parent = m_parent.unsafe();
    while (parent)
    {
        if (parent->is(parentObjectClass))
            return parent;
        parent = parent->m_parent.unsafe();
    }

    return nullptr;
}

bool IObject::is(ClassType objectClass) const
{
    return cls().is(objectClass);
}

void IObject::onPostLoad()
{
}
    
void IObject::print(IFormatStream& f) const
{
    f.appendf("{} ID:{} 0x{}", cls()->name(), id(), Hex(this));
}

//--

void IObject::onReadBinary(SerializationReader& reader)
{
    TypeSerializationContext typeContext;
    typeContext.directObjectContext = this;
    typeContext.parentObjectContext = this;
    cls()->readBinary(typeContext, reader, this);
}

void IObject::onWriteBinary(SerializationWriter& writer) const
{
    TypeSerializationContext typeContext;
    typeContext.directObjectContext = (IObject*)this;
    typeContext.parentObjectContext = (IObject*)this;
    cls()->writeBinary(typeContext, writer, this, cls()->defaultObject());
}

//--

void IObject::writeText(ITextSerializationWriter& writer) const
{
	TypeSerializationContext typeContext;
	typeContext.directObjectContext = (IObject*)this;
	typeContext.parentObjectContext = (IObject*)this;
	cls()->writeText(typeContext, writer, this, cls()->defaultObject());
}

void IObject::readText(ITextSerializationReader& reader)
{
	TypeSerializationContext typeContext;
	typeContext.directObjectContext = this;
	typeContext.parentObjectContext = this;
	cls()->readText(typeContext, reader, this);

	onPostLoad();
}

//--

bool IObject::onPropertyShouldSave(const Property* prop) const
{
    // skip transient properties
    if (prop->flags().test(PropertyFlagBit::Transient))
        return false;

    // compare the property value with default, do not save if the same
    auto propData = prop->offsetPtr(this);
    auto propDefaultData = prop->offsetPtr(cls()->defaultObject());
    if (prop->type()->compare(propData, propDefaultData))
        return false;

    // TODO: additional checks ?

    // we can save this property
    return true;
}

bool IObject::onPropertyShouldLoad(const SerializationPropertyInfo& prop)
{
    return true;
}

bool IObject::onPropertyMissingRaw(const SerializationPropertyInfo& info, SerializationReader& reader)
{
    // a good place to handle data conversion from old data format
    return false;
}

bool IObject::onPropertyMissing(StringID propertyName, TypedMemory data)
{
    // a good place to inject old property data into new fields
    return false;
}

bool IObject::onPropertyTypeChangedRaw(const SerializationPropertyInfo& info, TypedMemory propertyData, SerializationReader& reader)
{
    // TODO: try to use RTTI system, add some "type converters" to types
    
    // good place to do low-level data conversion
    return false;
}

bool IObject::onPropertyTypeChanged(StringID propertyName, TypedMemory propertyData, TypedMemory loadedData)
{
    // use automatic conversion
    return ConvertData(loadedData, propertyData);
}

//--

void IObject::markModified()
{
    auto parent = this->parent();
    if (parent)
        parent->markModified();
}

//--

SpecificClassType<IObject> IObject::GetStaticClass()
{
    static ClassType objectType = RTTI::GetInstance().findClass("IObject"_id);
    return SpecificClassType<IObject>(*objectType.ptr());
}

//--

DataViewResult IObject::describeDataView(StringView viewPath, DataViewInfo& outInfo) const
{
    if (viewPath.empty())
    {
        outInfo.flags |= DataViewInfoFlagBit::Object;
        outInfo.flags |= DataViewInfoFlagBit::LikeStruct;

        if (outInfo.requestFlags.test(DataViewRequestFlagBit::ObjectInfo))
        {
            outInfo.objectClass = cls().ptr(); // get DYNAMIC class
            outInfo.objectPtr = this;
        }
    }

    return cls()->describeDataView(viewPath, this, outInfo);
}

DataViewResult IObject::readDataView(StringView viewPath, void* targetData, Type targetType) const
{
    if (viewPath == "__cls")
    {
        // TODO
    }

    return cls()->readDataView(viewPath, this, targetData, targetType.ptr());
}

DataViewResult IObject::writeDataView(StringView viewPath, const void* sourceData, Type sourceType)
{
    if (!onPropertyChanging(viewPath, sourceData, sourceType))
        return DataViewResultCode::ErrorIllegalOperation;

    auto ret = cls()->writeDataView(viewPath, this, sourceData, sourceType.ptr());
    if (!ret.valid())
        return ret;

    onPropertyChanged(viewPath);
    return DataViewResultCode::OK;
}

//--

DataViewPtr IObject::createDataView(bool forceReadOnly) const
{
    return RefNew<DataViewNative>(const_cast<IObject*>(this), forceReadOnly);
}

bool IObject::initializeFromTemplateProperties(const ITemplatePropertyValueContainer& templateProperties)
{
    for (const auto& prop : cls()->allTemplateProperties())
    {
        if (prop.nativeProperty)
        {
            auto* localData = prop.nativeProperty->offsetPtr(this);
            //TRACE_INFO("Applying '{}', offset {}, extended {}", prop.nativeProperty->name(), prop.nativeProperty->offset(), prop.nativeProperty->externalBuffer());
            templateProperties.compileValue(prop.name, prop.nativeProperty->type(), localData);
        }
    }

    return true;
}

void IObject::queryTemplateProperties(ITemplatePropertyBuilder& outTemplateProperties) const
{
    // nothing special here, all default overridable properties are filled by class type
}

//--

bool IObject::onPropertyChanging(StringView path, const void* newData, Type newDataType) const
{
    return true;
}

void IObject::onPropertyChanged(StringView path)
{
    markModified();

    static const auto stringType = RTTI::GetInstance().findType("StringBuf"_id);
    StringBuf stringPath(path);

    postEvent(EVENT_OBJECT_PROPERTY_CHANGED, &stringPath, stringType);
}

bool IObject::onPropertyFilter(StringID propertyName) const
{
    return true;
}

//---

bool IObject::onResourceReloading(IResource* currentResource, IResource* newResource)
{
    InplaceArray<StringID, 16> affectedProperties;
    if (!cls()->patchResourceReferences(this, currentResource, newResource, &affectedProperties))
        return false;

    for (const auto& propName : affectedProperties)
        onPropertyChanged(propName.view());

    return true;
}

void IObject::onResourceReloadFinished(IResource* currentResource, IResource* newResource)
{

}

//---

void IObject::postEvent(StringID eventID, const void* data, Type dataType)
{
    DispatchGlobalEvent(m_eventKey, eventID, this, data, dataType);
}

//--

void IObject::RegisterType(TypeSystem& typeSystem)
{
    NativeClass* ret = new NativeClass("IObject", sizeof(IObject), alignof(IObject), typeid(IObject).hash_code());
    ret->bindCtorDtor<IObject>();
    typeSystem.registerType(ret);
}

//--

extern void CreateType_Buffer(const char* name);
extern void InitType_Buffer();
extern void CreateType_AsyncFileBuffer(const char* name);
extern void InitType_AsyncFileBuffer();
extern void CreateType_GUID(const char* name);
extern void InitType_GUID();
extern void CreateType_ResourceID(const char* name);
extern void InitType_ResourceID();
extern void CreateType_Variant(const char* name);
extern void InitType_Variant();
extern void CreateType_Type(const char* name);
extern void InitType_Type();
extern void CreateType_ClassType(const char* name);
extern void InitType_ClassType();
extern void CreateType_VariantTable(const char* name);
extern void InitType_VariantTable();
extern void CreateType_TagList(const char* name);
extern void InitType_TagList();
extern void CreateType_TimeStamp(const char* name);
extern void InitType_TimeStamp();

void RegisterObjectTypes(TypeSystem &typeSystem)
{
    IObject::RegisterType(typeSystem);
    IMetadata::RegisterType(typeSystem);
    ShortTypeNameMetadata::RegisterType(typeSystem);
    TestOrderMetadata::RegisterType(typeSystem);
    ClassVersionMetadata::RegisterType(typeSystem);

	CreateType_Buffer("Buffer");
    CreateType_AsyncFileBuffer("AsyncFileBuffer");
	CreateType_GUID("GUID");
    CreateType_ResourceID("ResourceID");
    CreateType_Variant("Variant");
    CreateType_Type("Type");
    CreateType_ClassType("ClassType");
    CreateType_VariantTable("VariantTable");
    CreateType_TagList("TagList");
    CreateType_TimeStamp("TimeStamp");

	InitType_Buffer();
	InitType_AsyncFileBuffer();
	InitType_GUID();
	InitType_ResourceID();
	InitType_Variant();
	InitType_Type();
	InitType_ClassType();
	InitType_VariantTable();
	InitType_TagList();
	InitType_TimeStamp();

    //DEBUG_CHECK_RETURN(IMetadata::GetStaticClass());
    //DEBUG_CHECK_RETURN(IObject::GetStaticClass());
}

//--

bool IObject::CopyPropertyValue(const IObject* srcObject, const Property* srcProperty, IObject* targetObject, const Property* targetProperty)
{
    const auto srcType = srcProperty->type();
    const auto targetType = targetProperty->type();

    const auto* srcData = srcProperty->offsetPtr(srcObject);
    auto* targetData = targetProperty->offsetPtr(targetObject);

    // TODO: support for inlined objects! and more complex types

    if (!ConvertData(srcData, srcType, targetData, targetType))
        return false;

    return true;
}

//--

IHackyFontGlyphCacheIntegration::~IHackyFontGlyphCacheIntegration()
{}

void IHackyFontGlyphCacheIntegration::RunWithPointer(const std::function<void(IHackyFontGlyphCacheIntegration*&)>& func)
{
    static IHackyFontGlyphCacheIntegration* ptr = nullptr;
    static Mutex lock;

    lock.acquire();
    func(ptr);
    lock.release();
}

void IHackyFontGlyphCacheIntegration::PullNewData(Array<uint8_t>& outData, uint32_t& outOffset)
{
    RunWithPointer([&outData, &outOffset](IHackyFontGlyphCacheIntegration*& ptr)
        {
            if (ptr)
                ptr->pullNewData(outData, outOffset);
        });
}

//--

END_INFERNO_NAMESPACE()
