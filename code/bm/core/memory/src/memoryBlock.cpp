/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "memoryBlock.h"

BEGIN_INFERNO_NAMESPACE()

//---

MemoryBlock::MemoryBlock(const IMemoryBlockStateResolver* resolver, uint32_t generation, uint32_t index, uint64_t size)
	: m_data((void*)resolver)
	, m_size(size)
	, m_index(index)
	, m_generation(generation)
{
	ASSERT(resolver != 0);
	ASSERT(size != 0);
	ASSERT(generation != 0);
}

MemoryBlock::MemoryBlock(const void* data, uint64_t size)
	: m_data((void*)data)
	, m_size(size)
	, m_index(0)
	, m_generation(0)
{
	ASSERT(data != nullptr);
	ASSERT(size != 0);
}

//---

void* MemoryBlock::pointer() const
{
	if (m_generation)
		return ((const IMemoryBlockStateResolver*)m_data)->resolvePointer(m_index, m_generation);
	else
		return m_data;
}

uint64_t MemoryBlock::address() const
{
	if (m_generation)
		return ((const IMemoryBlockStateResolver*)m_data)->resolveAddress(m_index, m_generation);
	else
		return (uint64_t)m_data;
}

BufferView MemoryBlock::view() const
{
	return m_size ? BufferView(pointer(), m_size) : nullptr;
}

//--

void MemoryBlock::print(IFormatStream& f) const
{
	if (m_generation)
		f.appendf("ManagedBlock {}/G{} of size {} @ {}", m_index, m_generation, m_size, pointer());
	else  if (m_data)
		f.appendf("UnmanagedBlock of size {} @ {}", m_size, m_data);
	else
		f << "NullBlock";
}


//---

END_INFERNO_NAMESPACE()
