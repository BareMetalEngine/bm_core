/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

BEGIN_INFERNO_NAMESPACE()

///--

/// Simple inplace (on stack) buffer predefined amount of memory first and than growing x2 using dynamic allocations
template< uint32_t InitSize, uint32_t Alignment = 4 >
class InplaceBuffer : public MainPoolData<NoCopy>
{
public:
    INLINE InplaceBuffer(IPoolUnmanaged& pool = MainPool());
	INLINE ~InplaceBuffer();

	//--

	// data pointer
	INLINE uint8_t* data() { return m_start; }

	// data pointer
	INLINE const uint8_t* data() const { return m_start; }

	// check if buffer is empty
	INLINE bool empty() const { return m_pos == m_start; }

	// returns true if buffer was not reallocated to dynamic memory
	INLINE bool local() const { return m_start == m_staticBuffer; }

	// size of the buffer
	INLINE uint32_t size() const { return m_pos - m_start; }

	// capacity of the buffer
	INLINE uint32_t capacity() const { return m_end - m_start; }

	// get view of the buffer
	INLINE BufferView view() const { return BufferView(m_start, m_pos); }

	//--
	
	// clear buffer, resets to default capacity
	INLINE void clear();

	// reset buffer, does not free memory
	INLINE void reset();

    // make sure buffer has enough data to store given amount of data, if not, buffer is resized
	INLINE bool ensureCapacity(uint32_t requiredCapacity);

	// append next block to the buffer
	INLINE BufferView allocate(uint32_t size);

	//--

	// validity check
	INLINE operator bool() const { return m_pos > m_start; }

	// buffer cast
	INLINE operator BufferView() const { return view(); }

	//--

private:
	IPoolUnmanaged& m_pool;

	uint8_t* m_start = nullptr;;
	uint8_t* m_pos = nullptr;
	uint8_t* m_end = nullptr;

#ifdef PLATFORM_MSVC
	__declspec(align(Alignment)) uint8_t m_staticBuffer[InitSize];
#else
	uint8_t m_staticBuffer[InitSize] __attribute__((aligned(Alignment)));
#endif

	INLINE bool doubleCapacity();
};

///--

END_INFERNO_NAMESPACE()

#include "inplaceBuffer.inl"