/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

#include "bm/core/memory/include/bufferView.h"

//--

BEGIN_INFERNO_NAMESPACE()

/// Buffer for array, usually allocated from "Array" pool but can be allocated from other means, then it has the "fixed size" flag that means it can't be resized
/// NOTE: the buffer pointer itself is not copyable as it represents array data
class BaseArrayBuffer : public MainPoolData<NoCopy>
{
public:
    ALWAYS_INLINE BaseArrayBuffer();
    ALWAYS_INLINE BaseArrayBuffer(void* externalPointer, Count capacity, bool owned);
    ALWAYS_INLINE BaseArrayBuffer(BaseArrayBuffer&& other);
    ALWAYS_INLINE BaseArrayBuffer&operator=(BaseArrayBuffer&& other);
    BM_CORE_CONTAINERS_API ~BaseArrayBuffer();

    //--

    //! get internal data buffer
    ALWAYS_INLINE void* data();

    //! get internal data buffer (read-only)
    ALWAYS_INLINE const void* data() const;

    //! returns maximum number of elements array can hold without resizing
    ALWAYS_INLINE Count capacity() const;

    //! do we own this buffer ?
    ALWAYS_INLINE bool owned() const;

    //! is this buffer empty ?
    ALWAYS_INLINE bool empty() const;

    //--

    // release memory
    BM_CORE_CONTAINERS_API void release();

    // resize the buffer to match new capacity, will try to reuse existing memory unless the request is smaller or we were using memory that we don't own
    BM_CORE_CONTAINERS_API bool resize(Count newCapcity, uint64_t currentMemorySize, uint64_t newMemorySize, uint64_t memoryAlignment, const char* typeName);

    // replace with other pointer
    BM_CORE_CONTAINERS_API void* replaceAndReturn(void* other, uint32_t capacity, bool owned);

	// replace with other pointer and free existing one (if owned)
	BM_CORE_CONTAINERS_API void replaceAndFree(void* other, uint32_t capacity, bool owned);

    //--

private:
    void* m_ptr = nullptr;

    static_assert(sizeof(Count) == 4, "Change this if size of count changes");

    Count m_flagOwned:1;
    Count m_capacity:31;
};

//--

/// Raw wrapper for basic array operations, used mostly by the RTTI accessors
/// This wrapper allows us to operate on ANY array regardless of it's type, also it delegates some of the Type-independent code to a cpp file to reduce code bloat
class BaseArray
{
public:
    BaseArray() = default;
    BaseArray(BaseArrayBuffer&& buffer); // create base array on pre-existing data buffer
    ~BaseArray();

    //! get internal data buffer
    ALWAYS_INLINE void *data();

    //! get internal data buffer (read-only)
    ALWAYS_INLINE const void *data() const;

    //! returns true if the array is empty
    ALWAYS_INLINE bool empty() const;

    //! returns true if the array is empty
    ALWAYS_INLINE bool full() const;

    //! is the memory in the buffer owned ?
    ALWAYS_INLINE bool owned() const;

    //! returns number of elements in the array
    ALWAYS_INLINE Count size() const;

    //! get last valid index in this array (size-1)
    ALWAYS_INLINE Index lastValidIndex() const;

    //! returns maximum number of elements array can hold without resizing
    ALWAYS_INLINE Count capacity() const;

    //! get array's buffer
    ALWAYS_INLINE BaseArrayBuffer& buffer();

    //! get array's buffer
    ALWAYS_INLINE const BaseArrayBuffer& buffer() const;

    //--

    //! change size of the array, basically only changes the member + some validation, returns old size
    //! NOTE: HACK function, caller takes full responsibility
	BM_CORE_CONTAINERS_API Count changeSize(Count newSize);

    //! change the capacity of the array, basically just reallocates the data buffer, returns old capacity
    //! NOTE: calling this when buffer is not owned by array will make the array take ownership of the buffer
	BM_CORE_CONTAINERS_API bool changeCapacity(Count newCapacity, uint64_t currentMemorySize, uint64_t newMemorySize, uint64_t memoryAlignment, const char* typeNameInfo="");

    //--

    // calculate NEXT best array buffer size - when resizing array from given size
    BM_CORE_CONTAINERS_API static uint64_t CalcNextBufferSize(uint64_t currentSize);

protected:

#ifndef BUILD_RELEASE
	BM_CORE_CONTAINERS_API void checkIndex(Index index) const;
	BM_CORE_CONTAINERS_API void checkIndexRange(Index index, Count count) const;
#else
    ALWAYS_INLINE void checkIndex(Index index) const {};
    ALWAYS_INLINE void checkIndexRange(Index index, Count count) const {};
#endif

protected:
    BaseArrayBuffer m_buffer; // hold the capacity
    Count m_size = 0;
};

END_INFERNO_NAMESPACE()

//--

#include "baseArray.inl"