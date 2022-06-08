/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "bm/core/containers/include/stringBuf.h"

BEGIN_INFERNO_NAMESPACE()

TEST(StringBuf, EmptyStrintReportsEmpty)
{
    StringBuf x;
    EXPECT_TRUE(x.empty());
}

TEST(StringBuf, EmptyStringHasZeroLength)
{
    StringBuf x;
    EXPECT_EQ(0U, x.length());
}

TEST(StringBuf, EmptyStringHasZeroUniLength)
{
	StringBuf x;
	EXPECT_EQ(0U, x.unicodeLength());
}

TEST(StringBuf, AllEmptyStringsHasCBuf)
{
    StringBuf x, y;
    EXPECT_TRUE(x.c_str());
    EXPECT_EQ(0, x.c_str()[0]);
}

TEST(StringBuf, AllEmptyStringsHaveSameCBuf)
{
    StringBuf x, y;
    EXPECT_EQ(x.c_str(), y.c_str());
}

TEST(StringBuf, CreatedStringReportsNotEmpty)
{
    StringBuf x("test");
    EXPECT_FALSE(x.empty());
}

TEST(StringBuf, CreatedStringReportsLength)
{
    StringBuf x("test");
    ASSERT_EQ(4U, x.length());
}

TEST(StringBuf, CreatedStringReportsSameUniLength)
{
	StringBuf x("test");
	ASSERT_EQ(4U, x.unicodeLength());
}

TEST(StringBuf, CreatedStringReportsChars)
{
	StringBuf x("test");
    EXPECT_EQ('t', x.c_str()[0]);
    EXPECT_EQ('e', x.c_str()[1]);
    EXPECT_EQ('s', x.c_str()[2]);
    EXPECT_EQ('t', x.c_str()[3]);
}

TEST(StringBuf, CreatedStringZeroTerminated)
{
    StringBuf x("test");
    EXPECT_EQ(0, x.c_str()[4]);
}

TEST(StringBuf, CreatedUnicodeStringNotEmpty)
{
    StringBuf txt(u8"gęś zółćią");
    EXPECT_FALSE(txt.empty());
}

TEST(StringBuf, CreatedUnicodeStringReportsLength)
{
	StringBuf txt(u8"gęś zółćią");
    ASSERT_EQ(16, txt.length());
}

TEST(StringBuf, CreatedUnicodeStringReportsUniLength)
{
	StringBuf txt(u8"gęś zółćią");
	ASSERT_EQ(10, txt.unicodeLength());
}

TEST(StringBuf, CopyConstruct)
{
    StringBuf x("test");
    StringBuf y(x);

    ASSERT_EQ(4U, x.length());
    EXPECT_EQ('t', x.c_str()[0]);
    EXPECT_EQ('e', x.c_str()[1]);
    EXPECT_EQ('s', x.c_str()[2]);
    EXPECT_EQ('t', x.c_str()[3]);
    EXPECT_EQ(0, x.c_str()[4]);

    ASSERT_EQ(4U, y.length());
    EXPECT_EQ('t', y.c_str()[0]);
    EXPECT_EQ('e', y.c_str()[1]);
    EXPECT_EQ('s', y.c_str()[2]);
    EXPECT_EQ('t', y.c_str()[3]);
    EXPECT_EQ(0, y.c_str()[4]);
}

TEST(StringBuf, MoveConstruct)
{
    StringBuf x("test");
    StringBuf y(std::move(x));
    EXPECT_TRUE(x.empty());
    ASSERT_EQ(4U, y.length());
    EXPECT_EQ('t', y.c_str()[0]);
    EXPECT_EQ('e', y.c_str()[1]);
    EXPECT_EQ('s', y.c_str()[2]);
    EXPECT_EQ('t', y.c_str()[3]);
    EXPECT_EQ(0, y.c_str()[4]);
}

TEST(StringBuf, CopyAssignment)
{
    StringBuf x("test");
    StringBuf y;
    
    y = x;

    ASSERT_EQ(4U, x.length());
    EXPECT_EQ('t', x.c_str()[0]);
    EXPECT_EQ('e', x.c_str()[1]);
    EXPECT_EQ('s', x.c_str()[2]);
    EXPECT_EQ('t', x.c_str()[3]);
    EXPECT_EQ(0, x.c_str()[4]);

    ASSERT_EQ(4U, y.length());
    EXPECT_EQ('t', y.c_str()[0]);
    EXPECT_EQ('e', y.c_str()[1]);
    EXPECT_EQ('s', y.c_str()[2]);
    EXPECT_EQ('t', y.c_str()[3]);
    EXPECT_EQ(0, y.c_str()[4]);
}

TEST(StringBuf, MoveAssignment)
{
    StringBuf x("test");
    StringBuf y = std::move(x);

    EXPECT_TRUE(x.empty());

    ASSERT_EQ(4U, y.length());
    EXPECT_EQ('t', y.c_str()[0]);
    EXPECT_EQ('e', y.c_str()[1]);
    EXPECT_EQ('s', y.c_str()[2]);
    EXPECT_EQ('t', y.c_str()[3]);
    EXPECT_EQ(0, y.c_str()[4]);
}

TEST(StringBuf, EqualTest)
{
    StringBuf x("test");
    StringBuf y("test");
    EXPECT_EQ(x, y);
}

TEST(StringBuf, EmptyStringEqual)
{
	StringBuf x;
	StringBuf y("");
    EXPECT_EQ(x, y);
}

TEST(StringBuf, EmptyStringEqualGlobal)
{
    StringBuf x = StringBuf::EMPTY();
	StringBuf y;
    EXPECT_EQ(x, y);
}
TEST(StringBuf, EqualTestFails)
{
    StringBuf x("test");
    StringBuf y("dupa");
    EXPECT_NE(x, y);
}

TEST(StringBuf, RawEqualTest)
{
    StringBuf x("test");
    auto y  = "test";
    EXPECT_EQ(x, y);
}

TEST(StringBuf, RawEqualTestFails)
{
    StringBuf x("test");
    auto y  = "dupa";
    EXPECT_NE(x, y);
}

TEST(StringBuf, RawNonEqualTest)
{
    StringBuf x("test");
    auto y  = "dupa";
    EXPECT_NE(x, y);
}

TEST(StringBuf, RawNonEqualTestFails)
{
    StringBuf x("test");
    auto y  = "test";
    EXPECT_EQ(x, y);
}

TEST(StringBuf, OrderTest)
{
    StringBuf x("Alice");
    StringBuf y("Bob");
    EXPECT_LT(x, y);
}

TEST(StringBuf, EmptyStringBeforeAnyString)
{
    StringBuf x;
    StringBuf y(" ");
    EXPECT_LT(x, y);
}

TEST(StringBuf, Compare)
{
    StringBuf x("Bob");

    EXPECT_LT(0, x.compare(StringBuf::EMPTY()));
    EXPECT_LT(0, x.compare(StringBuf("Alice")));
    EXPECT_EQ(0, x.compare(StringBuf("Bob")));
    EXPECT_GT(0, x.compare(StringBuf("Charles ")));
}

TEST(StringBuf, CompareView)
{
    StringBuf x("Bob");

	EXPECT_LT(0, x.compare(""));
	EXPECT_LT(0, x.compare("Alice"));
	EXPECT_EQ(0, x.compare("Bob"));
	EXPECT_NE(0, x.compare("bob"));
	EXPECT_GT(0, x.compare("Charles"));
}

TEST(StringBuf, CompareNoCase)
{
    StringBuf x("bob");

    EXPECT_LT(0, x.compare(StringBuf::EMPTY(), StringCaseComparisonMode::NoCase));
    EXPECT_LT(0, x.compare(StringBuf("alice"), StringCaseComparisonMode::NoCase));
    EXPECT_EQ(0, x.compare(StringBuf("Bob"), StringCaseComparisonMode::NoCase));
    EXPECT_EQ(0, x.compare(StringBuf("bob"), StringCaseComparisonMode::NoCase));
    EXPECT_GT(0, x.compare(StringBuf("CHARLES"), StringCaseComparisonMode::NoCase));
}

TEST(StringBuf, CompareNoCaseView)
{
    StringBuf x("bob");

    EXPECT_LT(0, x.compare("", StringCaseComparisonMode::NoCase));
    EXPECT_LT(0, x.compare("Alice", StringCaseComparisonMode::NoCase));
    EXPECT_EQ(0, x.compare("Bob", StringCaseComparisonMode::NoCase));
    EXPECT_EQ(0, x.compare("bob", StringCaseComparisonMode::NoCase));
    EXPECT_GT(0, x.compare("CHARLES", StringCaseComparisonMode::NoCase)); // fuck Charles
}

TEST(StringBuf, CompareN)
{
    StringBuf x("testX");

    EXPECT_NE(0, x.compare("testY"));
    EXPECT_NE(0, x.compare(StringView("TESTY", 4)));
    EXPECT_EQ(0, x.view().leftPart(4).compare(StringView("testY", 4)));
}

TEST(StringBuf, CompareNoCaseN)
{
    StringBuf x("testX");

    EXPECT_NE(0, x.compare("testY", StringCaseComparisonMode::NoCase));
	EXPECT_EQ(0, x.view().leftPart(4).compare(StringView("TESTY", 4), StringCaseComparisonMode::NoCase));
	EXPECT_EQ(0, x.view().leftPart(4).compare(StringView("testY", 4), StringCaseComparisonMode::NoCase));
}

TEST(StringBuf, LeftPartOfEmptyStringIsAlwaysEmpty)
{
    StringBuf x;

    EXPECT_EQ("", x.view().leftPart(0));
    EXPECT_EQ("", x.view().leftPart(100));
}

TEST(StringBuf, LeftPartNotBiggerThanString)
{
    StringBuf x("test");

    EXPECT_EQ(StringBuf::EMPTY(), x.view().leftPart(0));
    EXPECT_EQ("t", x.view().leftPart(1));
    EXPECT_EQ("te", x.view().leftPart(2));
    EXPECT_EQ("tes", x.view().leftPart(3));
    EXPECT_EQ("test", x.view().leftPart(4));
    EXPECT_EQ("test", x.view().leftPart(5));
    EXPECT_EQ("test", x.view().leftPart(100));
}

TEST(StringBuf, RightPartOfEmptyStringIsAlwaysEmpty)
{
    StringBuf x;

    EXPECT_EQ(StringBuf::EMPTY(), x.view().rightPart(0));
    EXPECT_EQ(StringBuf::EMPTY(), x.view().rightPart(100));

}

TEST(StringBuf, RightPartNotBiggerThanString)
{
    StringBuf x("test");

    EXPECT_EQ(StringBuf::EMPTY(), x.view().rightPart(0));
    EXPECT_EQ("t", x.view().rightPart(1));
    EXPECT_EQ("st", x.view().rightPart(2));
    EXPECT_EQ("est", x.view().rightPart(3));
    EXPECT_EQ("test", x.view().rightPart(4));
    EXPECT_EQ("test", x.view().rightPart(5));
    EXPECT_EQ("test", x.view().rightPart(100));
}

TEST(StringBuf, EmptySubString)
{
    StringBuf x("test");
    EXPECT_EQ(StringBuf(), x.view().subString(0,0));
}

TEST(StringBuf, SubStringOutOfRangeIsEmpty)
{
    StringBuf x("test");
    EXPECT_EQ(StringBuf(), x.view().subString(100));
}

TEST(StringBuf, SubStringRangeIsClamped)
{
    StringBuf x("test");
    EXPECT_EQ(StringBuf("st"), x.view().subString(2,100));
}

TEST(StringBuf, SubStringGenericTest)
{
    StringBuf x("test");
    EXPECT_EQ("es", x.view().subString(1, 2));
    EXPECT_EQ("e", x.view().subString(1, 1));
    EXPECT_EQ("t", x.view().subString(0, 1));
    EXPECT_EQ("t", x.view().subString(3, 1));
    EXPECT_EQ("test", x.view().subString(0, 4));
    EXPECT_EQ("test", x.view().subString(0));
}

TEST(StringBuf, SplitMiddle)
{
    StringBuf x("test");

    StringView left, right;
    x.view().split(2, left, right);

    EXPECT_EQ("te", left);
    EXPECT_EQ("st", right);
}

TEST(StringBuf, SplitAtZero)
{
    StringBuf x("test");

    StringView left, right;
    x.view().split(0, left, right);

    EXPECT_EQ("", left);
    EXPECT_EQ("test", right);
}

TEST(StringBuf, SplitAtEnd)
{
    StringBuf x("test");

    StringView left, right;
    x.view().split(4, left, right);

    EXPECT_EQ("test", left);
    EXPECT_EQ("", right);
}

TEST(StringBuf, SplitOutOfRange)
{
    StringBuf x("test");

    StringView left, right;
    x.view().split(100, left, right);

    EXPECT_EQ("test", left);
    EXPECT_EQ("", right);
}

TEST(StringBuf, FindFirstChar)
{
    StringBuf x("HelloWorld");

    EXPECT_EQ(2, x.view().findFirstChar('l'));
    EXPECT_EQ(1, x.view().findFirstChar('e'));
    EXPECT_EQ(-1, x.view().findFirstChar('z'));
}

TEST(StringBuf, FindLastChar)
{
    StringBuf x("HelloWorld");

    EXPECT_EQ(8, x.view().findLastChar('l'));
    EXPECT_EQ(1, x.view().findLastChar('e'));
    EXPECT_EQ(-1, x.view().findLastChar('z'));
}

TEST(StringBuf, FindString)
{
    StringBuf x("HelloWorldhello");

    EXPECT_EQ(5, x.view().findStr("World"));
}

TEST(StringBuf, FindStringWithOffset)
{
    StringBuf x("HelloWorldhello");

    EXPECT_EQ(1, x.view().findStr("ello", StringCaseComparisonMode::WithCase, 0));
    EXPECT_EQ(11, x.view().findStr("ello", StringCaseComparisonMode::WithCase, 2));
}

TEST(StringBuf, FindStringRev)
{
    StringBuf x("HelloWorldhello");

    EXPECT_EQ(11, x.view().findRevStr("ello"));
}

TEST(StringBuf, FindStringRevWithOffset)
{
    StringBuf x("HelloWorldhello");

    EXPECT_EQ(1, x.view().findRevStr("ello", StringCaseComparisonMode::WithCase, 5));
}

TEST(StringBuf, BeginsWith)
{
    StringBuf x("HelloWorldhello");

    EXPECT_TRUE(x.beginsWith(""));
    EXPECT_TRUE(x.beginsWith("Hello"));
    EXPECT_FALSE(x.beginsWith("ello"));
    EXPECT_FALSE(x.beginsWith("hello"));
    EXPECT_TRUE(x.beginsWith("HelloWorldhello"));
    EXPECT_FALSE(x.beginsWith("HelloWorldhello2"));
}

TEST(StringBuf, BeginsWithNoCase)
{
    StringBuf x("HelloWorldhello");

    EXPECT_TRUE(x.beginsWith("", StringCaseComparisonMode::NoCase));
    EXPECT_TRUE(x.beginsWith("Hello", StringCaseComparisonMode::NoCase));
    EXPECT_FALSE(x.beginsWith("ello", StringCaseComparisonMode::NoCase));
    EXPECT_TRUE(x.beginsWith("hello", StringCaseComparisonMode::NoCase));
    EXPECT_TRUE(x.beginsWith("HELLOWORLDHELLO", StringCaseComparisonMode::NoCase));
    EXPECT_FALSE(x.beginsWith("HelloWorldhello2", StringCaseComparisonMode::NoCase));
}

TEST(StringBuf, EndsWith)
{
    StringBuf x("HelloWorldhello");

    EXPECT_TRUE(x.endsWith(""));
    EXPECT_TRUE(x.endsWith("hello"));
    EXPECT_TRUE(x.endsWith("ello"));
    EXPECT_FALSE(x.endsWith("ell"));
    EXPECT_FALSE(x.endsWith("Hello"));
    EXPECT_TRUE(x.endsWith("HelloWorldhello"));
    EXPECT_FALSE(x.endsWith("2HelloWorldhello"));
}

TEST(StringBuf, EndsWithNoCase)
{
    StringBuf x("HelloWorldhello");

    EXPECT_TRUE(x.endsWith("", StringCaseComparisonMode::NoCase));
    EXPECT_TRUE(x.endsWith("Hello", StringCaseComparisonMode::NoCase));
    EXPECT_TRUE(x.endsWith("ELLO", StringCaseComparisonMode::NoCase));
}

TEST(StringBuf, StringAfterFirst)
{
    StringBuf x("Ala_ma_kota_Ala_ma");
    EXPECT_EQ("_kota_Ala_ma", x.view().afterFirst("ma"));
    EXPECT_EQ("_Ala_ma", x.view().afterFirst("kota"));
    EXPECT_TRUE(x.view().afterFirst("dupa").empty());
    EXPECT_TRUE(x.view().afterFirst("MA").empty());
}

TEST(StringBuf, StringAfterFirstNoCase)
{
    StringBuf x("Ala_ma_kota_Ala_ma");
    EXPECT_EQ("_kota_Ala_ma", x.view().afterFirst("MA", bm::StringCaseComparisonMode::NoCase));
    EXPECT_EQ("_ma_kota_Ala_ma", x.view().afterFirst("ala", bm::StringCaseComparisonMode::NoCase));
}

TEST(StringBuf, StringBeforeFirst)
{
    StringBuf x("Ala_ma_kota_Ala_ma");
    EXPECT_EQ("Ala_", x.view().beforeFirst("ma"));
    EXPECT_TRUE(x.view().beforeFirst("MA").empty());
    EXPECT_EQ("Ala_ma_", x.view().beforeFirst("kota"));
}

TEST(StringBuf, StringBeforeFirstNoCase)
{
    StringBuf x("Ala_ma_kota_Ala_ma");
    EXPECT_EQ("Ala_", x.view().beforeFirst("MA", bm::StringCaseComparisonMode::NoCase));
}

TEST(StringBuf, StringAfterLast)
{
    StringBuf x("Ala_ma_kota_Ala_ma");
    EXPECT_TRUE(x.view().afterLast("ma").empty());
    EXPECT_EQ("_ma", x.view().afterLast("Ala"));
    EXPECT_TRUE(x.view().afterLast("dupa").empty());
}

TEST(StringBuf, StringAfterLastNoCase)
{
    StringBuf x("Ala_ma_kota_Ala_ma");
    EXPECT_EQ("_ma", x.view().afterLast("ala", bm::StringCaseComparisonMode::NoCase));
    EXPECT_TRUE(x.view().afterLast("dupa", bm::StringCaseComparisonMode::NoCase).empty());
}

TEST(StringBuf, StringBeforeLast)
{
    StringBuf x("Ala_ma_kota_Ala_ma");
	EXPECT_TRUE(x.view().beforeLast("ala").empty());
	EXPECT_TRUE(x.view().beforeLast("Ala") == "Ala_ma_kota_");
	EXPECT_TRUE(x.view().beforeLast("kota") == "Ala_ma_");
	EXPECT_TRUE(x.view().beforeLast("ma") == "Ala_ma_kota_Ala_");
}

TEST(StringBuf, StringBeforeLastNoCase)
{
    StringBuf x("Ala_ma_kota_Ala_ma");
    EXPECT_EQ("Ala_ma_kota_", x.view().beforeLast("ala", bm::StringCaseComparisonMode::NoCase));
    EXPECT_EQ("Ala_ma_",  x.view().beforeLast("KOTA", bm::StringCaseComparisonMode::NoCase));
}

TEST(StringBuf, ToUpperMakesUpperCase)
{
    StringBuf txt("ala ma kota");
    auto up = txt.toUpper();
    EXPECT_EQ("ALA MA KOTA", up);
}

TEST(StringBuf, ToUpperDoesNotChangeUpper)
{
	StringBuf txt("Ala 56 Kota");
	auto up = txt.toUpper();
	EXPECT_EQ("ALA 56 KOTA", up);
}

TEST(StringBuf, ToUpperCreatesCopyWhenNeeded)
{
	StringBuf txt("Ala Ma Kota");
	auto up = txt.toUpper();
    EXPECT_NE(txt.c_str(), up.c_str());
}

TEST(StringBuf, ToUpperCreatesDoesNotCreateCopyWhenNotNeeded)
{
	StringBuf txt("ALA MA KOTA");
	auto up = txt.toUpper();
	EXPECT_EQ(txt.c_str(), up.c_str());
}

TEST(StringBuf, ToLowerMakesLowerCase)
{
	StringBuf txt("ALA MA KOTA");
	auto up = txt.toLower();
	EXPECT_EQ("ala ma kota", up);
}

TEST(StringBuf, ToLowerDoesNotChangeLower)
{
	StringBuf txt("Ala 56 Kota");
	auto up = txt.toLower();
	EXPECT_EQ("ala 56 kota", up);
}

TEST(StringBuf, ToLowerCreatesCopyWhenNeeded)
{
	StringBuf txt("ALA MA KOTA");
	auto up = txt.toLower();
	EXPECT_NE(txt.c_str(), up.c_str());
}

TEST(StringBuf, ToLowerCreatesDoesNotCreateCopyWhenNotNeeded)
{
	StringBuf txt("ala ma kota");
	auto up = txt.toLower();
	EXPECT_EQ(txt.c_str(), up.c_str());
}

TEST(StringBuf, ToLowerDoesNotTouchEmpty)
{
    StringBuf txt;
	auto up = txt.toLower();
    EXPECT_TRUE(up.empty());
}

TEST(StringBuf, ToUpperNotTouchEmpty)
{
	StringBuf txt;
	auto up = txt.toLower();
	EXPECT_TRUE(up.empty());
}

TEST(StringBuf, TranscodeNotCalledOnEmptyString)
{
    uint32_t count = 0;
    StringBuf txt;
    txt.transcode([&count](uint32_t ch) { count += 1; return ch; });
    EXPECT_EQ(0, count);
}

TEST(StringBuf, TranscodeCalledForEveryChar)
{
	uint32_t count = 0;
	StringBuf txt("TEST");
	txt.transcode([&count](uint32_t ch) { count += 1; return ch; });
	EXPECT_EQ(4, count);
}

TEST(StringBuf, TranscodeCalledForEveryUnicodeChar)
{
	uint32_t count = 0;
    StringBuf txt(u8"gęś");
    EXPECT_EQ(5, txt.length());
    EXPECT_EQ(3, txt.unicodeLength());
	txt.transcode([&count](uint32_t ch) { count += 1; return ch; });
	EXPECT_EQ(3, count);
}

TEST(StringBuf, TranscodeCanChangeChars)
{
	StringBuf txt(u8"abc");
    txt = txt.transcode([](uint32_t ch) -> uint32_t {
        switch (ch)
        {
        case 'a': return '1';
        case 'b': return '2';
        case 'c': return '3';
        default: return ch;
        }});

    EXPECT_EQ("123", txt);
}

TEST(StringBuf, TranscodeCanRemoveChars)
{
	StringBuf txt(u8"abc");
    txt = txt.transcode([](uint32_t ch) -> uint32_t {
		switch (ch)
		{
		case 'a': return '1';
		case 'b': return 0;
		case 'c': return '3';
		default: return ch;
		}});

    EXPECT_EQ(2, txt.length());
	EXPECT_EQ("13", txt);
}

TEST(StringBuf, TranscodeCanClearString)
{
	StringBuf txt(u8"abc");
    txt = txt.transcode([](uint32_t ch) -> uint32_t { return 0; });

	EXPECT_EQ(0, txt.length());
    EXPECT_TRUE(txt.empty());
	EXPECT_EQ("", txt);
}

TEST(StringBuf, ReplaceCharReplacesChar)
{
	StringBuf txt("abcdef");
    auto up = txt.replaceChar('c', 'x');
    EXPECT_EQ("abxdef", up);
}

TEST(StringBuf, ReplaceCharReplacesMultipleChars)
{
	StringBuf txt("abcdcf");
	auto up = txt.replaceChar('c', 'x');
	EXPECT_EQ("abxdxf", up);
}

TEST(StringBuf, ReplaceDoesNotRecreateStringWhenNoReplacementDone)
{
	StringBuf txt("abcdef");
	auto up = txt.replaceChar('z', 'x');
	EXPECT_EQ(StringBuf("abcdef"), up);
    EXPECT_EQ(up.c_str(), txt.c_str());
}

TEST(StringBuf, ReplaceCharReplacesUnicodeChars)
{
	StringBuf txt(u8"gęś zółćią");
	auto up = txt.replaceChar(U'ó', U'u');
    auto same = StringBuf(u8"gęś zułćią") == up;
	EXPECT_TRUE(same);
}


END_INFERNO_NAMESPACE()
