/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

BEGIN_INFERNO_NAMESPACE()

template<typename T>
ALWAYS_INLINE Array<T>::Array()
{};

/*template<typename T>
Array<T>::Array(const T* ptr, Count size)
{
	auto elems  = allocateUninitialized(size);
	std::uninitialized_copy_n(ptr, size, elems);
}*/

template<typename T>
Array<T>::Array(BaseArrayBuffer&& buffer)
	: BaseArray(std::move(buffer))
{}

template<typename T>
Array<T>::Array(const Array<T>& other)
{
	auto elems = allocateUninitialized(other.size());
	std::uninitialized_copy_n(other.typedData(), other.size(), elems);
}

template<typename T>
Array<T>::Array(ArrayView<T> other)
{
	auto elems = allocateUninitialized(other.size());
	std::uninitialized_copy_n(other.typedData(), other.size(), elems);
}

template<typename T>
template< uint32_t N >
Array<T>::Array(T(&arr)[N])
{
	auto elems = allocateUninitialized(N);
	std::uninitialized_copy_n(&arr[0], N, elems);
}

template<typename T>
Array<T>::Array(Count size, const T& base /*= T()*/)
{
    auto elems = allocateUninitialized(size);
    std::uninitialized_fill_n(elems, size, base);
}

template<typename T>
Array<T>::Array(Array<T> &&other)
{
    if (other.m_buffer.owned())
    {
        movePointers(std::move(other));
    }
    else
    {
        reserve(other.size());
        moveElements(std::move(other));
    }
}

template<typename T>
Array<T>::Array(std::initializer_list<T> values)
{
    reserve(values.size());

    for (auto& val : values)
        emplaceBack(std::move(val));
}

template<typename T>
Array<T>::~Array()
{
    clear();
}

template<typename T>
ALWAYS_INLINE Count Array<T>::size() const
{
    return BaseArray::size();
}

template<typename T>
ALWAYS_INLINE int Array<T>::lastValidIndex() const
{
    return (int)BaseArray::size() - 1;
}

template<typename T>
ALWAYS_INLINE Count Array<T>::capacity() const
{
    return BaseArray::capacity();
}

template<typename T>
ALWAYS_INLINE void* Array<T>::data()
{
    return BaseArray::data();
}

template<typename T>
ALWAYS_INLINE const void* Array<T>::data() const
{
    return BaseArray::data();
}

template<typename T>
ALWAYS_INLINE uint64_t Array<T>::dataSize() const
{
    return ((uint64_t)size()) * sizeof(T);
}

template<typename T>
ALWAYS_INLINE const T* Array<T>::typedData() const
{
    return (const T*)data();
}

template<typename T>
ALWAYS_INLINE T* Array<T>::typedData()
{
    return (T*)data();
}

template<typename T>
ALWAYS_INLINE bool Array<T>::empty() const
{
    return BaseArray::empty();
}

template<typename T>
ALWAYS_INLINE bool Array<T>::full() const
{
    return BaseArray::full();
}

template<typename T>
ALWAYS_INLINE T& Array<T>::operator[](Index index)
{
    checkIndex(index);
    return typedData()[index];
}

template<typename T>
ALWAYS_INLINE const T& Array<T>::operator[](Index index) const
{
    checkIndex(index);
    return typedData()[index];
}

template<typename T>
ALWAYS_INLINE ArrayView<T> Array<T>::operator[](IndexRange range)
{
    checkIndexRange(range.first(), range.size());
    return ArrayView<T>(typedData() + range.first(), range.size());
}

template<typename T>
ALWAYS_INLINE const ArrayView<T> Array<T>::operator[](IndexRange range) const
{
	checkIndexRange(range.first(), range.size());
	return ArrayView<T>(typedData() + range.first(), range.size());
}

template<typename T>
ALWAYS_INLINE ArrayView<T> Array<T>::view()
{
    return ArrayView<T>(typedData(), size());
}

template<typename T>
ALWAYS_INLINE const ArrayView<T> Array<T>::view() const
{
    return ArrayView<T>(typedData(), size());
}

template<typename T>
ALWAYS_INLINE IndexRange Array<T>::indexRange() const
{
    return IndexRange(0, size());
}

template<typename T>
ALWAYS_INLINE BufferView Array<T>::bufferView() const
{
    return BufferView(data(), (const uint8_t*)data() + dataSize());
}

template<typename T>
ALWAYS_INLINE Array<T>::operator BufferView() const
{
    return bufferView();
}

template<typename T>
ALWAYS_INLINE Array<T>::operator ArrayView<T>() const
{
    return view();
}

template<typename T>
void Array<T>::popBack()
{
    checkIndex(size() - 1);
    changeSize(size() - 1);
    std::destroy_n(typedData() + size(), 1);
}

template<typename T>
bool Array<T>::popBackIfExists(T& outValue)
{
    if (!empty())
    {
        changeSize(size() - 1);
        outValue = std::move(typedData()[size()]);
        std::destroy_n(typedData() + size(), 1);
        return true;
    }
    
    return false;
}

template<typename T>
ALWAYS_INLINE T& Array<T>::back()
{
    checkIndex(size() - 1);
    return typedData()[size() - 1];
}

template<typename T>
ALWAYS_INLINE const T& Array<T>::back() const
{
    checkIndex(size() - 1);
    return typedData()[size() - 1];
}

template<typename T>
ALWAYS_INLINE T& Array<T>::front()
{
    checkIndex(0);
    return typedData()[0];
}

template<typename T>
ALWAYS_INLINE const T& Array<T>::front() const
{
    checkIndex(0);
    return typedData()[0];
}

template<typename T>
void Array<T>::shrink()
{
#ifdef PLATFORM_HASTYPEID
    BaseArray::changeCapacity(size(), sizeof(T) * capacity(), sizeof(T) * size(), __alignof(T), typeid(typename std::remove_cv<T>::type).name());
#else
    BaseArray::changeCapacity(size(), sizeof(T) * capacity(), sizeof(T) * size(), __alignof(T));
#endif
}

template<typename T>
bool Array<T>::resizeWith(Count newSize, const T& elementTemplate)
{
    if (newSize < size())
    {
        std::destroy(begin() + newSize, end());
    }
    else if (newSize > size())
    {
        if (!reserve(newSize))
            return false;

        std::uninitialized_fill_n(begin() + size(), newSize - size(), elementTemplate);
    }

    BaseArray::changeSize(newSize);
    return true;
}

template<typename T>
void Array<T>::changeSizeUnsafe(Count newSize)
{
    BaseArray::changeSize(newSize);
}

template<typename T>
bool Array<T>::resize(Count newSize)
{
    if (newSize < size())
    {
        std::destroy(begin() + newSize, end());
    }
    else if (newSize > size())
    {
        if (!reserve(newSize))
            return false;

        std::uninitialized_default_construct_n(begin() + size(), newSize - size());
    }

    BaseArray::changeSize(newSize);
    return true;
}

template<typename T>
bool Array<T>::prepare(Count minimalSize)
{
    if (minimalSize > size())
    {
        if (!reserve(minimalSize))
            return false;

		std::uninitialized_default_construct_n(begin() + size(), minimalSize - size());
		BaseArray::changeSize(minimalSize);
    }

    return true;
}

template<typename T>
bool Array<T>::prepareWith(Count minimalSize, const T& elementTemplate /*= T()*/)
{
    if (minimalSize > size())
    {
        if (!reserve(minimalSize))
            return false;

        std::uninitialized_fill_n(begin() + size(), minimalSize - size(), elementTemplate);
		BaseArray::changeSize(minimalSize);
	}

    return true;
}

template<typename T>
bool Array<T>::reserve(Count newSize)
{
    if (newSize > capacity())
    {
        const auto requiresBufferSize = sizeof(T) * newSize;
        const auto currentBufferSize = sizeof(T) * capacity();
        auto allocatedBufferSize = currentBufferSize;
        while (allocatedBufferSize < requiresBufferSize)
            allocatedBufferSize = BaseArray::CalcNextBufferSize(allocatedBufferSize);

        auto newCapacity = allocatedBufferSize / sizeof(T);
        ASSERT_EX(newCapacity >= newSize, "Computation overflow");

#ifdef PLATFORM_HASTYPEID
        return BaseArray::changeCapacity(newCapacity, currentBufferSize, allocatedBufferSize, __alignof(T), typeid(typename std::remove_cv<T>::type).name());
#else
        return BaseArray::changeCapacity(newCapacity, currentBufferSize, allocatedBufferSize, __alignof(T));
#endif
    }

    return true;
}

template<typename T>
template<typename FK>
bool Array<T>::contains(const FK& element) const
{
    return view().contains(element);
}

template<typename T>
template<typename FK>
Index Array<T>::find(const FK& element) const
{
    Index ret = -1;
    view().findFirst(element, ret);
    return ret;
}

template<typename T>
template<typename FK>
Index Array<T>::findLast(const FK& element) const
{
    Index ret = size();
    if (!view().findLast(element, ret))
        return Index();
    return ret;
}

template<typename T>
template<typename FK>
bool Array<T>::find(const FK& key, Index& outFoundIndex) const
{
    return view().findFirst(key, outFoundIndex);
}

template<typename T>
template<typename FK>
bool Array<T>::findLast(const FK& key, Index& outFoundIndex) const
{
    return view().findLast(key, outFoundIndex);
}

template<typename T>
void Array<T>::erase(Index index, Count count)
{
    checkIndexRange(index, count);

	std::destroy_n(begin() + index, count);
	memmove(typedData() + index, typedData() + index + count, (size() - (index + count)) * sizeof(T));
        
    BaseArray::changeSize(size() - count);
}

template<typename T>
void Array<T>::eraseUnordered(Index index)
{
    checkIndex(index);

    const auto last = lastValidIndex();
    if (index < last)
        std::swap(typedData()[index], typedData()[last]);

    std::destroy_n(begin() + last, 1);

    BaseArray::changeSize(size() - 1);
}

template<typename T>
void Array<T>::clear()
{
    std::destroy(begin(), end());
    BaseArray::changeSize(0);

#ifdef PLATFORM_HASTYPEID
	BaseArray::changeCapacity(0, capacity() * sizeof(T), 0, __alignof(T), typeid(typename std::remove_cv<T>::type).name());
#else
    BaseArray::changeCapacity(0, capacity() * sizeof(T), 0, __alignof(T));
#endif
}

template<typename T>
void Array<T>::clearPtr()
{
    std::for_each(begin(), end(), [](T& val) { delete val; });
    std::destroy(begin(), end());
    BaseArray::changeSize(0);

#ifdef PLATFORM_HASTYPEID
    BaseArray::changeCapacity(0, capacity() * sizeof(T), 0, __alignof(T), typeid(typename std::remove_cv<T>::type).name());
#else
    BaseArray::changeCapacity(0, capacity() * sizeof(T), 0, __alignof(T));
#endif
}

template<typename T>
void Array<T>::reset()
{
    std::destroy(begin(), end());
    BaseArray::changeSize(0);
}

template<typename T>
Array<T>& Array<T>::operator=(const Array<T> &other)
{
    if (this != &other)
    {
		reset(); // destroy elements without freeing memory
		std::uninitialized_copy_n(other.begin(), other.size(), allocateUninitialized(other.size()));
    }

    return *this;
}

template<typename T>
Array<T>& Array<T>::movePointers(Array<T>&& other)
{
	clear();

    ASSERT(other.m_buffer.owned()); // cannot move buffer that is not owned
	m_buffer = std::move(other.m_buffer);
	m_size = other.m_size;
	other.m_size = 0;

    return *this;
}

template<typename T>
Array<T>& Array<T>::moveElements(Array<T>&& other)
{
    ASSERT(capacity() >= other.size());

    reset();

    std::uninitialized_move_n(other.typedData(), other.size(), typedData());
    changeSizeUnsafe(other.size());

    other.reset(); // keep memory, maybe it will be useful
    return *this;
}

template<typename T>
Array<T>& Array<T>::operator=(Array<T> &&other)
{
    if (this != &other)
    {
        if (other.owned() && owned()) // moving from one heap array to other heap array
        {
            return movePointers(std::move(other));
        }
        else if (other.owned() && !owned()) // move from heap array to inplace array
        {
            if (capacity() <= other.capacity())
                return movePointers(std::move(other)); // if our local capacity is smaller, use the other buffer
            else
                return moveElements(std::move(other)); // if our local capacity is greater no point switching to smaller buffer, move just the elements
        }
        else 
        {
            ASSERT(!other.owned()); // only case that remained - we are moving from inplace array

            reserve(other.size()); // make sure there's space locally as we won't be able to move pointers
            return moveElements(std::move(other)); // if our local capacity is greater no point switching to smaller buffer, move just the elements
        }
    }

    return *this;
}

template<typename T>
bool Array<T>::operator==(const Array<T> &other) const
{
    return std::equal(begin(), end(), other.begin(), other.end());
}

template<typename T>
bool Array<T>::operator!=(const Array<T> &other) const
{
    return !operator==(other);
}

template<typename T>
void Array<T>::pushBack(const T &item)
{
    new (allocateUninitialized(1)) T(item);
}

template<typename T>
void Array<T>::pushBackMany(ArrayView<T> items)
{
    std::uninitialized_copy_n(items.typedData(), items.size(), allocateUninitialized(items.size()));
}

template<typename T>
void Array<T>::emplaceBackMany(Array<T>& items)
{
	std::uninitialized_move_n(items.typedData(), items.size(), allocateUninitialized(items.size()));
    items.reset();
}

template<typename T>
void Array<T>::pushBack(T &&item)
{
    new (allocateUninitialized(1)) T(std::move(item));
}

template<typename T>
template<typename ...Args>
ALWAYS_INLINE T& Array<T>::emplaceBack(Args&&... args)
{
    return *new (allocateUninitialized(1)) T(std::forward<Args>(args)...);
}

template<typename T>
void Array<T>::pushBackUnique(const T &element)
{
    if (!contains(element))
        pushBack(element);
}

template<typename T>
void Array<T>::insert(Index index, const T &item)
{
    insert(index, &item, 1);
}

template<typename T>
void Array<T>::insertWith(Index index, Count count, const T& itemTemplate)
{
    if (count == 0)
        return;

    auto prevSize  = size();
    allocateUninitialized(count);

    // just move the memory, item's MUST be movable like that
    if (index < prevSize)
    {
        auto moveCount  = prevSize - index;
        memmove(typedData() + index + count, typedData() + index, moveCount * sizeof(T));

#ifndef PLATFORM_RELEASE
        memset(typedData() + index, 0xCD, count * sizeof(T));
#endif
    }

    // initialize new items as if the memory was trash
    for (Count i=0; i<count; ++i)
        std::uninitialized_copy_n(&itemTemplate, 1, typedData() + index + i);
}

template<typename T>
void Array<T>::insert(Index index, const T* items, Count count)
{
	if (count == 0)
		return;

	auto prevSize  = size();
	allocateUninitialized(count);

	// just move the memory, item's MUST be movable like that
	if (index < prevSize)
	{
		auto moveCount  = prevSize - index;
		memmove(typedData() + index + count, typedData() + index, moveCount * sizeof(T));

#ifndef PLATFORM_RELEASE
		memset(typedData() + index, 0xCD, count * sizeof(T));
#endif
	}

	// initialize new items as if the memory was trash
	std::uninitialized_copy_n(items, count, typedData() + index);
}

template<typename T>
template<typename FK>
Count Array<T>::removeAll(const FK &item)
{
    auto it  = std::remove(begin(), end(), item);
    if (it == end())
        return 0;

    auto count = std::distance(it, end());

    std::destroy(it, end());
    BaseArray::changeSize(std::distance(begin(), it));
    return count;
}

template<typename T>
template<typename FK>
bool Array<T>::remove(const FK& item)
{
    Index index = -1;
    if (view().findFirst(item, index))
    {
        erase(index);
        return true;
    }

    return false;
}

template<typename T>
template<typename FK>
bool Array<T>::removeLast(const FK& item)
{
    Index index = size();
    if (view().findLast(item, index))
    {
        erase(index);
        return true;
    }

    return false;
}

template<typename T>
template<typename FK>
Count Array<T>::removeUnorderedAll(const FK &item)
{
    Count removed = 0;

    Index cur = -1;
    while (view().findFirst(item, cur))
    {
        eraseUnordered(cur);
        removed += 1;
    }

    return removed;
}

template<typename T>
template<typename FK>
bool Array<T>::removeUnordered(const FK& item)
{
    Index index = -1;
    if (view().findFirst(item, index))
    {
        eraseUnordered(index);
        return true;
    }

    return false;
}

template<typename T>
template<typename FK>
bool Array<T>::removeUnorderedLast(const FK& item)
{
    Index index = size();
    if (view().findLast(item, index))
    {
        eraseUnordered(index);
        return true;
    }

    return false;
}

template<typename T>
template<typename FK>
Count Array<T>::replaceAll(const FK& item, const T& itemTemplate)
{
    return view().replaceAll(item, itemTemplate);
}

template<typename T>
template<typename FK>
bool Array<T>::replace(const FK& item, T&& itemTemplate)
{
    return view().replaceFirst(item, itemTemplate);
}

template<typename T>
template<typename FK>
bool Array<T>::replaceLast(const FK& item, T&& itemTemplate)
{
    return view().replaceLast(item, itemTemplate);
}

template<typename T>
template<typename FK>
bool Array<T>::replace(const FK& item, const T& itemTemplate)
{
    return view().replaceFirst(item, itemTemplate);
}

template<typename T>
template<typename FK>
bool Array<T>::replaceLast(const FK& item, const T& itemTemplate)
{
    return view().replaceLast(item, itemTemplate);
}

template<typename T>
ArrayIterator<T> Array<T>::begin()
{
    return ArrayIterator<T>(typedData());
}

template<typename T>
ArrayIterator<T> Array<T>::end()
{
    return ArrayIterator<T>(typedData() + size());
}

template<typename T>
ConstArrayIterator<T> Array<T>::begin() const
{
    return ConstArrayIterator<T>(typedData());
}

template<typename T>
ConstArrayIterator<T> Array<T>::end() const
{
    return ConstArrayIterator<T>(typedData() + size());
}

template<typename T>
T* Array<T>::allocateUninitialized(Count count)
{
    auto newSize  = size() + count;
    DEBUG_CHECK_RETURN_EX_V(reserve(newSize), "OOM", nullptr);        
    return typedData() + BaseArray::changeSize(newSize);
}

template<typename T>
ArrayView<T> Array<T>::allocateUninitializedView(Count count)
{
    auto ptr = allocateUninitialized(count);
	DEBUG_CHECK_RETURN_EX_V(ptr, "OOM", nullptr);

    return ArrayView<T>(ptr, ptr + count);
}

template<typename T>
T* Array<T>::allocate(Count count)
{
    auto newSize  = size() + count;
    DEBUG_CHECK_RETURN_EX_V(reserve(newSize), "OOM", nullptr);

    std::uninitialized_default_construct_n(typedData() + size(), count);
    return typedData() + BaseArray::changeSize(newSize);
}

template<typename T>
ArrayView<T> Array<T>::allocateView(Count count)
{
	auto ptr = allocate(count);
	DEBUG_CHECK_RETURN_EX_V(ptr, "OOM", nullptr);

	return ArrayView<T>(ptr, ptr + count);
}

template<typename T>
T* Array<T>::allocateWith(Count count, const T& templateElement)
{
    auto newSize  = size() + count;
    DEBUG_CHECK_RETURN_EX_V(reserve(newSize), "OOM", nullptr);

    std::uninitialized_fill_n(typedData() + size(), count, templateElement);
    return typedData() + BaseArray::changeSize(newSize);
}

template<typename T>
ArrayView<T> Array<T>::allocateWithView(Count count, const T& templateElement)
{
	auto ptr = allocateWith(count, templateElement);
	DEBUG_CHECK_RETURN_EX_V(ptr, "OOM", nullptr);

	return ArrayView<T>(ptr, ptr + count);
}

//--

template<typename T>
template< typename Pred >
uint32_t Array<T>::removeDuplicatesAndSort(Pred pred)
{
	auto originalSize = size();

	auto last = std::unique(begin(), end());
    auto removed = std::distance(last, end());
	std::destroy_n(last, removed);

    std::sort(begin(), last);

    auto last2 = std::unique(begin(), last);
    removed += std::distance(last2, end());
    std::destroy_n(last2, removed);
	
    changeSize(size() - removed);
    return removed;
}

template<typename T>
uint32_t Array<T>::sortedArrayRemoveDuplicates()
{
	auto originalSize = size();

	auto last = std::unique(begin(), end());
	auto removed = std::distance(last, end());
	std::destroy_n(last, removed);
	changeSize(originalSize - removed);

	return removed;
}

template<typename T>
template< typename FK, typename Pred >
bool Array<T>::sortedArrayRemove(const FK& element, Pred pred)
{
	auto it = std::lower_bound(begin(), end(), element, pred);
	if (it != end())
	{
		if (*it == element)
		{
			erase(std::distance(begin(), it));
			return true;
		}
	}

	return false;
}

//--

template<typename T>
template< typename Pred >
void Array<T>::sort(Pred pred)
{
    view().sort(pred);
}

template<typename T>
template< typename Pred >
void Array<T>::stableSort(Pred pred)
{
	view().stableSort(pred);
}

template<typename T>
template< typename Pred >
void Array<T>::quickSort(Pred pred)
{
	view().quickSort(pred);
}

//--

template< typename T >
template< typename Pred >
void Array<T>::sortedArrayInsert(const T& elem, Pred pred)
{
	auto index = std::distance(begin(), std::lower_bound(begin(), end(), elem, pred));
	insert(index, elem);
}

template< typename T >
template< typename Pred >
bool Array<T>::sortedArrayInsertUnique(const T& elem, Pred pred)
{
	auto it = std::lower_bound(begin(), end(), elem, pred);
    if (it != end())
    {
        if (*it != elem)
        {
            insert(std::distance(begin(), it), elem);
            return true;
        }
    }
    
    return false;
}

template<typename T>
template< typename FK, typename Pred >
bool Array<T>::sortedArrayContains(const FK& elem, Pred pred) const
{
    return view().sortedContains(elem, pred);
}

template<typename T>
template< typename FK, typename Pred >
Index Array<T>::sortedArrayFindFirst(const FK& elem, Pred pred) const
{
    return view().sortedFindFirst(elem, pred);
}

template<typename T>
template< typename FK, typename Pred  >
bool Array<T>::sortedArrayFindFirst(const FK& elem, Index& outFoundIndex, Pred pred) const
{
    return view().sortedFindFirst(elem, outFoundIndex, pred);
}

//---

END_INFERNO_NAMESPACE()

