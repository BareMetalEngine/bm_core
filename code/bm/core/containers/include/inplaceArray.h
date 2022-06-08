/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

#include "array.h"
#include "arrayIterator.h"

BEGIN_INFERNO_NAMESPACE()

//---

/// inplace array, contains some internal buffer with elements
template < class T, uint32_t N >
class InplaceArray : public Array<T>
{
public:
    InplaceArray();
	InplaceArray(const Array<T>& other);
	InplaceArray(Array<T>&& other);
	InplaceArray(const T* ptr, uint32_t size); // makes a copy of the data
	InplaceArray(const InplaceArray<T, N>& other);
	InplaceArray(InplaceArray<T, N>&& other);
	InplaceArray(std::initializer_list<T> values);

	InplaceArray& operator=(const Array<T>& other);
	InplaceArray& operator=(Array<T>&& other);
	InplaceArray& operator=(const InplaceArray<T, N>& other);
	InplaceArray& operator=(InplaceArray<T, N>&& other);

	// check if array is still inplace or was it relocated to heap
	INLINE bool inplace() const;

	// clear array, restores use of inplace storage
	INLINE void clear();

private:
    alignas(alignof(T)) uint8_t m_storage[N * sizeof(T)]; // internal storage buffer
};

//---

END_INFERNO_NAMESPACE()

#include "inplaceArray.inl"