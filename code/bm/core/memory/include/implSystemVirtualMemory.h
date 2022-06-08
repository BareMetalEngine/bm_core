/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

#include "poolPaged.h"
#include <queue>
#include <unordered_map>

BEGIN_INFERNO_NAMESPACE()

///--

/// setup for system page pool
struct PoolPaged_SystemVirtualMemorySetup
{
    uint64_t minimumPageSize = 4096;
    uint64_t maximumPageSize = 64U << 20;
    bool protectReleasedPages = false;
    bool zeroInitializedPages = false;
    uint64_t pageRetentionBudget = 256U << 20;

    bool flagCpuReadable = true;
    bool flagCpuWritable = true;
    bool flagCpuExecutable = false;
    bool flagGpuReadable = false;
    bool flagGpuWritable = false;

    INLINE PoolPaged_SystemVirtualMemorySetup() {};
};

///--

/// Page pool using system direct allocations from system memory (VirtualAlloc, etc)
class BM_CORE_MEMORY_API PoolPaged_SystemVirtualMemory : public IPoolPaged
{
public:
    PoolPaged_SystemVirtualMemory(const char* name, const PoolPaged_SystemVirtualMemorySetup& setup);
    virtual ~PoolPaged_SystemVirtualMemory();

    virtual MemoryPage allocatPage(uint64_t size) override;
    virtual void freePage(MemoryPage page) override;
    virtual uint64_t queryMinimumPageSize() const override;
    virtual uint64_t queryMaximumPageSize() const override;

    virtual void print(IFormatStream& f, int details = 0) const override;

private:
    struct PageInfo
    {
        void* basePtr = nullptr;
        uint64_t size = 0;
        char bucket = -1;
    };

    std::atomic<uint32_t> m_nextPageIndex = 0;
    PoolPaged_SystemVirtualMemorySetup m_setup;
    uint32_t m_minSizeLog2 = 0;
    uint32_t m_maxSizeLog2 = 0;

    SpinLock m_allocatedPagesLock;
    std::unordered_map<uint32_t, PageInfo> m_allocatedPages;

    struct FreeBucket
    {
        SpinLock lock;
        std::vector<PageInfo> pages; // FILO - to increase "hot cache"
    };
    
    FreeBucket m_freePagesBuckets[64];

    uint64_t pageSizeForBucket(uint8_t bucket) const;

    PageInfo allocateRawPage(uint64_t size);
    void freeRawPage(PageInfo page);

    uint32_t registerRawPage(const PageInfo& info);
    PageInfo unregisterRawPage(MemoryPage page);

private:
    void* allocateVirtualMemory(uint64_t size, uint64_t& outAllocatedSize) const;
    void freeVirtualMemory(void* ptr, uint64_t size) const;

    std::atomic<uint32_t> m_cachePageCount;
    std::atomic<uint64_t> m_cachePageSize;
};


///--

END_INFERNO_NAMESPACE()
