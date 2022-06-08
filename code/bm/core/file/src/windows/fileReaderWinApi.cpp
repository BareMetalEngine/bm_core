/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "private.h"

#include "asyncDispatcherWinApi.h"
#include "fileReaderWinApi.h"
#include "fileUnbufferedDiskViewWinApi.h"
#include "fileMemoryMappedViewWinApi.h"

#include "bm/core/task/include/taskSignal.h"

#ifdef PLATFORM_WINAPI

BEGIN_INFERNO_NAMESPACE_EX(windows)

//--

FileReader::FileReader(FileFlags flags, StringBuf info, const wchar_t* openFilePath, HANDLE hAsyncFile, uint64_t size, AsyncReadDispatcher* dispatcher, HANDLE hMemoryMapped)
    : IFileReader(flags, info, size)
    , m_hAsyncHandle(hAsyncFile)
	, m_hMemoryMappedHandle(hMemoryMapped)
    , m_asyncDispatcher(dispatcher)
    , m_syncFilePath(openFilePath)
{}

FileReader::~FileReader()
{
    for (auto handle : m_syncHandles)
        CloseHandle(handle);
    
    if (m_hMemoryMappedHandle)
    {
        CloseHandle(m_hMemoryMappedHandle);
        m_hMemoryMappedHandle = NULL;
    }

    CloseHandle(m_hAsyncHandle);
    m_hAsyncHandle = NULL;
}

bool FileReader::aquireSyncHandle(HANDLE& outHandle)
{
    {
        auto lock = CreateLock(m_syncHandlesLock);
        if (m_syncHandles.popBackIfExists(outHandle))
            return true;
    }

	HANDLE handle = CreateFileW(m_syncFilePath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    VALIDATION_RETURN_EX_V(handle != INVALID_HANDLE_VALUE, TempString("CreateFileW '{}' failed", m_syncFilePath.c_str()), false);

    return true;
}

void FileReader::releaseSyncHandle(HANDLE handle)
{
    DEBUG_CHECK_RETURN(handle != INVALID_HANDLE_VALUE);

	{
		auto lock = CreateLock(m_syncHandlesLock);
        if (!m_syncHandles.full())
        {
            m_syncHandles.pushBack(handle);
            return;
        }
	}

    CloseHandle(handle);
}


void FileReader::readAsync(FileAbsoluteRange range, void* ptr, TAsyncReadCallback callback)
{
    DEBUG_CHECK_RETURN_EX(!flags().test(FileFlagBit::MemoryMapped), "Cannot use ASYNC IO on memory mapped files");
    m_asyncDispatcher->scheduleAsync(m_hAsyncHandle, range, ptr, std::move(callback));
}

Buffer FileReader::loadToBuffer(IPoolUnmanaged& pool, FileAbsoluteRange range)
{
    DEBUG_CHECK_RETURN_EX_V(fullRange().contains(range), "Invalid file range", nullptr);

    // use memory mapped when possible
    if (m_hMemoryMappedHandle)
        if (auto view = createMapping(range))
            return view->createBuffer();

	// load file data
    auto view = createView(range);
    DEBUG_CHECK_RETURN_EX_V(view, "Failed to create file view", nullptr);

    // allocate buffer
    auto buffer = Buffer::CreateEmpty(pool, range.size(), 16);
    DEBUG_CHECK_RETURN_EX_V(buffer, "Failed to allocate data buffer, OOM?", nullptr);

    // load data
    view->seek(range.absoluteStart());
    const auto numRead = view->readSync(buffer.data(), range.size());
    DEBUG_CHECK_RETURN_EX_V(numRead == range.size(), "Unable to read all data from the file", nullptr);

    // return loaded buffer
    return buffer;
}

FileViewPtr FileReader::createView(FileAbsoluteRange range)
{
    DEBUG_CHECK_RETURN_V(fullRange().contains(range), nullptr);

	// fetch a handle for sync reading
	HANDLE hHandle = NULL;
	if (!aquireSyncHandle(hHandle))
		return nullptr;

    // create wrapper 
    FileFlags viewFlags = flags();
    return RefNew<FileUnbufferedDiskView>(viewFlags, info(), range, hHandle, this);
}

FileMappingPtr FileReader::createMapping(FileAbsoluteRange range)
{
    DEBUG_CHECK_RETURN_EX_V(flags().test(FileFlagBit::MemoryMapped), "File was not memory mapped", nullptr);
    DEBUG_CHECK_RETURN_EX_V(fullRange().contains(range), "File range is invalid", nullptr);
    DEBUG_CHECK_RETURN_EX_V(range.size(), "Cannot map empty file view", nullptr);
    DEBUG_CHECK_RETURN_EX_V(m_hMemoryMappedHandle, "No memory mapped handle", nullptr);

    ScopeTimer timer;

    const auto offsetLow = range.absoluteStart() & 0xFFFFFFFF;
    const auto offsetHigh = (range.absoluteStart() >> 32);
    void* ptr = MapViewOfFile(m_hMemoryMappedHandle, FILE_MAP_READ, offsetHigh, offsetLow, range.size());
    DEBUG_CHECK_RETURN_EX_V(ptr, TempString("Failed to create memory mapped view of file '{}' at {}", m_info, range), nullptr);

    TRACE_INFO("[FILE] Memory mapped '{}' in {} ({} in view)", m_info, timer, MemSize(range.size()));

    return RefNew<FileMemoryMappedView>(m_info, m_hMemoryMappedHandle, ptr, range.size());
}

//--



//--

END_INFERNO_NAMESPACE_EX(windows)

#endif
