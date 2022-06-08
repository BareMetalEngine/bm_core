/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "textToken.h"
#include "bm/core/containers/include/stringParser.h"

BEGIN_INFERNO_NAMESPACE()

//--

void TextTokenLocation::print(IFormatStream& f) const
{
    if (m_contextName.empty())
        f << "unknown";
    else if (m_pos != 0)
#ifdef PLATFORM_MSVC
        f.appendf("{}({},{})", m_contextName, m_line, m_pos);
#else
        f.appendf("{}({}:{})", m_contextName, m_line, m_pos);
#endif
    else if (m_line != 0)
        f.appendf("{}({})", m_contextName, m_line);
    else if (m_line != 0)
        f <<m_contextName;
}

//--

StringBuf Token::string() const
{
    if (m_end == m_str)
        return StringBuf::EMPTY();

    return StringBuf(m_str, m_end - m_str);
}

const char* Token::typeName() const
{
    switch (m_type)
    {
        case TextTokenType::Invalid: return "Invalid";
        case TextTokenType::String: return "String";
        case TextTokenType::Name: return "Name";
        case TextTokenType::IntNumber: return "IntNumber";
        case TextTokenType::UnsignedNumber: return "UnsignedNumber";
        case TextTokenType::FloatNumber: return "FloatNumber";
        case TextTokenType::Hex: return "Hex";
        case TextTokenType::Char: return "Char";
        case TextTokenType::Keyword: return "Keyword";
        case TextTokenType::Identifier: return "Identifier";
        case TextTokenType::Preprocessor: return "Preprocessor";
    }

    return "Unknown";
}

StringView Token::view() const
{
    if (m_end == m_str)
        return StringView();

    return StringView(m_str, m_end);
}

double Token::floatNumber() const
{
    DEBUG_CHECK_RETURN_EX_V(m_str < m_end, "Invalid token", 0);
    DEBUG_CHECK_RETURN_EX_V(m_type == TextTokenType::FloatNumber, "Not an floating point number", 0);

    double ret = 0.0;
    view().match(ret);
    return ret;
}

int64_t Token::integerNumber() const
{
    DEBUG_CHECK_RETURN_EX_V(m_str < m_end, "Invalid token", 0);
    DEBUG_CHECK_RETURN_EX_V(m_type == TextTokenType::IntNumber, "Not an integer number", 0);

	int64_t ret = 0;

    if (view().endsWith("i") || view().endsWith("I"))
        view().leftPart(view().length() - 1).match(ret);
    else
        view().match(ret);

    return ret;
}

uint64_t Token::unsignedNumber() const
{
    DEBUG_CHECK_RETURN_EX_V(m_str < m_end, "Invalid token", 0);
    DEBUG_CHECK_RETURN_EX_V(m_type == TextTokenType::UnsignedNumber, "Not an unsigned number", 0);

    uint64_t ret = 0;

	if (view().endsWith("u") || view().endsWith("U"))
		view().leftPart(view().length() - 1).match(ret);
	else
		view().match(ret);

    return ret;
}

static bool GetHexDigit(char ch, uint8_t& outValue)
{
    switch (ch)
    {
    case '0': outValue = 0; return true;
    case '1': outValue = 1; return true;
    case '2': outValue = 2; return true;
    case '3': outValue = 3; return true;
    case '4': outValue = 4; return true;
    case '5': outValue = 5; return true;
    case '6': outValue = 6; return true;
    case '7': outValue = 7; return true;
    case '8': outValue = 8; return true;
    case '9': outValue = 9; return true;
    case 'A': outValue = 10; return true;
    case 'B': outValue = 11; return true;
    case 'C': outValue = 12; return true;
    case 'D': outValue = 13; return true;
    case 'E': outValue = 14; return true;
    case 'F': outValue = 15; return true;
    case 'a': outValue = 10; return true;
    case 'b': outValue = 11; return true;
    case 'c': outValue = 12; return true;
    case 'd': outValue = 13; return true;
    case 'e': outValue = 14; return true;
    case 'f': outValue = 15; return true;
    }

    return false;
}

uint64_t Token::hexToNumber() const
{
    DEBUG_CHECK_RETURN_EX_V(m_str + 3 < m_end, "Invalid token", 0);
    DEBUG_CHECK_RETURN_EX_V(m_type == TextTokenType::Hex, "Not a hex", 0);

    auto ptr = m_str;

    DEBUG_CHECK_RETURN_EX_V(ptr[0] == '0' && ptr[1] == 'x', "Invalid hex start", 0);
    ptr += 2;

    uint64_t ret = 0;
    while (ptr < m_end)
    {
        uint8_t val = 0;
        DEBUG_CHECK_RETURN_EX_V(GetHexDigit(*ptr++, val), "Invalid hex char", 0);
        ret <<= 4;
        ret |= val;
    }

    return ret;
}

//--

void Token::print(IFormatStream& f) const
{
    f.appendf("{} ({})", view(), typeName());
}

//--

void TokenList::clear()
{
    m_head = nullptr;
    m_tail = nullptr;
}

void TokenList::pushBack(Token* token)
{
    if (token)
    {
        if (m_tail)
            m_tail->m_next = token;
        token->m_prev = m_tail;
        m_tail = token;

        if (!m_head)
            m_head = token;
    }
}

void TokenList::pushFront(Token* token)
{
    if (token)
    {
        if (m_head)
            m_head->m_prev = token;
        token->m_next = m_head;
        m_head = token;

        if (!m_tail)
            m_tail = token;
    }
}

void TokenList::pushBack(TokenList&& list)
{
    if (!list.empty())
    {
        if (!empty())
        {
            m_tail->m_next = list.m_head;
            list.m_head->m_prev = m_tail;
            m_tail = list.m_tail;
        }
        else
        {
            m_head = list.m_head;
            m_tail = list.m_tail;
        }

        list.m_head = nullptr;
        list.m_tail = nullptr;
    }
}

void TokenList::pushFront(TokenList&& list)
{
    if (!list.empty())
    {
        if (!empty())
        {
            m_head->m_prev = list.m_tail;
            list.m_tail->m_next = m_head;
            m_head = list.m_head;
        }
        else
        {
            m_head = list.m_head;
            m_tail = list.m_tail;
        }

        list.m_head = nullptr;
        list.m_tail = nullptr;
    }
}

Token* TokenList::unlink(Token* token)
{
    if (token)
    {
        auto next  = token->next();

        if (token->m_next)
        {
            DEBUG_CHECK(token != m_tail);
            token->m_next->m_prev = token->m_prev;
        }
        else
        {
            DEBUG_CHECK(token == m_tail);
            m_tail = token->m_prev;
        }

        if (token->m_prev)
        {
            DEBUG_CHECK(token != m_head);
            token->m_prev->m_next = token->m_next;
        }
        else
        {
            DEBUG_CHECK(token == m_head);
            m_head = token->m_next;
        }

        token->m_prev = nullptr;
        token->m_next = nullptr;

        return next;
    }
    else
    {
        return nullptr;
    }
}

void TokenList::linkAfter(Token* place, Token* other)
{
    if (other)
    {
        DEBUG_CHECK(place != other);
        if (place)
        {
            DEBUG_CHECK(!empty());
        }
        else
        {
            DEBUG_CHECK(empty());
            pushBack(other);
        }
    }
}

void TokenList::linkBefore(Token* place, Token* other)
{
    if (other)
    {
        DEBUG_CHECK(place != other);
        if (place)
        {
            DEBUG_CHECK(!empty());
        }
        else
        {
            DEBUG_CHECK(empty());
            pushBack(other);
        }
    }
}

Token* TokenList::popFront()
{
    Token* ret = nullptr;

    if (m_head)
    {
        ret = m_head;

        if (m_head->m_next)
        {
            m_head->m_next->m_prev = nullptr;
            m_head = m_head->m_next;
        }
        else
        {
            m_head = nullptr;
            m_tail = nullptr;
        }

        ret->m_next = nullptr;
        ret->m_prev = nullptr;
    }

    return ret;
}

Token* TokenList::popBack()
{
    Token* ret = nullptr;

    if (m_tail)
    {
        ret = m_tail;

        if (m_tail->m_prev)
        {
            m_tail->m_prev->m_next = nullptr;
            m_tail = m_tail->m_prev;
        }
        else
        {
            m_head = nullptr;
            m_tail = nullptr;
        }

        ret->m_next = nullptr;
        ret->m_prev = nullptr;
    }

    return ret;
}

Token* TokenList::unlinkLine(Token* start, TokenList& outList)
{
    auto cur  = start;
    uint32_t startLine = cur->location().line();
    while (cur != nullptr)
    {
        if (cur->location().line() != startLine)
            break;

        auto next  = cur->next();

        unlink(cur);
        outList.pushBack(cur);

        cur = next;
    }

    return cur;
}

TokenReprinter::TokenReprinter(IFormatStream& f, bool emitLinePragmas)
    : m_emitLinePragmas(emitLinePragmas)
    , m_output(f)
{}

void TokenReprinter::finishCurrentLine()
{
    if (m_currentChar > 0)
    {
        m_output << "\n";
        m_currentLine += 1;
        m_currentChar = 0;
    }
}

void TokenReprinter::emitLinePragma(const Token& token)
{
    if (m_emitLinePragmas)
    {
        m_output.appendf("#line {} \"{}\"\n", token.location().line(), token.location().contextName());
    }

    m_currentFile = token.location().contextName();
    m_currentLine = token.location().line();
    m_currentChar = 0;
}

void TokenReprinter::append(StringView txt)
{
    m_output << txt;
    m_currentChar += txt.length();

    m_prevChar = false;
}

void TokenReprinter::print(const Token& token)
{
    // ignore invalid tokens
    if (!token.valid())
        return;

    // emit the line macros
    if (token.location().contextName() != m_currentFile)
    {
        finishCurrentLine();
        emitLinePragma(token);
    }
    else if (token.location().line() < m_currentLine)
    {
        finishCurrentLine();
        emitLinePragma(token);
    }
    else if (token.location().line() > m_currentLine)
    {
        finishCurrentLine();

        auto numLinesToAdd = token.location().line() - m_currentLine;
        if (m_emitLinePragmas)
        {
            if (numLinesToAdd > 5)
            {
                emitLinePragma(token);
            }
            else
            {
                for (uint32_t i = 0; i < numLinesToAdd; ++i)
                {
                    m_currentLine += 1;
                    m_output << "\n";
                }
            }
        }
        else
        {
            m_currentLine = token.location().line();
        }
    }

    auto intendedPos = token.location().charPos();
    if (intendedPos > m_currentChar)
    {
        m_output.appendPadding(' ', intendedPos - m_currentChar);
        m_currentChar = intendedPos;
    }

    if (!token.isChar() && !m_prevChar)
        m_output << " ";
    m_prevChar = token.isChar();

    if (token.isString())
    {
        m_output << "\"";
        m_output << token.view();
        m_output << "\"";

        m_currentChar += token.view().length() + 2;
    }
    else if (token.isName())
    {
        m_output << "\'";
        m_output << token.view();
        m_output << "\'";

        m_currentChar += token.view().length() + 2;
    }
    else
    {
        m_output << token.view();
        m_currentChar += token.view().length();
    }
}

void TokenList::print(IFormatStream& f) const
{
    TokenReprinter reprinter(f, false);

    auto cur = head();
    while (cur)
    {
        reprinter.print(*cur);
        cur = cur->next();
    }
}

//--

TokenLineReprinter::TokenLineReprinter(IFormatStream& f)
    : m_output(f)
{}

void TokenLineReprinter::print(const Token& token)
{
    // ignore invalid tokens
    if (!token.valid())
        return;

    // add separators between tokens that can't be together
    if (!token.isChar() && !m_prevChar)
        m_output << " ";
    m_prevChar = token.isChar();

    // print
    if (token.isString())
    {
        m_output << "\"";
        m_output << token.view();
        m_output << "\"";
    }
    else if (token.isName())
    {
        m_output << "\'";
        m_output << token.view();
        m_output << "\'";
    }
    else
    {
        m_output << token.view();
    }
}

void TokenLineReprinter::append(StringView txt)
{
    m_output << txt;
    m_prevChar = false;
}

//---

END_INFERNO_NAMESPACE()
