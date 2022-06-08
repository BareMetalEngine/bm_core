/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

BEGIN_INFERNO_NAMESPACE()

//---

/// a simple pool for structures
template<typename T>
INLINE StructureAllocator<T>::StructureAllocator(uint32_t pageSize)
{
	ASSERT_EX(pageSize >= 32 * sizeof(T), "Page size is to small");
	StructureAllocatorState::CreateState(nullptr, pageSize, m_state);
}

template<typename T>
INLINE StructureAllocator<T>::StructureAllocator(IPoolUnmanaged& pool, uint32_t pageSize)
{
	ASSERT_EX(pageSize >= 32 * sizeof(T), "Page size is to small");
	StructureAllocatorState::CreateState(&pool, pageSize, m_state);
}

template<typename T>
INLINE StructureAllocator<T>::~StructureAllocator()
{
	StructureAllocatorState::ReleaseState(m_state);
}

template<typename T>
template<typename... Args >
INLINE T* StructureAllocator<T>::create(Args&& ... args)
{
	void* mem = StructureAllocatorState::AllocElement(m_state, sizeof(T), alignof(T));
	return new (mem) T(std::forward< Args >(args)...);
}

template<typename T>
INLINE void StructureAllocator<T>::free(T* ptr)
{
	((T*)ptr)->~T();
	StructureAllocatorState::FreeElement(m_state, ptr);
}

//---

END_INFERNO_NAMESPACE()
