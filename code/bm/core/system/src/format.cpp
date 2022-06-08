/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "format.h"

#include <stdarg.h>

BEGIN_INFERNO_NAMESPACE()

///---

namespace prv
{

    static void ReverseInplace(char* ptr)
    {
        if (*ptr)
        {
            auto end = ptr + strlen(ptr) - 1;
            while (ptr < end)std::swap(*ptr++, *end--);
        }
    }

    uint8_t ConvertChar(char* dest, uint32_t ch)
    {
        if (ch < 0x80)
        {
            dest[0] = (char)ch;
            return 1;
        }
        else if (ch < 0x800)
        {
            dest[0] = (char)((ch >> 6) | 0xC0);
            dest[1] = (char)((ch & 0x3F) | 0x80);
            return 2;
        }
        else if (ch < 0x10000)
        {
            dest[0] = (char)((ch >> 12) | 0xE0);
            dest[1] = (char)(((ch >> 6) & 0x3F) | 0x80);
            dest[2] = (char)((ch & 0x3F) | 0x80);
            return 3;
        }
        else if (ch < 0x110000)
        {
            dest[0] = (char)((ch >> 18) | 0xF0);
            dest[1] = (char)(((ch >> 12) & 0x3F) | 0x80);
            dest[2] = (char)(((ch >> 6) & 0x3F) | 0x80);
            dest[3] = (char)((ch & 0x3F) | 0x80);
            return 4;
        }
        return 0;
    }

	static const char* BASE10_DIGITS = "0123456789";
    static const char* BASE16_DIGITS_SMALL = "0123456789abcdef";
    static const char* BASE16_DIGITS_BIG =   "0123456789ABCDEF";

    struct SimpleNumberPrinter : public NoCopy
    {
        static const uint32_t MAX_CHARS = 256;

        char m_txt[MAX_CHARS];
        int m_pos = 0;

        ALWAYS_INLINE SimpleNumberPrinter()
        {
            m_txt[0] = 0;
        }

        ALWAYS_INLINE void print(uint64_t val)
        {
            do
            {
                m_txt[m_pos++] = BASE10_DIGITS[val % 10];
                val /= 10;
            } while (val);
        }

        ALWAYS_INLINE void pad(char sign, char pad, int count)
        {
            if (pad == '0' && sign)
            {
                count -= 1;
                while (m_pos < count)
                    m_txt[m_pos++] = pad;
                m_txt[m_pos++] = sign;                 

            }
            else if (pad)
            {
				if (sign)
					m_txt[m_pos++] = sign;

				while (m_pos < count)
					m_txt[m_pos++] = pad;                
            }
            else if (sign)
            {
                m_txt[m_pos++] = sign; // just add sign if it's there
            }
        }

        ALWAYS_INLINE void sign(char ch)
        {
            if (ch)
                m_txt[m_pos++] = ch;
        }

        ALWAYS_INLINE void write(IFormatStream& f)
        {
            std::reverse(m_txt, m_txt + m_pos);
            f.append(m_txt, m_pos);
        }
    };

    struct FractionValuePrinter : public NoCopy
    {
		static const uint32_t MAX_CHARS = 64;

		char m_txt[MAX_CHARS];
		char m_pos = 0;

		ALWAYS_INLINE FractionValuePrinter()
		{
			m_txt[0] = 0;
		}

		ALWAYS_INLINE void printPrec(double val, char maxLength = 30)
		{
            bool hadNonZero = false;

			val *= 10.0;
			if (val > 0.0 && m_pos < maxLength)
			{
				m_txt[m_pos++] = '.';
				maxLength += 1;

				while (val && m_pos < maxLength)
				{
                    int digit = (int)val;
                    if (digit != 0)
                        hadNonZero = true;

					m_txt[m_pos++] = BASE10_DIGITS[(int)val];
					val -= (int)val;
					val *= 10.0;

                    if (m_pos < MAX_CHARS && hadNonZero)
                        maxLength += 1;
				}
			}
		}

		ALWAYS_INLINE void printAuto(double val, char maxLength = 30)
		{
			val *= 10.0;
			if (val > 0.0 && m_pos < maxLength)
			{
				m_txt[m_pos++] = '.';
                maxLength += 1;

				while (val && m_pos < maxLength)
				{
					m_txt[m_pos++] = BASE10_DIGITS[(int)val];
                    val -= (int)val;
					val *= 10.0;
				}
			}
		}

		ALWAYS_INLINE void printCustom(double val, char maxLength = 30)
		{
			val *= 10.0;
			if (m_pos < maxLength)
			{
				m_txt[m_pos++] = '.';
				maxLength += 1;

				while (m_pos < maxLength)
				{
					m_txt[m_pos++] = BASE10_DIGITS[(int)val];
					val -= (int)val;
					val *= 10.0;
				}
			}
		}

        ALWAYS_INLINE void write(IFormatStream& f)
        {
            f.append(m_txt, m_pos);
        }
    };

	struct HexBlockPrinter : public NoCopy
	{
		static const uint32_t MAX_CHARS = 512;

		char m_txt[MAX_CHARS+1];
		char m_pos = 0;

		ALWAYS_INLINE HexBlockPrinter()
		{
			m_txt[0] = 0;
		}

		ALWAYS_INLINE void printSmall(const void* data, uint32_t length)
		{
            const auto* ptr = (const uint8_t*)data;
            const auto* end = (const uint8_t*)data + length;
            while (ptr < end)
            {
                m_txt[m_pos++] = BASE16_DIGITS_SMALL[*ptr >> 4];
                m_txt[m_pos++] = BASE16_DIGITS_SMALL[*ptr & 0xF];
                ++ptr;
            }
		}

		ALWAYS_INLINE void printBig(const void* data, uint32_t length)
		{
			const auto* ptr = (const uint8_t*)data;
			const auto* end = (const uint8_t*)data + length;
			while (ptr < end)
			{
				m_txt[m_pos++] = BASE16_DIGITS_BIG[*ptr >> 4];
				m_txt[m_pos++] = BASE16_DIGITS_BIG[*ptr & 0xF];
				++ptr;
			}
		}

		ALWAYS_INLINE void write(IFormatStream& f)
		{
			f.append(m_txt, m_pos);
		}
	};

    struct HexNumberPrinter : public NoCopy
	{
		static const uint32_t MAX_CHARS = 32;

		char m_txt[MAX_CHARS + 1];
		char m_pos = 0;

		ALWAYS_INLINE HexNumberPrinter()
		{
			m_txt[0] = 0;
		}

		ALWAYS_INLINE void printSmall(uint64_t val)
		{
			do
			{
				m_txt[m_pos++] = BASE16_DIGITS_SMALL[val & 0xF];
                val >>= 4;
            } while (val);
		}

		ALWAYS_INLINE void printBig(uint64_t val)
		{
			do
			{
				m_txt[m_pos++] = BASE16_DIGITS_BIG[val & 0xF];
				val >>= 4;
			} while (val);
		}

		ALWAYS_INLINE void pad(char pad, int count)
		{
			if (pad)
			{
				while (m_pos < count)
					m_txt[m_pos++] = pad;
			}			
		}

		ALWAYS_INLINE void write(IFormatStream& f)
		{
            std::reverse(m_txt, m_txt + m_pos);
			f.append(m_txt, m_pos);
		}

    };

	static int DigitValue(char ch)
	{
        switch (ch)
        {
        case '0': return 0;
        case '1': return 1;
        case '2': return 2;
        case '3': return 3;
        case '4': return 4;
        case '5': return 5;
        case '6': return 6;
        case '7': return 7;
        case '8': return 8;
        case '9': return 9;
        }

        return -1;
	}

} // prv

///---

IFormatStream::~IFormatStream()
{}

void IFormatStream::PrintMemorySize(IFormatStream& f, uint64_t value)
{
	static const double GB_SIZE = 1.0 / (double)(1ULL << 30);
	static const double MB_SIZE = 1.0 / (double)(1ULL << 20);
	static const double KB_SIZE = 1.0 / (double)(1ULL << 10);

    if (value >= (1ULL << 30))
        f.appendf("{} GB", PaddedFloat<1, 2>(value * GB_SIZE));
    else if (value >= (1ULL << 20))
        f.appendf("{} MB", PaddedFloat<1, 2>(value * MB_SIZE));
    else if (value >= (1ULL << 10))
        f.appendf("{} KB", PaddedFloat<1, 2>(value * KB_SIZE));
	else
        f.appendf("{} bytes", value);
}

void IFormatStream::PrintTimeInterval(IFormatStream& f, double value)
{
    static const double HOUR = 3600.0f;
	static const double MIN = 60.0f;
	static const double SEC = 1.0f;
	static const double MS = 0.0001f;
	static const double US = 0.0000001f;

    if (value >= MIN)
    {
        const auto integerSeconds = (int64_t)value;
        const auto numHours = integerSeconds / 3600;
        const auto numMinutes = (integerSeconds / 60) % 60;
        const auto numSeconds = (integerSeconds) % 60;
        const auto miliseconds = value - integerSeconds;

        if (numHours)
            f.appendf("{}:{}:{}", PaddedInteger<2, '0'>(numHours), PaddedInteger<2, '0'>(numMinutes), PaddedInteger<2,'0'>(numSeconds));
        else
            f.appendf("{}:{}.{}", PaddedInteger<2, '0'>(numMinutes), PaddedInteger<2, '0'>(numSeconds), miliseconds);
    }
    else if (value >= SEC)
    {
        f.appendf("{} s", PaddedFloat<1,2>(value));
    }
    else if (value >= MS)
    {
        f.appendf("{} ms", PaddedFloat<1, 2>(value * 1000.0));
    }
	else if (value >= US)
	{
		f.appendf("{} us", PaddedFloat<1, 2>(value * 1000000.0));
	}
	else
	{
		f.appendf("{} ns", PaddedFloat<1, 2>(value * 1000000000.0));
	}
}

void IFormatStream::PrintNumber(IFormatStream& f, char sign, uint64_t val, char paddingChar, int wholeDigits)
{
	prv::SimpleNumberPrinter number;
    number.print(val);

    int digitCount = 0;
    if (wholeDigits >= 0)
        number.pad(sign, paddingChar, wholeDigits);
    else
        number.sign(sign);

    number.write(f);
}

void IFormatStream::PrintNumberF(IFormatStream& f, char sign, double val, char paddingChar, int wholeDigits, int fractionDigits)
{
    uint64_t wholeNumber = (uint64_t)val;
    val -= (double)wholeNumber; // fraction

	prv::SimpleNumberPrinter wholeNumberPrinter;
    wholeNumberPrinter.print(wholeNumber);

    if (wholeDigits >= 0)
    {
        wholeNumberPrinter.pad(sign, paddingChar, wholeDigits);
        wholeNumberPrinter.write(f);

        if (fractionDigits >= 0)
        {
            prv::FractionValuePrinter fractionPrinter;
            fractionPrinter.printCustom(val, fractionDigits);
            fractionPrinter.write(f);
        }
    }
    else
    {
        wholeNumberPrinter.sign(sign);
        wholeNumberPrinter.write(f);

		prv::FractionValuePrinter fractionPrinter;
		fractionPrinter.printAuto(val, 6);
		fractionPrinter.write(f);
    }
}

void IFormatStream::PrintPreciseNumber(IFormatStream& f, float val)
{
#if 1
    char buffer[128];
    sprintf_s(buffer, "%.9g", val);
    f << buffer;
#else

    char sign = 0;
    if (val < 0.0f) { val = -val; sign = '-'; }

	uint64_t wholeNumber = (uint64_t)val;
	val -= (double)wholeNumber; // fraction

	prv::SimpleNumberPrinter wholeNumberPrinter;
	wholeNumberPrinter.print(wholeNumber);

    const auto fractionDigits = std::max<int>(0, 9 - wholeNumberPrinter.m_pos);

	wholeNumberPrinter.sign(sign);
	wholeNumberPrinter.write(f);

    if (fractionDigits > 0)
    {
		prv::FractionValuePrinter fractionPrinter;
		fractionPrinter.printPrec(val, fractionDigits);
		fractionPrinter.write(f);
    }  
#endif
}

void IFormatStream::PrintPreciseNumber(IFormatStream& f, double val)
{
#if 1
	char buffer[128];
	sprintf_s(buffer, "%.17g", val);
	f << buffer;
#else
	char sign = 0;
    if (val < 0.0f) { val = -val; sign = '-'; }

	uint64_t wholeNumber = (uint64_t)val;
	val -= (double)wholeNumber; // fraction

	prv::SimpleNumberPrinter wholeNumberPrinter;
	wholeNumberPrinter.print(wholeNumber);

	const auto fractionDigits = std::max<int>(0, 18 - wholeNumberPrinter.m_pos);

	wholeNumberPrinter.sign(sign);
	wholeNumberPrinter.write(f);
	if (fractionDigits > 0)
	{
		prv::FractionValuePrinter fractionPrinter;
		fractionPrinter.printPrec(val, fractionDigits);
		fractionPrinter.write(f);
	}
#endif
}

void IFormatStream::PrintPointer(IFormatStream& f, const void* ptr)
{
    if (!ptr)
    {
        f << "null";
    }
    else if (sizeof(void*) == 4)
    {
        f << "0x";

        prv::HexNumberPrinter address;
        address.printSmall((uint32_t)ptr);
        address.pad('0', 8);
        address.write(f);
    }
	else if (sizeof(void*) == 16)
	{
		f << "0x";

		prv::HexNumberPrinter address;
		address.printSmall((uint64_t)ptr);
		address.pad('0', 8); // minimum 8 but do not print full addresses
		address.write(f);
	}
}

void IFormatStream::PrintHexNumber(IFormatStream& f, uint64_t value, char paddingChar, int wholeDigits)
{
	prv::HexNumberPrinter address;
	address.printSmall(value);

    if (wholeDigits >= 0)
        address.pad(paddingChar, wholeDigits);

	address.write(f);
}

void IFormatStream::PrintHexData(IFormatStream& f, const void* ptr, uint32_t size)
{
    const auto* start = (const uint8_t*)ptr;
    const auto* end = start + size;

    while (start < end)
    {
        const auto val = *start++;

        char ch[3];
        ch[0] = prv::BASE16_DIGITS_SMALL[val >> 4];
        ch[1] = prv::BASE16_DIGITS_SMALL[val & 0xF];
        ch[2] = 0;
        f.append(ch);
    }
}

IFormatStream& IFormatStream::append(const wchar_t* str, uint32_t len)
{
    // NOTE: unicode conversion is not efficient but on the other hand it's also not very common
    // TODO: preallocate all needed memory in one go
    auto end = (len == INDEX_MAX) ? (str + wcslen(str)) : (str + len);
    while (str < end)
    {
        char buf[6];
        auto len = prv::ConvertChar(buf, *str++);
        append(buf, len);
    }

    return *this;
}

IFormatStream& IFormatStream::appendUTF32(uint32_t ch)
{
    char buf[7];
    auto len = prv::ConvertChar(buf, ch);
    append(buf, len);

    return *this;
}
    
IFormatStream& IFormatStream::appendPadding(char ch, uint32_t count)
{
    auto maxPadding = std::min<uint32_t>(count, 256);

    char str[256];
    memset(str, ch, maxPadding);

    return append(str, maxPadding);
}

///---

class DevNullStream : public IFormatStream
{
public:
    virtual IFormatStream& append(const char* str, uint32_t len) { return *this; }
};

IFormatStream& IFormatStream::NullStream()
{
    static DevNullStream theNullStream;
    return theNullStream;
}

///---

bool IFormatStream::consumeFormatString(const char*& pos)
{
    auto start = pos;
    while (*pos)
    {
        if (pos[0] == '{' && pos[1] == '}')
        {
            append(start, range_cast<uint32_t>(pos - start));
            pos += 2;
            return true;
        }

        // just a normal char
        ++pos;
    }

    // no more argument injections found
    append(start, range_cast<uint32_t>(pos - start));
    return false;
}

//---

END_INFERNO_NAMESPACE()
