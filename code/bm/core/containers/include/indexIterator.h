/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

BEGIN_INFERNO_NAMESPACE()

//--

/// Iterator class for iterating over index ranges FOWARD
class IndexIterator
{
public:
    using iterator_category = std::random_access_iterator_tag;
    using value_type = Index;
    using difference_type = ptrdiff_t;
    using pointer = Index;
    using reference = Index;

    ALWAYS_INLINE IndexIterator() = default;
    ALWAYS_INLINE IndexIterator(const IndexIterator& other) = default;
    ALWAYS_INLINE IndexIterator& operator=(const IndexIterator& other) = default;
    ALWAYS_INLINE IndexIterator(IndexIterator&& other) = default;
    ALWAYS_INLINE IndexIterator& operator=(IndexIterator&& other) = default;
    explicit ALWAYS_INLINE IndexIterator(Index pos, Index first, Index last);

    ALWAYS_INLINE IndexIterator& operator++();
    ALWAYS_INLINE IndexIterator operator++(int);
    ALWAYS_INLINE IndexIterator& operator--();
    ALWAYS_INLINE IndexIterator operator--(int);

    ALWAYS_INLINE IndexIterator& operator+=(std::ptrdiff_t delta);
    ALWAYS_INLINE IndexIterator& operator-=(std::ptrdiff_t delta);

    ALWAYS_INLINE std::ptrdiff_t operator-(IndexIterator other) const;

    ALWAYS_INLINE Index operator*() const;

    ALWAYS_INLINE bool operator==(IndexIterator other) const;
    ALWAYS_INLINE bool operator!=(IndexIterator other) const;
    ALWAYS_INLINE bool operator<(IndexIterator other) const;
    ALWAYS_INLINE bool operator<=(IndexIterator other) const;
    ALWAYS_INLINE bool operator>(IndexIterator other) const;
    ALWAYS_INLINE bool operator>=(IndexIterator other) const;

    ALWAYS_INLINE Index index() const;

    ALWAYS_INLINE bool first() const;
    ALWAYS_INLINE bool last() const;

private:
    Index m_index = -1;
    Index m_first = -1;
    Index m_last = -1;
};

//--

/// Iterator class for iterating over index ranges BACKWARD
class ReversedIndexIterator
{
public:
    using iterator_category = std::random_access_iterator_tag;
    using value_type = Index;
    using difference_type = ptrdiff_t;
    using pointer = Index;
    using reference = Index;

    ALWAYS_INLINE ReversedIndexIterator() = default;
    ALWAYS_INLINE ReversedIndexIterator(const ReversedIndexIterator & other) = default;
    ALWAYS_INLINE ReversedIndexIterator& operator=(const ReversedIndexIterator & other) = default;
    ALWAYS_INLINE ReversedIndexIterator(ReversedIndexIterator && other) = default;
    ALWAYS_INLINE ReversedIndexIterator& operator=(ReversedIndexIterator && other) = default;
    explicit ALWAYS_INLINE ReversedIndexIterator(Index pos, Index first, Index last);

    ALWAYS_INLINE ReversedIndexIterator& operator++();
    ALWAYS_INLINE ReversedIndexIterator operator++(int);
    ALWAYS_INLINE ReversedIndexIterator& operator--();
    ALWAYS_INLINE ReversedIndexIterator operator--(int);

    ALWAYS_INLINE ReversedIndexIterator& operator+=(std::ptrdiff_t delta);
    ALWAYS_INLINE ReversedIndexIterator& operator-=(std::ptrdiff_t delta);

    ALWAYS_INLINE std::ptrdiff_t operator-(ReversedIndexIterator other) const;

    ALWAYS_INLINE Index operator*() const;

    ALWAYS_INLINE bool operator==(ReversedIndexIterator other) const;
    ALWAYS_INLINE bool operator!=(ReversedIndexIterator other) const;
    ALWAYS_INLINE bool operator<(ReversedIndexIterator other) const;
    ALWAYS_INLINE bool operator<=(ReversedIndexIterator other) const;
    ALWAYS_INLINE bool operator>(ReversedIndexIterator other) const;
    ALWAYS_INLINE bool operator>=(ReversedIndexIterator other) const;

    ALWAYS_INLINE Index index() const;

    ALWAYS_INLINE bool first() const;
    ALWAYS_INLINE bool last() const;

private:
    Index m_index = -1;
    Index m_first = -1;
    Index m_last = -1;
};

END_INFERNO_NAMESPACE()

#include "indexIterator.inl"