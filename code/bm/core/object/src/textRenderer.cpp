/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "textRenderer.h"

#include "bm/core/file/include/fileSystemInterface.h"
#include "bm/core/object/include/rttiVariantTable.h"
#include "bm/core/parser/include/textToken.h"
#include "bm/core/parser/include/textSegmentedPrinter.h"
#include "bm/core/parser/include/textLanguageDefinition.h"
#include "bm/core/parser/include/textParser.h"
#include "bm/core/parser/include/public.h"

BEGIN_INFERNO_NAMESPACE()

//--

TextRenderer::TextRenderer(const IFileSystemInterface& loader, ITextErrorReporter& err)
	: m_fileSystem(loader)
	, m_err(err)
{}

TextRenderer::~TextRenderer()
{
}

bool TextRenderer::renderFile(IFormatStream& f, StringView filePath, const VariantTable& params)
{
	// prepare variable stack
	m_rootVariables = &params;
	m_varibleStack.clear();

	// resolve absolute path
	auto absolutePath = StringBuf(filePath);
	m_fileSystem.queryFileAbsolutePath(filePath, absolutePath);

	// load the content
	auto content = m_fileSystem.loadContentToString(filePath);
	if (!content)
	{
		m_err.reportError(TextTokenLocation(absolutePath, 1, 1), TempString("Unable to load content of '{}'", filePath));
		return false;
	}

	// build content
	Content ctx;
	ctx.localPath = StringBuf(filePath);
	ctx.absolutePath = absolutePath;
	ctx.content = content;

	// process the content
	TextSegmentedPrinter printer;
	if (!processContent(printer, ctx))
		return false;

	// export final text
	printer.print(f);
	return true;
}

bool TextRenderer::renderText(IFormatStream& f, StringView text, StringView contextFilePath, const VariantTable& params)
{
	// prepare variable stack
	m_rootVariables = &params;
	m_varibleStack.clear();

	// setup parsing context
	Content ctx;
	ctx.localPath = StringBuf(contextFilePath);
	ctx.absolutePath = ctx.localPath;
	ctx.content = StringBuf(text);

	// process the content
	TextSegmentedPrinter printer;
	if (!processContent(printer, ctx))
		return false;

	// export final text
	printer.print(f);
	return true;
}

//--

bool TextRenderer::processContent(TextSegmentedPrinter& f, const Content& content)
{
	// setup parser
	TextParser parser(content.absolutePath, m_err, ITextCommentEater::NoComments());
	parser.reset(content.content);

	// process while we end
	for (;;)
	{
		// get start of the block
		const auto* start = parser.pos();

		// block ?
		char tagType = 0;
		StringView tagContent;
		TextTokenLocation tagLocation;
		StringView skippedContent;
		if (parser.findDjangoTag(skippedContent, tagContent, tagType, tagLocation))
		{
			f << skippedContent;

			if (tagType == '#')
			{
				if (!processTagComment(f, tagContent, tagLocation))
					return false;
			}
			else if (tagType == '{')
			{
				if (!processTagVariable(f, tagContent, tagLocation))
					return false;
			}
			else if (tagType == '%')
			{
				if (!processTagCommand(f, tagContent, tagLocation))
					return false;
			}
			else
			{
				m_err.reportError(tagLocation, TempString("Unknown tag scope '{}'", tagType));
			}
		}
		else
		{
			if (skippedContent.empty())
				break;
			f << skippedContent;
		}
	}

	// all content processed
	return true;
}

bool TextRenderer::processTagComment(TextSegmentedPrinter& f, StringView tag, const TextTokenLocation& loc)
{
	return true;
}

TypedMemory TextRenderer::evaluateVar(StringView txt) const
{
	for (auto i : m_varibleStack.indexRange().reversed())
	{
		auto table = m_varibleStack[i].view();
		if (auto view = table[txt])
			return view;
	}

	return m_rootVariables->view()[txt];
}

bool TextRenderer::evaluateVar(StringParser& line, const TextTokenLocation& loc, TypedMemory& outValue) const
{
	// find variable name, we always must have it specified directly
	StringView name;
	if (!line.parseIdentifier(name))
	{
		m_err.reportError(loc, TempString("Expected variable name"));
		return false;
	}

	// find variable data
	auto var = evaluateVar(name);
	if (!var)
	{
		m_err.reportError(loc, TempString("Unknown variable '{}'", var));
		return false;
	}

	// refine
	for (;;)
	{
		// member
		if (line.parseKeyword("."))
		{
			// not a dictionary
			if (!var.isDictionary())
			{
				m_err.reportError(loc, TempString("Variable '{}' is not a dictionary", name));
				return false;
			}

			// find member name
			// TODO: sub evaluation via ``
			StringView memberName;
			if (!line.parseIdentifier(memberName))
			{
				m_err.reportError(loc, TempString("Expected member name at variable '{}'", name));
				return false;
			}

			// get member
			auto oldVar = var;
			var = var[memberName];
			if (!var)
			{
				m_err.reportError(loc, TempString("Variable {} (current type {}) does not contain member '{}'",
					name, oldVar.unwrappedType(), memberName));
				return false;
			}

			continue;
		}

		// array access
		if (line.parseKeyword("["))
		{
			// not a dictionary
			if (!var.isArray())
			{
				m_err.reportError(loc, TempString("Variable '{}' is not an array", name));
				return false;
			}

			// parse array index
			// TODO: sub evaluation via ``
			uint32_t index = 0;
			if (!line.parseUint32(index))
			{
				m_err.reportError(loc, TempString("Expected array index at variable '{}'", name));
				return false;
			}

			// get member
			auto oldVar = var;
			var = var[index];
			if (!var)
			{
				m_err.reportError(loc, TempString("Variable {} (current type {}) does not have index {} (size: {})",
					name, oldVar.unwrappedType(), index, oldVar.arraySize()));
				return false;
			}

			// continue
			continue;
		}

		// nothing
		break;
	}

	// valid
	outValue = var;
	return true;
}

bool TextRenderer::processTagVariable(TextSegmentedPrinter& f, StringView tag, const TextTokenLocation& loc)
{
	StringParser line(tag);

	// evaluate 
	TypedMemory value;
	if (!evaluateVar(line, loc, value))
		return false;

	// make sure it's printable
	if (!value)
	{
		m_err.reportWarning(loc, TempString("No value for variable '{}'", tag));
		f << "(null variable)";
		return true;
	}
	else if (value.isArray())
	{
		m_err.reportWarning(loc, TempString("Value for variable '{}' is an array, can't display directly", tag));
		f << "(array variable)";
		return true;
	}
	else if (value.isDictionary())
	{
		m_err.reportWarning(loc, TempString("Value for variable '{}' is a dictionary, can't display directly", tag));
		f << "(dictionary variable)";
		return true;
	}

	// print
	value.print(f);
	return true;
}

bool TextRenderer::processTagCommand(TextSegmentedPrinter& f, StringView tag, const TextTokenLocation& loc)
{
	StringParser line(tag);

	// parse command name
	StringView name;
	if (!line.parseIdentifier(name))
	{
		m_err.reportError(loc, TempString("Expected command name"));
		return false;
	}

	// not found
	m_err.reportError(loc, TempString("Unknown command '{}'", name));
	return false;
}

//--

END_INFERNO_NAMESPACE()
