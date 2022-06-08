/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

BEGIN_INFERNO_NAMESPACE()

/// a any-type value holder similar to DataHolder but supports reflection and has a special case for small data
/// NOTE: variant itself can be serialized (thus allowing for complex map<name, variant> structures like object templates) but be sure not to have variant with variant...
TYPE_ALIGN(8, class) BM_CORE_OBJECT_API Variant
{
public:
    static const uint32_t INTERNAL_STORAGE_SIZE = 16;

    //--

    INLINE Variant() {}
    INLINE Variant(std::nullptr_t) {}
    INLINE Variant(Type type, const void* data = nullptr) { reset(type, data); }
    Variant(const Variant& other);
    Variant(Variant&& other);
    ~Variant();

    template< typename T >
    INLINE Variant(const T& data)
    {
        static_assert(!std::is_same<T, Variant>::value, "Cannot initialize variant from variant like that");
        reset(data);
    }

    // copy/move
    Variant& operator=(const Variant& other);
    Variant& operator=(Variant&& other);

    // simple comparisons
    bool operator==(const Variant& other) const;
    bool operator!=(const Variant& other) const;

    //--

    // is variant empty ? (no data)
    INLINE bool empty() const { return m_type == nullptr; }

    // is variant valid?
    INLINE operator bool() const { return m_type; }

    // get assigned type
    INLINE Type type() const { return m_type; }

	// get the memory where variant data is stored
    // NOTE: changes are not monitored
	INLINE void* data() const { return m_data; }

    // check if we are of given type
    template< typename T >
    INLINE bool is() const { return m_type == GetTypeObject<T>(); }

    //--

    // reset to empty state
    void reset();

	// reset to different type
	bool reset(Type type, const void* data = nullptr);

	// reset to different type
    template< typename T >
    INLINE void reset(const T& value)
    {
        static_assert(!std::is_same<T, Variant>::value, "Cannot initialize variant from variant like that");
        reset(GetTypeObject<T>(), &value);
    }

    //--

    // set new value form given data, does type conversion
    bool set(Type srcType, const void* srcData);

    // set new value form given data, does type conversion
    template< typename T >
    INLINE bool set(const T& data)
    {
        return set(GetTypeObject<T>(), &data);
    }

    //--

    // write value into a remote buffer, small type conversion may occur
    bool get(Type destType, void* destData) const;

	// set new value form given data, does type conversion
	template< typename T >
	INLINE bool get(T& outData) const
	{
		return get(GetTypeObject<T>(), &outData);
	}

	// set new value form given data, does type conversion
	template< typename T >
	INLINE T getSafe(T ret) const
	{
		get(GetTypeObject<T>(), &ret);
        return ret;
	}

	//--

	// write text representation of the value (no debug info, pure value)
	void print(IFormatStream& f) const;

    //--

    // try to parse value from text into this variant (will override current data)
    bool fromString(StringView txt);

	// write text representation of the value (no debug info, pure value)
	StringBuf toString() const;

    //--

    // get empty variant
    static const Variant& EMPTY();

    //--

    // get a typed "view" of the variant's data
    // NOTE: this view will NOT see that it's a variant so type change will not be possible
    TypedMemory view() const;

    //--

private:
    Type m_type = nullptr;
    void* m_data = nullptr; // owned

    static void* AllocateVariantMemory(Type type);
    void FreeVariantMemory(void* data);
};

//--
    
END_INFERNO_NAMESPACE()
