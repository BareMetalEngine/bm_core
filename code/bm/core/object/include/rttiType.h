/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

#include "bm/core/containers/include/stringID.h"
#include "rttiMetadata.h"

BEGIN_INFERNO_NAMESPACE()

class Type;
class Property;
class IMetadata;
class IClassType;

/// type conversion "matrix" values
enum class TypeConversionClass : char
{
    TypeUnknown = 0,
    Typebool = 1,
    Typechar,
    Typeshort,
    Typeint,
    Typeint64_t,
    Typeuint8_t,
    Typeuint16_t,
    Typeuint32_t,
    Typeuint64_t,
    Typefloat,
    Typedouble,
    TypeStringBuf,
    TypeStringID,
    TypeStrongHandle,
    TypeWeakHandle,
    TypeEnum,
    TypeClassRef,
    TypeResRef,

    MAX,
};

////-----

/// General type serialization context
struct BM_CORE_OBJECT_API TypeSerializationContext : public MainPoolData<NoCopy>
{
    const IClassType* classContext = nullptr;
    const Property* propertyContext = nullptr; // property being deserialized/serialized
    IObject* parentObjectContext = nullptr; // parent object, can be used to handle property data conversion/missing properties
    IObject* directObjectContext = nullptr; // parent object, can be used to handle property data conversion/missing properties

    StringID classContextName() const;
    StringID propertyContextName() const;
    StringID propertyContextType() const;

    void print(IFormatStream& f) const; // print context name
};

struct TypeSerializationContextSetClass
{
    INLINE TypeSerializationContextSetClass(TypeSerializationContext& ctx, const IClassType* cls)
        : m_context(ctx)
        , m_prevClass(ctx.classContext)
    {
        ctx.classContext = cls;
    }

    INLINE ~TypeSerializationContextSetClass()
    {
        m_context.classContext = m_prevClass;
    }

private:
    TypeSerializationContext& m_context;
    const IClassType* m_prevClass = nullptr;
};

struct TypeSerializationContextSetProperty
{
    INLINE TypeSerializationContextSetProperty(TypeSerializationContext& ctx, const Property* prop)
        : m_context(ctx)
        , m_prevProperty(ctx.propertyContext)
    {
        ctx.propertyContext = prop;

        m_prevDirectObjectContext = ctx.directObjectContext;
        ctx.directObjectContext = nullptr;
    }

    INLINE ~TypeSerializationContextSetProperty()
    {
        m_context.propertyContext = m_prevProperty;
        m_context.directObjectContext = m_prevDirectObjectContext;
    }

private:
    TypeSerializationContext& m_context;
    const Property* m_prevProperty = nullptr;
    IObject* m_prevDirectObjectContext = nullptr;
};

////-----

/// flags for printToText/printFromText
enum PrintToFlags : uint32_t
{
    PrintToFlag_TextSerializaitonStructElement = FLAG(1),
    PrintToFlag_TextSerializaitonArrayElement = FLAG(2),
    PrintToFlag_Editor = FLAG(3),
};

////-----

/// Type traits
struct TypeRuntimeTraits
{
    TypeConversionClass convClass = TypeConversionClass::TypeUnknown; // runtime conversion class
    MetaType metaType = MetaType::Void; // general class of the type
    uint64_t nativeHash = 0; // typeid().hash() of the type
    uint32_t size = 0; // size of the type in memory
    uint32_t alignment = 1; // alignment required for the type

    bool scripted = false; // type was defined in scripts
    bool initializedFromZeroMem = false; // type does not require construct() if initialized from zero memory
    bool requiresConstructor = true; // call to construct() is not needed in all cases (no undefined state)
    bool requiresDestructor = true; // call to destroy() is not needed in all cases (no memory leaks)
    bool simpleCopyCompare = false; // use memcpy/memcmp instead of full copy() compare() calls
    bool hashable = false; // is this type hashable ? (can be used as Key in hashmap)

};

////-----

/// Base type in the RTTI system
class BM_CORE_OBJECT_API IType : public MetadataContainer
{
public:
    IType(StringID name = StringID::EMPTY());
    virtual ~IType();

    ///----

    /// DataInternface: Construct valid data in given memory
    /// Equivalent of constructor call
    /// NOTE: zero-filled memory is ALSO a good initial state, in this case the Construct() can be omitted
    virtual void construct(void *object) const = 0;

    /// DataInternface: Destroy data from given memory
    /// Equivalent of destructor call, usually not called if NeedsCleaning() returns false
    virtual void destruct(void *object) const = 0;

    /// DataInternface: Compare two values, returns true if values are equal
    virtual bool compare(const void* data1, const void* data2) const = 0;

    /// DataInternface: Copy value from one place in memory to other place
    /// Equivalent of assignment operator
    /// NOTE: the target memory MUST be properly initialized before we copy value into it
    virtual void copy(void* dest, const void* src) const = 0;

    //----
    // Binary serialization

    /// DataInternface: Write data to file
    virtual void writeBinary(TypeSerializationContext& typeContext, SerializationWriter& file, const void* data, const void* defaultData) const = 0;

    /// DataInternface: Read data from file
    virtual void readBinary(TypeSerializationContext& typeContext, SerializationReader& file, void* data) const = 0;

	//----
	// Text serialization

	// Save type data to XML
	virtual void writeText(TypeSerializationContext& typeContext, ITextSerializationWriter& file, const void* data, const void* defaultData) const = 0;

	// Load type data from XML
	virtual void readText(TypeSerializationContext& typeContext, ITextSerializationReader& file, void* data) const = 0;

    //----
    // Data view

    /// Get metadata for view - describe what we will find here: flags, list of members, size of array, etc
    virtual DataViewResult describeDataView(StringView viewPath, const void* viewData, DataViewInfo& outInfo) const;

    /// Read data from memory
    virtual DataViewResult readDataView(StringView viewPath, const void* viewData, void* targetData, Type targetType) const;

    /// Write data to memory
    virtual DataViewResult writeDataView(StringView viewPath, void* viewData, const void* sourceData, Type sourceType) const;

    //----
    // Type traits

    /// get type traits
    INLINE const TypeRuntimeTraits& traits() const { return m_traits; }

    /// get type traits
    INLINE TypeRuntimeTraits& traits() { return m_traits; }

    /// Get size in the memory, always well known
    INLINE uint32_t size() const { return m_traits.size; }

    /// Get required memory alignment in case the storage for typed data is allocated dynamically
    /// NOTE: this is basically _alignof + some hacks
    INLINE uint32_t alignment() const { return m_traits.alignment; }

    /// Get meta type (type of the type), this can be used to static_cast to derived classes
    INLINE MetaType metaType() const { return m_traits.metaType; }

    /// is this a scripted type ?
    INLINE bool scripted() const { return m_traits.scripted; }

    /// Get name of the RTTI type, internal, can be stored in files
    INLINE StringID name() const { return m_name; }

    //---
    // RTTI system native

    /// (shutdown only) release all internal references to other type
    virtual void releaseTypeReferences();

    /// (scripts only) cache type related data for faster performance
    virtual void cacheTypeData();

    //--

    // given a value or list of values parse them into a value
    // NOTE: for arrays all elements are treated as array elements, for simple types last value from array is taken
    // NOTE: this functions suports structures but does not support objects
    static bool ApplyFromString(Type type, void* data, const Array<StringBuf>& values);

    // capture current value into a string or a list of strings (for arrays)
    static void CaptureToString(Type type, const void* data, Array<StringBuf>& outValues);

	//----
	// To/From text

	/// Print data of this type into a string buffer
    /// NOTE: forks only for simple types (scalars, strings, etc), does not work for anything complicated like arrays
    static bool PrintToString(Type type, const void* data, IFormatStream& f);

    /// Parse data from string
	/// NOTE: forks only for simple types (scalars, strings, etc), does not work for anything complicated like arrays
    static bool ParseFromString(Type type, void* data, StringView txt);

    //--

protected:
    TypeRuntimeTraits m_traits;
    StringID m_name;
};

// type naming, slight hack to put it here but this allows way less includes cross-visibility
// to be honest, those functions are needed ONLY in the reflection part because type knows how to format its name, but to move it there would require to have a copy paste somewhere..
extern BM_CORE_OBJECT_API StringID FormatStrongHandleTypeName(StringID className);
extern BM_CORE_OBJECT_API StringID FormatWeakHandleTypeName(StringID className);
extern BM_CORE_OBJECT_API StringID FormatDynamicArrayTypeName(StringID innerTypeName);
extern BM_CORE_OBJECT_API StringID FormatNativeArrayTypeName(StringID innerTypeName, uint32_t maxSize);
extern BM_CORE_OBJECT_API StringID FormatClassRefTypeName(StringID className);

// helpers
extern BM_CORE_OBJECT_API const char* GetEnumValueName(const EnumType* enumType, int64_t enumValue);
extern BM_CORE_OBJECT_API bool GetEnumNameValue(const EnumType* enumType, StringID name, int64_t& outEnumValue);

END_INFERNO_NAMESPACE()
