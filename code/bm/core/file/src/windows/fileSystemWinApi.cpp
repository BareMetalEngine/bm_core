/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "private.h"

#include "fileSystemWinApi.h"
#include "fileReaderWinApi.h"
#include "fileWriterWinApi.h"
#include "directoryIteratorWinApi.h"
#include "directoryWatcherWinApi.h"
#include "asyncDispatcherWinApi.h"
#include "bm/core/containers/include/utf8StringFunctions.h"

#ifdef PLATFORM_WINAPI

BEGIN_INFERNO_NAMESPACE_EX(windows)

//--

#define GLOBAL_PATH(x) m_globalPaths[(int)FileSystemGlobalPath::##x]

//--

FileSystem::FileSystem()
{
    m_asyncDispatcher = new AsyncReadDispatcher();
    cacheSystemPaths();
}

FileSystem::~FileSystem()
{
    delete m_asyncDispatcher;
    m_asyncDispatcher = nullptr;
}

FileReaderPtr FileSystem::openForReading(StringView absoluteFilePath, FileReadMode mode, TimeStamp* outTimestamp) const
{
    ScopeTimer timer;
    TempPathStringBufferUTF16 str(absoluteFilePath);

    // open flags
    DWORD openFlags = FILE_ATTRIBUTE_NORMAL;
    if (mode != FileReadMode::MemoryMapped)
        openFlags |= FILE_FLAG_OVERLAPPED; // do not use async on memory mapped files

    // get the async handle to the file
    HANDLE hFile = CreateFileW(str, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, openFlags, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        TRACE_WARNING("[FILE] Failed to create reading handle for '{}', error 0x{}", absoluteFilePath, Hex(GetLastError()));
        return nullptr;
    }

    // fetch current timestamp
    if (outTimestamp)
    {
        FILETIME fileTime;
        ::GetFileTime(hFile, NULL, NULL, &fileTime);
        *outTimestamp = TimeStamp(*(const uint64_t*)&fileTime);
    }

    // get current file size
	LARGE_INTEGER size;
	if (!::GetFileSizeEx(hFile, &size))
	{
		TRACE_WARNING("[FILE] Failed to get file size for '{}', error: 0x{}", str, Hex(GetLastError()));
		CloseHandle(hFile);
		return nullptr;
	}

    // if we want to use memory mapping create the view of the file
    HANDLE hFileView = NULL;
    if (mode == FileReadMode::MemoryMapped)
    {
        hFileView = CreateFileMappingW(hFile, NULL, PAGE_READONLY, size.HighPart, size.LowPart, NULL);
        if (hFileView == NULL)
        {
            TRACE_ERROR("[FILE] Failed to create file mapping of '{}'", absoluteFilePath);
            CloseHandle(hFile);
            return nullptr;
        }
    }

    // setup flags
    FileFlags flags;
    flags |= FileFlagBit::FileBacked;
    if (mode == FileReadMode::DirectBuffered)
    {
        flags |= FileFlagBit::Buffered;
    }
    else if (mode == FileReadMode::MemoryMapped)
    {
        flags |= FileFlagBit::MemoryMapped;
        flags |= FileFlagBit::MemoryBacked;
    }

    // Return file reader
    // FileReader(FileFlags flags, StringBuf info, const wchar_t* openFilePath, HANDLE hAsyncFile, uint64_t size, AsyncReadDispatcher* dispatcher);
    TRACE_INFO("[FILE] Opened '{}' in {}", absoluteFilePath, timer);
    return RefNew<FileReader>(flags, StringBuf(absoluteFilePath), str, hFile, size.QuadPart, m_asyncDispatcher, hFileView);
}


FileWriterPtr FileSystem::openForWriting(StringView absoluteFilePath, FileWriteMode mode)
{
    ScopeTimer timer;
    TempPathStringBufferUTF16 str(absoluteFilePath);

    // Create path
    if (!createPath(absoluteFilePath))
    {
        TRACE_ERROR("[FILE] Failed to crate directory structure to save file '{}'", absoluteFilePath);
        return nullptr;
    }

    // Remove the read only flag
    if (isFileReadOnly(absoluteFilePath))
    {
        TRACE_ERROR("[FILE] File '{}' has read only flag set and can't be overriden", absoluteFilePath);
        return nullptr;
    }

	// setup flags
	uint32_t winFlags = 0; // no sharing while writing
	uint32_t createFlags = (mode == FileWriteMode::ReadWrite) ? OPEN_ALWAYS : CREATE_ALWAYS;

	// make the file handle
	HANDLE handle = CreateFileW(str, GENERIC_WRITE, winFlags, NULL, createFlags, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS, NULL);
	if (handle == INVALID_HANDLE_VALUE)
	{
		TRACE_ERROR("[FILE] Failed to open '{}' for writing, error: 0x{}", str, Hex(GetLastError()));
		return nullptr;
	}

    // flags
    FileFlags flags;
    flags |= FileFlagBit::FileBacked;

    // return wrapped file handle
    //FileWriter(FileFlags flags, StringBuf info, HANDLE hSyncHandle);
    TRACE_INFO("[FILE] Opened '{}' in {}", absoluteFilePath, timer);
    return RefNew<FileWriter>(flags, StringBuf(absoluteFilePath), handle);
}

//--

bool FileSystem::loadFileToBuffer_MemoryMapped(StringView absoluteFilePath, Buffer& outBuffer, TimeStamp* outTimestamp) const
{
	TempPathStringBufferUTF16 str(absoluteFilePath);

	// open file for reading
	HANDLE hHandle = CreateFileW(str, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hHandle == INVALID_HANDLE_VALUE)
		return nullptr; // missing file is not exception

	// get current file size
	LARGE_INTEGER size;
	if (!GetFileSizeEx(hHandle, &size))
	{
		TRACE_WARNING("[FILE] Failed to get file size for '{}', error: 0x{}", str, Hex(GetLastError()));
		CloseHandle(hHandle);
		return 0;
	}

	// extract the timestamp
	if (outTimestamp)
	{
		FILETIME fileTime;
		::GetFileTime(hHandle, NULL, NULL, &fileTime);
		*outTimestamp = TimeStamp(*(const uint64_t*)&fileTime);
	}

	// use memory mapping if possible
	HANDLE hMemoryMappedHandle = CreateFileMappingW(hHandle, NULL, PAGE_READONLY, size.HighPart, size.LowPart, NULL);
    if (!hMemoryMappedHandle)
    {
        CloseHandle(hHandle);
        TRACE_WARNING("[FILE] Failed to create file mapping for '{}': 0x{}", absoluteFilePath, Hex(GetLastError()));
        return false;
    }

    // map the whole file
    void* mappingPtr = MapViewOfFile(hMemoryMappedHandle, FILE_MAP_READ, 0, 0, size.QuadPart);
    if (!mappingPtr)
    {
        CloseHandle(hMemoryMappedHandle);
        CloseHandle(hHandle);
        return false;
    }

    // get the view
    auto view = BufferView(mappingPtr, size.QuadPart);

    // return buffer with free callback that will close all opened resources
	outBuffer = Buffer::CreateExternal(view, [hMemoryMappedHandle, mappingPtr, hHandle](void* ptr)
		{
			UnmapViewOfFile(mappingPtr);
			CloseHandle(hMemoryMappedHandle);
			CloseHandle(hHandle);
		});			

    return true;
}

bool FileSystem::loadFileToBuffer_ReadWhole(StringView absoluteFilePath, IPoolUnmanaged& pool, Buffer& outBuffer, TimeStamp* outTimestamp) const
{
    TempPathStringBufferUTF16 str(absoluteFilePath);

    // open file for reading
    HANDLE hHandle = CreateFileW(str, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hHandle == INVALID_HANDLE_VALUE)
        return nullptr; // missing file is not exception

    // get current file size
    LARGE_INTEGER size;
    if (!GetFileSizeEx(hHandle, &size))
    {
		TRACE_WARNING("[FILE] Failed to get file size for '{}', error: 0x{}", str, Hex(GetLastError()));
		CloseHandle(hHandle);
		return 0;
    }

    // allocate buffer
    auto ret = Buffer::CreateEmpty(pool, size.QuadPart, 16, BufferInitState::NoClear);
    if (!ret)
    {
        TRACE_WARNING("[FILE] Failed to allocate memory for loading file '{}' (size {})", absoluteFilePath, MemSize(size.QuadPart));
        CloseHandle(hHandle);
        return nullptr;
    }

    // load content, we need to implement some windows since buffers can be >2GB
    uint64_t pos = 0;
    while (pos < size.QuadPart)
    {
        static const uint64_t WINDOW_SIZE = 1U << 30;

        // how much we can load
        const auto readSize = std::min<uint64_t>(size.QuadPart - pos, WINDOW_SIZE);

        // load content
		DWORD numRead = 0;
        ReadFile(hHandle, ret.data() + pos, readSize, &numRead, NULL);
		if (numRead != readSize)
        {
			TRACE_WARNING("[FILE] Failed to read content of file '{}' at {} (expected {}, got {}), error: 0x{}", 
                absoluteFilePath, pos, readSize, numRead, Hex(GetLastError()));
			CloseHandle(hHandle);
			return nullptr;
		}

        // advance
        pos += numRead;
    }

    // extract the timestamp
    if (outTimestamp)
    {
        FILETIME fileTime;
        ::GetFileTime(hHandle, NULL, NULL, &fileTime);
        *outTimestamp = TimeStamp(*(const uint64_t*)&fileTime);
    }

    // done reading
    CloseHandle(hHandle);
    outBuffer = ret;
    return true;
}

bool FileSystem::loadFileToBuffer(StringView absoluteFilePath, IPoolUnmanaged& pool, Buffer& outBuffer, TimeStamp* outTimestamp /*= nullptr*/, FileReadMode mode /*= FileReadMode::MemoryMapped*/) const
{
    // try memory mapped first
    if (mode == FileReadMode::MemoryMapped)
        if (loadFileToBuffer_MemoryMapped(absoluteFilePath, outBuffer, outTimestamp))
            return true;

    // load directly (slow and risky..)
    return loadFileToBuffer_ReadWhole(absoluteFilePath, pool, outBuffer, outTimestamp);
}

bool FileSystem::saveFileFromBuffer(StringView absoluteFilePath, BufferView data, const TimeStamp* timestampToAssign)
{
    TempPathStringBufferUTF16 str(absoluteFilePath);
	TempPathStringBufferUTF16 path(absoluteFilePath.pathParent());

    // Remove the read only flag
    if (isFileReadOnly(absoluteFilePath))
    {
        TRACE_ERROR("[FILE] File '{}' has read only flag set and can't be overriden", absoluteFilePath);
        return nullptr;
    }

    // does the target file exist ?
    if (fileInfo(absoluteFilePath))
    {
        // Generate a temp file name
        wchar_t tmpStr[MAX_PATH];
        if (!GetTempFileNameW(path, L"_inferno", 0, tmpStr))
        {
            TRACE_ERROR("[FILE] Unable to generate temporary file name for saving '{}'", absoluteFilePath);
            return nullptr;
        }

        // make the file handle
        HANDLE hHandle = CreateFileW(tmpStr, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN | FILE_ATTRIBUTE_TEMPORARY, NULL);
        if (hHandle == INVALID_HANDLE_VALUE)
        {
            TRACE_ERROR("[FILE] Failed to open '{}' for writing, error: 0x{}", str, Hex(GetLastError()));
            return nullptr;
        }

        // load content, we need to implement some windows since buffers can be >2GB
        uint64_t pos = 0;
        while (pos < data.size())
        {
            static const uint64_t WINDOW_SIZE = 1U << 30;

            // how much we can write
            const auto writeSize = std::min<uint64_t>(data.size() - pos, WINDOW_SIZE);

            // load content
            DWORD numWritten = 0;
            WriteFile(hHandle, data.data() + pos, writeSize, &numWritten, NULL);
            if (numWritten != writeSize)
            {
                TRACE_WARNING("[FILE] Failed to write content of file '{}' at {} (expected {}, got {}), error: 0x{}",
                    tmpStr, pos, writeSize, numWritten, Hex(GetLastError()));
                CloseHandle(hHandle);
                DeleteFileW(tmpStr);
                return nullptr;
            }

            // advance
            pos += numWritten;
        }

        // close temp file
        CloseHandle(hHandle);

        // move to target place using a backup
        TempPathStringBufferUTF16 backupDestStr(absoluteFilePath);
        backupDestStr.append(L".bak");

        DWORD flags = REPLACEFILE_IGNORE_MERGE_ERRORS | REPLACEFILE_IGNORE_ACL_ERRORS;
        if (!ReplaceFileW(str, tmpStr, backupDestStr, flags, NULL, NULL))
        {
            TRACE_ERROR("[FILE] Failed to save file '{}' into '{}': 0x{}", tmpStr, str.buffer(), Hex(GetLastError()));
            return false;
        }

        // delete the backup
        if (!DeleteFileW(backupDestStr.buffer()))
        {
            TRACE_WARNING("[FILE] Failed to delete temporary file '{}'", backupDestStr.buffer());
        }
    }
    // target file does not exist, write directly
    else
    {
		// make the file handle
		HANDLE hHandle = CreateFileW(str, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN | FILE_ATTRIBUTE_TEMPORARY, NULL);
		if (hHandle == INVALID_HANDLE_VALUE)
		{
			TRACE_ERROR("[FILE] Failed to open '{}' for writing, error: 0x{}", str, Hex(GetLastError()));
			return nullptr;
		}

		// load content, we need to implement some windows since buffers can be >2GB
		uint64_t pos = 0;
		while (pos < data.size())
		{
			static const uint64_t WINDOW_SIZE = 1U << 30;

			// how much we can write
			const auto writeSize = std::min<uint64_t>(data.size() - pos, WINDOW_SIZE);

			// load content
			DWORD numWritten = 0;
			WriteFile(hHandle, data.data() + pos, writeSize, &numWritten, NULL);
			if (numWritten != writeSize)
			{
				TRACE_WARNING("[FILE] Failed to write content of file '{}' at {} (expected {}, got {}), error: 0x{}",
					str, pos, writeSize, numWritten, Hex(GetLastError()));
				CloseHandle(hHandle);
				DeleteFileW(str);
				return nullptr;
			}

			// advance
			pos += numWritten;
		}

		// close temp file
		CloseHandle(hHandle);
    }

    // file saved
    return true;
}

bool FileSystem::createPath(StringView absoluteFilePath)
{
    TempPathStringBufferUTF16 str(absoluteFilePath);

    // Create path
    const wchar_t* path = str;
    for (const wchar_t* pos = path; *pos; pos++)
    {
        if (*pos == '\\' || *pos == '/')
        {
            auto old = *pos;
            *(wchar_t*)pos = 0;
            bool ok = CreateDirectoryW(path, NULL);
            *(wchar_t*)pos = old;

            if (!ok && !wcsrchr(path, ':'))
                return false;
        }
    }

    return true;
}

bool FileSystem::moveFile(StringView srcAbsolutePath, StringView destAbsolutePath)
{
	TempPathStringBufferUTF16 srcStr(srcAbsolutePath);
	TempPathStringBufferUTF16 destStr(destAbsolutePath);
    TempPathStringBufferUTF16 backupDestStr(destAbsolutePath);
    backupDestStr.append(L".bak");

    DWORD flags = REPLACEFILE_IGNORE_MERGE_ERRORS | REPLACEFILE_IGNORE_ACL_ERRORS;
    if (!ReplaceFileW(destStr, srcStr, backupDestStr, flags, NULL, NULL))
    {
        TRACE_WARNING("[FILE] Failed to move file '{}' into '{}': 0x{}", srcAbsolutePath, destAbsolutePath, Hex(GetLastError()));
        return false;
    }

    return true;
}

bool FileSystem::copyFile(StringView srcAbsolutePath, StringView destAbsolutePath)
{
    // Make sure target path exists
    if (!createPath(destAbsolutePath))
    {
        TRACE_WARNING("[FILE] Failed to create path for destination file \"{}\"", destAbsolutePath);
        return false;
    }

    // We fail if target file exists
    if (fileInfo(destAbsolutePath))
    {
		TRACE_WARNING("[FILE] Trying to copy over existing file at \"{}\"", destAbsolutePath);
		return false;
    }

    // Copy File
    TempPathStringBufferUTF16 srcStr(srcAbsolutePath);
    TempPathStringBufferUTF16 destStr(destAbsolutePath);
    if (0 == ::CopyFileW(srcStr, destStr, FALSE))
    {
        TRACE_WARNING("[FILE] Failed to copy file '{}' into '{}': 0x{}", srcAbsolutePath, destAbsolutePath, Hex(GetLastError()));
        return false;
    }

    // file copied
    return true;
}

bool FileSystem::deleteFile(StringView absoluteFilePath)
{
    if (!isFileReadOnly(absoluteFilePath))
    {
        TRACE_WARNING("[FILE] Failed to delete file '{}' because it's read only", absoluteFilePath);
        return false;
    }

    TempPathStringBufferUTF16 cstr(absoluteFilePath);
    if (!::DeleteFileW(cstr))
    {
        TRACE_WARNING("[FILE] Failed to delete file '{}', error: 0x{}", cstr, Hex(GetLastError()));
        return false;
    }

    return true;
}

bool FileSystem::deleteDir(StringView absoluteDirPath)
{
    TempPathStringBufferUTF16 cstr(absoluteDirPath);
    if (!::RemoveDirectoryW(cstr))
    {
        TRACE_WARNING("[FILE] Failed to delete directory '{}', error: 0x{}", cstr, Hex(GetLastError()));
        return false;
    }

    return true;
}

bool FileSystem::touchFile(StringView absoluteFilePath)
{
    TempPathStringBufferUTF16 cstr(absoluteFilePath);

    return false;
}

bool FileSystem::fileInfo(StringView absoluteFilePath, TimeStamp* outTimeStamp /*= nullptr*/, uint64_t* outFileSize /*= nullptr*/) const
{
    TempPathStringBufferUTF16 cstr(absoluteFilePath);
    DWORD dwAttrib = GetFileAttributes(cstr);

    const auto exists = (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));

    if (exists && (outFileSize || outTimeStamp))
    {
        HANDLE hHandle = CreateFileW(cstr, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hHandle != INVALID_HANDLE_VALUE)
        {
            if (outTimeStamp)
            {
                FILETIME fileTime;
                ::GetFileTime(hHandle, NULL, NULL, &fileTime);
                *outTimeStamp = TimeStamp(*(const uint64_t*)&fileTime);
            }

            if (outFileSize)
            {
                ::GetFileSizeEx(hHandle, (PLARGE_INTEGER)outFileSize);
            }

            ::CloseHandle(hHandle);
        }
    }

    return exists;
}

bool FileSystem::isFileReadOnly(StringView absoluteFilePath) const
{
    TempPathStringBufferUTF16 cstr(absoluteFilePath);

    auto attr = ::GetFileAttributesW(cstr);
    if (attr == INVALID_FILE_ATTRIBUTES)
        return false;

    bool readOnly = (attr & FILE_ATTRIBUTE_READONLY) != 0;

    return readOnly;
}

bool FileSystem::readOnlyFlag(StringView absoluteFilePath, bool flag)
{
    TempPathStringBufferUTF16 cstr(absoluteFilePath);

    auto attr = ::GetFileAttributesW(cstr);
    auto srcAttr = attr;

    // Change read only flag
    if (flag)
        attr |= FILE_ATTRIBUTE_READONLY;
    else
        attr &= ~FILE_ATTRIBUTE_READONLY;

    // same ?
    if (attr == srcAttr)
        return true;

    if (SetFileAttributes(cstr, attr) != 0)
    {
        TRACE_WARNING("[FILE] Failed to change read-only attribute of file \"{}\": 0x{}", absoluteFilePath, Hex(GetLastError()));
        return false;
    }

    return true;
}

//--

static bool FindFilesInternal(TempPathStringBufferUTF16& dirPath, TempPathStringBufferAnsi& dirPathUTF, StringView searchPattern, const std::function<bool(StringView fullPath, StringView fileName)>& enumFunc, bool recurse)
{
    auto* org = dirPath.capture();
    auto* org2 = dirPathUTF.capture();

    if (!dirPath.append(searchPattern))
        return false;

    for (DirectoryIterator it(dirPath, true, false); it; ++it)
    {
        dirPath.restore(org);
        dirPathUTF.restore(org2);

        const auto* fileName = it.fileName();

        if (dirPath.append(fileName) && dirPathUTF.append(fileName))
            if (enumFunc((const char*)dirPathUTF, fileName))
                return true;
    }

    dirPath.restore(org);
    dirPathUTF.restore(org2);

    if (recurse && dirPath.append(L"*."))
    {
        for (DirectoryIterator it(dirPath, false, true); it; ++it)
        {
            dirPath.restore(org);
            dirPathUTF.restore(org2);

            if (dirPath.append(it.fileNameRaw()) && dirPath.append("\\"))
            {
                if (dirPathUTF.append(it.fileNameRaw()) && dirPathUTF.append("\\"))
                {
                    if (FindFilesInternal(dirPath, dirPathUTF, searchPattern, enumFunc, recurse))
                        return true;
                }
            }
        }
    }

    return false;
}

bool FileSystem::enumFiles(StringView absoluteFilePath, StringView searchPattern, const std::function<bool(StringView fullPath, StringView fileName)>& enumFunc, bool recurse) const
{
    if (absoluteFilePath.empty())
        return false;

    TempPathStringBufferUTF16 dirPath(absoluteFilePath);
    TempPathStringBufferAnsi dirPath2(absoluteFilePath);
    return FindFilesInternal(dirPath, dirPath2, searchPattern, enumFunc, recurse);
}

bool FileSystem::enumSubDirs(StringView absoluteFilePath, const std::function<bool(StringView name)>& enumFunc) const
{
    TempPathStringBufferUTF16 dirPath(absoluteFilePath);
    if (dirPath.append(L"*."))
    {
		WIN32_FIND_DATA FindFileData;
        HANDLE hFind = FindFirstFileExW(dirPath, FindExInfoBasic, &FindFileData, FindExSearchLimitToDirectories, NULL, FIND_FIRST_EX_LARGE_FETCH);
		if (hFind == INVALID_HANDLE_VALUE)
        {
            TRACE_WARNING("[FILE] Failed to create directory search at '{}'", absoluteFilePath);
			return false;
		}

        do
        {
            if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                if (FindFileData.cFileName[0] == '.')
                    continue; // skip hidden folders

                char buffer[MAX_PATH];
				utf8::FromUniChar(buffer, sizeof(buffer) - 1, FindFileData.cFileName, wcslen(FindFileData.cFileName));
                if (enumFunc(buffer))
                    return true;
            }
        } while (FindNextFileW(hFind, &FindFileData));

        FindClose(hFind);
    }

    return false;
}

bool FileSystem::enumLocalFiles(StringView absoluteFilePath, StringView searchPattern, const std::function<bool(StringView name)>& enumFunc) const
{
    TempPathStringBufferUTF16 dirPath(absoluteFilePath);
    if (dirPath.append(searchPattern))
    {
        if (searchPattern == "*.*")
        {
			WIN32_FIND_DATA FindFileData;
			HANDLE hFind = FindFirstFileExW(dirPath, FindExInfoBasic, &FindFileData, FindExSearchNameMatch, NULL, FIND_FIRST_EX_LARGE_FETCH);
			if (hFind == INVALID_HANDLE_VALUE)
			{
				TRACE_WARNING("[FILE] Failed to create directory search at '{}'", absoluteFilePath);
				return false;
			}

			do
			{
				if (!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
				{
					if (FindFileData.cFileName[0] == '.')
						continue; // skip hidden folders

					char buffer[MAX_PATH];
					utf8::FromUniChar(buffer, sizeof(buffer) - 1, FindFileData.cFileName, wcslen(FindFileData.cFileName));
					if (enumFunc(buffer))
						return true;
				}
			} while (FindNextFileW(hFind, &FindFileData));

			FindClose(hFind);
        }
        else
        {
            for (DirectoryIterator it(dirPath, true, false); it; ++it)
                if (enumFunc(it.fileName()))
                    return true;
        }
    }

    return false;
}

bool FileSystem::enumFileSystemRoots(const std::function<bool(StringView name)>& enumFunc, bool allowNetworkDrives) const
{
    auto driveMask = ::GetLogicalDrives();
    for (uint32_t i = 0; i < 26; ++i, driveMask >>= 1)
    {
        if (driveMask & 1)
        {
            char txt[4];
            txt[0] = 'A' + i;
            txt[1] = ':';
            txt[2] = '\\';
            txt[3] = 0;

            if (!allowNetworkDrives)
            {
                const auto driveType = ::GetDriveTypeA(txt);
                if (driveType == DRIVE_REMOTE)
                    continue;
            }

            if (enumFunc(txt))
                return true;
        }
    }

    return false;
}

DirectoryWatcherPtr FileSystem::createDirectoryWatcher(StringView path)
{
    TempPathStringBufferUTF16 str(path);
    return RefNew<DirectoryWatcher>(Array<wchar_t>(str.view()));
}

//---

void FileSystem::cacheSystemPaths()
{
    wchar_t path[MAX_PATH + 1];

    {
        StringBuilder f;
        GetModuleFileNameW(NULL, path, MAX_PATH);
        f.append(path);

        GLOBAL_PATH(ExecutableFile) = StringBuf(f);
        TRACE_INFO("[FILE] Executable path: '{}'", GLOBAL_PATH(ExecutableFile));
    
        if (auto* ch = wcsrchr(path, '\\'))
            ch[1] = 0;

        f.clear();
        f.append(path);

        GLOBAL_PATH(ExecutableDir) = StringBuf(f);
        TRACE_INFO("[FILE] Executable directory: '{}'", GLOBAL_PATH(ExecutableDir));

        for (;;)
        {
            auto* ch = wcsrchr(path, '\\');
            if (!ch)
                break;

            ch[1] = 0;
            wcscat(ch, L"project.xml");

            DWORD dwAttrib = GetFileAttributes(path);
            const auto exists = (dwAttrib != INVALID_FILE_ATTRIBUTES) && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY);
            if (!exists)
            {
                ch[0] = 0;
            }
            else
            {
                ch[1] = 0;
                f.clear();
                f.append(path);
                break;
            }
        }

        GLOBAL_PATH(EngineDir) = StringBuf(f);
        TRACE_INFO("[FILE] Engine root dir: '{}'", GLOBAL_PATH(EngineDir));
    }

    {
        wchar_t path[MAX_PATH + 1];
        GetModuleFileNameW(NULL, path, MAX_PATH);

        if (auto* ch = wcsrchr(path, '\\'))
            ch[1] = 0;

        StringBuilder f;
        wcscat(path, L".temp\\local\\");
        f << path;

        GLOBAL_PATH(LocalTempDir) = StringBuf(f);
        TRACE_INFO("[FILE] Local temp dir: '{}'", GLOBAL_PATH(LocalTempDir));
    }

    {
        wchar_t path[MAX_PATH + 1];
        auto length = GetTempPathW(MAX_PATH, path);

        if (length > 0)
        {
            StringBuilder f;
            wcscat(path, L"Inferno\\");
            f << path;

            GLOBAL_PATH(SystemTempDir) = StringBuf(f);
        }
        else
        {
            GLOBAL_PATH(SystemTempDir) = GLOBAL_PATH(LocalTempDir);
        }

        TRACE_INFO("[FILE] System temp dir: '{}'", GLOBAL_PATH(SystemTempDir));
    }

    {
        wchar_t path[MAX_PATH + 1];

        HRESULT result = SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, path);
        ASSERT(result == S_OK);
        wcscat_s(path, MAX_PATH, L"\\Inferno\\config\\");

        StringBuilder f;
        f << path;

        auto str = StringBuf(f).replaceChar(WRONG_SYSTEM_PATH_SEPARATOR, SYSTEM_PATH_SEPARATOR);
        TRACE_INFO("[FILE] User config dir: '{}'", str);

        GLOBAL_PATH(UserConfigDir) = str;
    }

    {
        wchar_t path[MAX_PATH + 1];

        HRESULT result = SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, path);
        ASSERT(result == S_OK);
        wcscat_s(path, MAX_PATH, L"\\");

        StringBuilder f;
        f << path;

        auto str = StringBuf(f).replaceChar(WRONG_SYSTEM_PATH_SEPARATOR, SYSTEM_PATH_SEPARATOR);
        TRACE_INFO("[FILE] User documents dir: '{}'", str);

        GLOBAL_PATH(UserDocumentsDir) = str;
    }
}

//--

END_INFERNO_NAMESPACE_EX(windows)

#endif
