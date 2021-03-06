/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "bm/core/parser/include/textFilePreprocessor.h"

BEGIN_INFERNO_NAMESPACE()

extern UniquePtr<ITextLanguageDefinition> BuildLanguage();

static ITextLanguageDefinition& GetTestLanguage()
{
    static UniquePtr<ITextLanguageDefinition> ret = BuildLanguage();
    return *ret;
}

TEST(Preprocessor, SimpleTokensPassthrough)
{
    const char* code = "ident class 1.05f ## - \"string\" 'name'";

    LocalAllocator allocator;
    TextFilePreprocessor test(allocator, ITextIncludeHandler::GetEmptyHandler(), ITextErrorReporter::GetDefault(), ITextCommentEater::StandardComments(), GetTestLanguage());
    ASSERT_TRUE(test.processContent(code, "TestFile"));

    ASSERT_FALSE(test.tokens().empty());

    {
        auto t  = test.tokens().popFront();
        ASSERT_TRUE(t);
        ASSERT_TRUE(t->isIdentifier());
        ASSERT_TRUE(t->view() == "ident");
        ASSERT_EQ(StringBuf("TestFile"), t->location().contextName());
    }

    {
        auto t  = test.tokens().popFront();
        ASSERT_TRUE(t);
        ASSERT_TRUE(t->isKeyword());
        ASSERT_TRUE(t->view() == "class");
        ASSERT_EQ(StringBuf("TestFile"), t->location().contextName());
    }

    {
        auto t  = test.tokens().popFront();
        ASSERT_TRUE(t);
        ASSERT_TRUE(t->isFloat());
        ASSERT_FLOAT_EQ(1.05f, t->floatNumber());
        ASSERT_EQ(StringBuf("TestFile"), t->location().contextName());
    }

    {
        auto t  = test.tokens().popFront();
        ASSERT_TRUE(t);
        ASSERT_TRUE(t->isKeyword());
        ASSERT_TRUE(t->view() == "##");
        ASSERT_EQ(StringBuf("TestFile"), t->location().contextName());
    }

    {
        auto t  = test.tokens().popFront();
        ASSERT_TRUE(t);
        ASSERT_TRUE(t->isChar());
        ASSERT_TRUE(t->view() == "-");
        ASSERT_EQ(StringBuf("TestFile"), t->location().contextName());
    }

    {
        auto t  = test.tokens().popFront();
        ASSERT_TRUE(t);
        ASSERT_TRUE(t->isString());
        ASSERT_TRUE(t->view() == "string");
        ASSERT_EQ(StringBuf("TestFile"), t->location().contextName());
    }

    {
        auto t  = test.tokens().popFront();
        ASSERT_TRUE(t);
        ASSERT_TRUE(t->isName());
        ASSERT_TRUE(t->view() == "name");
        ASSERT_EQ(StringBuf("TestFile"), t->location().contextName());
    }

    {
        auto t  = test.tokens().popFront();
        ASSERT_TRUE(!t);
    }
}

TEST(Preprocessor, PragmaEaten)
{
    const char* code = "line\n#pragma test\nline";

    LocalAllocator allocator;
    TextFilePreprocessor test(allocator, ITextIncludeHandler::GetEmptyHandler(), ITextErrorReporter::GetDefault(), ITextCommentEater::StandardComments(), GetTestLanguage());
    ASSERT_TRUE(test.processContent(code, "TestFile"));

    ASSERT_FALSE(test.tokens().empty());

    {
        auto t  = test.tokens().popFront();
        ASSERT_TRUE(t);
        ASSERT_TRUE(t->isIdentifier());
        ASSERT_TRUE(t->view() == "line");
        ASSERT_EQ(1, t->location().line());
    }

    {
        auto t  = test.tokens().popFront();
        ASSERT_TRUE(t);
        ASSERT_TRUE(t->isIdentifier());
        ASSERT_TRUE(t->view() == "line");
        ASSERT_EQ(3, t->location().line());
    }
}


TEST(Preprocessor, SimpleReplacement)
{
    const char* code = "X";

    LocalAllocator allocator;
    TextFilePreprocessor test(allocator, ITextIncludeHandler::GetEmptyHandler(), ITextErrorReporter::GetDefault(), ITextCommentEater::StandardComments(), GetTestLanguage());
    test.defineSymbol("X", "test");
    ASSERT_TRUE(test.processContent(code, "TestFile"));

    ASSERT_FALSE(test.tokens().empty());

    {
        auto t  = test.tokens().popFront();
        ASSERT_TRUE(t);
        ASSERT_TRUE(t->isIdentifier());
        ASSERT_TRUE(t->view() == "test");
        ASSERT_EQ(1, t->location().line());
    }
}

TEST(Preprocessor, ReplacementFromDefine)
{
    const char* code = "#define X test\nX";

    LocalAllocator allocator;
    TextFilePreprocessor test(allocator, ITextIncludeHandler::GetEmptyHandler(), ITextErrorReporter::GetDefault(), ITextCommentEater::StandardComments(), GetTestLanguage());
    ASSERT_TRUE(test.processContent(code, "TestFile"));

    ASSERT_FALSE(test.tokens().empty());

    {
        auto t  = test.tokens().popFront();
        ASSERT_TRUE(t);
        ASSERT_TRUE(t->isIdentifier());
        ASSERT_TRUE(t->view() == "test");
        ASSERT_EQ(2, t->location().line());
    }
}

class HelperErrorReporter : public ITextErrorReporter
{
public:
    uint32_t m_numErrors = 0;
    uint32_t m_numWarnings = 0;

    virtual void reportError(const TextTokenLocation& loc, StringView message) override final
    {
        TRACE_ERROR("{}: {}", loc, message);
        m_numErrors += 1;
    }

    virtual void reportWarning(const TextTokenLocation& loc, StringView message) override final
    {
        TRACE_WARNING("{}: {}", loc, message);
        m_numWarnings += 1;
    }

};

TEST(Preprocessor, DefinieRedefinitionWarns)
{
    const char* code = "#define X test\n#define X dupa\nX";

    LocalAllocator allocator;
    HelperErrorReporter errorReporter;
    TextFilePreprocessor test(allocator, ITextIncludeHandler::GetEmptyHandler(), errorReporter, ITextCommentEater::StandardComments(), GetTestLanguage());
    ASSERT_TRUE(test.processContent(code, "TestFile"));
    EXPECT_EQ(1, errorReporter.m_numWarnings);

    ASSERT_FALSE(test.tokens().empty());

    {
        auto t  = test.tokens().popFront();
        ASSERT_TRUE(t);
        ASSERT_TRUE(t->isIdentifier());
        ASSERT_TRUE(t->view() == "dupa");
        ASSERT_EQ(3, t->location().line());
    }
}

TEST(Preprocessor, UndefOfUnknownMacroWarns)
{
    const char* code = "#undef Y";

    LocalAllocator allocator;
    HelperErrorReporter errorReporter;
    TextFilePreprocessor test(allocator, ITextIncludeHandler::GetEmptyHandler(), errorReporter, ITextCommentEater::StandardComments(), GetTestLanguage());
    ASSERT_TRUE(test.processContent(code, "TestFile"));
    EXPECT_EQ(1, errorReporter.m_numWarnings);
}

TEST(Preprocessor, UndefOfGlobalMacro)
{
    const char* code = "X\n#undef X\nX";

    LocalAllocator allocator;
    HelperErrorReporter errorReporter;
    TextFilePreprocessor test(allocator, ITextIncludeHandler::GetEmptyHandler(), errorReporter, ITextCommentEater::StandardComments(), GetTestLanguage());
    test.defineSymbol("X", "test");
    ASSERT_TRUE(test.processContent(code, "TestFile"));

    ASSERT_FALSE(test.tokens().empty());

    {
        auto t  = test.tokens().popFront();
        ASSERT_TRUE(t);
        ASSERT_TRUE(t->isIdentifier());
        ASSERT_TRUE(t->view() == "test");
        ASSERT_EQ(1, t->location().line());
    }

    {
        auto t  = test.tokens().popFront();
        ASSERT_TRUE(t);
        ASSERT_TRUE(t->isIdentifier());
        ASSERT_TRUE(t->view() == "X");
        ASSERT_EQ(3, t->location().line());
    }
}

TEST(Preprocessor, NoArgPassing)
{
    const char* code = "#define X() test\nX()";

    LocalAllocator allocator;
    HelperErrorReporter errorReporter;
    TextFilePreprocessor test(allocator, ITextIncludeHandler::GetEmptyHandler(), errorReporter, ITextCommentEater::StandardComments(), GetTestLanguage());
    ASSERT_TRUE(test.processContent(code, "TestFile"));

    ASSERT_FALSE(test.tokens().empty());

    {
        auto t  = test.tokens().popFront();
        ASSERT_TRUE(t);
        ASSERT_TRUE(t->isIdentifier());
        ASSERT_TRUE(t->view() == "test");
        ASSERT_EQ(2, t->location().line());
    }
}

TEST(Preprocessor, EmptyArgPassing)
{
    const char* code = "#define X(a) a\nX()";

    LocalAllocator allocator;
    HelperErrorReporter errorReporter;
    TextFilePreprocessor test(allocator, ITextIncludeHandler::GetEmptyHandler(), errorReporter, ITextCommentEater::StandardComments(), GetTestLanguage());
    ASSERT_TRUE(test.processContent(code, "TestFile"));

    ASSERT_TRUE(test.tokens().empty());
}

TEST(Preprocessor, SingleArgPassing)
{
    const char* code = "#define X(a) a\nX(test)\n";

    LocalAllocator allocator;
    HelperErrorReporter errorReporter;
    TextFilePreprocessor test(allocator, ITextIncludeHandler::GetEmptyHandler(), errorReporter, ITextCommentEater::StandardComments(), GetTestLanguage());
    ASSERT_TRUE(test.processContent(code, "TestFile"));

    ASSERT_FALSE(test.tokens().empty());

    {
        auto t  = test.tokens().popFront();
        ASSERT_TRUE(t);
        ASSERT_TRUE(t->isIdentifier());
        ASSERT_TRUE(t->view() == "test");
        ASSERT_EQ(2, t->location().line());
    }
}

TEST(Preprocessor, SignleArgGluedTokensBoth)
{
    const char* code = "#define X(a) a2a\nX(test)\n";

    LocalAllocator allocator;
    HelperErrorReporter errorReporter;
    TextFilePreprocessor test(allocator, ITextIncludeHandler::GetEmptyHandler(), errorReporter, ITextCommentEater::StandardComments(), GetTestLanguage());
    ASSERT_TRUE(test.processContent(code, "TestFile"));

    ASSERT_FALSE(test.tokens().empty());

    {
        auto t  = test.tokens().popFront();
        ASSERT_TRUE(t);
        ASSERT_TRUE(t->isIdentifier());
        ASSERT_TRUE(t->view() == "a2a");
        ASSERT_EQ(2, t->location().line());
    }
}

TEST(Preprocessor, SignleArgGluedTokensLeft)
{
    const char* code = "#define X(a) a2 a\nX(test)\n";

    LocalAllocator allocator;
    HelperErrorReporter errorReporter;
    TextFilePreprocessor test(allocator, ITextIncludeHandler::GetEmptyHandler(), errorReporter, ITextCommentEater::StandardComments(), GetTestLanguage());
    ASSERT_TRUE(test.processContent(code, "TestFile"));

    ASSERT_FALSE(test.tokens().empty());

    {
        auto t  = test.tokens().popFront();
        ASSERT_TRUE(t);
        ASSERT_TRUE(t->isIdentifier());
        ASSERT_TRUE(t->view() == "a2");
        ASSERT_EQ(2, t->location().line());
    }

    {
        auto t  = test.tokens().popFront();
        ASSERT_TRUE(t);
        ASSERT_TRUE(t->isIdentifier());
        ASSERT_TRUE(t->view() == "test");
        ASSERT_EQ(2, t->location().line());
    }
}

TEST(Preprocessor, SignleArgGluedTokensRight)
{
    const char* code = "#define X(a) a 2a\nX(test)\n";

    LocalAllocator allocator;
    HelperErrorReporter errorReporter;
    TextFilePreprocessor test(allocator, ITextIncludeHandler::GetEmptyHandler(), errorReporter, ITextCommentEater::StandardComments(), GetTestLanguage());
    ASSERT_TRUE(test.processContent(code, "TestFile"));

    ASSERT_FALSE(test.tokens().empty());

    {
        auto t  = test.tokens().popFront();
        ASSERT_TRUE(t);
        ASSERT_TRUE(t->isIdentifier());
        ASSERT_TRUE(t->view() == "test");
        ASSERT_EQ(2, t->location().line());
    }

    {
        auto t  = test.tokens().popFront();
        ASSERT_TRUE(t);
        ASSERT_TRUE(t->isInteger());
        ASSERT_TRUE(t->view() == "2");
        ASSERT_EQ(2, t->location().line());
    }

    {
        auto t  = test.tokens().popFront();
        ASSERT_TRUE(t);
        ASSERT_TRUE(t->isIdentifier());
        ASSERT_TRUE(t->view() == "test");
        ASSERT_EQ(2, t->location().line());
    }
}


TEST(Preprocessor, SingleArgPassingTokenSpecifiedTwice)
{
    const char* code = "#define X(a) a 2 a\nX(test)\n";

    LocalAllocator allocator;
    HelperErrorReporter errorReporter;
    TextFilePreprocessor test(allocator, ITextIncludeHandler::GetEmptyHandler(), errorReporter, ITextCommentEater::StandardComments(), GetTestLanguage());
    ASSERT_TRUE(test.processContent(code, "TestFile"));

    ASSERT_FALSE(test.tokens().empty());

    {
        auto t  = test.tokens().popFront();
        ASSERT_TRUE(t);
        ASSERT_TRUE(t->isIdentifier());
        ASSERT_TRUE(t->view() == "test");
        ASSERT_EQ(2, t->location().line());
    }

    {
        auto t  = test.tokens().popFront();
        ASSERT_TRUE(t);
        ASSERT_TRUE(t->isInteger());
        ASSERT_TRUE(t->view() == "2");
        ASSERT_EQ(2, t->location().line());
    }

    {
        auto t  = test.tokens().popFront();
        ASSERT_TRUE(t);
        ASSERT_TRUE(t->isIdentifier());
        ASSERT_TRUE(t->view() == "test");
        ASSERT_EQ(2, t->location().line());
    }

}

TEST(Preprocessor, SingleArgPassingMultipleTokenArguments)
{
    const char* code = "#define X(a) a\nX(that is test)\n";

    LocalAllocator allocator;
    HelperErrorReporter errorReporter;
    TextFilePreprocessor test(allocator, ITextIncludeHandler::GetEmptyHandler(), errorReporter, ITextCommentEater::StandardComments(), GetTestLanguage());
    ASSERT_TRUE(test.processContent(code, "TestFile"));

    ASSERT_FALSE(test.tokens().empty());

    {
        auto t  = test.tokens().popFront();
        ASSERT_TRUE(t);
        ASSERT_TRUE(t->isIdentifier());
        ASSERT_TRUE(t->view() == "that");
        ASSERT_EQ(2, t->location().line());
    }

    {
        auto t  = test.tokens().popFront();
        ASSERT_TRUE(t);
        ASSERT_TRUE(t->isIdentifier());
        ASSERT_TRUE(t->view() == "is");
        ASSERT_EQ(2, t->location().line());
    }

    {
        auto t  = test.tokens().popFront();
        ASSERT_TRUE(t);
        ASSERT_TRUE(t->isIdentifier());
        ASSERT_TRUE(t->view() == "test");
        ASSERT_EQ(2, t->location().line());
    }

}

TEST(Preprocessor, MultiArgPassing)
{
    const char* code = "#define X(a,b) a b\nX(x,y)\n";

    LocalAllocator allocator;
    HelperErrorReporter errorReporter;
    TextFilePreprocessor test(allocator, ITextIncludeHandler::GetEmptyHandler(), errorReporter, ITextCommentEater::StandardComments(), GetTestLanguage());
    ASSERT_TRUE(test.processContent(code, "TestFile"));

    ASSERT_FALSE(test.tokens().empty());

    {
        auto t  = test.tokens().popFront();
        ASSERT_TRUE(t);
        ASSERT_TRUE(t->isIdentifier());
        ASSERT_TRUE(t->view() == "x");
        ASSERT_EQ(2, t->location().line());
    }

    {
        auto t  = test.tokens().popFront();
        ASSERT_TRUE(t);
        ASSERT_TRUE(t->isIdentifier());
        ASSERT_TRUE(t->view() == "y");
        ASSERT_EQ(2, t->location().line());
    }
}

TEST(Preprocessor, MultiEmptyArgs)
{
    const char* code = "#define X(a,b) a b\nX(,)\n";

    LocalAllocator allocator;
    HelperErrorReporter errorReporter;
    TextFilePreprocessor test(allocator, ITextIncludeHandler::GetEmptyHandler(), errorReporter, ITextCommentEater::StandardComments(), GetTestLanguage());
    ASSERT_TRUE(test.processContent(code, "TestFile"));

    ASSERT_TRUE(test.tokens().empty());
}

TEST(Preprocessor, MultiArgNoFirstArg)
{
    const char* code = "#define X(a,b) a b\nX(x,)\n";

    LocalAllocator allocator;
    HelperErrorReporter errorReporter;
    TextFilePreprocessor test(allocator, ITextIncludeHandler::GetEmptyHandler(), errorReporter, ITextCommentEater::StandardComments(), GetTestLanguage());
    ASSERT_TRUE(test.processContent(code, "TestFile"));

    ASSERT_FALSE(test.tokens().empty());

    {
        auto t  = test.tokens().popFront();
        ASSERT_TRUE(t);
        ASSERT_TRUE(t->isIdentifier());
        ASSERT_TRUE(t->view() == "x");
        ASSERT_EQ(2, t->location().line());
    }
}

TEST(Preprocessor, MultiArgNoSecondArg)
{
    const char* code = "#define X(a,b) a b\nX(,y)\n";

    LocalAllocator allocator;
    HelperErrorReporter errorReporter;
    TextFilePreprocessor test(allocator, ITextIncludeHandler::GetEmptyHandler(), errorReporter, ITextCommentEater::StandardComments(), GetTestLanguage());
    ASSERT_TRUE(test.processContent(code, "TestFile"));

    ASSERT_FALSE(test.tokens().empty());

    {
        auto t  = test.tokens().popFront();
        ASSERT_TRUE(t);
        ASSERT_TRUE(t->isIdentifier());
        ASSERT_TRUE(t->view() == "y");
        ASSERT_EQ(2, t->location().line());
    }
}

TEST(Preprocessor, WrappedArgs)
{
    const char* code = "#define X(a) a\nX((x,y))\n";

    LocalAllocator allocator;
    HelperErrorReporter errorReporter;
    TextFilePreprocessor test(allocator, ITextIncludeHandler::GetEmptyHandler(), errorReporter, ITextCommentEater::StandardComments(), GetTestLanguage());
    ASSERT_TRUE(test.processContent(code, "TestFile"));

    ASSERT_FALSE(test.tokens().empty());

    {
        auto t  = test.tokens().popFront();
        ASSERT_TRUE(t);
        ASSERT_TRUE(t->isChar());
        ASSERT_TRUE(t->view() == "(");
        ASSERT_EQ(2, t->location().line());
    }

    {
        auto t  = test.tokens().popFront();
        ASSERT_TRUE(t);
        ASSERT_TRUE(t->isIdentifier());
        ASSERT_TRUE(t->view() == "x");
        ASSERT_EQ(2, t->location().line());
    }

    {
        auto t  = test.tokens().popFront();
        ASSERT_TRUE(t);
        ASSERT_TRUE(t->isChar());
        ASSERT_TRUE(t->view() == ",");
        ASSERT_EQ(2, t->location().line());
    }

    {
        auto t  = test.tokens().popFront();
        ASSERT_TRUE(t);
        ASSERT_TRUE(t->isIdentifier());
        ASSERT_TRUE(t->view() == "y");
        ASSERT_EQ(2, t->location().line());
    }

    {
        auto t  = test.tokens().popFront();
        ASSERT_TRUE(t);
        ASSERT_TRUE(t->isChar());
        ASSERT_TRUE(t->view() == ")");
        ASSERT_EQ(2, t->location().line());
    }
}

TEST(Preprocessor, StringifySimple)
{
    const char* code = "#define X(a) #a\nX(test)\n";

    LocalAllocator allocator;
    HelperErrorReporter errorReporter;
    TextFilePreprocessor test(allocator, ITextIncludeHandler::GetEmptyHandler(), errorReporter, ITextCommentEater::StandardComments(), GetTestLanguage());
    ASSERT_TRUE(test.processContent(code, "TestFile"));

    ASSERT_FALSE(test.tokens().empty());

    {
        auto t  = test.tokens().popFront();
        ASSERT_TRUE(t);
        ASSERT_TRUE(t->isString());
        ASSERT_TRUE(t->view() == "test");
        ASSERT_EQ(2, t->location().line());
    }
}

TEST(Preprocessor, StringifyComplex)
{
    const char* code = "#define X(a) #a\nX(test+dupa)\n";

    LocalAllocator allocator;
    HelperErrorReporter errorReporter;
    TextFilePreprocessor test(allocator, ITextIncludeHandler::GetEmptyHandler(), errorReporter, ITextCommentEater::StandardComments(), GetTestLanguage());
    ASSERT_TRUE(test.processContent(code, "TestFile"));

    ASSERT_FALSE(test.tokens().empty());

    {
        auto t  = test.tokens().popFront();
        ASSERT_TRUE(t);
        ASSERT_TRUE(t->isString());
        ASSERT_TRUE(t->view() == "test+dupa");
        ASSERT_EQ(2, t->location().line());
    }
}

TEST(Preprocessor, StringifyVeryComplex)
{
    const char* code = "#define X(a) #a\nX(!world -> HasFlag(    Flags::Dupa ))\n";

    LocalAllocator allocator;
    HelperErrorReporter errorReporter;
    TextFilePreprocessor test(allocator, ITextIncludeHandler::GetEmptyHandler(), errorReporter, ITextCommentEater::StandardComments(), GetTestLanguage());
    ASSERT_TRUE(test.processContent(code, "TestFile"));

    ASSERT_FALSE(test.tokens().empty());

    {
        auto t  = test.tokens().popFront();
        ASSERT_TRUE(t);
        ASSERT_TRUE(t->isString());
        ASSERT_TRUE(t->view() == "!world -> HasFlag(    Flags::Dupa )");
        ASSERT_EQ(2, t->location().line());
    }
}

TEST(Preprocessor, MergeTwoTokens)
{
    const char* code = "#define X(a,b) a##b\nX(x,y)";

    LocalAllocator allocator;
    HelperErrorReporter errorReporter;
    TextFilePreprocessor test(allocator, ITextIncludeHandler::GetEmptyHandler(), errorReporter, ITextCommentEater::StandardComments(), GetTestLanguage());
    ASSERT_TRUE(test.processContent(code, "TestFile"));

    ASSERT_FALSE(test.tokens().empty());

    {
        auto t  = test.tokens().popFront();
        ASSERT_TRUE(t);
        ASSERT_TRUE(t->isIdentifier());
        ASSERT_TRUE(t->view() == "xy");
        ASSERT_EQ(2, t->location().line());
    }
}

TEST(Preprocessor, MergeMutlipleTokens)
{
    const char* code = "#define X(a,b,c,d) a##b##c##d\nX(x,y,z,w)";

    LocalAllocator allocator;
    HelperErrorReporter errorReporter;
    TextFilePreprocessor test(allocator, ITextIncludeHandler::GetEmptyHandler(), errorReporter, ITextCommentEater::StandardComments(), GetTestLanguage());
    ASSERT_TRUE(test.processContent(code, "TestFile"));

    ASSERT_FALSE(test.tokens().empty());

    {
        auto t  = test.tokens().popFront();
        ASSERT_TRUE(t);
        ASSERT_TRUE(t->isIdentifier());
        ASSERT_TRUE(t->view() == "xyzw");
        ASSERT_EQ(2, t->location().line());
    }
}

TEST(Preprocessor, NestedMacro)
{
    const char* code = "#define Y(y) 2##y\n#define X(a) Y(a)+Y(a)\nX(3)";

    LocalAllocator allocator;
    HelperErrorReporter errorReporter;
    TextFilePreprocessor test(allocator, ITextIncludeHandler::GetEmptyHandler(), errorReporter, ITextCommentEater::StandardComments(), GetTestLanguage());
    ASSERT_TRUE(test.processContent(code, "TestFile"));

    ASSERT_FALSE(test.tokens().empty());

    {
        auto t  = test.tokens().popFront();
        ASSERT_TRUE(t);
        ASSERT_TRUE(t->isInteger());
        ASSERT_TRUE(t->view() == "23");
        ASSERT_EQ(3, t->location().line());
    }

    {
        auto t  = test.tokens().popFront();
        ASSERT_TRUE(t);
        ASSERT_TRUE(t->isChar());
        ASSERT_TRUE(t->view() == "+");
        ASSERT_EQ(3, t->location().line());
    }

    {
        auto t  = test.tokens().popFront();
        ASSERT_TRUE(t);
        ASSERT_TRUE(t->isInteger());
        ASSERT_TRUE(t->view() == "23");
        ASSERT_EQ(3, t->location().line());
    }
}

TEST(Preprocessor, UltimateTest1)
{
    const char* code = ""\
        "#define HE HI\n"\
        "#define LLO _THERE\n"\
        "#define HELLO \"HI THERE\"\n"\
        "#define CAT(a,b) a##b\n"\
        "#define XCAT(a,b) CAT(a,b)\n"\
        "#define CALL(fn) fn(HE,LLO)\n"\
        "CAT(HE,LLO)";

    LocalAllocator allocator;
    HelperErrorReporter errorReporter;
    TextFilePreprocessor test(allocator, ITextIncludeHandler::GetEmptyHandler(), errorReporter, ITextCommentEater::StandardComments(), GetTestLanguage());
    ASSERT_TRUE(test.processContent(code, "TestFile"));

    ASSERT_FALSE(test.tokens().empty());

    {
        auto t  = test.tokens().popFront();
        ASSERT_TRUE(t);
        ASSERT_TRUE(t->isString());
        ASSERT_TRUE(t->view() == "HI THERE");
    }
}

TEST(Preprocessor, UltimateTest2)
{
    const char* code = ""\
        "#define HE HI\n"\
        "#define LLO _THERE\n"\
        "#define HELLO \"HI THERE\"\n"\
        "#define CAT(a,b) a##b\n"\
        "#define XCAT(a,b) CAT(a,b)\n"\
        "#define CALL(fn) fn(HE,LLO)\n"\
        "XCAT(HE,LLO)";

    LocalAllocator allocator;
    HelperErrorReporter errorReporter;
    TextFilePreprocessor test(allocator, ITextIncludeHandler::GetEmptyHandler(), errorReporter, ITextCommentEater::StandardComments(), GetTestLanguage());
    ASSERT_TRUE(test.processContent(code, "TestFile"));

    ASSERT_FALSE(test.tokens().empty());

    {
        auto t  = test.tokens().popFront();
        ASSERT_TRUE(t);
        ASSERT_TRUE(t->isIdentifier());
        ASSERT_TRUE(t->view() == "HI_THERE");
    }
}

TEST(Preprocessor, UltimateTest3)
{
    const char* code = ""\
        "#define HE HI\n"\
        "#define LLO _THERE\n"\
        "#define HELLO \"HI THERE\"\n"\
        "#define CAT(a,b) a##b\n"\
        "#define XCAT(a,b) CAT(a,b)\n"\
        "#define CALL(fn) fn(HE,LLO)\n"\
        "CALL(CAT)";

    LocalAllocator allocator;
    HelperErrorReporter errorReporter;
    TextFilePreprocessor test(allocator, ITextIncludeHandler::GetEmptyHandler(), errorReporter, ITextCommentEater::StandardComments(), GetTestLanguage());
    ASSERT_TRUE(test.processContent(code, "TestFile"));

    ASSERT_FALSE(test.tokens().empty());

    {
        auto t  = test.tokens().popFront();
        ASSERT_TRUE(t);
        ASSERT_TRUE(t->isString());
        ASSERT_TRUE(t->view() == "HI THERE");
    }
}

class TestIncluder1 : public ITextIncludeHandler
{
public:
    virtual bool loadInclude(bool global, StringView path, StringView referencePath, Buffer& outContent, StringBuf& outPath) override
    {
        m_path = StringBuf(path);
        m_called = true;
        return true;
    }

    mutable StringBuf m_path;
    mutable bool m_called = false;
};

TEST(Preprocessor, IncludeForwardPath)
{
    const char* code = "#include \"other\"\n";

    TestIncluder1 includer;
    LocalAllocator allocator;
    HelperErrorReporter errorReporter;
    TextFilePreprocessor test(allocator, includer, errorReporter, ITextCommentEater::StandardComments(), GetTestLanguage());
    EXPECT_TRUE(test.processContent(code, "TestFile"));
    EXPECT_TRUE(test.tokens().empty());
    EXPECT_TRUE(includer.m_called);
    EXPECT_TRUE(includer.m_path == "other");
}

class TestIncluder2 : public ITextIncludeHandler
{
public:
    virtual bool loadInclude(bool global, StringView path, StringView referencePath, Buffer& outContent, StringBuf& outPath) override
    {
        m_path = StringBuf(path);
        m_called = true;
        return false;
    }

    mutable StringBuf m_path;
    mutable bool m_called = false;
};

TEST(Preprocessor, IncludeCanFail)
{
    const char* code = "#include \"other\"\n";

    TestIncluder2 includer;
    LocalAllocator allocator;
    HelperErrorReporter errorReporter;
    TextFilePreprocessor test(allocator, includer, errorReporter, ITextCommentEater::StandardComments(), GetTestLanguage());
    EXPECT_FALSE(test.processContent(code, "TestFile"));
    EXPECT_EQ(1, errorReporter.m_numErrors);
    EXPECT_TRUE(test.tokens().empty());
    EXPECT_TRUE(includer.m_called);
    EXPECT_TRUE(includer.m_path == "other");
}

class TestIncluder3 : public ITextIncludeHandler
{
public:
    virtual bool loadInclude(bool global, StringView path, StringView referencePath, Buffer& outContent, StringBuf& outPath) override
    {
        outContent = Buffer::CreateFromCopy(MainPool(), StringView("dupa"));
        outPath = StringBuf("OtherFile");
        m_path = StringBuf(path);
        m_called = true;
        return true;
    }

    mutable StringBuf m_path;
    mutable bool m_called = false;
};

TEST(Preprocessor, IncludeCanEmitContent)
{
    const char* code = "#include \"other\"\n";

    TestIncluder3 includer;
    LocalAllocator allocator;
    HelperErrorReporter errorReporter;
    TextFilePreprocessor test(allocator, includer, errorReporter, ITextCommentEater::StandardComments(), GetTestLanguage());
    EXPECT_TRUE(test.processContent(code, "TestFile"));
    EXPECT_FALSE(test.tokens().empty());

    {
        auto t  = test.tokens().popFront();
        ASSERT_TRUE(t);
        EXPECT_TRUE(t->isIdentifier());
        EXPECT_TRUE(t->view() == "dupa");
    }
}

TEST(Preprocessor, IncludeHasProperContext)
{
    const char* code = "#include \"other\"\n";

    TestIncluder3 includer;
    LocalAllocator allocator;
    HelperErrorReporter errorReporter;
    TextFilePreprocessor test(allocator, includer, errorReporter, ITextCommentEater::StandardComments(), GetTestLanguage());
    EXPECT_TRUE(test.processContent(code, "TestFile"));
    EXPECT_FALSE(test.tokens().empty());

    {
        auto t  = test.tokens().popFront();
        ASSERT_TRUE(t);
        EXPECT_TRUE(t->isIdentifier());
        EXPECT_TRUE(t->location().contextName() == "OtherFile");
        EXPECT_EQ(1, t->location().line());
    }
}

TEST(Preprocessor, IncludeTwice)
{
    const char* code = "#include \"other\"\n#include \"other\"";

    TestIncluder3 includer;
    LocalAllocator allocator;
    HelperErrorReporter errorReporter;
    TextFilePreprocessor test(allocator, includer, errorReporter, ITextCommentEater::StandardComments(), GetTestLanguage());
    EXPECT_TRUE(test.processContent(code, "TestFile"));
    EXPECT_FALSE(test.tokens().empty());

    {
        auto t  = test.tokens().popFront();
        ASSERT_TRUE(t);
        EXPECT_TRUE(t->isIdentifier());
        EXPECT_TRUE(t->location().contextName() == "OtherFile");
        EXPECT_EQ(1, t->location().line());
    }

    {
        auto t  = test.tokens().popFront();
        ASSERT_TRUE(t);
        EXPECT_TRUE(t->isIdentifier());
        EXPECT_TRUE(t->location().contextName() == "OtherFile");
        EXPECT_EQ(1, t->location().line());
    }
}

class TestIncluder4 : public ITextIncludeHandler
{
public:
    virtual bool loadInclude(bool global, StringView path, StringView referencePath, Buffer& outContent, StringBuf& outPath) override
    {
        auto txt  = "#include \"other\"";
        outContent = Buffer::CreateFromCopy(MainPool(), StringView(txt));
        outPath = StringBuf("OtherFile");
        m_path = StringBuf(path);
        m_called = true;
        return true;
    }

    mutable StringBuf m_path;
    mutable bool m_called = false;
};

TEST(Preprocessor, IncludeRecursiveProtected)
{
    const char* code = "#include \"other\"";

    TestIncluder4 includer;
    LocalAllocator allocator;
    HelperErrorReporter errorReporter;
    TextFilePreprocessor test(allocator, includer, errorReporter, ITextCommentEater::StandardComments(), GetTestLanguage());
    EXPECT_FALSE(test.processContent(code, "TestFile"));
}

class TestIncluder5 : public ITextIncludeHandler
{
public:
    virtual bool loadInclude(bool global, StringView path, StringView referencePath, Buffer& outContent, StringBuf& outPath) override
    {
        auto txt  = "#pragma once\n#include \"other\"\ntest";
        outContent = Buffer::CreateFromCopy(MainPool(), StringView(txt));
        outPath = StringBuf("OtherFile");
        m_path = StringBuf(path);
        m_called = true;
        return true;
    }

    mutable StringBuf m_path;
    mutable bool m_called = false;
};

TEST(Preprocessor, IncludeRecursivePragmaOnce)
{
    const char* code = "#include \"other\"";

    TestIncluder5 includer;
    LocalAllocator allocator;
    HelperErrorReporter errorReporter;
    TextFilePreprocessor test(allocator, includer, errorReporter, ITextCommentEater::StandardComments(), GetTestLanguage());
    
    EXPECT_TRUE(test.processContent(code, "TestFile"));
    EXPECT_FALSE(test.tokens().empty());

    {
        auto t  = test.tokens().popFront();
        ASSERT_TRUE(t);
        EXPECT_TRUE(t->isIdentifier());
        EXPECT_TRUE(t->view() == "test");
    }

    EXPECT_TRUE(test.tokens().empty());
}

TEST(Preprocessor, IfDefExpectsEndif)
{
    const char* code = "#ifdef X\n";

    TestIncluder5 includer;
    LocalAllocator allocator;
    HelperErrorReporter errorReporter;
    TextFilePreprocessor test(allocator, includer, errorReporter, ITextCommentEater::StandardComments(), GetTestLanguage());

    EXPECT_FALSE(test.processContent(code, "TestFile"));
    EXPECT_EQ(1, errorReporter.m_numErrors);
}

TEST(Preprocessor, IfDefExpectsSymbolName)
{
    const char* code = "#ifdef\n";

    TestIncluder5 includer;
    LocalAllocator allocator;
    HelperErrorReporter errorReporter;
    TextFilePreprocessor test(allocator, includer, errorReporter, ITextCommentEater::StandardComments(), GetTestLanguage());

    EXPECT_FALSE(test.processContent(code, "TestFile"));
    EXPECT_EQ(1, errorReporter.m_numErrors);
}

TEST(Preprocessor, StrayEndifError)
{
    const char* code = "#endif\n";

    TestIncluder5 includer;
    LocalAllocator allocator;
    HelperErrorReporter errorReporter;
    TextFilePreprocessor test(allocator, includer, errorReporter, ITextCommentEater::StandardComments(), GetTestLanguage());

    EXPECT_FALSE(test.processContent(code, "TestFile"));
    EXPECT_EQ(1, errorReporter.m_numErrors);
}

TEST(Preprocessor, StrayElseError)
{
    const char* code = "#else\n";

    TestIncluder5 includer;
    LocalAllocator allocator;
    HelperErrorReporter errorReporter;
    TextFilePreprocessor test(allocator, includer, errorReporter, ITextCommentEater::StandardComments(), GetTestLanguage());

    EXPECT_FALSE(test.processContent(code, "TestFile"));
    EXPECT_EQ(1, errorReporter.m_numErrors);
}

TEST(Preprocessor, IfDefSimpleFilter_FilterAway)
{
    const char* code = "#ifdef X\ntest\n#endif";

    TestIncluder5 includer;
    LocalAllocator allocator;
    HelperErrorReporter errorReporter;
    TextFilePreprocessor test(allocator, includer, errorReporter, ITextCommentEater::StandardComments(), GetTestLanguage());

    EXPECT_TRUE(test.processContent(code, "TestFile"));
    EXPECT_TRUE(test.tokens().empty());
}

TEST(Preprocessor, IfDefSimpleFilter_FilterInWithGlobalDef)
{
    const char* code = "#ifdef X\ntest\n#endif";

    TestIncluder5 includer;
    LocalAllocator allocator;
    HelperErrorReporter errorReporter;
    TextFilePreprocessor test(allocator, includer, errorReporter, ITextCommentEater::StandardComments(), GetTestLanguage());
    test.defineSymbol("X", "");

    EXPECT_TRUE(test.processContent(code, "TestFile"));
    EXPECT_FALSE(test.tokens().empty());

    {
        auto t  = test.tokens().popFront();
        ASSERT_TRUE(t);
        EXPECT_TRUE(t->isIdentifier());
        EXPECT_TRUE(t->view() == "test");
    }
}

TEST(Preprocessor, IfDefSimpleFilter_FilterWithLocalDef)
{
    const char* code = "#define X 2\n#ifdef X\ntest\n#endif";

    TestIncluder5 includer;
    LocalAllocator allocator;
    HelperErrorReporter errorReporter;
    TextFilePreprocessor test(allocator, includer, errorReporter, ITextCommentEater::StandardComments(), GetTestLanguage());

    EXPECT_TRUE(test.processContent(code, "TestFile"));
    EXPECT_FALSE(test.tokens().empty());

    {
        auto t  = test.tokens().popFront();
        ASSERT_TRUE(t);
        EXPECT_TRUE(t->isIdentifier());
        EXPECT_TRUE(t->view() == "test");
    }
}

TEST(Preprocessor, IfDefElseNotTaken)
{
    const char* code = "#ifdef X\nyes\n#else\nno\n#endif";

    TestIncluder5 includer;
    LocalAllocator allocator;
    HelperErrorReporter errorReporter;
    TextFilePreprocessor test(allocator, includer, errorReporter, ITextCommentEater::StandardComments(), GetTestLanguage());
    test.defineSymbol("X", "");

    EXPECT_TRUE(test.processContent(code, "TestFile"));
    EXPECT_FALSE(test.tokens().empty());

    {
        auto t  = test.tokens().popFront();
        ASSERT_TRUE(t);
        EXPECT_TRUE(t->isIdentifier());
        EXPECT_TRUE(t->view() == "yes");
    }
}

TEST(Preprocessor, IfDefElseTaken)
{
    const char* code = "#ifdef X\nyes\n#else\nno\n#endif";

    TestIncluder5 includer;
    LocalAllocator allocator;
    HelperErrorReporter errorReporter;
    TextFilePreprocessor test(allocator, includer, errorReporter, ITextCommentEater::StandardComments(), GetTestLanguage());
    //test.defineSymbol("X", "");

    EXPECT_TRUE(test.processContent(code, "TestFile"));
    EXPECT_FALSE(test.tokens().empty());

    {
        auto t  = test.tokens().popFront();
        ASSERT_TRUE(t);
        EXPECT_TRUE(t->isIdentifier());
        EXPECT_TRUE(t->view() == "no");
    }
}

TEST(Preprocessor, IfDefDoubleElseInvalid)
{
    const char* code = "#ifdef X\nyes\n#else\nno\n#else\ncrap\n#endif";

    TestIncluder5 includer;
    LocalAllocator allocator;
    HelperErrorReporter errorReporter;
    TextFilePreprocessor test(allocator, includer, errorReporter, ITextCommentEater::StandardComments(), GetTestLanguage());
    //test.defineSymbol("X", "");

    EXPECT_FALSE(test.processContent(code, "TestFile"));
    EXPECT_EQ(1, errorReporter.m_numErrors);
}

TEST(Preprocessor, IfDefUndef)
{
    const char* code = "#define X\n#ifdef X\nfirst\n#endif\n#undef X\n#ifdef X\nsecond\n#endif";

    TestIncluder5 includer;
    LocalAllocator allocator;
    HelperErrorReporter errorReporter;
    TextFilePreprocessor test(allocator, includer, errorReporter, ITextCommentEater::StandardComments(), GetTestLanguage());
    //test.defineSymbol("X", "");

    EXPECT_TRUE(test.processContent(code, "TestFile"));
    EXPECT_FALSE(test.tokens().empty());

    {
        auto t  = test.tokens().popFront();
        ASSERT_TRUE(t);
        EXPECT_TRUE(t->isIdentifier());
        EXPECT_TRUE(t->view() == "first");
    }

    EXPECT_TRUE(test.tokens().empty());
}

END_INFERNO_NAMESPACE()