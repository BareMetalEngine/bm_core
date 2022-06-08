/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

BEGIN_INFERNO_NAMESPACE()

//--

ALWAYS_INLINE BufferViewSegmentIterator::BufferViewSegmentIterator(BufferView view, uint64_t step, uint32_t index)
	: m_view(view)
	, m_step(step)
	, m_index(index)
{}

ALWAYS_INLINE BufferViewSegmentIterator& BufferViewSegmentIterator::operator++()
{
	m_index += 1;
	return *this;
}

ALWAYS_INLINE BufferViewSegmentIterator BufferViewSegmentIterator::operator++(int)
{
	auto index = m_index;
	m_index += 1;
	return BufferViewSegmentIterator(m_view, m_step, index);
}

ALWAYS_INLINE BufferViewSegmentIterator& BufferViewSegmentIterator::operator--()
{
	m_index -= 1;
	return *this;
}

ALWAYS_INLINE BufferViewSegmentIterator BufferViewSegmentIterator::operator--(int)
{
	auto index = m_index;
	m_index -= 1;
	return BufferViewSegmentIterator(m_view, m_step, index);
}

ALWAYS_INLINE BufferView BufferViewSegmentIterator::operator*() const
{
	return m_view.segmentView(m_step, m_index);
}

ALWAYS_INLINE bool BufferViewSegmentIterator::operator==(BufferViewSegmentIterator other) const
{
	return m_index == other.m_index;
}

ALWAYS_INLINE bool BufferViewSegmentIterator::operator!=(BufferViewSegmentIterator other) const
{
	return m_index != other.m_index;
}

ALWAYS_INLINE bool BufferViewSegmentIterator::operator<(BufferViewSegmentIterator other) const
{
	return m_index < other.m_index;
}

ALWAYS_INLINE bool BufferViewSegmentIterator::operator<=(BufferViewSegmentIterator other) const
{
	return m_index <= other.m_index;
}

ALWAYS_INLINE bool BufferViewSegmentIterator::operator>(BufferViewSegmentIterator other) const
{
	return m_index > other.m_index;
}

ALWAYS_INLINE bool BufferViewSegmentIterator::operator>=(BufferViewSegmentIterator other) const
{
	return m_index >= other.m_index;
}

//--

END_INFERNO_NAMESPACE()
