/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

BEGIN_INFERNO_NAMESPACE()

///--

template< uint32_t StackSize >
ALWAYS_INLINE StackAllocator<StackSize>::StackAllocator()
    : m_allocator(m_buffer, StackSize)
{}

template< uint32_t StackSize >
ALWAYS_INLINE StackAllocator<StackSize>::StackAllocator(IPoolPaged& pool)
	: m_allocator(pool, m_buffer, StackSize)
{}

template< uint32_t StackSize >
ALWAYS_INLINE StackAllocator<StackSize>::StackAllocator(IPoolPaged& pool, IPoolUnmanaged& overflowPool)
	: m_allocator(pool, overflowPool, m_buffer, StackSize)
{}

template< uint32_t StackSize >
ALWAYS_INLINE void* StackAllocator<StackSize>::alloc(uint64_t size, uint32_t align)
{
    return m_allocator.alloc(size, align);
}

template< uint32_t StackSize >
ALWAYS_INLINE char* StackAllocator<StackSize>::strcpy(const char* txt, uint32_t length)
{
    return m_allocator.strcpy(txt, length);
}

template< uint32_t StackSize >
template< typename T, typename... Args >
ALWAYS_INLINE T* StackAllocator<StackSize>::createAndRegisterDestroyer(Args && ... args)
{
    return m_allocator.createAndRegisterDestroyer(std::forward< Args >(args)...);
}

template< uint32_t StackSize >
template< typename T, typename... Args >
ALWAYS_INLINE T* StackAllocator<StackSize>::createWithoutDestruction(Args&& ... args)
{
    return m_allocator.createWithoutDestruction(std::forward< Args >(args)...);
}
    
///--

END_INFERNO_NAMESPACE()
