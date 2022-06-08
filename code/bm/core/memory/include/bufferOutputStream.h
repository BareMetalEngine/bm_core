/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

BEGIN_INFERNO_NAMESPACE()

//--

class BufferView;

/// Simple wrapped output memory stream with boundary check
template< typename T >
struct BufferOutputStream
{
public:
	ALWAYS_INLINE BufferOutputStream() = default;
	ALWAYS_INLINE BufferOutputStream(std::nullptr_t) {};
	ALWAYS_INLINE BufferOutputStream(BufferView view);
	ALWAYS_INLINE BufferOutputStream(T* ptr, uint64_t length);
	ALWAYS_INLINE BufferOutputStream(T* ptr, const T* ptrEnd);
	ALWAYS_INLINE BufferOutputStream(const BufferOutputStream& other) = default;
	ALWAYS_INLINE BufferOutputStream(BufferOutputStream&& other) = default;
	ALWAYS_INLINE BufferOutputStream& operator=(const BufferOutputStream& other) = default;
	ALWAYS_INLINE BufferOutputStream& operator=(BufferOutputStream&& other) = default;

	//--

	// start of the stream
	ALWAYS_INLINE T* start() const;

	// current position in the stream
	ALWAYS_INLINE T* pos() const;

	// current position in stream
	ALWAYS_INLINE uint64_t size() const;

	// size of the stream
	ALWAYS_INLINE uint64_t capacity() const;

	// elements left in stream
	ALWAYS_INLINE uint64_t capacityLeft() const;

	// write single element to stream
	ALWAYS_INLINE T* alloc(uint32_t count);

	// write single item
	ALWAYS_INLINE bool write(T val);

	// is buffer empty ?
	ALWAYS_INLINE bool empty() const;

	// get view of the usable part
	ALWAYS_INLINE BufferView view() const;

	// get view of the usable part
	ALWAYS_INLINE operator BufferView() const;

	// anything written ?
	ALWAYS_INLINE operator bool() const;

	//--	

protected:
	T* m_pos = nullptr;
	T* m_end = nullptr;
	T* m_start = nullptr;
};

//--

template< typename T, uint32_t SIZE >
class InplaceBufferOutputStream : public BufferOutputStream<T>
{
public:
	ALWAYS_INLINE InplaceBufferOutputStream()
		: BufferOutputStream(m_data, SIZE)
	{}

	ALWAYS_INLINE operator const T* () const { return m_data; }

private:
	T m_data[SIZE];
};

//--

END_INFERNO_NAMESPACE()

#include "bufferOutputStream.inl"
