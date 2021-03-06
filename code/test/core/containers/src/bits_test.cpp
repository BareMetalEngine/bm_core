/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "bm/core/containers/include/bitUtils.h"
#include "bm/core/containers/include/array.h"

BEGIN_INFERNO_NAMESPACE()

TEST(Bits, ClearBitsFirstWord)
{
	uint8_t test[8];
	memset(&test, 0xFF, sizeof(test));

	EXPECT_EQ(0xFF, test[0]);
	ClearBits((BitWord*)&test, 0, 8);
	EXPECT_EQ(0x00, test[0]);
	EXPECT_EQ(0xFF, test[1]);
	EXPECT_EQ(0xFF, test[2]);
	EXPECT_EQ(0xFF, test[3]);
	EXPECT_EQ(0xFF, test[4]);
	EXPECT_EQ(0xFF, test[5]);
	EXPECT_EQ(0xFF, test[6]);
	EXPECT_EQ(0xFF, test[7]);
}

TEST(Bits, ClearBitsMiddleFirstWord)
{
	uint8_t test[8];
	memset(&test, 0xFF, sizeof(test));

	EXPECT_EQ(0xFF, test[0]);
	ClearBits((BitWord*)&test, 8, 8);
	EXPECT_EQ(0xFF, test[0]);
	EXPECT_EQ(0x00, test[1]);
	EXPECT_EQ(0xFF, test[2]);
	EXPECT_EQ(0xFF, test[3]);
	EXPECT_EQ(0xFF, test[4]);
	EXPECT_EQ(0xFF, test[5]);
	EXPECT_EQ(0xFF, test[6]);
	EXPECT_EQ(0xFF, test[7]);
}

TEST(Bits, ClearBitsSecondWord)
{
	uint8_t test[8];
	memset(&test, 0xFF, sizeof(test));

	EXPECT_EQ(0xFF, test[0]);
	ClearBits((BitWord*)&test, 32, 8);
	EXPECT_EQ(0xFF, test[0]);
	EXPECT_EQ(0xFF, test[1]);
	EXPECT_EQ(0xFF, test[2]);
	EXPECT_EQ(0xFF, test[3]);
	EXPECT_EQ(0x00, test[4]);
	EXPECT_EQ(0xFF, test[5]);
	EXPECT_EQ(0xFF, test[6]);
	EXPECT_EQ(0xFF, test[7]);
}

TEST(Bits, ClearBitsSpanWords)
{
	uint8_t test[8];
	memset(&test, 0xFF, sizeof(test));

	EXPECT_EQ(0xFF, test[0]);
	ClearBits((BitWord*)&test, 24, 16);
	EXPECT_EQ(0xFF, test[0]);
	EXPECT_EQ(0xFF, test[1]);
	EXPECT_EQ(0xFF, test[2]);
	EXPECT_EQ(0x00, test[3]);
	EXPECT_EQ(0x00, test[4]);
	EXPECT_EQ(0xFF, test[5]);
	EXPECT_EQ(0xFF, test[6]);
	EXPECT_EQ(0xFF, test[7]);
}


TEST(Bits, SetBitsFirstWord)
{
	uint8_t test[8];
	memset(&test, 0x00, sizeof(test));

	EXPECT_EQ(0x00, test[0]);
	SetBits((BitWord*)&test, 0, 8);
	EXPECT_EQ(0xFF, test[0]);
	EXPECT_EQ(0x00, test[1]);
	EXPECT_EQ(0x00, test[2]);
	EXPECT_EQ(0x00, test[3]);
	EXPECT_EQ(0x00, test[4]);
	EXPECT_EQ(0x00, test[5]);
	EXPECT_EQ(0x00, test[6]);
	EXPECT_EQ(0x00, test[7]);
}

TEST(Bits, SetBitsMiddleFirstWord)
{
	uint8_t test[8];
	memset(&test, 0x00, sizeof(test));

	EXPECT_EQ(0x00, test[0]);
	SetBits((BitWord*)&test, 8, 8);
	EXPECT_EQ(0x00, test[0]);
	EXPECT_EQ(0xFF, test[1]);
	EXPECT_EQ(0x00, test[2]);
	EXPECT_EQ(0x00, test[3]);
	EXPECT_EQ(0x00, test[4]);
	EXPECT_EQ(0x00, test[5]);
	EXPECT_EQ(0x00, test[6]);
	EXPECT_EQ(0x00, test[7]);
}

TEST(Bits, SetBitsSecondWord)
{
	uint8_t test[8];
	memset(&test, 0x00, sizeof(test));

	EXPECT_EQ(0x00, test[0]);
	SetBits((BitWord*)&test, 32, 8);
	EXPECT_EQ(0x00, test[0]);
	EXPECT_EQ(0x00, test[1]);
	EXPECT_EQ(0x00, test[2]);
	EXPECT_EQ(0x00, test[3]);
	EXPECT_EQ(0xFF, test[4]);
	EXPECT_EQ(0x00, test[5]);
	EXPECT_EQ(0x00, test[6]);
	EXPECT_EQ(0x00, test[7]);
}

TEST(Bits, SetBitsSpanWords)
{
	uint8_t test[8];
	memset(&test, 0x00, sizeof(test));

	EXPECT_EQ(0x00, test[0]);
	SetBits((BitWord*)&test, 24, 16);
	EXPECT_EQ(0x00, test[0]);
	EXPECT_EQ(0x00, test[1]);
	EXPECT_EQ(0x00, test[2]);
	EXPECT_EQ(0xFF, test[3]);
	EXPECT_EQ(0xFF, test[4]);
	EXPECT_EQ(0x00, test[5]);
	EXPECT_EQ(0x00, test[6]);
	EXPECT_EQ(0x00, test[7]);
}

TEST(Bits, FindNextBitSet)
{
    BitWord bits = 0x001FF100ULL;
    EXPECT_EQ(8U, FindNextBitSet(&bits, 64, 0));
    EXPECT_EQ(64U, FindNextBitSet(&bits, 64, 32));
}

TEST(Bits, FindNextBitSet_Empty)
{
    BitWord bits = 0;
    EXPECT_EQ(64U, FindNextBitSet(&bits, 64, 0));
}

TEST(Bits, FindNextBitSet_First)
{
    BitWord bits = 1;
    EXPECT_EQ(0U, FindNextBitSet(&bits, 64, 0));
    EXPECT_EQ(64U, FindNextBitSet(&bits, 64, 1));
}

TEST(Bits, FindNextBitSet_Last)
{
    BitWord bits = UINT64_C(0x8000000000000000);
    EXPECT_EQ(63U, FindNextBitSet(&bits, 64, 0));
}

TEST(Bits, FindNextBitSetLong_Last)
{
    BitWord bits[] = { 0x0, UINT64_C(0x8000000000000000) };
    EXPECT_EQ(127U, FindNextBitSet(bits, 128, 0));
}

TEST(Bits, FindNextBitSetLong_Enumeration)
{
    BitWord bits[] = { 0x10101, 0x10101 };

    Array<uint32_t> setBits;

    auto len  = sizeof(bits) * 8;
    uint32_t i = FindNextBitSet(bits, len, 0);
    while (i != len)
    {
        setBits.pushBack(i);
        i = FindNextBitSet(bits, len, i + 1);
    }

    EXPECT_EQ(6U, setBits.size());
    EXPECT_EQ(0U, setBits[0]);
    EXPECT_EQ(8U, setBits[1]);
    EXPECT_EQ(16U, setBits[2]);
    EXPECT_EQ(64U + 0, setBits[3]);
    EXPECT_EQ(64U + 8, setBits[4]);
    EXPECT_EQ(64U + 16, setBits[5]);
}

TEST(Bits, FindNextBitSetLong_EnumerationAll)
{
    BitWord bits[] = { INDEX_MAX64, INDEX_MAX64 };

    Array<uint32_t> setBits;

    auto len  = sizeof(bits) * 8;
    uint32_t i = FindNextBitSet(bits, len, 0);
    while (i != len)
    {
        setBits.pushBack(i);
        i = FindNextBitSet(bits, len, i + 1);
    }

    EXPECT_EQ(128U, setBits.size());
}

TEST(Bits, FindNextBitSetLong_EnumerationNone)
{
    BitWord bits[] = { 0, 0 };

    Array<uint32_t> setBits;

    auto len  = sizeof(bits) * 8;
    uint32_t i = FindNextBitSet(bits, len, 0);
    while (i != len)
    {
        setBits.pushBack(i);
        i = FindNextBitSet(bits, len, i + 1);
    }

    EXPECT_EQ(0U, setBits.size());
}

TEST(Bits, FindNextBitSetLong_EnumerationInv)
{
    BitWord bits[] = { ~0x10101ULL, ~0x10101ULL };

    Array<uint32_t> setBits;

    auto len  = sizeof(bits) * 8;
    uint32_t i = FindNextBitCleared(bits, len, 0);
    while (i != len)
    {
        setBits.pushBack(i);
        i = FindNextBitCleared(bits, len, i + 1);
    }

    EXPECT_EQ(6U, setBits.size());
    EXPECT_EQ(0U, setBits[0]);
    EXPECT_EQ(8U, setBits[1]);
    EXPECT_EQ(16U, setBits[2]);
    EXPECT_EQ(64U + 0, setBits[3]);
    EXPECT_EQ(64U + 8, setBits[4]);
    EXPECT_EQ(64U + 16, setBits[5]);
}

TEST(Bits, FindNextBitSetLong_EnumerationAllInv)
{
    BitWord bits[] = { INDEX_MAX64, INDEX_MAX64 };

    Array<uint32_t> setBits;

    auto len  = sizeof(bits) * 8;
    uint32_t i = FindNextBitCleared(bits, len, 0);
    while (i != len)
    {
        setBits.pushBack(i);
        i = FindNextBitCleared(bits, len, i + 1);
    }

    EXPECT_EQ(0U, setBits.size());
}

TEST(Bits, FindNextBitSetLong_EnumerationNoneInv)
{
    BitWord bits[] = { 0, 0 };

    Array<uint32_t> setBits;

    auto len  = sizeof(bits) * 8;
    uint32_t i = FindNextBitCleared(bits, len, 0);
    while (i != len)
    {
        setBits.pushBack(i);
        i = FindNextBitCleared(bits, len, i + 1);
    }

    EXPECT_EQ(128U, setBits.size());
}

TEST(Bits, CompareSmall)
{
    BitWord dataA = 0x001FF100;
    BitWord dataB = 0x00100100;;

    EXPECT_FALSE(CompareBits(&dataA, &dataB, 0, 64));
    EXPECT_TRUE(CompareBits(&dataA, &dataB, 0, 8));
    EXPECT_FALSE(CompareBits(&dataA, &dataB, 9, 11));
    EXPECT_TRUE(CompareBits(&dataA, &dataB, 9, 1));
    EXPECT_TRUE(CompareBits(&dataA, &dataB, 9, 1));
    EXPECT_FALSE(CompareBits(&dataA, &dataB, 19, 13));
    EXPECT_TRUE(CompareBits(&dataA, &dataB, 20, 12));
    EXPECT_TRUE(CompareBits(&dataA, &dataB, 21, 11));
}

TEST(Bits, CompareCrossingWords)
{
    const BitWord dataA[] = { 0x80000000001FF100ULL, 0x80000000001FF101ULL };
    const BitWord dataB[] = { 0xF000000000100100ULL, 0x80000000001FF103ULL };

    EXPECT_TRUE(CompareBits(dataA, dataB, 63, 2));
    EXPECT_TRUE(CompareBits(dataA, dataB, 63, 1));
    EXPECT_TRUE(CompareBits(dataA, dataB, 64, 1));
    EXPECT_FALSE(CompareBits(dataA, dataB, 62, 4));
    EXPECT_FALSE(CompareBits(dataA, dataB, 63, 3));
    EXPECT_FALSE(CompareBits(dataA, dataB, 62, 3));
}

TEST(Bits, CompareCrossingMultiWords)
{
    const BitWord dataA[] = { 0x80000000001FF100ULL, 0xFF00FF00FF, 0x80000000001FF100ULL };
    const BitWord dataB[] = { 0x80000000001FF100ULL, 0xFF00FF00FF, 0x80000000001FF100ULL };

    EXPECT_TRUE(CompareBits(dataA, dataB, 47, 64 + 40));
}

TEST(Bits, CompareCrossingMultiWordsDiff)
{
    const uint64_t dataA[] = { 0x80000000001FF100ULL, 0xFF00FF00FF, 0x80000000001FF100ULL };
    const uint64_t dataB[] = { 0x80000000001FF100ULL, 0xFF001100FF, 0x80000000001FF100ULL };

    EXPECT_FALSE(CompareBits(dataA, dataB, 47, 64 + 40));
}

END_INFERNO_NAMESPACE()