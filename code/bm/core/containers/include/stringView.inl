/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

BEGIN_INFERNO_NAMESPACE()

namespace prv
{
    static uint32_t SafeStringLength(const char* str, uint32_t length = std::numeric_limits<uint32_t>::max())
    {
        if (str)
        {
            if (length == std::numeric_limits<uint32_t>::max())
                length = strlen(str);
        }
        else
        { 
            length = 0;
		}

        return length;
    }
}

INLINE StringView::StringView(const char* start, uint32_t length)
    : ArrayView<char>(start, prv::SafeStringLength(start, length))
{
}

INLINE StringView::StringView(ConstArrayIterator<char> start, ConstArrayIterator<char> end)
    : ArrayView<char>(start.ptr(), end.ptr())
{}

INLINE StringView::StringView(const char* start, const char* end)
    : ArrayView<char>(start, end)
{}

INLINE StringView::StringView(BufferView rawData)
	: ArrayView<char>(rawData)
{}

INLINE StringView::StringView(ArrayView<char> rawData)
	: ArrayView<char>(rawData)
{}

template< uint32_t N >
INLINE StringView::StringView(const BaseTempString<N>& tempString)
    : ArrayView<char>(tempString.c_str(), strlen(tempString.c_str()))
{
}

INLINE bool StringView::empty() const
{
    return ArrayView<char>::empty();
}

INLINE StringView::operator bool() const
{
    return !ArrayView<char>::empty();
}

INLINE StringView::operator BufferView() const
{
    return bufferView();
}

INLINE uint32_t StringView::length() const
{
    return ArrayView<char>::size();
}

INLINE const char* StringView::data() const
{
    return ArrayView<char>::typedData();
}

INLINE void StringView::reset()
{
    ArrayView<char>::reset();
}

INLINE void StringView::print(IFormatStream& p) const
{
    p.append(data(), length());
}

INLINE bool StringView::operator==(StringView other) const
{
    return 0 == compare(other);
}

INLINE bool StringView::operator!=(StringView other) const
{
    return 0 != compare(other);
}

INLINE bool StringView::operator<(StringView other) const
{
    return compare(other) < 0;
}

INLINE bool StringView::operator<=(StringView other) const
{
    return compare(other) <= 0;
}

INLINE bool StringView::operator>(StringView other) const
{
    return compare(other) > 0;
}

INLINE bool StringView::operator>=(StringView other) const
{
    return compare(other) >= 0;
}

INLINE static bool operator==(const char* first, StringView other)
{
	return StringView(first) == other;
}

INLINE static bool operator!=(const char* first, StringView other)
{
	return StringView(first) != other;
}

INLINE static bool operator<(const char* first, StringView other)
{
	return StringView(first) < other;
}

INLINE static bool operator<=(const char* first, StringView other)
{
	return StringView(first) <= other;
}

INLINE static bool operator>(const char* first, StringView other)
{
	return StringView(first) > other;
}

INLINE static bool operator>=(const char* first, StringView other)
{
	return StringView(first) >= other;
}

INLINE StringView StringView::leftPart(uint32_t count) const
{
    const auto cut = std::min<uint32_t>(count, length());
    return StringView(data(), data() + cut);
}

INLINE StringView StringView::rightPart(uint32_t count) const
{
    const auto cut = std::min<uint32_t>(count, length());
    return StringView(data() + length() - cut, data() + length());
}

INLINE StringView StringView::subString(uint32_t first, uint32_t count) const
{
    if (first > length()) first = length();
    const auto cut = std::min(count, length() - first);
    return StringView(data() + first, data() + first + cut);
}

INLINE StringView StringView::innerString(uint32_t frontTrim, uint32_t backTrim) const
{
    if (frontTrim + backTrim >= length()) return StringView();
    return StringView(data() + frontTrim, length() - frontTrim - backTrim);
}

INLINE void StringView::split(uint32_t index, StringView& outLeft, StringView& outRight) const
{
    outLeft = leftPart(index);
    outRight = subString(index);
}

INLINE bool StringView::splitAt(StringView str, StringView& outLeft, StringView& outRight) const
{
    auto pos = findStr(str);
    if (pos != INDEX_NONE)
    {
        outLeft = leftPart(pos);
        outRight = subString(pos + str.length());
        return true;
    }

    return false;
}

//--

INLINE bool StringView::beginsWith(StringView pattern, StringCaseComparisonMode caseMode) const
{
    return 0 == leftPart(pattern.length()).compare(pattern, caseMode);
}

INLINE bool StringView::endsWith(StringView pattern, StringCaseComparisonMode caseMode) const
{
    return 0 == rightPart(pattern.length()).compare(pattern, caseMode);
}

//--

INLINE StringView StringView::afterFirst(StringView pattern, StringCaseComparisonMode caseMode, StringFindFallbackMode fallback) const
{
    auto index = findStr(pattern, caseMode);
    if (index != INDEX_NONE)
        return subString(index + pattern.length());

    return (fallback == StringFindFallbackMode::Empty) ? nullptr : *this;
}

INLINE StringView StringView::beforeFirst(StringView pattern, StringCaseComparisonMode caseMode, StringFindFallbackMode fallback) const
{
	auto index = findStr(pattern, caseMode);
	if (index != INDEX_NONE)
		return leftPart(index);

	return (fallback == StringFindFallbackMode::Empty) ? nullptr : *this;
}

INLINE StringView StringView::afterLast(StringView pattern, StringCaseComparisonMode caseMode, StringFindFallbackMode fallback) const
{
	auto index = findRevStr(pattern, caseMode);
	if (index != INDEX_NONE)
		return subString(index + pattern.length());

    return (fallback == StringFindFallbackMode::Empty) ? nullptr : *this;
}

INLINE StringView StringView::beforeLast(StringView pattern, StringCaseComparisonMode caseMode, StringFindFallbackMode fallback) const
{
    auto index = findRevStr(pattern, caseMode);
	if (index != INDEX_NONE)
		return leftPart(index);

	return (fallback == StringFindFallbackMode::Empty) ? nullptr : *this;
}

//--

INLINE BufferView StringView::bufferView() const
{
    return BufferView(data(), data() + length());
}

INLINE ArrayView<char> StringView::arrayView() const
{
	return ArrayView<char>(data(), data() + length());
}

//--

INLINE ConstArrayIterator<char> StringView::begin() const
{
    return ConstArrayIterator<char>(m_start);
}

INLINE ConstArrayIterator<char> StringView::end() const
{
    return ConstArrayIterator<char>(m_end);
}

//--

template< typename T >
INLINE uint32_t StringView::sliceAndMatch(char splitChar, Array< T >& outValues) const
{
    uint32_t errors = 0;

	const char* str = data();
	const char* end = data() + length();
	const char* start = str;
	while (str < end)
	{
		char ch = *str;

		if (ch == '\"' || ch == '\'')
		{
			const char root = *str++;

			while (str < end)
			{
				if (*str == '\\')
				{
					str += 2;
					continue;
				}

				if (*str++ == root)
					break;
			}

			continue;
		}

		if (splitChar == ch)
		{
			const auto view = StringView(start, str);
			const auto trimView = view.trim();

            if (trimView)
            {
                T val;
                if (trimView.match(val))
                    outValues.emplaceBack(val);
                else
                    errors += 1;
            }

			start = str + 1;
		}

		str += 1;
	}

	if (start < str)
	{
		const auto view = StringView(start, str);
		const auto trimView = view.trim();

		if (trimView)
		{
			T val;
			if (trimView.match(val))
				outValues.emplaceBack(val);
			else
                errors += 1;
		}		
	}

    return errors;
}

//--

END_INFERNO_NAMESPACE()
