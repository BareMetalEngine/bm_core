/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

#include "memoryBlock.h"
#include "pool.h"

BEGIN_INFERNO_NAMESPACE()

///--

/// Abstract managed memory pool
class BM_CORE_MEMORY_API IPoolManaged : public IPool
{
public:
    IPoolManaged(const char* name);
    virtual ~IPoolManaged();

    //--

    // Allocate managed memory block, such blocks could be moved
    virtual MemoryBlock allocateBlock(uint64_t size, uint32_t alignment) = 0;

    // Free managed memory block
    virtual void freeBlock(MemoryBlock block) = 0;

    // Pin managed memory block - it won't be able to move and won't be touched by defragmentation
    virtual void* pinBlock(MemoryBlock block) = 0;

	// Unpin previously pinned memory block
	virtual void unpinlock(MemoryBlock block) = 0;

    //--
};

///--

END_INFERNO_NAMESPACE()
