/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once


#ifdef PLATFORM_MSVC
ALWAYS_INLINE int  __builtin_ctzll(uint64_t val)
{
    unsigned long ret = 0;
    _BitScanForward64(&ret, val);
    return ret;
}

ALWAYS_INLINE int  __builtin_clzll(uint64_t val)
{
    unsigned long ret = 0;
    _BitScanReverse64(&ret, val);
    return 63 - ret;
}

ALWAYS_INLINE int  __builtin_clzll_pos(uint64_t val)
{	
    unsigned long ret = 0;
    _BitScanReverse64(&ret, val);
    return ret;
}

ALWAYS_INLINE int  __builtin_ctz(uint32_t val)
{
    unsigned long ret = 0;
    _BitScanForward(&ret, val);
    return ret;
}

ALWAYS_INLINE int  __builtin_clz(uint32_t val)
{
    unsigned long ret = 0;
    _BitScanReverse(&ret, val);
    return 31 - ret;
}

ALWAYS_INLINE int  __builtin_clz_pos(uint32_t val)
{
    unsigned long ret = 0;
    _BitScanReverse(&ret, val);
    return ret;
}

#endif

BEGIN_INFERNO_NAMESPACE()

//--

namespace helper
{
#if defined(PLATFORM_32BIT)
	static ALWAYS_INLINE uint32_t BitScanForward(uint32_t bits)
	{
#ifdef PLATFORM_MSVC
		if (bits != 0)
		{
			uint32_t theIndex = 0;
			::_BitScanForward((unsigned long*)& theIndex, bits);
			return theIndex;
		}
		else
		{
			return 0;
		}
#else
		return mask == 0 ? 0 : __builtin_ctz(mask);
#endif
	}
#elif defined(PLATFORM_64BIT)
	static ALWAYS_INLINE uint32_t BitScanForward(uint64_t bits)
	{
#ifdef PLATFORM_MSVC
		if (bits != 0)
		{
			uint32_t theIndex = 0;
			::_BitScanForward64((unsigned long*)& theIndex, bits);
			return theIndex;
		}
		else
		{
			return 0;
		}
#else
		return bits == 0 ? 0 : (uint32_t)__builtin_ctzll(bits);
#endif
	}
#endif

	struct BitTable
	{
		static const uint32_t NUM_BITS_IN_WORD = sizeof(BitWord) * 8;
		static const uint32_t BIT_WORD_SHIFT = 6;
		static const uint32_t BIT_INDEX_MASK = (1U << BIT_WORD_SHIFT) - 1;
		static const BitWord ALL_ONES = ~(BitWord)0;
		static const BitWord ALL_ZEROS = (BitWord)0;
	};
}

//--

ALWAYS_INLINE uint32_t WorldCountForBitCount(uint32_t bitCount)
{
	return (bitCount + helper::BitTable::NUM_BITS_IN_WORD - 1) >> helper::BitTable::BIT_WORD_SHIFT;
}

ALWAYS_INLINE BitWord WordMask(uint32_t count)
{
	return (count < helper::BitTable::NUM_BITS_IN_WORD) ? ((((BitWord)1) << count) - 1) : helper::BitTable::ALL_ONES;
}

ALWAYS_INLINE bool TestBit(const BitWord* bits, uint32_t index)
{
	auto wordIndex = index >> helper::BitTable::BIT_WORD_SHIFT;
	auto bitIndex = index & helper::BitTable::BIT_INDEX_MASK;
	auto one = (BitWord)1;
	return 0 != (bits[wordIndex] & (one << bitIndex));
}

ALWAYS_INLINE bool ClearBit(BitWord* bits, uint32_t index)
{
	auto one = (BitWord)1;

	auto wordIndex = index >> helper::BitTable::BIT_WORD_SHIFT;
	auto bitMask = one << (index & helper::BitTable::BIT_INDEX_MASK);

	auto& word = bits[wordIndex];
	auto oldValue = 0 != (word & bitMask);
	word &= ~bitMask;

	return oldValue;
}

ALWAYS_INLINE bool SetBit(BitWord* bits, uint32_t index)
{
	auto one = (BitWord)1;

	auto wordIndex = index >> helper::BitTable::BIT_WORD_SHIFT;
	auto bitMask = ((BitWord)1) << (index & helper::BitTable::BIT_INDEX_MASK);

	auto& word = ((BitWord*)bits)[wordIndex];
	auto oldValue = 0 != (word & bitMask);
	word |= bitMask;

	return oldValue;
}

//--

ALWAYS_INLINE uint32_t FindFirstBitSet(const BitWord* words, uint32_t maxBits)
{
	auto maxWord = (maxBits + (helper::BitTable::NUM_BITS_IN_WORD - 1)) / helper::BitTable::NUM_BITS_IN_WORD;

	uint32_t wordBitIndex = 0;
	for (uint32_t i = 0; i < maxWord; ++i, wordBitIndex += helper::BitTable::NUM_BITS_IN_WORD)
	{
		if (words[i] != 0)
		{
			auto bitSetIndex = helper::BitScanForward(words[i]);
			return wordBitIndex + bitSetIndex;
		}
	}

	return maxBits;
}

ALWAYS_INLINE uint32_t FindFirstBitCleared(const BitWord* words, uint32_t maxBits)
{
	auto maxWord = (maxBits + (helper::BitTable::NUM_BITS_IN_WORD - 1)) / helper::BitTable::NUM_BITS_IN_WORD;

	uint32_t wordBitIndex = 0;
	for (uint32_t i = 0; i < maxWord; ++i, wordBitIndex += helper::BitTable::NUM_BITS_IN_WORD)
	{
		if (~words[i] != 0)
		{
			auto bitSetIndex = helper::BitScanForward(~words[i]);
			return wordBitIndex + bitSetIndex;
		}
	}

	return maxBits;
}

ALWAYS_INLINE uint32_t FindNextBitSet(const BitWord* words, uint32_t maxBits, uint32_t index)
{
	auto wordIndex = index >> helper::BitTable::BIT_WORD_SHIFT;
	while (index < maxBits)
	{
		auto bitIndex = index & helper::BitTable::BIT_INDEX_MASK;

		auto bits = words[wordIndex] >> bitIndex;
		if (bits != 0)
		{
			index += helper::BitScanForward(bits);
			break;
		}

		index = (++wordIndex) << helper::BitTable::BIT_WORD_SHIFT;
	}

	return index;
}

ALWAYS_INLINE uint32_t FindNextBitCleared(const BitWord* words, uint32_t maxBits, uint32_t index)
{
	auto wordIndex = index >> helper::BitTable::BIT_WORD_SHIFT;

	while (index < maxBits)
	{
		auto bitIndex = index & helper::BitTable::BIT_INDEX_MASK;

		auto bits = (~words[wordIndex]) >> bitIndex; // inverse
		if (bits != 0)
		{
			index += helper::BitScanForward(bits);
			break;
		}

		index = (++wordIndex) << helper::BitTable::BIT_WORD_SHIFT;
	}

	return index;
}

//--

INLINE static uint8_t CalcAdaptiveNumberSize4(uint32_t number)
{
	uint8_t usedBits = 32 - __builtin_clz(number | 1);
	uint32_t check = 1U << usedBits;
	ASSERT(number < check);
	return (usedBits + 6) / 7;
}

INLINE static uint8_t CalcAdaptiveNumberSize8(uint64_t number)
{
	uint8_t usedBits = 64 - __builtin_clzll(number | 1);
	return (usedBits + 6) / 7;
}

INLINE static void WriteAdaptiveNumber4(void* ptr, uint8_t size, uint32_t value)
{
	auto* bytes = (uint8_t*)ptr;

	switch (size)
	{
	case 1:
		bytes[0] = value & 0x7F;
		break;

	case 2:
		bytes[0] = ((value >> 0) & 0x7F) | 0x80;
		bytes[1] = ((value >> 7) & 0x7F) | 0x00;
		break;

	case 3:
		bytes[0] = ((value >> 0) & 0x7F) | 0x80;
		bytes[1] = ((value >> 7) & 0x7F) | 0x80;
		bytes[2] = ((value >> 14) & 0x7F) | 0x00;
		break;

	case 4:
		bytes[0] = ((value >> 0) & 0x7F) | 0x80;
		bytes[1] = ((value >> 7) & 0x7F) | 0x80;
		bytes[2] = ((value >> 14) & 0x7F) | 0x80;
		bytes[3] = ((value >> 21) & 0x7F) | 0x00;
		break;

	case 5:
		bytes[0] = ((value >> 0) & 0x7F) | 0x80;
		bytes[1] = ((value >> 7) & 0x7F) | 0x80;
		bytes[2] = ((value >> 14) & 0x7F) | 0x80;
		bytes[3] = ((value >> 21) & 0x7F) | 0x80;
		bytes[4] = ((value >> 28) & 0x7F) | 0x00;
		break;
	}

	uint8_t valueSize;
	const auto readValue = ReadAdaptiveNumber4(bytes, valueSize);
	ASSERT(value == readValue);
}

INLINE static void WriteAdaptiveNumber8(void* ptr, uint8_t size, uint64_t value)
{
	auto* bytes = (uint8_t*)ptr;

	switch (size)
	{
	case 1:
		bytes[0] = value & 0x7F;
		break;

	case 2:
		bytes[0] = ((value >> 0) & 0x7F) | 0x80;
		bytes[1] = ((value >> 7) & 0x7F) | 0x00;
		break;

	case 3:
		bytes[0] = ((value >> 0) & 0x7F) | 0x80;
		bytes[1] = ((value >> 7) & 0x7F) | 0x80;
		bytes[2] = ((value >> 14) & 0x7F) | 0x00;
		break;

	case 4:
		bytes[0] = ((value >> 0) & 0x7F) | 0x80;
		bytes[1] = ((value >> 7) & 0x7F) | 0x80;
		bytes[2] = ((value >> 14) & 0x7F) | 0x80;
		bytes[3] = ((value >> 21) & 0x7F) | 0x00;
		break;

	case 5:
		bytes[0] = ((value >> 0) & 0x7F) | 0x80;
		bytes[1] = ((value >> 7) & 0x7F) | 0x80;
		bytes[2] = ((value >> 14) & 0x7F) | 0x80;
		bytes[3] = ((value >> 21) & 0x7F) | 0x80;
		bytes[4] = ((value >> 28) & 0x7F) | 0x00;
		break;

	case 6:
		bytes[0] = ((value >> 0) & 0x7F) | 0x80;
		bytes[1] = ((value >> 7) & 0x7F) | 0x80;
		bytes[2] = ((value >> 14) & 0x7F) | 0x80;
		bytes[3] = ((value >> 21) & 0x7F) | 0x80;
		bytes[4] = ((value >> 28) & 0x7F) | 0x80;
		bytes[5] = ((value >> 35) & 0x7F) | 0x00;
		break;

	case 7:
		bytes[0] = ((value >> 0) & 0x7F) | 0x80;
		bytes[1] = ((value >> 7) & 0x7F) | 0x80;
		bytes[2] = ((value >> 14) & 0x7F) | 0x80;
		bytes[3] = ((value >> 21) & 0x7F) | 0x80;
		bytes[4] = ((value >> 28) & 0x7F) | 0x80;
		bytes[5] = ((value >> 35) & 0x7F) | 0x80;
		bytes[6] = ((value >> 42) & 0x7F) | 0x00;
		break;

	case 8:
		bytes[0] = ((value >> 0) & 0x7F) | 0x80;
		bytes[1] = ((value >> 7) & 0x7F) | 0x80;
		bytes[2] = ((value >> 14) & 0x7F) | 0x80;
		bytes[3] = ((value >> 21) & 0x7F) | 0x80;
		bytes[4] = ((value >> 28) & 0x7F) | 0x80;
		bytes[5] = ((value >> 35) & 0x7F) | 0x80;
		bytes[6] = ((value >> 42) & 0x7F) | 0x80;
		bytes[7] = ((value >> 49) & 0x7F) | 0x00;
		break;

	case 9:
		bytes[0] = ((value >> 0) & 0x7F) | 0x80;
		bytes[1] = ((value >> 7) & 0x7F) | 0x80;
		bytes[2] = ((value >> 14) & 0x7F) | 0x80;
		bytes[3] = ((value >> 21) & 0x7F) | 0x80;
		bytes[4] = ((value >> 28) & 0x7F) | 0x80;
		bytes[5] = ((value >> 35) & 0x7F) | 0x80;
		bytes[6] = ((value >> 42) & 0x7F) | 0x80;
		bytes[7] = ((value >> 49) & 0x7F) | 0x80;
		bytes[8] = ((value >> 56) & 0x7F) | 0x00;
		break;

	case 10:
		bytes[0] = ((value >> 0) & 0x7F) | 0x80;
		bytes[1] = ((value >> 7) & 0x7F) | 0x80;
		bytes[2] = ((value >> 14) & 0x7F) | 0x80;
		bytes[3] = ((value >> 21) & 0x7F) | 0x80;
		bytes[4] = ((value >> 28) & 0x7F) | 0x80;
		bytes[5] = ((value >> 35) & 0x7F) | 0x80;
		bytes[6] = ((value >> 42) & 0x7F) | 0x80;
		bytes[7] = ((value >> 49) & 0x7F) | 0x80;
		bytes[8] = ((value >> 56) & 0x7F) | 0x80;
		bytes[9] = ((value >> 63) & 0x7F) | 0x00;
		break;
	}

	uint8_t valueSize;
	const auto readValue = ReadAdaptiveNumber8(bytes, valueSize);
	ASSERT(value == readValue);
}

uint32_t ReadAdaptiveNumber4(const uint8_t* ptr, uint8_t& outValueSize)
{
	const auto* start = ptr;
	auto singleByte = *ptr++;
	uint32_t ret = singleByte & 0x7F;
	uint32_t offset = 7;

	while (singleByte & 0x80)
	{
		singleByte = *ptr++;
		ret |= (singleByte & 0x7F) << offset;
		offset += 7;
	}

	outValueSize = ptr - start;
	return ret;
}

uint64_t ReadAdaptiveNumber8(const uint8_t* ptr, uint8_t& outValueSize)
{
	const auto* start = ptr;
	auto singleByte = *ptr++;
	uint64_t ret = singleByte & 0x7F;
	uint32_t offset = 7;

	while (singleByte & 0x80)
	{
		singleByte = *ptr++;
		ret |= (singleByte & 0x7F) << offset;
		offset += 7;
	}

	outValueSize = ptr - start;
	return ret;
}

//--

END_INFERNO_NAMESPACE()


