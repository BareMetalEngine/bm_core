/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "fileView.h"
#include "fileUtils.h"
#include "fileFormat.h"

#include "bm/core/containers/include/utf8StringFunctions.h"

BEGIN_INFERNO_NAMESPACE()

//--

TempPathStringBufferUTF16::TempPathStringBufferUTF16()
{
	reset();
}

TempPathStringBufferUTF16::TempPathStringBufferUTF16(StringView view)
{
	reset();
	append(view);
}

void TempPathStringBufferUTF16::reset()
{
	m_writePos = m_buffer;
	m_writeEnd = m_buffer + MAX_SIZE - 1;
	*m_writePos = 0;
}

template< typename T >
static T AdjustPathChar(T ch)
{
#ifdef PLATFORM_WINDOWS
	if (ch == '/')
		ch = '\\';
#else
	if (ch == '\\')
		ch = '/';
#endif
	return ch;
}

bool TempPathStringBufferUTF16::append(StringView txt)
{
	DEBUG_CHECK(m_writePos <= m_writeEnd);
	DEBUG_CHECK(*m_writePos == 0);

	auto start = m_writePos;

	auto ptr = txt.data();
	auto endPtr = txt.data() + txt.length();
	while (ptr < endPtr)
	{
		auto ch = utf8::NextChar(ptr, endPtr);
		ch = AdjustPathChar(ch);

		if (m_writePos < m_writeEnd)
		{
			*m_writePos++ = (wchar_t)ch;
			*m_writePos = 0;
		}
		else
		{
			m_writePos = start;
			m_writePos[1] = 0;
			return false;
		}
	}

	return true;
}

bool TempPathStringBufferUTF16::append(const wchar_t* txt)
{
	DEBUG_CHECK(m_writePos <= m_writeEnd);
	DEBUG_CHECK(*m_writePos == 0);

	if (txt && *txt)
	{
		const auto len = wcslen(txt);
		if (m_writePos + len < m_writeEnd)
		{
			while (*txt)
			{
				auto ch = *txt++;
				ch = AdjustPathChar(ch);
				*m_writePos++ = ch;
			}

			*m_writePos = 0;
		}
		else
		{
			return false;
		}
	}

	return true;
}

void TempPathStringBufferUTF16::print(IFormatStream& f) const
{
	f.append(m_buffer, m_writePos - m_buffer);
}

//--

TempPathStringBufferAnsi::TempPathStringBufferAnsi()
{
	reset();
}

TempPathStringBufferAnsi::TempPathStringBufferAnsi(StringView view)
{
	append(view);
}

void TempPathStringBufferAnsi::reset()
{
	m_writePos = m_buffer;
	m_writeEnd = m_buffer + MAX_SIZE - 1;
	*m_writePos = 0;
}

bool TempPathStringBufferAnsi::append(StringView txt)
{
	DEBUG_CHECK(m_writePos <= m_writeEnd);
	DEBUG_CHECK(*m_writePos == 0);

	auto start = m_writePos;

	auto ptr = txt.data();
	auto endPtr = txt.data() + txt.length();
	while (ptr < endPtr)
	{
		auto ch = *ptr++;
		ch = AdjustPathChar(ch);

		if (m_writePos < m_writeEnd)
		{
			*m_writePos++ = ch;
			*m_writePos = 0;
		}
		else
		{
			m_writePos = start;
			m_writePos[1] = 0;
			return false;
		}
	}

	return true;
}

bool TempPathStringBufferAnsi::append(const wchar_t* txt)
{
	DEBUG_CHECK(m_writePos <= m_writeEnd);
	DEBUG_CHECK(*m_writePos == 0);

	if (txt && *txt)
	{
		auto* org = m_writePos;
		while (*txt)
		{
			auto ch = *txt++;

			ch = AdjustPathChar(ch);

			char data[8];
			auto size = utf8::ConvertChar(data, ch);

			if (m_writePos + size < m_writeEnd)
			{
				memcpy(m_writePos, data, size);
				m_writePos += size;
			}
			else
			{
				m_writePos = org;
				*m_writePos = 0;
				return false;
			}
		}

		*m_writePos = 0;
	}

	return true;
}

void TempPathStringBufferAnsi::print(IFormatStream& f) const
{
	f.append(m_buffer, m_writePos - m_buffer);
}

//--

PreserveCurrentDirectory::PreserveCurrentDirectory(StringView dirToSet)
{
#ifndef PLATFORM_WINAPI
	GetCurrentDirectoryW(MAX_STRING, m_currentDirectory);

	if (dirToSet)
	{
		TempPathStringBufferUTF16 cstr(dirToSet);
		SetCurrentDirectoryW(cstr);
	}
#else

#endif
}

PreserveCurrentDirectory::~PreserveCurrentDirectory()
{
#ifndef PLATFORM_WINAPI
		SetCurrentDirectoryW(m_currentDirectory);
#endif
}

//--

void AppendStringUTF16(Array<wchar_t>& outFormatString, const wchar_t* buf)
{
	while (*buf)
	{
		auto ch = *buf++;
		ch = AdjustPathChar(ch);
		outFormatString.pushBack(ch);
	}
}

void AppendStringUTF16(Array<wchar_t>& outFormatString, StringView txt)
{
	auto ptr = txt.data();
	auto endPtr = txt.data() + txt.length();
	while (ptr < endPtr)
	{
		auto ch = utf8::NextChar(ptr, endPtr);
		ch = AdjustPathChar(ch);
		outFormatString.pushBack(ch);
	}
}

void AppendFormatStrings(Array<wchar_t>& outFormatString, Array<StringBuf>& outFormatNames, int& outCurrentFilterIndex, const Array<FileFormat>& formats, const StringBuf& currentFilter, bool allowMultipleFormats)
{
	int currentFilterIndex = 1;

	if (!formats.empty())
	{
		if (allowMultipleFormats && (formats.size() > 1))
		{
			StringBuilder formatDisplayString;
			StringBuilder formatFilterString;

			if (currentFilter == "AllSupported" || currentFilter == "")
				outCurrentFilterIndex = currentFilterIndex;

			for (const auto& format : formats)
			{
				if (!formatFilterString.empty())
					formatFilterString.append(";");
				formatFilterString.appendf("*.{}", format.extension());

				if (!formatDisplayString.empty())
					formatDisplayString.append(", ");
				formatDisplayString.appendf("*.{}", format.extension());
			}

			// all supported files
			AppendStringUTF16(outFormatString, L"All supported files [");
			AppendStringUTF16(outFormatString, formatDisplayString.c_str());
			AppendStringUTF16(outFormatString, L"]");
			outFormatString.pushBack(0);

			AppendStringUTF16(outFormatString, formatFilterString.c_str());
			outFormatString.pushBack(0);

			outFormatNames.pushBack(StringBuf("AllSupported"));
			currentFilterIndex += 1;
		}

		for (const auto& format : formats)
		{
			if (format.extension() == currentFilter)
				outCurrentFilterIndex = currentFilterIndex;

			AppendStringUTF16(outFormatString, format.description().c_str());
			AppendStringUTF16(outFormatString, L" [*.");
			AppendStringUTF16(outFormatString, format.extension().c_str());
			AppendStringUTF16(outFormatString, L"]");
			outFormatString.pushBack(0);

			AppendStringUTF16(outFormatString, L"*.");
			AppendStringUTF16(outFormatString, format.extension().c_str());
			outFormatString.pushBack(0);

			outFormatNames.pushBack(format.extension());
			currentFilterIndex += 1;
		}
	}

	if (/*allowMultipleFormats || */formats.empty())
	{
		if (currentFilter == "All")
			outCurrentFilterIndex = currentFilterIndex;

		AppendStringUTF16(outFormatString, L"All files");
		outFormatString.pushBack(0);
		AppendStringUTF16(outFormatString, L"*.*");

		outFormatNames.pushBack(StringBuf("All"));
		currentFilterIndex += 1;
	}

	// end of format list
	outFormatString.pushBack(0);
	outFormatString.pushBack(0);
}

void ExtractFilePaths(const wchar_t* resultBuffer, Array<StringBuf>& outPaths)
{
	Array<const wchar_t*> parts;

	// Parse file names
	auto pos = resultBuffer;
	auto start = resultBuffer;
	while (1)
	{
		if (0 == *pos)
		{
			if (pos == start)
				break;

			parts.pushBack(start);
			pos += 1;
			start = pos;
		}
		else
		{
			++pos;
		}
	}

	if (parts.size() == 1)
	{
		// single path case
		outPaths.emplaceBack(parts[0]);
	}
	else if (parts.size() > 1)
	{
		// in multiple paths case the first entry is the directory path and the rest are the file names
		for (uint32_t i = 1; i < parts.size(); ++i)
		{
			StringBuilder txt;
			txt << parts[0];
			txt << "\\";
			txt << parts[i];
			outPaths.emplaceBack(txt);
		}
	}
}

//---

END_INFERNO_NAMESPACE()
