/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "localAllocator.h"
#include "localAllocatorState.h"
#include "poolPaged.h"

BEGIN_INFERNO_NAMESPACE()

//---

LocalAllocatorStats::LocalAllocatorStats()
{}

void LocalAllocatorStats::print(IFormatStream& f) const
{
    f.appendf("Allocated {} in {} blocks ({} reserved, {} wasted) across {} pages",
        MemSize(numAllocatedBytes), numBlocks, MemSize(numReservedBytes), MemSize(numWastedBytes), numPages);
    if (numOversidedBlocks)
        f.appendf("Allocated {} in {} oversized blocks", MemSize(numOversidedBytes), numOversidedBlocks);
    if (numDestroyers)
        f.appendf("Registered {} pending destroyed", numDestroyers);
}

//---

LocalAllocator::LocalAllocator(EUninitialized)
{}

LocalAllocator::LocalAllocator()
{
    initialize(LocalPagePool(), LargeAllocationPool(), nullptr, 0);
}

LocalAllocator::LocalAllocator(const void* staticBuffer, uint32_t staticBufferSize)
{
    initialize(LocalPagePool(), LargeAllocationPool(), staticBuffer, staticBufferSize);
}

LocalAllocator::LocalAllocator(IPoolPaged& pool)
{
    initialize(pool, LargeAllocationPool(), nullptr, 0);
}

LocalAllocator::LocalAllocator(IPoolPaged& pool, const void* staticBuffer, uint32_t staticBufferSize)
{
    initialize(pool, LargeAllocationPool(), staticBuffer, staticBufferSize);
}

LocalAllocator::LocalAllocator(IPoolPaged& pool, IPoolUnmanaged& overflowPool)
{
    initialize(pool, overflowPool, nullptr, 0);
}

LocalAllocator::LocalAllocator(IPoolPaged& pool, IPoolUnmanaged& overflowPool, const void* staticBuffer, uint32_t staticBufferSize)
{
    initialize(pool, overflowPool, staticBuffer, staticBufferSize);
}

LocalAllocator::~LocalAllocator()
{
    auto state = m_state;
    m_state = nullptr;
    state->~LocalAllocatorState();
}

LocalAllocator* LocalAllocator::CreateLocalAllocator(uint32_t initialPageSize /*= 1U << 1024*/, IPoolPaged& pool /*= PagePool()*/, IPoolUnmanaged& overflowPool /*= LargeAllocationPool()*/)
{
    // page can't be to small
    initialPageSize = std::max<uint32_t>(65536, initialPageSize);

    // allocate root page
    auto memoryPage = pool.allocatPage(initialPageSize);
    DEBUG_CHECK_RETURN_EX_V(memoryPage.basePtr, "Out of memory", nullptr);

    // outer wrapper pointer
    auto* wrapperPtr = AlignPtr(memoryPage.basePtr, alignof(LocalAllocator));
    auto* wrapper = new (wrapperPtr) LocalAllocator(UNINITIALIZED);

    // state data
    auto* statePtr = AlignPtr(wrapperPtr + alignof(LocalAllocator), alignof(LocalAllocatorState));
    auto* firstFreeByte = statePtr + sizeof(LocalAllocatorState);

    wrapper->m_state = new (statePtr) LocalAllocatorState(wrapper->m_stats, pool, overflowPool, memoryPage, firstFreeByte);

    return wrapper;
}

//---

void LocalAllocator::initialize(IPoolPaged& pool, IPoolUnmanaged& overflowPool, const void* staticBuffer, uint32_t staticBufferSize)
{
    if (!staticBuffer || staticBufferSize < (sizeof(LocalAllocatorState) + alignof(LocalAllocatorState)))
    {
        auto memorySize = std::max<uint32_t>(4096, NextPowerOf2(staticBufferSize));
        auto memoryPage = pool.allocatPage(memorySize);
        DEBUG_CHECK_RETURN_EX(memoryPage.basePtr, "Out of memory");
        
        auto* statePtr = AlignPtr(memoryPage.basePtr, alignof(LocalAllocatorState));
        auto* firstFreeByte = statePtr + sizeof(LocalAllocatorState);

        m_state = new (statePtr) LocalAllocatorState(m_stats, pool, overflowPool, memoryPage, firstFreeByte);
    }
    else
    {
		auto* statePtr = AlignPtr((uint8_t*)staticBufferSize, alignof(LocalAllocatorState));
		auto* firstFreeByte = statePtr + sizeof(LocalAllocatorState);

        auto consumedSize = firstFreeByte - (uint8_t*)staticBufferSize;
        ASSERT(consumedSize <= staticBufferSize);
        staticBufferSize -= consumedSize;

        m_state = new (statePtr) LocalAllocatorState(m_stats, pool, overflowPool, firstFreeByte, staticBufferSize);
    }
}

void LocalAllocator::deferCleanup(TCleanupFunc func, void* ptr)
{
    m_state->registerDestroyed(ptr, func);
}

//---

/*void LocalAllocator::clear()
{
    m_state->clear();
}*/

void* LocalAllocator::alloc(uint64_t size, uint32_t align)
{
    return m_state->alloc(size, align);
}

char* LocalAllocator::strcpy(const char* txt, uint32_t length)
{
    if (!txt)
        return nullptr;

    if (length == INDEX_MAX)
        length = strlen(txt);

    auto ptr = (char*)m_state->alloc(length + 1, 1);
    memcpy(ptr, txt, length);
	ptr[length] = 0;

    return ptr;
}

//---

END_INFERNO_NAMESPACE()

