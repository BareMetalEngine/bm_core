/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

#include "bufferSegmentIterator.h"

BEGIN_INFERNO_NAMESPACE()

//--

// a temporary view of a buffer that contains a "step" size, used to iterate in chunks
class BufferSegmentedView
{
public:
	INLINE BufferSegmentedView() = default;
	INLINE BufferSegmentedView(const BufferSegmentedView& other) = default;
	INLINE BufferSegmentedView(BufferSegmentedView&& other) = default;
	INLINE BufferSegmentedView& operator=(const BufferSegmentedView& other) = default;
	INLINE BufferSegmentedView& operator=(BufferSegmentedView&& other) = default;
	INLINE ~BufferSegmentedView() = default;
	INLINE BufferSegmentedView(BufferView view, uint32_t step, uint32_t count);

	INLINE BufferView view() const { return m_view; }
	INLINE uint32_t step() const { return m_step; }
	INLINE uint32_t count() const { return m_count; }

	INLINE BufferViewSegmentIterator begin() const;
	INLINE BufferViewSegmentIterator end() const;

private:
	BufferView m_view;
	uint32_t m_step = 0;	
	uint32_t m_count = 0;
};

//--

END_INFERNO_NAMESPACE()

#include "bufferSegmentView.inl"
#include "bufferSegmentIterator.inl"
