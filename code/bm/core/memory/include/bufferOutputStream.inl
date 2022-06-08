/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

BEGIN_INFERNO_NAMESPACE()

//--

template< typename T >
ALWAYS_INLINE BufferOutputStream<T>::BufferOutputStream(T* ptr, uint64_t length)
	: m_start(ptr)
	, m_pos(ptr)
	, m_end(ptr + length)
{
}

template< typename T >
ALWAYS_INLINE BufferOutputStream<T>::BufferOutputStream(T* ptr, const T* ptrEnd)
	: m_start(ptr)
	, m_pos(ptr)
	, m_end(ptrEnd)
{
}

template< typename T >
ALWAYS_INLINE T* BufferOutputStream<T>::start() const
{
	return m_start;
}

template< typename T >
ALWAYS_INLINE T* BufferOutputStream<T>::pos() const
{
	return m_pos;
}

template< typename T >
ALWAYS_INLINE bool BufferOutputStream<T>::empty() const
{
	return m_pos == m_start;
}

template< typename T >
ALWAYS_INLINE BufferOutputStream<T>::operator bool() const
{
	return m_pos > m_start;
}

template< typename T >
ALWAYS_INLINE uint64_t BufferOutputStream<T>::size() const
{
	return m_pos - m_start;
}

template< typename T >
ALWAYS_INLINE uint64_t BufferOutputStream<T>::capacity() const
{
	return m_end - m_start;
}

template< typename T >
ALWAYS_INLINE uint64_t BufferOutputStream<T>::capacityLeft() const
{
	return m_end - m_pos;
}

template< typename T >
ALWAYS_INLINE T* BufferOutputStream<T>::alloc(uint32_t count)
{
	if (m_pos + count <= m_end)
	{
		auto* ret = m_pos;
		m_pos += count;
		return ret;
	}

	return nullptr;
}

template< typename T >
ALWAYS_INLINE bool BufferOutputStream<T>::write(T val)
{
	if (m_pos < m_end)
	{
		*m_pos++ = val;
		return true;
	}

	return false;
}

//--

END_INFERNO_NAMESPACE()

#include "bufferOutputStream.inl"
