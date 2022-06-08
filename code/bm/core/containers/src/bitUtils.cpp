/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "bitUtils.h"

BEGIN_INFERNO_NAMESPACE()

//--

bool CompareBits(const BitWord* wordsA, const BitWord* wordsB, uint32_t index, uint32_t count)
{
	auto maxBits = index + count;

	auto wordIndex = index >> helper::BitTable::BIT_WORD_SHIFT;

	while (index < maxBits)
	{
		auto bitIndex = index & helper::BitTable::BIT_INDEX_MASK;
		auto wordMask = WordMask(count);

		auto wordA = (wordsA[wordIndex] >> bitIndex) & wordMask;
		auto wordB = (wordsB[wordIndex] >> bitIndex) & wordMask;
		if (wordA != wordB)
			break;

		index = (++wordIndex) << helper::BitTable::BIT_WORD_SHIFT;
		count -= (helper::BitTable::NUM_BITS_IN_WORD - bitIndex);
	}

	return index >= maxBits;
}

bool AllBitsSet(const BitWord* words, uint32_t index, uint32_t count)
{
	auto maxBits = index + count;
	auto wordIndex = index >> helper::BitTable::BIT_WORD_SHIFT;

	while (index < maxBits)
	{
		auto bitIndex = index & helper::BitTable::BIT_INDEX_MASK;
		auto wordMask = WordMask(count);

		auto word = (words[wordIndex] >> bitIndex) & wordMask;
		if (word != wordMask)
			break;

		index = (++wordIndex) << helper::BitTable::BIT_WORD_SHIFT;
		count -= (helper::BitTable::NUM_BITS_IN_WORD - bitIndex);
	}

	return index >= maxBits;
}

bool AnyBitsSet(const BitWord* words, uint32_t index, uint32_t count)
{
	auto maxBits = index + count;

	auto wordIndex = index >> helper::BitTable::BIT_WORD_SHIFT;

	while (index < maxBits)
	{
		auto bitIndex = index & helper::BitTable::BIT_INDEX_MASK;
		auto wordMask = WordMask(count);

		auto word = (words[wordIndex] >> bitIndex) & wordMask;
		if (word != 0)
			break;

		index = (++wordIndex) << helper::BitTable::BIT_WORD_SHIFT;
		count -= (helper::BitTable::NUM_BITS_IN_WORD - bitIndex);
	}

	return index < maxBits;
}

bool AllBitsSet(const BitWord* words, const BitWord* maskWords, uint32_t index, uint32_t count)
{
	auto maxBits = index + count;

	auto wordIndex = index >> helper::BitTable::BIT_WORD_SHIFT;

	while (index < maxBits)
	{
		auto bitIndex = index & helper::BitTable::BIT_INDEX_MASK;
		auto bitMask = WordMask(count) << bitIndex;

		auto word = words[wordIndex] & maskWords[wordIndex];
		if ((word & bitMask) != bitMask)
			break;

		index = (++wordIndex) << helper::BitTable::BIT_WORD_SHIFT;
		count -= (helper::BitTable::NUM_BITS_IN_WORD - bitIndex);
	}

	return index >= maxBits;
}

bool AnyBitsSet(const BitWord* words, const BitWord* maskWords, uint32_t index, uint32_t count)
{
	auto maxBits = index + count;

	auto wordIndex = index >> helper::BitTable::BIT_WORD_SHIFT;

	while (index < maxBits)
	{
		auto bitIndex = index & helper::BitTable::BIT_INDEX_MASK;
		auto bitMask = WordMask(count) << bitIndex;

		auto word = words[wordIndex] & maskWords[wordIndex];
		if ((word & bitMask) != 0)
			break;

		index = (++wordIndex) << helper::BitTable::BIT_WORD_SHIFT;
		count -= (helper::BitTable::NUM_BITS_IN_WORD - bitIndex);
	}

	return index < maxBits;
}

void SetBits(BitWord* words, uint32_t index, uint32_t count)
{
	auto maxBits = index + count;

	auto wordIndex = index >> helper::BitTable::BIT_WORD_SHIFT;
	while (index < maxBits)
	{
		auto bitIndex = index & helper::BitTable::BIT_INDEX_MASK;
		auto bitMask = WordMask(count) << bitIndex;

		words[wordIndex] |= bitMask;

		index = (++wordIndex) << helper::BitTable::BIT_WORD_SHIFT;
		count -= (helper::BitTable::NUM_BITS_IN_WORD - bitIndex);
	}
}

void OrBits(BitWord* words, const BitWord* maskWords, uint32_t index, uint32_t count)
{
	auto maxBits = index + count;

	auto wordIndex = index >> helper::BitTable::BIT_WORD_SHIFT;
	while (index < maxBits)
	{
		auto bitIndex = index & helper::BitTable::BIT_INDEX_MASK;
		auto bitMask = WordMask(count) << bitIndex;

		words[wordIndex] |= (maskWords[wordIndex] & bitMask);

		index = (++wordIndex) << helper::BitTable::BIT_WORD_SHIFT;
		count -= (helper::BitTable::NUM_BITS_IN_WORD - bitIndex);
	}
}

void AndBits(BitWord* words, const BitWord* maskWords, uint32_t index, uint32_t count)
{
	auto maxBits = index + count;

	auto wordIndex = index >> helper::BitTable::BIT_WORD_SHIFT;
	while (index < maxBits)
	{
		auto bitIndex = index & helper::BitTable::BIT_INDEX_MASK;
		auto bitMask = WordMask(count) << bitIndex;

		words[wordIndex] &= maskWords[wordIndex] | ~bitMask;

		index = (++wordIndex) << helper::BitTable::BIT_WORD_SHIFT;
		count -= (helper::BitTable::NUM_BITS_IN_WORD - bitIndex);
	}
}

void XorBits(BitWord* words, const BitWord* maskWords, uint32_t index, uint32_t count)
{
	auto maxBits = index + count;

	auto wordIndex = index >> helper::BitTable::BIT_WORD_SHIFT;
	while (index < maxBits)
	{
		auto bitIndex = index & helper::BitTable::BIT_INDEX_MASK;
		auto bitMask = WordMask(count) << bitIndex;

		words[wordIndex] ^= (maskWords[wordIndex] & bitMask);

		index = (++wordIndex) << helper::BitTable::BIT_WORD_SHIFT;
		count -= (helper::BitTable::NUM_BITS_IN_WORD - bitIndex);
	}
}

void ClearBits(BitWord* words, const BitWord* maskWords, uint32_t index, uint32_t count)
{
	auto maxBits = index + count;

	auto wordIndex = index >> helper::BitTable::BIT_WORD_SHIFT;
	while (index < maxBits)
	{
		auto bitIndex = index & helper::BitTable::BIT_INDEX_MASK;
		auto bitMask = WordMask(count) << bitIndex;

		words[wordIndex] &= ~maskWords[wordIndex] | ~bitMask;

		index = (++wordIndex) << helper::BitTable::BIT_WORD_SHIFT;
		count -= (helper::BitTable::NUM_BITS_IN_WORD - bitIndex);
	}
}

void ClearBits(BitWord* words, uint32_t index, uint32_t count)
{
	auto maxBits = index + count;

	auto wordIndex = index >> helper::BitTable::BIT_WORD_SHIFT;
	while (index < maxBits)
	{
		auto bitIndex = index & helper::BitTable::BIT_INDEX_MASK;
		auto bitMask = WordMask(count) << bitIndex;

		words[wordIndex] &= ~bitMask;

		index = (++wordIndex) << helper::BitTable::BIT_WORD_SHIFT;
		count -= (helper::BitTable::NUM_BITS_IN_WORD - bitIndex);
	}
}

//--

END_INFERNO_NAMESPACE()


