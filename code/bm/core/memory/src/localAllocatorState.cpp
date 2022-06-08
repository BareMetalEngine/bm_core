/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "localAllocator.h"
#include "localAllocatorState.h"
#include "poolUnmanaged.h"
#include "poolPaged.h"

BEGIN_INFERNO_NAMESPACE()

//---

LocalAllocatorState::LocalAllocatorState(LocalAllocatorStats& stats, IPoolPaged& pagePool, IPoolUnmanaged& overflowPool, MemoryPage firstPage, void* firstUsableByte)
	: m_pagePool(pagePool)
	, m_overflowPool(overflowPool)
    , m_stats(stats)
{
    auto* pageEntry = (Page*)firstUsableByte;
    pageEntry->page = firstPage;
    pageEntry->next = nullptr;
    m_pageList = pageEntry;

    m_curBuffer = (uint8_t*)(pageEntry + 1);
    m_curBufferEnd = firstPage.endPtr;

    m_stats.numPages += 1;
    m_stats.numReservedBytes += firstPage.endPtr - firstPage.basePtr;
}

LocalAllocatorState::LocalAllocatorState(LocalAllocatorStats& stats, IPoolPaged& pagePool, IPoolUnmanaged& overflowPool, const void* staticPool, uint32_t staticPoolSize)
	: m_pagePool(pagePool)
	, m_overflowPool(overflowPool)
	, m_stats(stats)
{
    m_curBuffer = (uint8_t*)staticPool;
    m_curBufferEnd = m_curBuffer + staticPoolSize;
}

LocalAllocatorState::~LocalAllocatorState()
{
    callDestroyers();
    releaseExternalAllocations();
    releasePages(); // MUST BEST LAST
}


void LocalAllocatorState::callDestroyers()
{
    auto ptr = m_destroyerList;
    m_destroyerList = nullptr;

    while (ptr)
    {
        ptr->func(ptr->data);
        ptr = ptr->next;
    }
}

void LocalAllocatorState::releaseExternalAllocations()
{
    auto ptr = m_externalAllocations;
    m_externalAllocations = nullptr;

    while (ptr)
    {
        m_overflowPool.freeMemory(ptr->ptr);
        ptr = ptr->next;
    }
}

void* LocalAllocatorState::allocateExternalMemory(uint64_t size, uint32_t alignment)
{
    auto* ptr = m_overflowPool.allocateMemory(size, alignment);
    DEBUG_CHECK_RETURN_EX_V(ptr, "Out of memory", nullptr);

    auto* block = (ExternalAllocation*)alloc(sizeof(ExternalAllocation), alignof(ExternalAllocation));
    block->ptr = ptr;
    block->size = size;

    m_stats.numOversidedBlocks += 1;
    m_stats.numOversidedBytes += size;
    m_stats.numWastedBytes += sizeof(ExternalAllocation); // wasted memory

    return ptr;
}

void LocalAllocatorState::releasePages()
{
    m_curBuffer = nullptr;
    m_curBufferEnd = nullptr;

	auto ptr = m_pageList;
    m_pageList = nullptr;

	while (ptr)
	{
        auto* next = ptr->next;
        m_pagePool.freePage(ptr->page);
		ptr = next;
	}
}

void LocalAllocatorState::registerDestroyed(void* ptr, TCleanupFunc func)
{
    auto* entry = (Destroyer*) alloc(sizeof(Destroyer), alignof(Destroyer));
    entry->func = func;
    entry->data = ptr;
    entry->next = m_destroyerList;
    m_destroyerList = entry;

    m_stats.numDestroyers += 1;
    m_stats.numWastedBytes += sizeof(Destroyer); // wasted memory
}

bool LocalAllocatorState::allocatePage(uint32_t size)
{
    const auto page = m_pagePool.allocatPage(size);
    DEBUG_CHECK_RETURN_EX_V(page.basePtr, "Out of memory", false);

	m_stats.numWastedBytes += sizeof(Page);
	m_stats.numReservedBytes += size;
	m_stats.numPages += 1;

	auto* pageLinkPtr = AlignPtr(page.basePtr, alignof(Page));
	ASSERT(pageLinkPtr + sizeof(Page) <= page.endPtr);
	m_curBuffer = pageLinkPtr + sizeof(Page);
    m_curBufferEnd = page.endPtr;

    auto pageLink = (Page*)pageLinkPtr;
    pageLink->page = page;
    pageLink->next = m_pageList;
    m_pageList = pageLink;

    return true;
}

void* LocalAllocatorState::alloc(uint64_t size, uint32_t align)
{
    // most typical case - we fit
    auto* alignedPtr = AlignPtr(m_curBuffer, align);
    if (alignedPtr + size <= m_curBufferEnd)
    {
        m_stats.numBlocks += 1;
        m_stats.numAllocatedBytes += size;
        m_stats.numWastedBytes += (alignedPtr - m_curBuffer);
        m_curBuffer = alignedPtr + size;

        return alignedPtr;
    }

    // we don't fit, calculate maximum page size
    static const auto InitialPageSize = 64U << 10;
    static const auto MaximiumPageSize = 64U << 20;
    const auto prevPageSize = m_pageList ? std::max<uint32_t>(NextPowerOf2(1 + m_pageList->page.size()), InitialPageSize) : InitialPageSize;
    const auto nextPageSize = std::min<uint32_t>(prevPageSize, MaximiumPageSize);

    // if the allocation fits in the page size allocate it from page
    const auto wastedPageSize = sizeof(Page) * 2;
    if (size <= (nextPageSize - wastedPageSize))
    {
        DEBUG_CHECK_RETURN_EX_V(allocatePage(nextPageSize), "Out of memory", nullptr);

		auto* alignedPtr = AlignPtr(m_curBuffer, align);
        ASSERT(alignedPtr + size <= m_curBufferEnd);

		m_stats.numBlocks += 1;
		m_stats.numAllocatedBytes += size;
		m_stats.numWastedBytes += (alignedPtr - m_curBuffer);
        m_curBuffer = alignedPtr + size;

		return alignedPtr;
    }

    // allocate as external allocation
    return allocateExternalMemory(size, align);
}

//---

END_INFERNO_NAMESPACE()
