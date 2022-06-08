/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

BEGIN_INFERNO_NAMESPACE()

//--

ALWAYS_INLINE BitSet::BitSet()
{}

ALWAYS_INLINE BitSet::BitSet(BitSet&& other)
	: m_words(other.m_words)
	, m_bitSize(other.m_bitSize)
{
	other.m_words = nullptr;
	other.m_bitSize = 0;
}

ALWAYS_INLINE BitSet::BitSet(uint32_t bitCount, EBitStateZero)
{
	resizeWithZeros(bitCount);
}

ALWAYS_INLINE BitSet::BitSet(uint32_t bitCount, EBitStateOne)
{
	resizeWithOnes(bitCount);
}

ALWAYS_INLINE BitSet& BitSet::operator=(BitSet&& other)
{
	if (this != &other)
	{
		m_words = other.m_words;
		m_bitSize = other.m_bitSize;
		other.m_words = nullptr;
		other.m_bitSize = 0;
	}
	return *this;
}

ALWAYS_INLINE uint32_t BitSet::bitCount() const
{
	return m_bitSize;
}

ALWAYS_INLINE uint32_t BitSet::wordCount() const
{
	return WorldCountForBitCount(m_bitSize);
}

ALWAYS_INLINE bool BitSet::empty() const
{
	return (0 == m_bitSize);
}

ALWAYS_INLINE BitWord* BitSet::words()
{
	return m_words;
}

ALWAYS_INLINE const BitWord* BitSet::words() const
{
	return m_words;
}

ALWAYS_INLINE void BitSet::clearAll()
{
	memset(m_words, 0x00, wordCount() * sizeof(BitWord));
}

ALWAYS_INLINE void BitSet::enableAll()
{
    memset(m_words, 0xFF, wordCount() * sizeof(BitWord));
}

ALWAYS_INLINE bool BitSet::operator[](uint32_t bit) const
{
	return TestBit(m_words, bit);
}

ALWAYS_INLINE bool BitSet::set(uint32_t bit)
{
	return SetBit(m_words, bit);
}

ALWAYS_INLINE bool BitSet::clear(uint32_t bit)
{
	return ClearBit(m_words, bit);
}

ALWAYS_INLINE bool BitSet::toggle(uint32_t bit, bool state)
{
	return state ? SetBit(m_words, bit) : ClearBit(m_words, bit);
}

//--

INLINE bool BitSet::iterateSetBits(const std::function<bool(uint32_t index)>& enumFunc) const
{
	auto* ptr = m_words;
	auto* ptrEnd = m_words + wordCount();
	uint32_t index = 0;
    while (ptr < ptrEnd)
    {
        auto mask = *ptr;
        while (mask)
        {
            uint32_t localIndex = __builtin_ctzll(mask) + index;
            if (enumFunc(localIndex))
                return true;

            mask ^= mask & -mask;
        }

        index += NUM_BITS_IN_WORD;
        ptr += 1;
    }

	return false;
}

INLINE bool BitSet::iterateClearBits(const std::function<bool(uint32_t index)>& enumFunc) const
{
    auto* ptr = m_words;
	auto* ptrEnd = m_words + wordCount();
    uint32_t index = 0;
    while (ptr < ptrEnd)
    {
        auto mask = ~*ptr;
        while (mask)
        {
            uint32_t localIndex = __builtin_ctzll(mask) + index;
            if (enumFunc(localIndex))
                return true;

            mask ^= mask & -mask;
        }

        index += NUM_BITS_IN_WORD;
        ptr += 1;
    }

    return false;
}

//--

END_INFERNO_NAMESPACE()

