/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "implDynamicNativeAllocator.h"

BEGIN_INFERNO_NAMESPACE()

//--

#ifndef BUILD_FINAL
#define TRACK_SIZES
#endif

PoolUnmanaged_DynamicNativeAllocator::PoolUnmanaged_DynamicNativeAllocator(const char* name)
	: IPoolUnmanaged(name)
{}

PoolUnmanaged_DynamicNativeAllocator::~PoolUnmanaged_DynamicNativeAllocator()
{}

void* PoolUnmanaged_DynamicNativeAllocator::allocateMemory(uint64_t size, uint32_t alignment)
{
#if defined(TRACK_SIZES)
	alignment = 16;
#endif

#if defined(PLATFORM_MSVC)
	void* ret = _aligned_malloc(size, alignment);
#elif defined(PLATFORM_POSIX)
	void* ret = aligned_alloc(alignment, size);
#elif defined(PLATFORM_PSX)
	void* ret = aligned_alloc(alignment, size);
#else
#error "Add platform crap"
#endif

	DEBUG_CHECK_RETURN_EX_V(ret, "OOM in allocator", nullptr);

#if defined(TRACK_SIZES)
	notifyAllocation(size);
#endif

	return ret;
}

void PoolUnmanaged_DynamicNativeAllocator::freeMemory(void* mem, uint64_t* outAllocationSize)
{
	if (mem)
	{
#if defined(TRACK_SIZES)
#if defined(PLATFORM_MSVC)
		auto size = _aligned_msize(mem, 16, 0);
#else
		auto size = malloc_usable_size(mem);
#endif
		if (outAllocationSize)
			*outAllocationSize = size;
		notifyFree(size);
#endif

#if defined(PLATFORM_MSVC)
		_aligned_free(mem);
#elif defined(PLATFORM_POSIX)
		free(mem);
#elif defined(PLATFORM_PSX)
		free(mem);
#else
#error "Add platform crap"
#endif
	}
}

void* PoolUnmanaged_DynamicNativeAllocator::resizeMemory(void* mem, uint64_t newSize, uint32_t alignment, uint64_t* outAllocationSize)
{
	DEBUG_CHECK_RETURN_EX_V(alignment <= 16, "Alignment larger than 16 is NOT supported", nullptr);

	if (newSize == 0)
	{
		freeMemory(mem, outAllocationSize);
		return nullptr;
	}
	else if (mem == nullptr)
	{
		return allocateMemory(newSize, alignment);
	}

#if defined(TRACK_SIZES)
	auto size = _aligned_msize(mem, 16, 0);
	if (outAllocationSize)
		*outAllocationSize = size;

	void* ret = _aligned_realloc(mem, newSize, alignment);
	DEBUG_CHECK_RETURN_EX_V(ret, "OOM in allocator", nullptr);

	notifyAllocation(newSize);
	notifyFree(size);

	return ret;
#else
	#if defined(PLATFORM_MSVC)
		return _aligned_realloc(mem, newSize, alignment);
	#elif defined(PLATFORM_POSIX)
		void* ret = allocate(newSize, alignment, tag);
		size_t currentSize = malloc_usable_size(mem);
		memcpy(ret, mem, std::min<size_t>(currentSize, newSize));
		deallocate(mem);
		return ret;
	#elif defined(PLATFORM_PSX)
		return reallocalign(mem, newSize, alignment);
	#else
		#error "Please provide ALIGNED realloc on this platform"
		return nullptr;
	#endif
#endif
}

//---

// TODO: find better way of initializing this

static IPoolUnmanaged* GMainPool = new PoolUnmanaged_DynamicNativeAllocator("MainPool");
static IPoolUnmanaged* GExtendedPool = new PoolUnmanaged_DynamicNativeAllocator("ExtendedPool");
static IPoolUnmanaged* GLargePool = new PoolUnmanaged_DynamicNativeAllocator("LargeAllocations");

IPoolUnmanaged& MainPool()
{
	return *GMainPool;
}

IPoolUnmanaged& ExtendedPool()
{
	return *GExtendedPool;
}

IPoolUnmanaged& LargeAllocationPool()
{
	return *GLargePool;
}

//---

END_INFERNO_NAMESPACE()
