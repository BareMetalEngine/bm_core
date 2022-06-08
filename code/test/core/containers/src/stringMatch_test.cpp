/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "bm/core/containers/include/inplaceArray.h"

DECLARE_TEST_FILE(StringFunctions);

//---

BEGIN_INFERNO_NAMESPACE()

TEST(CompileTimeCRC, EmptyTest)
{
    constexpr auto crc = prv::CompileTime_CalcCRC32("");
    EXPECT_EQ(0, crc);
}

TEST(CompileTimeCRC, CompileTimeCRCEqualsRuntimeCRC)
{
    constexpr auto crc = prv::CompileTime_CalcCRC32("Ala ma kota");

    CRC32 rt;
    rt << "Ala ma kota";
    auto crc2 = rt.crc();

    EXPECT_EQ(crc2, crc);
}

//---

TEST(MatchInteger, Match_InvalidChars)
{
    char val = 0;
    auto ret = StringView("100.00").match(val);
    EXPECT_EQ(MatchResultStatus::InvalidCharacter, ret.status);
    EXPECT_EQ(val, 0);
}

TEST(MatchInteger, Match_PlusConsumed)
{
    char val = 0;
    auto ret = StringView("+5").match(val);
    EXPECT_EQ(MatchResultStatus::OK, ret.status);
    EXPECT_EQ(val, 5);
}

TEST(MatchInteger, Match_DoublePlusNotConsumed)
{
    char val = 0;
    auto ret = StringView("++5").match(val);
    EXPECT_EQ(MatchResultStatus::InvalidCharacter, ret.status);
    EXPECT_EQ(val, 0);
}

TEST(MatchInteger, Match_DoubleMinusNotConsumed)
{
    char val = 0;
    auto ret = StringView("--5").match(val);
    EXPECT_EQ(MatchResultStatus::InvalidCharacter, ret.status);
    EXPECT_EQ(val, 0);
}

TEST(MatchInteger, Match_MinusNotConsumedForUnsigned)
{
    uint8_t val = 0;
    auto ret = StringView("-5").match(val);
    EXPECT_EQ(MatchResultStatus::InvalidCharacter, ret.status);
    EXPECT_EQ(val, 0);
}

//---

TEST(MatchInteger, MatchInt8_Pos)
{
    char val = 0;
    auto ret = StringView("42").match(val);
    EXPECT_EQ(MatchResultStatus::OK, ret.status);
    EXPECT_EQ(val, 42);
}

TEST(MatchInteger, MatchInt8_Neg)
{
    char val = 0;
    auto ret = StringView("-42").match(val);
    EXPECT_EQ(MatchResultStatus::OK, ret.status);
    EXPECT_EQ(val, -42);
}

TEST(MatchInteger, MatchInt8_Max)
{
    char val = 0;
    auto ret = StringView("127").match(val);
    EXPECT_EQ(MatchResultStatus::OK, ret.status);
    EXPECT_EQ(val, 127);
}

TEST(MatchInteger, MatchInt8_Min)
{
    char val = 0;
    auto ret = StringView("-128").match(val);
    EXPECT_EQ(MatchResultStatus::OK, ret.status);
    EXPECT_EQ(val, -128);
}

TEST(MatchInteger, MatchInt8_Overflow)
{
    char val = 0;
    auto ret = StringView("128").match(val);
    EXPECT_EQ(MatchResultStatus::Overflow, ret.status);
    EXPECT_EQ(val, 0);
}

TEST(MatchInteger, MatchInt8_OverflowMin)
{
    char val = 0;
    auto ret = StringView("-129").match(val);
    EXPECT_EQ(MatchResultStatus::Overflow, ret.status);
    EXPECT_EQ(val, 0);
}

TEST(MatchInteger, MatchInt8_OverflowLargeNubmerWithZeros)
{
    char val = 0;
    auto ret = StringView("10000").match(val);
    EXPECT_EQ(MatchResultStatus::Overflow, ret.status);
    EXPECT_EQ(val, 0);
}


TEST(MatchInteger, MatchInt8_LeadingZerosNoOverlow)
{
    char val = 0;
    auto ret = StringView("0000042").match(val);
    EXPECT_EQ(MatchResultStatus::OK, ret.status);
    EXPECT_EQ(val, 42);
}

//------

TEST(MatchInteger, MatchInt16_Pos)
{
    short val = 0;
    auto ret = StringView("42").match(val);
    EXPECT_EQ(MatchResultStatus::OK, ret.status);
    EXPECT_EQ(val, 42);
}

TEST(MatchInteger, MatchInt16_Neg)
{
    short val = 0;
    auto ret = StringView("-42").match(val);
    EXPECT_EQ(MatchResultStatus::OK, ret.status);
    EXPECT_EQ(val, -42);
}

TEST(MatchInteger, MatchInt16_Max)
{
    short val = 0;
    auto ret = StringView("32767").match(val);
    EXPECT_EQ(MatchResultStatus::OK, ret.status);
    EXPECT_EQ(val, 32767);
}

TEST(MatchInteger, MatchInt16_Min)
{
    short val = 0;
    auto ret = StringView("-32768").match(val);
    EXPECT_EQ(MatchResultStatus::OK, ret.status);
    EXPECT_EQ(val, -32768);
}

TEST(MatchInteger, MatchInt16_Overflow)
{
    short val = 0;
    auto ret = StringView("32768").match(val);
    EXPECT_EQ(MatchResultStatus::Overflow, ret.status);
    EXPECT_EQ(val, 0);
}

TEST(MatchInteger, MatchInt16_OverflowMin)
{
    short val = 0;
    auto ret = StringView("-32769").match(val);
    EXPECT_EQ(MatchResultStatus::Overflow, ret.status);
    EXPECT_EQ(val, 0);
}

TEST(MatchInteger, MatchInt16_OverflowLargeNubmerWithZeros)
{
    short val = 0;
    auto ret = StringView("100000000").match(val);
    EXPECT_EQ(MatchResultStatus::Overflow, ret.status);
    EXPECT_EQ(val, 0);
}


TEST(MatchInteger, MatchInt16_LeadingZerosNoOverlow)
{
    short val = 0;
    auto ret = StringView("00000235").match(val);
    EXPECT_EQ(MatchResultStatus::OK, ret.status);
    EXPECT_EQ(val, 235);
}

//------

TEST(MatchInteger, MatchInt32_Pos)
{
    int val = 0;
    auto ret = StringView("42").match(val);
    EXPECT_EQ(MatchResultStatus::OK, ret.status);
    EXPECT_EQ(val, 42);
}

TEST(MatchInteger, MatchInt32_Neg)
{
    int val = 0;
    auto ret = StringView("-42").match(val);
    EXPECT_EQ(MatchResultStatus::OK, ret.status);
    EXPECT_EQ(val, -42);
}

TEST(MatchInteger, MatchInt32_Max)
{
    int val = 0;
    auto ret = StringView("2147483647").match(val);
    EXPECT_EQ(MatchResultStatus::OK, ret.status);
    EXPECT_EQ(val, 2147483647);
}

TEST(MatchInteger, MatchInt32_Min)
{
    int val = 0;
    auto ret = StringView("-2147483648").match(val);
    EXPECT_EQ(MatchResultStatus::OK, ret.status);
    EXPECT_EQ(val, -(int64_t)2147483648);
}

TEST(MatchInteger, MatchInt32_Overflow)
{
    int val = 0;
    auto ret = StringView("2147483648").match(val);
    EXPECT_EQ(MatchResultStatus::Overflow, ret.status);
    EXPECT_EQ(val, 0);
}

TEST(MatchInteger, MatchInt32_OverflowMin)
{
    int val = 0;
    auto ret = StringView("-2147483649").match(val);
    EXPECT_EQ(MatchResultStatus::Overflow, ret.status);
    EXPECT_EQ(val, 0);
}

TEST(MatchInteger, MatchInt32_OverflowLargeNubmerWithZeros)
{
    int val = 0;
    auto ret = StringView("100000000000").match(val);
    EXPECT_EQ(MatchResultStatus::Overflow, ret.status);
    EXPECT_EQ(val, 0);
}


TEST(MatchInteger, MatchInt32_LeadingZerosNoOverlow)
{
    int val = 0;
    auto ret = StringView("0000234234").match(val);
    EXPECT_EQ(MatchResultStatus::OK, ret.status);
    EXPECT_EQ(val, 234234);
}

//------

TEST(MatchInteger, MatchInt64_Pos)
{
    int64_t val = 0;
    auto ret = StringView("42").match(val);
    EXPECT_EQ(MatchResultStatus::OK, ret.status);
    EXPECT_EQ(val, 42);
}

TEST(MatchInteger, MatchInt64_Neg)
{
    int64_t val = 0;
    auto ret = StringView("-42").match(val);
    EXPECT_EQ(MatchResultStatus::OK, ret.status);
    EXPECT_EQ(val, -42);
}

TEST(MatchInteger, MatchInt64_Max)
{
    int64_t val = 0;
    auto ret = StringView("9223372036854775807").match(val);
    EXPECT_EQ(MatchResultStatus::OK, ret.status);
    EXPECT_EQ(val, 9223372036854775807);
}

TEST(MatchInteger, MatchInt64_Min)
{
    int64_t val = 0;
    auto ret = StringView("-9223372036854775808").match(val);
    EXPECT_EQ(MatchResultStatus::OK, ret.status);
    EXPECT_EQ(val, std::numeric_limits<int64_t>::min());
}

TEST(MatchInteger, MatchInt64_Overflow)
{
    int64_t val = 0;
    auto ret = StringView("9223372036854775808").match(val);
    EXPECT_EQ(MatchResultStatus::Overflow, ret.status);
    EXPECT_EQ(val, 0);
}

TEST(MatchInteger, MatchInt64_OverflowMin)
{
    int64_t val = 0;
    auto ret = StringView("-9223372036854775809").match(val);
    EXPECT_EQ(MatchResultStatus::Overflow, ret.status);
    EXPECT_EQ(val, 0);
}

TEST(MatchInteger, MatchInt64_OverflowLargeNubmerWithZeros)
{
    int64_t val = 0;
    auto ret = StringView("1000000000000000000000").match(val);
    EXPECT_EQ(MatchResultStatus::Overflow, ret.status);
    EXPECT_EQ(val, 0);
}


TEST(MatchInteger, MatchInt64_LeadingZerosNoOverlow)
{
    int64_t val = 0;
    auto ret = StringView("0000234234").match(val);
    EXPECT_EQ(MatchResultStatus::OK, ret.status);
    EXPECT_EQ(val, 234234);
}

//------

TEST(MatchInteger, MatchFloat_Pos)
{
    double val = 0;
    auto ret = StringView("42").match(val);
    EXPECT_EQ(MatchResultStatus::OK, ret.status);
    EXPECT_EQ(val, 42.0);
}

TEST(MatchInteger, MatchFloat_Neg)
{
    double val = 0;
    auto ret = StringView("-42").match(val);
    EXPECT_EQ(MatchResultStatus::OK, ret.status);
    EXPECT_EQ(val, -42.0);
}

TEST(MatchInteger, MatchFloat_SimpleFrac)
{
    double val = 0;
    auto ret = StringView("3.14").match(val);
    EXPECT_EQ(MatchResultStatus::OK, ret.status);
    EXPECT_NEAR(val, 3.14, 0.001);
}

TEST(MatchInteger, MatchFloat_OnlyFrac)
{
    double val = 0;
    auto ret = StringView(".14").match(val);
    EXPECT_EQ(MatchResultStatus::OK, ret.status);
    EXPECT_NEAR(val, 0.14, 0.001);
}

TEST(MatchInteger, MatchFloat_InvalidChars)
{
    double val = 0;
    auto ret = StringView("124x").match(val);
    EXPECT_EQ(MatchResultStatus::InvalidCharacter, ret.status);
    EXPECT_EQ(val, 0);
}

//---

TEST(MatchString, EmptyStringMatch)
{
    StringBuf val;
	auto ret = StringView("").match(val);
	EXPECT_EQ(MatchResultStatus::OK, ret.status);
    EXPECT_STREQ("", val.c_str());
}

TEST(MatchString, TrimRemovesEmptySpaces)
{
	StringBuf val;
	auto ret = StringView("  ").match(val);
	EXPECT_EQ(MatchResultStatus::OK, ret.status);
	EXPECT_STREQ("", val.c_str());
}

TEST(MatchString, SimpleStringMatch)
{
	StringBuf val;
	auto ret = StringView("test").match(val);
	EXPECT_EQ(MatchResultStatus::OK, ret.status);
	EXPECT_STREQ("test", val.c_str());
}

TEST(MatchString, SimpleStringMatchFrontTrim)
{
	StringBuf val;
	auto ret = StringView("  test").match(val);
	EXPECT_EQ(MatchResultStatus::OK, ret.status);
	EXPECT_STREQ("test", val.c_str());
}

TEST(MatchString, SimpleStringMatchBackTrim)
{
	StringBuf val;
	auto ret = StringView("test  ").match(val);
	EXPECT_EQ(MatchResultStatus::OK, ret.status);
	EXPECT_STREQ("test", val.c_str());
}

TEST(MatchString, SimpleStringMatchBothTrim)
{
	StringBuf val;
	auto ret = StringView("  test  ").match(val);
	EXPECT_EQ(MatchResultStatus::OK, ret.status);
	EXPECT_STREQ("test", val.c_str());
}

TEST(MatchString, TrimKeepsInternalSpace)
{
	StringBuf val;
	auto ret = StringView("  ala ma kota  ").match(val);
	EXPECT_EQ(MatchResultStatus::OK, ret.status);
	EXPECT_STREQ("ala ma kota", val.c_str());
}

TEST(MatchString, EmptyQuotedString)
{
	StringBuf val;
	auto ret = StringView("\"\"").match(val);
	EXPECT_EQ(MatchResultStatus::OK, ret.status);
	EXPECT_STREQ("", val.c_str());
}

TEST(MatchString, EmptyQuotedString2)
{
	StringBuf val;
	auto ret = StringView("''").match(val);
	EXPECT_EQ(MatchResultStatus::OK, ret.status);
	EXPECT_STREQ("", val.c_str());
}

TEST(MatchString, EmptyQuotedStringWithTrim)
{
	StringBuf val;
	auto ret = StringView("  \"\"  ").match(val);
	EXPECT_EQ(MatchResultStatus::OK, ret.status);
	EXPECT_STREQ("", val.c_str());
}

TEST(MatchString, EmptyQuotedStringWithTrim2)
{
	StringBuf val;
	auto ret = StringView("  ''  ").match(val);
	EXPECT_EQ(MatchResultStatus::OK, ret.status);
	EXPECT_STREQ("", val.c_str());
}

TEST(MatchString, SimpleQuotedString)
{
	StringBuf val;
	auto ret = StringView(" \"test\" ").match(val);
	EXPECT_EQ(MatchResultStatus::OK, ret.status);
	EXPECT_STREQ("test", val.c_str());
}

TEST(MatchString, SimpleQuotedString2)
{
	StringBuf val;
	auto ret = StringView(" 'test' ").match(val);
	EXPECT_EQ(MatchResultStatus::OK, ret.status);
	EXPECT_STREQ("test", val.c_str());
}

TEST(MatchString, QuotedStringKeepsInternalSpaces)
{
	StringBuf val;
	auto ret = StringView(" \" test \" ").match(val);
	EXPECT_EQ(MatchResultStatus::OK, ret.status);
	EXPECT_STREQ(" test ", val.c_str());
}

TEST(MatchString, QuotedStringKeepsInternalSpaces2)
{
	StringBuf val;
	auto ret = StringView(" ' test ' ").match(val);
	EXPECT_EQ(MatchResultStatus::OK, ret.status);
	EXPECT_STREQ(" test ", val.c_str());
}

//---

TEST(MatchStringID, EmptyStringMatch)
{
	StringID val;
	auto ret = StringView("").match(val);
	EXPECT_EQ(MatchResultStatus::OK, ret.status);
	EXPECT_STREQ("", val.c_str());
}

TEST(MatchStringID, TrimRemovesEmptySpaces)
{
	StringID val;
	auto ret = StringView("  ").match(val);
	EXPECT_EQ(MatchResultStatus::OK, ret.status);
	EXPECT_STREQ("", val.c_str());
}

TEST(MatchStringID, SimpleStringMatch)
{
	StringID val;
	auto ret = StringView("test").match(val);
	EXPECT_EQ(MatchResultStatus::OK, ret.status);
	EXPECT_STREQ("test", val.c_str());
}

TEST(MatchStringID, SimpleStringMatchFrontTrim)
{
	StringID val;
	auto ret = StringView("  test").match(val);
	EXPECT_EQ(MatchResultStatus::OK, ret.status);
	EXPECT_STREQ("test", val.c_str());
}

TEST(MatchStringID, SimpleStringMatchBackTrim)
{
	StringID val;
	auto ret = StringView("test  ").match(val);
	EXPECT_EQ(MatchResultStatus::OK, ret.status);
	EXPECT_STREQ("test", val.c_str());
}

TEST(MatchStringID, SimpleStringMatchBothTrim)
{
	StringID val;
	auto ret = StringView("  test  ").match(val);
	EXPECT_EQ(MatchResultStatus::OK, ret.status);
	EXPECT_STREQ("test", val.c_str());
}

TEST(MatchStringID, TrimKeepsInternalSpace)
{
	StringID val;
	auto ret = StringView("  ala ma kota  ").match(val);
	EXPECT_EQ(MatchResultStatus::OK, ret.status);
	EXPECT_STREQ("ala ma kota", val.c_str());
}

TEST(MatchStringID, EmptyQuotedString)
{
	StringID val;
	auto ret = StringView("\"\"").match(val);
	EXPECT_EQ(MatchResultStatus::OK, ret.status);
	EXPECT_STREQ("", val.c_str());
}

TEST(MatchStringID, EmptyQuotedString2)
{
	StringID val;
	auto ret = StringView("''").match(val);
	EXPECT_EQ(MatchResultStatus::OK, ret.status);
	EXPECT_STREQ("", val.c_str());
}

TEST(MatchStringID, EmptyQuotedStringWithTrim)
{
	StringID val;
	auto ret = StringView("  \"\"  ").match(val);
	EXPECT_EQ(MatchResultStatus::OK, ret.status);
	EXPECT_STREQ("", val.c_str());
}

TEST(MatchStringID, EmptyQuotedStringWithTrim2)
{
	StringID val;
	auto ret = StringView("  ''  ").match(val);
	EXPECT_EQ(MatchResultStatus::OK, ret.status);
	EXPECT_STREQ("", val.c_str());
}

TEST(MatchStringID, SimpleQuotedString)
{
	StringID val;
	auto ret = StringView(" \"test\" ").match(val);
	EXPECT_EQ(MatchResultStatus::OK, ret.status);
	EXPECT_STREQ("test", val.c_str());
}

TEST(MatchStringID, SimpleQuotedString2)
{
	StringID val;
	auto ret = StringView(" 'test' ").match(val);
	EXPECT_EQ(MatchResultStatus::OK, ret.status);
	EXPECT_STREQ("test", val.c_str());
}

TEST(MatchStringID, QuotedStringKeepsInternalSpaces)
{
	StringID val;
	auto ret = StringView(" \" test \" ").match(val);
	EXPECT_EQ(MatchResultStatus::OK, ret.status);
	EXPECT_STREQ(" test ", val.c_str());
}

TEST(MatchStringID, QuotedStringKeepsInternalSpaces2)
{
	StringID val;
	auto ret = StringView(" ' test ' ").match(val);
	EXPECT_EQ(MatchResultStatus::OK, ret.status);
	EXPECT_STREQ(" test ", val.c_str());
}

//---

TEST(MatchIntArray, EmptyValue)
{
	InplaceArray<int, 10> ar;
    EXPECT_EQ(0, StringView("").sliceAndMatch(';', ar));
	ASSERT_EQ(0, ar.size());
}

TEST(MatchIntArray, SingleValue)
{
	InplaceArray<int, 10> ar;
    EXPECT_EQ(0, StringView("42").sliceAndMatch(';', ar));
	ASSERT_EQ(1, ar.size());
    EXPECT_EQ(42, ar[0]);
}

TEST(MatchIntArray, MultiValues)
{
    InplaceArray<int, 10> ar;
    EXPECT_EQ(0, StringView("1;-2;3").sliceAndMatch(';', ar));
    ASSERT_EQ(3, ar.size());
    EXPECT_EQ(1, ar[0]);
    EXPECT_EQ(-2, ar[1]);
    EXPECT_EQ(3, ar[2]);
}

TEST(MatchIntArray, InvalidValuesNotParsed)
{
	InplaceArray<uint8_t, 10> ar;
    EXPECT_EQ(1, StringView("1;-2;3").sliceAndMatch(';', ar));
	ASSERT_EQ(2, ar.size());
	EXPECT_EQ(1, ar[0]);
	EXPECT_EQ(3, ar[1]);
}

TEST(MatchIntArray, MultipleInvalidValuesReported)
{
	InplaceArray<uint8_t, 10> ar;
	EXPECT_EQ(3, StringView("-1;-2;-3").sliceAndMatch(';', ar));
	ASSERT_EQ(0, ar.size());
}

TEST(MatchIntArray, EmptyValuesNotParsed)
{
	InplaceArray<uint8_t, 10> ar;
    EXPECT_EQ(0, StringView(";1;;3;").sliceAndMatch(';', ar));
	ASSERT_EQ(2, ar.size());
	EXPECT_EQ(1, ar[0]);
	EXPECT_EQ(3, ar[1]);
}

//---

TEST(MatchStringArray, MultipleValues)
{
	InplaceArray<StringBuf, 10> ar;
	EXPECT_EQ(0, StringView("Ala;ma;kota").sliceAndMatch(';', ar));
	ASSERT_EQ(3, ar.size());
	EXPECT_STREQ("Ala", ar[0].c_str());
    EXPECT_STREQ("ma", ar[1].c_str());
    EXPECT_STREQ("kota", ar[2].c_str());	
}

TEST(MatchStringArray, MultipleValuesInQuotes)
{
	InplaceArray<StringBuf, 10> ar;
    EXPECT_EQ(0, StringView("\"Ala ma kota\";\"kot ma ale\"").sliceAndMatch(';', ar));
	ASSERT_EQ(2, ar.size());
	EXPECT_STREQ("Ala ma kota", ar[0].c_str());
	EXPECT_STREQ("kot ma ale", ar[1].c_str());
}

//---

TEST(WildcardMatch, DirectMatch)
{
    EXPECT_TRUE(StringView("geeks").matchPattern("geeks"));
}

TEST(WildcardMatch, EmptyDirectMatch)
{
	EXPECT_TRUE(StringView("").matchPattern(""));
}

TEST(WildcardMatch, EmptyMatchesAny)
{
	EXPECT_TRUE(StringView("").matchPattern("*"));
}

TEST(WildcardMatch, EmptyDoesNotMatchSingle)
{
    EXPECT_FALSE(StringView("").matchPattern("?"));
}

TEST(WildcardMatch, InternalAnySingle)
{
    EXPECT_TRUE(StringView("geks").matchPattern("g*ks"));
}

TEST(WildcardMatch, InternalAnyMultiple)
{
	EXPECT_TRUE(StringView("geeks").matchPattern("g*ks"));
}

TEST(WildcardMatch, InternalAnyMultiplePrefix)
{
	EXPECT_TRUE(StringView("geeks").matchPattern("ge*ks"));
}

TEST(WildcardMatch, InternalAnyMultiplePrefixWorksIfNoLeft)
{
	EXPECT_TRUE(StringView("geeks").matchPattern("gee*ks"));
}

TEST(WildcardMatch, InternalSpecificFailsIfNothingLeft)
{
	EXPECT_FALSE(StringView("geeks").matchPattern("gee?ks"));
}

TEST(WildcardMatch, Mixed1)
{
    EXPECT_TRUE(StringView("geeksforgeeks").matchPattern("ge?ks*"));
}

TEST(WildcardMatch, Mixed2)
{
    EXPECT_FALSE(StringView("gee").matchPattern("g*k"));
}
    
TEST(WildcardMatch, EmptyAnyPrefix)
{
    EXPECT_FALSE(StringView("pqrst").matchPattern("*pqrs"));
    EXPECT_TRUE(StringView("pqrst").matchPattern("*pqrst"));
}

TEST(WildcardMatch, EmptySpecificPrefix)
{
    EXPECT_FALSE(StringView("pqrst").matchPattern("?pqrs"));
    EXPECT_FALSE(StringView("pqrst").matchPattern("?pqrst"));
}

TEST(WildcardMatch, ExactCharCount)
{
	EXPECT_FALSE(StringView("pqrst").matchPattern("????"));
    EXPECT_TRUE(StringView("pqrst").matchPattern("?????"));
	EXPECT_FALSE(StringView("pqrst").matchPattern("??????"));
}

TEST(WildcardMatch, InfixAny)
{
    EXPECT_TRUE(StringView("abcdhghgbcd").matchPattern("abc*bcd"));
}

TEST(WildcardMatch, Unskippable)
{
    EXPECT_FALSE(StringView("abc*c?d").matchPattern("abcd"));
}

TEST(WildcardMatch, Checker)
{
    EXPECT_TRUE(StringView("abcd").matchPattern("*c*d"));
    EXPECT_TRUE(StringView("abcd").matchPattern("*?c*d"));
}

//---

END_INFERNO_NAMESPACE()