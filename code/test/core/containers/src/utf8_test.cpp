/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "bm/core/containers/include/stringView.h"
#include "bm/core/containers/include/inplaceArray.h"
#include "bm/core/containers/include/utf8StringFunctions.h"

BEGIN_INFERNO_NAMESPACE()

//--

TEST(UTF8, EncodeCharOneByte)
{
	char txt[10];
	auto len = utf8::ConvertChar(txt, 32);
	EXPECT_EQ(1, len);
	EXPECT_EQ(32, (uint8_t)txt[0]);
}

TEST(UTF8, EncodeCharOneByteMax)
{
	char txt[10];
	auto len = utf8::ConvertChar(txt, 0x7F);
	EXPECT_EQ(1, len);
	EXPECT_EQ(0x7F, (uint8_t)txt[0]);
}

TEST(UTF8, EncodeCharTwoBytesMin)
{
	char txt[10];
	auto len = utf8::ConvertChar(txt, 0x80);
	EXPECT_EQ(2, len);
	EXPECT_EQ(0xC2, (uint8_t)txt[0]);
	EXPECT_EQ(0x80, (uint8_t)txt[1]);
}

TEST(UTF8, EncodeCharTwoBytesMax)
{
	char txt[10];
	auto len = utf8::ConvertChar(txt, 0x7FF);
	EXPECT_EQ(2, len);
	EXPECT_EQ(0xDF, (uint8_t)txt[0]);
	EXPECT_EQ(0xBF, (uint8_t)txt[1]);
}

TEST(UTF8, EncodeCharThreeBytesMin)
{
	char txt[10];
	auto len = utf8::ConvertChar(txt, 0x800);
	EXPECT_EQ(3, len);
	EXPECT_EQ(0xE0, (uint8_t)txt[0]);
	EXPECT_EQ(0xA0, (uint8_t)txt[1]);
	EXPECT_EQ(0x80, (uint8_t)txt[2]);
}

TEST(UTF8, EncodeCharThreeBytesMax)
{
	char txt[10];
	auto len = utf8::ConvertChar(txt, 0xFFFF);
	EXPECT_EQ(3, len);
	EXPECT_EQ(0xEF, (uint8_t)txt[0]);
	EXPECT_EQ(0xBF, (uint8_t)txt[1]);
	EXPECT_EQ(0xBF, (uint8_t)txt[2]);
}

TEST(UTF8, EncodeCharFourBytesMin)
{
	char txt[10];
	auto len = utf8::ConvertChar(txt, 0x10000);
	EXPECT_EQ(4, len);
	EXPECT_EQ(0xF0, (uint8_t)txt[0]);
	EXPECT_EQ(0x90, (uint8_t)txt[1]);
	EXPECT_EQ(0x80, (uint8_t)txt[2]);
	EXPECT_EQ(0x80, (uint8_t)txt[3]);
}

TEST(UTF8, EncodeCharFourBytesMax)
{
	char txt[10];
	auto len = utf8::ConvertChar(txt, 0x10FFFF);
	EXPECT_EQ(4, len);
	EXPECT_EQ(0xF4, (uint8_t)txt[0]);
	EXPECT_EQ(0x8F, (uint8_t)txt[1]);
	EXPECT_EQ(0xBF, (uint8_t)txt[2]);
	EXPECT_EQ(0xBF, (uint8_t)txt[3]);
}

TEST(UTF8, Valid1)
{
	uint8_t data[] = { 0x20 };
	const auto* txt = (const char*)data;
	EXPECT_TRUE(utf8::ValidChar(txt, txt + sizeof(data)));
}

TEST(UTF8, Valid1FailsWithInvalidLength)
{
	uint8_t data[] = { 0x20 };
	const auto* txt = (const char*)data;
	EXPECT_FALSE(utf8::ValidChar(txt, txt));
}

TEST(UTF8, Valid2FailsWithWrongBitPattern1)
{
	uint8_t data[] = { 0x80 };
	const auto* txt = (const char*)data;
	EXPECT_FALSE(utf8::ValidChar(txt, txt + sizeof(data)));
}

TEST(UTF8, Valid2FailsWithWrongBitPattern2)
{
	uint8_t data[] = { 0xFF, 0x00 };
	const auto* txt = (const char*)data;
	EXPECT_FALSE(utf8::ValidChar(txt, txt + sizeof(data)));
}

TEST(UTF8, Valid2FailsWithWrongContinuation)
{
	uint8_t data[] = { 0xC0, 0x00 };
	const auto* txt = (const char*)data;
	EXPECT_FALSE(utf8::ValidChar(txt, txt + sizeof(data)));
}

TEST(UTF8, Valid2FailsWithWrongContinuation2)
{
	uint8_t data[] = { 0xC0, 0xF0 };
	const auto* txt = (const char*)data;
	EXPECT_FALSE(utf8::ValidChar(txt, txt + sizeof(data)));
}

TEST(UTF8, Valid2FailsWithWrongLength)
{
	uint8_t data[] = { 0xC0 };
	const auto* txt = (const char*)data;
	EXPECT_FALSE(utf8::ValidChar(txt, txt + sizeof(data)));
}

TEST(UTF8, Valid2)
{
	uint8_t data[] = { 0xC0, 0x80 };
	const auto* txt = (const char*)data;
	EXPECT_TRUE(utf8::ValidChar(txt, txt + sizeof(data)));
}

TEST(UTF8, Valid3FailsWithWrongContinuation1)
{
	uint8_t data[] = { 0xE0, 0x00, 0x00 };
	const auto* txt = (const char*)data;
	EXPECT_FALSE(utf8::ValidChar(txt, txt + sizeof(data)));
}

TEST(UTF8, Valid3FailsWithWrongContinuation)
{
	uint8_t data[] = { 0xE0, 0x80, 0x00 };
	const auto* txt = (const char*)data;
	EXPECT_FALSE(utf8::ValidChar(txt, txt + sizeof(data)));
}

TEST(UTF8, Valid3FailsWithWrongLength1)
{
	uint8_t data[] = { 0xE0 };
	const auto* txt = (const char*)data;
	EXPECT_FALSE(utf8::ValidChar(txt, txt + sizeof(data)));
}

TEST(UTF8, Valid3FailsWithWrongLength2)
{
	uint8_t data[] = { 0xE0, 0x80 };
	const auto* txt = (const char*)data;
	EXPECT_FALSE(utf8::ValidChar(txt, txt + sizeof(data)));
}

TEST(UTF8, Valid3)
{
	uint8_t data[] = { 0xE0, 0x80, 0x80 };
	const auto* txt = (const char*)data;
	EXPECT_TRUE(utf8::ValidChar(txt, txt + sizeof(data)));
}

TEST(UTF8, Valid4FailsWithWrongContinuation1)
{
	uint8_t data[] = { 0xF0 };
	const auto* txt = (const char*)data;
	EXPECT_FALSE(utf8::ValidChar(txt, txt + sizeof(data)));
}

TEST(UTF8, Valid4FailsWithWrongContinuation2)
{
	uint8_t data[] = { 0xF0, 0x00 };
	const auto* txt = (const char*)data;
	EXPECT_FALSE(utf8::ValidChar(txt, txt + sizeof(data)));
}

TEST(UTF8, Valid4FailsWithWrongContinuation3)
{
	uint8_t data[] = { 0xF0, 0x80, 0x00 };
	const auto* txt = (const char*)data;
	EXPECT_FALSE(utf8::ValidChar(txt, txt + sizeof(data)));
}

TEST(UTF8, Valid4FailsWithWrongLength1)
{
	uint8_t data[] = { 0xF0 };
	const auto* txt = (const char*)data;
	EXPECT_FALSE(utf8::ValidChar(txt, txt + sizeof(data)));
}

TEST(UTF8, Valid4FailsWithWrongLength2)
{
	uint8_t data[] = { 0xF0, 0x80 };
	const auto* txt = (const char*)data;
	EXPECT_FALSE(utf8::ValidChar(txt, txt + sizeof(data)));
}

TEST(UTF8, Valid4FailsWithWrongLength3)
{
	uint8_t data[] = { 0xF0, 0x80, 0x80 };
	const auto* txt = (const char*)data;
	EXPECT_FALSE(utf8::ValidChar(txt, txt + sizeof(data)));
}

TEST(UTF8, Valid4)
{
	uint8_t data[] = { 0xF0, 0x80, 0x80, 0x80 };
	const auto* txt = (const char*)data;
	EXPECT_TRUE(utf8::ValidChar(txt, txt + sizeof(data)));
}

//--

TEST(UTF8, LengthEmpty)
{
	const char* txt = "";
	EXPECT_EQ(0, utf8::Length(txt, txt + strlen(txt)));
}

TEST(UTF8, LengthAnsi)
{
	const char* txt = "test";
	EXPECT_EQ(4, utf8::Length(txt, txt + strlen(txt)));
}

TEST(UTF8, LengthTwoBytes)
{
	const char* txt = u8"zażółć";
	EXPECT_EQ(6, utf8::Length(txt, txt + strlen(txt)));
}

TEST(UTF8, LengthThreeBytes)
{
	const char* txt = u8"रऱलळऴव";
	EXPECT_EQ(6, utf8::Length(txt, txt + strlen(txt)));
}

TEST(UTF8, LengthFourBytes)
{
	const char* txt = u8"𒀁𒀂𒀃𒀄𒀅𒀆";
	EXPECT_EQ(6, utf8::Length(txt, txt + strlen(txt)));
}

//--

TEST(UTF8, NextCharDetectsEnd)
{
	const char* txt = "";
	const char* end = txt + strlen(txt);
	EXPECT_EQ(0, utf8::NextChar(txt, end));
}

TEST(UTF8, NextCharAnsi)
{
	const char* txt = "test";
	const char* end = txt + strlen(txt);
	EXPECT_EQ('t', utf8::NextChar(txt, end));
	EXPECT_EQ('e', utf8::NextChar(txt, end));
	EXPECT_EQ('s', utf8::NextChar(txt, end));
	EXPECT_EQ('t', utf8::NextChar(txt, end));
	EXPECT_EQ(0, utf8::NextChar(txt, end));
}

TEST(UTF8, NextCharTwoByte)
{
	const char* txt = u8"zażółć";
	const char* end = txt + strlen(txt);
	EXPECT_EQ('z', utf8::NextChar(txt, end));
	EXPECT_EQ('a', utf8::NextChar(txt, end));

	const auto ch0 = u'ż';
	const auto ch1 = u'ó';
	const auto ch2 = u'ł';
	const auto ch3 = u'ć';
	EXPECT_EQ(ch0, utf8::NextChar(txt, end));
	EXPECT_EQ(ch1, utf8::NextChar(txt, end));
	EXPECT_EQ(ch2, utf8::NextChar(txt, end));
	EXPECT_EQ(ch3, utf8::NextChar(txt, end));
	EXPECT_EQ(0, utf8::NextChar(txt, end));
}

TEST(UTF8, NextCharThreeBytes)
{
	const char* txt = u8"रऱलळऴव";
	const char* end = txt + strlen(txt);
	const auto ch0 = U'र';
	const auto ch1 = U'ऱ';
	const auto ch2 = U'ल';
	const auto ch3 = U'ळ';
	const auto ch4 = U'ऴ';
	const auto ch5 = U'व';
	EXPECT_EQ(ch0, utf8::NextChar(txt, end));
	EXPECT_EQ(ch1, utf8::NextChar(txt, end));
	EXPECT_EQ(ch2, utf8::NextChar(txt, end));
	EXPECT_EQ(ch3, utf8::NextChar(txt, end));
	EXPECT_EQ(ch4, utf8::NextChar(txt, end));
	EXPECT_EQ(ch5, utf8::NextChar(txt, end));
	EXPECT_EQ(0, utf8::NextChar(txt, end));
}

TEST(UTF8, NextCharFourBytes)
{
	const char* txt = u8"𒀁𒀂𒀃𒀄𒀅𒀆";
	const char* end = txt + strlen(txt);
	const auto ch0 = U'𒀁';
	const auto ch1 = U'𒀂';
	const auto ch2 = U'𒀃';
	const auto ch3 = U'𒀄';
	const auto ch4 = U'𒀅';
	const auto ch5 = U'𒀆';
	EXPECT_EQ(ch0, utf8::NextChar(txt, end));
	EXPECT_EQ(ch1, utf8::NextChar(txt, end));
	EXPECT_EQ(ch2, utf8::NextChar(txt, end));
	EXPECT_EQ(ch3, utf8::NextChar(txt, end));
	EXPECT_EQ(ch4, utf8::NextChar(txt, end));
	EXPECT_EQ(ch5, utf8::NextChar(txt, end));
	EXPECT_EQ(0, utf8::NextChar(txt, end));
}

TEST(UTF8, CalcSizeAnsi)
{
	uint32_t txt[5] = { 't', 'e', 's', 't', 0 };
	const auto size = utf8::CalcSizeRequired(txt);
	EXPECT_EQ(4, size);
}

TEST(UTF8, CalcSizeTwoByte)
{
	uint32_t txt[7] = { 'z', 'a', U'ż', U'ó', U'ł', U'ć', 0 };
	const auto size = utf8::CalcSizeRequired(txt);
	EXPECT_EQ(10, size);
}

TEST(UTF8, CalcSizeThreeByte)
{
	uint32_t txt[7] = { U'र', U'ऱ', U'ल', U'ळ', U'ऴ', U'व', 0 };	
	const auto size = utf8::CalcSizeRequired(txt);
	EXPECT_EQ(18, size);
}

TEST(UTF8, CalcSizeFourBytes)
{
	uint32_t txt[7] = { U'𒀁', U'𒀂', U'𒀃', U'𒀄', U'𒀅', U'𒀆', 0 };
	const auto size = utf8::CalcSizeRequired(txt);
	EXPECT_EQ(24, size);
}

//--

TEST(UTF8, ToUniCharAnsi)
{
	wchar_t dest[10];
	const auto* txt = "test";

	auto written = utf8::ToUniChar(dest, ARRAY_COUNT(dest), txt, strlen(txt));
	EXPECT_EQ(4, written);
	EXPECT_EQ('t', dest[0]);
	EXPECT_EQ('e', dest[1]);
	EXPECT_EQ('s', dest[2]);
	EXPECT_EQ('t', dest[3]);
	EXPECT_EQ(0, dest[4]);
}

TEST(UTF8, ToUniCharTwoBytes)
{
	wchar_t dest[30];
	const auto* txt = u8"zażółć";

	auto written = utf8::ToUniChar(dest, ARRAY_COUNT(dest), txt, strlen(txt));
	EXPECT_EQ(6, written);
	EXPECT_EQ('z', dest[0]);
	EXPECT_EQ('a', dest[1]);
	const auto ch0 = U'ż';
	const auto ch1 = U'ó';
	const auto ch2 = U'ł';
	const auto ch3 = U'ć';
	EXPECT_EQ(ch0, dest[2]);
	EXPECT_EQ(ch1, dest[3]);
	EXPECT_EQ(ch2, dest[4]);
	EXPECT_EQ(ch3, dest[5]);	
	EXPECT_EQ(0, dest[6]);
}

TEST(UTF8, ToUniCharThreeBytes)
{
	wchar_t dest[30];
	const auto* txt = u8"रऱलळऴव";

	auto written = utf8::ToUniChar(dest, ARRAY_COUNT(dest), txt, strlen(txt));
	EXPECT_EQ(6, written);
	const auto ch0 = U'र';
	const auto ch1 = U'ऱ';
	const auto ch2 = U'ल';
	const auto ch3 = U'ळ';
	const auto ch4 = U'ऴ';
	const auto ch5 = U'व';
	EXPECT_EQ(ch0, dest[0]);
	EXPECT_EQ(ch1, dest[1]);
	EXPECT_EQ(ch2, dest[2]);
	EXPECT_EQ(ch3, dest[3]);
	EXPECT_EQ(ch4, dest[4]);
	EXPECT_EQ(ch5, dest[5]);
	EXPECT_EQ(0, dest[6]);
}

TEST(UTF8, ToUniCharFourBytes)
{
	wchar_t dest[30];
	const auto* txt = u8"𒀁𒀂𒀃𒀄𒀅𒀆";

	auto written = utf8::ToUniChar(dest, ARRAY_COUNT(dest), txt, strlen(txt));
	EXPECT_EQ(6, written);
	EXPECT_EQ('?', dest[0]);
	EXPECT_EQ('?', dest[1]);
	EXPECT_EQ('?', dest[2]);
	EXPECT_EQ('?', dest[3]);
	EXPECT_EQ('?', dest[4]);
	EXPECT_EQ('?', dest[5]);
	EXPECT_EQ(0, dest[6]);
}

//--

TEST(UTF8, ToUniCharWritesZeroTermination)
{
	wchar_t dest[10] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
	const auto* txt = "test";

	auto written = utf8::ToUniChar(dest, ARRAY_COUNT(dest), txt, strlen(txt));
	EXPECT_EQ(0, dest[4]);
}

TEST(UTF8, ToUniCharDoesNotFit)
{
	wchar_t dest[3];
	const auto* txt = "test";

	auto written = utf8::ToUniChar(dest, ARRAY_COUNT(dest), txt, strlen(txt));
	EXPECT_EQ(2, written);
	EXPECT_EQ('t', dest[0]);
	EXPECT_EQ('e', dest[1]);
	EXPECT_EQ(0, dest[2]);
}

TEST(UTF8, ToUniCharDoesNotFitZeroTerminationStillWritten)
{
	wchar_t dest[1] = {0xFF};
	const auto* txt = "test";

	auto written = utf8::ToUniChar(dest, ARRAY_COUNT(dest), txt, strlen(txt));
	EXPECT_EQ(0, written);
	EXPECT_EQ(0, dest[0]);
}

TEST(UTF8, ToUniCharDoesNotFitZeroBuffer)
{
	wchar_t dest[3] = {0xFF, 0xFF, 0xFF };
	const auto* txt = "test";

	auto written = utf8::ToUniChar(dest, 0, txt, strlen(txt));
	EXPECT_EQ(0, written);
}

TEST(UTF8, ToUniCharDoesNotFitZeroBufferTerminationNotWritten)
{
	wchar_t dest[3] = { 0xFF, 0xFF, 0xFF };
	const auto* txt = "test";

	auto written = utf8::ToUniChar(dest, 0, txt, strlen(txt));
	EXPECT_EQ(0xFF, dest[0]);
}

//--

TEST(UTF8, ToUniChar32Ansi)
{
	uint32_t dest[10];
	const auto* txt = "test";

	auto written = utf8::ToUniChar32(dest, ARRAY_COUNT(dest), txt, strlen(txt));
	EXPECT_EQ(4, written);
	EXPECT_EQ('t', dest[0]);
	EXPECT_EQ('e', dest[1]);
	EXPECT_EQ('s', dest[2]);
	EXPECT_EQ('t', dest[3]);
	EXPECT_EQ(0, dest[4]);
}

TEST(UTF8, ToUniChar32TwoBytes)
{
	uint32_t dest[30];
	const auto* txt = u8"zażółć";

	auto written = utf8::ToUniChar32(dest, ARRAY_COUNT(dest), txt, strlen(txt));
	EXPECT_EQ(6, written);
	EXPECT_EQ('z', dest[0]);
	EXPECT_EQ('a', dest[1]);
	const auto ch0 = U'ż';
	const auto ch1 = U'ó';
	const auto ch2 = U'ł';
	const auto ch3 = U'ć';
	EXPECT_EQ(ch0, dest[2]);
	EXPECT_EQ(ch1, dest[3]);
	EXPECT_EQ(ch2, dest[4]);
	EXPECT_EQ(ch3, dest[5]);
	EXPECT_EQ(0, dest[6]);
}

TEST(UTF8, ToUniChar32ThreeBytes)
{
	uint32_t dest[30];
	const auto* txt = u8"रऱलळऴव";

	auto written = utf8::ToUniChar32(dest, ARRAY_COUNT(dest), txt, strlen(txt));
	EXPECT_EQ(6, written);
	const auto ch0 = U'र';
	const auto ch1 = U'ऱ';
	const auto ch2 = U'ल';
	const auto ch3 = U'ळ';
	const auto ch4 = U'ऴ';
	const auto ch5 = U'व';
	EXPECT_EQ(ch0, dest[0]);
	EXPECT_EQ(ch1, dest[1]);
	EXPECT_EQ(ch2, dest[2]);
	EXPECT_EQ(ch3, dest[3]);
	EXPECT_EQ(ch4, dest[4]);
	EXPECT_EQ(ch5, dest[5]);
	EXPECT_EQ(0, dest[6]);
}

TEST(UTF8, ToUniChar32FourBytes)
{
	uint32_t dest[30];
	const auto* txt = u8"𒀁𒀂𒀃𒀄𒀅𒀆";

	auto written = utf8::ToUniChar32(dest, ARRAY_COUNT(dest), txt, strlen(txt));
	EXPECT_EQ(6, written);
	const auto ch0 = U'𒀁';
	const auto ch1 = U'𒀂';
	const auto ch2 = U'𒀃';
	const auto ch3 = U'𒀄';
	const auto ch4 = U'𒀅';
	const auto ch5 = U'𒀆';
	EXPECT_EQ(ch0, dest[0]);
	EXPECT_EQ(ch1, dest[1]);
	EXPECT_EQ(ch2, dest[2]);
	EXPECT_EQ(ch3, dest[3]);
	EXPECT_EQ(ch4, dest[4]);
	EXPECT_EQ(ch5, dest[5]);
	EXPECT_EQ(0, dest[6]);
}

//--

TEST(UTF8, ToUniChar32WritesZeroTermination)
{
	uint32_t dest[10] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
	const auto* txt = "test";

	auto written = utf8::ToUniChar32(dest, ARRAY_COUNT(dest), txt, strlen(txt));
	EXPECT_EQ(0, dest[4]);
}

TEST(UTF8, ToUniChar32DoesNotFit)
{
	uint32_t dest[3];
	const auto* txt = "test";

	auto written = utf8::ToUniChar32(dest, ARRAY_COUNT(dest), txt, strlen(txt));
	EXPECT_EQ(2, written);
	EXPECT_EQ('t', dest[0]);
	EXPECT_EQ('e', dest[1]);
	EXPECT_EQ(0, dest[2]);
}

TEST(UTF8, ToUniChar32DoesNotFitZeroTerminationStillWritten)
{
	uint32_t dest[1] = { 0xFF };
	const auto* txt = "test";

	auto written = utf8::ToUniChar32(dest, ARRAY_COUNT(dest), txt, strlen(txt));
	EXPECT_EQ(0, written);
	EXPECT_EQ(0, dest[0]);
}

TEST(UTF8, ToUniChar32DoesNotFitZeroBuffer)
{
	uint32_t dest[3] = { 0xFF, 0xFF, 0xFF };
	const auto* txt = "test";

	auto written = utf8::ToUniChar32(dest, 0, txt, strlen(txt));
	EXPECT_EQ(0, written);
}

TEST(UTF8, ToUniChar32DoesNotFitZeroBufferTerminationNotWritten)
{
	uint32_t dest[3] = { 0xFF, 0xFF, 0xFF };
	const auto* txt = "test";

	auto written = utf8::ToUniChar32(dest, 0, txt, strlen(txt));
	EXPECT_EQ(0xFF, dest[0]);
}

//--

TEST(UTF8, FromUniCharAnsi)
{
	wchar_t txt[] = { 't', 'e', 's', 't' };

	char dest[10];
	const auto len = utf8::FromUniChar(dest, ARRAY_COUNT(dest), txt, ARRAY_COUNT(txt));
	EXPECT_EQ(4, len);
	EXPECT_EQ('t', dest[0]);
	EXPECT_EQ('e', dest[1]);
	EXPECT_EQ('s', dest[2]);
	EXPECT_EQ('t', dest[3]);
}

TEST(UTF8, FromUniCharTwoBytes)
{
	wchar_t txt[] = { 'z', 'a', U'ż', U'ó', U'ł', U'ć' };

	char dest[32];
	const auto len = utf8::FromUniChar(dest, ARRAY_COUNT(dest), txt, ARRAY_COUNT(txt));

	const auto* test = "\x7A\x61\xC5\xBC\xC3\xB3\xC5\x82\xC4\x87";
	EXPECT_EQ(10, len);
	EXPECT_STREQ(test, dest);
}

TEST(UTF8, FromUniCharWritesZeroTermination)
{
	wchar_t txt[] = { 't', 'e', 's', 't' };

	char dest[10] = { -1,-1,-1,-1,-1,-1,-1,-1,-1,-1 };
	const auto len = utf8::FromUniChar(dest, ARRAY_COUNT(dest), txt, ARRAY_COUNT(txt));
	EXPECT_EQ(0, dest[4]);
}

TEST(UTF8, FromUniCharDoesNotFit)
{
	wchar_t txt[] = { 't', 'e', 's', 't' };

	char dest[3];
	const auto len = utf8::FromUniChar(dest, ARRAY_COUNT(dest), txt, ARRAY_COUNT(txt));
	EXPECT_EQ(2, len);
	EXPECT_EQ('t', dest[0]);
	EXPECT_EQ('e', dest[1]);
	EXPECT_EQ(0, dest[2]);
}

TEST(UTF8, FromUniCharDoesNotFitZeroTerminationStillWritten)
{
	wchar_t txt[] = { 't', 'e', 's', 't' };

	char dest[1];
	const auto len = utf8::FromUniChar(dest, ARRAY_COUNT(dest), txt, ARRAY_COUNT(txt));
	EXPECT_EQ(0, len);
	EXPECT_EQ(0, dest[0]);
}

TEST(UTF8, FromUniCharDoesNotFitZeroBuffer)
{
	wchar_t txt[] = { 't', 'e', 's', 't' };

	char dest[1] = { -1 };
	const auto len = utf8::FromUniChar(dest, 0, txt, ARRAY_COUNT(txt));
	EXPECT_EQ(0, len);
}

TEST(UTF8, FromUniCharDoesNotFitZeroBufferTerminationNotWritten)
{
	wchar_t txt[] = { 't', 'e', 's', 't' };

	char dest[1] = { -1 };
	const auto len = utf8::FromUniChar(dest, 0, txt, ARRAY_COUNT(txt));
	EXPECT_EQ(-1, dest[0]);
}

//--


TEST(UTF8, FromUniChar32Ansi)
{
	uint32_t txt[] = { 't', 'e', 's', 't' };

	char dest[10];
	const auto len = utf8::FromUniChar32(dest, ARRAY_COUNT(dest), txt, ARRAY_COUNT(txt));
	EXPECT_EQ(4, len);
	EXPECT_EQ('t', dest[0]);
	EXPECT_EQ('e', dest[1]);
	EXPECT_EQ('s', dest[2]);
	EXPECT_EQ('t', dest[3]);
}

TEST(UTF8, FromUniChar32TwoBytes)
{
	uint32_t txt[] = { 'z', 'a', U'ż', U'ó', U'ł', U'ć' };

	char dest[32];
	const auto len = utf8::FromUniChar32(dest, ARRAY_COUNT(dest), txt, ARRAY_COUNT(txt));

	const auto* test = "\x7A\x61\xC5\xBC\xC3\xB3\xC5\x82\xC4\x87";
	EXPECT_EQ(10, len);
	EXPECT_STREQ(test, dest);
}

TEST(UTF8, FromUniChar32ThreeBytes)
{
	uint32_t txt[] = { U'र', U'ऱ', U'ल', U'ळ', U'ऴ', U'व' };

	char dest[32];
	const auto len = utf8::FromUniChar32(dest, ARRAY_COUNT(dest), txt, ARRAY_COUNT(txt));

	const auto* test = "\xE0\xA4\xB0\xE0\xA4\xB1\xE0\xA4\xB2\xE0\xA4\xB3\xE0\xA4\xB4\xE0\xA4\xB5";
	EXPECT_EQ(18, len);
	EXPECT_STREQ(test, dest);
}

TEST(UTF8, FromUniChar32FourBytes)
{
	uint32_t txt[] = { U'𒀁', U'𒀂', U'𒀃', U'𒀄', U'𒀅', U'𒀆' };

	char dest[32];
	const auto len = utf8::FromUniChar32(dest, ARRAY_COUNT(dest), txt, ARRAY_COUNT(txt));

	const auto* test = "\xF0\x92\x80\x81\xF0\x92\x80\x82\xF0\x92\x80\x83\xF0\x92\x80\x84\xF0\x92\x80\x85\xF0\x92\x80\x86";
	EXPECT_EQ(24, len);
	EXPECT_STREQ(test, dest);
}

TEST(UTF8, FromUniChar32WritesZeroTermination)
{
	uint32_t txt[] = { 't', 'e', 's', 't' };

	char dest[10] = { -1,-1,-1,-1,-1,-1,-1,-1,-1,-1 };
	const auto len = utf8::FromUniChar32(dest, ARRAY_COUNT(dest), txt, ARRAY_COUNT(txt));
	EXPECT_EQ(0, dest[4]);
}

TEST(UTF8, FromUniChar32DoesNotFit)
{
	uint32_t txt[] = { 't', 'e', 's', 't' };

	char dest[3];
	const auto len = utf8::FromUniChar32(dest, ARRAY_COUNT(dest), txt, ARRAY_COUNT(txt));
	EXPECT_EQ(2, len);
	EXPECT_EQ('t', dest[0]);
	EXPECT_EQ('e', dest[1]);
	EXPECT_EQ(0, dest[2]);
}

TEST(UTF8, FromUniChar32DoesNotFitZeroTerminationStillWritten)
{
	uint32_t txt[] = { 't', 'e', 's', 't' };

	char dest[1];
	const auto len = utf8::FromUniChar32(dest, ARRAY_COUNT(dest), txt, ARRAY_COUNT(txt));
	EXPECT_EQ(0, len);
	EXPECT_EQ(0, dest[0]);
}

TEST(UTF8, FromUniChar32DoesNotFitZeroBuffer)
{
	uint32_t txt[] = { 't', 'e', 's', 't' };

	char dest[1] = { -1 };
	const auto len = utf8::FromUniChar32(dest, 0, txt, ARRAY_COUNT(txt));
	EXPECT_EQ(0, len);
}

TEST(UTF8, FromUniChar32DoesNotFitZeroBufferTerminationNotWritten)
{
	uint32_t txt[] = { 't', 'e', 's', 't' };

	char dest[1] = { -1 };
	const auto len = utf8::FromUniChar32(dest, 0, txt, ARRAY_COUNT(txt));
	EXPECT_EQ(-1, dest[0]);
}

//--

END_INFERNO_NAMESPACE()
