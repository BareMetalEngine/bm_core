/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "pagedBuffer.h"

#include "bm/core/memory/include/poolPaged.h"

BEGIN_INFERNO_NAMESPACE()

///--

PagedBuffer::PagedBuffer(uint32_t alignment)
    : m_alignment(alignment)
    , m_pool(LocalPagePool())
{
    m_pageSize = 1U << 20;
}

PagedBuffer::PagedBuffer(IPoolPaged& pool, uint32_t alignment)
    : m_alignment(alignment)
    , m_pool(pool)
{
    m_pageSize = 1U << 20;
}

PagedBuffer::~PagedBuffer()
{
    clear();
}

void PagedBuffer::clear()
{
    auto* page = m_pageList;

    // reset state
    m_writePtr = nullptr;
    m_writeStartPtr = nullptr;
    m_writeEndPtr = nullptr;
    m_numBytes = 0;
    m_writeStartPtr = nullptr;
    m_writeOffsetSoFar = 0;
    m_pageCount = 0;
    m_pageHead = nullptr;
    m_pageList = nullptr;

    // free full pages
    while (page)
    {
        auto* next = page->prev;
        m_pool.freePage(page->page);
        page = next;
    }

    // do not put anything here !
}

uint8_t* PagedBuffer::allocSmall(uint32_t memorySize)
{
	ASSERT_EX(memorySize <= m_pageSize / 16, "Memory size of small allocation is way to big"); // typically

    auto* ret = m_writePtr;
    auto* retEnd = ret + memorySize;
    if (retEnd <= m_writeEndPtr)
    {
        m_writePtr = retEnd;
        m_numBytes += memorySize;
    }
    else
    {
        allocPage();

        ret = m_writePtr;
        m_writePtr += memorySize;
		m_numBytes += memorySize;
    }

    return ret;
}

uint8_t* PagedBuffer::allocateBatch(uint64_t memorySize, uint32_t elementSize, uint32_t& outNumAllocated)
{
    auto* ret = m_writePtr;
    auto* retEnd = ret + memorySize;

	// easiest case first: we fit
	if (retEnd <= m_writeEndPtr)
    {
        m_writePtr = retEnd;
		m_numBytes += memorySize;
		outNumAllocated = memorySize / elementSize; // all elements allocated
		return ret;
    }

	// second easy case: there's some memory on the page
	const auto bytesLeftOnPage = m_writeEndPtr - m_writePtr;
    if (bytesLeftOnPage > 0)
    {
		auto maxElmentsInThisPage = bytesLeftOnPage / elementSize;
		if (maxElmentsInThisPage > 0)
		{
			outNumAllocated = maxElmentsInThisPage;
			m_writePtr += maxElmentsInThisPage * elementSize;
			m_numBytes += maxElmentsInThisPage * elementSize;
			return ret;
		}
    }

	// if we are here then there's no memory left on the page
    allocPage();

	// retry
	return allocateBatch(memorySize, elementSize, outNumAllocated);
}

void PagedBuffer::writeLarge(const void* data, uint64_t size)
{
    const auto* readPtr = (uint8_t*)data;
    const auto* readEndPtr = readPtr + size;
        
    uint64_t left = size;
    while (readPtr < readEndPtr)
    {
        uint32_t memorySizeToCopy = std::min<uint64_t>(INDEX_MAX, left);
        void* writePtr = allocateBatch(memorySizeToCopy, 1, memorySizeToCopy);
		DEBUG_CHECK_RETURN_EX(writePtr != nullptr, "Out of memory");
		DEBUG_CHECK_RETURN_EX(memorySizeToCopy, "Out of memory");

        if (memorySizeToCopy)
        {
			memcpy(writePtr, readPtr, memorySizeToCopy);
            readPtr += memorySizeToCopy;
            left -= memorySizeToCopy;
        }
    }
}

/*void PagedBuffer::fill(const void* templateData, uint64_t elementCountToWrite)
{
    uint64_t left = elementCountToWrite;
    while (left > 0)
    {
        uint32_t toCopy = std::min<uint64_t>(INDEX_MAX, left);
        uint64_t memorySize = toCopy * m_elementSize;
        uint8_t* writePtr = (uint8_t* )allocateBatch(memorySize, toCopy, toCopy);
        if (toCopy)
        {
            uint8_t* writePtrEnd = writePtr + (toCopy * m_elementSize);

            while (writePtr < writePtrEnd)
            {
                memcpy(writePtr, templateData, m_elementSize);
                writePtr += m_elementSize;
            }
                
            left -= toCopy;
        }
    }
}*/

void PagedBuffer::append(const PagedBuffer& buffer)
{
	buffer.iteratePages([this](const void* pageData, uint32_t pageSize, uint64_t offsetSoFar)
		{
			writeLarge(pageData, pageSize);
		});        
}

void PagedBuffer::copy(BufferView view) const
{
	auto* writePtr = view.data();
	auto* writeEndPtr = view.data() + view.size();

	iteratePages([writePtr, writeEndPtr](const void* pageData, uint32_t pageSize, uint64_t offsetSoFar)
		{
			DEBUG_CHECK_RETURN(writePtr + offsetSoFar + pageSize <= writeEndPtr);
			memcpy(writePtr + offsetSoFar, pageData, pageSize);
		});
}

void PagedBuffer::allocPage()
{
    // update current page
    if (m_writePtr > m_writeStartPtr)
    {
        m_pageList->dataSize = m_writePtr - m_writeStartPtr;
        m_writeOffsetSoFar += m_pageList->dataSize;
    }

    // allocate memory page
    auto memoryPage = m_pool.allocatPage(m_pageSize);

    // allocate page memory
    auto page = (Page*)memoryPage.basePtr;
    page->next = nullptr;
    page->prev = m_pageList;
    page->dataSize = 0;
    page->offsetSoFar = m_writeOffsetSoFar;
    if (m_pageList) m_pageList->next = page;
    m_pageList = page;

    // link to list
    if (nullptr == m_pageHead)
        m_pageHead = page;

    // setup write pointers
    m_writeEndPtr = memoryPage.endPtr;
    m_writeStartPtr = AlignPtr((uint8_t*)memoryPage.basePtr + sizeof(Page), m_alignment);
    m_writePtr = m_writeStartPtr;
}

//--

void PagedBuffer::iteratePages(const std::function<void(const void* pageData, uint32_t pageDataSize, uint64_t totalOffset)>& func) const
{
	uint64_t offsetSoFar = 0;
	if (auto* page = m_pageHead)
	{
		while (page)
		{
			auto* pagePayload = AlignPtr((char*)page + sizeof(Page), m_alignment);
			func(pagePayload, page->dataSize, offsetSoFar);
			offsetSoFar += page->dataSize;
			page = page->next;
		}
	}

	if (m_writePtr > m_writeStartPtr)
		func(m_writeStartPtr, m_writePtr - m_writeStartPtr, offsetSoFar);
}

void PagedBuffer::iteratePages(const std::function<void(void* pageData, uint32_t pageDataSize, uint64_t totalOffset)>& func)
{
	uint64_t offsetSoFar = 0;
	if (auto* page = m_pageHead)
	{
		while (page)
		{
			auto* pagePayload = AlignPtr((char*)page + sizeof(Page), m_alignment);
			func(pagePayload, page->dataSize, offsetSoFar);
			offsetSoFar += page->dataSize;
			page = page->next;
		}
	}

	if (m_writePtr > m_writeStartPtr)
		func(m_writeStartPtr, m_writePtr - m_writeStartPtr, offsetSoFar);
}

//--

Buffer PagedBuffer::toBuffer(IPoolUnmanaged& pool) const
{
    auto buf = Buffer::CreateEmpty(pool, dataSize(), m_alignment);
    DEBUG_CHECK_RETURN_EX_V(buf, "OOM", nullptr);

    copy(buf);

    return buf;
}

//--

END_INFERNO_NAMESPACE()
