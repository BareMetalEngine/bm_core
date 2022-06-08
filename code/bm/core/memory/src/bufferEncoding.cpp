/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "buffer.h"
#include "bufferView.h"
#include "bufferSegmentView.h"

BEGIN_INFERNO_NAMESPACE()

//--

namespace prv
{
	// copied from CURL
	static inline bool Curl_isunreserved(unsigned char in)
	{
		switch (in) {
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
		case 'a': case 'b': case 'c': case 'd': case 'e':
		case 'f': case 'g': case 'h': case 'i': case 'j':
		case 'k': case 'l': case 'm': case 'n': case 'o':
		case 'p': case 'q': case 'r': case 's': case 't':
		case 'u': case 'v': case 'w': case 'x': case 'y': case 'z':
		case 'A': case 'B': case 'C': case 'D': case 'E':
		case 'F': case 'G': case 'H': case 'I': case 'J':
		case 'K': case 'L': case 'M': case 'N': case 'O':
		case 'P': case 'Q': case 'R': case 'S': case 'T':
		case 'U': case 'V': case 'W': case 'X': case 'Y': case 'Z':
		case '-': case '.': case '_': case '~':
			return true;
		default:
			break;
		}
		return false;
	}

	static uint64_t EstimateURLEncodingSize(BufferView data)
	{
		uint64_t size = 0;

		for (auto ch : data)
		{
			if (Curl_isunreserved(ch))
				size += 1;
			else
				size += 3;
		}

		return size;
	}

	static uint64_t EstimateCStringEncodingSize(BufferView data)
	{
		uint64_t size = 0;

		for (auto ch : data)
		{
			switch (ch)
			{
			case 0x07:
			case 0x08:
			case 0x1B:
			case 0x0C:
			case 0x0A:
			case 0x0D:
			case 0x09:
			case 0x0B:
			case 0x5C:
			case 0x27:
			case 0x22:
			case 0x3F:
				size += 2; // \n
				break;

			default:
				if (ch >= ' ' && ch <= 127)
					size += 1;
				else
					size += 4; // \xAA
			}
		}

		return size;
	}

	template< typename T >
	static T ReadChar(const T*& ptr, const T* end)
	{
		return (ptr < end) ? (*ptr++) : 0;
	}

	static uint64_t EsimateCStringDecodingSize(BufferView view)
	{
		uint64_t size = 0;

		const auto* readPtr = (const char*)view.data();
		const auto* endPtr = readPtr + view.size();

		while (readPtr < endPtr)
		{
			size += 1;

			const auto ch = ReadChar(readPtr, endPtr);
			if (*readPtr++ == '\\')
			{
				const auto ch2 = ReadChar(readPtr, endPtr);
				if (*readPtr == 'x')
					readPtr += 2;
			}
		}

		return size;
	}

	static uint64_t EsimateURLDecodingSize(BufferView view)
	{
		uint64_t size = 0;

		const auto* readPtr = (const char*)view.data();
		const auto* endPtr = readPtr + view.size();

		while (readPtr < endPtr)
		{
			size += 1;

			const auto ch = ReadChar(readPtr, endPtr);
			if (*readPtr++ == '%')
				readPtr += 2;
		}

		return size;
	}


	class DecodingTable
	{
	public:
		DecodingTable()
		{
			memset(decodingBase64, 0xFF, sizeof(decodingBase64));
			memset(decodingHex, 0xFF, sizeof(decodingHex));

			const char* ptr = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
			uint8_t value = 0;
			while (*ptr)
				decodingBase64[(int)*ptr++] = value++;

			decodingHex['0'] = 0;
			decodingHex['1'] = 1;
			decodingHex['2'] = 2;
			decodingHex['3'] = 3;
			decodingHex['4'] = 4;
			decodingHex['5'] = 5;
			decodingHex['6'] = 6;
			decodingHex['7'] = 7;
			decodingHex['8'] = 8;
			decodingHex['9'] = 9;
			decodingHex['A'] = 10;
			decodingHex['B'] = 11;
			decodingHex['C'] = 12;
			decodingHex['D'] = 13;
			decodingHex['E'] = 14;
			decodingHex['F'] = 15;
			decodingHex['a'] = 10;
			decodingHex['b'] = 11;
			decodingHex['c'] = 12;
			decodingHex['d'] = 13;
			decodingHex['e'] = 14;
			decodingHex['f'] = 15;
		}

		INLINE char valueBase64(char ch) const
		{
			return decodingBase64[(int)ch];
		}

		INLINE char valueHex(char ch) const
		{
			return decodingHex[(int)ch];
		}

		INLINE int valueHex(char a, char b) const
		{
			char va = decodingHex[(int)a];
			char vb = decodingHex[(int)b];
			if (va < 0 || vb < 0)
				return -1;

			return (va << 4) | vb;
		}

	private:
		char decodingBase64[256];
		char decodingHex[256];
	};

	static DecodingTable GDecodingTables;

	static bool EncodeBase64(BufferView input, BufferOutputStream<char>& output)
	{
		static const char* Base64Chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

		const auto* readPtr = input.data();
		const auto* readEndPtr = readPtr + input.size();

		// encode full blocks
		while (readPtr + 3 <= readEndPtr)
		{
			auto* writePtr = output.alloc(4);
			if (!writePtr)
				return false; // out of space

			writePtr[0] = Base64Chars[(readPtr[0] & 0xfc) >> 2];
			writePtr[1] = Base64Chars[((readPtr[0] & 0x03) << 4) + ((readPtr[1] & 0xf0) >> 4)];
			writePtr[2] = Base64Chars[((readPtr[1] & 0x0f) << 2) + ((readPtr[2] & 0xc0) >> 6)];
			writePtr[3] = Base64Chars[readPtr[2] & 0x3f];
			readPtr += 3;
		}

		// encode padded remainder
		if (readPtr < readEndPtr)
		{
			auto i = (readEndPtr - readPtr);
			ASSERT(i == 1 || i == 2);

			auto* writePtr = output.alloc(4);
			if (!writePtr)
				return false;

			uint8_t bytes[3];
			bytes[0] = readPtr[0];
			bytes[1] = (i == 2) ? readPtr[1] : 0;
			bytes[2] = 0;

			uint8_t chars[3];
			chars[0] = (bytes[0] & 0xfc) >> 2;
			chars[1] = ((bytes[0] & 0x03) << 4) + ((bytes[1] & 0xf0) >> 4);
			chars[2] = ((bytes[1] & 0x0f) << 2) + ((bytes[2] & 0xc0) >> 6);

			for (uint32_t j = 0; j < i + 1; j++)
				*writePtr++ = Base64Chars[chars[j]];

			while (i++ < 3)
				*writePtr++ = '=';
		}

		return true;
	}

	static bool EncodeHex(BufferView input, BufferOutputStream<char>& output)
	{
		static const char* HexChars = "0123456789ABCDEF";

		const auto* readPtr = input.data();
		const auto* readEndPtr = readPtr + input.size();

		while (readPtr < readEndPtr)
		{
			auto* writePtr = output.alloc(2);
			if (!writePtr)
				return false;

			*writePtr++ = HexChars[*readPtr >> 4];
			*writePtr++ = HexChars[*readPtr & 15];
			readPtr += 1;			
		}

		return true;
	}

	static bool EncodeURL(BufferView input, BufferOutputStream<char>& output)
	{
		static const char* HexChars = "0123456789ABCDEF";

		const auto* readPtr = input.data();
		const auto* readEndPtr = readPtr + input.size();

		while (readPtr < readEndPtr)
		{
			auto val = *readPtr++;

			if (Curl_isunreserved(val))
			{
				auto* writePtr = output.alloc(1);
				if (!writePtr)
					return false;

				*writePtr++ = val;
			}
			else
			{
				auto* writePtr = output.alloc(3);
				if (!writePtr)
					return false;

				*writePtr++ = '%';
				*writePtr++ = HexChars[val >> 4];
				*writePtr++ = HexChars[val & 15];
			}
		}

		return true;
	}

	static ALWAYS_INLINE bool WriteEscapedChar(BufferOutputStream<char>& output, char ch)
	{
		auto* writePtr = output.alloc(2);
		if (!writePtr)
			return false;

		*writePtr++ = '\\';
		*writePtr++ = ch;
		return true;
	}

	static ALWAYS_INLINE bool WriteNormalChar(BufferOutputStream<char>& output, char ch)
	{
		auto* writePtr = output.alloc(1);
		if (!writePtr)
			return false;

		*writePtr++ = ch;
		return true;
	}

	static ALWAYS_INLINE bool WriteHexChar(BufferOutputStream<char>& output, char ch)
	{
		static const char* HexChars = "0123456789ABCDEF";

		auto* writePtr = output.alloc(4);
		if (!writePtr)
			return false;

		*writePtr++ = '\\';
		*writePtr++ = 'x';
		*writePtr++ = HexChars[((uint8_t)ch) >> 4];
		*writePtr++ = HexChars[((uint8_t)ch) & 15];
		return true;
	}

	bool EncodeCString(BufferView input, BufferOutputStream<char>& output)
	{
		const auto* readPtr = input.data();
		const auto* readEndPtr = readPtr + input.size();

		while (readPtr < readEndPtr)
		{
			auto ch = *readPtr++;

			switch (ch)
			{
			case 0x00:
				if (!WriteEscapedChar(output, '0')) return false;
				break;
			case 0x07:
				if (!WriteEscapedChar(output, 'a')) return false;
				break;

			case 0x08:
				if (!WriteEscapedChar(output, 'b')) return false;
				break;

			/*case 0x1B:
				if (!WriteEscapedChar(output, 'e')) return false;
				break;*/

			case 0x0C:
				if (!WriteEscapedChar(output, 'f')) return false;
				break;

			case 0x0A:
				if (!WriteEscapedChar(output, 'n')) return false;
				break;

			case 0x0D:
				if (!WriteEscapedChar(output, 'r')) return false;
				break;

			case 0x09:
				if (!WriteEscapedChar(output, 't')) return false;
				break;

			case 0x0B:
				if (!WriteEscapedChar(output, 'v')) return false;
				break;

			case 0x5C:
				if (!WriteEscapedChar(output, '\\')) return false;
				break;

			case 0x27:
				if (!WriteEscapedChar(output, '\'')) return false;
				break;

			case 0x22:
				if (!WriteEscapedChar(output, '\"')) return false;
				break;

			case 0x3F:
				if (!WriteEscapedChar(output, '\?')) return false;
				break;

			default:
				if (ch >= ' ' && ch <= 127)
				{
					if (!WriteNormalChar(output, ch)) 
						return false;
				}
				else
				{
					if (!WriteHexChar(output, ch))
						return false;
				}
				break;
			}
		}

		return true;
	}

	//--

	static bool DecodeBase64(BufferView input, BufferOutputStream<uint8_t>& output, bool allowWhitespaces)
	{
		uint32_t i = 0;
		uint8_t chars[4];

		const auto* str = (const char*)input.data();
		const auto* strEnd = str + input.size();

		while (str < strEnd)
		{
			auto ch = *str++;

			if (ch == '=')
				break;

			if (ch <= ' ' && allowWhitespaces)
				continue;

			int value = GDecodingTables.valueBase64(ch);
			if (value < 0)
				return false;

			chars[i++] = (uint8_t)value;
			if (i == 4)
			{
				auto* writePtr = output.alloc(3);
				if (!writePtr)
					return false;

				*writePtr++ = (chars[0] << 2) + ((chars[1] & 0x30) >> 4);
				*writePtr++ = ((chars[1] & 0xf) << 4) + ((chars[2] & 0x3c) >> 2);
				*writePtr++ = ((chars[2] & 0x3) << 6) + chars[3];
				i = 0;
			}
		}

		if (i)
		{
			uint8_t data[2];
			data[0] = (chars[0] << 2) + ((chars[1] & 0x30) >> 4);
			data[1] = ((chars[1] & 0xf) << 4) + ((chars[2] & 0x3c) >> 2);

			auto count = i - 1;
			auto* writePtr = output.alloc(count);
			if (!writePtr)
				return false;

			for (uint32_t j = 0; (j < i - 1); j++)
				*writePtr++ = data[j];
		}

		return true;
	}

	static bool DecodeHex(BufferView input, BufferOutputStream<uint8_t>& output, bool allowWhitespaces)
	{
		const auto* str = (const char*)input.data();
		const auto* strEnd = str + input.size();
		
		uint32_t i = 0;
		uint8_t chars[2];

		while (str < strEnd)
		{
			auto ch = *str++;

			if (ch <= ' ' && allowWhitespaces) // allow and filter white spaces from HEX content
				continue;

			int value = GDecodingTables.valueHex(ch);
			if (value < 0)
				return false;

			chars[i++] = (uint8_t)value;
			if (i == 2)
			{
				const auto ch = (chars[0] << 4) + chars[1];
				if (!output.write(ch))
					return false;
				
				i = 0;
			}
		}

		// odd case
		if (i != 0)
			return false;

		// valid case
		return true;
	}

	static bool IsLegalURLChar(char ch)
	{
		if (ch >= 'A' && ch <= 'Z') return true;
		if (ch >= 'a' && ch <= 'z') return true;
		if (ch >= '0' && ch <= '9') return true;
		return false;
	}

	static bool DecodeURL(BufferView input, BufferOutputStream<uint8_t>& output, bool allowWhitespaces)
	{
		const char* str = (const char*)input.data();
		const char* strEnd = str + input.size();

		while (str < strEnd)
		{
			if (allowWhitespaces && *str <= ' ')
			{
				str++;
				continue;
			}

			auto ch = ReadChar(str, strEnd);
			if (ch == '+')
			{ 
				ch = ' ';
			}
			else if (ch == '%')
			{
				const auto ch2 = ReadChar(str, strEnd);
				const auto ch3 = ReadChar(str, strEnd);

				int value = GDecodingTables.valueHex(ch2, ch3);
				if (value < 0)
					return false;

				ch = (char)value;
			}
			else
			{
				if (!IsLegalURLChar(ch))
					return false;
			}

			if (!output.write(ch))
				return false;
		}

		return true;
	}

	static bool DecodeCString(BufferView input, BufferOutputStream<uint8_t>& output)
	{
		const char* str = (const char*)input.data();
		const char* strEnd = str + input.size();

		while (str < strEnd)
		{
			auto ch = ReadChar(str, strEnd);
			if (ch == '\\')
			{
				const auto ch2 = ReadChar(str, strEnd);
				switch (ch2)
				{
					case '0':
						ch = 0x00;
						break;
					case 'a':
						ch = 0x07;
						break;
					case 'b':
						ch = 0x08;
						break;
					/*case 'e':
						ch = 0x1B;
						break;*/
					case 'f':
						ch = 0x0C;
						break;
					case 'n':
						ch = 0x0A;
						break;
					case 'r':
						ch = 0x0D;
						break;
					case 't':
						ch = 0x09;
						break;
					case 'v':
						ch = 0x0B;
						break;
					case '\\':
						ch = 0x5C;
						break;
					case '\'':
						ch = 0x27;
						break;
					case '\"':
						ch = 0x22;
						break;
					case '?':
						ch = 0x3F;
						break;
					case 'x':
					{
						const auto ch3 = ReadChar(str, strEnd);
						const auto ch4 = ReadChar(str, strEnd);

						int value = GDecodingTables.valueHex(ch3, ch4);
						if (value < 0)
							return false;

						ch = (uint8_t)value;
						break;
					}
					default:
						return false;
				}
			}

			if (!output.write(ch))
				return false;
		}

		return true;
	}

	//--

} // prv

//--

uint64_t BufferView::estimateEncodedSize(EncodingType et) const
{
	switch (et)
	{
	case EncodingType::Base64:
		return (size() * 6) / 8;

	case EncodingType::Hex:
		return (size() * 2);

	case EncodingType::URL:
		return prv::EstimateURLEncodingSize(*this);

	case EncodingType::CString:
		return prv::EstimateCStringEncodingSize(*this);

	default:
		ASSERT(!"Unsupported encoding");
		return 0;
	}
}

uint64_t BufferView::estimateDecodedSize(EncodingType et) const
{
	switch (et)
	{
	case EncodingType::Base64:
		DEBUG_CHECK_RETURN_EX_V(size() % 4 == 0, "Base64 buffer should be multiple of 4", 0);
		return (size() / 4) * 3;

	case EncodingType::Hex:
		DEBUG_CHECK_RETURN_EX_V(size() % 2 == 0, "Base64 buffer should be multiple of 2", 0);
		return (size() / 2);

	case EncodingType::CString:
		return prv::EsimateURLDecodingSize(*this);

	case EncodingType::URL:
		return prv::EsimateURLDecodingSize(*this);

	default:
		ASSERT(!"Unsupported decoding");
		return 0;
	}
}

//--

void BufferView::encode(EncodingType et, IFormatStream& f) const
{
	static const auto PAGE_SIZE = 6 * 128; // HACK: make it divisible by 3 to make sure BASE64 encoding emits full blocks...

	for (auto part : segmentedView(PAGE_SIZE))
	{
		InplaceBufferOutputStream<char, PAGE_SIZE * 4> output;
		DEBUG_CHECK_RETURN_EX(part.encode(et, output), "Unexpected encoding problem - was internal buffer to small ?");

		f.append(output.start(), output.size());
	}
}

bool BufferView::encode(EncodingType et, BufferOutputStream<char>& output) const
{
	switch (et)
	{
	case EncodingType::Base64:
		return prv::EncodeBase64(*this, output);

	case EncodingType::Hex:
		return prv::EncodeHex(*this, output);

	case EncodingType::URL:
		return prv::EncodeURL(*this, output);

	case EncodingType::CString:
		return prv::EncodeCString(*this, output);

	default:
		DEBUG_CHECK(!"Unknown encoding mode");
		return false;
	}
}

bool BufferView::decode(EncodingType et, BufferOutputStream<uint8_t>& output, bool allowWhiteSpaces /*= true*/) const
{
	switch (et)
	{
	case EncodingType::Base64:
		return prv::DecodeBase64(*this, output, allowWhiteSpaces);

	case EncodingType::Hex:
		return prv::DecodeHex(*this, output, allowWhiteSpaces);

	case EncodingType::CString:
		return prv::DecodeCString(*this, output);

	case EncodingType::URL:
		return prv::DecodeURL(*this, output, allowWhiteSpaces);

	default:
		DEBUG_CHECK(!"Unknown encoding mode");
		return false;
	}
}

//--

Buffer Buffer::CreateFromEncoding(IPoolUnmanaged& pool, EncodingType et, const BufferView& view, bool allowWhitespces, uint32_t alignment /*= BUFFER_DEFAULT_ALIGNMNET*/)
{
	// determine output size
	const auto decodedMemorySize = view.estimateDecodedSize(et);
	VALIDATION_RETURN_V(decodedMemorySize, nullptr);

	// allocate target buffer
	auto ret = Buffer::CreateEmpty(pool, decodedMemorySize, alignment, BufferInitState::NoClear);
	DEBUG_CHECK_RETURN_EX_V(ret, "OOM", nullptr);

	// decode, may fail
	BufferOutputStream<uint8_t> writer(ret);
	VALIDATION_RETURN_EX_V(view.decode(et, writer, allowWhitespces), "Decoding failed", nullptr);

	// trim the buffer to real size
	// NOTE: extra bytes will be wasted until buffer if freed
	ret.adjustSize(writer.size());
	return ret;
}

Buffer Buffer::CreateEncoded(IPoolUnmanaged& pool, EncodingType et, const BufferView& view, uint32_t alignment /*= BUFFER_DEFAULT_ALIGNMNET*/)
{
	// determine EXACT output size
	const auto encodedMemorySize = view.estimateEncodedSize(et);
	VALIDATION_RETURN_V(encodedMemorySize, nullptr);

	// allocate target buffer
	auto ret = Buffer::CreateEmpty(pool, encodedMemorySize, alignment, BufferInitState::NoClear);
	DEBUG_CHECK_RETURN_EX_V(ret, "OOM", nullptr);

	// encode, does not fail since input is matched
	BufferOutputStream<char> writer(ret);
	VALIDATION_RETURN_EX_V(view.encode(et, writer), "Decoding failed", nullptr);

	// trim the buffer to real size
	// NOTE: extra bytes will be wasted until buffer if freed
	ret.adjustSize(writer.size());
	return ret;
}

//--

END_INFERNO_NAMESPACE()
