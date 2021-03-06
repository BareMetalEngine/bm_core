/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
*
* [#filter: parser #]
***/

#pragma once

#include "bm/core/containers/include/stringBuf.h"

BEGIN_INFERNO_NAMESPACE()

///----

/// location information for a token
class BM_CORE_PARSER_API TextTokenLocation
{
public:
    INLINE TextTokenLocation()
        : m_line(0)
        , m_pos(0)
    {}

    INLINE TextTokenLocation(const StringBuf& contextName, const uint32_t& line=0, const uint32_t& pos=0)
        : m_contextName(contextName)
        , m_line(line)
        , m_pos(pos)
    {}

    INLINE TextTokenLocation(const TextTokenLocation& txt) = default;
    INLINE TextTokenLocation(TextTokenLocation&& txt) = default;
    INLINE TextTokenLocation& operator=(const TextTokenLocation& txt) = default;
    INLINE TextTokenLocation& operator=(TextTokenLocation&& txt) = default;

    //--

    // get the context name (usually a file name)
    INLINE const StringBuf& contextName() const { return m_contextName; }

    // get the line index in the file (0 if unknown)
    INLINE uint32_t line() const { return m_line; }

    // get the char position in the line (0 if unknown)
    INLINE uint32_t charPos() const { return m_pos; }

    //--

    // get a string representation (for debug and error printing)
    void print(IFormatStream& f) const;

private:
    StringBuf m_contextName; // context name
    uint32_t m_line; // line number, if known
    uint32_t m_pos; // position in line, if known
};

///----

/// string view of the token
class BM_CORE_PARSER_API Token
{
public:
    INLINE Token(TextTokenType type, const char* str, const char* endStr, int keywordID)
        : m_type(type)
        , m_keywordID(keywordID)
        , m_str(str)
        , m_end(endStr)
    {}

    INLINE Token(const Token& base)
        : m_type(base.m_type)
        , m_keywordID(base.m_keywordID)
        , m_str(base.m_str)
        , m_end(base.m_end)
    {}

    // base
    INLINE Token() = default;
    INLINE Token(Token&& base) = default;
    INLINE Token& operator=(const Token& base) = delete;
    INLINE Token& operator=(Token&& base) = default;

    // bind token to location
    // NOTE: this is a separate function because location comes from other source than the token itself
    INLINE void assignLocation(const TextTokenLocation& loc) { m_location = loc; }

    /// is this a valid token ?
    INLINE bool valid() const { return m_type != TextTokenType::Invalid; }

    /// is this a string ?
    INLINE bool isString() const { return m_type == TextTokenType::String; }

    /// is this a single quoted string ?
    INLINE bool isName() const { return m_type == TextTokenType::Name; }

    /// is this a number ?
    INLINE bool isUnsigned() const { return m_type == TextTokenType::UnsignedNumber; }

    /// is this an integer number ?
    INLINE bool isInteger() const { return m_type == TextTokenType::IntNumber; }

    /// is this a hexadecimal number
    INLINE bool isHex() const { return m_type == TextTokenType::Hex; }

    /// is this a floating point number ?
    INLINE bool isFloat() const { return m_type == TextTokenType::FloatNumber; }

    /// is this a character ?
    INLINE bool isChar() const { return m_type == TextTokenType::Char; }

    /// is this keyword ?
    INLINE bool isKeyword() const { return m_type == TextTokenType::Keyword; }

    /// is this an identifier ?
    INLINE bool isIdentifier() const { return m_type == TextTokenType::Identifier; }

    /// is this a preprocesor token
    INLINE bool isPreprocessor() const { return m_type == TextTokenType::Preprocessor; }

    //--

    /// get length of the text data
	INLINE uint32_t length() const { return range_cast<uint32_t>(m_end - m_str); }

    /// get location of ths token
    INLINE const TextTokenLocation& location() const { return m_location; }

    /// get the keyword ID (as registered previously in the parser)
    INLINE int keywordID() const { return m_keywordID; }

    /// get the single character (usually used for the Char tokens)
    INLINE char ch() const { return (m_type == TextTokenType::Char && m_str) ? *m_str : 0; }

    /// get type of token
    INLINE TextTokenType type() const { return m_type; }

    ///--

    /// is this a head token ?
    INLINE bool head() const { return (m_prev == nullptr); }

    /// is this a tail token ?
    INLINE bool tail() const { return (m_next == nullptr); }

    /// get next token
    INLINE Token* next() const { return m_next; }

    /// get previous token
    INLINE Token* prev() const { return m_prev; }

    //--

    /// get the full string of the token range, slow, must allocate memory, use as a final step
    StringBuf string() const;

    /// get name of the type of the token
    const char* typeName() const;

    /// get the string view
    StringView view() const;

    /// get the numerical value
    double floatNumber() const;

    // get the numerical value
    int64_t integerNumber() const;

    // get the numerical value
    uint64_t unsignedNumber() const;

    // get a number from hex value
    uint64_t hexToNumber() const;

    //--

    // print to log
    void print(IFormatStream& f) const;

private:
    TextTokenType m_type = TextTokenType::Invalid;
    int m_keywordID = -1;
    const char* m_str = nullptr;
    const char* m_end = nullptr;
    TextTokenLocation m_location;
    Token* m_next = nullptr;
    Token* m_prev = nullptr;

    friend class TokenList;

public:
    bool m_hackFromMacroArguments = false;
};

///----

/// list of tokens
class BM_CORE_PARSER_API TokenList
{
public:
    INLINE TokenList() = default;
    INLINE TokenList(const TokenList& other) = delete;
    INLINE TokenList& operator=(const TokenList & other) = delete;

    INLINE TokenList(TokenList&& other)
        : m_head(other.m_head)
        , m_tail(other.m_tail)
    {
        other.m_head = nullptr;
        other.m_tail = nullptr;
    }

    INLINE TokenList& operator=(TokenList&& other)
    {
        if (&other != this)
        {
            m_head = other.m_head;
            m_tail = other.m_tail;
            other.m_head = nullptr;
            other.m_tail = nullptr;
        }
        return *this;
    }

    INLINE Token* head() const { return m_head; }
    INLINE Token* tail() const { return m_tail; }
    INLINE bool empty() const { return m_head == nullptr; }

    void clear();

    void pushBack(Token* token);
    void pushFront(Token* token);

    void pushBack(TokenList&& list);
    void pushFront(TokenList&& list);

    Token* popFront();
    Token* popBack();

    Token* unlink(Token* token); // returns next valid token
    void linkAfter(Token* place, Token* other);
    void linkBefore(Token* place, Token* other);

    Token* unlinkLine(Token* start, TokenList& outList); // returns next token after the line

    void print(IFormatStream& f) const;

private:
    Token* m_head = nullptr;
    Token* m_tail = nullptr;
};

///----

/// helper class that rebuild readable and formated text from original tokens
class BM_CORE_PARSER_API TokenReprinter : public MainPoolData<NoCopy>
{
public:
    TokenReprinter(IFormatStream& f, bool emitLinePragmas);

    // print token to output stream, will automatically insert new lines and spaces to match the location of original token
    void print(const Token& token);

    // append normal string
    void append(StringView txt);

private:
    StringBuf m_currentFile;
    uint32_t m_currentLine = 0;
    uint32_t m_currentChar = 0;
    bool m_prevChar = true;

    bool m_emitLinePragmas = false;
    IFormatStream& m_output;

    void finishCurrentLine();
    void emitLinePragma(const Token& token);
};

///----

/// helper class that reprints tokens into a single line
class BM_CORE_PARSER_API TokenLineReprinter : public MainPoolData<NoCopy>
{
public:
    TokenLineReprinter(IFormatStream& f);

    // print token to output stream, will automatically insert new lines and spaces to match the location of original token
    void print(const Token& token);

    // append normal string
    void append(StringView txt);

private:
    bool m_prevChar = true;
    IFormatStream& m_output;
};

///---

END_INFERNO_NAMESPACE()
