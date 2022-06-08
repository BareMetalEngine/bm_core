/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "private.h"

#include "fileWriterWinApi.h"

#ifdef PLATFORM_WINAPI

BEGIN_INFERNO_NAMESPACE_EX(windows)

//--

FileWriter::FileWriter(FileFlags flags, StringBuf info, HANDLE hSyncHandle)
	: IFileWriter(flags, info)
	, m_hSyncHandle(hSyncHandle)
{
}

FileWriter::~FileWriter()
{
	if (m_hSyncHandle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hSyncHandle);
		m_hSyncHandle = INVALID_HANDLE_VALUE;
	}
}

uint64_t FileWriter::size() const
{
	LARGE_INTEGER size;
	if (!::GetFileSizeEx(m_hSyncHandle, &size))
		return 0;

	return (uint64_t)size.QuadPart;
}

uint64_t FileWriter::pos() const
{
	LARGE_INTEGER pos;
	pos.QuadPart = 0;

	if (!::SetFilePointerEx(m_hSyncHandle, pos, &pos, FILE_CURRENT))
		return 0;

	return (uint64_t)pos.QuadPart;
}

void FileWriter::seek(uint64_t offset)
{
	LARGE_INTEGER pos;
	pos.QuadPart = offset;
	::SetFilePointerEx(m_hSyncHandle, pos, &pos, FILE_BEGIN);
}

uint64_t FileWriter::readSync(void* ptr, uint64_t size)
{
	uint64_t actualRead = 0;

	while (actualRead < size)
	{
		// calculate size of data to read in this block
		const auto readSize = std::min<uint64_t>(size - actualRead, 4U << 20);

		// read data
		DWORD bytesRead = 0;
		if (!ReadFile(m_hSyncHandle, (char*)ptr + actualRead, readSize, &bytesRead, NULL))
			break;

		// accumulate total read count
		actualRead += bytesRead;

		// if not enough data read this pass then exit now
		if (bytesRead != readSize)
			break;
	}

	return actualRead;
}

uint64_t FileWriter::writeSync(const void* ptr, uint64_t size)
{
	uint64_t actualWrite = 0;

	while (actualWrite < size)
	{
		// calculate size of data to write in this block
		const auto writeSize = std::min<uint64_t>(size - actualWrite, 4U << 20);

		// write data
		DWORD bytesWrote = 0;
		if (!WriteFile(m_hSyncHandle, (const char*)ptr + actualWrite, writeSize, &bytesWrote, NULL))
			break;

		// accumulate total read count
		actualWrite += bytesWrote;

		// if not enough data read this pass then exit now
		if (bytesWrote != writeSize)
			break;
	}

	return actualWrite;
}

//--

END_INFERNO_NAMESPACE_EX(windows)

#endif
