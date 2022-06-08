/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

BEGIN_INFERNO_NAMESPACE()

//--

template <typename T, typename Container, typename Compare>
INLINE Heap<T, Container, Compare>::Heap(Container&& baseData)
    : m_values(std::move(baseData))
{
    std::make_heap(m_values.begin(), m_values.end(), Compare());
}

template <typename T, typename Container, typename Compare>
INLINE Heap<T, Container, Compare>::Heap(const Container& baseData)
    : m_values(baseData)
{
    std::make_heap(m_values.begin(), m_values.end(), Compare());
}

template <typename T, typename Container, typename Compare>
INLINE Heap<T, Container, Compare>::Heap(ArrayView<T> data)
	: m_values(data)
{
	std::make_heap(m_values.begin(), m_values.end(), Compare());
}

template <typename T, typename Container, typename Compare>
INLINE bool Heap<T, Container, Compare>::empty() const
{
    return m_values.empty();
}

template <typename T, typename Container, typename Compare>
INLINE uint32_t Heap<T, Container, Compare>::size() const
{
    return m_values.size();
}

template <typename T, typename Container, typename Compare>
INLINE void Heap<T, Container, Compare>::clear()
{
    m_values.clear();
}

template <typename T, typename Container, typename Compare>
INLINE const T& Heap<T, Container, Compare>::front() const
{
    return m_values.front();
}

template <typename T, typename Container, typename Compare>
INLINE void Heap<T, Container, Compare>::push(const T &data)
{
    m_values.pushBack(data);
    std::push_heap(m_values.begin(), m_values.end(), Compare());
}

template <typename T, typename Container, typename Compare>
INLINE void Heap<T, Container, Compare>::pop()
{
    std::pop_heap(m_values.begin(), m_values.end(), Compare());
    m_values.popBack();
}

template <typename T, typename Container, typename Compare>
INLINE ConstArrayIterator<T> Heap<T, Container, Compare>::begin() const
{
	return m_values.begin();
}

template <typename T, typename Container, typename Compare>
INLINE ConstArrayIterator<T> Heap<T, Container, Compare>::end() const
{
	return m_values.end();
}

//--

END_INFERNO_NAMESPACE()
