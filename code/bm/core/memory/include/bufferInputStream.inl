/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

BEGIN_INFERNO_NAMESPACE()

//--

template< typename T >
ALWAYS_INLINE BufferInputStream<T>::BufferInputStream(const T* ptr, uint64_t length)
	: m_start(ptr)
	, m_pos(ptr)
	, m_end(ptr + length)
{	
}

template< typename T >
ALWAYS_INLINE BufferInputStream<T>::BufferInputStream(const T* ptr, const T* ptrEnd)
	: m_start(ptr)
	, m_pos(ptr)
	, m_end(ptrEnd)
{
}

template< typename T >
ALWAYS_INLINE uint64_t BufferInputStream<T>::pos() const
{
	return m_pos - m_start;
}

template< typename T >
ALWAYS_INLINE uint64_t BufferInputStream<T>::size() const
{
	return m_end - m_start;
}

template< typename T >
ALWAYS_INLINE uint64_t BufferInputStream<T>::left() const
{
	return m_end - m_pos;
}

template< typename T >
ALWAYS_INLINE const T* BufferInputStream<T>::peek(uint32_t count)
{
	if (m_pos + count <= m_end)
		return m_pos;
	return nullptr;
}

template< typename T >
ALWAYS_INLINE void BufferInputStream<T>::advance(uint32_t count)
{
	ASSERT(m_pos + count <= m_end);
	m_pos += count;
}

template< typename T >
ALWAYS_INLINE BufferInputStream<T> consume(uint32_t count)
{
	count = std::min<uint64_t>(left(), count);
	auto* ret = m_pos;
	m_pos += count;
	return BufferInputStream<Y>(ret, count);
}

//--	

END_INFERNO_NAMESPACE()
