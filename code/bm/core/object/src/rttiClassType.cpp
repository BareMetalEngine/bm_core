/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "rttiClassType.h"
#include "rttiProperty.h"
#include "rttiFunction.h"
#include "rttiTypeSystem.h"
#include "rttiHandleType.h"
#include "rttiArrayType.h"
#include "rttiDataView.h"
#include "rttiDataHolder.h"
#include "rttiTypeRef.h"
#include "rttiResourceReferenceType.h"

#include "serializationWriter.h"
#include "serializationReader.h"
#include "serializationErrorReporter.h"

#include "textSerializationWriter.h"
#include "textSerializationReader.h"

BEGIN_INFERNO_NAMESPACE()

//--

TemplateProperty::TemplateProperty(Type type_)
    : type(type_)
{
    defaultValue = Memory::AllocateBlock(type->size(), type->alignment(), "DefaultValue");
    memzero(defaultValue, type->size());

    type->construct(defaultValue);
}

TemplateProperty::~TemplateProperty()
{
    if (defaultValue)
    {
        //type->destruct(defaultValue);

        Memory::FreeBlock(defaultValue);
        defaultValue = nullptr;
    }
}

//--

IClassType::IClassType(StringID name, uint32_t size, uint32_t alignment)
    : IType(name)
    , m_userIndex(INDEX_NONE)
    , m_baseClass(nullptr)
{
    m_traits.metaType = MetaType::Class;
    m_traits.size = size;
    m_traits.alignment = alignment;
}

IClassType::~IClassType()
{
    clearCachedData();

    //m_localProperties.clearPtr();
    //m_localFunctions.clearPtr();
}

bool IClassType::compare(const void* data1, const void* data2) const
{
    for (auto prop  : allProperties().values())
        if (!prop->type()->compare(prop->offsetPtr(data1), prop->offsetPtr(data2)))
            return false;

    return true;
}

void IClassType::copy(void* dest, const void* src) const
{
    for (auto prop  : allProperties().values())
        prop->type()->copy(prop->offsetPtr(dest), prop->offsetPtr(src));
}
        
void IClassType::writeBinary(TypeSerializationContext& typeContext, SerializationWriter& file, const void* data, const void* defaultData) const
{
    TypeSerializationContextSetClass classContext(typeContext, this);

    // get the default data directly from class for if nothing specified
    if (!defaultData)
        defaultData = defaultObject();

    // save properties
    for (const auto* prop  : allProperties().values())
    {
        // skip transient properties
        if (prop->flags().test(PropertyFlagBit::Transient))
            continue;

        // ask object if we want to save this property
        auto propData = prop->offsetPtr(data);
        auto propDefaultData = defaultData ? prop->offsetPtr(defaultData) : nullptr;
        if (typeContext.directObjectContext)
        {
            // object may request property NOT to be saved for "reasons" or may tell us to save property even if the data is different, also for "reasons"
            if (!typeContext.directObjectContext->onPropertyShouldSave(prop))
                continue;
        }
        else
        {
            // compare the property value with default, do not save if the same
            if (propDefaultData)
                if (prop->type()->compare(propData, propDefaultData))
                    continue;
        }

        // write the reference to the property (it'll be mapped to an index as it's much faster to load it like that compared to ie. finding it by name every time)
        file.writeProperty(prop);

        // save property data in a skip able block so we can go over it if we can't read it
        {
            TypeSerializationContextSetProperty propertyContext(typeContext, prop);

            file.beginSkipBlock();
            prop->type()->writeBinary(typeContext, file, propData, propDefaultData);
            file.endSkipBlock();
        }
    }

    // write null property as sentinel
    file.writeProperty(nullptr);
}

namespace helper
{
    static bool AreTypesBinaryComaptible(Type originalType, Type targetType)
    {
        if (!originalType)
            return false;

        else if (originalType == targetType)
            return true;

        else if (originalType->metaType() == MetaType::ResourceRef && targetType->metaType() == MetaType::ResourceRef)
            return true;

        else if (originalType->metaType() == MetaType::StrongHandle && targetType->metaType() == MetaType::StrongHandle)
            return true;

        else if (originalType->metaType() == MetaType::WeakHandle && targetType->metaType() == MetaType::WeakHandle)
            return true;

        else if (originalType->metaType() == MetaType::Array && targetType->metaType() == MetaType::Array)
            return AreTypesBinaryComaptible(originalType.innerType(), targetType.innerType());

        return false;
    }
}

void IClassType::readBinary(TypeSerializationContext& typeContext, SerializationReader& file, void* data) const
{
    TypeSerializationContextSetClass classContext(typeContext, this);

    // while all properties until we get a null property
    for (;;)
    {
		// read the property reference
        SerializationPropertyInfo propertyInfo;
        if (!file.readProperty(propertyInfo))
            break;

        // enter data skip block for the whole property value to recover from any stream errors
        SerializationSkipBlock block(file);

        // ask if we want to read this property
        if (propertyInfo.prop && typeContext.directObjectContext && !typeContext.directObjectContext->onPropertyShouldLoad(propertyInfo))
        {
            TRACE_WARNING("Discarded property at {}: property '{}' was discarded by object", typeContext, propertyInfo.name);
            continue;
        }

        // handle any issues with properties
        const auto result = handlePropertyIssues(typeContext, propertyInfo, data, file);
        if (result != PropertyIssuesResult::NoIssues)
        {
            if (result == PropertyIssuesResult::NotHandled)
            {
                TRACE_INFO("Unabled data change on property '{} {}::{}', data will be lost",
                    propertyInfo.typeName, name(), propertyInfo.name);
            }
            continue;
        }

        // load property data directly
        {
            TypeSerializationContextSetProperty propertyContext(typeContext, propertyInfo.prop);

            void* targetData = propertyInfo.prop->offsetPtr(data);
            propertyInfo.prop->type()->readBinary(typeContext, file, targetData);
        }
    }
}

//--

void IClassType::writeText(TypeSerializationContext& typeContext, ITextSerializationWriter& file, const void* data, const void* defaultData) const
{
	TypeSerializationContextSetClass classContext(typeContext, this);

	// get the default context for saving if not specified
	if (!defaultData)
		defaultData = defaultObject();

    // begin compound
    file.beginStruct(this, data);

	// save properties
	for (const auto* prop : allProperties().values())
	{
		// skip transient properties
		if (prop->flags().test(PropertyFlagBit::Transient))
			continue;

		// ask object if we want to save this property
		auto propData = prop->offsetPtr(data);
		auto propDefaultData = defaultData ? prop->offsetPtr(defaultData) : nullptr;
		if (typeContext.directObjectContext)
		{
			// object may request property NOT to be saved for "reasons" or may tell us to save property even if the data is different, also for "reasons"
			if (!typeContext.directObjectContext->onPropertyShouldSave(prop))
				continue;
		}
		else
		{
			// compare the property value with default, do not save if the same
			if (propDefaultData)
				if (prop->type()->compare(propData, propDefaultData))
					continue;
		}

		// properties are saved in child nodes
		{
            file.beginStructElement(prop->name().view());

			TypeSerializationContextSetProperty propertyContext(typeContext, prop);
			prop->type()->writeText(typeContext, file, propData, propDefaultData);

            file.endStructElement(prop->name().view());
		}
	}

    // end compound
    file.endStruct(this, data);
}

void IClassType::readText(TypeSerializationContext& typeContext, ITextSerializationReader& file, void* data) const
{
	TypeSerializationContextSetClass classContext(typeContext, this);

	// begin compound
    if (!file.beginStruct())
        return;

	// process properties
    StringView memberName;
    while (file.beginStructElement(memberName))
    {
        // find property
        StringID memberNameId(memberName);
        if (const auto* prop = findProperty(memberNameId))
        {
            // property data 
            void* propData = prop->offsetPtr(data);

            // load property's value
			TypeSerializationContextSetProperty propertyContext(typeContext, prop);
			prop->type()->readText(typeContext, file, propData);
		}
        else
        {
            file.reportError(TempString("Type '{}' has no member '{}'", name(), memberName));
        }

        // close
        file.endStructElement();
	}

	// end compound
    file.endStruct();
}

//--

DataViewResult IClassType::describeDataView(StringView viewPath, const void* viewData, DataViewInfo& outInfo) const
{
    StringView propertyName;

    if (viewPath.empty())
    {
        if (outInfo.requestFlags.test(DataViewRequestFlagBit::MemberList))
        {
            for (const auto* prop : allProperties().values())
            {
                if (outInfo.categoryFilter && prop->category() != outInfo.categoryFilter)
                    continue;

                if (prop->editable())
                {
                    auto& memberInfo = outInfo.members.emplaceBack();
                    memberInfo.name = prop->name();
                    memberInfo.category = prop->category();
                    memberInfo.type = prop->type();
                }
            }
        }

        outInfo.flags |= DataViewInfoFlagBit::LikeStruct;
                
        return IType::describeDataView(viewPath, viewData, outInfo);
    }
    else if (ParsePropertyName(viewPath, propertyName))
    {
        if (auto prop  = findProperty(StringID::Find(propertyName)))
        {
            auto propViewData  = prop->offsetPtr(viewData);

            if (viewPath.empty())
            {
                if (outInfo.requestFlags.test(DataViewRequestFlagBit::PropertyMetadataEx))
                    prop->collectMetadataList(outInfo.typeMetadata);

                if (outInfo.requestFlags.test(DataViewRequestFlagBit::PropertyEditorData))
                    outInfo.editorData = prop->editorData();

                if (prop->inlined())
                    outInfo.flags |= DataViewInfoFlagBit::Inlined;
                if (prop->readonly())
                    outInfo.flags |= DataViewInfoFlagBit::ReadOnly;
            }
            else if (prop->type().metaType() == MetaType::Array)
            {
                uint32_t arrayIndex = 0;
                auto tempViewPath = viewPath;
                if (ParseArrayIndex(tempViewPath, arrayIndex) && tempViewPath.empty())
                {
                    if (outInfo.requestFlags.test(DataViewRequestFlagBit::PropertyMetadataEx))
                        prop->collectMetadataList(outInfo.typeMetadata);

                    if (outInfo.requestFlags.test(DataViewRequestFlagBit::PropertyEditorData))
                        outInfo.editorData = prop->editorData();

                    if (prop->inlined())
                        outInfo.flags |= DataViewInfoFlagBit::Inlined;
                    if (prop->readonly())
                        outInfo.flags |= DataViewInfoFlagBit::ReadOnly;
                }
            }

            return prop->type()->describeDataView(viewPath, propViewData, outInfo);
        }
        else
        {
            return DataViewResultCode::ErrorUnknownProperty;
        }
    }

    return DataViewResultCode::ErrorIllegalAccess;
}

DataViewResult IClassType::readDataView(StringView viewPath, const void* viewData, void* targetData, Type targetType) const
{
    const auto orgDataView = viewPath;

    if (viewPath.empty())
        return IType::readDataView(viewPath, viewData, targetData, targetType);

    StringView propertyName;
    if (ParsePropertyName(viewPath, propertyName))
    {
        if (auto prop  = findProperty(StringID::Find(propertyName)))
        {
            auto propViewData  = prop->offsetPtr(viewData);
            return prop->type()->readDataView(viewPath, propViewData, targetData, targetType);
        }
    }

    return IType::readDataView(orgDataView, viewData, targetData, targetType);
}

DataViewResult IClassType::writeDataView(StringView viewPath, void* viewData, const void* sourceData, Type sourceType) const
{
    const auto orgDataView = viewPath;

    if (viewPath.empty())
        return IType::writeDataView(viewPath, viewData, sourceData, sourceType);

    StringView propertyName;
    if (ParsePropertyName(viewPath, propertyName))
    {
        if (auto prop  = findProperty(StringID::Find(propertyName)))
        {
            auto propViewData  = prop->offsetPtr(viewData);
            return prop->type()->writeDataView(viewPath, propViewData, sourceData, sourceType);
        }
    }

    return IType::writeDataView(orgDataView, viewData, sourceData, sourceType);
}

bool IClassType::is(ClassType otherClass) const
{
    ClassType testClass = this;
    while (testClass != nullptr)
    {
        if (otherClass == testClass)
            return true;

        testClass = testClass->baseClass();
    }

    return !otherClass; // "null" class is a base of all classes
}

const Property* IClassType::findProperty(StringID propertyName) const
{
    const Property* prop = nullptr;
    allProperties().find(propertyName, prop);
    return prop;
}

const NativeFunction* IClassType::findNativeFunction(StringID functionName) const
{
    const NativeFunction* func = nullptr;
    allNativeFunctions().find(functionName, func);
    return func;
}

const IScriptFunction* IClassType::findScriptFunction(StringID functionName) const
{
    const IScriptFunction* func = nullptr;
    allScriptFunctions().find(functionName, func);
    return func;
}

const IClassType::TConstProperties& IClassType::allProperties() const
{
    if (auto* propList = m_allProperties.load())
        return *propList;

    //--

    auto lock = CreateLock(m_listBuildLock);

    auto* propList = new TConstProperties();

    if (m_baseClass)
    {
        propList->reserve(m_baseClass->allProperties().size() + m_localProperties.size());

        for (const auto& prop : m_baseClass->allProperties().pairs())
            propList->set(prop.key, prop.value);
    }
    else
    {
        propList->reserve(m_localProperties.size());
    }

    for (const auto* prop : m_localProperties)
        propList->set(prop->name(), prop);

    m_allProperties = propList;

    //--

    return *propList;
}

const IClassType::TConstNativeFunctions& IClassType::allNativeFunctions() const
{
    if (auto* funcList = m_allNativeFunctions.load())
        return *funcList;

    auto lock = CreateLock(m_listBuildLock);

    auto* funcList = new TConstNativeFunctions();

    if (m_baseClass)
    {
        funcList->reserve(m_baseClass->allNativeFunctions().size() + m_localNativeFunctions.size());

        for (const auto& prop : m_baseClass->allNativeFunctions().pairs())
            funcList->set(prop.key, prop.value);
    }
    else
    {
        funcList->reserve(m_localNativeFunctions.size());
    }

    for (const auto* func : m_localNativeFunctions)
        funcList->set(func->name(), func);

    m_allNativeFunctions = funcList;

    return *funcList;
}

const IClassType::TConstScriptFunctions& IClassType::allScriptFunctions() const
{
    if (auto* funcList = m_allScriptFunctions.load())
        return *m_allScriptFunctions;

    auto lock = CreateLock(m_listBuildLock);

    auto* funcList = new TConstScriptFunctions();

    if (m_baseClass)
    {
        funcList->reserve(m_baseClass->allScriptFunctions().size() + m_localScriptFunctions.size());

        for (const auto& pair : m_baseClass->allScriptFunctions().pairs())
            funcList->set(pair.key, pair.value);
    }
    else
    {
        funcList->reserve(m_localScriptFunctions.size());
    }

    for (const auto* func : m_localScriptFunctions)
        funcList->set(func->name(), func);

    m_allScriptFunctions = funcList;

    return *funcList;
}

class LocalClassTemplatePropertyCollector : public ITemplatePropertyBuilder
{
public:
    LocalClassTemplatePropertyCollector(Array<TemplateProperty>& outList)
        : m_outList(outList)
    {
    }

    virtual void prop(StringID category, StringID name, Type type, const void* defaultValue, const PropertyEditorData& editorData)
    {
        if (!category)
            category = "Template Properties"_id;

        DEBUG_CHECK_RETURN_EX(name, "Invalid name");
        DEBUG_CHECK_RETURN_EX(type, "Invalid type");

        for (auto index : m_outList.indexRange())
            if (m_outList[index].name == name)
                m_outList.erase(index);

        auto& entry = m_outList.emplaceBack(type);
        entry.category = category;
        entry.name = name;
        entry.editorData = editorData;
        entry.nativeProperty = nullptr;

        if (defaultValue)
            entry.type->copy(entry.defaultValue, defaultValue);
    }

    //--

    Array<TemplateProperty>& m_outList;
};

const IClassType::TConstTemplateProperties& IClassType::allTemplateProperties() const
{
    if (auto* propList = m_allTemplateProperties.load())
        return *propList;

    auto propList = new TConstTemplateProperties();

    if (is<IObject>() && !isAbstract())
    {
        auto* def = (IObject*)defaultObject();

        LocalClassTemplatePropertyCollector localCollector(*propList);

        // collect the native properties that are overridable
        {
            for (const auto* prop : allProperties().values())
            {
                if (prop->overridable()) // TODO: different tag ?
                {
                    const void* defaultData = prop->offsetPtr(def);

                    auto& entry = localCollector.m_outList.emplaceBack(prop->type());
                    prop->type()->copy(entry.defaultValue, defaultData);
                    entry.nativeProperty = prop;
                    entry.category = prop->category();
                    entry.name = prop->name();
                    entry.editorData = prop->editorData();
                }
            }
        }

        // get additional properties
        def->queryTemplateProperties(localCollector);
    }

    // store
    TConstTemplateProperties* existingProps = nullptr;
    if (!m_allTemplateProperties.compare_exchange_strong(existingProps, propList))
    {
        delete propList;
        return *existingProps;
    }

    return *propList;
}

const IMetadata* IClassType::metadata(ClassType metadataType) const
{
    // check the whole class hierarchy
    ClassType testClass = this;
    while (testClass != nullptr)
    {
        auto metadata  = testClass->MetadataContainer::metadata(metadataType);
        if (metadata != nullptr)
            return metadata;

        testClass = testClass->baseClass();
    }

    // meta data not found
    return nullptr;
}

void IClassType::collectMetadataList(Array<const IMetadata*>& outMetadataList) const
{
    if (m_baseClass)
        m_baseClass->collectMetadataList(outMetadataList);
    MetadataContainer::collectMetadataList(outMetadataList);
}

void IClassType::clearCachedData()
{
    delete m_allScriptFunctions.exchange(nullptr);
    delete m_allNativeFunctions.exchange(nullptr);
    delete m_allProperties.exchange(nullptr);
    delete m_allTemplateProperties.exchange(nullptr);
}

void IClassType::baseClass(ClassType baseClass)
{
    if (baseClass == this)
        baseClass = nullptr; // special case for  root classes

    DEBUG_CHECK_RETURN_EX(m_baseClass == nullptr, "Base class already set");
    DEBUG_CHECK_RETURN_EX(!baseClass || !baseClass->is(this), "Recursive class chain");

    if (baseClass != m_baseClass)
    {
        clearCachedData();

        m_baseClass = baseClass.ptr();

        if (m_baseClass && m_baseClass->name().view().endsWith("Metadata"))
        {
            DEBUG_CHECK_EX(name().view().endsWith("Metadata"), TempString("Metadata class '{}' name must end with Metadata", name()));
        }
    }
}

void IClassType::addProperty(Property* property)
{
    DEBUG_CHECK_RETURN(property != nullptr);
    DEBUG_CHECK_RETURN(property->parent() == this);
    DEBUG_CHECK_RETURN(!property->name().empty());
    DEBUG_CHECK_RETURN(!m_localProperties.contains(property));
    
    for (auto* prop : m_localProperties)
    {
        DEBUG_CHECK_RETURN(prop->name() != property->name());
    }

    m_localProperties.pushBack(property);

    delete m_allProperties.exchange(nullptr);
    delete m_allTemplateProperties.exchange(nullptr);
}

void IClassType::addNativeFunction(NativeFunction* function)
{
    DEBUG_CHECK_RETURN(function != nullptr);
    DEBUG_CHECK_RETURN(function->parent() == this);
    DEBUG_CHECK_RETURN(!function->name().empty());
    DEBUG_CHECK_RETURN(!m_localNativeFunctions.contains(function));

    for (auto* func : m_localNativeFunctions)
    {
        DEBUG_CHECK_RETURN(func->name() != function->name());
    }

    for (auto* func : m_localScriptFunctions)
    {
        DEBUG_CHECK_RETURN(func->name() != function->name());
    }

    m_localNativeFunctions.pushBack(function);
    
    delete m_allNativeFunctions.exchange(nullptr); // reset cache
}

void IClassType::addScriptFunction(IScriptFunction* function)
{
    DEBUG_CHECK_RETURN(function != nullptr);
    DEBUG_CHECK_RETURN(function->parent() == this);
    DEBUG_CHECK_RETURN(!function->name().empty());
    DEBUG_CHECK_RETURN(!m_localScriptFunctions.contains(function));

    for (auto* func : m_localNativeFunctions)
    {
        DEBUG_CHECK_RETURN(func->name() != function->name());
    }

    for (auto* func : m_localScriptFunctions)
    {
        DEBUG_CHECK_RETURN(func->name() != function->name());
    }

    m_localScriptFunctions.pushBack(function);

    delete m_allScriptFunctions.exchange(nullptr); // reset cache
}

void IClassType::resetCachedLists()
{
    delete m_allProperties.exchange(nullptr);
    delete m_allTemplateProperties.exchange(nullptr);
    delete m_allNativeFunctions.exchange(nullptr);
    delete m_allScriptFunctions.exchange(nullptr);
}

IClassType::PropertyIssuesResult IClassType::handlePropertyIssues(TypeSerializationContext& context, const SerializationPropertyInfo& info, void* data, SerializationReader& reader) const
{
    // property is missing
    if (!info.prop)
    {
        // ask object to handle
        if (context.directObjectContext && context.directObjectContext->onPropertyMissingRaw(info, reader))
        {
            reader.errors().reportPropertyMissing(reader.context(), context.classContextName(), info.name, info.typeName, true);
            return PropertyIssuesResult::HandledByObject;
        }

        // if we have for that missing data use it to load content to temporary storage
        if (info.type)
        {
            DataHolder tempData(info.type);
            info.type->readBinary(context, reader, tempData.data());

            // try version with data
            if (context.directObjectContext && context.directObjectContext->onPropertyMissing(info.name, tempData.view()))
            {
                reader.errors().reportPropertyMissing(reader.context(), context.classContextName(), info.name, info.typeName, true);
                return PropertyIssuesResult::HandledByObject;
            }
        }

        // we can't do much more here - property was not handled in RAW form and it was not handled in converted form
		reader.errors().reportPropertyMissing(reader.context(), context.classContextName(), info.name, info.typeName, false);
        return PropertyIssuesResult::NotHandled;
    }

    // we have valid local property, get pointer to data 
    const auto propertyData = TypedMemory(info.prop->type(), info.prop->offsetPtr(data));

    // type the data was serialized with is missing
    if (!info.type)
    {
        // use the raw handling as there's no conversion option
        if (context.directObjectContext && context.directObjectContext->onPropertyTypeChangedRaw(info, propertyData, reader))
        {
            reader.errors().reportPropertyTypeMissing(reader.context(), context.classContextName(), info.name, info.typeName, true);
            return PropertyIssuesResult::HandledByObject;
        }

        // TODO: add ability to register handlers directly in types or globally to handle data conversion
        reader.errors().reportPropertyTypeMissing(reader.context(), context.classContextName(), info.name, info.typeName, false);
        return PropertyIssuesResult::NotHandled;
    }

    // type is there, but is this the same type :)
    if (info.type != info.prop->type())
    {
        // load data to temporary storage
		DataHolder tempData(info.type);
        info.type->readBinary(context, reader, tempData.data());

		// use the object handling
        if (context.directObjectContext && context.directObjectContext->onPropertyTypeChanged(info.name, propertyData, tempData.view()))
        {
            reader.errors().reportPropertyTypeChanged(reader.context(), context.classContextName(), info.name, info.typeName, info.prop->type().name(), true);
            return PropertyIssuesResult::HandledByObject;
        }

        // maybe we can handle it by automatic type conversion
        if (ConvertData(tempData.view(), propertyData))
        {
            reader.errors().reportPropertyTypeChanged(reader.context(), context.classContextName(), info.name, info.typeName, info.prop->type().name(), true);
            return PropertyIssuesResult::HandledGeneric;
        }

        // type change not handled
        reader.errors().reportPropertyTypeChanged(reader.context(), context.classContextName(), info.name, info.typeName, info.prop->type().name(), false);
        return PropertyIssuesResult::NotHandled;
    }

    // property is there and the type matches what we saved with, assume type will be internally compatible
    return PropertyIssuesResult::NoIssues;
}

void IClassType::cacheTypeData()
{
    IType::cacheTypeData();

    if (auto metadata  = static_cast<const ShortTypeNameMetadata*>(MetadataContainer::metadata(ShortTypeNameMetadata::GetStaticClass())))
        m_shortName = StringID(metadata->shortName());
    else
        m_shortName = StringID(name().view().afterLast("::", StringCaseComparisonMode::NoCase, StringFindFallbackMode::Full).afterLast("_", StringCaseComparisonMode::NoCase, StringFindFallbackMode::Full));
}

void IClassType::releaseTypeReferences()
{
    clearCachedData();

    m_localProperties.clearPtr();
    m_localScriptFunctions.clearPtr();
    m_localNativeFunctions.clearPtr();
}

static std::atomic<int> GGlobalClassIndex(0);

int IClassType::assignUserIndex() const
{
    auto index = m_userIndex.load();
    if (index == -1)
    {
        auto assignedIndex = GGlobalClassIndex++;
        m_userIndex.compare_exchange_strong(index, assignedIndex);
    }

    return index;
}

//--

uint32_t IClassType::version() const
{
    if (const auto* metadata = findMetadata<ClassVersionMetadata>())
        return metadata->version();
    return 0;
}

//--

BM_CORE_OBJECT_API bool PatchResourceReferences(Type type, void* data, IResource* currentResource, IResource* newResource)
{
    bool patched = false;

    switch (type.metaType())
    {
        case MetaType::ResourceRef:
        {
            const auto* specificType = static_cast<const IResourceReferenceType*>(type.ptr());
            patched = specificType->referencePatchResource(data, currentResource, newResource);
            break;
        }

        case MetaType::Array:
        {
            const auto* specificType = static_cast<const IArrayType*>(type.ptr());
            const auto innerType = specificType->innerType();
            const auto size = specificType->arraySize(data);
            for (uint32_t i = 0; i < size; ++i)
            {
                auto arrayElementData = specificType->arrayElementData(data, i);
                patched |= PatchResourceReferences(innerType, arrayElementData, currentResource, newResource);
            }
            break;
        }

        case MetaType::Class:
        {
            const auto* specificType = static_cast<const IClassType*>(type.ptr());
            return specificType->patchResourceReferences(data, currentResource, newResource, nullptr);
        }

        default:
            break;
    }

    return patched;
}

bool IClassType::patchResourceReferences(void* data, IResource* currentResource, IResource* newResource, Array<StringID>* outPatchedProperties) const
{
    bool patched = false;

    for (const auto* prop : allProperties().values())
    {
        auto* propertyData = prop->offsetPtr(data);
        if (PatchResourceReferences(prop->type(), propertyData, currentResource, newResource))
        {
            if (outPatchedProperties)
                outPatchedProperties->pushBack(prop->name());
            patched = true;
        }
    }

    return patched;
}

void* IClassType::allocateClassMemory(uint32_t size, uint32_t alignment) const
{
    void* ptr = Memory::AllocateBlock(size, alignment, m_name.c_str());

#ifdef BUILD_DEBUG
    memset(ptr, 0xB0, size); // fill with crap to detect uninitialized properties better
#endif

/*#ifndef BUILD_FINAL
    *(void**)OffsetPtr(ptr, 24) = nullptr;
#endif*/

    return ptr;
}

void IClassType::freeClassMemory(void* ptr) const
{
    Memory::FreeBlock(ptr);
}

//--

END_INFERNO_NAMESPACE()
