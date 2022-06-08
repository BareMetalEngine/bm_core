/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "bm/core/system/include/output.h"

BEGIN_INFERNO_NAMESPACE()

//---

class StdPrinter : public IFormatStream
{
public:
	StdPrinter()
	{
		m_text.push_back(0);
	}

	virtual IFormatStream& append(const char* str, uint32_t len = INDEX_MAX) override final
	{
		if (len == INDEX_MAX)
			len = strlen(str);

		m_text.pop_back();
		
		while (len--)
			m_text.push_back(*str++);

		m_text.push_back(0);
		return *this;
	}

	inline const char* c_str() const
	{
		return m_text.data();
	}

private:
	std::vector<char> m_text;
};

//---

TEST(Format, TextPrints)
{
	StdPrinter printer;
	printer << "Text";
	EXPECT_STREQ(printer.c_str(), "Text");
}

TEST(Format, TextViewPrints)
{
	StdPrinter printer;
	printer.append("Ala", 3);
	printer.append("ma", 2);
	printer.append("kota", 4);
	EXPECT_STREQ(printer.c_str(), "Alamakota");
}

TEST(Format, FormatPrintEmpty)
{
	StdPrinter printer;
	printer.appendf("");
	EXPECT_STREQ(printer.c_str(), "");
}

TEST(Format, FormatPrintPassThrough)
{
	StdPrinter printer;
	printer.appendf("test");
	EXPECT_STREQ(printer.c_str(), "test");
}

TEST(Format, FormatPrintSingleBracketDoesNotTriggerFormatting)
{
	StdPrinter printer;
	printer.appendf("test{");
	EXPECT_STREQ(printer.c_str(), "test{");
}

TEST(Format, FormatPrintDoubleBrackedWithSpaceDoesNotTriggerFormatting)
{
	StdPrinter printer;
	printer.appendf("test{ }");
	EXPECT_STREQ(printer.c_str(), "test{ }");
}

TEST(Format, FormatPrintIncompleteFormatBracketIgnored)
{
	StdPrinter printer;
	printer.appendf("test{<>test");
	EXPECT_STREQ(printer.c_str(), "test{<>test");
}

TEST(Format, FormatPrintEmptyFormat)
{
	StdPrinter printer;
	printer.appendf("{}");
	EXPECT_STREQ(printer.c_str(), "<undefined>");
}

TEST(Format, FormatPrintProperGluing)
{
	StdPrinter printer;
	printer.appendf("abc{}test");
	EXPECT_STREQ(printer.c_str(), "abc<undefined>test");
}

TEST(Format, FormatPrintCStringLiteral)
{
	StdPrinter printer;
	printer.appendf("abc_{}_test", "CString");
	EXPECT_STREQ(printer.c_str(), "abc_CString_test");
}

TEST(Format, FormatPrintCStringPointer)
{
	StdPrinter printer;
	const char* str = "CPtr";
	printer.appendf("abc_{}_test", str);
	EXPECT_STREQ(printer.c_str(), "abc_CPtr_test");
}

TEST(Format, FormatPrintWStringLiteral)
{
	StdPrinter printer;
	printer.appendf("abc_{}_test", L"CWideString");
	EXPECT_STREQ(printer.c_str(), "abc_CWideString_test");
}

TEST(Format, FormatPrintWStringPointer)
{
	StdPrinter printer;
	const wchar_t* str = L"WPtr";
	printer.appendf("abc_{}_test", str);
	EXPECT_STREQ(printer.c_str(), "abc_WPtr_test");
}

TEST(Format, FormatPrintSimpleNumber)
{
	StdPrinter printer;
	printer.appendf("x={};", 42);
	EXPECT_STREQ(printer.c_str(), "x=42;");
}

TEST(Format, FormatPrintSimpleNumberNegative)
{
	StdPrinter printer;
	printer.appendf("x={};", -42);
	EXPECT_STREQ(printer.c_str(), "x=-42;");
}

TEST(Format, FormatPrintSimpleNumberZero)
{
	StdPrinter printer;
	printer.appendf("x={};", 0);
	EXPECT_STREQ(printer.c_str(), "x=0;");
}

TEST(Format, FormatPrintSimpleNumberPaddingWithSpace)
{
	StdPrinter printer;
	printer.appendf("x={};", PaddedInteger<4, ' '>(42));
	EXPECT_STREQ(printer.c_str(), "x=  42;");
}

TEST(Format, FormatPrintSimpleNumberPaddingWithZero)
{
	StdPrinter printer;
	printer.appendf("x={};", PaddedInteger<4, '0'>(42));
	EXPECT_STREQ(printer.c_str(), "x=0042;");
}

TEST(Format, FormatPrintNegativeNumberPaddingWithSpace)
{
	StdPrinter printer;
	printer.appendf("x={};", PaddedInteger<4, ' '>(-42));
	EXPECT_STREQ(printer.c_str(), "x= -42;");
}

TEST(Format, FormatPrintNegativeNumberPaddingWithZero)
{
	StdPrinter printer;
	printer.appendf("x={};", PaddedInteger<4, '0'>(-42));
	EXPECT_STREQ(printer.c_str(), "x=-042;");
}

TEST(Format, FormatPrintZeroPaddingWithSpace)
{
	StdPrinter printer;
	printer.appendf("x={};", PaddedInteger<4, ' '>(0));
	EXPECT_STREQ(printer.c_str(), "x=   0;");
}

TEST(Format, FormatPrintZeroPaddingWithZero)
{
	StdPrinter printer;
	printer.appendf("x={};", PaddedInteger<4, '0'>(0));
	EXPECT_STREQ(printer.c_str(), "x=0000;");
}

TEST(Format, FormatPrintSimpleNumberPaddingWithSpacePaddingToShort)
{
	StdPrinter printer;
	printer.appendf("x={};", PaddedInteger<4, ' '>(12345678));
	EXPECT_STREQ(printer.c_str(), "x=12345678;");
}

TEST(Format, FormatPrintSimpleNumberPaddingWithZeroPaddingToShort)
{
	StdPrinter printer;
	printer.appendf("x={};", PaddedInteger<4, ' '>(-12345678));
	EXPECT_STREQ(printer.c_str(), "x=-12345678;");
}

TEST(Format, FormatFloatingPointIntegerHasNoFractionalPoint)
{
	StdPrinter printer;
	printer.appendf("x={};", 1.0f);
	EXPECT_STREQ(printer.c_str(), "x=1;");
}

TEST(Format, FormatFloatingPointSimpleFractionsAreShort)
{
	StdPrinter printer;
	printer.appendf("x={};", 0.25f);
	EXPECT_STREQ(printer.c_str(), "x=0.25;");
}

TEST(Format, FormatFloatingPointFullInfiniteExpansionUsesSizeFractionalDigits)
{
	StdPrinter printer;
	printer.appendf("x={};", 1.0f / 3.0f);
	EXPECT_STREQ(printer.c_str(), "x=0.333333;");
}

TEST(Format, FormatFloatingPointLimitedPrecission)
{
	StdPrinter printer;
	printer.appendf("x={};", PaddedFloat<1,3>(1.0f / 3.0f));
	EXPECT_STREQ(printer.c_str(), "x=0.333;");
}

TEST(Format, FormatFloatingPointLimitedPrecissionZero)
{
	StdPrinter printer;
	printer.appendf("x={};", PaddedFloat<1, 0>(1.0f / 3.0f));
	EXPECT_STREQ(printer.c_str(), "x=0;");
}

TEST(Format, FormatFloatingPointIntegerPadding)
{
	StdPrinter printer;
	printer.appendf("x={};", PaddedFloat<5,1,' '>(123));
	EXPECT_STREQ(printer.c_str(), "x=  123.0;");
}

TEST(Format, FormatFloatingPointIntegerPaddingWithZero)
{
	StdPrinter printer;
	printer.appendf("x={};", PaddedFloat<5,1,'0'>(123));
	EXPECT_STREQ(printer.c_str(), "x=00123.0;");
}

TEST(Format, FormatFloatingPointIntegerPaddingNegative)
{
	StdPrinter printer;
	printer.appendf("x={};", PaddedFloat<5,1,' '>(-123));
	EXPECT_STREQ(printer.c_str(), "x= -123.0;");
}

TEST(Format, FormatFloatingPointIntegerPaddingWithZeroNegative)
{
	StdPrinter printer;
	printer.appendf("x={};", PaddedFloat<5, 1, '0'>(-123));
	EXPECT_STREQ(printer.c_str(), "x=-0123.0;");
}

//---

TEST(Format, TimeIntervalPrintUs)
{
	StdPrinter printer;
	printer.appendf("x={};", TimeInterval(0.025f));
	EXPECT_STREQ(printer.c_str(), "x=25.00 ms;");
}

TEST(Format, TimeIntervalPrintMs)
{
	StdPrinter printer;
	printer.appendf("x={};", TimeInterval(0.25f));
	EXPECT_STREQ(printer.c_str(), "x=250.00 ms;");
}

TEST(Format, TimeIntervalPrintShort)
{
	StdPrinter printer;
	printer.appendf("x={};", TimeInterval(1.32f));
	EXPECT_STREQ(printer.c_str(), "x=1.32 s;");
}

TEST(Format, TimeIntervalPrintS)
{
	StdPrinter printer;
	printer.appendf("x={};", TimeInterval(25.0f));
	EXPECT_STREQ(printer.c_str(), "x=25.00 s;");
}

TEST(Format, TimeIntervalPrintHours)
{
	StdPrinter printer;
	printer.appendf("x={};", TimeInterval(3600.0f + 900.0f + 10.0f + 0.25f));
	EXPECT_STREQ(printer.c_str(), "x=01:15:10;");
}

//---

END_INFERNO_NAMESPACE()