/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once
#include <vector>

BEGIN_INFERNO_NAMESPACE()

//--

ALWAYS_INLINE BufferView::BufferView(BufferView&& other)
	: m_start(other.m_start)
	, m_end(other.m_end)
{
	other.m_start = nullptr;
	other.m_end = nullptr;
}

ALWAYS_INLINE BufferView& BufferView::operator=(BufferView&& other)
{
	if (this != &other)
	{
		m_start = other.m_start;
		m_end = other.m_end;
		other.m_start = nullptr;
		other.m_end = nullptr;
	}

	return *this;
}

ALWAYS_INLINE BufferView::BufferView(void* data, uint64_t length)
	: m_start((uint8_t*)data)
	, m_end((uint8_t*)data + length)
{
	ASSERT_EX(m_end >= m_start, "Inverted buffer view");
}

ALWAYS_INLINE BufferView::BufferView(void* data, void* dataEnd)
	: m_start((uint8_t*)data)
	, m_end((uint8_t*)dataEnd)
{
	ASSERT_EX(m_end >= m_start, "Inverted buffer view");
}

ALWAYS_INLINE BufferView::BufferView(const void* data, uint64_t length)
	: m_start((uint8_t*)data)
	, m_end((uint8_t*)data + length)
{
	ASSERT_EX(m_end >= m_start, "Inverted buffer view");
}

ALWAYS_INLINE BufferView::BufferView(const void* data, const void* dataEnd)
	: m_start((uint8_t*)data)
	, m_end((uint8_t*)dataEnd)
{
	ASSERT_EX(m_end >= m_start, "Inverted buffer view");
}

//--

ALWAYS_INLINE uint8_t* BufferView::data()
{
	return m_start;
}

ALWAYS_INLINE const uint8_t* BufferView::data() const
{
	return m_start;
}

ALWAYS_INLINE uint64_t BufferView::size() const
{
	return m_end - m_start;
}

ALWAYS_INLINE uint32_t BufferView::segmentCount(uint32_t segmentSize) const
{
	return segmentSize ? ((size() + segmentSize - 1) / segmentSize) : 0;
}

ALWAYS_INLINE bool BufferView::empty() const
{
	return m_start >= m_end;
}

ALWAYS_INLINE BufferView::operator bool() const
{
	return m_start < m_end;
}

//--

ALWAYS_INLINE void BufferView::reset()
{
	m_start = nullptr;
	m_end = nullptr;
}

ALWAYS_INLINE bool BufferView::containsRange(BufferView other) const
{
	return other.empty() || (other.m_start >= (const uint8_t*)m_start && other.m_end <= (const uint8_t*)m_end);
}

ALWAYS_INLINE bool BufferView::containsPointer(const void* ptr) const
{
	return (uint8_t*)ptr >= (uint8_t*)m_start && (uint8_t*)ptr < (uint8_t*)m_end;
}

ALWAYS_INLINE bool BufferView::validateRange(uint64_t offset, uint64_t size_) const
{
	// NOTE: a little bit more test to avoid overflows
	return ((offset <= size()) && (size_ <= size()) && (offset + size_ <= size()));
}

ALWAYS_INLINE uint64_t BufferView::evaluateCopiableSize(uint64_t offset, uint64_t copySize) const
{
	const auto remaining = (offset <= size()) ? (size() - offset) : 0;
	return std::min<uint64_t>(remaining, copySize);
}

//--

ALWAYS_INLINE uint8_t* BufferView::begin()
{
	return m_start;
}

ALWAYS_INLINE uint8_t* BufferView::end()
{
	return m_end;
}

ALWAYS_INLINE const uint8_t* BufferView::begin() const
{
	return m_start;
}

ALWAYS_INLINE const uint8_t* BufferView::end() const
{
	return m_end;
}

//--

ALWAYS_INLINE BufferView BufferView::leftView(uint64_t cut) const
{
	DEBUG_CHECK_RETURN_EX_V(cut <= size(), "Cut size is larger than buffer", nullptr);
	return BufferView(m_start, m_start + cut);
}

ALWAYS_INLINE BufferView BufferView::rightView(uint64_t cut) const
{
	DEBUG_CHECK_RETURN_EX_V(cut <= size(), "Cut size is larger than buffer", nullptr);
	return BufferView(m_end - cut, m_end);
}

ALWAYS_INLINE BufferView BufferView::subView(uint64_t offset, uint64_t cut) const
{
	DEBUG_CHECK_RETURN_EX_V(validateRange(offset, cut), "Sub view placement is invalid", nullptr);
	return BufferView(m_start + offset, m_start + offset + cut);
}

ALWAYS_INLINE BufferView BufferView::relaxedSubView(uint64_t offset, uint64_t size) const
{
	const auto copiableSize = evaluateCopiableSize(offset, size);
	return copiableSize ? BufferView(data() + offset, data() + offset + copiableSize) : nullptr;
}

ALWAYS_INLINE BufferView BufferView::segmentView(uint32_t chunkSize, uint32_t index) const
{
	const auto offset = index * chunkSize;
	DEBUG_CHECK_RETURN_EX_V(offset < size(), "Segment index lies outside the buffer", nullptr);
	const auto cut = std::min<uint32_t>(chunkSize, size() - offset);
	return BufferView(m_start + offset, m_start + offset + cut);
}

//--

template< typename T >
ALWAYS_INLINE BufferInputStream<T>::BufferInputStream(BufferView view)
	: m_start((const T*)view.data())
	, m_pos((const T*)view.data())
	, m_end((const T*)(view.data() + view.size()))
{
}

template< typename T >
ALWAYS_INLINE BufferOutputStream<T>::BufferOutputStream(BufferView view)
	: m_start((T*)view.data())
	, m_pos((T*)view.data())
	, m_end((T*)(view.data() + view.size()))
{
}

template< typename T >
ALWAYS_INLINE BufferView BufferOutputStream<T>::view() const
{
	return BufferView(m_start, m_pos);
}

template< typename T >
ALWAYS_INLINE BufferOutputStream<T>::operator BufferView() const
{
	return BufferView(m_start, m_pos);
}

//--

END_INFERNO_NAMESPACE()
