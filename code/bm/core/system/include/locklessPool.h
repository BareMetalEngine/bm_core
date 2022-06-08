/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

BEGIN_INFERNO_NAMESPACE()

//--

/// NOTE: will fatal assert if limits are exceeded
template< uint32_t MAX, typename TIndex = uint32_t >
class LockLessPoolAllocator : public NoCopy
{
public:
    INLINE LockLessPoolAllocator();
    INLINE ~LockLessPoolAllocator();

    //--

    // reset state
    INLINE void reset();

    // allocate next free entry
    INLINE TIndex allocEntry();

    // return entry to pool
    INLINE void freeEntry(TIndex index);

    //--

private:
    static const auto SENTINEL = std::numeric_limits<TIndex>::max();

    std::atomic<uint32_t> m_pushCounter;
    std::atomic<uint32_t> m_popCounter;
    TIndex m_freeList[MAX];
};

//--

/// Lock less pool of static size for objects 
template< typename T, uint32_t MAX, typename TIndex = uint32_t >
class LockLessPool : public NoCopy
{
public:
    INLINE LockLessPool();
    INLINE ~LockLessPool();

    //--

    // allocate next free entry (calls constructor)
    INLINE T* alloc();

    // deallocate entry (calls destructor)
    INLINE void free(T* ptr);

    //--

private:
    alignas(alignof(T)) uint8_t m_storage[MAX * sizeof(T)];
    LockLessPoolAllocator<MAX, TIndex> m_indices;
};

//--

END_INFERNO_NAMESPACE()

#include "locklessPool.inl"