/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

#include "bm/core/containers/include/stringBuf.h"

BEGIN_INFERNO_NAMESPACE()

///----

/// helper class that walks the token stream
class BM_CORE_PARSER_API TextTokenWalker : public MainPoolData<NoCopy>
{
public:
    TextTokenWalker(const TokenList& list, ITextErrorReporter& err);
    ~TextTokenWalker();

    //--

    // get token at +/- position, for positions out of range it returns empty token
    const Token& token(int delta) const;

    // get token at current position
    INLINE const Token& current() const { return token(0); }

    // eat one token
    INLINE const Token& eat() { const auto& ret = token(0); advance(1); return ret; }

    // advance position by x tokens
    void advance(int delta=1);

    //--

    // save current state (active token), useful when we wan to to "tryParse" sth
    void pushState();

    // restore previously pushed state
    void restoreState();

    //--

    // extract token list until a delimiter is found (other brackets are counted properly so nested scopes are not a problem)
    // NOTE: makes a COPY of tokens
    // NOTE: final bracket is not extracted
    TokenList extractScope(LocalAllocator& mem, char finalBracket);

    // extra array index (finishes at ']' or ',', final token is not extracted)
    StringBuf extractArrayIndex();

    // extract argument, stops at ',' or ')', final token is not extracted
    StringBuf extractArgument();

    // skip tokens until we leave scope
    bool skipScope(char finalBracket);

    //--

    // report error at current location
    void error(int delta, StringView txt) const;

    // report warning at current location
    void warning(int delta, StringView txt) const;

    //--

    // consume identifier, prints error
    bool consumeIdentifier(StringID& outIdent, bool printError = true);

    // consume specific identifier, prints error
    bool consumeIdentifier(StringView ident, bool printError = true);

    // consume specific keyword, prints error
    bool consumeKeyword(int id, bool printError = true);

    // consume char, prints error
    bool consumeChar(char ch, bool printError = true);

    // consume integer value, prints error
    bool consumeInt(int& outInt, bool printError = true);

    //--

private:
    ITextErrorReporter& m_err;

    Array<const Token*> m_tokens;
    int m_current = 0;

    Array<int> m_stack;
};

///----

END_INFERNO_NAMESPACE()
