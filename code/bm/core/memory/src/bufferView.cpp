/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "buffer.h"
#include "bufferView.h"

BEGIN_INFERNO_NAMESPACE()

//---

void BufferView::zeroMemory()
{
    memzero(data(), size());
}

void BufferView::fillMemory(uint8_t value)
{
    memset(data(), value, size());
}

int BufferView::compareMemory(BufferView other) const
{
    if (size() < other.size())
        return -1;
    else if (size() > other.size())
        return 1;

    return memcmp(data(), other.data(), size());
}

void BufferView::reverseMemory()
{
    auto* a = m_start;
    auto* b = m_end - 1;
    while (a < b)
        std::swap(*a++, *b--);
}

uint64_t BufferView::Copy(BufferView src, uint64_t srcOffset, BufferView dest, uint64_t destOffset, uint64_t size)
{
    const auto srcCopy = src.evaluateCopiableSize(srcOffset, size);
    const auto destCopy = dest.evaluateCopiableSize(destOffset, size);
    const auto totalCopy = std::min<uint64_t>(srcCopy, destCopy);
    memcpy(dest.data() + destOffset, src.data() + srcOffset, totalCopy);
    return totalCopy;
}

//---

bool BufferView::cutLeft(uint64_t size_, BufferView& outRegion)
{
    auto* cutPtr = m_start + size_;
    if (cutPtr <= m_end && cutPtr >= m_start) // prevent wrap around on negative input
    {
        outRegion = BufferView(m_start, cutPtr);
        m_start = cutPtr;
        return true;
    }

    return false;
}

bool BufferView::cutRight(uint64_t size_, BufferView& outRegion)
{
	auto* cutPtr = m_end - size_;
    if (cutPtr <= m_end && cutPtr >= m_start) // prevent wrap around on negative input
	{
		outRegion = BufferView(cutPtr, m_end);
        m_end = cutPtr;
		return true;
	}

	return false;
}

bool BufferView::cutLeftWithAlignment(uint64_t size_, uint32_t alignment, BufferView& outRegion)
{
    ASSERT_EX(alignment >= 1, "Invalid alignment");

    auto* basePtr = AlignPtr(m_start, alignment);
    auto* cutPtr = basePtr + size_;
	if (cutPtr <= m_end && cutPtr >= m_start) // prevent wrap around on negative input
	{
		outRegion = BufferView(basePtr, cutPtr);
		m_end = cutPtr;
		return true;
	}

    return false;
}

bool BufferView::cutRightWithAlignment(uint64_t size_, uint32_t alignment, BufferView& outRegion)
{
	ASSERT_EX(alignment >= 1, "Invalid alignment");

	auto* basePtr = AlignPtr(m_end - size_, alignment);
    if ((m_end - basePtr) < size_)
        basePtr -= alignment;

	if (basePtr <= m_end && basePtr >= m_start) // prevent wrap around on negative input
	{
        outRegion = BufferView(basePtr, basePtr + size_);
		m_end = basePtr;
		return true;
	}

    return false;
}

//---

END_INFERNO_NAMESPACE()
