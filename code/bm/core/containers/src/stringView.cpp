/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
**/

#include "build.h"
#include "stringView.h"
#include "inplaceArray.h"
#include "utf8StringFunctions.h"

BEGIN_INFERNO_NAMESPACE()

namespace prv
{
    namespace search
    {

        template<typename C>
        struct Identity
        {
            INLINE static C Convert(C c)
            {
                return c;
            }
        };

        template<typename C>
        struct RemoveCase
        {
            INLINE static C Convert(C c)
            {
                if (c >= 'A' && c <= 'Z')
                    return 'a' + (c - 'A');
                return c;
            }
        };

        template<typename C, typename F = Identity<C>>
        static int64_t KMP(const C* TData, uint32_t TLength, const C* PData, uint32_t PLength)
        {
            if (PLength == 0)
                return 0;

            InplaceArray<int, 32> pi;
            pi.allocateWith(PLength, 0);
            for (int i=1, k=0; i<(int)PLength; ++i)
            {
                while (k && F::Convert(PData[k]) != F::Convert(PData[i]))
                    k = pi[k - 1];

                if (F::Convert(PData[k]) == F::Convert(PData[i]))
                    ++k;

                pi[i] = k;
            }

            for (int i=0, k=0; i < (int)TLength; ++i)
            {
                while (k && F::Convert(PData[k]) != F::Convert(TData[i]))
                    k = pi[k - 1];

                if (F::Convert(PData[k]) == F::Convert(TData[i]))
                    ++k;

                if (k == (int)PLength)
                    return i - k + 1;
            }

            return -1;
        }

        template<typename C, typename F = Identity<C>>
        static bool Test(const C* TData, const C* PData, int64_t PLength)
        {
            for (int64_t i=0; i<PLength; ++i)
            {
                if (F::Convert(TData[i]) != F::Convert(PData[i]))
                    return false;
            }
            return true;
        }

        template<typename C, typename F = Identity<C>>
        static int64_t Linear(const C* TData, int64_t TLength, const C* PData, int64_t PLength)
        {
            if (PLength == 0)
                return 0;

            int64_t maxTest = TLength - PLength;
            for (int64_t i=0; i<=maxTest; ++i)
                if (Test<C, F>(TData + i, PData, PLength))
                    return i;

            return -1;
        }

        template<typename C, typename F = Identity<C>>
        static int64_t LinearRev(const C* TData, int64_t TLength, const C* PData, int64_t PLength)
        {
            if (PLength == 0)
                return 0;

            int64_t maxTest = TLength - PLength;
            for (int64_t i=maxTest; i>=0; --i)
                if (Test<C, F>(TData + i, PData, PLength))
                    return i;

            return -1;
        }

    } // search

    //--

    template< typename T>
    static bool Contains(const char* ptr, T test)
    {
        while (*ptr)
        {
            if (*ptr++ == test)
                return true;
        }

        return false;
    }

    template< typename T>
    static void EatEscapedSequence(const T*& str, const T*& end)
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
    }

    static int64_t Find(const char* haystack, uint64_t length, const char* key, uint64_t keyLength)
    {
        if (keyLength <= 3)
            return search::Linear<char>(haystack, length, key, keyLength);
        else
            return search::KMP<char>(haystack, (uint32_t)length, key, (uint32_t)keyLength);
    }

    static int64_t Find(const wchar_t* haystack, uint64_t length, const wchar_t* key, uint64_t keyLength)
    {
        if (keyLength <= 3)
            return search::Linear<wchar_t>(haystack, length, key, keyLength);
        else
            return search::KMP<wchar_t>(haystack, (uint32_t)length, key, (uint32_t)keyLength);
    }

    static int64_t FindNoCase(const char* haystack, uint64_t length, const char* key, uint64_t keyLength)
    {
        if (keyLength <= 3)
            return search::Linear<char, search::RemoveCase<char>>(haystack, length, key, keyLength);
        else
            return search::KMP<char, search::RemoveCase<char>>(haystack, (uint32_t)length, key, (uint32_t)keyLength);
    }

    static int64_t FindNoCase(const wchar_t* haystack, uint64_t length, const wchar_t* key, uint64_t keyLength)
    {
        if (keyLength <= 3)
            return search::Linear<wchar_t, search::RemoveCase<wchar_t>>(haystack, length, key, keyLength);
        else
            return search::KMP<wchar_t, search::RemoveCase<wchar_t>>(haystack, (uint32_t)length, key, (uint32_t)keyLength);
    }

    static int64_t FindRev(const char* haystack, uint64_t length, const char* key, uint64_t keyLength)
    {
        return search::LinearRev<char>(haystack, length, key, keyLength);
    }

    static int64_t FindRev(const wchar_t* haystack, uint64_t length, const wchar_t* key, uint64_t keyLength)
    {
        return search::LinearRev<wchar_t>(haystack, length, key, keyLength);
    }

    static int64_t FindRevNoCase(const char* haystack, uint64_t length, const char* key, uint64_t keyLength)
    {
        return search::LinearRev<char, search::RemoveCase<char>>(haystack, length, key, keyLength);
    }

    static int64_t FindRevNoCase(const wchar_t* haystack, uint64_t length, const wchar_t* key, uint64_t keyLength)
    {
        return search::LinearRev<wchar_t, search::RemoveCase<wchar_t>>(haystack, length, key, keyLength);
    }

    static uint64_t StringHash(const char* str, const char* end)
    {
        uint64_t hval = UINT64_C(0xcbf29ce484222325);
        while (str < end)
        {
            hval ^= (uint64_t)*str++;
            hval *= UINT64_C(0x100000001b3);
        }
        return hval;
    }

    static uint64_t StringHashNoCase(const char* str, const char* end)
    {
        uint64_t hval = UINT64_C(0xcbf29ce484222325);
        while (str < end)
        {
            auto ch = *str++;

            if (ch >= 'A' && ch <= 'Z')
                ch = 'a' + (ch - 'A');

            hval ^= (uint64_t)ch;
            hval *= UINT64_C(0x100000001b3);
        }
        return hval;
    }

    static uint32_t StringCRC32(const char* str, const char* end, uint32_t crc)
    {
        CRC32 calc(crc);
        calc.append(str, end-str);
        return calc.crc();
    }

    static uint64_t StringCRC64(const char* str, const char* end, uint64_t crc)
    {
        CRC64 calc(crc);
        calc.append(str, end-str);
        return calc.crc();
    }

    static uint64_t StringHash(const wchar_t* str, const wchar_t* end)
    {
        uint64_t hval = UINT64_C(0xcbf29ce484222325);
        while (str < end)
        {
            hval ^= (uint64_t)*str++;
            hval *= UINT64_C(0x100000001b3);
        }
        return hval;
    }

    static uint32_t StringCRC32(const wchar_t* str, const wchar_t* end, uint32_t crc)
    {
        CRC32 calc(crc);
        calc.append(str, (end-str) * sizeof(wchar_t));
        return calc.crc();
    }

    static uint64_t StringCRC64(const wchar_t* str, const wchar_t* end, uint64_t crc)
    {
        CRC64 calc(crc);
        calc.append(str, (end-str) * sizeof(wchar_t));
        return calc.crc();
    }

    //--

    // http://www.geeksforgeeks.org/wildcard-character-matching/

    template<typename Ch>
    struct MatchCase
    {
        static ALWAYS_INLINE bool Match(Ch a, Ch b)
        {
            return a == b || a == '?';
        }
    };

    template<typename Ch>
    struct MatchNoCase
    {
        static ALWAYS_INLINE bool Match(Ch a, Ch b)
        {
            if (a == '?')
                return true;

            if (a >= 'A' && a <= 'Z') a = (a - 'A') + 'a';
            if (b >= 'A' && b <= 'Z') b = (b - 'A') + 'a';
            return a == b;
        }
    };

    template<typename Ch, typename Matcher = MatchCase<Ch>>
    bool MatchWildcardPattern(const Ch* first, const Ch* firstEnd, const Ch* second, const Ch* secondEnd)
    {
		// If we reach at the end of both strings, we are done
		if (first == firstEnd && second == secondEnd)
			return true;

		// Make sure that the characters after '*' are present
		// in second string. This function assumes that the first
		// string will not contain two consecutive '*'
		if ((first < firstEnd) && (*first == '*') && (first + 1 < firstEnd) && (second == secondEnd))
			return false;

		// If the first string contains '?', or current characters
		// of both strings match
		if ((first < firstEnd) && (second < secondEnd) && (*first == '?' || *first == *second))
			return MatchWildcardPattern(first + 1, firstEnd, second + 1, secondEnd);

		// If there is *, then there are two possibilities
		// a) We consider current character of second string
		// b) We ignore current character of second string.
		if ((first < firstEnd) && *first == '*')
			return MatchWildcardPattern(first + 1, firstEnd, second, secondEnd) || MatchWildcardPattern(first, firstEnd, second + 1, secondEnd);
		return false;
	}

    template< typename Ch >
    bool MatchWildcardPatternNoCase(const Ch* str, const Ch* strEnd, const Ch* pattern, const Ch* patternEnd)
    {
        return MatchWildcardPattern<Ch, MatchNoCase<Ch>>(str, strEnd, pattern, patternEnd);
    }

    ///--

    static bool MatchPattern(StringView str, StringView pattern)
    {
        return MatchWildcardPattern(pattern.data(), pattern.data() + pattern.length(), str.data(), str.data() + str.length());
    }

    static bool MatchPatternNoCase(StringView str, StringView pattern)
    {
        return MatchWildcardPatternNoCase(pattern.data(), pattern.data() + pattern.length(), str.data(), str.data() + str.length());
    }

    //--

    static bool MatchString(StringView str, StringView pattern)
    {
        return str.findStr(pattern) != INDEX_NONE;
    }

    bool MatchStringNoCase(StringView str, StringView pattern)
    {
        return str.findStr(pattern, StringCaseComparisonMode::NoCase) != INDEX_NONE;
    }

    //--

    static bool GetNumberValueForDigit(char ch, uint8_t& outDigit)
    {
        switch (ch)
        {
        case '0': outDigit = 0; return true;
		case '1': outDigit = 1; return true;
		case '2': outDigit = 2; return true;
		case '3': outDigit = 3; return true;
		case '4': outDigit = 4; return true;
		case '5': outDigit = 5; return true;
		case '6': outDigit = 6; return true;
		case '7': outDigit = 7; return true;
		case '8': outDigit = 8; return true;
		case '9': outDigit = 9; return true;
        }

        return false;
    }

    template<typename T>
    INLINE bool CheckNumericalOverflow(T val, T valueToAdd)
    {
        if (valueToAdd > 0)
        {
            auto left = std::numeric_limits<T>::max() - val;
            return valueToAdd > left;
        }
        else if (valueToAdd < 0)
        {
            auto left = std::numeric_limits<T>::lowest() - val;
            return valueToAdd < left;
        }

        return false;
    }

    template<typename Ch, typename T>
    INLINE static MatchResult AMatchInteger(const Ch *str, T &outValue, size_t strLength)
    {
        static_assert(std::is_signed<T>::value || std::is_unsigned<T>::value, "Only integer types are allowed here");

        // empty strings are not valid input to this function
        if (!str || !*str)
            return MatchResultStatus::EmptyString;

        // determine start and end of parsing range as well as the sign
        auto negative = (*str == '-');
        auto strStart = (*str == '+' || *str == '-') ? str + 1 : str;
        auto strEnd = str + strLength;

        // unsigned values cannot be negative :)
        if (std::is_unsigned<T>::value && negative)
            return MatchResultStatus::InvalidCharacter;

        T value = 0;
        T mult = negative ? -1 : 1;

        // assemble number
        auto pos = strEnd;
        bool overflowed = false;
        while (pos > strStart)
        {
            auto ch = *(--pos);

            // if a non-zero digit is encountered we must make sure that he mult is not overflowed already
            uint8_t digitValue;
            if (!GetNumberValueForDigit((char)ch, digitValue))
                return MatchResultStatus::InvalidCharacter;

            // apply
            if (digitValue != 0 && overflowed)
                return MatchResultStatus::Overflow;

            // validate that we will not overflow the type
            auto valueToAdd = range_cast<T>(digitValue * mult);
            if ((valueToAdd / mult) != digitValue)
                return MatchResultStatus::Overflow;
            if (prv::CheckNumericalOverflow<T>(value, valueToAdd))
                return MatchResultStatus::Overflow;

            // accumulate
            value += valueToAdd;

            // advance to next multiplier
            T newMult = mult * 10;
            if (newMult / 10 != mult)
                overflowed = true;
            mult = newMult;
        }

        outValue = value;
        return MatchResultStatus::OK;
    }

    template<typename Ch>
    INLINE bool ATestDigit(Ch ch)
    {
        return (ch >= '0' && ch <= '9');
    }

    template<typename Ch>
    INLINE bool ATestFloatExponentPart(const Ch* pos, const Ch* strEnd)
    {
        if (pos >= strEnd)
            return false; // ended to soon, just 'e'

        if (*pos == '-' || *pos == '+')
            pos += 1;
        
        if (pos >= strEnd)
            return false; // ended to soon, just 'e-'

        while (pos < strEnd)
        {
            auto ch = *pos++;
            if (ATestDigit(ch))
                continue;
            return false; // invalid char
        }

        return true; // all chars in range were valid
    }

    template<typename Ch>
    INLINE bool ATestFloatFactionalPart(const Ch* pos, const Ch* strEnd)
    {
        while (pos < strEnd)
        {
            auto ch = *pos++;

            if (ATestDigit(ch))
                continue;

            if (ch == 'f')
                return pos == strEnd; // we parsed all chars and there were valid

            if (ch == 'e' || ch == 'E')
                return ATestFloatExponentPart(pos, strEnd); // we are valid only if remaining part is an perfect exponent

            return false; // unexpected char
        }

        return true; // digits ended without 'f' or 'e' ending but still legal
    }

    template<typename Ch>
    INLINE bool ATestFloat(const Ch* pos, const Ch* strEnd)
    {
        if (pos >= strEnd)
            return false;

        if (*pos == '-' || *pos == '+')
            pos += 1;

        if (pos >= strEnd)
            return false;

        if (*pos == '.')
            return ATestFloatFactionalPart(pos + 1, strEnd);

        while (pos < strEnd)
        {
            auto ch = *pos++;
            if (ATestDigit(ch))
                continue;

            if (ch == 'e' || ch == 'E')
                return ATestFloatExponentPart(pos, strEnd); // we are valid only if remaining part is an perfect exponent

            if (ch == '.')
                return ATestFloatFactionalPart(pos, strEnd);

            return false;
        }

        return true;
    }

    template<typename Ch>
    INLINE MatchResult AMatchFloat(const Ch* str, double& outValue, size_t strLength)
    {
        // empty strings are not valid input to this function
        if (!str || !*str)
            return MatchResultStatus::EmptyString;

        // check if it's even a float
        if (!ATestFloat<Ch>(str, str + strLength))
            return MatchResultStatus::InvalidCharacter;

        // copy to local 
        char stack[100];

        // to long
        if (strLength >= ARRAY_COUNT(stack))
            return MatchResultStatus::InvalidCharacter;

        for (uint32_t i = 0; i < strLength; ++i)
            stack[i] = (char)str[i];
        stack[strLength] = 0;

        outValue = strtod(stack, nullptr);
        return MatchResultStatus::OK;
        /*
        // determine start and end of parsing range as well as the sign
        auto negative = (*str == '-');
        auto strEnd  = str + strLength;
        auto strStart  = (*str == '+' || *str == '-') ? str + 1 : str;

        // validate that we have a proper characters, discover the decimal point position
        auto strDecimal  = strEnd; // if decimal point was not found assume it's at the end
        {
            auto pos  = strStart;
            while (pos < strEnd)
            {
                auto ch = *pos++;

                if (pos == strEnd && ch == 'f')
                    break;

                if (ch == '.')
                {
                    strDecimal = pos - 1;
                }
                else
                {
                    uint8_t value = 0;
                    if (!prv::GetNumberValueForDigit((char)ch, 10, value))
                        return MatchResultStatus::InvalidCharacter;
                }
            }
        }

        // accumulate values
        double value = 0.0f;

        // TODO: this is tragic where it comes to the precision loss....
        // TODO: overflow/underflow
        {
            double mult = 1.0f;

            auto pos  = strDecimal;
            while (pos > strStart)
            {
                auto ch = *(--pos);

                uint8_t digitValue = 0;
                if (!prv::GetNumberValueForDigit((char)ch, 10, digitValue))return MatchResultStatus::InvalidCharacter;

                // accumulate
                value += (double)digitValue * mult;
                mult *= 10.0;
            }
        }

        // Fractional part
        if (strDecimal < strEnd)
        {
            double mult = 0.1f;

            auto pos  = strDecimal + 1;
            while (pos < strEnd)
            {
                auto ch = *(pos++);

                if (pos == strEnd && ch == 'f')
                    break;

                uint8_t digitValue = 0;
                if (!prv::GetNumberValueForDigit((char)ch, 10, digitValue))
                    return MatchResultStatus::InvalidCharacter;

                // accumulate
                value += (double)digitValue * mult;
                mult /= 10.0;
            }
        }

        outValue = negative ? -value : value;
        return MatchResultStatus::OK;*/
    }

    template< typename T >
    static bool StringEmpty(const T* a)
    {
        return !a || !*a;
    }

	template< typename T >
    static int StringComapre(const T* a, const T* b, uint32_t length)
    {
        const auto sa = StringEmpty(a) || length == 0;
        const auto sb = StringEmpty(b) || length == 0;

        if (sa || sb)
        {
            if (sa && !sb) return -1;
            if (!sa && sb) return 1;
            return 0;
        }

        while (length-- && (*a || *b))
        {
            if (*a < *b)
                return -1;
			else if (*a > *b)
				return 1;

            a += 1;
            b += 1;
        }

        return 0;
    }

    class TempUTF32StringLowerCase
    {
    public:
        TempUTF32StringLowerCase(StringView txt)
        {
            for (utf8::CharIterator it(txt); it; ++it)
            {
                auto ch = StringView::MapUpperToLowerCaseUTF32(*it);
                chars.pushBack(ch);
            }

            chars.pushBack(0);
        }

        InplaceArray<uint32_t, 1024> chars;
    };

} // prv

//---

StringView theEmptyView;

StringView& StringView::EMPTY()
{
    return theEmptyView;
}

uint32_t StringView::unicodeLength() const
{
    return utf8::Length(m_start, m_end);
}

int StringView::compare(StringView other, StringCaseComparisonMode caseMode /*= StringCaseComparisonMode::WithCase*/) const
{
    const auto count = std::max<uint64_t>(length(), other.length());
    return compareN(other, count, caseMode);
}

int StringView::compareN(StringView other, uint32_t count, StringCaseComparisonMode caseMode /*= StringCaseComparisonMode::WithCase*/) const
{
    if (caseMode == StringCaseComparisonMode::WithCase)
    {
        return prv::StringComapre(data(), other.data(), count);
    }
    else
    {
        prv::TempUTF32StringLowerCase stringA(*this);
        prv::TempUTF32StringLowerCase stringB(other);
        return prv::StringComapre(stringA.chars.typedData(), stringB.chars.typedData(), count);
    }
}

void StringView::slice(char splitChar, Array< StringView >& outTokens, StringSliceFlags flags) const
{
	const bool ignoreQuotes = flags.test(StringSliceBit::IgnoreQuotes);
	const bool ignoreTrim = flags.test(StringSliceBit::IgnoreTrim);
	const bool keepEmpty = flags.test(StringSliceBit::KeepEmpty);

	const char* str = data();
	const char* end = data() + length();
	const char* start = str;
    bool lastCharSeparator = false;
	while (str < end)
	{
		char ch = *str;

		if (!ignoreQuotes && (ch == '\"' || ch == '\''))
		{
			prv::EatEscapedSequence(str, end);
            lastCharSeparator = false;
			continue;
		}

		if (ch == splitChar)
		{
			const auto view = StringView(start, str);
			const auto trimView = view.trim();
			if (keepEmpty || !trimView.empty())
				outTokens.emplaceBack(ignoreTrim ? view : trimView);

			start = str + 1;
            lastCharSeparator = true;
		}
        else
        {
            lastCharSeparator = false;
        }

		str += 1;
	}

	if (start < str || lastCharSeparator)
	{
		const auto view = StringView(start, str);
		const auto trimView = view.trim();
		if (keepEmpty || !trimView.empty())
			outTokens.emplaceBack(ignoreTrim ? view : trimView);
	}
}

void StringView::slice(const char* splitChars, Array< StringView >& outTokens, StringSliceFlags flags) const
{
    const bool ignoreQuotes = flags.test(StringSliceBit::IgnoreQuotes);
	const bool ignoreTrim = flags.test(StringSliceBit::IgnoreTrim);
    const bool keepEmpty = flags.test(StringSliceBit::KeepEmpty);

	const char* str = data();
	const char* end = data() + length();
	const char* start = str;
    bool lastCharSeparator = false;
	while (str < end)
	{
		char ch = *str;

		if (!ignoreQuotes && (ch == '\"' || ch == '\''))
		{
			prv::EatEscapedSequence(str, end);
            lastCharSeparator = false;
			continue;
		}

		if (prv::Contains<char>(splitChars, ch))
		{
            const auto view = StringView(start, str);
            const auto trimView = view.trim();
            if (keepEmpty || !trimView.empty())
                outTokens.emplaceBack(ignoreTrim ? view : trimView);

			start = str + 1;
            lastCharSeparator = true;
		}
        else
        {
            lastCharSeparator = false;
        }

		str += 1;
	}

	if (start < str || lastCharSeparator)
	{
		const auto view = StringView(start, str);
		const auto trimView = view.trim();
		if (keepEmpty || !trimView.empty())
			outTokens.emplaceBack(ignoreTrim ? view : trimView);
	}
}

Index StringView::findStr(StringView pattern, StringCaseComparisonMode caseMode /*= StringCaseComparisonMode::WithCase*/, int firstPosition /*= 0*/) const
{
	if (firstPosition + (int)pattern.length() >= (int)length())
		return -1;

	auto ret = (caseMode == StringCaseComparisonMode::WithCase) 
        ? prv::Find(data() + firstPosition, length() - firstPosition, pattern.data(), pattern.length())
        : prv::FindNoCase(data() + firstPosition, length() - firstPosition, pattern.data(), pattern.length());

	if (ret != -1)
		ret += firstPosition;

	return range_cast<int>(ret);
}

Index StringView::findRevStr(StringView pattern, StringCaseComparisonMode caseMode /*= StringCaseComparisonMode::WithCase*/, int firstPosition /*= 0*/) const
{
	auto searchLimit = std::min<int64_t>(firstPosition, length());

    if (caseMode == StringCaseComparisonMode::WithCase)
	    return prv::FindRev(data(), searchLimit, pattern.data(), pattern.length());
    else
        return prv::FindRevNoCase(data(), searchLimit, pattern.data(), pattern.length());
}

int StringView::findFirstChar(char ch) const
{
	auto pos = m_start;
	while (pos < m_end)
	{
		if (*pos == ch)
			return pos - m_start;
		pos += 1;
	}

	return -1;
}

int StringView::findLastChar(char ch) const
{
	auto pos = m_end - 1;
	while (pos >= m_start)
	{
		if (*pos == ch)
			return pos - m_start;
		pos -= 1;
	}

	return -1;
}

static bool IsWhiteSpace(char ch)
{
    return (ch >= 0 && ch <= 32);
}

StringView StringView::trimLeft() const
{
	auto pos = m_start;
	while (pos < m_end)
	{
		if (!IsWhiteSpace(*pos))
			break;
		pos += 1;
	}

	return StringView(pos, m_end);
}

StringView StringView::trimRight() const
{
	auto pos = m_end;
	while (pos > m_start)
	{
		if (!IsWhiteSpace(pos[-1]))
			break;
		pos -= 1;
	}

	return StringView(m_start, pos);
}

StringView StringView::trim() const
{
	return trimLeft().trimRight();
}

StringView StringView::trimQuotes() const
{
    if (length() >= 2)
    {
        if ((m_start[0] == '\"' && m_end[-1] == '\"') || (m_start[0] == '\'' && m_end[-1] == '\''))
            return StringView(m_start + 1, m_end - 1);
    }

    return *this;
}

StringView StringView::trimTailNumbers(uint32_t* outNumber) const
{
	auto pos = m_end;
	while (pos > m_start)
	{
		if (!(pos[-1] >= '0' && pos[-1] <= '9'))
			break;
		pos -= 1;
	}

	if (pos < m_end && outNumber)
		StringView(pos, m_end).match(*outNumber);

	return StringView(m_start, pos);
}

//--

StringView StringView::pathFileName() const
{
	const char* fileNameStart = m_start;

	const char* ptr = m_start;
	while (ptr < m_end)
	{
		if (*ptr == '\\' || *ptr == '/')
			fileNameStart = ptr + 1;
		++ptr;
	}

	return StringView(fileNameStart, m_end);
}

StringView StringView::pathFileStem() const
{
    const auto fileName = pathFileName();
    if (!fileName.beginsWith("."))
    {
        const auto pos = fileName.findFirstChar('.');
        return (pos == INDEX_NONE) ? fileName : fileName.leftPart(pos);
    }
    else
    {
		const auto pos = fileName.subString(1).findFirstChar('.');
		return (pos == INDEX_NONE) ? fileName : fileName.leftPart(pos+1);
    }        
}

StringView StringView::pathFileExtensions() const
{
	const char dot[2] = { '.', 0 };
    const auto fileName = pathFileName();
    if (!fileName.beginsWith("."))
        return fileName.afterFirst(dot);
    else
        return "";
}

StringView StringView::pathStripExtensions() const
{
    const auto extensions = pathFileExtensions();
    return extensions ? StringView(m_start, extensions.m_start - 1) : StringView(*this);
}

StringView StringView::pathFileLastExtension() const
{
	const char dot[2] = { '.', 0 };
	return pathFileName().afterLast(dot);
}

StringView StringView::pathParent() const
{
    if (empty())
        return "";

    const auto* testEnd = m_end;
    if (m_end[-1] == '/' || m_end[-1] == '\\')
        testEnd -= 1;

	const char* pathEnd = nullptr;

	const char* ptr = m_start;
	while (ptr < testEnd)
	{
		if (*ptr == '\\' || *ptr == '/')
			pathEnd = ptr + 1;
		++ptr;
	}

	if (pathEnd)
		return StringView(m_start, pathEnd);
	else
		return StringView();
}

/*StringView StringView::parentDirectory() const
{
	const char* pathEnd = nullptr;
	const char* prevPathEnd = nullptr;

	const char* ptr = m_start;
	while (ptr < m_end)
	{
		if (*ptr == '\\' || *ptr == '/')
		{
			prevPathEnd = pathEnd;
			pathEnd = ptr + 1;
		}
		++ptr;
	}

	if (prevPathEnd)
		return StringView(m_start, prevPathEnd);
	else
		return StringView();
}*/

StringView StringView::pathDirectoryName() const
{
	const char* pathEnd = m_start;
	const char* prevPathEnd = m_start;

	const char* ptr = m_start;
	while (ptr < m_end)
	{
		if (*ptr == '\\' || *ptr == '/')
		{
			prevPathEnd = pathEnd;
			pathEnd = ptr + 1;
		}
		++ptr;
	}

	if (pathEnd > prevPathEnd)
		return StringView(prevPathEnd, pathEnd - 1);
	else
		return StringView();
}

bool StringView::matchString(StringView pattern, StringCaseComparisonMode caseMode) const
{
    if (caseMode == StringCaseComparisonMode::WithCase)
	    return prv::MatchString(*this, pattern);
    else
		return prv::MatchStringNoCase(*this, pattern);
}

bool StringView::matchPattern(StringView pattern, StringCaseComparisonMode caseMode) const
{
	if (caseMode == StringCaseComparisonMode::WithCase)
		return prv::MatchPattern(*this, pattern);
	else
		return prv::MatchPatternNoCase(*this, pattern);
}

MatchResult StringView::match(bool& outValue) const
{
	if (*this == "true")
	{
		outValue = true;
		return MatchResultStatus::OK;
	}
	else if (*this == "false")
	{
		outValue = false;
		return MatchResultStatus::OK;
	}
    else
    {
        int val = 0;
        if (match(val))
        {
            outValue = val != 0;
            return MatchResultStatus::OK;
        }
    }

	return MatchResultStatus::InvalidCharacter;
}

MatchResult StringView::match(uint8_t& outValue) const
{
	return prv::AMatchInteger(data(), outValue, length());
}

MatchResult StringView::match(uint16_t& outValue) const
{
	return prv::AMatchInteger(data(), outValue, length());
}

MatchResult StringView::match(uint32_t& outValue) const
{
	return prv::AMatchInteger(data(), outValue, length());
}

MatchResult StringView::match(uint64_t& outValue) const
{
	return prv::AMatchInteger(data(), outValue, length());
}

MatchResult StringView::match(char& outValue) const
{
	return prv::AMatchInteger(data(), outValue, length());
}

MatchResult StringView::match(short& outValue) const
{
	return prv::AMatchInteger(data(), outValue, length());
}

MatchResult StringView::match(int& outValue) const
{
	return prv::AMatchInteger(data(), outValue, length());
}

MatchResult StringView::match(int64_t& outValue) const
{
	return prv::AMatchInteger(data(), outValue, length());
}

MatchResult StringView::match(float& outValue) const
{
	double result = 0.0f;
	const auto ret = prv::AMatchFloat(data(), result, length());
	if (ret)
		outValue = (float)result;
	return ret;
}

MatchResult StringView::match(double& outValue) const
{
	return prv::AMatchFloat(data(), outValue, length());
}

MatchResult StringView::match(StringID& outValue) const
{
    const auto trimmed = StringView(*this).trim().trimQuotes();
	outValue = StringID(trimmed);
	return MatchResultStatus::OK;
}

MatchResult StringView::match(StringBuf& outValue) const
{
	const auto trimmed = StringView(*this).trim().trimQuotes();
	outValue = StringBuf(trimmed);
	return MatchResultStatus::OK;
}

uint32_t StringView::CalcHash(StringView txt)
{
	return prv::StringHash(txt.m_start, txt.m_end);
}

uint32_t StringView::evaluateCRC32(const uint32_t crc/*= CRC32Init*/) const
{
	return prv::StringCRC32(m_start, m_end, crc);
}

uint64_t StringView::evaluateCRC64(const uint64_t crc/*= CRC64Init*/) const
{
	return prv::StringCRC64(m_start, m_end, crc);
}

//--

void StringView::encode(EncodingType et, IFormatStream& f) const
{
	bufferView().encode(et, f);
}

bool StringView::encode(EncodingType et, StringBuf& outString) const
{
	const auto length = bufferView().estimateEncodedSize(et);
	DEBUG_CHECK_RETURN_EX_V(length, "Invalid encoding", false);

	const auto ret = StringBuf(length);
	DEBUG_CHECK_RETURN_EX_V(ret, "Out of memory", false);

    BufferOutputStream<char> output((char*)ret.c_str(), ret.length());
	DEBUG_CHECK_RETURN_EX_V(bufferView().encode(et, output), "Encoding error", false);

	outString = ret;
	return true;
}

bool StringView::encode(EncodingType et, Buffer& outBuffer, IPoolUnmanaged& pool /*= MainPool()*/) const
{
	const auto length = bufferView().estimateEncodedSize(et);
	DEBUG_CHECK_RETURN_EX_V(length, "Invalid encoding", false);

	const auto ret = Buffer::CreateEmpty(pool, length, 1);
	DEBUG_CHECK_RETURN_EX_V(ret, "Out of memory", false);

    BufferOutputStream<char> output(ret);
	DEBUG_CHECK_RETURN_EX_V(bufferView().encode(et, output), "Encoding error", false);

	outBuffer = ret;
	return true;
}

bool StringView::encode(EncodingType et, BufferOutputStream<char>& output) const
{
    return bufferView().encode(et, output);
}

StringBuf StringView::encode(EncodingType et) const
{
	StringBuf ret;
	if (encode(et, ret))
		return ret;

	return nullptr;
}

bool StringView::decode(IPoolUnmanaged& pool, Buffer& outBuffer, EncodingType et, bool allowWhiteSpaces /*= true*/, uint32_t alignment /*= 16*/) const
{
    if (empty())
    {
        outBuffer = nullptr;
        return true;
    }

	const auto neededMemory = bufferView().estimateDecodedSize(et);
	DEBUG_CHECK_RETURN_EX_V(neededMemory, "Invalid URL encoding", false);

	const auto ret = Buffer::CreateEmpty(pool, neededMemory, alignment);
	DEBUG_CHECK_RETURN_EX_V(ret, "Out of memory", false);

    BufferOutputStream<uint8_t> output(ret);
    if (!bufferView().decode(et, output, allowWhiteSpaces))
        return false;

    outBuffer = ret;
	return ret;
}

Buffer StringView::decode(IPoolUnmanaged& pool, EncodingType et, bool allowWhiteSpaces /*= true*/, uint32_t alignment /*= 16*/) const
{
    Buffer ret;
    if (decode(pool, ret, et, allowWhiteSpaces, alignment))
        return ret;

    return nullptr;
}

bool StringView::decode(EncodingType et, StringBuf& outString, bool allowWhiteSpaces /*= true*/) const
{
    if (empty())
    {
        outString = StringBuf::EMPTY();
        return true;
    }

	const auto neededMemory = bufferView().estimateDecodedSize(et);
	DEBUG_CHECK_RETURN_EX_V(neededMemory, "Invalid URL encoding", false);

	const auto ret = StringBuf(neededMemory);
	DEBUG_CHECK_RETURN_EX_V(ret, "Out of memory", false);

    BufferOutputStream<uint8_t> output((uint8_t*)ret.c_str(), ret.length());
	if (!bufferView().decode(et, output, allowWhiteSpaces))
		return false;

	outString = ret;
	return true;
}

StringBuf StringView::decode(EncodingType et, bool allowWhiteSpaces /*= true*/) const
{
	StringBuf ret;
	if (decode(et, ret, allowWhiteSpaces))
		return ret;

	return nullptr;
}

//--

Buffer StringView::toBuffer(IPoolUnmanaged& pool /*= MainPool()*/) const
{
    return Buffer::CreateFromCopy(pool, bufferView(), 1);
}

Buffer StringView::toBufferZeroTerminated(IPoolUnmanaged& pool /*= MainPool()*/) const
{
    const auto len = length();

    auto ret = Buffer::CreateEmpty(pool, len + 1, 1);
    DEBUG_CHECK_RETURN_EX_V(ret, "Out of memory", nullptr);

    memcpy(ret.data(), data(), len);
    ret.data()[len] = 0;

    return ret;
}

Buffer StringView::toBufferEncoded(EncodingType encoding, IPoolUnmanaged& pool /*= MainPool()*/) const
{
    Buffer ret;
    encode(encoding, ret);
    return ret;
}

//--

void StringView::iterateUTF32(const std::function<void(uint32_t)>& func) const
{
	const auto* str = data();
	const auto* strEnd = data() + length();
    while (str < strEnd)
    {
        const auto ch = utf8::GetChar(str, strEnd);
        if (!ch)
            break;

        func(ch);
    }
}

Array<wchar_t> StringView::exportUTF16() const
{
    Array<wchar_t> ret;
    ret.reserve(length());

    iterateUTF32([&ret](uint32_t ch)
        {
            ret.pushBack((wchar_t)ch);
        });

    return ret;
}

Array<uint32_t> StringView::exportUTF32() const
{
	Array<uint32_t> ret;
	ret.reserve(length());

	iterateUTF32([&ret](uint32_t ch)
		{
            ret.pushBack(ch);
		});

	return ret;
}

//--

// TODO!!! copy those tables from somewhere...

uint32_t StringView::MapLowerToUpperCaseUTF32(uint32_t ch)
{
	if (ch >= 'a' && ch <= 'z')
		return 'A' + (ch - 'a');
	return ch;
}

uint32_t StringView::MapUpperToLowerCaseUTF32(uint32_t ch)
{
	if (ch >= 'A' && ch <= 'Z')
		return 'a' + (ch - 'A');
	return ch;
}

//--

namespace helper
{
	struct Region
	{
		const char* m_start;
		const char* m_end;
		StringView m_name;
		uint64_t m_hash;
	};

	static bool IsInsertionPointMarkerStart(const char* str)
	{
		return (str[0] == '{') && (str[1] == '$');
	}

	static bool IsInsertionPointMarkerEnd(const char* str)
	{
		return (str[0] == '$') && (str[1] == '}');
	}

	static bool ExtractInsertionPointName(const char*& str, StringView& outName)
	{
		auto start = str;
		while (*str)
		{
			if (IsInsertionPointMarkerEnd(str))
			{
				outName = StringView(start, str);
				str += 3;
				return true;
			}

			str += 1;
		}

		return false;
	}

	static void SplitIntoInsertionRegions(StringView templateText, Array<Region>& outRegions)
	{
		auto str = templateText.data();
		auto endStr = str + templateText.length();

		while (str < endStr)
		{
			// find the insertion point
			auto start = str;
			auto end = str;
			StringView name;
			while (str < endStr)
			{
				if (IsInsertionPointMarkerStart(str))
				{
					end = str;

					str += 2;
					ExtractInsertionPointName(str, name);
					break;
				}

				end = str;
				str += 1;
			}

			// create region
			auto& region = outRegions.emplaceBack();
			region.m_start = start;
			region.m_end = end;
			region.m_name = name;
			region.m_hash = name.evaluateCRC64();
		}
	}

} // helper

void StringView::replaceText(IFormatStream& f, ArrayView<StringReplaceSetup> patterns) const
{
	// split into regions
	InplaceArray<helper::Region, 100> regions;
	helper::SplitIntoInsertionRegions(*this, regions);

	// assemble code
	for (auto& region : regions)
	{
		// append block text
		auto length = range_cast<uint32_t>(region.m_end - region.m_start);
		f.append(region.m_start, length);

		// find the text for the insertion point
		if (!region.m_name)
		{
			for (const auto& pattern : patterns)
			{
				if (0 == pattern.name.compare(region.m_name, pattern.caseMode))
				{
                    f << pattern.data;
					break;
				}
			}
		}
	}
}

///---

END_INFERNO_NAMESPACE()
