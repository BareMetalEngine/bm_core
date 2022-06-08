/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "structureAllocator.h"
#include "structureAllocatorState.h"
#include "poolUnmanaged.h"

BEGIN_INFERNO_NAMESPACE()

//---

namespace helper
{
    static uint32_t FindBestNumberOfElementsPerPage(uint32_t constHeaderSize, uint32_t elementSize, uint32_t elementAlignment, uint32_t elementsPerPage)
    {
        const auto defaultPageSize = 4096;

        // how many pages we need for all elements if we use minimal amount (64)
        elementSize = std::max<uint32_t>(elementSize, elementAlignment);
        auto minPageSize = Align<uint64_t>(constHeaderSize + (64 * elementSize), defaultPageSize);

        // how many elements can the min page really fit (fill it up to the full page size)
        elementsPerPage = (minPageSize - constHeaderSize) / elementSize;
        for (;;)
        {
            auto numBitWords = (elementsPerPage + 63) / 64;
            auto headerSize = Align<uint32_t>(constHeaderSize + sizeof(uint64_t) * numBitWords, elementAlignment);

            auto newElementsPerPage = (minPageSize - headerSize) / elementSize;
            if (newElementsPerPage == elementsPerPage)
                return newElementsPerPage;

            elementsPerPage = newElementsPerPage;
        }
    }

    static bool CheckMaskWord(const uint64_t* mask, uint32_t index)
    {
        const auto wordIndex = index / 64;
        const auto bitMask = 1ULL << (index & 63);
        return 0 != (mask[wordIndex] & bitMask);
    }

    static void SetMaskWord(uint64_t* mask, uint32_t index)
    {
        const auto wordIndex = index / 64;
        const auto bitMask = 1ULL << (index & 63);
        mask[wordIndex] |= bitMask;
    }

#ifdef PLATFORM_MSVC
    ALWAYS_INLINE int  __builtin_ctzll(uint64_t val)
    {
        unsigned long ret = 0;
        _BitScanForward64(&ret, val);
        return ret;
    }
#endif

    static uint64_t FindFirstBitSetAndClearIt(uint64_t* mask, uint32_t count)
    {
        auto numWords = (count + 63) / 64;
        auto* curPtr = mask;
        auto* endPtr = mask + numWords;
        while (curPtr < endPtr)
        {
            if (*curPtr != 0)
            {
                uint32_t bitIndex = __builtin_ctzll(*curPtr);
                uint32_t totalBitIndex = bitIndex + ((curPtr - mask) * 64);
                DEBUG_CHECK_EX(totalBitIndex < count, "Bit outside allowed range");
                *curPtr &= ~(1ULL << bitIndex);
                return totalBitIndex;
            }

            ++curPtr;
        }

        DEBUG_CHECK(!"No bits found");
        return 0;
    }
}


//---

template< typename T >
INLINE static T* SimpleAlloc(uint8_t*& ptr)
{
    auto* alignedPtr = AlignPtr(ptr, alignof(T));
    ptr = alignedPtr + sizeof(T);
    return (T*)alignedPtr;
}

StructureAllocatorStateImpl* StructureAllocatorStateImpl::Create(IPoolUnmanaged* pool, uint32_t pageSize)
{
    DEBUG_CHECK_RETURN_EX_V(pageSize >= sizeof(StructureAllocatorStateImpl) + sizeof(Page), "Page size to small for internal data", nullptr);

    // use default pool if not specified
    if (!pool)
        pool = &MainPool();

    // allocate page
    auto* memory = (uint8_t*)pool->allocateMemory(pageSize, 16);
    DEBUG_CHECK_RETURN_EX_V(memory, "Out of memory", nullptr);

    // page usable memory range
    auto* pageStartPtr = memory;
    auto* pageEndPtr = memory + pageSize;

    // page header
    auto* page = SimpleAlloc<Page>(pageStartPtr);
    page->next = nullptr;

    // setup header
    auto* stateBlockMemory = SimpleAlloc<StructureAllocatorStateImpl>(pageStartPtr);

    // create state
    return new (stateBlockMemory) StructureAllocatorStateImpl(*pool, pageSize, page, pageStartPtr, pageEndPtr);
}

StructureAllocatorStateImpl::StructureAllocatorStateImpl(IPoolUnmanaged& pool, uint32_t pageSize, Page* page, uint8_t* start, uint8_t* end)
    : m_pageList(page)
    , m_currentPos(start)
    , m_currentEnd(end)
    , m_pageSize(pageSize)
    , m_pageCount(1)
    , m_pool(pool)
{
}

StructureAllocatorStateImpl::~StructureAllocatorStateImpl()
{
    //DEBUG_CHECK_EX(m_numAllocated == 0, TempString("Not all structures from structure allocator were freed ({} remains), you are risking dangling pointers!", m_numAllocated));
}

void* StructureAllocatorStateImpl::allocate(uint32_t elemSize, uint32_t elemAlign) // NOTE: we pass those values here because they can be passed in registeres from caller based on template arguments
{
    // use free list if possible
    if (m_freeList)
    {
		m_numAllocated += 1;
        auto* elem = m_freeList;
        m_freeList = elem->next;
        return elem;
    }

    // allocate new element from page
    {
        auto* alignedPtr = AlignPtr(m_currentPos, elemAlign);
        if (alignedPtr + elemSize <= m_currentEnd)
        {
            m_numAllocated += 1;
            m_maxAllocated = std::max<uint32_t>(m_maxAllocated, m_numAllocated);
            m_currentPos = alignedPtr + elemSize;
            return alignedPtr;
        }
    }

    // allocate new page
    auto* page = (Page*) m_pool.allocateMemory(m_pageSize);
    DEBUG_CHECK_RETURN_EX_V(page, "Out of memory", nullptr);

    page->next = m_pageList;
    m_pageList = page;

    m_currentPos = (uint8_t*)(page + 1);
    m_currentEnd = (uint8_t*)page + m_pageSize;

    m_pageCount += 1;
	m_numAllocated += 1;
	m_maxAllocated = std::max<uint32_t>(m_maxAllocated, m_numAllocated);

    auto* alignedPtr = AlignPtr(m_currentPos, elemAlign);
	m_currentPos = alignedPtr + elemSize;

	return alignedPtr;
}

void StructureAllocatorStateImpl::free(void* ptr)
{
    DEBUG_CHECK_RETURN_EX(ptr, "Freeing invalid pointer");
    DEBUG_CHECK_RETURN_EX(m_numAllocated, "Allocator has nothing allocated");

    auto* freeElement = (FreeEntry*)ptr;
    freeElement->next = m_freeList;
    m_freeList = freeElement;
    m_numAllocated -= 1;
}

//---

void StructureAllocatorState::CreateState(IPoolUnmanaged* pool, uint32_t pageSize, StructureAllocatorState*& outState)
{
    outState = (StructureAllocatorState*) StructureAllocatorStateImpl::Create(pool, pageSize);
}

void StructureAllocatorState::ReleaseState(StructureAllocatorState* state)
{
    return ((StructureAllocatorStateImpl*)state)->~StructureAllocatorStateImpl();
}

void* StructureAllocatorState::AllocElement(StructureAllocatorState* state, uint32_t elemSize, uint32_t elemAlign)
{
    return ((StructureAllocatorStateImpl*)state)->allocate(elemSize, elemAlign);
}

void StructureAllocatorState::FreeElement(StructureAllocatorState* state, void* ptr)
{
    return ((StructureAllocatorStateImpl*)state)->free(ptr);
}

uint32_t StructureAllocatorState::Size(StructureAllocatorState* state)
{
    return ((StructureAllocatorStateImpl*)state)->size();
}

//---

END_INFERNO_NAMESPACE()
