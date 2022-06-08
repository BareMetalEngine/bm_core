/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

BEGIN_INFERNO_NAMESPACE()

//---

INLINE void StringBuf::clear()
{
    if (m_data)
    {
        m_data->release();
        m_data = nullptr;
    }
}

INLINE bool StringBuf::empty() const
{
    return !m_data;
}

INLINE uint32_t StringBuf::length() const
{
    return m_data ? m_data->length() : 0;
}

INLINE uint32_t StringBuf::unicodeLength() const
{
	return m_data ? m_data->unicodeLength() : 0;
}

INLINE uint32_t StringBuf::CalcHash(const StringBuf& txt)
{
    return StringView::CalcHash(txt.view());
}

INLINE uint32_t StringBuf::CalcHash(StringView txt)
{
    return StringView::CalcHash(txt);
}

INLINE uint32_t StringBuf::CalcHash(const char* txt)
{
    return StringView::CalcHash(txt);
}

INLINE uint32_t StringBuf::evaluateCRC32(uint32_t crc /* = CRC32Init */) const
{
    return view().evaluateCRC32(crc);
}

INLINE uint64_t StringBuf::evaluateCRC64(uint64_t crc /* = CRC64Init */) const
{
    return view().evaluateCRC64(crc);
}

INLINE const char* StringBuf::c_str() const
{
    return m_data ? m_data->c_str() : "";
}

/*INLINE StringBuf::operator const char* () const
{
	return m_data ? m_data->c_str() : "";
}*/

INLINE StringView StringBuf::view() const
{
    return StringView(c_str(), length());
}

INLINE StringBuf::operator StringView() const
{
    return view();
}

INLINE StringBuf::operator bool() const
{
    return !empty();
}

INLINE bool StringBuf::operator==(StringView other) const
{
	return view() == other;
}

INLINE bool StringBuf::operator!=(StringView other) const
{
	return view() != other;
}

INLINE bool StringBuf::operator<(StringView other) const
{
	return view() < other;
}

INLINE bool StringBuf::operator<=(StringView other) const
{
	return view() <= other;
}

INLINE bool StringBuf::operator>(StringView other) const
{
	return view() > other;
}

INLINE bool StringBuf::operator>=(StringView other) const
{
	return view() >= other;
}

/*INLINE bool StringBuf::operator==(const StringBuf& other) const
{
	return view() == other.view();
}

INLINE bool StringBuf::operator!=(const StringBuf& other) const
{
	return view() != other.view();
}

INLINE bool StringBuf::operator<(const StringBuf& other) const
{
	return view() < other.view();
}

INLINE bool StringBuf::operator<=(const StringBuf& other) const
{
	return view() <= other.view();
}

INLINE bool StringBuf::operator>(const StringBuf& other) const
{
	return view() > other.view();
}

INLINE bool StringBuf::operator>=(const StringBuf& other) const
{
	return view() >= other.view();
}*/

INLINE static bool operator==(const char* first, const StringBuf& other)
{
	return StringView(first) == other;
}

INLINE static bool operator!=(const char* first, const StringBuf& other)
{
	return StringView(first) != other;
}

INLINE static bool operator<(const char* first, const StringBuf& other)
{
	return StringView(first) < other;
}

INLINE static bool operator<=(const char* first, const StringBuf& other)
{
	return StringView(first) <= other;
}

INLINE static bool operator>(const char* first, const StringBuf& other)
{
	return StringView(first) > other;
}

INLINE static bool operator>=(const char* first, const StringBuf& other)
{
	return StringView(first) >= other;
}

INLINE int StringBuf::compare(StringView other, StringCaseComparisonMode caseMode) const
{
    return view().compare(other, caseMode);
}

INLINE StringBuf& StringBuf::operator=(const StringBuf& other)
{
	if (this != &other)
	{
		clear();
		m_data = other.m_data;
		if (m_data)
			m_data->addRef();
	}

	return *this;
}

INLINE StringBuf& StringBuf::operator=(StringBuf&& other)
{
	if (this != &other)
	{
		clear();
		m_data = other.m_data;
		other.m_data = nullptr;
	}
	return *this;
}

INLINE bool StringBuf::beginsWith(StringView pattern, StringCaseComparisonMode caseMode /*= StringCaseComparisonMode::WithCase*/) const
{
	return view().beginsWith(pattern, caseMode);
}

INLINE bool StringBuf::endsWith(StringView pattern, StringCaseComparisonMode caseMode /*= StringCaseComparisonMode::WithCase*/) const
{
	return view().endsWith(pattern, caseMode);
}

//---


END_INFERNO_NAMESPACE()
