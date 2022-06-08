/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

BEGIN_INFERNO_NAMESPACE()

//--

template< typename T >
ALWAYS_INLINE ArrayView<T>::ArrayView(T* ptr, uint64_t length)
{
    auto* end = ptr + length;

    ASSERT_EX(AlignPtr(ptr, alignof(T)) == ptr, "Input pointer is not aligned");
    ASSERT_EX(end >= ptr, "Reversed points");

    if (ptr && length)
    {
        m_start = ptr;
        m_end = ptr + length;
    }
}

template< typename T >
ALWAYS_INLINE ArrayView<T>::ArrayView(T* start, T* end)
{
    ASSERT_EX(AlignPtr(start, alignof(T)) == start, "Input pointer is not aligned");
    ASSERT_EX(AlignPtr(end, alignof(T)) == end, "Input pointer is not aligned");
    ASSERT_EX(end >= start, "Reversed points");
    if (start && end >= start)
    {
        m_start = start;
        m_end = end;
    }
}

template< typename T >
ALWAYS_INLINE ArrayView<T>::ArrayView(const BufferView& buffer)
{
    auto* start = (T*)buffer.data();
    auto* end = (T*)(buffer.data() + buffer.size());

	ASSERT_EX(AlignPtr(start, alignof(T)) == start, "Input pointer is not aligned");
	ASSERT_EX(AlignPtr(end, alignof(T)) == end, "Input pointer is not aligned");
	ASSERT_EX(end >= start, "Reversed points");
	if (start && end >= start)
	{
		m_start = start;
		m_end = end;
	}
}

template< typename T >
template< uint32_t N >
ALWAYS_INLINE ArrayView<T>::ArrayView(T(&arr)[N])
{
	m_start = &arr[0];
    m_end = &arr[N];
}

template< typename T >
ALWAYS_INLINE ArrayView<T>::ArrayView(const T* ptr, uint64_t length)
{
    if (ptr && length)
    {
        m_start = const_cast<T*>(ptr);
        m_end = m_start + length;
    }
}

template< typename T >
ALWAYS_INLINE ArrayView<T>::ArrayView(const T* start, const T* end)
{
    if (start && end >= start)
    {
        m_start = const_cast<T*>(start);
        m_end = const_cast<T*>(end);
    }
}

template< typename T >
ALWAYS_INLINE ArrayView<T>::ArrayView(const ArrayView<T>& other) = default;

template< typename T >
ALWAYS_INLINE ArrayView<T>::ArrayView(ArrayView<T>&& other)
    : m_start(other.m_start)
    , m_end(other.m_end)
{
    other.m_start = nullptr;
    other.m_end = nullptr;
}

template< typename T >
ALWAYS_INLINE ArrayView<T>& ArrayView<T>::operator=(const ArrayView<T>& other) = default;

template< typename T >
ALWAYS_INLINE ArrayView<T>& ArrayView<T>::operator=(ArrayView<T>&& other)
{
    if (this != &other)
    {
        m_start = other.m_start;
        m_end = other.m_end;
        other.m_start = nullptr;
        other.m_end = nullptr;
    }

    return *this;
}

template< typename T >
ALWAYS_INLINE ArrayView<T>::~ArrayView() = default;

template< typename T >
ALWAYS_INLINE void* ArrayView<T>::data()
{
    return m_start;
}

template< typename T >
ALWAYS_INLINE const void* ArrayView<T>::data() const
{
    return m_start;
}

template< typename T >
ALWAYS_INLINE T* ArrayView<T>::typedData()
{
    return m_start;
}

template< typename T >
ALWAYS_INLINE const T* ArrayView<T>::typedData() const
{
    return m_start;
}

template< typename T >
ALWAYS_INLINE uint64_t ArrayView<T>::dataSize() const
{
    return (m_end - m_start) * sizeof(T);
}

template< typename T >
ALWAYS_INLINE int ArrayView<T>::lastValidIndex() const
{
    return (int)((m_end - m_start) - 1);
}

template< typename T >
ALWAYS_INLINE uint64_t ArrayView<T>::size() const
{
    return m_end - m_start;
}

template< typename T >
ALWAYS_INLINE bool ArrayView<T>::empty() const
{
    return m_end <= m_start;
}

template< typename T >
ALWAYS_INLINE BufferView ArrayView<T>::bufferView()
{
    return BufferView(m_start, m_end);
}

template< typename T >
ALWAYS_INLINE const BufferView ArrayView<T>::bufferView() const
{
    return BufferView(m_start, m_end);
}

template< typename T >
ALWAYS_INLINE IndexRange ArrayView<T>::indexRange() const
{
    return IndexRange(0, size());
}

template< typename T >
ALWAYS_INLINE ArrayView<T>::operator BufferView() const
{
    return BufferView(m_start, m_end);
}

template< typename T >
ALWAYS_INLINE ArrayView<T>::operator bool() const
{
    return !empty();
}

template< typename T >
ALWAYS_INLINE void ArrayView<T>::reset()
{
    m_start = nullptr;
    m_end = nullptr;
}

template< typename T >
ALWAYS_INLINE bool ArrayView<T>::containsRange(BufferView other) const
{
    return other.empty() || (other.data() >= (const uint8_t*)m_start && (other.data() + other.size()) <= (const uint8_t*)m_end);
}

template< typename T >
ALWAYS_INLINE bool ArrayView<T>::containsRange(ArrayView<T> other) const
{
    return other.empty() || (other.m_start >= m_start && other.m_end <= m_end);
}

template< typename T >
ALWAYS_INLINE bool ArrayView<T>::containsPointer(const void* ptr) const
{
    return (uint8_t*)ptr >= (uint8_t*)m_start && (uint8_t*)ptr < (uint8_t*)m_end;
}

template< typename T >
ALWAYS_INLINE T& ArrayView<T>::operator[](Index index)
{
    return m_start[index];
}

template< typename T >
ALWAYS_INLINE const T& ArrayView<T>::operator[](Index index) const
{
    return m_start[index];
}

template< typename T >
ALWAYS_INLINE ArrayView<T> ArrayView<T>::operator[](IndexRange range)
{
    return ArrayView<T>(typedData() + range.first(), range.size());
}

template< typename T >
ALWAYS_INLINE const ArrayView<T> ArrayView<T>::operator[](IndexRange range) const
{
    return ArrayView<T>(typedData() + range.first(), range.size());
}

//--

template< typename T >
ALWAYS_INLINE void ArrayView<T>::destroyElements()
{
    std::destroy_n(m_start, m_end - m_start);
}

template< typename T >
ALWAYS_INLINE void ArrayView<T>::constructElements()
{
    std::uninitialized_default_construct_n(m_start, m_end - m_start);
}

template< typename T >
ALWAYS_INLINE void ArrayView<T>::constructElementsFrom(const T& elementTemplate)
{
    std::uninitialized_fill_n(m_start, m_end - m_start, elementTemplate);
}

template< typename T >
ALWAYS_INLINE void ArrayView<T>::reverseElements()
{
    auto* cur = m_start;
    auto* end = m_end - 1;
    while (cur < end)
    {
        std::swap(*cur, *end);
        cur += 1;
        end -= 1;
    }
}

//--

template< typename T >
template< typename FK >
bool ArrayView<T>::contains(const FK& key) const
{
    auto* cur = m_start;
    while (cur < m_end)
        if (*cur++ == key)
            return true;

    return false;
}

//--

template< typename T >
template< typename FK >
bool ArrayView<T>::findFirst(const FK& key, Index& outFoundIndex) const
{
    auto* cur = m_start + outFoundIndex;
    while (++cur < m_end)
    {
        if (*cur == key)
        {
            outFoundIndex = cur - m_start;
            return true;
        }
    }

    return false;
}

template< typename T >
template< typename FK >
bool ArrayView<T>::findLast(const FK& key, Index& outFoundIndex) const
{
    auto* cur = m_start + outFoundIndex;
    while (--cur >= m_start)
    {
        if (*cur == key)
        {
            outFoundIndex = cur - m_start;
            return true;
        }
    }

    return false;
}

template<typename T>
template< typename FK >
Count ArrayView<T>::replaceAll(const FK& item, const T& itemTemplate)
{
    Count count = 0;
    auto* ptr = m_start;
    while (ptr < m_end)
    {
        if (*ptr == item)
        {
            *ptr = itemTemplate;
            count += 1;
        }
        ++ptr;
    }

    return count;
}

template<typename T>
template< typename FK >
bool ArrayView<T>::replaceFirst(const FK& item, T&& itemTemplate)
{
    auto* ptr = m_start;
    while (ptr < m_end)
    {
        if (*ptr == item)
        {
            *ptr = std::move(itemTemplate);
            return true;
        }
        ++ptr;
    }

    return false;
}

template<typename T>
template< typename FK >
bool ArrayView<T>::replaceFirst(const FK& item, const T& itemTemplate)
{
    auto* ptr = m_start;
    while (ptr < m_end)
    {
        if (*ptr == item)
        {
            *ptr = itemTemplate;
            return true;
        }
        ++ptr;
    }

    return false;

}

template<typename T>
template< typename FK >
bool ArrayView<T>::replaceLast(const FK& item, T&& itemTemplate)
{
    auto* ptr = m_end;
    while (--ptr >= m_start)
    {
        if (*ptr == item)
        {
            *ptr = std::move(itemTemplate);
            return true;
        }
        ++ptr;
    }

    return false;
}

template<typename T>
template< typename FK >
bool ArrayView<T>::replaceLast(const FK& item, const T& itemTemplate)
{
    auto* ptr = m_end;
    while (--ptr >= m_start)
    {
        if (*ptr == item)
        {
            *ptr = itemTemplate;
            return true;
        }
        ++ptr;
    }

    return false;
}

template< typename T >
template< typename FK >
Index ArrayView<T>::lowerBound(const FK& key) const
{
    auto it = std::lower_bound(begin(), end(), key);
    return (it == end()) ? size() : std::distance(begin(), it);
}

template< typename T >
template< typename FK >
Index ArrayView<T>::upperBound(const FK& key) const
{
    auto it = std::upper_bound(begin(), end(), key);
    return (it == end()) ? size() : std::distance(begin(), it);
}

//--

template< typename T >
template< typename Pred >
void ArrayView<T>::sort(Pred pred)
{
    std::sort(begin(), end(), pred);
}

template< typename T >
template< typename Pred >
void ArrayView<T>::stableSort(Pred pred)
{
    std::stable_sort(begin(), end(), pred);
}

template< typename T >
template< typename Pred >
void ArrayView<T>::quickSort(Pred pred)
{
    std::qsort(begin(), end(), pred);
}

//--

template<typename T>
template< typename FK, typename Pred >
bool ArrayView<T>::sortedContains(const FK& element, Pred pred) const
{
	auto it = std::lower_bound(begin(), end(), element, pred);
    return (it != end()) && (*it == element);
}

template<typename T>
template< typename FK, typename Pred >
Index ArrayView<T>::sortedFindFirst(const FK& element, Pred pred) const
{
	auto it = std::lower_bound(begin(), end(), element);
    if ((it != end()) && (*it == element))
        return std::distance(begin(), it);
    return INDEX_NONE;
}

template<typename T>
template< typename FK, typename Pred  >
bool ArrayView<T>::sortedFindFirst(const FK& key, Index& outFoundIndex, Pred pred) const
{
	auto it = std::lower_bound(begin(), end(), element);
	if ((it != end()) && (*it == element))
	{
		outFoundIndex = std::distance(begin(), it);
		return true;
	}

	return false;
}

template<typename T>
template< typename Pred  >
bool ArrayView<T>::checkSortedOrder(Pred pred) const
{
	const auto* ptr = typedData();
	const auto count = size();
	for (uint32_t i = 1; i < count; ++i)
		if ((ptr[i - 1] != ptr[i]) && !pred(ptr[i - 1], ptr[i]))
			return false;


	return true;
}

template<typename T>
template< typename Pred  >
bool ArrayView<T>::checkStrictSortedOrder(Pred pred) const
{
	const auto* ptr = typedData();
	const auto count = size();
	for (uint32_t i = 1; i < count; ++i)
		if (!pred(ptr[i - 1], ptr[i]))
			return false;


	return true;
}
//--

template< typename T >
ALWAYS_INLINE ArrayIterator<T> ArrayView<T>::begin()
{
    return ArrayIterator<T>(m_start);
}

template< typename T >
ALWAYS_INLINE ArrayIterator<T> ArrayView<T>::end()
{
    return ArrayIterator<T>(m_end);
}

template< typename T >
ALWAYS_INLINE ConstArrayIterator<T> ArrayView<T>::begin() const
{
    return ConstArrayIterator<T>(m_start);
}

template< typename T >
ALWAYS_INLINE ConstArrayIterator<T> ArrayView<T>::end() const
{
    return ConstArrayIterator<T>(m_end);
}

//--

template< typename T >
template< typename K >
ALWAYS_INLINE ArrayView<K> ArrayView<T>::cast() const
{
    return ArrayView<K>((K*)m_start, (K*)m_end);
}

//--

END_INFERNO_NAMESPACE()
