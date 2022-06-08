/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

BEGIN_INFERNO_NAMESPACE()

///--

class LocalAllocatorState;

///--

/// Stats for local allocator
struct BM_CORE_MEMORY_API LocalAllocatorStats
{
    uint32_t numBlocks = 0; // number of distinct allocations performed
    uint32_t numPages = 0; // number of pages allocated
    uint32_t numDestroyers = 0; // number of registered destruction callbacks
    uint32_t numOversidedBlocks = 0; // number of allocations that were bigger than the maximum page size

    uint64_t numAllocatedBytes = 0; // number of total bytes allocated
    uint64_t numReservedBytes = 0; // number of total bytes reserved
    uint64_t numWastedBytes = 0; // number of bytes wasted due to alignment and other structures
    uint64_t numOversidedBytes = 0; // number of extra bytes allocated directly from heap because there were to big

    //--

    LocalAllocatorStats();

    void print(IFormatStream& f) const;
};

///--

/// Simple local allocator that is meant to allocate bunch of temporary allocations and release them all at once
/// NOTE: local allocator is strictly single threaded utility
class BM_CORE_MEMORY_API LocalAllocator : public MainPoolData<NoCopy>
{
public:
    LocalAllocator();
    LocalAllocator(const void* staticBuffer, uint32_t staticBufferSize);
    LocalAllocator(IPoolPaged& pool);
    LocalAllocator(IPoolPaged& pool, const void* staticBuffer, uint32_t staticBufferSize);
    LocalAllocator(IPoolPaged& pool, IPoolUnmanaged& overflowPool);
    LocalAllocator(IPoolPaged& pool, IPoolUnmanaged& overflowPool, const void* staticBuffer, uint32_t staticBufferSize);
    ~LocalAllocator();

    //--

    // local stats
    INLINE const LocalAllocatorStats& stats() const { return m_stats; }

    //--

    // Allocate memory, larger blocks may be allocated as outstanding allocations
    // NOTE: there's NO GUARANTEE of any continuity between blocks, just they they are allocated fast
    void* alloc(uint64_t size, uint32_t align);

    // create a copy of a string and store it locally, useful for parsers to preserve temporary strings
    char* strcpy(const char* txt, uint32_t length = INDEX_MAX);

    //--

    // allocate memory for an object, if it requires destruction we also add a proper cleanup function to call later
    template< typename T, typename... Args >
    INLINE T* createAndRegisterDestroyer(Args && ... args)
    {
        void* mem = alloc(sizeof(T), alignof(T));
        T* ptr = new (mem) T(std::forward< Args >(args)...);
        if (!std::is_trivially_destructible<T>::value)
            deferCleanup<T>(ptr);
        return ptr;
    }

    // allocate memory for an object ensure that there's no cleanup
    template< typename T, typename... Args >
    INLINE T* createWithoutDestruction(Args&& ... args)
    {
        void* mem = alloc(sizeof(T), alignof(T));
        T* ptr = new (mem) T(std::forward< Args >(args)...);
        return ptr;
    }

    //--

    // create new allocator directly in a fresh page from page pool, saves allocations
    static LocalAllocator* CreateLocalAllocator(uint32_t initialPageSize = 1024* 1024, IPoolPaged& pool = LocalPagePool(), IPoolUnmanaged& overflowPool = LargeAllocationPool());

    //--


private:
    enum EUninitialized { UNINITIALIZED };

    LocalAllocator(EUninitialized);

    LocalAllocatorState* m_state = nullptr;
    LocalAllocatorStats m_stats;

	template< typename T >
	struct TypeDestroyer
	{
		static void CallDestructor(void* ptr)
		{
			((T*)ptr)->~T();
		}
	};

	typedef void (*TCleanupFunc)(void* userData);
	void deferCleanup(TCleanupFunc func, void* userData);

	template< typename T >
	INLINE void deferCleanup(T* ptr)
	{
		deferCleanup(&TypeDestroyer<T>::CallDestructor, ptr);
	}

	void initialize(IPoolPaged& pool, IPoolUnmanaged& overflowPool, const void* staticBuffer, uint32_t staticBufferSize);
};
    
///--

END_INFERNO_NAMESPACE()
