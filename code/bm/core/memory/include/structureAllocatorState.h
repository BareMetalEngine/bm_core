/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

BEGIN_INFERNO_NAMESPACE()

//---

/// Internal state of structure allocator
class BM_CORE_MEMORY_API StructureAllocatorStateImpl : public MainPoolData<NoCopy>
{
public:
    ~StructureAllocatorStateImpl();

    INLINE uint32_t size() const { return m_numAllocated; }

    void* allocate(uint32_t elemSize, uint32_t elemAlign);
    void free(void* ptr);

	static StructureAllocatorStateImpl* Create(IPoolUnmanaged* pool, uint32_t pageSize);

private:
    struct FreeEntry
    {
        FreeEntry* next = nullptr;
    };

    struct Page
    {
        Page* next = nullptr;
    };

    Page* m_pageList = nullptr;
    FreeEntry* m_freeList = nullptr;

    uint8_t* m_currentPos = nullptr;
    uint8_t* m_currentEnd = nullptr;

    uint32_t m_numAllocated = 0;
    uint32_t m_maxAllocated = 0;
    uint32_t m_pageSize = 0;
    uint32_t m_pageCount = 0;

    IPoolUnmanaged& m_pool;

	StructureAllocatorStateImpl(IPoolUnmanaged& pool, uint32_t pageSize, Page* page, uint8_t* start, uint8_t* end);
};

//---

END_INFERNO_NAMESPACE()
