/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

BEGIN_INFERNO_NAMESPACE()

//--

template< uint32_t MAX, typename TIndex = uint32_t >
INLINE LockLessPoolAllocator<MAX, TIndex>::LockLessPoolAllocator()
{
    reset();
}

template< uint32_t MAX, typename TIndex = uint32_t >
INLINE LockLessPoolAllocator<MAX, TIndex>::~LockLessPoolAllocator()
{}

template< uint32_t MAX, typename TIndex = uint32_t >
INLINE void LockLessPoolAllocator<MAX, TIndex>::reset()
{
    m_popCounter = 0;
    m_pushCounter = MAX - 1;

    for (uint32_t i = 0; i < MAX; ++i)
        m_freeList[i] = (TIndex)i;
    m_freeList[MAX - 1] = SENTINEL;
}

template< uint32_t MAX, typename TIndex = uint32_t >
INLINE TIndex LockLessPoolAllocator<MAX, TIndex>::allocEntry()
{
    std::atomic_thread_fence(std::memory_order_acquire); // aquire needed for the current state
    auto position = m_popCounter++ % MAX;
    ASSERT_EX(m_freeList[position] != SENTINEL, "All elements from lock less pool were consumed");
    auto index = m_freeList[position];
    m_freeList[position] = SENTINEL;
    std::atomic_thread_fence(std::memory_order_release); // release needed for the updated state
    return index;
}

template< uint32_t MAX, typename TIndex = uint32_t >
INLINE void LockLessPoolAllocator<MAX, TIndex>::freeEntry(TIndex index)
{
    std::atomic_thread_fence(std::memory_order_acquire); // aquire needed for the current state
    auto position = m_pushCounter++ % MAX; // NOTE: we don't care if we support picking up the MAX-1 element, working at 99% of capacity is not a feature if this LockLessPool anyway
    ASSERT_EX(m_freeList[position] == SENTINEL, "Free list corruption - pushed to many elements");
    m_freeList[position] = index;
    std::atomic_thread_fence(std::memory_order_release);// release needed for the updated state
}
    
//--

/// Lock less pool of static size for objects 
template< typename T, uint32_t MAX, typename TIndex = uint32_t >
INLINE LockLessPool<T, MAX, TIndex>::LockLessPool()
{}


template< typename T, uint32_t MAX, typename TIndex = uint32_t >
INLINE LockLessPool<T, MAX, TIndex>::~LockLessPool()
{
}

template< typename T, uint32_t MAX, typename TIndex = uint32_t >
INLINE T* LockLessPool<T, MAX, TIndex>::alloc()
{
    auto index = m_indices.allocEntry();
    auto* mem = m_storage + (index * sizeof(T));
    return new(mem) T();
}

template< typename T, uint32_t MAX, typename TIndex = uint32_t >
INLINE void LockLessPool<T, MAX, TIndex>::free(T* ptr)
{
    ptr->~T();
    auto index = ptr - (T*)m_storage;
    m_indices.freeEntry(index);
}

//--

END_INFERNO_NAMESPACE()
