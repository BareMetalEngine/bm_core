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

TEST(StringView, EmptyStringReportsEmpty)
{
	StringView x;
	EXPECT_TRUE(x.empty());
}

TEST(StringView, EmptyStringHasZeroLength)
{
	StringView x;
	EXPECT_EQ(0U, x.length());
}

TEST(StringView, EmptyStringHasZeroUniLength)
{
	StringView x;
	EXPECT_EQ(0U, x.unicodeLength());
}

TEST(StringView, EmptyStringViewHasNullBuffer)
{
	const char* txt = "test";
	StringView x(txt, txt);
	EXPECT_EQ(txt, x.data());
}

TEST(StringView, CreatedStringReportsNotEmpty)
{
	StringView x("test");
	EXPECT_FALSE(x.empty());
}

TEST(StringView, CreatedStringReportsLength)
{
	StringView x("test");
	ASSERT_EQ(4U, x.length());
}

TEST(StringView, CreatedStringReportsSameUniLength)
{
	StringView x("test");
	ASSERT_EQ(4U, x.unicodeLength());
}

TEST(StringView, CreatedStringReportsChars)
{
	StringView x("test");
	EXPECT_EQ('t', x.data()[0]);
	EXPECT_EQ('e', x.data()[1]);
	EXPECT_EQ('s', x.data()[2]);
	EXPECT_EQ('t', x.data()[3]);
}

TEST(StringView, ManualConstructionCanHaveZeroElements)
{
	const char* txt = "te\0st";
	StringView x(txt, 5);
	ASSERT_EQ(5, x.length());
	EXPECT_EQ('t', x.data()[0]);
	EXPECT_EQ('e', x.data()[1]);
	EXPECT_EQ(0, x.data()[2]);
	EXPECT_EQ('s', x.data()[3]);
	EXPECT_EQ('t', x.data()[4]);
}

TEST(StringView, CreatedUnicodeStringNotEmpty)
{
	StringView txt(u8"gęś zółćią");
	EXPECT_FALSE(txt.empty());
}

TEST(StringView, CreatedUnicodeStringReportsLength)
{
	StringView txt(u8"gęś zółćią");
	ASSERT_EQ(16, txt.length());
}

TEST(StringView, CreatedUnicodeStringReportsUniLength)
{
	StringView txt(u8"gęś zółćią");
	ASSERT_EQ(10, txt.unicodeLength());
}

TEST(StringView, CopyConstructPreservesPointer)
{
	StringView x("test");
	StringView y(x);

	EXPECT_EQ(x.data(), y.data());	
}

TEST(StringView, CopyConstructPreservesLength)
{
	StringView x("test");
	StringView y(x);

	EXPECT_EQ(x.length(), y.length());
}

TEST(StringView, CopyConstructPreservesChars)
{
	StringView x("test");
	StringView y(x);

	ASSERT_EQ(4U, y.length());
	EXPECT_EQ('t', y.data()[0]);
	EXPECT_EQ('e', y.data()[1]);
	EXPECT_EQ('s', y.data()[2]);
	EXPECT_EQ('t', y.data()[3]);
}

TEST(StringView, MoveConstructClearsPointer)
{
	StringView x("test");
	StringView y(std::move(x));
	EXPECT_EQ(nullptr, x.data());
}

TEST(StringView, MoveConstructCopiesPointer)
{
	StringView x("test");

	const auto* oldXPtr = x.data();
	StringView y(std::move(x));

	EXPECT_EQ(oldXPtr, y.data());
}

TEST(StringView, MoveConstructClearsLength)
{
	StringView x("test");
	StringView y(std::move(x));
	EXPECT_EQ(0, x.length());
}

TEST(StringView, MoveConstructCopiesLength)
{
	StringView x("test");

	const auto oldXLength = x.length();
	StringView y(std::move(x));
	EXPECT_EQ(oldXLength, y.length());
}

TEST(StringView, CopyAssignmentPreservesDataPointer)
{
	StringView x("test");
	StringView y;

	y = x;

	EXPECT_EQ(x.data(), y.data());
}

TEST(StringView, CopyAssignmentPreservesLength)
{
	StringView x("test");
	StringView y;

	y = x;

	EXPECT_EQ(x.length(), y.length());
}

TEST(StringView, CopyAssignmentPreservesChars)
{
	StringView x("test");
	StringView y;
	
	y = x;

	ASSERT_EQ(4U, y.length());
	EXPECT_EQ('t', y.data()[0]);
	EXPECT_EQ('e', y.data()[1]);
	EXPECT_EQ('s', y.data()[2]);
	EXPECT_EQ('t', y.data()[3]);
}

TEST(StringView, MoveAssignmentClearsPointer)
{
	StringView x("test");
	StringView y;

	y = std::move(x);

	EXPECT_EQ(nullptr, x.data());
}

TEST(StringView, MoveAssignmentClearsLength)
{
	StringView x("test");
	StringView y;

	y = std::move(x);

	EXPECT_EQ(0, x.length());
}

TEST(StringView, MoveAssignmentCopiesPointer)
{
	StringView x("test");
	StringView y;

	const auto* oldXPtr = x.data();
	y = std::move(x);

	EXPECT_EQ(oldXPtr, y.data());
}

TEST(StringView, MoveAssignmentCopiesLength)
{
	StringView x("test");
	StringView y;

	const auto oldLength = x.length();
	y = std::move(x);

	EXPECT_EQ(oldLength, y.length());
}

TEST(StringView, MoveAssignmentCopiesChars)
{
	StringView x("test");
	StringView y;

	y = std::move(x);

	ASSERT_EQ(4U, y.length());
	EXPECT_EQ('t', y.data()[0]);
	EXPECT_EQ('e', y.data()[1]);
	EXPECT_EQ('s', y.data()[2]);
	EXPECT_EQ('t', y.data()[3]);
}

TEST(StringView, EqualTest)
{
	StringView x("test");
	StringView y("test");
	EXPECT_EQ(x, y);
}

TEST(StringView, EmptyStringEqual)
{
	StringView x;
	StringView y("");
	EXPECT_EQ(x, y);
}

TEST(StringView, EmptyStringEqualGlobal)
{
	StringView x = StringView::EMPTY();
	StringView y;
	EXPECT_EQ(x, y);
}
TEST(StringView, EqualTestFails)
{
	StringView x("test");
	StringView y("dupa");
	EXPECT_NE(x, y);
}

TEST(StringView, RawEqualTest)
{
	StringView x("test");
	auto y = "test";
	EXPECT_EQ(x, y);
}

TEST(StringView, RawEqualTestFails)
{
	StringView x("test");
	auto y = "dupa";
	EXPECT_NE(x, y);
}

TEST(StringView, RawNonEqualTest)
{
	StringView x("test");
	auto y = "dupa";
	EXPECT_NE(x, y);
}

TEST(StringView, RawNonEqualTestFails)
{
	StringView x("test");
	auto y = "test";
	EXPECT_EQ(x, y);
}

TEST(StringView, OrderTest)
{
	StringView x("Alice");
	StringView y("Bob");
	EXPECT_LT(x, y);
}

TEST(StringView, EmptyStringBeforeAnyString)
{
	StringView x;
	StringView y(" ");
	EXPECT_LT(x, y);
}

TEST(StringView, Compare)
{
	StringView x("Bob");

	EXPECT_LT(0, x.compare(StringView::EMPTY()));
	EXPECT_LT(0, x.compare(StringView("Alice")));
	EXPECT_EQ(0, x.compare(StringView("Bob")));
	EXPECT_GT(0, x.compare(StringView("Charles ")));
}

TEST(StringView, CompareView)
{
	StringView x("Bob");

	EXPECT_LT(0, x.compare(""));
	EXPECT_LT(0, x.compare("Alice"));
	EXPECT_EQ(0, x.compare("Bob"));
	EXPECT_NE(0, x.compare("bob"));
	EXPECT_GT(0, x.compare("Charles"));
}

TEST(StringView, CompareNoCase)
{
	StringView x("bob");

	EXPECT_LT(0, x.compare(StringView::EMPTY(), StringCaseComparisonMode::NoCase));
	EXPECT_LT(0, x.compare(StringView("alice"), StringCaseComparisonMode::NoCase));
	EXPECT_EQ(0, x.compare(StringView("Bob"), StringCaseComparisonMode::NoCase));
	EXPECT_EQ(0, x.compare(StringView("bob"), StringCaseComparisonMode::NoCase));
	EXPECT_GT(0, x.compare(StringView("CHARLES"), StringCaseComparisonMode::NoCase));
}

TEST(StringView, CompareNoCaseView)
{
	StringView x("bob");

	EXPECT_LT(0, x.compare("", StringCaseComparisonMode::NoCase));
	EXPECT_LT(0, x.compare("Alice", StringCaseComparisonMode::NoCase));
	EXPECT_EQ(0, x.compare("Bob", StringCaseComparisonMode::NoCase));
	EXPECT_EQ(0, x.compare("bob", StringCaseComparisonMode::NoCase));
	EXPECT_GT(0, x.compare("CHARLES", StringCaseComparisonMode::NoCase)); // fuck Charles
}

TEST(StringView, CompareN)
{
	StringView x("testX");

	EXPECT_NE(0, x.compare("testY"));
	EXPECT_NE(0, x.compare(StringView("TESTY", 4)));
	EXPECT_EQ(0, x.leftPart(4).compare(StringView("testY", 4)));
}

TEST(StringView, CompareNoCaseN)
{
	StringView x("testX");

	EXPECT_NE(0, x.compare("testY", StringCaseComparisonMode::NoCase));
	EXPECT_EQ(0, x.leftPart(4).compare(StringView("TESTY", 4), StringCaseComparisonMode::NoCase));
	EXPECT_EQ(0, x.leftPart(4).compare(StringView("testY", 4), StringCaseComparisonMode::NoCase));
}

TEST(StringView, LeftPartOfEmptyStringIsAlwaysEmpty)
{
	StringView x;

	EXPECT_EQ("", x.leftPart(0));
	EXPECT_EQ("", x.leftPart(100));
}

TEST(StringView, LeftPartNotBiggerThanString)
{
	StringView x("test");

	EXPECT_EQ(StringView::EMPTY(), x.leftPart(0));
	EXPECT_EQ("t", x.leftPart(1));
	EXPECT_EQ("te", x.leftPart(2));
	EXPECT_EQ("tes", x.leftPart(3));
	EXPECT_EQ("test", x.leftPart(4));
	EXPECT_EQ("test", x.leftPart(5));
	EXPECT_EQ("test", x.leftPart(100));
}

TEST(StringView, RightPartOfEmptyStringIsAlwaysEmpty)
{
	StringView x;

	EXPECT_EQ(StringView::EMPTY(), x.rightPart(0));
	EXPECT_EQ(StringView::EMPTY(), x.rightPart(100));

}

TEST(StringView, RightPartNotBiggerThanString)
{
	StringView x("test");

	EXPECT_EQ(StringView::EMPTY(), x.rightPart(0));
	EXPECT_EQ("t", x.rightPart(1));
	EXPECT_EQ("st", x.rightPart(2));
	EXPECT_EQ("est", x.rightPart(3));
	EXPECT_EQ("test", x.rightPart(4));
	EXPECT_EQ("test", x.rightPart(5));
	EXPECT_EQ("test", x.rightPart(100));
}

TEST(StringView, EmptySubString)
{
	StringView x("test");
	EXPECT_EQ(StringView(), x.subString(0, 0));
}

TEST(StringView, SubStringOutOfRangeIsEmpty)
{
	StringView x("test");
	EXPECT_EQ(StringView(), x.subString(100));
}

TEST(StringView, SubStringRangeIsClamped)
{
	StringView x("test");
	EXPECT_EQ(StringView("st"), x.subString(2, 100));
}

TEST(StringView, SubStringGenericTest)
{
	StringView x("test");
	EXPECT_EQ("es", x.subString(1, 2));
	EXPECT_EQ("e", x.subString(1, 1));
	EXPECT_EQ("t", x.subString(0, 1));
	EXPECT_EQ("t", x.subString(3, 1));
	EXPECT_EQ("test", x.subString(0, 4));
	EXPECT_EQ("test", x.subString(0));
}

TEST(StringView, SplitMiddle)
{
	StringView x("test");

	StringView left, right;
	x.split(2, left, right);

	EXPECT_EQ("te", left);
	EXPECT_EQ("st", right);
}

TEST(StringView, SplitAtZero)
{
	StringView x("test");

	StringView left, right;
	x.split(0, left, right);

	EXPECT_EQ("", left);
	EXPECT_EQ("test", right);
}

TEST(StringView, SplitAtEnd)
{
	StringView x("test");

	StringView left, right;
	x.split(4, left, right);

	EXPECT_EQ("test", left);
	EXPECT_EQ("", right);
}

TEST(StringView, SplitOutOfRange)
{
	StringView x("test");

	StringView left, right;
	x.split(100, left, right);

	EXPECT_EQ("test", left);
	EXPECT_EQ("", right);
}

TEST(StringView, FindFirstChar)
{
	StringView x("HelloWorld");

	EXPECT_EQ(2, x.findFirstChar('l'));
	EXPECT_EQ(1, x.findFirstChar('e'));
	EXPECT_EQ(-1, x.findFirstChar('z'));
}

TEST(StringView, FindLastChar)
{
	StringView x("HelloWorld");

	EXPECT_EQ(8, x.findLastChar('l'));
	EXPECT_EQ(1, x.findLastChar('e'));
	EXPECT_EQ(-1, x.findLastChar('z'));
}

TEST(StringView, FindString)
{
	StringView x("HelloWorldhello");

	EXPECT_EQ(5, x.findStr("World"));
}

TEST(StringView, FindStringWithOffset)
{
	StringView x("HelloWorldhello");

	EXPECT_EQ(1, x.findStr("ello", StringCaseComparisonMode::WithCase, 0));
	EXPECT_EQ(11, x.findStr("ello", StringCaseComparisonMode::WithCase, 2));
}

TEST(StringView, FindStringRev)
{
	StringView x("HelloWorldhello");

	EXPECT_EQ(11, x.findRevStr("ello"));
}

TEST(StringView, FindStringRevWithOffset)
{
	StringView x("HelloWorldhello");

	EXPECT_EQ(1, x.findRevStr("ello", StringCaseComparisonMode::WithCase, 5));
}

TEST(StringView, BeginsWith)
{
	StringView x("HelloWorldhello");

	EXPECT_TRUE(x.beginsWith(""));
	EXPECT_TRUE(x.beginsWith("Hello"));
	EXPECT_FALSE(x.beginsWith("ello"));
	EXPECT_FALSE(x.beginsWith("hello"));
	EXPECT_TRUE(x.beginsWith("HelloWorldhello"));
	EXPECT_FALSE(x.beginsWith("HelloWorldhello2"));
}

TEST(StringView, BeginsWithNoCase)
{
	StringView x("HelloWorldhello");

	EXPECT_TRUE(x.beginsWith("", StringCaseComparisonMode::NoCase));
	EXPECT_TRUE(x.beginsWith("Hello", StringCaseComparisonMode::NoCase));
	EXPECT_FALSE(x.beginsWith("ello", StringCaseComparisonMode::NoCase));
	EXPECT_TRUE(x.beginsWith("hello", StringCaseComparisonMode::NoCase));
	EXPECT_TRUE(x.beginsWith("HELLOWORLDHELLO", StringCaseComparisonMode::NoCase));
	EXPECT_FALSE(x.beginsWith("HelloWorldhello2", StringCaseComparisonMode::NoCase));
}

TEST(StringView, EndsWith)
{
	StringView x("HelloWorldhello");

	EXPECT_TRUE(x.endsWith(""));
	EXPECT_TRUE(x.endsWith("hello"));
	EXPECT_TRUE(x.endsWith("ello"));
	EXPECT_FALSE(x.endsWith("ell"));
	EXPECT_FALSE(x.endsWith("Hello"));
	EXPECT_TRUE(x.endsWith("HelloWorldhello"));
	EXPECT_FALSE(x.endsWith("2HelloWorldhello"));
}

TEST(StringView, EndsWithNoCase)
{
	StringView x("HelloWorldhello");

	EXPECT_TRUE(x.endsWith("", StringCaseComparisonMode::NoCase));
	EXPECT_TRUE(x.endsWith("Hello", StringCaseComparisonMode::NoCase));
	EXPECT_TRUE(x.endsWith("ELLO", StringCaseComparisonMode::NoCase));
}

TEST(StringView, StringAfterFirst)
{
	StringView x("Ala_ma_kota_Ala_ma");
	EXPECT_EQ("_kota_Ala_ma", x.afterFirst("ma"));
	EXPECT_EQ("_Ala_ma", x.afterFirst("kota"));
	EXPECT_TRUE(x.afterFirst("dupa").empty());
	EXPECT_TRUE(x.afterFirst("MA").empty());
}

TEST(StringView, StringAfterFirstNoCase)
{
	StringView x("Ala_ma_kota_Ala_ma");
	EXPECT_EQ("_kota_Ala_ma", x.afterFirst("MA", bm::StringCaseComparisonMode::NoCase));
	EXPECT_EQ("_ma_kota_Ala_ma", x.afterFirst("ala", bm::StringCaseComparisonMode::NoCase));
}

TEST(StringView, StringBeforeFirst)
{
	StringView x("Ala_ma_kota_Ala_ma");
	EXPECT_EQ("Ala_", x.beforeFirst("ma"));
	EXPECT_TRUE(x.beforeFirst("MA").empty());
	EXPECT_EQ("Ala_ma_", x.beforeFirst("kota"));
}

TEST(StringView, StringBeforeFirstNoCase)
{
	StringView x("Ala_ma_kota_Ala_ma");
	EXPECT_EQ("Ala_", x.beforeFirst("MA", bm::StringCaseComparisonMode::NoCase));
}

TEST(StringView, StringAfterLast)
{
	StringView x("Ala_ma_kota_Ala_ma");
	EXPECT_TRUE(x.afterLast("ma").empty());
	EXPECT_EQ("_ma", x.afterLast("Ala"));
	EXPECT_TRUE(x.afterLast("dupa").empty());
}

TEST(StringView, StringAfterLastNoCase)
{
	StringView x("Ala_ma_kota_Ala_ma");
	EXPECT_EQ("_ma", x.afterLast("ala", bm::StringCaseComparisonMode::NoCase));
	EXPECT_TRUE(x.afterLast("dupa", bm::StringCaseComparisonMode::NoCase).empty());
}

TEST(StringView, StringBeforeLast)
{
	StringView x("Ala_ma_kota_Ala_ma");
	EXPECT_TRUE(x.beforeLast("ala").empty());
	EXPECT_TRUE(x.beforeLast("Ala") == "Ala_ma_kota_");
	EXPECT_TRUE(x.beforeLast("kota") == "Ala_ma_");
	EXPECT_TRUE(x.beforeLast("ma") == "Ala_ma_kota_Ala_");
}

TEST(StringView, StringBeforeLastNoCase)
{
	StringView x("Ala_ma_kota_Ala_ma");
	EXPECT_EQ("Ala_ma_kota_", x.beforeLast("ala", bm::StringCaseComparisonMode::NoCase));
	EXPECT_EQ("Ala_ma_", x.beforeLast("KOTA", bm::StringCaseComparisonMode::NoCase));
}

//--

END_INFERNO_NAMESPACE()
