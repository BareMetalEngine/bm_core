/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "poolUnmanaged.h"

BEGIN_INFERNO_NAMESPACE()

//--

IPoolUnmanaged::IPoolUnmanaged(const char* name)
	: IPool(name, PoolType::Unmanaged)
{}

IPoolUnmanaged::~IPoolUnmanaged()
{}

//---

void* PoolAllocate(IPoolUnmanaged& pool, uint64_t size, uint32_t alignment)
{
	return pool.allocateMemory(size, alignment);
}

void* PoolReallocate(IPoolUnmanaged& pool, void* ptr, uint64_t size, uint32_t alignment)
{
	return pool.resizeMemory(ptr, size, alignment);
}

void PoolFree(IPoolUnmanaged& pool, void* ptr)
{
	pool.freeMemory(ptr);
}

//---

#if 0
   // allocate (and construct) N objects
template< typename T >
INLINE T* allocateCount(uint32_t count)
{
	T* mem = (T*)allocateMemory(sizeof(T) * count, alignof(T));
	DEBUG_CHECK_RETURN_EX_V(mem, "OOM", nullptr);

	std::uninitialized_default_construct_n(mem, count);

	return mem;
}

// allocate (and construct) N objects
template< typename T >
INLINE T* allocateCountFromTemplate(uint32_t count, const T& defaultValue)
{
	T* mem = (T*)allocateMemory(sizeof(T) * count, alignof(T));
	DEBUG_CHECK_RETURN_EX_V(mem, "OOM", nullptr);

	std::uninitialized_fill_n(mem, count, defaultValue);

	return mem;
}

// allocate (and construct) N objects
template< typename T >
INLINE void freeCount(T* ptr, uint32_t count)
{
	std::destroy_n(ptr, count);

	freeMemory(ptr);

	return mem;
}

// resize pointer, creates/destroys elements
template< typename T >
INLINE T* resizeCount(T* currentPointer, uint32_t currentSize, uint32_t newSize)
{
	if (newSize > currentSize)
	{
		auto* mem = (T*)resizeMemory(currentPointer, sizeof(T) * newSize, alignof(T));
		DEBUG_CHECK_RETURN_EX_V(mem, "OOM", currentPointer);

		const auto extra = newSize - currentSize;
		std::uninitialized_default_construct_n(mem + currentSize, extra);
		return mem;
	}
	else if (newSize < currentSize)
	{
		const auto removed = currentSize - newSize;
		std::destroy_n(currentPointer + newSize, removed);

		auto* mem = (T*)resizeMemory(currentPointer, sizeof(T) * newSize, alignof(T));
		DEBUG_CHECK_RETURN_EX_V(mem, "OOM", currentPointer);
		return mem;
	}
	else
	{
		return currentPointer;
	}
}
#endif

END_INFERNO_NAMESPACE()
