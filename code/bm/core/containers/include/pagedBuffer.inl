/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

BEGIN_INFERNO_NAMESPACE()

//---

INLINE void PagedBuffer::writeSmall(const void* data, uint32_t size)
{
	memcpy(allocSmall(size), data, size);
}

//---

template< typename T >
INLINE PagedBufferTyped<T>::PagedBufferTyped()
    : PagedBuffer(alignof(T))
{}

template< typename T >
INLINE PagedBufferTyped<T>::PagedBufferTyped(IPoolPaged& pageAllocator)
	: PagedBuffer(pageAllocator, alignof(T))
{}

template< typename T >
INLINE PagedBufferTyped<T>::~PagedBufferTyped()
{}

template< typename T >
INLINE uint32_t PagedBufferTyped<T>::size() const
{
	return dataSize() / sizeof(T);
}

template< typename T >
INLINE T* PagedBufferTyped<T>::allocSingle()
{
    return (T*)PagedBuffer::allocSmall(sizeof(T));
}

template< typename T >
INLINE T* PagedBufferTyped<T>::allocateBatch(uint32_t elementCount, uint32_t& outNumAllocated)
{
    return (T*)PagedBuffer::allocateBatch(sizeof(T) * elementCount, elementCount, outNumAllocated);
}

template< typename T >
template< typename F >
INLINE void PagedBufferTyped<T>::forEach(const F& func) const
{
    if (auto* page = m_pageHead)
    {
        while (page)
        {
            auto* pagePayload = AlignPtr((char*)page + sizeof(Page), m_alignment);
            std::for_each((const T*)pagePayload, (const T*)(pagePayload + page->dataSize), func);
            page = page->next;
        }
    }

    if (m_writePtr > m_writeStartPtr)
        std::for_each((const T*)m_writeStartPtr, (const T*)m_writePtr, func);
}

//---

END_INFERNO_NAMESPACE()
