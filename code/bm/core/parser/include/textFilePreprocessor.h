/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
*
* [#filter: parser #]
***/

#pragma once

#include "textToken.h"
#include "textLanguageDefinition.h"

#include "bm/core/memory/include/localAllocator.h"
#include "bm/core/containers/include/hashMap.h"
#include "bm/core/containers/include/hashSet.h"
#include "bm/core/containers/include/inplaceArray.h"

BEGIN_INFERNO_NAMESPACE()

///----

// helper class for "C style" processing text
// optionally can eat the comments as well
class BM_CORE_PARSER_API TextFilePreprocessor : public MainPoolData<NoCopy>
{
public:
    TextFilePreprocessor(LocalAllocator& mem, ITextIncludeHandler& includeHandler, ITextErrorReporter& errorHandler, ITextCommentEater& commentEater, const ITextLanguageDefinition& parentLanguage);
    ~TextFilePreprocessor();

    //--

    INLINE TokenList& tokens() { return m_finalTokens; }
    INLINE const TokenList& tokens() const { return m_finalTokens; }

    //--

    /// add a global define
    bool defineSymbol(StringView name, StringView value);

    /// process provided text content
    bool processContent(StringView content, StringView contextPath);

protected:
    LocalAllocator& m_allocator;

    ITextIncludeHandler& m_includeHandler;
    ITextErrorReporter& m_errorHandler;
    ITextCommentEater& m_commentEater;

    const ITextLanguageDefinition& m_parentLanguage;

    bool processFileContent(StringView content, StringView contextPath, TokenList& outTokenList);

    bool processPreprocessorDeclaration(TokenList& line, bool& keepProcessing);
    bool processDefine(const Token* head, TokenList& line);
    bool processInclude(const Token* head, TokenList& line);
    bool processUndef(const Token* head, TokenList& line);
    bool processIf(const Token* head, TokenList& line);
    bool processIfdef(const Token* head, TokenList& line, bool swap = false);
    bool processElif(const Token* head, TokenList& line);
    bool processElse(const Token* head, TokenList& line);
    bool processEndif(const Token* head, TokenList& line);
    bool processPragma(const Token* head, TokenList& line, bool& keepProcessing);
    bool processError(const Token* head, TokenList& line);
    bool processMessage(const Token* head, TokenList& line);

    bool extractArgumentList(TokenList& line, Array<StringBuf>& outArguments) const;

    HashSet<StringBuf> m_pragmaOnceFilePaths;
    Array<StringBuf> m_currentIncludeStack;
    Array<Buffer> m_loadedIncludeBuffers;

    TokenList m_finalTokens;

    //--

    struct MacroDefinition
    {
        StringBuf m_name;
        TextTokenLocation m_definedAt;
        Array<StringBuf> m_arguments;
        Array<StringBuf> m_values;
        TokenList m_replacement;
        bool m_hasArguments = false;
        bool m_defined = false;
    };

    HashMap<StringBuf, MacroDefinition*> m_defineMap;
    Array<MacroDefinition*> m_defineList;

    //--

    struct MacroArgument
    {
        StringBuf m_name;
        TokenList m_tokens;
    };

    struct MacroPlacement
    {
        const Token* m_placementRef = nullptr;
        const MacroDefinition* m_def = nullptr;
        InplaceArray<MacroArgument, 16> m_arguments;

        const MacroArgument* arg(const Token* token) const;
    };

    bool shouldExpandMacro(TokenList& list, MacroPlacement& macroList, bool fromParamsOnly=false);

    bool expandAllPossibleMacros(TokenList& list, TokenList& outputList, bool fromParamsOnly=false);
    bool expandMacro(const MacroPlacement& macro, TokenList& outputList);
    bool evaluateExpression(TokenList& list, bool& result);

    bool extractReplacementListForArgument(const Token* start, TokenList& list, TokenList& outputList, bool& outEnd);

    bool macroInitialExpand(const MacroPlacement& macro, TokenList& outputList);
    bool macroSringify(const MacroPlacement& macro, TokenList& inputList, TokenList& outputList);
    bool macroReplaceParameters(const MacroPlacement& macro, TokenList& inputList, TokenList& outputList);
    bool macroConcatenate(const MacroPlacement& macro, TokenList& inputList, TokenList& outputList);

    //--

    struct OutputFilterStack
    {
        const Token* m_token = nullptr;
        bool m_enabled = false;
        bool m_hadElse = false;
        bool m_anyTaken = false;
    };

    Array<OutputFilterStack> m_filterStack;
    bool evalFilterFlag() const;
    bool evalParentFilterFlag() const;

    //bool m_filterStatus;

    //--

    MacroDefinition* define(StringView name) const;
    MacroDefinition* createDefine(StringView name);

    Token* copyToken(const Token* source, const Token* baseLocataion = nullptr);
    void copyTokens(const TokenList& list, TokenList& outList, const Token* baseLocataion = nullptr, bool isMacroArgument = false);
    bool createTokens(const Token* baseLocataion, StringView text, TokenList& outList);

    //--
};

///----

END_INFERNO_NAMESPACE()