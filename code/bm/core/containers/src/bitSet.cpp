/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "bitSet.h"

BEGIN_INFERNO_NAMESPACE()

BitSet::BitSet(const BitSet& other)
    : m_bitSize(other.m_bitSize)
{
    if (other.m_bitSize)
    {
        resizeWithZeros(other.m_bitSize);
        memcpy(m_words, other.m_words, wordCount() * sizeof(BitWord));
    }
}

BitSet& BitSet::operator=(const BitSet& other)
{
    if (this != &other)
    {
        resizeWithZeros(other.m_bitSize);
        memcpy(m_words, other.m_words, wordCount() * sizeof(BitWord));
    }

    return *this;
}

BitSet::~BitSet()
{
    clear();
}

void BitSet::clear()
{
    if (m_words)
    {
        Memory::FreeBlock(m_words);
        m_words = nullptr;
        m_bitSize = 0;
    }
}

void BitSet::resizeWithZeros(uint32_t newBitCount)
{
    const auto wordCount = WorldCountForBitCount(newBitCount);
    const auto dataSize = sizeof(BitWord) * wordCount;

    m_bitSize = newBitCount;
    m_words = (BitWord*) Memory::ResizeBlock(m_words, dataSize, 16, "BitSet");

    memzero(m_words, dataSize);
}

void BitSet::resizeWithOnes(uint32_t newBitCount)
{
    const auto wordCount = WorldCountForBitCount(newBitCount);
    const auto dataSize = sizeof(BitWord) * wordCount;

    m_bitSize = newBitCount;
    m_words = (BitWord*)Memory::ResizeBlock(m_words, dataSize, 16, "BitSet");

    memset(m_words, 0xFF, dataSize);
}

//--

INLINE BitSet& BitSet::operator|=(const BitSet& other)
{
    if (this != &other)
    {
        auto count = std::min(wordCount(), other.wordCount());

        auto* ptr = m_words;
        auto* ptrEnd = m_words + count;
        auto* maskPtr = other.m_words;

        while (ptr < ptrEnd)
            *ptr++ |= *maskPtr++;
    }

    return *this;
}

INLINE BitSet& BitSet::operator&=(const BitSet& other)
{
    if (this != &other)
    {
        auto count = std::min(wordCount(), other.wordCount());

        auto* ptr = m_words;
        auto* ptrEnd = m_words + count;
        auto* maskPtr = other.m_words;

        while (ptr < ptrEnd)
            *ptr++ &= *maskPtr++;
    }

    return *this;
}

BitSet& BitSet::operator^=(const BitSet& other)
{
    if (this != &other)
    {
        auto count = std::min(wordCount(), other.wordCount());

        auto* ptr = m_words;
        auto* ptrEnd = m_words + count;
        auto* maskPtr = other.m_words;

        while (ptr < ptrEnd)
            *ptr++ ^= *maskPtr++;
    }

    return *this;
}

INLINE BitSet& BitSet::operator-=(const BitSet& other)
{
    if (this != &other)
    {
        auto count = std::min(wordCount(), other.wordCount());

        auto* ptr = m_words;
        auto* ptrEnd = m_words + count;
        auto* maskPtr = other.m_words;

        while (ptr < ptrEnd)
            *ptr++ &= ~*maskPtr++;
    }

    return *this;
}

bool BitSet::isAllSet() const
{
    auto* ptr = m_words;
    auto* ptrEnd = m_words + wordCount();

    auto bitsLeft = m_bitSize;

    while (ptr < ptrEnd)
    {
        const auto mask = WordMask(bitsLeft);
        if ((*ptr++ & mask) != mask)
            return false;

        bitsLeft -= NUM_BITS_IN_WORD;
    }

    return true;
}

bool BitSet::isAnySet() const
{
    auto* ptr = m_words;
    auto* ptrEnd = m_words + wordCount();

    auto bitsLeft = m_bitSize;

    while (ptr < ptrEnd)
    {
        const auto mask = WordMask(bitsLeft);
        if ((*ptr++ & mask) != 0)
            return false;

        bitsLeft -= NUM_BITS_IN_WORD;
    }

    return true;
}

bool BitSet::isNoneSet() const
{
    return !isAnySet();
}

bool BitSet::operator==(const BitSet& other) const
{
    if (m_bitSize != other.m_bitSize)
        return false;
    
    const auto* ptr = m_words;
    const auto* ptr2 = other.m_words;
    const auto* ptrEnd = m_words + wordCount();

    auto bitsLeft = m_bitSize;

    while (ptr < ptrEnd)
    {
        const auto mask = WordMask(bitsLeft);
        if ((*ptr++ & mask) != (*ptr2++ & mask))
            return false;

        bitsLeft -= NUM_BITS_IN_WORD;
    }

    return true;
}

bool BitSet::operator!=(const BitSet& other) const
{
    return !operator==(other);
}

//--

END_INFERNO_NAMESPACE()