/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

BEGIN_INFERNO_NAMESPACE()

//--

ALWAYS_INLINE IndexRange::IndexRange(Count count)
    : m_first(0)
    , m_count(count)
{}

ALWAYS_INLINE IndexRange::IndexRange(Index first, Count count)
    : m_first(first)
    , m_count(count)
{}

ALWAYS_INLINE IndexRange::IndexRange(IndexRange&& other)
    : m_first(other.m_first)
    , m_count(other.m_count)
{
    other.m_first = 0;
    other.m_count = 0;
}

ALWAYS_INLINE IndexRange& IndexRange::operator=(IndexRange&& other)
{
    if (this != &other)
    {
        m_first = other.m_first;
        m_count = other.m_count;
        other.m_first = 0;
        other.m_count = 0;
    }

    return *this;
}

//--

ALWAYS_INLINE Count IndexRange::size() const
{
    return m_count;
}

ALWAYS_INLINE bool IndexRange::empty() const
{
    return m_count == 0;
}

ALWAYS_INLINE IndexRange::operator bool() const
{
    return m_count != 0;
}

//--

ALWAYS_INLINE Index IndexRange::first() const
{
    checkNotEmpty();
    return m_first;
}

ALWAYS_INLINE Index IndexRange::last() const
{
    checkNotEmpty();
    return m_first + m_count - 1;
}

ALWAYS_INLINE IndexIterator IndexRange::begin() const
{
    return IndexIterator(m_first, m_first, m_first + m_count - 1);
}

ALWAYS_INLINE IndexIterator IndexRange::end() const
{
    return IndexIterator(m_first + m_count, m_first, m_first + m_count - 1);
}

ALWAYS_INLINE bool IndexRange::contains(Index index) const
{
    return (index != -1) && (index >= m_first) && (index < m_first + m_count);
}

ALWAYS_INLINE uint32_t IndexRange::blockCount(uint32_t blockSize) const
{
    DEBUG_CHECK_RETURN_EX_V(blockSize >= 1, "Invalid block size", 0);
    return (size() + blockSize - 1) / blockSize;
}

ALWAYS_INLINE IndexRange IndexRange::blockRange(uint32_t blockSize, uint32_t index) const
{
    const auto offset = blockSize * index;
    const auto count = m_count - std::min<Index>(offset + blockSize, m_count);
    return IndexRange(m_first + offset, count);
}

ALWAYS_INLINE bool IndexRange::contains(IndexRange range) const
{
    return range.empty() || (contains(range.first()) && contains(range.last()));
}

ALWAYS_INLINE ReversedIndexRange IndexRange::reversed() const
{
    return empty() ? ReversedIndexRange() : ReversedIndexRange(last(), m_count);
}

//--

ALWAYS_INLINE ReversedIndexRange::ReversedIndexRange(Index last, Count count)
    : m_last(last)
    , m_count(count)
{}

ALWAYS_INLINE ReversedIndexRange::ReversedIndexRange(ReversedIndexRange&& other)
    : m_last(other.m_last)
    , m_count(other.m_count)
{
    other.m_last = 0;
    other.m_count = 0;
}

ALWAYS_INLINE ReversedIndexRange& ReversedIndexRange::operator=(ReversedIndexRange&& other)
{
    if (this != &other)
    {
        m_last = other.m_last;
        m_count = other.m_count;
        other.m_last = 0;
        other.m_count = 0;
    }

    return *this;
}

ALWAYS_INLINE Count ReversedIndexRange::size() const
{
    return m_count;
}

ALWAYS_INLINE bool ReversedIndexRange::empty() const
{
    return m_count == 0;
}

ALWAYS_INLINE ReversedIndexRange::operator bool() const
{
    return m_count != 0;
}

//--

ALWAYS_INLINE Index ReversedIndexRange::first() const
{
    checkNotEmpty();
    return m_last - (m_count - 1);
}

ALWAYS_INLINE Index ReversedIndexRange::last() const
{
    checkNotEmpty();
    return m_last;
}

ALWAYS_INLINE bool ReversedIndexRange::contains(Index index) const
{
    return (index != -1) && (index > (m_last - m_count)) && (index <= m_last);
}

ALWAYS_INLINE bool ReversedIndexRange::contains(ReversedIndexRange range) const
{
    return range.empty() || (contains(range.first()) && contains(range.last()));
}

ALWAYS_INLINE IndexRange ReversedIndexRange::reversed() const
{
    return empty() ? IndexRange() : IndexRange(first(), m_count);
}

ALWAYS_INLINE ReversedIndexIterator ReversedIndexRange::begin() const
{
    return ReversedIndexIterator(m_last, m_last, m_last - m_count + 1);
}

ALWAYS_INLINE ReversedIndexIterator ReversedIndexRange::end() const
{
    return ReversedIndexIterator(m_last - m_count, m_last, m_last - m_count + 1);
}

//--

ALWAYS_INLINE DirtyRange::DirtyRange() = default;
ALWAYS_INLINE DirtyRange::DirtyRange(const DirtyRange& other) = default;
ALWAYS_INLINE DirtyRange& DirtyRange::operator=(const DirtyRange& other) = default;

ALWAYS_INLINE DirtyRange::operator bool() const { return indexMax > indexMin; }
ALWAYS_INLINE bool DirtyRange::empty() const { return indexMax <= indexMin; }

ALWAYS_INLINE void DirtyRange::reset()
{
    indexMin = INDEX_MAX;
    indexMax = 0;
}

ALWAYS_INLINE void DirtyRange::add(uint32_t index, uint32_t count)
{
    if (count > 0)
    {
        indexMin = std::min(indexMin, index);
        indexMax = std::max(indexMax, index + count);
    }
}

ALWAYS_INLINE void DirtyRange::add(const DirtyRange& range)
{
    if (range)
    {
        indexMin = std::min<uint32_t>(indexMin, range.indexMin);
        indexMax = std::max<uint32_t>(indexMax, range.indexMax);
    }
}

ALWAYS_INLINE IndexRange DirtyRange::indexRange() const
{
    if (!empty())
        return IndexRange(first(), size());
    else
        return IndexRange();
}

ALWAYS_INLINE uint32_t DirtyRange::first() const
{
    return indexMin;
}

ALWAYS_INLINE uint32_t DirtyRange::size() const
{
    return (indexMax > indexMin) ? (indexMax - indexMin) : 0;
}

//--

END_INFERNO_NAMESPACE()

