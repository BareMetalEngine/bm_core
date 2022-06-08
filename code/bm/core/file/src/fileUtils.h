/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once
#include "bm/core/containers/include/inplaceArray.h"

BEGIN_INFERNO_NAMESPACE()

//----

// on-stack buffer with push/pop functionality for building paths
class TempPathStringBufferUTF16 : public NoCopy
{
public:
	TempPathStringBufferUTF16();
	TempPathStringBufferUTF16(StringView view);

	void reset();

	INLINE operator const wchar_t* () const
	{
		DEBUG_CHECK(m_writePos <= m_writeEnd);
		DEBUG_CHECK(*m_writePos == 0);
		return m_buffer;
	}

	INLINE ArrayView<wchar_t> view() const
	{
		DEBUG_CHECK(m_writePos <= m_writeEnd);
		DEBUG_CHECK(*m_writePos == 0);
		auto count = (m_writePos - m_buffer) + 1;
		return ArrayView<wchar_t>(m_buffer, count);
	}

	INLINE void restore(wchar_t* pos) { m_writePos = pos; }
	INLINE wchar_t* capture() { return m_writePos; }
	INLINE wchar_t* buffer() { return m_buffer; }

	bool append(StringView txt);
	bool append(const wchar_t* txt);

	void print(IFormatStream& f) const;

private:
	static const auto MAX_SIZE = 4096;

	wchar_t m_buffer[MAX_SIZE];
	wchar_t* m_writePos;
	wchar_t* m_writeEnd;
};

//----

class TempPathStringBufferAnsi
{
public:
	TempPathStringBufferAnsi();
	TempPathStringBufferAnsi(StringView view);

	void reset();

	INLINE operator const char* () const
	{
		DEBUG_CHECK(m_writePos <= m_writeEnd);
		DEBUG_CHECK(*m_writePos == 0);
		return m_buffer;
	}

	StringView view() const
	{
		DEBUG_CHECK(m_writePos <= m_writeEnd);
		DEBUG_CHECK(*m_writePos == 0);
		auto count = (m_writePos - m_buffer) + 1;
		return StringView(m_buffer, count);
	}

	INLINE void restore(char* pos) { m_writePos = pos; }
	INLINE char* capture() { return m_writePos; }

	bool append(StringView txt);
	bool append(const wchar_t* txt);

	void print(IFormatStream& f) const;

private:
	static const auto MAX_SIZE = 4096;

	char m_buffer[MAX_SIZE];
	char* m_writePos;
	char* m_writeEnd;
};

//----

// capture active directory on entry and restore it at exit
class PreserveCurrentDirectory : public NoCopy
{
public:
	PreserveCurrentDirectory(StringView dirToSet = ""); // can change directory right away
	~PreserveCurrentDirectory();

private:
	static const uint32_t MAX_STRING = 4096;

#ifndef PLATFORM_WINAPI
	wchar_t m_currentDirectory[MAX_STRING];
#else
	char m_currentDirectory[MAX_STRING];
#endif
};

//----

extern void AppendStringUTF16(Array<wchar_t>& outFormatString, const wchar_t* buf);
extern void AppendStringUTF16(Array<wchar_t>& outFormatString, StringView txt);

extern void AppendFormatStrings(Array<wchar_t>& outFormatString, Array<StringBuf>& outFormatNames, int& outCurrentFilterIndex, const Array<FileFormat>& formats, const StringBuf& currentFilter, bool allowMultipleFormats);

extern void ExtractFilePaths(const wchar_t* resultBuffer, Array<StringBuf>& outPaths);

//----

END_INFERNO_NAMESPACE();
