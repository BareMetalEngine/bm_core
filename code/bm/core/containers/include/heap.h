/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

#include "array.h"
#include "arrayView.h"

BEGIN_INFERNO_NAMESPACE()

/// heap, implemented on top of array
template <typename T, typename Container=Array<T>, typename Compare = std::less<T>>
class Heap
{
public:
    Heap() = default;
    Heap(Container&& baseData); // allow to construct heap from un-sorted data
    Heap(const Container& baseData); // allow to construct heap from un-sorted data
	explicit Heap(ArrayView<T> data); // allow to construct heap from un-sorted data
    Heap(const Heap &other) = default;
    Heap(Heap &&other) = default;
    Heap& operator=(const Heap &other) = default;
    Heap& operator=(Heap &&other) = default;

    //--

    //! true is queue is empty
    bool empty() const;

    //! get number of elements in the heap
    uint32_t size() const;

    //! clear the heap
    void clear();

    //! get head item (read only)
    const T &front() const;

    //! insert element to heap
    void push(const T& data);

    //! remove the top element from the heap
    void pop();

    //--

    //! get all values for iteration
    const Array<T>& values() const;

	//--

	//! get read only iterator to start of the array
    ConstArrayIterator<T> begin() const;

	//! get read only iterator to end of the array
    ConstArrayIterator<T> end() const;

    //--

private:
    Container m_values;
};

END_INFERNO_NAMESPACE()

#include "heap.inl"