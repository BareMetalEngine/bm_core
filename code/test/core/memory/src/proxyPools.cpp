/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "proxyPools.h"
#include "bm/core/memory/include/poolUnmanaged.h"

BEGIN_INFERNO_NAMESPACE()

//---

namespace prv
{
	ProxyUnmanagedPool::ProxyUnmanagedPool(IPoolUnmanaged& pool)
		: IPoolUnmanaged("ProxyUnmanagedPool")
		, m_pool(pool)
	{}

	void* ProxyUnmanagedPool::allocateMemory(uint64_t size, uint32_t alignment)
	{
		auto* ret = m_pool.allocateMemory(size, alignment);
		if (ret) 
		{
			m_countedAllocations += 1;
			m_countedBytesAllocated += size;
			m_activeAllocations += 1;
			m_activeBytes += size;
		}

		return ret;
	}

	void ProxyUnmanagedPool::freeMemory(void* ptr, uint64_t* outAllocationSize)
	{
		uint64_t retSize = 0;
		m_pool.freeMemory(ptr, &retSize);

		if (outAllocationSize)
			*outAllocationSize = retSize;

		m_countedFree += 1;
		m_countedBytesFreed += retSize;
		m_activeAllocations -= 1;
		m_activeBytes -= retSize;
	}

	void* ProxyUnmanagedPool::resizeMemory(void* ptr, uint64_t size, uint32_t alignment, uint64_t* outAllocationSize)
	{
		if (!ptr)
		{
			return allocateMemory(size, alignment);
		}
		else if (!size)
		{
			freeMemory(ptr, outAllocationSize);
			return nullptr;
		}

		uint64_t retSize = 0;
		void* ret = m_pool.resizeMemory(ptr, size, alignment, &retSize);

		if (outAllocationSize)
			*outAllocationSize = retSize;

		m_activeBytes += size;
		m_activeBytes -= retSize;
		return ret;
	}

} // prv

//---

END_INFERNO_NAMESPACE()