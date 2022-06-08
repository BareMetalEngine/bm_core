/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

BEGIN_INFERNO_NAMESPACE()

//--

ALWAYS_INLINE IndexIterator::IndexIterator(Index pos, Index first, Index last)
    : m_index(pos)
    , m_first(first)
    , m_last(last)
{}

ALWAYS_INLINE IndexIterator& IndexIterator::operator++()
{
    m_index += 1;
    return *this;
}

ALWAYS_INLINE IndexIterator IndexIterator::operator++(int)
{
    auto index = m_index;
    m_index += 1;
    return IndexIterator(index, m_first, m_last);
}

ALWAYS_INLINE IndexIterator& IndexIterator::operator--()
{
    m_index -= 1;
    return *this;
}

ALWAYS_INLINE IndexIterator IndexIterator::operator--(int)
{
    auto index = m_index;
    m_index -= 1;
    return IndexIterator(index, m_first, m_last);
}

ALWAYS_INLINE IndexIterator& IndexIterator::operator+=(std::ptrdiff_t delta)
{
    m_index += delta;
    return *this;
}

ALWAYS_INLINE IndexIterator& IndexIterator::operator-=(std::ptrdiff_t delta)
{
    m_index -= delta;
    return *this;
}

ALWAYS_INLINE std::ptrdiff_t IndexIterator::operator-(IndexIterator other) const
{
    return other.m_index - m_index;
}

ALWAYS_INLINE Index IndexIterator::operator*() const
{
    return m_index;
}

ALWAYS_INLINE bool IndexIterator::operator==(IndexIterator other) const
{
    return m_index == other.m_index;
}

ALWAYS_INLINE bool IndexIterator::operator!=(IndexIterator other) const
{
    return m_index != other.m_index;
}

ALWAYS_INLINE bool IndexIterator::operator<(IndexIterator other) const
{
    return m_index < other.m_index;
}

ALWAYS_INLINE bool IndexIterator::operator<=(IndexIterator other) const
{
    return m_index <= other.m_index;
}

ALWAYS_INLINE bool IndexIterator::operator>(IndexIterator other) const
{
    return m_index > other.m_index;
}

ALWAYS_INLINE bool IndexIterator::operator>=(IndexIterator other) const
{
    return m_index >= other.m_index;
}

ALWAYS_INLINE Index IndexIterator::index() const
{
    return m_index;
}

ALWAYS_INLINE bool IndexIterator::first() const
{
    return m_index == m_first;
}

ALWAYS_INLINE bool IndexIterator::last() const
{
    return m_index == m_last;
}

//--

ALWAYS_INLINE ReversedIndexIterator::ReversedIndexIterator(Index pos, Index first, Index last)
    : m_index(pos)
    , m_first(first)
    , m_last(last)
{}

ALWAYS_INLINE ReversedIndexIterator& ReversedIndexIterator::operator++()
{
    m_index -= 1;
    return *this;
}

ALWAYS_INLINE ReversedIndexIterator ReversedIndexIterator::operator++(int)
{
    auto index = m_index;
    m_index -= 1;
    return ReversedIndexIterator(index, m_first, m_last);
}

ALWAYS_INLINE ReversedIndexIterator& ReversedIndexIterator::operator--()
{
    m_index += 1;
    return *this;
}

ALWAYS_INLINE ReversedIndexIterator ReversedIndexIterator::operator--(int)
{
    auto index = m_index;
    m_index += 1;
    return ReversedIndexIterator(index, m_first, m_last);
}

ALWAYS_INLINE ReversedIndexIterator& ReversedIndexIterator::operator+=(std::ptrdiff_t delta)
{
    m_index -= delta;
    return *this;
}

ALWAYS_INLINE ReversedIndexIterator& ReversedIndexIterator::operator-=(std::ptrdiff_t delta)
{
    m_index += delta;
    return *this;
}

ALWAYS_INLINE std::ptrdiff_t ReversedIndexIterator::operator-(ReversedIndexIterator other) const
{
    return other.m_index - m_index;
}

ALWAYS_INLINE Index ReversedIndexIterator::operator*() const
{
    return m_index;
}

ALWAYS_INLINE bool ReversedIndexIterator::operator==(ReversedIndexIterator other) const
{
    return m_index == other.m_index;
}

ALWAYS_INLINE bool ReversedIndexIterator::operator!=(ReversedIndexIterator other) const
{
    return m_index != other.m_index;
}

ALWAYS_INLINE bool ReversedIndexIterator::operator<(ReversedIndexIterator other) const
{
    return m_index > other.m_index;
}

ALWAYS_INLINE bool ReversedIndexIterator::operator<=(ReversedIndexIterator other) const
{
    return m_index >= other.m_index;
}

ALWAYS_INLINE bool ReversedIndexIterator::operator>(ReversedIndexIterator other) const
{
    return m_index < other.m_index;
}

ALWAYS_INLINE bool ReversedIndexIterator::operator>=(ReversedIndexIterator other) const
{
    return m_index <= other.m_index;
}

ALWAYS_INLINE Index ReversedIndexIterator::index() const
{
    return m_index;
}

ALWAYS_INLINE bool ReversedIndexIterator::first() const
{
    return m_index == m_first;
}

ALWAYS_INLINE bool ReversedIndexIterator::last() const
{
    return m_index == m_last;
}

//--

END_INFERNO_NAMESPACE()
