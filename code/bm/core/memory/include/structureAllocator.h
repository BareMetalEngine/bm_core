/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

BEGIN_INFERNO_NAMESPACE()

//---

/// Internal state of structure allocator
struct StructureAllocatorState : public MainPoolData<NoCopy>
{
    static BM_CORE_MEMORY_API void CreateState(IPoolUnmanaged* pool, uint32_t pageSize, StructureAllocatorState*& outState);
    static BM_CORE_MEMORY_API void ReleaseState(StructureAllocatorState* state);

    static BM_CORE_MEMORY_API void* AllocElement(StructureAllocatorState* state, uint32_t elemSize, uint32_t elemAlign);
    static BM_CORE_MEMORY_API void FreeElement(StructureAllocatorState* state, void* ptr);

    static BM_CORE_MEMORY_API uint32_t Size(StructureAllocatorState* state);
};

//---

/// A simple pool for structures, pool is meant for high reuse 
/// Elements are organized in blocks each with it's own free list done using bit mask
template< typename T >
class StructureAllocator : public MainPoolData<NoCopy>
{
public:
    static const uint32_t DEFAULT_PAGE_SIZE = 128U << 10;

    INLINE StructureAllocator(uint32_t pageSize = DEFAULT_PAGE_SIZE);
    INLINE StructureAllocator(IPoolUnmanaged& pool, uint32_t elementsPerPage = DEFAULT_PAGE_SIZE);
    INLINE ~StructureAllocator(); // asserts if all elements are not freed

    //--

    INLINE uint32_t size() const { return StructureAllocatorState::Size(m_state); }

    //--

    // construct single object
    template<typename... Args>
    INLINE T* create(Args&&... args);

    // release single element
    INLINE void free(T* ptr);

    //--

private:
    StructureAllocatorState* m_state = nullptr;
};

//---

END_INFERNO_NAMESPACE()

#include "structureAllocator.inl"

