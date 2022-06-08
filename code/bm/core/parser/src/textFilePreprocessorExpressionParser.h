/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

#include "textToken.h"

BEGIN_INFERNO_NAMESPACE()

struct TextFilePreprocessorExpressionNode
{
    TextTokenLocation location;
    StringView text;
    int64_t value = 0;
};

class TextFilePreprocessorExpressionContext
{
public:
    typedef std::function<bool(StringView)> TDefinedFunc;

    TextFilePreprocessorExpressionContext(TokenList& list, ITextErrorReporter& err, const TDefinedFunc& definedFunc);

    int readToken(TextFilePreprocessorExpressionNode& outElem);
    bool reportError(StringView txt);

    bool evaluate(bool& result);

    void writeResult(int result);

private:
    const Token* m_root = nullptr;
    const Token* m_tokens = nullptr;
    ITextErrorReporter& m_err;

    TDefinedFunc m_definedFunc;

    int m_result = 0;
};

END_INFERNO_NAMESPACE()
