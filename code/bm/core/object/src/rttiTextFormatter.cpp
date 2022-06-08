/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "rttiTextFormater.h"

BEGIN_INFERNO_NAMESPACE()

//--

IDataTextFormatter::IDataTextFormatter(DataTextFormatterFlags flags /*= DataTextFormatterFlagBit::Relaxed*/)
	: m_flags(flags)
{}

IDataTextFormatter::~IDataTextFormatter()
{}

//--

void DataTextFormatter_JSON::makeNewLine(IFormatStream& f)
{
	if (m_flags.test(DataTextFormatterFlagBit::UseNewLines))
	{
		f << "\n";

		if (m_flags.test(DataTextFormatterFlagBit::UseIndentation))
			f.appendPadding(' ', 4 * m_indentation);
	}

	m_needsNewLine = false;
}

void DataTextFormatter_JSON::makeNewLineIfNeeded(IFormatStream& f)
{
	if (m_needsNewLine)
		makeNewLine(f);
}

void DataTextFormatter_JSON::makeElementSeparator(IFormatStream& f)
{
	if (m_needsSeparator.back())
	{
		const auto relaxed = m_flags.test(DataTextFormatterFlagBit::Relaxed);
		const auto useNewLine = m_flags.test(DataTextFormatterFlagBit::UseNewLines);

		if (relaxed)
			f << " ";
		else
			f << (useNewLine ? "," : ", ");

		m_needsSeparator.back() = false;
	}

	m_needsNewLine = true;
}

void DataTextFormatter_JSON::makeBlockStart(IFormatStream& f, const char* ch)
{
	f << ch;

	m_indentation += 1;
	m_needsNewLine = true;

	m_needsSeparator.pushBack(false);
}

void DataTextFormatter_JSON::makeBlockEnd(IFormatStream& f, const char* ch)
{
	ASSERT(!m_needsSeparator.empty());
	m_needsSeparator.popBack();

	m_indentation -= 1;
	makeNewLineIfNeeded(f);

	f << ch;
}

//--

void DataTextFormatter_JSON::beingArray(IFormatStream& f)
{
	makeBlockStart(f, "[");
}

void DataTextFormatter_JSON::endArray(IFormatStream& f)
{
	makeBlockEnd(f, "]");
}

void DataTextFormatter_JSON::beingArrayElement(IFormatStream& f)
{
	makeElementSeparator(f);
	makeNewLineIfNeeded(f);
}

void DataTextFormatter_JSON::endArrayElement(IFormatStream& f)
{
	m_needsSeparator.back() = true;
	m_needsNewLine = true;
}

void DataTextFormatter_JSON::beginStruct(IFormatStream& f)
{
	makeBlockStart(f, "{");
}

void DataTextFormatter_JSON::endStruct(IFormatStream& f)
{
	makeBlockEnd(f, "}");
}

namespace prv
{

	static bool JSonNeedsQuotes(StringView view)
	{
		const auto* pos = view.data();
		const auto* end = pos + view.length();

		while (pos < end)
		{
			const auto ch = *pos++;

			switch (ch)
			{
			case '"':
			case '\\':
			case '/':
			case '[':
			case ']':
			case '{':
			case '}':
			case '\'':
			case '`':
				return true;

			default:
				if (ch <= ' ')
					return true;
			}
		}

		return view.empty(); // empty strings need quotes
	}

	static void AppendJSon(IFormatStream& f, StringView view, bool forceQuotes)
	{
		bool needsQuotes = forceQuotes || JSonNeedsQuotes(view);

		if (needsQuotes)
			f.append("\"");

		const auto* pos = view.data();
		const auto* end = pos + view.length();

		while (pos < end)
		{
			const auto ch = *pos++;

			switch (ch)
			{
			case '\\':
				f.append("\\\\");
				break;
			case '"':
				f.append("\\\"");
				break;
			case '/':
				f.append("\\/");
				break;
			case '\b':
				f.append("\\b");
				break;
			case '\t':
				f.append("\\t");
				break;
			case '\n':
				f.append("\\n");
				break;
			case '\f':
				f.append("\\f");
				break;
			case '\r':
				f.append("\\r");
				break;
			default:
				if (ch < ' ') {
					uint16_t code = ch;
					f.appendf("\\{}", Hex(code));
				}
				else
				{
					f.appendch(ch);
				}
			}
		}

		if (needsQuotes)
			f.append("\"");
	}
} // prv


void DataTextFormatter_JSON::beginStructElement(IFormatStream& f, StringView name)
{
	makeElementSeparator(f);
	makeNewLineIfNeeded(f);

	const auto relaxed = m_flags.test(DataTextFormatterFlagBit::Relaxed);
	prv::AppendJSon(f, name, !relaxed);

	f << ": ";
}

void DataTextFormatter_JSON::endStructElement(IFormatStream& f, StringView nam)
{
	m_needsSeparator.back() = true;
	m_needsNewLine = true;
}

void DataTextFormatter_JSON::beginValue(IFormatStream& f, Type type)
{

}

void DataTextFormatter_JSON::endValue(IFormatStream& f, Type type)
{

}

void DataTextFormatter_JSON::printValue(IFormatStream& f, StringView value)
{
	const auto relaxed = m_flags.test(DataTextFormatterFlagBit::Relaxed);
	prv::AppendJSon(f, value, !relaxed);
}

void DataTextFormatter_JSON::printBuffer(IFormatStream& f, const void* data, uint32_t size)
{
	// TOdO
}

//--

END_INFERNO_NAMESPACE()
