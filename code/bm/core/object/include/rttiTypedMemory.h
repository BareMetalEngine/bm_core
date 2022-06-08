/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

#include "rttiTypeRef.h"
#include "rttiDataHolder.h"

BEGIN_INFERNO_NAMESPACE()

///---

/// Result status
enum class TypedMemoryResultStatus : uint8_t
{
    OK,
    ErrorReadOnly,
    ErrorIndexOutOfRange,
	ErrorMissingChildren,
    ErrorInvalidType,
    ErrorInvalidOperation,
    ErrorNotFound,
};

struct TypedMemoryResult;

//--

/// Pointer to a memory with type information
class BM_CORE_OBJECT_API TypedMemory
{
public:
    INLINE TypedMemory() {};
    INLINE TypedMemory(std::nullptr_t) {};
    INLINE TypedMemory(const TypedMemory& other) = default;
    INLINE TypedMemory(TypedMemory&& other) = default;
	INLINE TypedMemory& operator=(const TypedMemory& other) = default;
	INLINE TypedMemory& operator=(TypedMemory && other) = default;
	INLINE ~TypedMemory() {};

    INLINE TypedMemory(Type type, const void* ptr, bool readOnly = false)
        : m_type(type)
        , m_data((void*)ptr)
        , m_readOnly(readOnly)
    {};

    template< typename T >
    INLINE static TypedMemory Wrap(T& data)
    {
		return TypedMemory(GetTypeObject<T>(), &data, false);
    }

	template< typename T >
    INLINE static TypedMemory Wrap(const T& data)
    {
        return TypedMemory(GetTypeObject<T>(), &data, true);
    }

    // compare values via the type method
    bool operator==(const TypedMemory& other) const;
    bool operator!=(const TypedMemory& other) const;

    ///---

    // no data?
    INLINE bool empty() const { return !m_data; }

    // is the data read only ?
    INLINE bool readOnly() const { return m_readOnly; }

    // check if valid (has data)
    INLINE operator bool() const { return m_data; }

    // get assigned type
    INLINE Type rawType() const { return m_type; }

    // get raw pointer to data
    INLINE const void* rawData() const { return m_data; }

    // get raw pointer to data (writable)
    // NOTE: writes are NOT observed
    INLINE void* rawData() { return m_data; }

	///---

    // get unwrapped type (Variant and other containers are removed)
	Type unwrappedType() const;

    // get unwrapped data (Variant and other containers are removed)
	void* unwrappedData() const;

	// is this a simple value ?
	bool isSimple() const;

    ///---

    // is wrapper value a variant ?
    bool isVariant() const;

    // change variant type
    TypedMemoryResult variantChangeType(Type otherType);

    ///--

    // is this value a pointer to some object ?    
    bool isPointer() const;

    // is this an object pointer ?
    bool isObject() const;

    // get class of the pointed object
    ClassType pointedClass() const;

    // get internal pointer, returns it only if the pointer's type is compatible with expectedType
    void* pointedObjectPtrRaw(ClassType expectedType) const;    

    // cast to pointer for given class, returns nullptr if type does not match
	template<typename T>
	INLINE T* pointedObjectPtr() const
	{
        return static_cast<const T*>(pointedObjectPtrRaw(T::GetStaticClass()));
	}

    // cast to refcounted pointer for given class, returns nullptr if type does not match
    template<typename T>
    INLINE RefPtr<T> pointedObjectRefPtr() const
    {
        static_assert(std::is_base_of<IReferencable, T>::value, "Only IReferencable types are supported here");
        return RefPtr<T>(AddRef<T>(sttic_cast<const T*>(pointedObjectPtrRaw(T::GetStaticClass()))));
    }

    //--

    // are we an array ?
    bool isArray() const;

    // get current size of the array
    uint32_t arraySize() const;

    // get array's inner type
    Type arrayInnerType() const;

    // remote all elements from array
    TypedMemoryResult arrayClear();

    // remote n-th array element
    TypedMemoryResult arrayRemove(uint32_t index);

    // resize array
    TypedMemoryResult arrayResize(uint32_t size);

    // create element in the array at given index
    TypedMemoryResult arrayInsert(uint32_t index);

    // create element in the array at the end
    TypedMemoryResult arrayPushBack();

	// get typed memory of n-th element
    // NOTE: for invalid index we return empty TypedMemory()
    TypedMemoryResult arrayElement(uint32_t index) const;

    // get typed memory of n-th element
    // NOTE: for invalid index we return empty TypedMemory()
    INLINE TypedMemory operator[](uint32_t index) const;

	// iterate over all existing array elements
    TypedMemoryResult arrayIterate(const std::function<bool(uint32_t, TypedMemory)>& func) const;

    //--

    // has type some named keys, can be a struct, object, variant table, etc?
	bool isDictionary() const; 

    // can the dictionary be modified (VariantTable only)
    bool isDynamicDictionary() const;

    // check if dictionary has elements
    bool dictionaryHasElements() const;

	// get type of the dictionary
	Type dictionaryType() const;

    // get value for dictionary element
    TypedMemoryResult dictionaryElement(StringID name) const;
    TypedMemoryResult dictionaryElement(StringView name) const;

	// get value for dictionary element
    INLINE TypedMemory operator[](StringID name) const;
    INLINE TypedMemory operator[](StringView name) const;

    // remove dictionary element, works only for VariantTable
    TypedMemoryResult dictionaryRemove(StringID name);
    TypedMemoryResult dictionaryRemove(StringView name);

    // insert value into dictionary
    TypedMemoryResult dictionaryInsert(StringID name, Type type);
    TypedMemoryResult dictionaryInsert(StringView name, Type type);

    // iterate over all existing dictionary elements
    TypedMemoryResult dictionaryIterate(const std::function<bool(StringID, TypedMemory)>& func) const;

    //--

    // print values in a JSON format
	void print(IFormatStream& f, DataTextFormatterFlags flags = DataTextFormatterFlagBit::Relaxed) const;

	// print values in a JSON format
	bool format(IFormatStream& f, IDataTextFormatter& ctx) const;

    //--

    // get (with type conversion) current value
	bool get(Type destType, void* destData) const;

    // get (with type conversion) current value
    template< typename T >
    INLINE bool get(T& outData) const
    {
        return get(GetTypeObject<T>(), &outData);
    }

    // get (with type conversion) current value, use default value if something fails
	template< typename T >
	INLINE T getSafe(T ret = T()) const
	{
        get(GetTypeObject<T>(), &ret);
        return ret;
	}

    //--
    
    // set (with type conversion) new value
    // NOTE: value is COPIED
    bool set(Type srcType, const void* srcData) const;

	// get (with type conversion) current value
	template< typename T >
	INLINE bool set(const T& outData) const
	{
		return set(GetTypeObject<T>(), &outData);
	}

	//--

    // convert to JSON-like string
	StringBuf toString(DataTextFormatterFlags flags = DataTextFormatterFlagBit::Relaxed) const;

	// convert to JSON-like string
	StringBuf toString(IDataTextFormatter& ctx) const;

    // convert to persistent data holder
	DataHolder toDataHolder() const;  

    //--

	/// write data to given format
    /// NOTE: to load objects pass either ObjectPtr or Array<ObjectPtr> as the type
	static bool SaveData(SerializationFormat format, const ObjectSavingContext& ctx, TypedMemory data, IFormatStream& f);

	/// load data to text format
	/// NOTE: to load objects pass either ObjectPtr or Array<ObjectPtr> as the type
	static bool LoadData(SerializationFormat format, const ObjectLoadingContext& ctx, TypedMemory data, StringView txt, ITextErrorReporter& err = ITextErrorReporter::GetDefault());

    //--

private:
    Type m_type;
    void* m_data = nullptr;
    bool m_readOnly = false;

    static TypedMemoryResult DictionaryElementClass(ClassType type, void* data, bool readOnly, StringID name);
    static TypedMemoryResult DictionaryIterateClass(ClassType type, void* data, bool readOnly, const std::function<bool(StringID, TypedMemory)>& func);
    static TypedMemoryResult DictionaryIterateTable(VariantTable* table, bool readOnly, const std::function<bool(StringID, TypedMemory)>& func);
};

///---

/// result of on writable operation on the typed memory
struct BM_CORE_OBJECT_API TypedMemoryResult
{
	TypedMemoryResultStatus status = TypedMemoryResultStatus::ErrorInvalidOperation;
	TypedMemory ptr;

	INLINE TypedMemoryResult() = default;
	INLINE TypedMemoryResult(const TypedMemoryResult& other) = default;
	INLINE TypedMemoryResult& operator=(const TypedMemoryResult& other) = default;
	INLINE TypedMemoryResult(TypedMemoryResultStatus status_) : status(status_) {};
	INLINE TypedMemoryResult(TypedMemory ptr_) : status(TypedMemoryResultStatus::OK), ptr(ptr_) {};

	INLINE operator bool() const { return (status == TypedMemoryResultStatus::OK); }
	INLINE operator TypedMemory() const { return ptr; }

	void print(IFormatStream& f) const;
};

INLINE TypedMemory TypedMemory::operator[](uint32_t index) const
{
	return arrayElement(index).ptr;
}

INLINE TypedMemory TypedMemory::operator[](StringID name) const
{
    return dictionaryElement(name).ptr;
}

INLINE TypedMemory TypedMemory::operator[](StringView name) const
{
    return dictionaryElement(name).ptr;
}

INLINE TypedMemory DataHolder::view() const
{ 
    return TypedMemory(m_type, m_data);
}

//---

END_INFERNO_NAMESPACE()
