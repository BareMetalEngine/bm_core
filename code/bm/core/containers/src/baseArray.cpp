/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "baseArray.h"
#include "arrayView.h"
#include "array.h"

BEGIN_INFERNO_NAMESPACE()

//--

BaseArrayBuffer::~BaseArrayBuffer()
{
    release();
}

bool BaseArrayBuffer::resize(Count newCapcity, uint64_t currentMemorySize, uint64_t newMemorySize, uint64_t memoryAlignment, const char* typeName)
{
    if (newCapcity == 0)
    {
        release();
    }
    else if (m_flagOwned)
    {
        auto* newPtr = Memory::ResizeBlock(m_ptr, newMemorySize, memoryAlignment, typeName);
        if (!newPtr)
            return false;

        m_ptr = newPtr;
    }
    else
    {
        auto* newPtr = Memory::ResizeBlock(nullptr, newMemorySize, memoryAlignment, typeName);
        if (!newPtr)
            return false;

        memcpy(newPtr, m_ptr, std::min<uint64_t>(currentMemorySize, newMemorySize));
        m_ptr = newPtr;
    }

    m_capacity = newCapcity;
    m_flagOwned = true;
    return true;
}

void BaseArrayBuffer::release()
{
    if (m_flagOwned)
        Memory::FreeBlock(m_ptr);

    m_flagOwned = true;
    m_capacity = 0;
    m_ptr = nullptr;
}

void* BaseArrayBuffer::replaceAndReturn(void* other, uint32_t capacity, bool owned)
{
    auto old = m_ptr;
    m_ptr = other;
    m_capacity = capacity;
    m_flagOwned = owned;
    return old;
}

void BaseArrayBuffer::replaceAndFree(void* other, uint32_t capacity, bool owned)
{
	if (m_flagOwned)
		Memory::FreeBlock(m_ptr);
	
	m_ptr = other;
    m_capacity = capacity;
	m_flagOwned = owned;
}

//--

#ifndef BUILD_RELEASE
void BaseArray::checkIndex(Index index) const
{
    ASSERT_EX(index >= 0 && index <= lastValidIndex(), "Array index out of range");
}

void BaseArray::checkIndexRange(Index index, Count count) const
{
    ASSERT_EX((index >= 0) && (index + count) <= size(), "Array range out of range");
}
#endif

Count BaseArray::changeSize(Count newSize)
{
    ASSERT_EX(newSize <= capacity(), "Array does not have capacity for given size")
    auto oldSize = m_size;
    m_size = newSize;
    return oldSize;
}

bool BaseArray::changeCapacity(Count newCapacity, uint64_t currentMemorySize, uint64_t newMemorySize, uint64_t memoryAlignment, const char* typeNameInfo)
{
    return m_buffer.resize(newCapacity, currentMemorySize, newMemorySize, memoryAlignment, typeNameInfo);
}

// NOTE: this is in CPP ONLY so we can tinker with it and NOT recompile everything
    
uint64_t BaseArray::CalcNextBufferSize(uint64_t currentSize)
{
    if (currentSize < 64)
        return 64; // smallest sensible allocation for array

    return (currentSize * 3) / 2;
}

END_INFERNO_NAMESPACE()
