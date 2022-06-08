/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

#include "fileWriter.h"

BEGIN_INFERNO_NAMESPACE()

//--

// a memory based file writer
class BM_CORE_FILE_API FileMemoryWriter : public IFileWriter
{
public:
    FileMemoryWriter(StringBuf info = "", IPoolPaged* pool = nullptr);
    virtual ~FileMemoryWriter();

	INLINE uint32_t pageSize() const { return PAGE_SIZE; }
	INLINE uint32_t pageCount() const { return m_pages.size(); }

    //----
    /// IFileHandler

	virtual uint64_t size() const override final;
	virtual uint64_t pos() const override final;

	virtual void seek(uint64_t offset) override final;

	virtual uint64_t readSync(void* ptr, uint64_t size) override final;
	virtual uint64_t writeSync(const void* ptr, uint64_t size) override final;

    //---

	// export data from this file into a linear buffer output
	uint64_t exportDataToMemory(uint64_t offset, uint64_t size, BufferOutputStream<uint8_t>& output) const;

	// write portion of this memory data to file on disk
	uint64_t exportDataToFile(uint64_t offset, uint64_t size, IFileWriter* writer) const;

	// export data to buffer
	Buffer exportDataToBuffer(IPoolUnmanaged& pool = MainPool()) const;

	//--

protected:
	static const uint32_t PAGE_SIZE = 4U << 20;

	uint64_t m_pos = 0;
	uint64_t m_size = 0;

	int m_activePageIndex = -1;
	uint8_t* m_activePagePtr = nullptr;
	uint64_t m_activePageStart = 0;
	uint64_t m_activePageEnd = 0; // capacity, not size

	Array<MemoryPage> m_pages;// ordered page list
	bool m_outOfMemory = false;

	IPoolPaged& m_pool;

	void switchPage(uint32_t pageIndex);
};

//--

END_INFERNO_NAMESPACE()
