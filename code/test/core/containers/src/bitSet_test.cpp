/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "bm/core/containers/include/bitSet.h"

BEGIN_INFERNO_NAMESPACE()

DECLARE_TEST_FILE(BitSet);

TEST(BitSet, InitEmpty)
{
	BitSet b;
	EXPECT_EQ(0, b.bitCount());
	EXPECT_EQ(0, b.wordCount());
}

TEST(BitSet, InitZeros)
{
	BitSet b(10, EBitStateZero::ZERO);
	EXPECT_EQ(10, b.bitCount());
	EXPECT_FALSE(b[0]);
	EXPECT_FALSE(b[9]);
}

TEST(BitSet, InitOnes)
{
	BitSet b(10, EBitStateOne::ONE);
	EXPECT_EQ(10, b.bitCount());
	EXPECT_TRUE(b[0]);
	EXPECT_TRUE(b[9]);
}

END_INFERNO_NAMESPACE()