/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

#include "rttiVariant.h"
#include "reflectionMacros.h"
#include "bm/core/containers/include/hashMap.h"

BEGIN_INFERNO_NAMESPACE()

//---

/// proxy for accessing elements of variant table
class TypedMemory;
class VariantTableElementProxy;

/// Optimized key/value parameter table
/// Allows to store set of named values without limiting ourselves to one type
/// use the Parameter table for the extended version (with template operators)
/// NOTE: the parameter table is a custom type in RTTI this allows the table to store another table
/// NOTE: the table supports deep copy but it's very slow, it's better to hold on to it by pointer
class BM_CORE_OBJECT_API VariantTable
{
public:
    VariantTable();
    VariantTable(const VariantTable& other);
    VariantTable(VariantTable&& other);
    VariantTable& operator=(const VariantTable& other);
    VariantTable& operator=(VariantTable&& other);
    ~VariantTable();

    //--

    // empty ?
    INLINE bool empty() const { return m_numEntries == 0; };

    //--

    // remove all values from the table, frees the memory
    void clear();

    // check if table contains value
    bool contains(StringView name) const;
    bool contains(StringID name) const;

    // remove entry from table
    // NOTE: space is not recovered
    bool remove(StringID name);
    bool remove(StringView name);

    // iterate values in the table
    TypedMemory iterate(const std::function<bool(StringID, TypedMemory)>& func, bool readOnly = false) const;

    //--

    // get access to named element
    const VariantTableElementProxy operator[](StringView name) const;
    const VariantTableElementProxy operator[](StringID name) const;

    VariantTableElementProxy operator[](StringView name);
    VariantTableElementProxy operator[](StringID name);

    //--

    // view of whole table
    TypedMemory view(bool readOnly = false) const;

    //--

    // print content
    void print(IFormatStream& f) const;

    // custom formatted print
    void format(IFormatStream& f, IDataTextFormatter& ctx) const;

    //--

private:
    struct Page
    {
        Page* next = nullptr;
        uint32_t dataSize = 0;
        uint32_t dataCapacity = 0;
    };

    struct Entry
    {
        StringID name;
        Type type;
        Entry* next = nullptr;
        void* data = nullptr;

		INLINE TypedMemory view(bool readOnly) const
		{
            return TypedMemory(type, data, readOnly);
		}
    };

    uint32_t m_numBuckets = 0;
    uint32_t m_numEntries = 0;
    uint32_t m_maxEnties = 0;

    Entry** m_buckets = nullptr;
    Entry* m_entries = nullptr;
    Page* m_pages = nullptr;

    //--

    TypedMemory internalIterateEntries(const std::function<bool(const Entry&)>& func, bool readOnly = false) const;
    void internalInsertElement(Entry* entry);
    TypedMemory internalAppendEntry(StringID name, Type type, const void* data);
    TypedMemory internalAppendEntry(StringID name, TypedMemory value);

	Entry* internalElementFind(StringID name) const;
    Entry* internalElementAllocate(StringID name, Type type);
    TypedMemory internalElementView(StringID name, bool readOnly) const;

    void* internalAllocateMemory(uint32_t size, uint32_t alignment);

    bool internalRebuildBuckets(uint32_t newCount);

	static uint32_t BucketsPerElementCount(uint32_t count);
    static uint32_t CalcTotalInternalSize(uint32_t bucketCount, uint32_t dataSize);

    static void* AllocateVariantMemory(uint32_t size);
    static void FreeVariantMemory(void* data);

    friend class VariantTableElementProxy;
    friend class TypedMemory;
};

//---

// proxy for accessing elements of variant table
class VariantTableElementProxy : public MainPoolData<NoCopy>
{
public:
    INLINE VariantTableElementProxy(std::nullptr_t) {};
    INLINE VariantTableElementProxy(VariantTable* table, StringID name, StringView nameTxt, TypedMemory view) 
        : m_table(table), m_name(name), m_nameTxt(nameTxt), m_view(view)
    {}

    INLINE VariantTableElementProxy(VariantTableElementProxy&& other) = default;
    INLINE VariantTableElementProxy& operator=(VariantTableElementProxy&& other) = default;

    template< typename T >
    INLINE operator T() const
    {
        return m_view.getSafe();
    }

	template< typename T >
	INLINE T getSafe(T defaultValue = t()) const
	{
		return m_view.getSafe(defaultValue);
	}

	template< typename T >
	INLINE bool get(T& outValue) const
	{
		return m_view.get(outValue);
	}

	template< typename T >
	INLINE bool set(const T& value)
	{
        if (m_view)
		    return m_view.set(value);

        if (!m_name)
        {
            ASSERT(m_nameTxt);
            m_name = StringID(m_nameTxt);
        }

        if (!m_table)
            return false;

        auto view = m_table->internalAppendEntry(m_name, GetTypeObject<T>(), &value);
        if (!view)
            return false;

        m_view = view;
        return true;
	}

    INLINE bool copy(const Variant& v)
    {
        if (!v)
            return false;

        if (m_view)
            return m_view.set(v.type(), v.data());

		if (!m_name)
		{
			ASSERT(m_nameTxt);
			m_name = StringID(m_nameTxt);
		}

		if (!m_table)
			return false;

        auto view = m_table->internalAppendEntry(m_name, v.type(), v.data());
		if (!view)
			return false;

		m_view = view;
		return true;
    }

	template< typename T >
	INLINE VariantTableElementProxy& operator=(const T& value)
	{
		set(value);
        return *this;
	}

    INLINE TypedMemory view() const
    {
        return m_view;
    }

	INLINE StringView name() const
	{
		return m_nameTxt;
	}

private:
    VariantTable* m_table = nullptr;
    TypedMemory m_view;
    StringView m_nameTxt;
    StringID m_name;
};

//--

END_INFERNO_NAMESPACE()

