/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "textFilePreprocessorExpressionParser.h"
#include "textFilePreprocessorExpressionParser_Symbols.h"

extern int tfpe_parse(bm::TextFilePreprocessorExpressionContext& ctx);

BEGIN_INFERNO_NAMESPACE()

//--

TextFilePreprocessorExpressionContext::TextFilePreprocessorExpressionContext(TokenList& list, ITextErrorReporter& err, const TDefinedFunc& definedFunc)
    : m_tokens(list.head())
    , m_root(list.head())
    , m_definedFunc(definedFunc)
    , m_err(err)
{}

int TextFilePreprocessorExpressionContext::readToken(TextFilePreprocessorExpressionNode& outElem)
{
    if (!m_tokens)
        return 0;

    auto* cur = m_tokens;
    m_tokens = m_tokens->next();

    outElem.location = cur->location();
    outElem.text = cur->view();

    if (cur->isInteger() || cur->isUnsigned())
    {
        cur->view().match(outElem.value);
        return TOKEN_NUMBER;
    }
    else if (cur->isKeyword())
    {
        if (cur->view() == "==") return TOKEN_EQ_OP;
        if (cur->view() == "!=") return TOKEN_NE_OP;
        if (cur->view() == "<<") return TOKEN_LEFT_OP;
        if (cur->view() == ">>") return TOKEN_RIGHT_OP;
        if (cur->view() == "<=") return TOKEN_LE_OP;
        if (cur->view() == ">=") return TOKEN_GE_OP;
        if (cur->view() == "&&") return TOKEN_AND_OP;
        if (cur->view() == "||") return TOKEN_OR_OP;
    }
    else if (cur->isChar())
    {
        return cur->ch();
    }

    m_err.reportError(outElem.location, TempString("Unexpected token '{}'", outElem.text));
    return 0;
}

bool TextFilePreprocessorExpressionContext::reportError(StringView txt)
{
    m_err.reportError(m_root->location(), txt);
    return false;
}

void TextFilePreprocessorExpressionContext::writeResult(int result)
{
    m_result = result;
}

bool TextFilePreprocessorExpressionContext::evaluate(bool& result)
{
    // run though the bison parser to get the structure
    auto ret = tfpe_parse(*this);
    if (ret != 0)
        return false;

    // return written result
    result = (m_result != 0);
    return true;
}

//--

END_INFERNO_NAMESPACE()