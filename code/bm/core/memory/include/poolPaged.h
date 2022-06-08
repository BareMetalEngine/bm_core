/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

#include "pool.h"

BEGIN_INFERNO_NAMESPACE()

///--

/// Abstract pool for strictly POW2 pages
class BM_CORE_MEMORY_API IPoolPaged : public IPool
{
public:
    IPoolPaged(const char* name);
    virtual ~IPoolPaged();

    //---
    
    // Allocate a page
    // NOTE: requested page size will be rounded up to next power of two
    // NOTE: pages are statically aligned to 64 bytes and are returned at full capacity (no bytes are lost for internal management)
	virtual MemoryPage allocatPage(uint64_t size) = 0;

    // Free allocate page
    virtual void freePage(MemoryPage page) = 0;

    //--

    //! query minimum page size (anything smaller will be rounded up)
    virtual uint64_t queryMinimumPageSize() const = 0;

    //! query maximum page size (anything bigger will fail to allocate)
    virtual uint64_t queryMaximumPageSize() const = 0;

    //--
};

//---

END_INFERNO_NAMESPACE()
