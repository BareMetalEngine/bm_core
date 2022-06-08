/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

BEGIN_INFERNO_NAMESPACE()

//--

/// Buffer view iterator that splits buffer into chunks of given maximum size
class BufferViewSegmentIterator
{
public:
	using iterator_category = std::input_iterator_tag;
	using value_type = BufferView;
	using pointer = BufferView;
	using reference = BufferView;

	ALWAYS_INLINE BufferViewSegmentIterator() = default;
	ALWAYS_INLINE BufferViewSegmentIterator(const BufferViewSegmentIterator& other) = default;
	ALWAYS_INLINE BufferViewSegmentIterator& operator=(const BufferViewSegmentIterator& other) = default;
	ALWAYS_INLINE BufferViewSegmentIterator(BufferViewSegmentIterator&& other) = default;
	ALWAYS_INLINE BufferViewSegmentIterator& operator=(BufferViewSegmentIterator&& other) = default;
	explicit ALWAYS_INLINE BufferViewSegmentIterator(BufferView view, uint64_t step, uint32_t index);

	ALWAYS_INLINE BufferViewSegmentIterator& operator++();
	ALWAYS_INLINE BufferViewSegmentIterator operator++(int);
	ALWAYS_INLINE BufferViewSegmentIterator& operator--();
	ALWAYS_INLINE BufferViewSegmentIterator operator--(int);

	ALWAYS_INLINE BufferView operator*() const;

	ALWAYS_INLINE bool operator==(BufferViewSegmentIterator other) const;
	ALWAYS_INLINE bool operator!=(BufferViewSegmentIterator other) const;
	ALWAYS_INLINE bool operator<(BufferViewSegmentIterator other) const;
	ALWAYS_INLINE bool operator<=(BufferViewSegmentIterator other) const;
	ALWAYS_INLINE bool operator>(BufferViewSegmentIterator other) const;
	ALWAYS_INLINE bool operator>=(BufferViewSegmentIterator other) const;

private:
	BufferView m_view;
	uint32_t m_index = 0;
	uint64_t m_step = 0;
};

//--

END_INFERNO_NAMESPACE()
