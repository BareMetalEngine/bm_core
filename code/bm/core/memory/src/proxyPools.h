/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

#include "bm/core/memory/include/buffer.h"
#include "bm/core/memory/include/bufferView.h"
#include "bm/core/memory/include/poolUnmanaged.h"

BEGIN_INFERNO_NAMESPACE()

//---

namespace prv
{
	class ProxyUnmanagedPool : public IPoolUnmanaged
	{
	public:
		ProxyUnmanagedPool(IPoolUnmanaged& pool = MainPool());

		uint32_t m_countedAllocations = 0;
		uint32_t m_countedBytesAllocated = 0;

		uint32_t m_countedFree = 0;
		uint32_t m_countedBytesFreed = 0;

		uint32_t m_activeBytes = 0;
		uint32_t m_activeAllocations = 0;

		//--

		virtual void* allocateMemory(uint64_t size, uint32_t alignment) override final;
		virtual void freeMemory(void* ptr, uint64_t* outAllocationSize = nullptr) override final;
		virtual void* resizeMemory(void* ptr, uint64_t size, uint32_t alignment, uint64_t* outAllocationSize) override final;

	private:
		IPoolUnmanaged& m_pool;
	};
}

//---

END_INFERNO_NAMESPACE()