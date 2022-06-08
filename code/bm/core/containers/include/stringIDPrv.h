/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

#include "stringBuf.h"

BEGIN_INFERNO_NAMESPACE()

namespace prv
{

	///--

	struct StringIDDataEntry
	{
		StringIDIndex next;
		char txt[1];
	};

	class StringIDDataStorage
	{
	public:
		static StringIDDataStorage& GetInstance();

		StringIDIndex place(StringView buf);

		StringIDDataEntry* entry(StringIDIndex index) const;
		const char* text(StringIDIndex index) const;

	private:
		StringIDDataStorage();

		SpinLock m_writeLock;

		uint8_t* m_writeStart = nullptr;
		uint8_t* m_writePtr = nullptr;
		uint8_t* m_writeEndPtr = nullptr;
		uint32_t m_numPages = 0;

		void allocPage();
	};

	///--

	class StringIDMap
	{
	public:
		StringIDMap();
		~StringIDMap();

		StringIDIndex find(uint32_t stringHash, const char* stringData, uint32_t stringLength) const;

		void insert(uint32_t stringHash, StringIDIndex stringIndex);

	private:
		static const uint32_t NUM_BUCKETS = 128 << 10;
		uint32_t m_buckets[NUM_BUCKETS];

		StringIDDataStorage& m_storage;

		void resize();

		void insertAfterResize(uint32_t stringHash, StringIDIndex stringIndex);
	};

	///--

} // prv

END_INFERNO_NAMESPACE()

