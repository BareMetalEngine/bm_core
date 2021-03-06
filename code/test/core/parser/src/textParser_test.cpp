/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "bm/core/parser/include/textParser.h"
//#include "bm/core/parser/include/textParsingTreeBuilder.h"
#include "bm/core/parser/include/textSimpleLanguageDefinition.h"


BEGIN_INFERNO_NAMESPACE()

UniquePtr<ITextLanguageDefinition> BuildLanguage()
{
    SimpleLanguageDefinitionBuilder defs;
    defs.enableIntegerNumbers(true);
    defs.enableFloatNumbers(true);
    defs.enableHexadecimalNumbers(true);
    defs.enableStrings(true);
    defs.enableNames(true);

    int keyword = 300;
    defs.addKeyword("import", keyword++);
    defs.addKeyword("importonly", keyword++);
    defs.addKeyword("abstract", keyword++);
    defs.addKeyword("const", keyword++);
    defs.addKeyword("extends", keyword++);
    defs.addKeyword("in", keyword++);
    defs.addKeyword("class", keyword++);
    defs.addKeyword("enum", keyword++);
    defs.addKeyword("struct", keyword++);
    defs.addKeyword("function", 400);
    defs.addKeyword("def", keyword++);
    defs.addKeyword("editable", keyword++);
    defs.addKeyword("replicated", keyword++);
    defs.addKeyword("final", keyword++);
    defs.addKeyword("virtual", keyword++);
    defs.addKeyword("override", keyword++);
    defs.addKeyword("out", keyword++);
    defs.addKeyword("optional", keyword++);
    defs.addKeyword("skip", keyword++);
    defs.addKeyword("local", keyword++);
    defs.addKeyword("inlined", keyword++);
    defs.addKeyword("private", keyword++);
    defs.addKeyword("protected", keyword++);
    defs.addKeyword("public", keyword++);
    defs.addKeyword("event", keyword++);
    defs.addKeyword("timer", keyword++);
    defs.addKeyword("array", keyword++);
    defs.addKeyword("hint", keyword++);
    defs.addKeyword("true", keyword++);
    defs.addKeyword("false", keyword++);
    defs.addKeyword("NULL", keyword++);
    defs.addKeyword("var", keyword++);
    defs.addKeyword("exec", keyword++);
    defs.addKeyword("saved", keyword++);
    defs.addKeyword("weak", keyword++);
    defs.addKeyword("operator", keyword++);
    defs.addKeyword("cast", keyword++);
    defs.addKeyword("implicit", keyword++);
    defs.addKeyword("static", keyword++);
    defs.addKeyword("multicast", keyword++);
    defs.addKeyword("server", keyword++);
    defs.addKeyword("client", keyword++);
    defs.addKeyword("reliable", keyword++);
    defs.addKeyword("undecorated", keyword++);
    defs.addKeyword("new", 402);
    defs.addKeyword("delete", keyword++);
    defs.addKeyword("if", keyword++);
    defs.addKeyword("else", keyword++);
    defs.addKeyword("switch", keyword++);
    defs.addKeyword("case", keyword++);
    defs.addKeyword("default", keyword++);
    defs.addKeyword("for", keyword++);
    defs.addKeyword("while", keyword++);
    defs.addKeyword("do", keyword++);
    defs.addKeyword("return", 401);
    defs.addKeyword("break", keyword++);
    defs.addKeyword("continue", keyword++);
    defs.addKeyword("this", keyword++);
    defs.addKeyword("super", keyword++);
    defs.addKeyword("+=", keyword++);
    defs.addKeyword("-=", keyword++);
    defs.addKeyword("*=", keyword++);
    defs.addKeyword("/=", keyword++);
    defs.addKeyword("&=", keyword++);
    defs.addKeyword("|=", keyword++);
    defs.addKeyword("||", keyword++);
    defs.addKeyword("&&", keyword++);
    defs.addKeyword("!=", keyword++);
    defs.addKeyword("==", keyword++);
    defs.addKeyword(">=", keyword++);
    defs.addKeyword("<=", keyword++);

    defs.addChar('(');
    defs.addChar(')');
    defs.addChar('{');
    defs.addChar('}');
    defs.addChar('[');
    defs.addChar(']');
    defs.addChar('<');
    defs.addChar('>');
    defs.addChar(',');
    defs.addChar('=');
    defs.addChar(';');
    defs.addChar('&');
    defs.addChar('|');
    defs.addChar('^');
    defs.addChar('+');
    defs.addChar('-');
    defs.addChar('*');
    defs.addChar('/');
    defs.addChar('%');
    defs.addChar('.');
    defs.addChar('!');
    defs.addChar('?');
    defs.addChar(':');
    defs.addChar('~');

    return defs.buildLanguageDefinition();
}

#if 0    // test skipped because parser no longer glues signs with numbers
TEST(ComplexParser, SignsGluedToNumbers)
{

    GTEST_SKIP();

    auto lang = BuildLanguage();

    TextParser p;
    auto str = "- +     1 -1 +1     -.1 +.1 .1     -0.1 +0.1 0.1     -1e10 +1e10 1e10     -0.e1 +0.e1 0.e1";
    p.reset(str);

	Token token;

	{
		auto t = p.parseToken(*lang);
		ASSERT_TRUE(t.isChar());
        ASSERT_EQ('-', t.ch());
	}

	{
		auto t = p.parseToken(*lang);
		ASSERT_TRUE(t.isChar());
		ASSERT_EQ('+', t.ch());
	}

    //--

	{
		auto t = p.parseToken(*lang);
		ASSERT_TRUE(t.isInteger());
        ASSERT_EQ(1, t.integerNumber());
	}

	{
		auto t = p.parseToken(*lang);
		ASSERT_TRUE(t.isInteger());
		ASSERT_EQ(-1, t.integerNumber());
	}

	{
		auto t = p.parseToken(*lang);
		ASSERT_TRUE(t.isInteger());
		ASSERT_EQ(1, t.integerNumber());
	}

    //--

	{
		auto t = p.parseToken(*lang);
		ASSERT_TRUE(t.isFloat());
		ASSERT_FLOAT_EQ(-0.1f, t.floatNumber());
	}

	{
		auto t = p.parseToken(*lang);
		ASSERT_TRUE(t.isFloat());
		ASSERT_FLOAT_EQ(0.1f, t.floatNumber());
	}

	{
		auto t = p.parseToken(*lang);
		ASSERT_TRUE(t.isFloat());
		ASSERT_FLOAT_EQ(0.1f, t.floatNumber());
	}

	//--

	{
		auto t = p.parseToken(*lang);
		ASSERT_TRUE(t.isFloat());
		ASSERT_FLOAT_EQ(-0.1f, t.floatNumber());
	}

	{
		auto t = p.parseToken(*lang);
		ASSERT_TRUE(t.isFloat());
		ASSERT_FLOAT_EQ(0.1f, t.floatNumber());
	}

	{
		auto t = p.parseToken(*lang);
		ASSERT_TRUE(t.isFloat());
		ASSERT_FLOAT_EQ(0.1f, t.floatNumber());
	}

	//--

	{
		auto t = p.parseToken(*lang);
		ASSERT_TRUE(t.isFloat());
		ASSERT_FLOAT_EQ(-1e10f, t.floatNumber());
	}

	{
		auto t = p.parseToken(*lang);
		ASSERT_TRUE(t.isFloat());
		ASSERT_FLOAT_EQ(1e10f, t.floatNumber());
	}

	{
		auto t = p.parseToken(*lang);
		ASSERT_TRUE(t.isFloat());
		ASSERT_FLOAT_EQ(1e10, t.floatNumber());
	}

	//--

	{
		auto t = p.parseToken(*lang);
		ASSERT_TRUE(t.isFloat());
		ASSERT_FLOAT_EQ(-0.e1f, t.floatNumber());
	}

	{
		auto t = p.parseToken(*lang);
		ASSERT_TRUE(t.isFloat());
		ASSERT_FLOAT_EQ(0.e1f, t.floatNumber());
	}

	{
		auto t = p.parseToken(*lang);
		ASSERT_TRUE(t.isFloat());
		ASSERT_FLOAT_EQ(0.e1f, t.floatNumber());
	}
}
#endif

TEST(ComplexParser, FloatingPoint)
{
    auto lang = BuildLanguage();

    TextParser p;
    auto str = "1 1. .1 1.f .1f 0.1f 0.1 .5 5. 1e10 1E10 1.e2 .2e10 2E-20";
    p.reset(str);

    Token token;

    {
        auto t = p.parseToken(*lang);
        ASSERT_TRUE(t.isInteger());
        ASSERT_EQ(1, t.integerNumber());
    }

    {
        auto t = p.parseToken(*lang);
        ASSERT_TRUE(t.isFloat());
        ASSERT_FLOAT_EQ(1., t.floatNumber());
    }

    {
        auto t = p.parseToken(*lang);
        ASSERT_TRUE(t.isFloat());
        ASSERT_FLOAT_EQ(.1f, t.floatNumber());
    }

    {
        auto t = p.parseToken(*lang);
        ASSERT_TRUE(t.isFloat());
        ASSERT_FLOAT_EQ(1.f, t.floatNumber());
    }

    {
        auto t = p.parseToken(*lang);
        ASSERT_TRUE(t.isFloat());
        ASSERT_FLOAT_EQ(.1f, t.floatNumber());
    }

    {
        auto t = p.parseToken(*lang);
        ASSERT_TRUE(t.isFloat());
        ASSERT_FLOAT_EQ(0.1f, t.floatNumber());
    }

    {
        auto t = p.parseToken(*lang);
        ASSERT_TRUE(t.isFloat());
        ASSERT_FLOAT_EQ(0.1f, t.floatNumber());
    }

    {
        auto t = p.parseToken(*lang);
        ASSERT_TRUE(t.isFloat());
        ASSERT_FLOAT_EQ(.5, t.floatNumber());
    }

    {
        auto t = p.parseToken(*lang);
        ASSERT_TRUE(t.isFloat());
        ASSERT_FLOAT_EQ(5.f, t.floatNumber());
    }

    // 1e10 1E10 1.e2 .2e10 2E-20";

    {
        auto t = p.parseToken(*lang);
        ASSERT_TRUE(t.isFloat());
        ASSERT_FLOAT_EQ(1e10f, t.floatNumber());
    }

    {
        auto t = p.parseToken(*lang);
        ASSERT_TRUE(t.isFloat());
        ASSERT_FLOAT_EQ(1E10f, t.floatNumber());
    }

    {
        auto t = p.parseToken(*lang);
        ASSERT_TRUE(t.isFloat());
        ASSERT_FLOAT_EQ(1.e2f, t.floatNumber());
    }

    {
        auto t = p.parseToken(*lang);
        ASSERT_TRUE(t.isFloat());
        ASSERT_FLOAT_EQ(.2e10f, t.floatNumber());
    }

    {
        auto t = p.parseToken(*lang);
        ASSERT_TRUE(t.isFloat());
        ASSERT_FLOAT_EQ(2e-20f, t.floatNumber());
    }
}

TEST(ComplexParser, Unsigned)
{
    auto lang = BuildLanguage();

    TextParser p;
    auto str = "1 42U";
    p.reset(str);

    Token token;

    {
        auto t = p.parseToken(*lang);
        ASSERT_TRUE(t.isInteger());
        ASSERT_EQ(1, t.integerNumber());
    }

    {
        auto t = p.parseToken(*lang);
        ASSERT_TRUE(t.isUnsigned());
        ASSERT_EQ(42, t.unsignedNumber());
    }
}

TEST(ComplexParser, Hex)
{
    auto lang = BuildLanguage();

    TextParser p;
    auto str = "0xABCD1020";
    p.reset(str);

    Token token;

    {
        auto t = p.parseToken(*lang);
        ASSERT_TRUE(t.isHex());
        ASSERT_EQ(0xABCD1020, t.hexToNumber());
    }
}


TEST(ComplexParser, FullTest)
{
    auto lang = BuildLanguage();

    TextParser p;
    auto str  = "void function Test() { return new Dupa(123, \"Test\", 'Ident', 3.14); }";
    p.reset(str);

    Token token;

    {
        auto t = p.parseToken(*lang);
        ASSERT_TRUE(t.isIdentifier());
        ASSERT_EQ(StringBuf("void"), t.string());
    }

    {
        auto t = p.parseToken(*lang);
        ASSERT_TRUE(t.isKeyword());
        ASSERT_EQ(StringBuf("function"), t.string());
        ASSERT_EQ(400, t.keywordID());
    }

    {
        auto t = p.parseToken(*lang);
        ASSERT_TRUE(t.isIdentifier());
        ASSERT_EQ(StringBuf("Test"), t.string());
    }

    {
        auto t = p.parseToken(*lang);
        ASSERT_TRUE(t.isChar());
        ASSERT_EQ(StringBuf("("), t.string());
        ASSERT_EQ('(', t.ch());
    }

    {
        auto t = p.parseToken(*lang);
        ASSERT_TRUE(t.isChar());
        ASSERT_EQ(StringBuf(")"), t.string());
        ASSERT_EQ(')', t.ch());
    }

    {
        auto t = p.parseToken(*lang);
        ASSERT_TRUE(t.isChar());
        ASSERT_EQ(StringBuf("{"), t.string());
        ASSERT_EQ('{', t.ch());
    }

    {
        auto t = p.parseToken(*lang);
        ASSERT_TRUE(t.isKeyword());
        ASSERT_EQ(StringBuf("return"), t.string());
        ASSERT_EQ(401, t.keywordID());
    }

    {
        auto t = p.parseToken(*lang);
        ASSERT_TRUE(t.isKeyword());
        ASSERT_EQ(StringBuf("new"), t.string());
        ASSERT_EQ(402, t.keywordID());
    }

    {
        auto t = p.parseToken(*lang);
        ASSERT_TRUE(t.isIdentifier());
        ASSERT_EQ(StringBuf("Dupa"), t.string());
    }

    {
        auto t = p.parseToken(*lang);
        ASSERT_TRUE(t.isChar());
        ASSERT_EQ(StringBuf("("), t.string());
        ASSERT_EQ('(', t.ch());
    }

    {
        auto t = p.parseToken(*lang);
        ASSERT_TRUE(t.isInteger());
        ASSERT_EQ(123, t.integerNumber());
    }

    {
        auto t = p.parseToken(*lang);
        ASSERT_TRUE(t.isChar());
        ASSERT_EQ(StringBuf(","), t.string());
        ASSERT_EQ(',', t.ch());
    }

    {
        auto t = p.parseToken(*lang);
        ASSERT_TRUE(t.isString());
        ASSERT_EQ(StringBuf("Test"), t.string());
    }

    {
        auto t = p.parseToken(*lang);
        ASSERT_TRUE(t.isChar());
        ASSERT_EQ(StringBuf(","), t.string());
        ASSERT_EQ(',', t.ch());
    }

    {
        auto t = p.parseToken(*lang);
        ASSERT_TRUE(t.isName());
        ASSERT_EQ(StringBuf("Ident"), t.string());
    }

    {
        auto t = p.parseToken(*lang);
        ASSERT_TRUE(t.isChar());
        ASSERT_EQ(StringBuf(","), t.string());
        ASSERT_EQ(',', t.ch());
    }

    {
        auto t = p.parseToken(*lang);
        ASSERT_TRUE(t.isFloat());
        ASSERT_TRUE(abs(3.14 - t.floatNumber()) < 0.001);
    }

    {
        auto t = p.parseToken(*lang);
        ASSERT_TRUE(t.isChar());
        ASSERT_EQ(StringBuf(")"), t.string());
        ASSERT_EQ(')', t.ch());
    }

    {
        auto t = p.parseToken(*lang);
        ASSERT_TRUE(t.isChar());
        ASSERT_EQ(StringBuf(";"), t.string());
        ASSERT_EQ(';', t.ch());
    }

    {
        auto t = p.parseToken(*lang);
        ASSERT_TRUE(t.isChar());
        ASSERT_EQ(StringBuf("}"), t.string());
        ASSERT_EQ('}', t.ch());
    }
}

END_INFERNO_NAMESPACE()