/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

BEGIN_INFERNO_NAMESPACE()

//--

INLINE BufferSegmentedView::BufferSegmentedView(BufferView view, uint32_t step, uint32_t count)
	: m_view(view)
	, m_step(step)
	, m_count(count)
{}

INLINE BufferViewSegmentIterator BufferSegmentedView::begin() const
{
	return BufferViewSegmentIterator(m_view, m_step, 0);
}

INLINE BufferViewSegmentIterator BufferSegmentedView::end() const
{
	return BufferViewSegmentIterator(m_view, m_step, m_count);
}

//--

ALWAYS_INLINE BufferSegmentedView BufferView::segmentedView(uint32_t chunkSize) const
{
	const auto chunkCount = segmentCount(chunkSize);
	return BufferSegmentedView(*this, chunkSize, chunkCount);
}

//--

END_INFERNO_NAMESPACE()
