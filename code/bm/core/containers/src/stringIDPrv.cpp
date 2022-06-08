/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "stringView.h"
#include "stringID.h"
#include "stringIDPrv.h"
#include "bm/core/memory/include/localAllocator.h"

BEGIN_INFERNO_NAMESPACE()

//#define DEBUG_STRINGID

namespace prv
{
	///--

	StringIDDataStorage::StringIDDataStorage()
	{
		allocPage();

		memset(m_writePtr, 0, sizeof(StringIDDataStorage));
		m_writePtr += sizeof(StringIDDataStorage);		
	}

	void StringIDDataStorage::allocPage()
	{
		m_writeStart = (uint8_t*)Memory::AllocateBlock(StringID::STRING_TABLE_PAGE_SIZE, 1, "StringIDTables");
		m_writePtr = m_writeStart;
		m_writeEndPtr = m_writePtr + StringID::STRING_TABLE_PAGE_SIZE;

		StringID::st_StringTable[m_numPages] = (const char*)m_writeStart;
		m_numPages += 1;

#ifdef DEBUG_STRINGID
		TRACE_INFO("[StringID] Allocated page, {} total", m_numPages);
#endif
	}

	StringIDDataStorage& StringIDDataStorage::GetInstance()
	{
		static StringIDDataStorage* theInstance = new StringIDDataStorage();
		return *theInstance;
	}

	StringIDDataEntry* StringIDDataStorage::entry(StringIDIndex index) const
	{
		const auto pageIndex = index / StringID::STRING_TABLE_PAGE_SIZE;
		const auto pageOffset = index % StringID::STRING_TABLE_PAGE_SIZE;
		return (StringIDDataEntry*)(StringID::st_StringTable[pageIndex] + pageOffset - sizeof(uint32_t));
	}

	const char* StringIDDataStorage::text(StringIDIndex index) const
	{
		const auto* e = entry(index);		
		return &e->txt[0];
	}

	StringIDIndex StringIDDataStorage::place(StringView buf)
	{
		auto lock = CreateLock(m_writeLock);

		const auto writeSize = sizeof(StringIDDataEntry) + buf.length();

		if (m_writePtr + writeSize > m_writeEndPtr)
			allocPage();

		uint32_t baseOffset = m_writePtr - m_writeStart;
		baseOffset += (m_numPages - 1) * StringID::STRING_TABLE_PAGE_SIZE;

		auto* entry = (StringIDDataEntry*)m_writePtr;
		entry->next = 0;

		memcpy(&entry->txt, buf.data(), buf.length());
		entry->txt[buf.length()] = 0;

#ifdef DEBUG_STRINGID
		TRACE_INFO("[StringID] Placed string '{}' at {}", buf, baseOffset);
#endif

		m_writePtr += writeSize;

		return baseOffset + sizeof(uint32_t);
	}

	///--

	StringIDMap::StringIDMap()
		: m_storage(StringIDDataStorage::GetInstance())
	{
		memset(m_buckets, 0, sizeof(m_buckets));
	}

	StringIDMap::~StringIDMap()
	{
	}

	void StringIDMap::insert(uint32_t stringHash, StringIDIndex stringIndex)
	{
		const auto bucketIndex = stringHash % NUM_BUCKETS;

		auto* entry = m_storage.entry(stringIndex);
		entry->next = m_buckets[bucketIndex];
		m_buckets[bucketIndex] = stringIndex;
	}
			   
	StringIDIndex StringIDMap::find(uint32_t stringHash, const char* stringData, uint32_t stringLength) const
	{
		const auto bucketIndex = stringHash % NUM_BUCKETS;
		auto entryIndex = m_buckets[bucketIndex];

		while (entryIndex)
		{
			const auto* entry = m_storage.entry(entryIndex);

			if (0 == strncmp(stringData, entry->txt, stringLength))
				return entryIndex;

			entryIndex = entry->next;
		}

		return 0;
	}

	///--

} // prv

END_INFERNO_NAMESPACE()

