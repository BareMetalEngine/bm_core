/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "bm/core/memory/include/bufferView.h"
#include "bm/core/memory/include/buffer.h"

BEGIN_INFERNO_NAMESPACE()

//---

static BufferView CreateStringView(const char* str)
{
	if (str && *str)
		return BufferView(str, str + strlen(str));
	else
		return nullptr;
}

template< typename T >
static BufferView CreateDataView(const T& data)
{
	return BufferView(&data, sizeof(T));
}

#define COMPARE_STRING(input, str) { \
	auto view = BufferView(input); \
	const auto a = std::string_view((const char*)view.data(), view.size()); \
	const auto b = std::string_view(str); \
	EXPECT_EQ(a, b); } \

TEST(EncodeURL, EncodeEmpty)
{
	const auto txt = CreateStringView("");

	InplaceBufferOutputStream<char, 256> output;
	EXPECT_TRUE(txt.encode(EncodingType::URL, output));

	EXPECT_TRUE(output.empty());
}

TEST(EncodeURL, EncodeSimpleNoChange)
{
	auto txt = CreateStringView("test");

	InplaceBufferOutputStream<char, 256> output;
	EXPECT_TRUE(txt.encode(EncodingType::URL, output));
	
	COMPARE_STRING(output, "test");
}

TEST(EncodeURL, EncodeUTF8)
{
	auto txt = CreateStringView(u8"Hello Günter");

	InplaceBufferOutputStream<char, 256> output;
	EXPECT_TRUE(txt.encode(EncodingType::URL, output));

	COMPARE_STRING(output, "Hello%20G%C3%BCnter");
}

TEST(EncodeURL, EncodingFailsIfBufferToSmall)
{
	auto txt = CreateStringView("Ala ma kota");

	InplaceBufferOutputStream<char, 5> output;
	EXPECT_FALSE(txt.encode(EncodingType::URL, output));
}


TEST(EncodeURL, EncodeComplex)
{
	auto txt = CreateStringView("{x=15;z=10;w=\"This is not a test\"}");

	InplaceBufferOutputStream<char, 256> output;
	EXPECT_TRUE(txt.encode(EncodingType::URL, output));

	COMPARE_STRING(output, "%7Bx%3D15%3Bz%3D10%3Bw%3D%22This%20is%20not%20a%20test%22%7D");	
}

//---

TEST(DecodeURL, DecodeEmpty)
{
	const auto txt = CreateStringView("");

	InplaceBufferOutputStream<char, 256> output;
	EXPECT_TRUE(txt.encode(EncodingType::URL, output));

	EXPECT_TRUE(output.empty());
}

TEST(DecodeURL, DecodeSimple)
{
	const auto txt = CreateStringView("test");

	InplaceBufferOutputStream<uint8_t, 256> output;
	EXPECT_TRUE(txt.decode(EncodingType::URL, output));

	COMPARE_STRING(output, "test");
}

TEST(DecodeURL, DecodeFailsOnInvalidChar)
{
	const auto txt = CreateStringView("te:st");

	InplaceBufferOutputStream<uint8_t, 256> output;
	EXPECT_FALSE(txt.decode(EncodingType::URL, output));
}

TEST(DecodeURL, DecodeWorksWithOldSpaceEncoding)
{
	const auto txt = CreateStringView("te+st");

	InplaceBufferOutputStream<uint8_t, 256> output;
	EXPECT_TRUE(txt.decode(EncodingType::URL, output, false));

	COMPARE_STRING(output, "te st");
}

TEST(DecodeURL, DecodeFailsOnWhiteSpaceIfIngoringNotEnabled)
{
	const auto txt = CreateStringView("te st");

	InplaceBufferOutputStream<uint8_t, 256> output;
	EXPECT_FALSE(txt.decode(EncodingType::URL, output, false));
}

TEST(DecodeURL, DecodeWorksWithWhiteSpaceIfIngoringEnabled)
{
	const auto txt = CreateStringView("te st");

	InplaceBufferOutputStream<uint8_t, 256> output;
	EXPECT_TRUE(txt.decode(EncodingType::URL, output, true));

	COMPARE_STRING(output, "test");
}

TEST(DecodeURL, DecodeUTF8)
{
	const auto txt = CreateStringView("Hello%20G%C3%BCnter");

	InplaceBufferOutputStream<uint8_t, 256> output;
	EXPECT_TRUE(txt.decode(EncodingType::URL, output));

	COMPARE_STRING(output, u8"Hello Günter");
}

TEST(DecodeURL, DecodeFailsIfBufferToSmall)
{
	auto txt = CreateStringView("Ala ma kota");

	InplaceBufferOutputStream<uint8_t, 5> output;
	EXPECT_FALSE(txt.decode(EncodingType::URL, output));
}

TEST(DecodeURL, DecodeComplex)
{
	auto txt = CreateStringView("%7Bx%3D15%3Bz%3D10%3Bw%3D%22This%20is%20not%20a%20test%22%7D");

	InplaceBufferOutputStream<uint8_t, 256> output;
	EXPECT_TRUE(txt.decode(EncodingType::URL, output));

	COMPARE_STRING(output, "{x=15;z=10;w=\"This is not a test\"}");
}

//---

TEST(EncodeHEX, EncodeEmpty)
{
	auto txt = BufferView();

	InplaceBufferOutputStream<char, 256> output;
	EXPECT_TRUE(txt.encode(EncodingType::Hex, output));
	EXPECT_TRUE(output.empty());
}

TEST(EncodeHEX, EncodeValueShort)
{
	uint8_t data[4] = { 0xDE,0xAD,0xF0,0x0D };
	auto txt = CreateDataView(data);

	InplaceBufferOutputStream<char, 256> output;
	EXPECT_TRUE(txt.encode(EncodingType::Hex, output));

	COMPARE_STRING(output, "DEADF00D");
}

TEST(EncodeHEX, EncodeValueZero)
{
	uint8_t data[4] = { 0,0,0,0 };
	auto txt = CreateDataView(data);

	InplaceBufferOutputStream<char, 256> output;
	EXPECT_TRUE(txt.encode(EncodingType::Hex, output));

	COMPARE_STRING(output, "00000000");
}

TEST(EncodeHEX, EncodeValueLong)
{
	uint8_t data[32] = { 0xDE,0xAD,0xF0,0x0D,0xDE,0xAD,0xF0,0x0D,0xDE,0xAD,0xF0,0x0D,0xDE,0xAD,0xF0,0x0D,0xDE,0xAD,0xF0,0x0D,0xDE,0xAD,0xF0,0x0D,0xDE,0xAD,0xF0,0x0D,0xDE,0xAD,0xF0,0x0D };
	auto txt = CreateDataView(data);

	InplaceBufferOutputStream<char, 256> output;
	EXPECT_TRUE(txt.encode(EncodingType::Hex, output));

	COMPARE_STRING(output, "DEADF00DDEADF00DDEADF00DDEADF00DDEADF00DDEADF00DDEADF00DDEADF00D");
}

TEST(EncodeHEX, EncodingFailsIfBufferToSmall)
{
	uint8_t data[4] = { 0xDE,0xAD,0xF0,0x0D };
	auto txt = CreateDataView(data);

	InplaceBufferOutputStream<char, 4> output;
	EXPECT_FALSE(txt.encode(EncodingType::Hex, output));
}

//---

TEST(DecodeHEX, DecodeEmpty)
{
	const auto txt = CreateStringView("");

	InplaceBufferOutputStream<char, 256> output;
	EXPECT_TRUE(txt.encode(EncodingType::Hex, output));

	EXPECT_TRUE(output.empty());
}

TEST(DecodeHEX, DecodeSimple)
{
	const auto txt = CreateStringView("DEADF00D");

	InplaceBufferOutputStream<uint8_t, 256> output;
	EXPECT_TRUE(txt.decode(EncodingType::Hex, output));
	EXPECT_EQ(4, output.size());

	const uint8_t data[4] = { 0xDE, 0xAD, 0xF0, 0x0D };
	EXPECT_EQ(0, memcmp(output.start(), data, 4));	
}

TEST(DecodeHEX, DecodeSimpleLowerCase)
{
	const auto txt = CreateStringView("deadf00d");

	InplaceBufferOutputStream<uint8_t, 256> output;
	EXPECT_TRUE(txt.decode(EncodingType::Hex, output));
	EXPECT_EQ(4, output.size());

	const uint8_t data[4] = { 0xDE, 0xAD, 0xF0, 0x0D };
	EXPECT_EQ(0, memcmp(output.start(), data, 4));
}

TEST(DecodeHEX, DecodeSimpleMixedCase)
{
	const auto txt = CreateStringView("DeaDF00d");

	InplaceBufferOutputStream<uint8_t, 256> output;
	EXPECT_TRUE(txt.decode(EncodingType::Hex, output));
	EXPECT_EQ(4, output.size());

	const uint8_t data[4] = { 0xDE, 0xAD, 0xF0, 0x0D };
	EXPECT_EQ(0, memcmp(output.start(), data, 4));
}

TEST(DecodeHEX, DecodeFailsOnInvalidChar)
{
	const auto txt = CreateStringView("DEA:F00D");

	InplaceBufferOutputStream<uint8_t, 256> output;
	EXPECT_FALSE(txt.decode(EncodingType::Hex, output));
}

TEST(DecodeHEX, DecodeFailsIfNoPairs)
{
	const auto txt = CreateStringView("DEA");

	InplaceBufferOutputStream<uint8_t, 256> output;
	EXPECT_FALSE(txt.decode(EncodingType::Hex, output));
}

TEST(DecodeHEX, DecodeFailsOnWhiteSpaceIfIngoringNotEnabled)
{
	const auto txt = CreateStringView("DeaD F00d");

	InplaceBufferOutputStream<uint8_t, 256> output;
	EXPECT_FALSE(txt.decode(EncodingType::Hex, output, false));
}

TEST(DecodeHEX, DecodeWorksWithWhiteSpaceIfIngoringEnabled)
{
	const auto txt = CreateStringView("DeaD F00d");

	InplaceBufferOutputStream<uint8_t, 256> output;
	EXPECT_TRUE(txt.decode(EncodingType::Hex, output, true));

	EXPECT_EQ(4, output.size());

	const uint8_t data[4] = { 0xDE, 0xAD, 0xF0, 0x0D };
	EXPECT_EQ(0, memcmp(output.start(), data, 4));
}

//---

TEST(EncodeCString, EncodeEmpty)
{
	const auto txt = CreateStringView("");

	InplaceBufferOutputStream<char, 256> output;
	EXPECT_TRUE(txt.encode(EncodingType::CString, output));

	EXPECT_TRUE(output.empty());
}

TEST(EncodeCString, EncodeSimpleNoChange)
{
	auto txt = CreateStringView("test");

	InplaceBufferOutputStream<char, 256> output;
	EXPECT_TRUE(txt.encode(EncodingType::CString, output));

	COMPARE_STRING(output, "test");
}

TEST(EncodeCString, EncodeUTF8)
{
	auto txt = CreateStringView(u8"Hello Günter");

	InplaceBufferOutputStream<char, 256> output;
	EXPECT_TRUE(txt.encode(EncodingType::CString, output));

	COMPARE_STRING(output, "Hello G\\xC3\\xBCnter");
}

TEST(EncodeCString, EncodingFailsIfBufferToSmall)
{
	auto txt = CreateStringView("Ala ma kota");

	InplaceBufferOutputStream<char, 5> output;
	EXPECT_FALSE(txt.encode(EncodingType::CString, output));
}

TEST(EncodeCString, EncodeEscaped)
{
	auto txt = CreateStringView("\a\b\f\n\r\t\v\b\\\'\"\?");

	InplaceBufferOutputStream<char, 256> output;
	EXPECT_TRUE(txt.encode(EncodingType::CString, output));

	COMPARE_STRING(output, "\\a\\b\\f\\n\\r\\t\\v\\b\\\\\\\'\\\"\\\?");
}

TEST(EncodeCString, EncodeComplex)
{
	auto txt = CreateStringView("\"This is path\\to\\sth with a test\"");

	InplaceBufferOutputStream<char, 256> output;
	EXPECT_TRUE(txt.encode(EncodingType::CString, output));

	COMPARE_STRING(output, "\\\"This is path\\\\to\\\\sth with a test\\\"");	                       
}

TEST(EncodeCString, EncodeZeros)
{
	auto txt = BufferView("First\0Second", 12);

	InplaceBufferOutputStream<char, 256> output;
	EXPECT_TRUE(txt.encode(EncodingType::CString, output));

	COMPARE_STRING(output, "First\\0Second");
}

//---

TEST(DecodeCString, DecodeEmpty)
{
	const auto txt = CreateStringView("");

	InplaceBufferOutputStream<char, 256> output;
	EXPECT_TRUE(txt.encode(EncodingType::CString, output));

	EXPECT_TRUE(output.empty());
}

TEST(DecodeCString, DecodeSimple)
{
	const auto txt = CreateStringView("test");

	InplaceBufferOutputStream<uint8_t, 256> output;
	EXPECT_TRUE(txt.decode(EncodingType::CString, output));

	COMPARE_STRING(output, "test");
}

TEST(DecodeCString, DecodeFailsOnInvalidChar)
{
	const auto txt = CreateStringView("te\\gst");

	InplaceBufferOutputStream<uint8_t, 256> output;
	EXPECT_FALSE(txt.decode(EncodingType::CString, output));
}

TEST(DecodeCString, DecodeEscapedChars)
{
	const auto txt = CreateStringView("\\a\\b\\f\\n\\r\\t\\v\\b\\\\\\\'\\\"\\\?");

	InplaceBufferOutputStream<uint8_t, 256> output;
	EXPECT_TRUE(txt.decode(EncodingType::CString, output, false));

	COMPARE_STRING(output, "\a\b\f\n\r\t\v\b\\\'\"\?");
}

TEST(DecodeCString, DecodeHexChars)
{
	const auto txt = CreateStringView("\\xDE\\xAD\\xF0\\x0D");

	InplaceBufferOutputStream<uint8_t, 256> output;
	EXPECT_TRUE(txt.decode(EncodingType::CString, output, false));

	COMPARE_STRING(output, "\xDE\xAD\xF0\x0D");
}

TEST(DecodeCString, DecodeHexFailsIfToShort)
{
	const auto txt = CreateStringView("\\xD");

	InplaceBufferOutputStream<uint8_t, 256> output;
	EXPECT_FALSE(txt.decode(EncodingType::CString, output, false));
}

TEST(DecodeCString, DecodeEscapedFailsIfToShort)
{
	const auto txt = CreateStringView("\\");

	InplaceBufferOutputStream<uint8_t, 256> output;
	EXPECT_FALSE(txt.decode(EncodingType::CString, output, false));
}

TEST(DecodeCString, DecodeZeros)
{
	const auto txt = CreateStringView("\\0\\0");

	InplaceBufferOutputStream<uint8_t, 256> output;
	EXPECT_TRUE(txt.decode(EncodingType::CString, output, false));
	EXPECT_EQ(2, output.size());
	EXPECT_EQ(0, output.start()[0]);
	EXPECT_EQ(0, output.start()[1]);
}

//--

TEST(EncodeBase64, EncodeEmpty)
{
	auto txt = BufferView();

	InplaceBufferOutputStream<char, 256> output;
	EXPECT_TRUE(txt.encode(EncodingType::Base64, output));
	EXPECT_TRUE(output.empty());
}

TEST(EncodeBase64, EncodeSingleByteGetsPadded)
{
	uint8_t data[1] = { 'A' };
	auto txt = CreateDataView(data);

	InplaceBufferOutputStream<char, 256> output;
	EXPECT_TRUE(txt.encode(EncodingType::Base64, output));
	EXPECT_EQ(4, output.size());

	COMPARE_STRING(output, "QQ==");
}

TEST(EncodeBase64, EncodeDoubleBytesGetsPadded)
{
	uint8_t data[2] = { 'A', 'B' };
	auto txt = CreateDataView(data);

	InplaceBufferOutputStream<char, 256> output;
	EXPECT_TRUE(txt.encode(EncodingType::Base64, output));
	EXPECT_EQ(4, output.size());

	COMPARE_STRING(output, "QUI=");
}

TEST(EncodeHEX, EncodeFullBlock)
{
	uint8_t data[3] = { 'A', 'B', 'C' };
	auto txt = CreateDataView(data);

	InplaceBufferOutputStream<char, 256> output;
	EXPECT_TRUE(txt.encode(EncodingType::Base64, output));

	COMPARE_STRING(output, "QUJD");
}

TEST(EncodeBase64, EncodeSingleByteFailsIsBufferToSmall)
{
	uint8_t data[1] = { 'A' };
	auto txt = CreateDataView(data);

	InplaceBufferOutputStream<char, 3> output;
	EXPECT_FALSE(txt.encode(EncodingType::Base64, output));	
}

TEST(EncodeBase64, EncodeDoubleBytesFailsIsBufferToSmall)
{
	uint8_t data[2] = { 'A', 'B' };
	auto txt = CreateDataView(data);

	InplaceBufferOutputStream<char, 3> output;
	EXPECT_FALSE(txt.encode(EncodingType::Base64, output));
}

TEST(EncodeHEX, EncodeFullBlockFailsIsBufferToSmall)
{
	uint8_t data[3] = { 'A', 'B', 'C' };
	auto txt = CreateDataView(data);

	InplaceBufferOutputStream<char, 3> output;
	EXPECT_FALSE(txt.encode(EncodingType::Base64, output));
}

TEST(EncodeHEX, EncodeText)
{
	auto txt = CreateStringView("This is a test");

	InplaceBufferOutputStream<char, 256> output;
	EXPECT_TRUE(txt.encode(EncodingType::Base64, output));

	COMPARE_STRING(output, "VGhpcyBpcyBhIHRlc3Q=");
}

//--

TEST(DecodeBase64, DecodeEmpty)
{
	const auto txt = CreateStringView("");

	InplaceBufferOutputStream<char, 256> output;
	EXPECT_TRUE(txt.encode(EncodingType::Hex, output));

	EXPECT_TRUE(output.empty());
}

TEST(DecodeBase64, DecodeOneByte)
{
	const auto txt = CreateStringView("QQ==");

	InplaceBufferOutputStream<uint8_t, 256> output;
	EXPECT_TRUE(txt.decode(EncodingType::Base64, output));
	EXPECT_EQ(1, output.size());

	const uint8_t data[1] = { 'A'};
	EXPECT_EQ(0, memcmp(output.start(), data, 1));
}

TEST(DecodeBase64, DecodeTwoBytes)
{
	const auto txt = CreateStringView("QUI=");

	InplaceBufferOutputStream<uint8_t, 256> output;
	EXPECT_TRUE(txt.decode(EncodingType::Base64, output));
	EXPECT_EQ(2, output.size());

	const uint8_t data[2] = { 'A', 'B' };
	EXPECT_EQ(0, memcmp(output.start(), data, 2));
}

TEST(DecodeBase64, DecodeBlock)
{
	const auto txt = CreateStringView("QUJD");

	InplaceBufferOutputStream<uint8_t, 256> output;
	EXPECT_TRUE(txt.decode(EncodingType::Base64, output));
	EXPECT_EQ(3, output.size());

	const uint8_t data[3] = { 'A', 'B', 'C'};
	EXPECT_EQ(0, memcmp(output.start(), data, 3));
}

TEST(DecodeBase64, DecodeFailsIfBufferToSmall)
{
	const auto txt = CreateStringView("QUJDQUJD");

	InplaceBufferOutputStream<uint8_t, 4> output;
	EXPECT_FALSE(txt.decode(EncodingType::Base64, output));
}

TEST(DecodeBase64, DecodeOneByteWorksWithSmallBuffer)
{
	const auto txt = CreateStringView("QQ==");

	InplaceBufferOutputStream<uint8_t, 1> output;
	EXPECT_TRUE(txt.decode(EncodingType::Base64, output));
	EXPECT_EQ(1, output.size());
}

TEST(DecodeBase64, DecodeTwoBytesWorksWithSmallBuffer)
{
	const auto txt = CreateStringView("QUI=");

	InplaceBufferOutputStream<uint8_t, 2> output;
	EXPECT_TRUE(txt.decode(EncodingType::Base64, output));
	EXPECT_EQ(2, output.size());
}

TEST(DecodeBase64, DecodeBlockWorksWithExactFitBuffer)
{
	const auto txt = CreateStringView("QUJD");

	InplaceBufferOutputStream<uint8_t, 3> output;
	EXPECT_TRUE(txt.decode(EncodingType::Base64, output));
	EXPECT_EQ(3, output.size());
}

TEST(DecodeBase64, DecodeFailsOnInvalidChar)
{
	const auto txt = CreateStringView("QU:JD");

	InplaceBufferOutputStream<uint8_t, 256> output;
	EXPECT_FALSE(txt.decode(EncodingType::Hex, output));
}

TEST(DecodeBase64, DecodeFailsOnWhiteSpaceIfIngoringNotEnabled)
{
	const auto txt = CreateStringView("QU JD");

	InplaceBufferOutputStream<uint8_t, 256> output;
	EXPECT_FALSE(txt.decode(EncodingType::Base64, output, false));
}

TEST(DecodeBase64, DecodeWorksWithWhiteSpaceIfIngoringEnabled)
{
	const auto txt = CreateStringView("QU JD");

	InplaceBufferOutputStream<uint8_t, 256> output;
	EXPECT_TRUE(txt.decode(EncodingType::Base64, output, true));
	EXPECT_EQ(3, output.size());
}

//--

END_INFERNO_NAMESPACE()