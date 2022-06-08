/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

#include "bm_core_parser_glue.inl"

BEGIN_INFERNO_NAMESPACE()

///----

/// type of token found
enum class TextTokenType : uint8_t
{
    Invalid=0,

    String, // double quoted string
    Name, // single quoted string
    IntNumber, // an integer numerical value, NOTE: no sign is allowed, may end with "i" or "I"
    UnsignedNumber, // an unsigned numerical value, we know because it ended with "u" or "U"
    FloatNumber, // a floating point numerical value, NOTE: no sign is allowed, may end with "f"
    Hex, // an hexadecimal constant, can be read as a number
    Char, // a single character from table of simple characters, usually used for {} [] () , + - etc
    Keyword, // a recognized keyword, ie. function, shader, etc whatever was registered in the language
    Identifier, // an identifier (a-z, A-Z, 0-9, _)
    Preprocessor, // a content of perprocessor line, usually beings with # and is eatean till the end
};

///----

class Token;
class TokenList;

class ITextCommentEater;
class ITextLanguageDefinition;
class ITextErrorReporter;

class SimpleLanguageDefinitionBuilder;

class TextParser;
class TextTokenLocation;
class TextTokenWalker;

//-----

/// error reporter for the text parser
class BM_CORE_PARSER_API ITextErrorReporter : public MainPoolData<NoCopy>
{
public:
    virtual ~ITextErrorReporter();
    virtual void reportError(const TextTokenLocation& loc, StringView message) = 0;
    virtual void reportWarning(const TextTokenLocation& loc, StringView message) = 0;

    //--

    static ITextErrorReporter& GetDevNull();
    static ITextErrorReporter& GetDefault();
};

//-----

// a handler for including files
class BM_CORE_PARSER_API ITextIncludeHandler : public MainPoolData<NoCopy>
{
public:
    virtual ~ITextIncludeHandler();

    //----

    // load content of local include
    // "global" include in the <> not ""
    // path - requested path
    // referencePath - path of the file we are calling the #include from 
    // outContent - produced content of the file
    // outPath - reference path for extracted text
    virtual bool loadInclude(bool global, StringView path, StringView referencePath, Buffer& outContent, StringBuf& outPath) = 0;


    //--

    /// get the default "NoIncludes" handler
    static ITextIncludeHandler& GetEmptyHandler();
};

//-----

// internally XML nodes are opaque types
typedef uint64_t NodeHandle;
typedef uint64_t AttributeHandle;

// XML document
class XMLReader;
typedef RefPtr<XMLReader> XMLReaderPtr;

// XML builder
class XMLWriter;
typedef RefPtr<XMLWriter> XMLWriterPtr;

// XML header string
static const inline StringView XML_HEADER_TEXT = "<?xml version=\"1.0\" standalone=\"yes\"?>";

//-----

enum class JSONNodeType
{
    None, // null node, does not output anything
    Value, // node with value
    Compound, // compound node - contains "key": "value" entries
    Array, // array node
};

// read only JSON document
class JSONReader;
typedef RefPtr<JSONReader> JSONReaderPtr;

// writable only JSON document
class JSONWriter;
typedef RefPtr<JSONWriter> JSONWriterPtr;

//------

enum class PrintFlagBit : uint8_t
{
    PrettyText = 0, // Human readable text
    NoHeader = 1, // skip header (XML only), useful when printing small bits
    Relaxed = 2, // skip ',' and other mess, mostly JSON
};

typedef BitFlags<PrintFlagBit> PrintFlags;

//------

END_INFERNO_NAMESPACE();
