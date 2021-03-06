/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "bm/core/containers/include/stringParser.h"
#include "bm/core/containers/include/stringBuilder.h"
#include "bm/core/containers/include/stringID.h"

DECLARE_TEST_FILE(StringParser);

namespace testing
{
    template<>
    INLINE ::std::string PrintToString<bm::StringBuf>(const bm::StringBuf& value)
    {
        return value.c_str();
    }

    template<>
    INLINE ::std::string PrintToString<bm::StringView>(const bm::StringView& value)
    {
        return std::string(value.data(), value.length());
    }

    template<>
    INLINE ::std::string PrintToString<bm::StringID>(const bm::StringID& value)
    {
        return std::string(value.c_str());
    }

} // testing

BEGIN_INFERNO_NAMESPACE()

struct Result
{
    StringView view;
    float f;
    double d;
    uint64_t u64;
    uint32_t u32;
    uint16_t u16;
    uint8_t u8;
    int64_t i64;
    int i32;
    short i16;
    char i8;
    bool b;

    inline Result()
    {
        memzero(this, sizeof(Result));
    }
};

TEST(StringParser, NothingParsesFromEmtpyParser)
{
    StringParser p;
    Result r;

    EXPECT_FALSE(p.parseString(r.view));
    EXPECT_FALSE(p.parseIdentifier(r.view));
    EXPECT_FALSE(p.parseIdentifier(r.view));
    EXPECT_FALSE(p.parseKeyword("test"));
    EXPECT_FALSE(p.parseFloat(r.f));
    EXPECT_FALSE(p.parseDouble(r.d));
    EXPECT_FALSE(p.parseUint64(r.u64));
    EXPECT_FALSE(p.parseUint32(r.u32));
    EXPECT_FALSE(p.parseUint16(r.u16));
    EXPECT_FALSE(p.parseUint8(r.u8));
    EXPECT_FALSE(p.parseInt8(r.i8));
    EXPECT_FALSE(p.parseInt16(r.i16));
    EXPECT_FALSE(p.parseInt32(r.i32));
    EXPECT_FALSE(p.parseInt64(r.i64));
    EXPECT_FALSE(p.parseBoolean(r.b));
}

TEST(StringParser, ParseStringSingle)
{
    StringParser p("Ala ma kota");
    Result r;

    EXPECT_TRUE(p.parseString(r.view));
    EXPECT_EQ(StringView("Ala"), r.view);
    EXPECT_TRUE(p.parseString(r.view));
    EXPECT_EQ(StringView("ma"), r.view);
    EXPECT_TRUE(p.parseString(r.view));
    EXPECT_EQ(StringView("kota"), r.view);
    EXPECT_FALSE(p.parseString(r.view));
}

TEST(StringParser, ParseStringWithSpecialChars)
{
    StringParser p("Ala/ma/kota.png");
    Result r;

    EXPECT_TRUE(p.parseString(r.view));
    EXPECT_EQ(StringView("Ala/ma/kota.png"), r.view);
    EXPECT_FALSE(p.parseString(r.view));
}

TEST(StringParser, ParseStringQuotes)
{
    StringParser p(" \"Ala ma kota\" ");
    Result r;

    EXPECT_TRUE(p.parseString(r.view));
    EXPECT_EQ(StringView("Ala ma kota"), r.view);
    EXPECT_FALSE(p.parseString(r.view));
}

TEST(StringParser, ParseStringStopsOnQuotes)
{
    StringParser p(" Ala\"ma\"kota ");
    Result r;

    EXPECT_TRUE(p.parseString(r.view));
    EXPECT_EQ(StringView("Ala"), r.view);
    EXPECT_TRUE(p.parseString(r.view));
    EXPECT_EQ(StringView("ma"), r.view);
    EXPECT_TRUE(p.parseString(r.view));
    EXPECT_EQ(StringView("kota"), r.view);
    EXPECT_FALSE(p.parseString(r.view));
}

TEST(StringParser, ParseStringNoDelims)
{
    StringParser p("Ala;ma;kota");
    Result r;

    EXPECT_TRUE(p.parseString(r.view));
    EXPECT_EQ(StringView("Ala;ma;kota"), r.view);
    EXPECT_FALSE(p.parseString(r.view));
}

TEST(StringParser, ParseStringWithDelims)
{
    StringParser p("Ala;ma;kota");
    Result r;

    EXPECT_TRUE(p.parseString(r.view, ";"));
    EXPECT_EQ(StringView("Ala"), r.view);
    EXPECT_TRUE(p.parseKeyword(";"));

    EXPECT_TRUE(p.parseString(r.view, ";"));
    EXPECT_EQ(StringView("ma"), r.view);
    EXPECT_TRUE(p.parseKeyword(";"));

    EXPECT_TRUE(p.parseString(r.view, ";"));
    EXPECT_EQ(StringView("kota"), r.view);

    EXPECT_FALSE(p.parseKeyword(";"));
    EXPECT_FALSE(p.parseString(r.view, ";"));
}

TEST(StringParser, ParseStringWithEmptyDelims)
{
    StringParser p("Ala;ma;;kota;");
    Result r;

    EXPECT_TRUE(p.parseString(r.view, ";"));
    EXPECT_EQ(StringView("Ala"), r.view);
    EXPECT_TRUE(p.parseKeyword(";"));

    EXPECT_TRUE(p.parseString(r.view, ";"));
    EXPECT_EQ(StringView("ma"), r.view);
    EXPECT_TRUE(p.parseKeyword(";"));

    EXPECT_TRUE(p.parseString(r.view, ";"));
    EXPECT_EQ(StringView(), r.view);
    EXPECT_TRUE(p.parseKeyword(";"));

    EXPECT_TRUE(p.parseString(r.view, ";"));
    EXPECT_EQ(StringView("kota"), r.view);
    EXPECT_TRUE(p.parseKeyword(";"));

    EXPECT_FALSE(p.parseString(r.view, ";"));
}

TEST(StringParser, ParseKeyword)
{
    StringParser p("test");
    Result r;

    EXPECT_TRUE(p.parseKeyword("test"));
    EXPECT_FALSE(p.parseKeyword("test"));
}

TEST(StringParser, ParseKeywordTouching)
{
    StringParser p("testtest");
    Result r;

    EXPECT_TRUE(p.parseKeyword("test"));
    EXPECT_TRUE(p.parseKeyword("test"));
    EXPECT_FALSE(p.parseKeyword("test"));
}

TEST(StringParser, ParseKeywordChars)
{
    StringParser p("{+.+}");
    Result r;

    EXPECT_TRUE(p.parseKeyword("{"));
    EXPECT_TRUE(p.parseKeyword("+"));
    EXPECT_TRUE(p.parseKeyword("."));
    EXPECT_TRUE(p.parseKeyword("+"));
    EXPECT_TRUE(p.parseKeyword("}"));
}

TEST(StringParser, ParseInt64Valid)
{
    StringParser p(" 100");
    Result r;

    EXPECT_TRUE(p.parseInt64(r.i64));
    EXPECT_EQ(100, r.i64);
}

TEST(StringParser, ParseInt64WithSign)
{
    StringParser p(" -100");
    Result r;

    EXPECT_TRUE(p.parseInt64(r.i64));
    EXPECT_EQ(-100, r.i64);
}

TEST(StringParser, ParseInt64WithDot)
{
    StringParser p(" -100.5");
    Result r;

    EXPECT_TRUE(p.parseInt64(r.i64));
    EXPECT_EQ(-100, r.i64);
    EXPECT_TRUE(p.parseKeyword("."));
    EXPECT_TRUE(p.parseInt64(r.i64));
    EXPECT_EQ(5, r.i64);
}

TEST(StringParser, ParseInt64StringAdj)
{
    StringParser p(" 100a");
    Result r;

    EXPECT_TRUE(p.parseInt64(r.i64));
    EXPECT_EQ(100, r.i64);
    EXPECT_TRUE(p.parseString(r.view));
    EXPECT_EQ(StringView("a"), r.view);
}

TEST(StringParser, ParseInt8Range)
{
    Result r;

    EXPECT_TRUE(StringParser("100").parseInt8(r.i8));
    EXPECT_EQ(100, r.i8);
    EXPECT_FALSE(StringParser("200").parseInt8(r.i8));
    EXPECT_EQ(100, r.i8);
    EXPECT_TRUE(StringParser("-100").parseInt8(r.i8));
    EXPECT_EQ(-100, r.i8);
    EXPECT_FALSE(StringParser("-200").parseInt8(r.i8));
    EXPECT_EQ(-100, r.i8);

    EXPECT_TRUE(StringParser("127").parseInt8(r.i8));
    EXPECT_EQ(127, r.i8);
    EXPECT_FALSE(StringParser("128").parseInt8(r.i8));
    EXPECT_TRUE(StringParser("-128").parseInt8(r.i8));
    EXPECT_EQ(-128, r.i8);
    EXPECT_FALSE(StringParser("-129").parseInt8(r.i8));
}

TEST(StringParser, ParseInt16Range)
{
    Result r;

    EXPECT_TRUE(StringParser("10000").parseInt16(r.i16));
    EXPECT_EQ(10000, r.i16);
    EXPECT_FALSE(StringParser("40000").parseInt16(r.i16));
    EXPECT_EQ(10000, r.i16);
    EXPECT_TRUE(StringParser("-10000").parseInt16(r.i16));
    EXPECT_EQ(-10000, r.i16);
    EXPECT_FALSE(StringParser("-40000").parseInt16(r.i16));
    EXPECT_EQ(-10000, r.i16);

    EXPECT_TRUE(StringParser("32767").parseInt16(r.i16));
    EXPECT_EQ(32767, r.i16);
    EXPECT_FALSE(StringParser("32768").parseInt16(r.i16));
    EXPECT_TRUE(StringParser("-32768").parseInt16(r.i16));
    EXPECT_EQ(-32768, r.i16);
    EXPECT_FALSE(StringParser("-32769").parseInt16(r.i16));
}

TEST(StringParser, ParseInt32Range)
{
    Result r;

    EXPECT_TRUE(StringParser("1000000000").parseInt32(r.i32));
    EXPECT_EQ(1000000000, r.i32);
    EXPECT_FALSE(StringParser("4000000000").parseInt32(r.i32));
    EXPECT_EQ(1000000000, r.i32);
    EXPECT_TRUE(StringParser("-1000000000").parseInt32(r.i32));
    EXPECT_EQ(-1000000000, r.i32);
    EXPECT_FALSE(StringParser("-4000000000").parseInt32(r.i32));
    EXPECT_EQ(-1000000000, r.i32);

    EXPECT_TRUE(StringParser("2147483647").parseInt32(r.i32));
    EXPECT_EQ(2147483647, r.i32);
    EXPECT_FALSE(StringParser("2147483648").parseInt32(r.i32));
    EXPECT_TRUE(StringParser("-2147483648").parseInt32(r.i32));
    EXPECT_EQ(-2147483648LL, r.i32);
    EXPECT_FALSE(StringParser("-2147483649").parseInt32(r.i32));
}

TEST(StringParser, ParseInt64Range)
{
    Result r;

    // - 9,223,372,036,854,775,808 to 9,223,372,036,854,775,807
    //   9223372036854775807

    EXPECT_TRUE(StringParser("9000000000000000000").parseInt64(r.i64));
    EXPECT_EQ(9000000000000000000ll, r.i64);
    EXPECT_FALSE(StringParser("10000000000000000000").parseInt64(r.i64));
    EXPECT_EQ(9000000000000000000ll, r.i64);
    EXPECT_TRUE(StringParser("-9000000000000000000").parseInt64(r.i64));
    EXPECT_EQ(-9000000000000000000ll, r.i64);
    EXPECT_FALSE(StringParser("-10000000000000000000").parseInt64(r.i64));
    EXPECT_EQ(-9000000000000000000ll, r.i64);

    EXPECT_TRUE(StringParser("9223372036854775807").parseInt64(r.i64));
    EXPECT_EQ(9223372036854775807ll, r.i64);
    EXPECT_FALSE(StringParser("9223372036854775808").parseInt64(r.i64));
    EXPECT_TRUE(StringParser("-9223372036854775807").parseInt64(r.i64));
    EXPECT_EQ(-9223372036854775807ll, r.i64);
    EXPECT_FALSE(StringParser("-9223372036854775809").parseInt64(r.i64));
}

TEST(StringParser, ParseUint8Range)
{
    Result r;

    EXPECT_TRUE(StringParser("200").parseUint8(r.u8));
    EXPECT_EQ(200, r.u8);
    EXPECT_FALSE(StringParser("300").parseUint8(r.u8));
    EXPECT_EQ(200, r.u8);

    EXPECT_TRUE(StringParser("255").parseUint8(r.u8));
    EXPECT_EQ(255, r.u8);
    EXPECT_FALSE(StringParser("256").parseUint8(r.u8));
}

TEST(StringParser, ParseUint16Range)
{
    Result r;

    EXPECT_TRUE(StringParser("50000").parseUint16(r.u16));
    EXPECT_EQ(50000, r.u16);
    EXPECT_FALSE(StringParser("70000").parseUint16(r.u16));
    EXPECT_EQ(50000, r.u16);

    EXPECT_TRUE(StringParser("65535").parseUint16(r.u16));
    EXPECT_EQ(65535, r.u16);
    EXPECT_FALSE(StringParser("65536").parseUint16(r.u16));
}

TEST(StringParser, ParseUint32Range)
{
    Result r;

    EXPECT_TRUE(StringParser("3000000000").parseUint32(r.u32));
    EXPECT_EQ(3000000000UL, r.u32);
    EXPECT_FALSE(StringParser("5000000000").parseUint32(r.u32));
    EXPECT_EQ(3000000000UL, r.u32);

    EXPECT_TRUE(StringParser("4294967295").parseUint32(r.u32));
    EXPECT_EQ(4294967295UL, r.u32);
    EXPECT_FALSE(StringParser("4294967296").parseUint32(r.u32));
}

TEST(StringParser, ParseUint64Range)
{
    Result r;

    // 18,446,744,073,709,551,615
    // 18446744073709551615

    EXPECT_TRUE(StringParser("18000000000000000000").parseUint64(r.u64));
    EXPECT_EQ(18000000000000000000ULL, r.u64);
    EXPECT_FALSE(StringParser("19000000000000000000").parseUint64(r.u64));
    EXPECT_EQ(18000000000000000000ULL, r.u64);

    EXPECT_TRUE(StringParser("18446744073709551615").parseUint64(r.u64));
    EXPECT_EQ(18446744073709551615ULL, r.u64);
    EXPECT_FALSE(StringParser("18446744073709551616").parseUint64(r.u64));
}

TEST(StringParser, ParseZerosDontAffectOverflow)
{
    Result r;

    EXPECT_TRUE(StringParser("000000000000000000000018446744073709551615").parseUint64(r.u64));
    EXPECT_EQ(18446744073709551615ULL, r.u64);
    EXPECT_FALSE(StringParser("000000000000000000000018446744073709551616").parseUint64(r.u64));
}

TEST(StringParser, ParseWholeNumberIsConsumedEvenInOverflow)
{
    StringParser p(" 3000000000a");
    Result r;

    EXPECT_FALSE(p.parseInt8(r.i8));
    EXPECT_FALSE(p.parseKeyword("a"));
    EXPECT_FALSE(p.parseInt16(r.i16));
    EXPECT_FALSE(p.parseKeyword("a"));
    EXPECT_FALSE(p.parseInt32(r.i32));
    EXPECT_FALSE(p.parseKeyword("a"));
    EXPECT_TRUE(p.parseInt64(r.i64));
    EXPECT_EQ(3000000000L, r.i64);
}

TEST(StringParser, ParseBoolean)
{
    Result r;

    EXPECT_TRUE(StringParser("true").parseBoolean(r.b));
    EXPECT_TRUE(r.b);
    EXPECT_TRUE(StringParser("false").parseBoolean(r.b));
    EXPECT_FALSE(r.b);
    EXPECT_TRUE(StringParser("0").parseBoolean(r.b));
    EXPECT_FALSE(r.b);
    EXPECT_TRUE(StringParser("-1").parseBoolean(r.b));
    EXPECT_TRUE(r.b);
    EXPECT_TRUE(StringParser("1").parseBoolean(r.b));
    EXPECT_TRUE(r.b);
    EXPECT_FALSE(StringParser("\"test\"").parseBoolean(r.b));
    EXPECT_FALSE(StringParser(".").parseBoolean(r.b));
}

TEST(StringParser, ParseFloat)
{
    Result r;

    EXPECT_TRUE(StringParser("-3").parseFloat(r.f));
    EXPECT_EQ(-3.0f, r.f);
    EXPECT_TRUE(StringParser("-3.14").parseFloat(r.f));
    EXPECT_EQ(-3.14f, r.f);
    EXPECT_TRUE(StringParser("3").parseFloat(r.f));
    EXPECT_EQ(3.0f, r.f);
    EXPECT_TRUE(StringParser("3.14").parseFloat(r.f));
    EXPECT_EQ(3.14f, r.f);
    EXPECT_TRUE(StringParser("+3").parseFloat(r.f));
    EXPECT_EQ(3.0f, r.f);
    EXPECT_TRUE(StringParser("+3.14").parseFloat(r.f));
    EXPECT_EQ(3.14f, r.f);
    EXPECT_TRUE(StringParser(".1").parseFloat(r.f));
    EXPECT_EQ(0.1f, r.f);
    EXPECT_TRUE(StringParser("-.1").parseFloat(r.f));
    EXPECT_EQ(-0.1f, r.f);
}

TEST(StringParser, ParseFloatPrecission)
{
    Result r;

    EXPECT_TRUE(StringParser("1.012345678901234567890123456789").parseFloat(r.f));
    EXPECT_EQ(1.012345678901234567890123456789f, r.f);
}

TEST(StringParser, ParseVector)
{
    Result r;
    StringParser p("[-1.024,.23432,2]");

    EXPECT_TRUE(p.parseKeyword("["));
    EXPECT_TRUE(p.parseFloat(r.f));
    EXPECT_EQ(-1.024f, r.f);
    EXPECT_TRUE(p.parseKeyword(","));
    EXPECT_TRUE(p.parseFloat(r.f));
    EXPECT_EQ(0.23432f, r.f);
    EXPECT_TRUE(p.parseKeyword(","));
    EXPECT_TRUE(p.parseFloat(r.f));
    EXPECT_EQ(2.0f, r.f);
    EXPECT_TRUE(p.parseKeyword("]"));
}

END_INFERNO_NAMESPACE()