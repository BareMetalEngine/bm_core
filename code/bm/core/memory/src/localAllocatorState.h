/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

BEGIN_INFERNO_NAMESPACE()

///--

struct LocalAllocatorStats;

/// Internal state for local allocator, separate from the public header 
class LocalAllocatorState : public MainPoolData<NoCopy>
{
public:
    LocalAllocatorState(LocalAllocatorStats& stats, IPoolPaged& pagePool, IPoolUnmanaged& overflowPool, MemoryPage firstPage, void* firstUsableByte);
    LocalAllocatorState(LocalAllocatorStats& stats, IPoolPaged& pagePool, IPoolUnmanaged& overflowPool, const void* staticPool, uint32_t staticPoolSize);
    ~LocalAllocatorState();

    //--

    // Allocate memory, larger blocks may be allocated as outstanding allocations
    // NOTE: there's NO GUARANTEE of any continuity between blocks, just they they are allocated fast
    void* alloc(uint64_t size, uint32_t align);

    // Register destroyer to run when allocator is cleared
    typedef void (*TCleanupFunc)(void* userData);
    void registerDestroyed(void* ptr, TCleanupFunc func);

    //--

private:
    //--

    uint8_t* m_curBuffer = nullptr;
    uint8_t* m_curBufferEnd = nullptr;

    //--

    struct Destroyer
    {
        TCleanupFunc func = nullptr;
        void* data = nullptr;
        Destroyer* next = nullptr;
    };

    Destroyer* m_destroyerList = nullptr; // NOTE: back to front

    void callDestroyers();

    //--

    struct ExternalAllocation
    {
        void* ptr = nullptr;
        uint64_t size = 0;
        ExternalAllocation* next = nullptr;
    };

    ExternalAllocation* m_externalAllocations = nullptr;

    void releaseExternalAllocations();
    void* allocateExternalMemory(uint64_t size, uint32_t alignment);

    //--

    struct Page
    {
        MemoryPage page;
        Page* next = nullptr;
    };

    Page* m_pageList = nullptr;

    void releasePages();
    bool allocatePage(uint32_t size);

    //--

    IPoolPaged& m_pagePool;
    IPoolUnmanaged& m_overflowPool;

    //--

    LocalAllocatorStats& m_stats;

    //--    
};

///--

END_INFERNO_NAMESPACE()
