/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

///--

#include "bufferView.h"

BEGIN_INFERNO_NAMESPACE()

///--

/// Helper class for resolving internal pointers to memory block
class BM_CORE_MEMORY_API IMemoryBlockStateResolver : public MainPoolData<NoCopy>
{
public:
	virtual ~IMemoryBlockStateResolver();

	/// resolve block pointer
	virtual void* resolvePointer(uint32_t index, uint32_t generation) const = 0;

	/// resolve block address
	virtual uint64_t resolveAddress(uint32_t index, uint32_t generation) const = 0;
};

///--

/// Managed memory block
struct BM_CORE_MEMORY_API MemoryBlock
{
public:
	INLINE MemoryBlock() = default;
	INLINE MemoryBlock(std::nullptr_t) {};
	INLINE MemoryBlock(const MemoryBlock& other) = default;
	INLINE MemoryBlock& operator=(const MemoryBlock& other) = default;
	INLINE ~MemoryBlock() {};
	MemoryBlock(const IMemoryBlockStateResolver* resolver, uint32_t generation, uint32_t index, uint64_t size); // managed
	MemoryBlock(const void* data, uint64_t size); // unmanaged wrapped

	INLINE bool operator==(const MemoryBlock& other) const { return m_generation == other.m_generation && m_size == other.m_size && m_data == other.m_data; }
	INLINE bool operator!=(const MemoryBlock& other) const { return !operator==(other); }

	// is the block valid
	INLINE operator bool() const { return m_size != 0; }
	INLINE uint64_t size() const { return m_size; }

	// is the block managed ?
	INLINE bool isManaged() const { return (m_generation != 0); }

	// resolve current pointer
	// NOTE: valid only if block if pool is not in defrag state
	void* pointer() const;

	// resolve current address
	uint64_t address() const;

	// view to the memory
	BufferView view() const;

	//--

	void print(IFormatStream& f) const;

	//--

public:
	void* m_data = nullptr;
	uint64_t m_size = 0;
	uint32_t m_index = 0;;
	uint32_t m_generation = 0;;
};

///--

END_INFERNO_NAMESPACE()
