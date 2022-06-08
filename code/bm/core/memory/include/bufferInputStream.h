/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

BEGIN_INFERNO_NAMESPACE()

//--

/// Simple wrapped input memory stream with boundary check
template< typename T >
struct BufferInputStream
{
public:
	ALWAYS_INLINE BufferInputStream() = default;
	ALWAYS_INLINE BufferInputStream(std::nullptr_t) {};
	ALWAYS_INLINE BufferInputStream(BufferView view);
	ALWAYS_INLINE BufferInputStream(const T* ptr, uint64_t length);
	ALWAYS_INLINE BufferInputStream(const T* ptr, const T* ptrEnd);
	ALWAYS_INLINE BufferInputStream(const BufferInputStream& other) = default;
	ALWAYS_INLINE BufferInputStream(BufferInputStream&& other) = default;
	ALWAYS_INLINE BufferInputStream& operator=(const BufferInputStream& other) = default;
	ALWAYS_INLINE BufferInputStream& operator=(BufferInputStream&& other) = default;

	//--

	// current position in stream
	ALWAYS_INLINE uint64_t pos() const; 

	// size of the stream
	ALWAYS_INLINE uint64_t size() const; 

	// elements left in stream
	ALWAYS_INLINE uint64_t left() const; 

	// get block of N bytes, returns false if we don't have enough data left
	ALWAYS_INLINE const T* peek(uint32_t count);

	// read multiple elements from stream
	ALWAYS_INLINE void advance(uint32_t count);
		
	// consume as much data as possible, return it as another input stream
	ALWAYS_INLINE BufferInputStream<T> consume(uint32_t count);

	//--	

protected:
	const T* m_pos = nullptr;
	const T* m_end = nullptr;
	const T* m_start = nullptr;
};

//--

END_INFERNO_NAMESPACE()

#include "bufferInputStream.inl"
