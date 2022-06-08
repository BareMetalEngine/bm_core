/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

#include "arrayIterator.h"
#include "indexRange.h"
#include "arrayView.h"
#include "baseArray.h"

#include "bm/core/memory/include/bufferView.h"

BEGIN_INFERNO_NAMESPACE()

enum EInplaceArrayData { InplaceArrayData };

/// Dynamic array class
/*
    A template version of dynamic array that can hold elements of type T.
        
    This array DOES NOT behave in the same way as the std::vector do. The main difference
    is items internally are held in a void* array and the interpretation is done as needed.
    This allows the type of the element to be forward declared.

    Array buffer is also allocated using normal memory operations (realloc) instead of using
    new/delete. Array elements are constructed using the placement new when needed - ie. when they are added to the array.
*/
template < class T >
class Array : protected BaseArray
{
public:
	//static const uint32_t ELEMENT_ALIGNMENT = alignof(T);
    //static const uint32_t ELEMENT_SIZE = sizeof(T);

    ALWAYS_INLINE Array();
    explicit Array(ArrayView<T> other); // copy data from view, items are not moved
    Array(const Array<T>& other); // copy data from view, items are not moved
    Array(BaseArrayBuffer&& buffer); // inplace array, use given buffer as storage
    explicit Array(Count size, const T& base = T()); // allocate with given size, initialize all elements from template
    Array(Array<T>&& other); // move
    Array(std::initializer_list<T> items); // move items
    ~Array();

	//--

	template< uint32_t N >
	Array(T(&arr)[N]);

    //---

    Array<T>& operator=(const Array<T>& other);
    Array<T>& operator=(Array<T>&& other);

    //--

    //! returns true if array is empty (no elements)
    ALWAYS_INLINE bool empty() const;

    //! returns true if array is full (size() == capacity(), next push will resize)
    ALWAYS_INLINE bool full() const;

    //! returns the number of elements in the array
    ALWAYS_INLINE Count size() const;

    //! returns UNSIGNED maximum number of elements the array can hold before resizing
    ALWAYS_INLINE Count capacity() const;

    //! returns SIGNED last valid element index of the array, can be used for reverse iteration
    ALWAYS_INLINE Index lastValidIndex() const; // size - 1

    //! get untyped pointer to array storage
    ALWAYS_INLINE void* data();

    //! get untyped read-only pointer to array storage
    ALWAYS_INLINE const void* data() const;

    //! get total size of data held by this array (in bytes)
    ALWAYS_INLINE uint64_t dataSize() const;

    //! get typed read-only pointer to array storage
    ALWAYS_INLINE const T* typedData() const;

    //! get type pointer to array storage
    ALWAYS_INLINE T* typedData();

    //! get last element of array, illegal if array is empty
    ALWAYS_INLINE T& back();

    //! get last element of array, illegal if array is empty
    ALWAYS_INLINE const T& back() const;

    //! get last element of array, illegal if array is empty
    ALWAYS_INLINE T& front();

    //!gGet last element of array, illegal if array is empty
    ALWAYS_INLINE const T& front() const;

    //! access array element, asserts if index is invalid
    //! NOTE: provides basic index check if not in Release build
    ALWAYS_INLINE T& operator[](Index index);

    //! access array element, asserts if index is invalid
    //! NOTE: provides basic index check if not in Release build
    ALWAYS_INLINE const T &operator[](Index index) const;

    //! Sub array view for index range
	ALWAYS_INLINE ArrayView<T> operator[](IndexRange range);

	//! Sub array view for index range
	ALWAYS_INLINE const ArrayView<T> operator[](IndexRange range) const;

    //! get the pointer range for the array
    ALWAYS_INLINE ArrayView<T> view();

    //! get the pointer range for the array
    ALWAYS_INLINE const ArrayView<T> view() const;

    //! get range of valid indices in the array
    ALWAYS_INLINE IndexRange indexRange() const;

    //! get raw array bytes
    ALWAYS_INLINE BufferView bufferView() const;

	//! auto cast to buffer view
	ALWAYS_INLINE operator BufferView() const;

	//! auto cast to array view
	ALWAYS_INLINE operator ArrayView<T>() const;

    ///-

    //! allocate new-initialized (default constructed) elements
    T* allocate(Count count);

	//! allocate new-initialized (default constructed) elements
	ArrayView<T> allocateView(Count count);

    //! allocate new elements initialized with copy constructor from the template
    T* allocateWith(Count count, const T& templateElement);

	//! allocate new elements initialized with copy constructor from the template
	ArrayView<T> allocateWithView(Count count, const T& templateElement);

    //! allocate memory for new elements but do not initialize anything, it's up to the caller to do so
    //! NOTE: this is the fastest way to grow a vector, should be used when adding bulk of POD data to the array
    T* allocateUninitialized(Count count);

	//! allocate memory for new elements but do not initialize anything, it's up to the caller to do so
	//! NOTE: this is the fastest way to grow a vector, should be used when adding bulk of POD data to the array
	ArrayView<T> allocateUninitializedView(Count count);

    //! add a single element at the end of the array and initialize it with copy constructor from provided template
    //! NOTE: is illegal to add element from the same array (arr.pushBack(arr[5]))
    void pushBack(const T &item);

    //! add a single element at the end of the array and initialize it with move constructor from provided template potentially destroying the source element
    //! NOTE: is illegal to add element from the same array (arr.pushBack(arr[5]))
    void pushBack(T &&item);

    //! push back many elements, the great thing is that it this function reserves memory only once and can use fast copy, great to merge POD arrays
    void pushBackMany(ArrayView<T> items);

    //! add element at the end of the array but only if it's not already added
    //! NOTE: this is a convenience function, not for serious use
    void pushBackUnique(const T &element);

    //! add a single element at the end of the array and construct it directly
    template<typename ...Args>
    ALWAYS_INLINE T& emplaceBack(Args&&... args);

	//! emplace all elements from one array into another ("steal" them)
	void emplaceBackMany(Array<T>& other);

    //! insert a single element at given index in the array, the new elements becomes the one at specified index
    void insert(Index index, const T &item);

    //! insert copy of multiple elements at given index in the array
    void insert(Index index, const T* items, Count count);

    //! insert copy of multiple elements at given index in the array
    void insertWith(Index index, Count count, const T& itemTemplate);
    
    //--

    //! assuming the array is sorted insert element into it
    void sortedInsert(T& item);

    //--

    //! remove last element from array, illegal if array is empty
    void popBack();

    //! remove last element from array, return it's value (if it exists)
    bool popBackIfExists(T& outValue);

    //! remove (stable) elements at given index from the array, order is preserved
    void erase(Index index, Count count = 1);

    //! remove elements at given index from the array, order is not preserved but this is faster as this moves elements from the end of the array to fill the space
    void eraseUnordered(Index index);

    //! find (via linear search) and remove FIRST matching element from the array, returns true if element was removed
    template< typename FK >
    bool remove(const FK& item);

    //! find (via linear search) and remove LAST matching element from the array, returns true if element was removed
    template< typename FK >
    bool removeLast(const FK& item);

    //! find (via linear search) and remove all matching elements from the array, returns number of elements removed, order of elements is preserved
    template< typename FK >
    Count removeAll(const FK& item);

    //! find (via linear search) and remove FIRST matching elements from the array, order of elements is NOT preserved but the function is faster
    template< typename FK >
    bool removeUnordered(const FK& item);

    //! find (via linear search) and remove LAST matching elements from the array, order of elements is NOT preserved but the function is faster
    template< typename FK >
    bool removeUnorderedLast(const FK& item);

    //! find (via linear search) and remove all matching elements from the array, order of elements is NOT preserved but the function is faster
    template< typename FK >
    Count removeUnorderedAll(const FK& item);

    //! replace FIRST occurrence of given element with element template, returns true if element was replaced, element can be std::moved
    template< typename FK >
    bool replace(const FK& item, T&& itemTemplate);

    //! replace LAST occurrence of given element with element template, returns true if element was replaced, element can be std::moved
    template< typename FK >
    bool replaceLast(const FK& item, T&& itemTemplate);

    //! replace FIRST occurrence of given element with element template, returns true if element was replaced, element can be std::moved
    template< typename FK >
    bool replace(const FK& item, const T& itemTemplate);

    //! replace LAST occurrence of given element with element template, returns true if element was replaced, element can be std::moved
    template< typename FK >
    bool replaceLast(const FK& item, const T& itemTemplate);

    //! replace all existing occurrences of given element with element template, returns number of elements replaced
    template< typename FK >
    Count replaceAll(const FK& item, const T& itemTemplate);

    //--

    //! Check if array contains given element (linear search)
    template< typename FK >
    bool contains(const FK& element) const;

    //! Find index of first element in the array matching given key
    template< typename FK >
    Index find(const FK& element) const;

    //! Find index of last element in the array matching given key
    template< typename FK >
    Index findLast(const FK& func) const;

    //! find first occurrence of element in the range, returns true if index was found
    template< typename FK >
    bool find(const FK& key, Index& outFoundIndex) const;

    //! find last occurrence of element in the range, returns true if index was found
    //! NOTE: valid initial values for the index is size()
    template< typename FK >
    bool findLast(const FK& key, Index& outFoundIndex) const;

    //--

    //! remove all duplicated elements, much faster if array is sorted
	template< typename Pred = std::less<T> >
	uint32_t removeDuplicatesAndSort(Pred pred = std::less<T>());

    //--

    //! Get iterator to start of the array
    ArrayIterator<T> begin();

    //! Get iterator to end of the array
    ArrayIterator<T> end();

    //! Get read only iterator to start of the array
    ConstArrayIterator<T> begin() const;

    //! Get read only iterator to end of the array
    ConstArrayIterator<T> end() const;

    //--

    //! reset array count to zero, destroys existing elements but does not free memory
    void reset();

    //! clear array, destroy all elements, release all memory
    void clear();

    //! clear pointer array, call delete on all members, release all memory
    void clearPtr();

    //! reduce array memory buffer to fit only actual array size
    void shrink();

    //! reserve space in array for AT LEAST that many elements, does reduce size, does not call constructor
    //! //! NOTE: it can fail during OOM and it's possible or meaningful it should be handled
    bool reserve(Count  newSize);

    //! resize array to EXACTLY the specific size, call constructors/destructor on elements accordingly, resizes memory block, new elements are initialized with default constructor
    //! NOTE: it can fail during OOM and it's possible or meaningful it should be handled
    bool resize(Count  newSize);

    //! resize array to specific size, if any new elements are created initialize them with the template
    //! NOTE: it can fail during OOM and it's possible or meaningful it should be handled
    bool resizeWith(Count newSize, const T& elementTemplate = T());

    //! ensure array has at AT LEAST given count of initialized element, similar to resize but the buffer can be bigger and we don't reduce/free memory. new elements are initialized with default constructor
    //! NOTE: it can fail during OOM and it's possible or meaningful it should be handled
    bool prepare(Count minimalSize);

    //! ensure array has at AT LEAST given count of initialized element, similar to resize but the buffer can be bigger and we don't free memory. new elements are initialized with provided template
    //! NOTE: it can fail during OOM and it's possible or meaningful it should be handled
    bool prepareWith(Count minimalSize, const T& elementTemplate = T());

    //! change array size in a direct, nothing is constructed, nothing is destroyed, very unsafe
    void changeSizeUnsafe(Count newSize);

    //--

    //! Compare array content (checks all elements). Returns true if arrays are equal.
    bool operator==(const Array<T> &other) const;

    //! Compare array content (checks all elements). Returns true if arrays are not equal.
    bool operator!=(const Array<T> &other) const;

    //--

    //! Sort array
	template< typename Pred = std::less<T> >
	void sort(Pred pred = std::less<T>());

	//! Stable sort array
	template< typename Pred = std::less<T> >
	void stableSort(Pred pred = std::less<T>());

	//! Quck sort array
	template< typename Pred = std::less<T> >
	void quickSort(Pred pred = std::less<T>());

    //--

    //! Insert into this array assuming it's sorted O(log N), will insert duplicated element
	template< typename Pred = std::less<T> >
	void sortedArrayInsert(const T& elem, Pred pred = std::less<T>());

	//! Insert into this array assuming it's sorted O(log N), will not insert duplicated element, returns true if inserted, false it if was duplicated
	template< typename Pred = std::less<T> >
	bool sortedArrayInsertUnique(const T& elem, Pred pred = std::less<T>());

	//! Check if array contains given element assuming array is sorted O(log N)
	template< typename FK, typename Pred = std::less<T> >
	bool sortedArrayContains(const FK& element, Pred pred = std::less<T>()) const;

	//! Remove element from sorted array, actually slow because unordered remove can't be used
	template< typename FK, typename Pred = std::less<T> >
	bool sortedArrayRemove(const FK& element, Pred pred = std::less<T>());

	//! Find index of first element in the array matching given key
	template< typename FK, typename Pred = std::less<T> >
	Index sortedArrayFindFirst(const FK& element, Pred pred = std::less<T>()) const;

	//! find first occurrence of element in the range, returns true if index was found
	template< typename FK, typename Pred = std::less<T> >
	bool sortedArrayFindFirst(const FK& key, Index& outFoundIndex, Pred pred = std::less<T>()) const;

	//! Remove duplicates from the array, returns number of duplicates removed
	uint32_t sortedArrayRemoveDuplicates();

    //--

private:
    // move array contents by moving pointers
    ALWAYS_INLINE Array<T>& movePointers(Array<T>&& other);

	// move array contents by preserving buffer and moving elements
	ALWAYS_INLINE Array<T>& moveElements(Array<T>&& other);
};

END_INFERNO_NAMESPACE()

#include "array.inl"