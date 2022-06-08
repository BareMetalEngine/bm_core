/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

#include "localAllocator.h"

BEGIN_INFERNO_NAMESPACE()

///--

/// Local allocator with some buffer preallocated on stack
template< uint32_t StackSize >
class StackAllocator : public MainPoolData<NoCopy>
{
public:
    StackAllocator();
    StackAllocator(IPoolPaged& pool);
    StackAllocator(IPoolPaged& pool, IPoolUnmanaged& overflowPool);

    //--

    // Allocate memory, larger blocks may be allocated as outstanding allocations
    // NOTE: there's NO GUARANTEE of any continuity between blocks, just they they are allocated fast
    ALWAYS_INLINE void* alloc(uint64_t size, uint32_t align);

	// create a copy of a string and store it locally, useful for parsers to preserve temporary strings
    ALWAYS_INLINE char* strcpy(const char* txt, uint32_t length = INDEX_MAX);

    //--

    // allocate memory for an object, if it requires destruction we also add a proper cleanup function to call later
    template< typename T, typename... Args >
    ALWAYS_INLINE T* createAndRegisterDestroyer(Args && ... args);

    // allocate memory for an object ensure that there's no cleanup
    template< typename T, typename... Args >
    ALWAYS_INLINE T* createWithoutDestruction(Args&& ... args);

    //--

private:
    uint8_t m_buffer[StackSize];

    LocalAllocator m_allocator;
};
    
///--

END_INFERNO_NAMESPACE()

#include "stackAllocator.inl"

