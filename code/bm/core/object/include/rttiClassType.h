/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
*
* [#filter: rtti\types\class #]
***/

#pragma once

#include "rttiType.h"
#include "rttiProperty.h"
#include "rttiFunctionPointer.h"
#include "bm/core/containers/include/array.h"
#include "bm/core/containers/include/stringID.h"
#include "bm/core/containers/include/hashMap.h"

BEGIN_INFERNO_NAMESPACE()

class ClassBuilder;
class Property;
class Interface;

/// property for templated object
struct BM_CORE_OBJECT_API TemplateProperty
{
    Type type;
    StringID name;
    StringID category;
    void* defaultValue = nullptr;
    PropertyEditorData editorData;
    const Property* nativeProperty = nullptr;

    TemplateProperty(Type type);
    ~TemplateProperty();
};

/// type describing a class, can be a native or abstract class, depends
class BM_CORE_OBJECT_API IClassType : public IType
{
public:
    IClassType(StringID name, uint32_t size, uint32_t alignment);
    virtual ~IClassType();

    // is this an abstract class
    virtual bool isAbstract() const = 0;

    // can we create objects from this class ? (does it have constructor, NOTE: we may have non abstract non-constructible classes - ie. no default constructor)
    virtual bool isConstructible() const = 0;

    // get default object for this class
    // NOTE: does not exist for abstract or runtime only classes
    virtual const void* defaultObject() const = 0;

    // create a default object
    virtual const void* createDefaultObject() const = 0;

    // destroy default object
    virtual void destroyDefaultObject() const = 0;

    // IRTTI TYPE INTERFACE IMPLEMENTATION
    virtual bool compare(const void* data1, const void* data2) const override;
    virtual void copy(void* dest, const void* src) const override;

    // TYPE SERIALIZATON
    virtual void writeBinary(TypeSerializationContext& typeContext, SerializationWriter& file, const void* data, const void* defaultData) const override;
    virtual void readBinary(TypeSerializationContext& typeContext, SerializationReader& file, void* data) const override;
	virtual void writeText(TypeSerializationContext& typeContext, ITextSerializationWriter& file, const void* data, const void* defaultData) const override final;
	virtual void readText(TypeSerializationContext& typeContext, ITextSerializationReader& file, void* data) const override final;

    // DATA VIEW
    virtual DataViewResult describeDataView(StringView viewPath, const void* viewData, DataViewInfo& outInfo) const override final;
    virtual DataViewResult readDataView(StringView viewPath, const void* viewData, void* targetData, Type targetType) const override final;
    virtual DataViewResult writeDataView(StringView viewPath, void* viewData, const void* sourceData, Type sourceType) const override final;


    //--

    // get metadata by type, if the metadata is not found in this class the parent class is checked
    virtual const IMetadata* metadata(ClassType metadataType) const override;

    // collect metadata from this class and base classes
    virtual void collectMetadataList(Array<const IMetadata*>& outMetadataList) const override;

    // get local (this class only) properties
    typedef HashMap<StringID, const Property*> TConstProperties;
    INLINE const Array<const Property*>& localProperties() const { return (const Array<const Property*>&)m_localProperties; }

    // get local (this class only) functions
    typedef HashMap<StringID, const NativeFunction*> TConstNativeFunctions;
    INLINE const Array<const NativeFunction*>& localNativeFunctions() const { return (const Array<const NativeFunction*>&)m_localNativeFunctions; }

    // get local (this class only) script functions
    typedef HashMap<StringID, const IScriptFunction*> TConstScriptFunctions;
    INLINE const Array<const IScriptFunction*>& localScriptFunctions() const { return (const Array<const IScriptFunction*>&)m_localScriptFunctions; }

    // get the declared base class of this class
    INLINE ClassType baseClass() const { return m_baseClass; }

    // get short class name (ie. engine::scene::Mesh -> "Mesh")
    INLINE StringID shortName() const { return m_shortName; }

    // get ALL properties (from this class and base classes)
    const TConstProperties& allProperties() const;

    // get ALL native functions (from this class and base classes)
    const TConstNativeFunctions& allNativeFunctions() const;

    // get ALL script functions (from this class and base classes)
    const TConstScriptFunctions& allScriptFunctions() const;

    // get all template properties for this class
    typedef Array<TemplateProperty> TConstTemplateProperties;
    const TConstTemplateProperties& allTemplateProperties() const;

    // find property with given name (recursive)
    const Property* findProperty(StringID propertyName) const;

    // find native function by name (recursive to base classes)
    const NativeFunction* findNativeFunction(StringID functionName) const;

    // find script function by name (recursive to base classes)
    const IScriptFunction* findScriptFunction(StringID functionName) const;

    // patch all pointers to resources, returns list of patched properties (top-level only)
    bool patchResourceReferences(void* data, IResource* currentResource, IResource* newResource, Array<StringID>* outPatchedProperties) const;

    //---------------

    // get assigned user index, used for indexed systems (app services, game systems)
    // by default returns -1 if no previous index was assigned
    INLINE int peekUserIndex() const { return m_userIndex; }

    // conditionally assign index to the class, returns existing index if class had index assigned before
    int assignUserIndex() const;

    // check class version (ClassVersionMetadata or 0 if no metadata specified)
    uint32_t version() const;

    //---------------

    // is this class derived from given base class
    bool is(ClassType otherClass) const;

    // is this class derived from given base class
    template< typename T >
    INLINE bool is() const
    {
        return is(T::GetStaticClass());
    }

    //---------------

    // clear all cached data
    void clearCachedData();

    // bind a base class
    // NOTE: works only on a non-const (buildable) RTTI class, do not call after RTII initialized for a given module
    void baseClass(ClassType baseClass);

    // add a property to this class
    // NOTE: works only on a non-const (buildable) RTTI class, do not call after RTII initialized for a given module
    void addProperty(Property* property);

    // add a function to this class
    // NOTE: works only on a non-const (buildable) RTTI class, do not call after RTII initialized for a given module
    void addNativeFunction(NativeFunction* function);

    // add a script function shim to this class
    // NOTE: works only on a non-const (buildable) RTTI class, do not call after RTII initialized for a given module
    void addScriptFunction(IScriptFunction* function);

    //---------------

    // reset all cached informations
    void resetCachedLists();

    //---------------

    // allocate memory (from proper pool) for object of this type
    void* allocateClassMemory(uint32_t size, uint32_t alignment) const;

    // free memory (to proper pool) used by object of this type
    void freeClassMemory(void* ptr) const;

    //---------------

    template< typename T >
    INLINE RefPtr<T> create() const
    {
        DEBUG_CHECK_EX(is<T>(), "Unrelated types");
        DEBUG_CHECK_EX(sizeof(T) <= size(), "Trying to allocate bigger type from smaller class");
        auto* mem = (T*)allocateClassMemory(size(), alignment());
        construct(mem);
        return AddRef(mem);
    }

    template< typename T >
    INLINE T* createPointer() const
    {
        DEBUG_CHECK_EX(is<T>(), "Unrelated types");
        DEBUG_CHECK_EX(sizeof(T) <= size(), "Trying to allocate bigger type from smaller class");
        auto* mem = (T*)allocateClassMemory(size(), alignment());
        construct(mem);
        return mem;
    }

    //--

protected:
    mutable std::atomic<int> m_userIndex;

    const IClassType* m_baseClass;

    StringID m_shortName;

    //--

    typedef Array< Property* > TNonConstProperties;
    TNonConstProperties m_localProperties;

    typedef Array< NativeFunction* > TNonConstNativeFunctions;
    TNonConstNativeFunctions m_localNativeFunctions;

    typedef Array< IScriptFunction* > TNonConstScriptFunctions;
    TNonConstScriptFunctions m_localScriptFunctions;

    //--

    mutable std::atomic<TConstProperties*> m_allProperties = nullptr;

    mutable std::atomic<TConstTemplateProperties*> m_allTemplateProperties = nullptr;

    mutable std::atomic<TConstNativeFunctions*> m_allNativeFunctions = nullptr;
    mutable std::atomic<TConstScriptFunctions*> m_allScriptFunctions = nullptr;

    SpinLock m_listBuildLock;

    //--

    SpinLock m_resourceRelatedPropertiesLock;
    bool m_resourceRelatedPropertiesUpdated = false;
    Array<Property*> m_resourceRelatedProperties;

    friend ClassBuilder;

    //--

    enum class PropertyIssuesResult : uint8_t
    {
        NoIssues,
        NotHandled,
        HandledByObject,
        HandledByTyped,
        HandledGeneric,
    };

    PropertyIssuesResult handlePropertyIssues(TypeSerializationContext& context, const SerializationPropertyInfo& info, void* data, SerializationReader& reader) const;

    virtual void cacheTypeData() override;
    virtual void releaseTypeReferences() override;
};

//--

// shared-ptr cast for RTTI SharedPointers
template< class _DestType, class _SrcType >
INLINE RefPtr< _DestType > rtti_cast(const RefPtr< _SrcType >& srcObj)
{
    if (srcObj && srcObj->cls()->is(_DestType::GetStaticClass()))
        return srcObj.template staticCast<_DestType>();

    return nullptr;
}

// normal-ptr cast for RTTI SharedPointers
template< class _DestType, class _SrcType >
INLINE _DestType* rtti_cast(_SrcType* srcObj)
{
    if (srcObj && srcObj->cls()->is(_DestType::GetStaticClass()))
        return static_cast<_DestType*>(srcObj);

    return nullptr;
}

// normal-ptr cast for RTTI SharedPointers
template< class _DestType, class _SrcType >
INLINE const _DestType* rtti_cast(const _SrcType* srcObj)
{
    if (srcObj && srcObj->cls()->is(_DestType::GetStaticClass()))
        return static_cast<const _DestType*>(srcObj);

    return nullptr;
}

template< typename BaseClass >
INLINE SpecificClassType<BaseClass>::SpecificClassType(const IClassType* type)
{
    DEBUG_CHECK_EX(!type || type->is<BaseClass>(), "Trying to initialize a specific class from incompatible class type");
    if (type->is<BaseClass>()) // to prevent weird bugs it's better not to assign
        m_classType = type;
}


template< typename T >
INLINE SpecificClassType<T> ClassType::cast() const
{
    if (m_classType && m_classType->is(T::GetStaticClass()))
        return SpecificClassType<T>(m_classType);
    else
        return SpecificClassType<T>(nullptr);
}

template< typename T >
INLINE RefPtr<T> ClassType::create() const
{
    if (m_classType && !m_classType->isAbstract() && m_classType->is(T::GetStaticClass()))
        return m_classType->create<T>();
    else
        return nullptr;
}

//--

END_INFERNO_NAMESPACE()
    
