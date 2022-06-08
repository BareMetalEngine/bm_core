/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

#include "bm/core/object/include/rttiVariantTable.h"

BEGIN_INFERNO_NAMESPACE()

//---

class TextSegmentedPrinter;

/// Render text (Django style) into output string formatter
class BM_CORE_OBJECT_API TextRenderer : public MainPoolData<NoCopy>
{
public:
    TextRenderer(const IFileSystemInterface& loader, ITextErrorReporter& err);
    ~TextRenderer();

    bool renderFile(IFormatStream& f, StringView filePath, const VariantTable& params);
    bool renderText(IFormatStream& f, StringView text, StringView filePath, const VariantTable& params);

private:
    ITextErrorReporter& m_err;
    const IFileSystemInterface& m_fileSystem;
    const VariantTable* m_rootVariables = nullptr;    

    struct Content
    {
        StringBuf localPath;
        StringBuf absolutePath;
        StringBuf content;
    };

    struct Stack
    {
        VariantTable vars;
		bool enabled = false;
		bool hadElse = false;
		bool anyTaken = false;
    };

    Array<VariantTable> m_varibleStack;
    UniquePtr<TextSegmentedPrinter> m_printer;

    TypedMemory evaluateVar(StringView txt) const;
	bool evaluateVar(StringParser& p, const TextTokenLocation& loc, TypedMemory& outValue) const;

    bool processContent(TextSegmentedPrinter& f, const Content& content);

    bool processTagComment(TextSegmentedPrinter& f, StringView tag, const TextTokenLocation& loc);
    bool processTagVariable(TextSegmentedPrinter& f, StringView tag, const TextTokenLocation& loc);
    bool processTagCommand(TextSegmentedPrinter& f, StringView tag, const TextTokenLocation& loc);
};

//---

END_INFERNO_NAMESPACE()


