/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

#include "poolUnmanaged.h"

BEGIN_INFERNO_NAMESPACE()

///--

/// Memory pool of CPU visible system memory that is not limited in size and uses system malloc/free
/// This is mostly used on Desktop platforms
class BM_CORE_MEMORY_API PoolUnmanaged_DynamicNativeAllocator : public IPoolUnmanaged
{
public:
	PoolUnmanaged_DynamicNativeAllocator(const char* name);
    virtual ~PoolUnmanaged_DynamicNativeAllocator();

	virtual void* allocateMemory(uint64_t size, uint32_t alignment) override final;
	virtual void freeMemory(void* ptr, uint64_t* outAllocationSize) override final;
	virtual void* resizeMemory(void* ptr, uint64_t size, uint32_t alignment, uint64_t* outAllocationSize) override final;
};

///--

END_INFERNO_NAMESPACE()
