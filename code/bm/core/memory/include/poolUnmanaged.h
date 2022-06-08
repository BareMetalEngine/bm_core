/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

#include "pool.h"

BEGIN_INFERNO_NAMESPACE()

///--

/// Abstract unmanaged (not movable) memory pool
class BM_CORE_MEMORY_API IPoolUnmanaged : public IPool
{
public:
    IPoolUnmanaged(const char* name);
    virtual ~IPoolUnmanaged();

    //---
    
    // Allocate unmanaged memory block from pool
    virtual void* allocateMemory(uint64_t size, uint32_t alignment = 4) = 0;

    // Free unmanaged memory block
    virtual void freeMemory(void* ptr, uint64_t* outAllocationSize = nullptr) = 0;

	// Resize unmanaged memory block from pool
	virtual void* resizeMemory(void* ptr, uint64_t size, uint32_t alignment = 4, uint64_t* outAllocationSize = nullptr) = 0;

    //--- 
};

//--


END_INFERNO_NAMESPACE()
