/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "private.h"

#include "fileReaderWinApi.h"
#include "fileUnbufferedDiskViewWinApi.h"

#ifdef PLATFORM_WINAPI

BEGIN_INFERNO_NAMESPACE_EX(windows)

//--

FileUnbufferedDiskView::FileUnbufferedDiskView(FileFlags flags, StringBuf info, FileAbsoluteRange range, HANDLE hSyncHandle, FileReader* owner)
	: IFileView(flags, info, range)
	, m_hSyncHandle(hSyncHandle)
	, m_owner(owner)
{
	m_absoluteOffset = range.absoluteStart();
}

FileUnbufferedDiskView::~FileUnbufferedDiskView()
{
	if (auto owner = m_owner.lock())
		owner->releaseSyncHandle(m_hSyncHandle);
	else
		CloseHandle(m_hSyncHandle);
}

uint64_t FileUnbufferedDiskView::offset() const
{
	return m_absoluteOffset;
}

void FileUnbufferedDiskView::seek(uint64_t offset)
{
	if (offset != m_absoluteOffset)
	{
		m_absoluteOffsetChanged = true;
		m_absoluteOffset = offset;
	}
}

bool FileUnbufferedDiskView::syncFileOffset()
{
	LARGE_INTEGER pos;
	pos.QuadPart = m_absoluteOffset;

	if (!::SetFilePointerEx(m_hSyncHandle, pos, &pos, FILE_BEGIN))
		return false;

	m_absoluteOffsetChanged = false;
	return true;
}

uint64_t FileUnbufferedDiskView::readSync(void* readBuffer, uint64_t size)
{
	uint64_t actualRead = 0;

	if (size)
	{
		// move to new file position
		if (!m_absoluteOffsetChanged || syncFileOffset())
		{
			// read content
			while (actualRead < size)
			{
				// calculate size of data to read in this block
				const auto readSize = std::min<uint64_t>(size - actualRead, 2ULL << 30);

				// read data
				DWORD bytesRead = 0;
				if (!ReadFile(m_hSyncHandle, (char*)readBuffer + actualRead, readSize, &bytesRead, NULL))
					break;

				// accumulate total read count
				actualRead += bytesRead;

				// if not enough data read this pass then exit now
				if (bytesRead != readSize)
					break;
			}
		}
	}

	return actualRead;
}

//--

END_INFERNO_NAMESPACE_EX(windows)

#endif
