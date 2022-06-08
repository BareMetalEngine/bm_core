/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "fileMemoryWriter.h"
#include "bm/core/memory/include/poolPaged.h"

BEGIN_INFERNO_NAMESPACE()

//--

FileMemoryWriter::FileMemoryWriter(StringBuf info, IPoolPaged* pool)
	: IFileWriter(FileFlagBit::MemoryBacked, info)
	, m_pool(pool ? *pool : LocalPagePool())
{
	m_activePageIndex = -1;
	m_activePagePtr = nullptr;
	m_activePageStart = 0;
	m_activePageEnd = 0;
}

FileMemoryWriter::~FileMemoryWriter()
{
	for (const auto & page : m_pages)
		m_pool.freePage(page);
}

uint64_t FileMemoryWriter::size() const
{
	return m_size;
}

uint64_t FileMemoryWriter::pos() const
{
	return m_pos;
}


void FileMemoryWriter::switchPage(uint32_t pageIndex)
{
	// create pages until we cover the requested size
	while (pageIndex >= m_pages.size())
	{
		auto page = m_pool.allocatPage(PAGE_SIZE);
		m_pages.pushBack(page);
	}

	// update offsets
	m_activePageIndex = pageIndex;
	m_activePageStart = pageIndex * PAGE_SIZE;
	m_activePageEnd = m_activePageStart + PAGE_SIZE;
	m_activePagePtr = m_pages[pageIndex].basePtr;
}

void FileMemoryWriter::seek(uint64_t offset)
{
	if (m_pos != offset)
	{
		m_pos = offset;

		auto pageIndex = offset / PAGE_SIZE;
		if (m_activePageIndex != pageIndex)
		{
			switchPage(pageIndex);
			DEBUG_CHECK(m_pos >= m_activePageStart);
			DEBUG_CHECK(m_pos < m_activePageEnd);
		}
	}
}

uint64_t FileMemoryWriter::readSync(void* ptr, uint64_t size)
{
	DEBUG_CHECK_RETURN_EX_V(ptr, "Invalid pointer", 0);

	uint64_t totalRead = 0;

	const auto readEnd = std::min<uint64_t>(m_pos + size, m_size);

	while (m_pos < readEnd)
	{
		if (m_pos >= m_activePageEnd)
		{
			m_activePageIndex = m_pos / PAGE_SIZE;
			m_activePageStart = m_activePageIndex * PAGE_SIZE;
			m_activePageEnd = m_activePageStart + PAGE_SIZE;
			m_activePagePtr = m_pages[m_activePageIndex].basePtr;
		}

		const auto readSize = std::min(readEnd - m_pos, m_activePageEnd - m_pos);
		memcpy((char*)ptr + totalRead, m_activePagePtr + (m_pos - m_activePageStart), readSize);
		m_pos += readSize;
		totalRead += readSize;
	}

	return totalRead;
}

uint64_t FileMemoryWriter::writeSync(const void* ptr, uint64_t size)
{
	DEBUG_CHECK_RETURN_EX_V(ptr, "Invalid pointer", 0);

	uint64_t totalWrite = 0;

	while (totalWrite < size)
	{
		if (m_pos >= m_activePageEnd)
		{
			ASSERT(m_activePageIndex <= m_pages.lastValidIndex());
			if (m_activePageIndex == m_pages.lastValidIndex())
			{
				auto page = m_pool.allocatPage(PAGE_SIZE);
				m_pages.pushBack(page);
			}

			m_activePageIndex = m_pos / PAGE_SIZE;
			m_activePageStart = m_activePageIndex * PAGE_SIZE;
			m_activePageEnd = m_activePageStart + PAGE_SIZE;
			m_activePagePtr = m_pages[m_activePageIndex].basePtr;
		}

		const auto writeSize = std::min(size - totalWrite, m_activePageEnd - m_pos);
		ASSERT(m_pos + writeSize <= m_activePageEnd);

		memcpy(m_activePagePtr + (m_pos - m_activePageStart), (const char*)ptr + totalWrite, writeSize);

		m_pos += writeSize;
		m_size = std::max(m_size, m_pos);
		totalWrite += writeSize;
	}

	return totalWrite;
}

//---

Buffer FileMemoryWriter::exportDataToBuffer(IPoolUnmanaged& pool) const
{
	if (!size())
		return nullptr;

	auto data = Buffer::CreateEmpty(pool, size());
	DEBUG_CHECK_RETURN_EX_V(data, "Out of memory", nullptr);

	BufferOutputStream<uint8_t> writer(data);
	const auto written = exportDataToMemory(0, size(), writer);
	DEBUG_CHECK_RETURN_EX_V(written == size(), "Not all data exported", nullptr);

	return data;
}

uint64_t FileMemoryWriter::exportDataToMemory(uint64_t offset, uint64_t size, BufferOutputStream<uint8_t>& output) const
{
	auto pageIndex = offset / PAGE_SIZE;
	auto pageOffset = offset % PAGE_SIZE;

	const auto contentLeft = (offset <= this->size()) ? (this->size() - offset) : 0;
	const auto readSize = std::min(contentLeft, std::min(size, output.capacityLeft()));
	size = readSize;

	while (size)
	{
		const auto leftOnPage = PAGE_SIZE - pageOffset;
		const auto copySize = std::min(size, leftOnPage);

		auto* ptr = output.alloc(copySize);
		DEBUG_CHECK_RETURN_EX_V(ptr, "Invalid output state", 0);

		const auto* pageData = m_pages[pageIndex].basePtr + pageOffset;
		memcpy(ptr, pageData, copySize);

		size -= copySize;
		pageOffset = 0;
		pageIndex += 1;
	}

	return readSize; // size is clamped to buffer capacity, we always write size bytes
}

uint64_t FileMemoryWriter::exportDataToFile(uint64_t offset, uint64_t size, IFileWriter* writer) const
{
	auto pageIndex = offset / PAGE_SIZE;
	auto pageOffset = offset % PAGE_SIZE;

	uint64_t totalWritten = 0;
	while (size)
	{
		const auto leftOnPage = PAGE_SIZE - pageOffset;
		const auto copySize = std::min(size, leftOnPage);

		const auto* pageData = m_pages[pageIndex].basePtr + pageOffset;
		const auto numWritten = writer->writeSync(pageData, copySize);

		totalWritten += numWritten;
		if (numWritten != copySize)
			break;
		
		size -= numWritten;
		pageOffset = 0;
		pageIndex += 1;
	}

	return totalWritten;
}

//--

END_INFERNO_NAMESPACE()
