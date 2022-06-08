/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "bm/core/containers/include/stringBuilder.h"

BEGIN_INFERNO_NAMESPACE();

TEST(StringBuilder, EmptyStateDetected)
{
	StringBuilder txt;
	EXPECT_TRUE(txt.empty());
}

TEST(StringBuilder, EmptyStateValidPointer)
{
	StringBuilder txt;
	EXPECT_NE(nullptr, txt.c_str());
}

TEST(StringBuilder, EmptyStateZeroString)
{
	StringBuilder txt;
	EXPECT_EQ(0, *txt.c_str());
}

TEST(StringBuilder, EmptyStateLocal)
{
	StringBuilder txt;
	EXPECT_TRUE(txt.local());
}

TEST(StringBuilder, EmptyStateZeroLength)
{
	StringBuilder txt;
	EXPECT_EQ(0, txt.view().length());
}

TEST(StringBuilder, AppendChangesEmpty)
{
	StringBuilder txt;
	txt.append("a");
	EXPECT_FALSE(txt.empty());
}

TEST(StringBuilder, AppendChangesLength)
{
	StringBuilder txt;
	txt.append("a");
	EXPECT_EQ(1, txt.view().length());
}

TEST(StringBuilder, AppendValidChar)
{
	StringBuilder txt;
	txt.append("a");
	EXPECT_EQ('a', txt.c_str()[0]);
}

TEST(StringBuilder, AppendZeroTerminated)
{
	StringBuilder txt;
	txt.append("a");
	EXPECT_EQ(0, txt.c_str()[1]);
}

TEST(StringBuilder, DoubleAppendChangesLength)
{
	StringBuilder txt;
	txt.append("a");
	txt.append("b");
	EXPECT_EQ(2, txt.view().length());
}

TEST(StringBuilder, DoubleAppendValidChars)
{
	StringBuilder txt;
	txt.append("a");
	txt.append("b");
	EXPECT_EQ('a', txt.c_str()[0]);
	EXPECT_EQ('b', txt.c_str()[1]);
}

TEST(StringBuilder, DoubleAppendZeroTerminated)
{
	StringBuilder txt;
	txt.append("a");
	txt.append("b");
	EXPECT_EQ(0, txt.c_str()[2]);
}

END_INFERNO_NAMESPACE()
