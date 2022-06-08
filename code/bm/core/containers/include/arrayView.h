/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

#include "bm/core/memory/include/bufferView.h"
#include "baseArray.h"
#include "indexRange.h"

BEGIN_INFERNO_NAMESPACE()

//--

/// Typed pointer range
template< typename T >
class ArrayView
{
public:
    ALWAYS_INLINE ArrayView() = default;
    ALWAYS_INLINE ArrayView(std::nullptr_t) {}
    ALWAYS_INLINE ArrayView(T* ptr, uint64_t length);
    ALWAYS_INLINE ArrayView(T* start, T* end);
    ALWAYS_INLINE ArrayView(const BufferView& buffer);
    ALWAYS_INLINE ArrayView(const T* ptr, uint64_t length);
    ALWAYS_INLINE ArrayView(const T* start, const T* end);
    ALWAYS_INLINE ArrayView(const ArrayView& other);
    ALWAYS_INLINE ArrayView(ArrayView&& other);
    ALWAYS_INLINE ArrayView& operator=(const ArrayView& other);
    ALWAYS_INLINE ArrayView& operator=(ArrayView&& other);
    ALWAYS_INLINE ~ArrayView();

	template< uint32_t N >
	ALWAYS_INLINE ArrayView(T(&arr)[N]);

    //--

    //! get internal data buffer
    ALWAYS_INLINE void* data();

    //! get internal data buffer (read-only)
    ALWAYS_INLINE const void* data() const;

    //! get TYPED data buffer
    ALWAYS_INLINE T* typedData();

    //! get internal data buffer (read-only)
    ALWAYS_INLINE const T* typedData() const;

    //! returns number of bytes in the pointer range
    ALWAYS_INLINE uint64_t dataSize() const;

    //! returns number of ELEMENTS in the pointer range
    ALWAYS_INLINE uint64_t size() const;

    //! last valid index for this range, -1 for empty range
    ALWAYS_INLINE int lastValidIndex() const;

    //! returns true if the range is empty
    ALWAYS_INLINE bool empty() const;

    //! get the base (untyped) pointer range
    ALWAYS_INLINE BufferView bufferView();

    //! get the base (untyped) pointer range
    ALWAYS_INLINE const BufferView bufferView() const;

    //! boolean-check, true if we are not an empty range
    ALWAYS_INLINE operator bool() const;

	//! auto cast to buffer view for memory operations
	ALWAYS_INLINE operator BufferView() const;
    
    //! get index range for this array view (NOTE: will be based on zero)
    ALWAYS_INLINE IndexRange indexRange() const;


    //--

    //! reset the range to empty range
    ALWAYS_INLINE void reset();

    //! check if this memory range contains other memory range
    ALWAYS_INLINE bool containsRange(BufferView other) const;

    //! check if this memory range contains other memory range
    ALWAYS_INLINE bool containsRange(ArrayView<T> other) const;

    //! check if this memory range contains given pointer
    ALWAYS_INLINE bool containsPointer(const void* ptr) const;

    //--

    //! get indexed element, no checks
    ALWAYS_INLINE T& operator[](Index index);

    //! get indexed element, no checks
    ALWAYS_INLINE const T& operator[](Index index) const;

    //--

    //! Sub array view for index range
    ALWAYS_INLINE ArrayView<T> operator[](IndexRange range);

	//! Sub array view for index range
	ALWAYS_INLINE const ArrayView<T> operator[](IndexRange range) const;

    //--

    //! call destructor on all elements in the range
    void destroyElements();

    //! default-construct all elements
    void constructElements();

    //! copy-construct all elements from a template
    void constructElementsFrom(const T& elementTemplate);

    //! reverse the elements in the range
    void reverseElements();

    //--

    //! check if the pointer range contains given element value
    template< typename FK >
    bool contains(const FK& key) const;

    //! find first occurrence of element in the range, returns true if index was found
    //! NOTE: valid initial value for the index is -1
    template< typename FK >
    bool findFirst(const FK& key, Index& outFoundIndex) const;

    //! find last occurrence of element in the range, returns true if index was found
    //! NOTE: valid initial values for the index is size()
    template< typename FK >
    bool findLast(const FK& key, Index& outFoundIndex) const;

    //--

    //! replace all existing occurrences of given element with element template, returns number of elements replaced
    template< typename FK >
    Count replaceAll(const FK& item, const T& itemTemplate);

    //! replace FIRST occurrence of given element with element template, returns true if element was replaced, element can be std::moved
    template< typename FK >
    bool replaceFirst(const FK& item, T&& itemTemplate);

    //! replace LAST occurrence of given element with element template, returns true if element was replaced, element can be std::moved
    template< typename FK >
    bool replaceLast(const FK& item, T&& itemTemplate);


    //! replace FIRST occurrence of given element with element template, returns true if element was replaced, element can be std::moved
    template< typename FK >
    bool replaceFirst(const FK& item, const T& itemTemplate);

    //! replace LAST occurrence of given element with element template, returns true if element was replaced, element can be std::moved
    template< typename FK >
    bool replaceLast(const FK& item, const T& itemTemplate);

    //--

    //! find lower_bound insertion point
    //! NOTE: data must be sorted
    template< typename FK >
    Index lowerBound(const FK& key) const;

    //! find upper_bound insertion point
    //! NOTE: data must be sorted
    template< typename FK >
    Index upperBound(const FK& key) const;

    //--
    
	//! sort elements of this view
    template< typename Pred = std::less<T> >
	void sort(Pred pred = std::less<T>);

	//! stable elements of this view
    template< typename Pred = std::less<T> >
	void stableSort(Pred pred = std::less<T>);

	//! quick elements of this view
	template< typename Pred = std::less<T> >
	void quickSort(Pred pred = std::less<T>);

    //--

    //! check if array is sorted, duplicated elements are allowed
	template< typename Pred = std::less<T> >
	bool checkSortedOrder(Pred pred = std::less<T>()) const;

	//! check if array is sorted, does not allow for duplicates
	template< typename Pred = std::less<T> >
	bool checkStrictSortedOrder(Pred pred = std::less<T>()) const;

	//--
    
	//! Check if array contains given element assuming array is sorted O(log N)
	template< typename FK, typename Pred = std::less<T> >
	bool sortedContains(const FK& element, Pred pred = std::less<T>()) const;

	//! Find index of first element in the array matching given key
	template< typename FK, typename Pred = std::less<T> >
	Index sortedFindFirst(const FK& element, Pred pred = std::less<T>()) const;

	//! find first occurrence of element in the range, returns true if index was found
	template< typename FK, typename Pred = std::less<T> >
	bool sortedFindFirst(const FK& key, Index& outFoundIndex, Pred pred = std::less<T>()) const;

    //--

    //! Get iterator to start of the array
    ALWAYS_INLINE ArrayIterator<T> begin();

    //! Get iterator to end of the array
    ALWAYS_INLINE ArrayIterator<T> end();

    //! Get read only iterator to start of the array
    ALWAYS_INLINE ConstArrayIterator<T> begin() const;

    //! Get read only iterator to end of the array
    ALWAYS_INLINE ConstArrayIterator<T> end() const;
        
    //--

    //! "cast to" other type, we must be mega sure about it
    template< typename K >
    ALWAYS_INLINE ArrayView<K> cast() const;

    //--

protected:
    T* m_start = nullptr;
    T* m_end = nullptr;
};

//--

END_INFERNO_NAMESPACE()

#include "arrayView.inl"