/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "bm/core/containers/include/stringView.h"
#include "bm/core/containers/include/inplaceArray.h"

BEGIN_INFERNO_NAMESPACE()

//--

TEST(StringSlice, EmptySlicesToZeroElements)
{
	InplaceArray<StringView, 4> items;
	StringView("").slice(";", items);

	EXPECT_EQ(0, items.size());
}

TEST(StringSlice, EmptySlicesToZeroElementsEvenIfEmptyIgnored)
{
	InplaceArray<StringView, 4> items;
	StringView("").slice(";", items, StringSliceBit::KeepEmpty);

	EXPECT_EQ(0, items.size());
}

TEST(StringSlice, SingleItem)
{
	InplaceArray<StringView, 4> items;
	StringView("test").slice(";", items);

	EXPECT_EQ(1, items.size());
	EXPECT_EQ("test", items[0]);
}

TEST(StringSlice, SingleItemTrimmed)
{
	InplaceArray<StringView, 4> items;
	StringView(" test ").slice(";", items);

	EXPECT_EQ(1, items.size());
	EXPECT_EQ("test", items[0]);
}

TEST(StringSlice, SingleItemTrimIgnored)
{
	InplaceArray<StringView, 4> items;
	StringView(" test ").slice(";", items, StringSliceBit::IgnoreTrim);

	EXPECT_EQ(1, items.size());
	EXPECT_EQ(" test ", items[0]);
}

TEST(StringSlice, MultiItems)
{
	InplaceArray<StringView, 4> items;
	StringView("ala;ma;kota").slice(";", items);

	EXPECT_EQ(3, items.size());
	EXPECT_EQ("ala", items[0]);
	EXPECT_EQ("ma", items[1]);
	EXPECT_EQ("kota", items[2]);
}

TEST(StringSlice, MultiItemsTrimmed)
{
	InplaceArray<StringView, 4> items;
	StringView(" ala ; ma ; kota ").slice(";", items);

	EXPECT_EQ(3, items.size());
	EXPECT_EQ("ala", items[0]);
	EXPECT_EQ("ma", items[1]);
	EXPECT_EQ("kota", items[2]);
}

TEST(StringSlice, MultiItemsTrimIgnored)
{
	InplaceArray<StringView, 4> items;
	StringView(" ala ; ma ; kota ").slice(";", items, StringSliceBit::IgnoreTrim);

	EXPECT_EQ(3, items.size());
	EXPECT_EQ(" ala ", items[0]);
	EXPECT_EQ(" ma ", items[1]);
	EXPECT_EQ(" kota ", items[2]);
}

TEST(StringSlice, DanglingSeparatorIgnoredFront)
{
	InplaceArray<StringView, 4> items;
	StringView(";ala").slice(";", items);

	EXPECT_EQ(1, items.size());
	EXPECT_EQ("ala", items[0]);
}

TEST(StringSlice, DanglingSeparatorIgnoredBack)
{
	InplaceArray<StringView, 4> items;
	StringView("ala;").slice(";", items);

	EXPECT_EQ(1, items.size());
	EXPECT_EQ("ala", items[0]);
}

TEST(StringSlice, DanglingSeparatorIgnoredBoth)
{
	InplaceArray<StringView, 4> items;
	StringView(";ala;").slice(";", items);

	EXPECT_EQ(1, items.size());
	EXPECT_EQ("ala", items[0]);
}

TEST(StringSlice, QuotesNotIngored)
{
	InplaceArray<StringView, 4> items;
	StringView("ala \"ma;kota\"").slice(";", items);

	EXPECT_EQ(1, items.size());
	EXPECT_EQ("ala \"ma;kota\"", items[0]);
}

TEST(StringSlice, QuotesIngoredWhenRequested)
{
	InplaceArray<StringView, 4> items;
	StringView("ala \"ma;kota\"").slice(";", items, StringSliceBit::IgnoreQuotes);

	ASSERT_EQ(2, items.size());
	EXPECT_EQ("ala \"ma", items[0]);
	EXPECT_EQ("kota\"", items[1]);
}

TEST(StringSlice, EmptyItemsIgnored)
{
	InplaceArray<StringView, 4> items;
	StringView("ala;;kota").slice(";", items);

	EXPECT_EQ(2, items.size());
	EXPECT_EQ("ala", items[0]);
	EXPECT_EQ("kota", items[1]);
}

TEST(StringSlice, EmptyWhitespaceItemsIgnored)
{
	InplaceArray<StringView, 4> items;
	StringView("ala;  ;kota").slice(";", items);

	EXPECT_EQ(2, items.size());
	EXPECT_EQ("ala", items[0]);
	EXPECT_EQ("kota", items[1]);
}

TEST(StringSlice, EmptyWhitespaceItemsIgnoredEvenWithTrimingDisabled)
{
	InplaceArray<StringView, 4> items;
	StringView("ala;  ;kota").slice(";", items, StringSliceBit::IgnoreTrim);

	EXPECT_EQ(2, items.size());
	EXPECT_EQ("ala", items[0]);
	EXPECT_EQ("kota", items[1]);
}

TEST(StringSlice, EmptyNotIngoredWhenRequested)
{
	InplaceArray<StringView, 4> items;
	StringView("ala;;kota").slice(";", items, StringSliceBit::KeepEmpty);

	EXPECT_EQ(3, items.size());
	EXPECT_EQ("ala", items[0]);
	EXPECT_EQ("", items[1]);
	EXPECT_EQ("kota", items[2]);
}

TEST(StringSlice, EmptyNotIngoredEvenIfWhitespaces)
{
	InplaceArray<StringView, 4> items;
	StringView("ala;  ;kota").slice(";", items, { StringSliceBit::IgnoreTrim, StringSliceBit::KeepEmpty });

	EXPECT_EQ(3, items.size());
	EXPECT_EQ("ala", items[0]);
	EXPECT_EQ("  ", items[1]);
	EXPECT_EQ("kota", items[2]);
}

TEST(StringSlice, DangingSeparatorNotIgnoredFront)
{
	InplaceArray<StringView, 4> items;
	StringView(";ala").slice(";", items, StringSliceBit::KeepEmpty);

	EXPECT_EQ(2, items.size());
	EXPECT_EQ("", items[0]);
	EXPECT_EQ("ala", items[1]);
}

TEST(StringSlice, DangingSeparatorNotIgnoredBack)
{
	InplaceArray<StringView, 4> items;
	StringView(";ala").slice(";", items, StringSliceBit::KeepEmpty);

	EXPECT_EQ(2, items.size());
	EXPECT_EQ("", items[0]);
	EXPECT_EQ("ala", items[1]);
}

TEST(StringSlice, DangingSeparatorNotIgnoredBoth)
{
	InplaceArray<StringView, 4> items;
	StringView(";ala;").slice(";", items, StringSliceBit::KeepEmpty);

	EXPECT_EQ(3, items.size());
	EXPECT_EQ("", items[0]);
	EXPECT_EQ("ala", items[1]);
	EXPECT_EQ("", items[2]);
}

//--

//--

END_INFERNO_NAMESPACE()
