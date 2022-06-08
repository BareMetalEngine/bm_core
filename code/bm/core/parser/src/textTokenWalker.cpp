/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "textToken.h"
#include "textTokenWalker.h"
#include "bm/core/memory/include/localAllocator.h"

BEGIN_INFERNO_NAMESPACE()

//--

TextTokenWalker::TextTokenWalker(const TokenList& list, ITextErrorReporter& err)
    : m_err(err)
{
    // extract linear tokens
    const auto* cur = list.head();
    while (cur)
    {
        m_tokens.pushBack(cur);
        if (cur == list.tail())
            break;

        cur = cur->next();
    }
}

TextTokenWalker::~TextTokenWalker()
{}

static const Token EMPTY_TOKEN;

const Token& TextTokenWalker::token(int delta) const
{
    auto index = m_current + delta;
    if (index >= 0 && index <= m_tokens.lastValidIndex())
        return *m_tokens[index];
    else
        return EMPTY_TOKEN;
}

void TextTokenWalker::advance(int delta)
{
    m_current += delta;
}

void TextTokenWalker::pushState()
{
    m_stack.pushBack(m_current);
}

void TextTokenWalker::restoreState()
{
    DEBUG_CHECK_RETURN_EX(!m_stack.empty(), "Empty state stack");
    m_stack.popBack();
}

static int BracketType(char ch)
{
    switch (ch)
    {
    case '{':
    case '}':
        return 0;
    case '[':
    case ']':
        return 1;
    case '(':
    case ')':
        return 2;
    }

    ASSERT(!"invalid");
    return -1;
}

TokenList TextTokenWalker::extractScope(LocalAllocator& mem, char finalBracket)
{
    int bracketCounts[3] = { 0,0,0 }; // {} [] ()
    bracketCounts[BracketType(finalBracket)] = 1;

    bool closed = false;
    int cur = m_current;
    while (cur <= m_tokens.lastValidIndex())
    {
        const auto& token = *m_tokens[cur];

        if (token.isChar())
        {
            if (token.ch() == '{' || token.ch() == '[' || token.ch() == '(')
            {
                bracketCounts[BracketType(token.ch())] += 1;
            }
            else if (token.ch() == '}' || token.ch() == ']' || token.ch() == ')')
            {
                const int type = BracketType(token.ch());

                if (bracketCounts[type] == 0)
                {
                    m_err.reportError(token.location(), TempString("Unexpected '{}' found", token.view()));
                    return TokenList();
                }

                bracketCounts[type] -= 1;

                if (bracketCounts[0] == 0 && bracketCounts[1] == 0 && bracketCounts[2] == 0)
                {
                    closed = true;
                    break;
                }
            }
        }

        cur += 1; // advance
    }

    TokenList ret;

    if (closed)
    {
        for (auto i = m_current; i < cur; ++i)
        {
            // TODO: remove destruction needs
            auto* copy = mem.createAndRegisterDestroyer<Token>(*m_tokens[i]);
            ret.pushBack(copy);
        }

        m_current = cur + 1;
    }
    else
    {
        m_err.reportError(token(0).location(), "Unexpected end of file before closing bracket was found");
    }

    return ret;
}

StringBuf TextTokenWalker::extractArrayIndex()
{
    int bracketCounts[3] = { 0,0,0 }; // {} [] ()
    bracketCounts[1] = 1; 

    bool closed = false;
    int cur = m_current;
    while (cur <= m_tokens.lastValidIndex())
    {
        const auto& token = *m_tokens[cur];

        if (token.isChar())
        {
            if (token.ch() == '{' || token.ch() == '[' || token.ch() == '(')
            {
                bracketCounts[BracketType(token.ch())] += 1;
            }
            else if (token.ch() == '}' || token.ch() == ']' || token.ch() == ')')
            {
                const int type = BracketType(token.ch());

                if (bracketCounts[type] == 0)
                {
                    m_err.reportError(token.location(), TempString("Unexpected '{}' found", token.view()));
                    return StringBuf();
                }

                bracketCounts[type] -= 1;

                if (bracketCounts[0] == 0 && bracketCounts[1] == 0 && bracketCounts[2] == 0)
                {
                    closed = true;
                    break;
                }
            }
            else if (token.ch() == ',')
            {
                // ',' is valid inside the array index, ie: [1,2,3]
                if (bracketCounts[0] == 0 && bracketCounts[1] == 1 && bracketCounts[2] == 0)
                {
                    closed = true;
                    break;
                }
            }
        }

        cur += 1; // advance
    }

    if (!closed)
    {
        m_err.reportError(token(0).location(), "Unexpected end of file before end of array index was found");
        return StringBuf::EMPTY();
    }

    StringBuilder txt;
    TokenLineReprinter line(txt);

    for (auto i = m_current; i < cur; ++i)
        line.print(*m_tokens[i]);

    m_current = cur; // do not advance past last token

    return StringBuf(txt.view());
}

StringBuf TextTokenWalker::extractArgument()
{
    int bracketCounts[3] = { 0,0,0 }; // {} [] ()
    bracketCounts[2] = 0;

    bool closed = false;
    int cur = m_current;
    while (cur <= m_tokens.lastValidIndex())
    {
        const auto& token = *m_tokens[cur];

        if (token.isChar())
        {
            if (token.ch() == '{' || token.ch() == '[' || token.ch() == '(')
            {
                bracketCounts[BracketType(token.ch())] += 1;
            }
            else if (token.ch() == '}' || token.ch() == ']' || token.ch() == ')')
            {
                const int type = BracketType(token.ch());

                if (bracketCounts[type] == 0)
                {
                    m_err.reportError(token.location(), TempString("Unexpected '{}' found", token.view()));
                    return StringBuf();
                }

                bracketCounts[type] -= 1;

                if (bracketCounts[0] == 0 && bracketCounts[1] == 0 && bracketCounts[2] == 0)
                {
                    closed = true;
                    break;
                }
            }
            else if (token.ch() == ',')
            {
                // ',' is valid inside the argument list, e.g: (a,b,c)
                if (bracketCounts[0] == 0 && bracketCounts[1] == 0 && bracketCounts[2] == 1)
                {
                    closed = true;
                    break;
                }
            }
        }

        cur += 1; // advance
    }

    if (!closed)
    {
        m_err.reportError(token(0).location(), "Unexpected end of file before end of array index was found");
        return StringBuf::EMPTY();
    }

    StringBuilder txt;
    TokenLineReprinter line(txt);

    for (auto i = m_current; i < cur; ++i)
        line.print(*m_tokens[i]);

    m_current = cur; // do not advance past last token

    return StringBuf(txt);
}

bool TextTokenWalker::skipScope(char finalBracket)
{
    int bracketCounts[3] = { 0,0,0 }; // {} [] ()
    bracketCounts[BracketType(finalBracket)] = 1;

    bool closed = false;
    int cur = m_current;
    while (cur <= m_tokens.lastValidIndex())
    {
        const auto& token = *m_tokens[cur];

        if (token.isChar())
        {
            if (token.ch() == '{' || token.ch() == '[' || token.ch() == '(')
            {
                bracketCounts[BracketType(token.ch())] += 1;
            }
            else if (token.ch() == '}' || token.ch() == ']' || token.ch() == ')')
            {
                const int type = BracketType(token.ch());

                if (bracketCounts[type] == 0)
                {
                    m_err.reportError(token.location(), TempString("Unexpected '{}' found", token.view()));
                    return false;
                }

                bracketCounts[type] -= 1;

                if (bracketCounts[0] == 0 && bracketCounts[1] == 0 && bracketCounts[2] == 0)
                {
                    closed = true;
                    break;
                }
            }
        }

        cur += 1; // advance
    }

    if (closed)
    {
        m_current = cur + 1;
        return true;
    }
    else
    {
        m_err.reportError(token(0).location(), "Unexpected end of file before closing bracket was found");
        return false;
    }
}

//--

bool TextTokenWalker::consumeIdentifier(StringID& outIdent, bool printError /*= true*/)
{
    if (!current().isIdentifier())
    {
        if (printError)
            error(0, TempString("Expected identifier, found '{}'", current().view()));
        return false;
    }

    outIdent = StringID(current().view());
    advance(1);
    return true;
}

bool TextTokenWalker::consumeIdentifier(StringView ident, bool printError /*= true*/)
{
    if (!current().isIdentifier() || current().view() != ident)
    {
        if (printError)
            error(0, TempString("Expected '{}', found '{}'", ident, current().view()));
        return false;
    }

    advance(1);
    return true;
}

bool TextTokenWalker::consumeInt(int& outInt, bool printError /*= true*/)
{
    if (!current().isInteger())
    {
        if (printError)
            error(0, TempString("Expected integer number, found '{}'", current().view()));
        return false;
    }

    outInt = current().integerNumber();
    advance(1);
    return true;
}

bool TextTokenWalker::consumeKeyword(int id, bool printError /*= true*/)
{
    if (!current().isKeyword() || current().keywordID() != id)
    {
        if (printError)
            error(0, TempString("Unexpected '{}' found when looking for keyword", current().view()));
        return false;
    }

    advance(1);
    return true;
}

bool TextTokenWalker::consumeChar(char ch, bool printError /*= true*/)
{
    if (!current().isChar() || current().ch() != ch)
    {
        if (printError)
        {
            char str[2] = { ch,0 };
            error(0, TempString("Unexpected '{}' found when looking for '{}'", current().view(), str));
        }
        return false;
    }

    advance(1);
    return true;
}

//--

void TextTokenWalker::error(int delta, StringView txt) const
{
    m_err.reportError(token(delta).location(), txt);
}

void TextTokenWalker::warning(int delta, StringView txt) const
{
    m_err.reportWarning(token(delta).location(), txt);
}

//--

END_INFERNO_NAMESPACE()
