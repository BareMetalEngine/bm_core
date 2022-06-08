/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "utf8StringFunctions.h"

BEGIN_INFERNO_NAMESPACE()

namespace utf8
{

    static const uint32_t OffsetsFromUTF8[6] =
    {
        0x00000000UL, 
        0x00003080UL, 
        0x000E2080UL,
        0x03C82080UL, 
        0xFA082080UL, 
        0x82082080UL
    };

    static const uint8_t TrailingBytesForUTF8[256] =
    {
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 
        3,3,3,3,3,3,3,3,4,4,4,4,5,5,5,5
    };

	static const int TrailingBytesForUTF8Ex[256] =
	{
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 0000xxxx
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 0001xxxx
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 0010xxxx
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 0011xxxx
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 0100xxxx
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 0101xxxx
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 0110xxxx
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 0111xxxx
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  // 1000xxxx
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  // 1001xxxx
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  // 1010xxxx
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  // 1011xxxx
		1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  // 1100xxxx
		1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  // 1101xxxx
		2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,  // 1110xxxx
		3,3,3,3,3,3,3,3, // 11110xxx
        4,4,4,4, // 111110xx
        5,5, // 1111110x
        -1,-1  // 1111110x
	};

    size_t ToUniChar(wchar_t* dest, size_t sz, const char* src, size_t srcsz)
    {
        size_t i = 0;

        if (!sz)
            return 0;

        auto src_end  = src + srcsz;
        while (i < sz - 1)
        {
            uint32_t nb = TrailingBytesForUTF8[(unsigned char)*src];
            if (srcsz == ~0U)
            {
                if (*src == 0)
                    break;
            }
            else
            {
                if (src + nb >= src_end)
                    break;
            }
            uint32_t ch = 0;
            switch (nb)
            {
                /* these fall through deliberately */
                case 3: ch += (uint8_t)*src++; ch <<= 6;
                case 2: ch += (uint8_t)*src++; ch <<= 6;
                case 1: ch += (uint8_t)*src++; ch <<= 6;
                case 0: ch += (uint8_t)*src++;
            }
            ch -= OffsetsFromUTF8[nb];
            dest[i++] = ch <= 0xFFFF ? (wchar_t)ch : '?';
        }

        dest[i] = 0;
        return i;
    }

    size_t ToUniChar32(uint32_t* dest, size_t sz, const char* src, size_t srcsz)
    {
        size_t i = 0;

		if (!sz)
			return 0;

        auto src_end  = src + srcsz;
        while (i < sz - 1)
        {
            auto nb = TrailingBytesForUTF8[(unsigned char)*src];
            if (srcsz == ~0U)
            {
                if (*src == 0)
                    break;
            }
            else
            {
                if (src + nb >= src_end)
                    break;
            }
            uint32_t ch = 0;
            switch (nb)
            {
                /* these fall through deliberately */
                case 3: ch += (uint8_t)*src++; ch <<= 6;
                case 2: ch += (uint8_t)*src++; ch <<= 6;
                case 1: ch += (uint8_t)*src++; ch <<= 6;
                case 0: ch += (uint8_t)*src++;
            }
            ch -= OffsetsFromUTF8[nb];
            dest[i++] = ch;
        }
        dest[i] = 0;
        return i;
    }


    size_t FromUniChar(char* dest, size_t sz, const wchar_t* src, size_t srcsz)
    {
        if (!sz)
            return 0;

        auto dest_start  = dest;
        size_t i = 0;
        auto dest_end  = dest + sz;
        while (i < srcsz)
        {
            uint32_t ch = src[i];

            if (ch < 0x80)
            {
                if ((dest + 1) >= dest_end)
                    break;
                *dest++ = (char)ch;
            }
            else if (ch < 0x800)
            {
				if ((dest + 2) >= dest_end)
					break;
                *dest++ = (char)((ch >> 6) | 0xC0);
                *dest++ = (char)((ch & 0x3F) | 0x80);
            }
            else if (ch < 0x10000)
            {
				if ((dest + 3) >= dest_end)
					break;
                *dest++ = (char)((ch >> 12) | 0xE0);
                *dest++ = (char)(((ch >> 6) & 0x3F) | 0x80);
                *dest++ = (char)((ch & 0x3F) | 0x80);
            }
            else if (ch < 0x110000)
            {
				if ((dest + 4) >= dest_end)
					break;
                *dest++ = (char)((ch >> 18) | 0xF0);
                *dest++ = (char)(((ch >> 12) & 0x3F) | 0x80);
                *dest++ = (char)(((ch >> 6) & 0x3F) | 0x80);
                *dest++ = (char)((ch & 0x3F) | 0x80);
            }
            i++;
        }
        if (dest < dest_end)
            *dest = '\0';
        return (uint32_t)(dest - dest_start);
    }

    size_t FromUniChar32(char* dest, size_t sz, const uint32_t* src, size_t srcsz)
    {
        if (!sz)
            return 0;

        auto dest_start  = dest;
        size_t i = 0;
        auto dest_end  = dest + sz;
        while (i < srcsz)
        {
            uint32_t ch = src[i];

            if (ch < 0x80)
            {
				if ((dest + 1) >= dest_end)
					break;
                *dest++ = (char)ch;
            }
            else if (ch < 0x800)
            {
				if ((dest + 2) >= dest_end)
					break;
                *dest++ = (char)((ch >> 6) | 0xC0);
                *dest++ = (char)((ch & 0x3F) | 0x80);
            }
            else if (ch < 0x10000)
            {
				if ((dest + 3) >= dest_end)
					break;
                *dest++ = (char)((ch >> 12) | 0xE0);
                *dest++ = (char)(((ch >> 6) & 0x3F) | 0x80);
                *dest++ = (char)((ch & 0x3F) | 0x80);
            }
            else if (ch < 0x110000)
            {
				if ((dest + 4) >= dest_end)
					break;
                *dest++ = (char)((ch >> 18) | 0xF0);
                *dest++ = (char)(((ch >> 12) & 0x3F) | 0x80);
                *dest++ = (char)(((ch >> 6) & 0x3F) | 0x80);
                *dest++ = (char)((ch & 0x3F) | 0x80);
            }
            i++;
        }
        if (dest < dest_end)
            *dest = '\0';
        return (uint32_t)(dest - dest_start);
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

    size_t FindByteOffset(const char* str, size_t charIndex)
    {
        size_t offs = 0;

        size_t index = charIndex;
        while (index > 0 && str[offs])
        {
            (void)(IsUTF8(str[++offs]) || IsUTF8(str[++offs]) || IsUTF8(str[++offs]) || ++offs);
            --index;
        }
        return offs;
    }

    size_t FindCharOffset(const char* str, size_t byteOffset)
    {
        size_t charnum = 0;
        size_t offs = 0;

        while (offs < byteOffset && str[offs])
        {
            (void)(IsUTF8(str[++offs]) || IsUTF8(str[++offs]) || IsUTF8(str[++offs]) || ++offs);
            ++charnum;
        }
        return charnum;
    }

    bool ValidChar(const char* ptr, const char* endPtr)
    {
        if (ptr >= endPtr)
            return false;

        auto nb = TrailingBytesForUTF8Ex[(unsigned char)*ptr];
        if (nb < 0)
            return false;

        if (ptr + nb >= endPtr)
            return false;

        ++ptr;

        bool valid = true;
        switch (nb)
        {
        case 5: valid &= IsUTF8Extension(*ptr++);
        case 4: valid &= IsUTF8Extension(*ptr++);
        case 3: valid &= IsUTF8Extension(*ptr++);
        case 2: valid &= IsUTF8Extension(*ptr++);
        case 1: valid &= IsUTF8Extension(*ptr++);
        case 0: valid &= true;
        }

        return valid;
    }

    uint32_t GetChar(const char* ptr, const char* endPtr)
    {
        return NextChar(ptr, endPtr);
    }

    uint32_t NextChar(const char*& ptr, const char* end)
    {
        if (ptr >= end)
            return 0;

		uint32_t ch = 0;
		uint32_t nb = TrailingBytesForUTF8[(unsigned char)*ptr];
        if (ptr + nb < end)
        {
            switch (nb)
            {
                /* these fall through deliberately */
            case 5: ch += (uint8_t)*ptr++; ch <<= 6;
            case 4: ch += (uint8_t)*ptr++; ch <<= 6;
            case 3: ch += (uint8_t)*ptr++; ch <<= 6;
            case 2: ch += (uint8_t)*ptr++; ch <<= 6;
            case 1: ch += (uint8_t)*ptr++; ch <<= 6;
            case 0: ch += (uint8_t)*ptr++;
            }

            ch -= OffsetsFromUTF8[nb];
        }

        return ch;
    }
    
    size_t Length(const char* ptr, const char* endPtr)
	{
		size_t count = 0;
		while (NextChar(ptr, endPtr))
			count++;
		return count;
	}

    size_t CalcSizeRequired(const uint32_t* s, size_t maxLength /*= MAX_SIZE_T*/)
    {
        size_t size = 0;
        char bytes[6];

        size_t i = 0;
        while (*s && i < maxLength)
        {
            size += ConvertChar(bytes, *s++);
            i += 1;
        }

        return size;
    }

    size_t CalcSizeRequired(const wchar_t* s, size_t maxLength /*= MAX_SIZE_T*/)
    {
        size_t size = 0;
        char bytes[6];

        size_t i = 0;
        while (*s && i < maxLength)
        {
            size += ConvertChar(bytes, *s++);
            i += 1;
        }

        return size;
    }

} // utf8

END_INFERNO_NAMESPACE();
