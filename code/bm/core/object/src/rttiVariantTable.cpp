/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "rttiVariantTable.h"
#include "rttiTextFormater.h"

BEGIN_INFERNO_NAMESPACE()

//---

RTTI_BEGIN_CUSTOM_TYPE(VariantTable);
	RTTI_BIND_NATIVE_CTOR_DTOR(VariantTable);
	RTTI_BIND_NATIVE_COPY(VariantTable);
	//RTTI_BIND_CUSTOM_BINARY_SERIALIZATION();
	//RTTI_BIND_CUSTOM_XML_SERIALIZATION();
RTTI_END_TYPE();

//---

VariantTable::VariantTable()
{}

VariantTable::VariantTable(const VariantTable& other)
{
	other.iterate([this](StringID name, TypedMemory value)
		{
			internalAppendEntry(name, value);
			return false;
		});
}

VariantTable::VariantTable(VariantTable&& other)
	: m_numBuckets(other.m_numBuckets)
	, m_numEntries(other.m_numEntries)
	, m_buckets(other.m_buckets)
	, m_entries(other.m_entries)
	, m_pages(other.m_pages)
{
	other.m_numBuckets = 0;
	other.m_numEntries = 0;
	other.m_buckets = nullptr;
	other.m_entries = nullptr;
	other.m_pages = nullptr;
}

VariantTable& VariantTable::operator=(const VariantTable& other)
{
	if (this != &other)
	{
		clear();

		other.iterate([this](StringID name, TypedMemory value)
			{
				internalAppendEntry(name, value);
				return false;
			});
	}

	return *this;
}

VariantTable& VariantTable::operator=(VariantTable&& other)
{
	if (this != &other)
	{
		m_numBuckets = other.m_numBuckets;
		m_numEntries = other.m_numEntries;
		m_buckets = other.m_buckets;
		m_entries = other.m_entries;
		m_pages = other.m_pages;

		other.m_numBuckets = 0;
		other.m_numEntries = 0;
		other.m_buckets = nullptr;
		other.m_entries = nullptr;
		other.m_pages = nullptr;
	}

	return *this;
}

VariantTable::~VariantTable()
{
	clear();
}

//--

void VariantTable::clear()
{
	internalIterateEntries([this](const Entry& entry)
		{
			entry.type->destruct(entry.data);
			return false;
		});

	auto* page = m_pages;
	while (page)
	{
		auto* next = page->next;
		FreeVariantMemory(page);
		page = next;
	}
}

//--

bool VariantTable::contains(StringView txt) const
{
	if (const auto name = StringID::Find(txt))
		if (const auto view = internalElementView(name, true))
			return true;

	return false;
}

bool VariantTable::contains(StringID name) const
{
	if (name)
		if (const auto view = internalElementView(name, true))
			return true;

	return false;
}

bool VariantTable::remove(StringID name)
{
	return false;
}

bool VariantTable::remove(StringView name)
{
	return false;
}

TypedMemory VariantTable::iterate(const std::function<bool(StringID, TypedMemory)>& func, bool readOnly) const
{
	return internalIterateEntries([&func, readOnly](const Entry& e)
		{
			return func(e.name, e.view(readOnly));
		});
}

//--

const VariantTableElementProxy VariantTable::operator[](StringView txt) const
{
	if (const auto name = StringID::Find(txt))
		if (const auto view = internalElementView(name, true))
			return VariantTableElementProxy((VariantTable*)this, name, txt, view);
	
	return nullptr;
}

const VariantTableElementProxy VariantTable::operator[](StringID name) const
{
	if (name)
		if (const auto view = internalElementView(name, true))
			return VariantTableElementProxy((VariantTable*)this, name, name.view(), view);

	return nullptr;
}

VariantTableElementProxy VariantTable::operator[](StringView txt)
{
	if (txt)
	{
		const auto name = StringID::Find(txt);
		const auto view = name ? internalElementView(name, true) : nullptr;
		return VariantTableElementProxy(this, name, txt, view);
	}

	return nullptr;
}

VariantTableElementProxy VariantTable::operator[](StringID name)
{
	if (name)
	{
		const auto view = name ? internalElementView(name, true) : nullptr;
		return VariantTableElementProxy(this, name, name.view(), view);
	}

	return nullptr;
}

//--

TypedMemory VariantTable::view(bool readOnly /*= false*/) const
{
	return TypedMemory(GetTypeObject<VariantTable>(), this);
}

//---

void VariantTable::print(IFormatStream& f) const
{
	DataTextFormatter_JSON ctx;
	format(f, ctx);
}

void VariantTable::format(IFormatStream& f, IDataTextFormatter& ctx) const
{
	ctx.beingArray(f);

	internalIterateEntries([&f, &ctx, this](const Entry& entry)
		{
			ctx.beingArrayElement(f);
			entry.view(true).format(f, ctx);
			ctx.endArrayElement(f);
			return false;
		});

	ctx.endArray(f);
}

//---

VariantTable::Entry* VariantTable::internalElementFind(StringID name) const
{
	DEBUG_CHECK_RETURN_EX_V(name, "Invalid name", nullptr);

	if (m_buckets)
	{
		const auto bucketIndex = name.index() % m_numBuckets;

		auto* cur = m_buckets[bucketIndex];
		while (cur)
		{
			if (cur->name == name)
				return cur;
			cur = cur->next;
		}
	}

	return nullptr;
}

TypedMemory VariantTable::internalElementView(StringID name, bool readOnly) const
{
	DEBUG_CHECK_RETURN_EX_V(name, "Invalid name", nullptr);

	if (const auto* elem = internalElementFind(name))
		return elem->view(readOnly);

	return nullptr;	
}

TypedMemory VariantTable::internalIterateEntries(const std::function<bool(const Entry&)>& func, bool readOnly) const
{
	for (auto i : IndexRange(m_numEntries))
	{
		const auto& elem = m_entries[i];
		if (func(elem))
			return elem.view(readOnly);
	}

	return nullptr;;
}

ALWAYS_INLINE uint32_t BucketNextPow2(uint32_t v)
{
	auto x = v;
	--x;
	x |= x >> 1;
	x |= x >> 2;
	x |= x >> 4;
	x |= x >> 8;
	x |= x >> 16;
	return ++x;
}

uint32_t VariantTable::BucketsPerElementCount(uint32_t count)
{
	return BucketNextPow2(std::max<uint32_t>(count / 4, 8));
}

/*bool VariantTable::internalAllocateNewTable(uint32_t size)
{
	auto bucketCount = 8;

	auto dataSize = 0;
	dataSize += sizeof(Header);
	dataSize += sizeof(Entry);
	dataSize += sizeof(int) * bucketCount;
	dataSize += size;

	dataSize = Align<uint32_t>(dataSize, 64);
	dataSize = std::max<uint32_t>(dataSize, 256);
	dataSize = BucketNextPow2(dataSize);

	auto* data = Memory::AllocateBlock(dataSize, 16, "VariantTable");
	DEBUG_CHECK_RETURN_EX_V(data, "Out of memory in VariantTable", false);

	const auto bucketTableSize = sizeof(int) * bucketCount;

	memset(data, 0xCC, dataSize - bucketTableSize);

	m_data = (Header*)data;
	m_data->numBuckets = bucketCount;
	m_data->numEntries = 0;
	m_data->dataSize = sizeof(Header);
	m_data->dataCapacity = dataSize - bucketTableSize;
	m_data->firstOffset = 0;
	m_data->lastOffset = 0;

	auto* buckets = m_data->bucketTable();
	memset(buckets, 0, bucketTableSize);

	return true;
}*/

void VariantTable::internalInsertElement(Entry* entry)
{
	ASSERT(m_numBuckets != 0);
	ASSERT(m_buckets != nullptr);
	ASSERT(entry != nullptr);
	ASSERT(entry->next == nullptr);
	ASSERT(entry->name);

	const auto bucketIndex = entry->name.index() % m_numBuckets;

	entry->next = m_buckets[bucketIndex];
	m_buckets[bucketIndex] = entry;
}

bool VariantTable::internalRebuildBuckets(uint32_t newCount)
{
	ASSERT(newCount > 0);

	const auto newBucketDataSize = sizeof(Entry*) * newCount;
	void* mem = AllocateVariantMemory(newBucketDataSize);
	DEBUG_CHECK_RETURN_EX_V(mem, "OOM in VariantTable", false);

	if (m_buckets)
		FreeVariantMemory(m_buckets);

	m_buckets = (Entry**)mem;
	m_numBuckets = newCount;
	memzero(mem, newBucketDataSize);

	for (auto i : IndexRange(m_numEntries))
	{
		auto* elem = m_entries + i;
		elem->next = nullptr;
		internalInsertElement(elem);
	}

	return true;	
}

void* VariantTable::internalAllocateMemory(uint32_t size, uint32_t alignment)
{
	// try head page only
	if (auto* page = m_pages)
	{
		auto* curPtr = (char*)page + page->dataSize;
		auto* endPtr = (char*)page + page->dataCapacity;

		// try to allocate from current page
		auto* ptr = AlignPtr(curPtr, alignment);
		if (ptr + size <= endPtr)
		{
			page->dataSize = (ptr + size) - (char*)page;
			return ptr;
		}
	}

	// allocate new page
	uint32_t pageSize = 256;
	if (m_pages) // if we already have a page make the new page twice as big but within reason
		pageSize = std::min<uint32_t>(m_pages->dataCapacity * 2, 1U << 20);

	// if the data won't fit keep resizing the page
	if (Align<uint32_t>(sizeof(Page), alignment) + size > pageSize)
		pageSize *= 2;

	// allocate new page
	auto* page = (Page*)AllocateVariantMemory(pageSize);
	DEBUG_CHECK_RETURN_EX_V(page, "OOM in VariantTable", nullptr);

	// setup page
	page->dataSize = sizeof(Page);
	page->dataCapacity = pageSize;
	page->next = m_pages;
	m_pages = nullptr;

	// allocate entry
	auto* curPtr = (char*)page + page->dataSize;
	auto* endPtr = (char*)page + page->dataCapacity;

	// try to allocate from current page
	auto* ptr = AlignPtr(curPtr, alignment);
	ASSERT(ptr + size <= endPtr);
	page->dataSize = (ptr + size) - (char*)page;
	return ptr;
}

VariantTable::Entry* VariantTable::internalElementAllocate(StringID name, Type type)
{
	ASSERT(name);
	ASSERT(type);

	// allocate element entry
	if (m_numEntries == m_maxEnties)
	{
		// grow element table
		auto newElementCount = std::max<uint32_t>(8, m_maxEnties * 2);
		auto* newElements = (Entry*)AllocateVariantMemory(sizeof(Entry) * newElementCount);
		DEBUG_CHECK_RETURN_EX_V(newElementCount, "OOM in VariantTable", nullptr);

		// copy
		memcpy(newElements, m_entries, sizeof(Entry) * m_numEntries);
		memzero(newElements + m_numEntries, sizeof(Entry) * (newElementCount - m_numEntries));
		m_maxEnties = newElementCount;
		m_entries = newElements;
	}

	// allocate element data
	auto* data = internalAllocateMemory(type->size(), type->alignment());
	VALIDATION_RETURN_V(data, nullptr);

	// construct memory
	type->construct(data);

	// add entry
	auto* entry = m_entries + m_numEntries;
	entry->data = data;
	entry->name = name;
	entry->type = type;
	entry->next = nullptr;
	m_numEntries += 1;

	// link
	uint32_t bucketCount = BucketsPerElementCount(m_numEntries);
	if (bucketCount > m_numBuckets)
		internalRebuildBuckets(bucketCount);
	else
		internalInsertElement(entry);
	return entry;
}

TypedMemory VariantTable::internalAppendEntry(StringID name, TypedMemory value)
{
	DEBUG_CHECK_RETURN_EX_V(name, "Invalid key", nullptr);
	DEBUG_CHECK_RETURN_EX_V(value, "Invalid value", nullptr);
	return internalAppendEntry(name, value.unwrappedType(), value.unwrappedData());
}

TypedMemory VariantTable::internalAppendEntry(StringID name, Type type, const void* data)
{
	DEBUG_CHECK_RETURN_EX_V(name, "Invalid key", nullptr);
	DEBUG_CHECK_RETURN_EX_V(type, "Invalid type", nullptr);
	DEBUG_CHECK_RETURN_EX_V(data, "Invalid data", nullptr);

	auto* entry = internalElementAllocate(name, type);
	DEBUG_CHECK_RETURN_EX_V(entry, "Out of memory", nullptr);

	type->copy(entry->data, data);

	return entry->view(false);
}

//--

void* VariantTable::AllocateVariantMemory(uint32_t size)
{
	return Memory::AllocateBlock(size, 8, "VariantTable");
}

void VariantTable::FreeVariantMemory(void* data)
{
	Memory::FreeBlock(data);
}

//--

END_INFERNO_NAMESPACE()
