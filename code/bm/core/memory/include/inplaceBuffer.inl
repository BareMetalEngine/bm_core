/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

BEGIN_INFERNO_NAMESPACE()

///--

template< uint32_t InitSize, uint32_t Alignment >
INLINE InplaceBuffer<InitSize, Alignment>::InplaceBuffer(IPoolUnmanaged& pool /*= MainPool()*/)
	: m_pool(pool)
{
	m_start = m_staticBuffer;
	m_pos = m_start;
	m_end = m_start + InitSize;
}

template< uint32_t InitSize, uint32_t Alignment >
INLINE InplaceBuffer<InitSize, Alignment>::~InplaceBuffer()
{
	clear();
}

template< uint32_t InitSize, uint32_t Alignment >
INLINE void InplaceBuffer<InitSize, Alignment>::clear()
{
	if (m_start != m_staticBuffer)
		PoolFree(m_pool, m_start);

	m_start = m_staticBuffer;
	m_pos = m_start;
	m_end = m_start + InitSize;
}

template< uint32_t InitSize, uint32_t Alignment >
INLINE void InplaceBuffer<InitSize, Alignment>::reset()
{
	m_pos = m_start;
}

template< uint32_t InitSize, uint32_t Alignment >
INLINE bool InplaceBuffer<InitSize, Alignment>::doubleCapacity()
{
	auto requiredCapacity = static_cast<uint32_t>(m_capacity * 2);
	return ensureCapacity(requiredCapacity);
}

template< uint32_t InitSize, uint32_t Alignment >
INLINE bool InplaceBuffer<InitSize, Alignment>::ensureCapacity(uint32_t requiredCapacity)
{
	const auto oldSize = size();
	const auto oldCapacity = capacity();
	if (requiredCapacity > capacity())
	{
		// grow in orderly fashion
		auto newCapacity = oldCapacity;
		while (newCapacity < requiredCapacity)
			newCapacity = (newCapacity * 3) / 2;

		// resize the buffer
		if (m_start != m_staticBuffer)
		{
			m_start = (uint8_t*) PoolReallocate(m_pool, m_start, newCapacity, Alignment);
		}
		else
		{
			m_start = (uint8_t*) PoolAllocate(m_pool, newCapacity, Alignment);
			memcpy(m_start, m_staticBuffer, oldCapacity);
		}

		// setup rest of pointer
		m_pos = m_start + oldSize;
		m_end = m_start + newCapacity;
	}

	return true;
}

template< uint32_t InitSize, uint32_t Alignment >
INLINE BufferView InplaceBuffer<InitSize, Alignment>::allocate(uint32_t extraElements)
{
	DEBUG_CHECK_RETURN_EX_V(ensureCapacity(size() + extraElements), "OOM", false);

	auto pos = m_pos;
	m_pos += extraElements;

	return BufferView(pos, pos + extraElements);
}

///--

END_INFERNO_NAMESPACE()
