/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "fileSystemPOSIX.h"

#include "bm/core/containers/include/utf8StringFunctions.h"
#include "bm/core/containers/include/path.h"
#include "bm/core/containers/include/inplaceArray.h"
#include "bm/core/system/include/thread.h"

#ifdef PLATFORM_POSIX

#include <sys/types.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>

BEGIN_INFERNO_NAMESPACE_EX(posix)

//--

namespace prv
{
    class TempPathStringBuffer
    {
    public:
        TempPathStringBuffer()
        {
            m_writePos = m_buffer;
            m_writeEnd = m_buffer + MAX_SIZE - 1;
            *m_writePos = 0;
        }

        TempPathStringBuffer(StringView view)
        {
            m_writePos = m_buffer;
            m_writeEnd = m_buffer + MAX_SIZE - 1;
            *m_writePos = 0;

            append(view);
        }

        void clear()
        {
            m_writePos = m_buffer;
            *m_writePos = 0;
        }

        operator const char* () const
        {
            DEBUG_CHECK(m_writePos <= m_writeEnd);
            DEBUG_CHECK(*m_writePos == 0);
            return m_buffer;
        }

        char* pos() const
        {
            DEBUG_CHECK(m_writePos <= m_writeEnd);
            DEBUG_CHECK(*m_writePos == 0);
            return m_writePos;
        }

        Array<char> buffer() const
        {
            DEBUG_CHECK(m_writePos <= m_writeEnd);
            DEBUG_CHECK(*m_writePos == 0);
            auto count = (m_writePos - m_buffer) + 1;
            return Array<char>(m_buffer, count);
        }

        bool append(StringView txt)
        {
            DEBUG_CHECK(m_writePos <= m_writeEnd);
            DEBUG_CHECK(*m_writePos == 0);

            auto start = m_writePos;

            auto ptr = txt.data();
            auto endPtr = txt.data() + txt.length();
            while (ptr < endPtr)
            {
                auto ch = *ptr++;

                if (ch == '/')
                    ch = '\\';

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

        void pop(char* oldWritePos)
        {
            DEBUG_CHECK(oldWritePos >= m_buffer && oldWritePos <= m_writeEnd);
            m_writePos = oldWritePos;
            *m_writePos = 0;
        }

        void print(IFormatStream& f) const
        {
            f.append((const char*)m_buffer);
        }

    private:
        static const auto MAX_SIZE = 1024;

        char m_buffer[MAX_SIZE];
        char* m_writePos;
        char* m_writeEnd;
    };
} // prv

//--

FileIterator::FileIterator(const char* path, const char* pattern, bool allowFiles, bool allowDirs)
    : m_allowDirs(allowDirs)
    , m_allowFiles(allowFiles)
    , m_searchPath(path)
    , m_searchPattern(pattern)
    , m_dir(nullptr)
    , m_entry(nullptr)
{
    // Format the search path
    m_dir = opendir(path);
    m_entry = m_dir ? readdir((DIR*)m_dir) : nullptr;

    if (StringView(pattern) != "*.*" && StringView(pattern) != "*.")
        m_matchPattern = true;

    // Get first valid entry
    while (!validateEntry())
        if (!nextEntry())
            break;
}

FileIterator::~FileIterator()
{
    // Close search handle
    if (m_dir != 0)
    {
        closedir((DIR*)m_dir);
        m_dir = 0;
    }
}

const char* FileIterator::fileName() const
{
    if (m_entry == 0)
        return nullptr;

    return ((struct dirent*)m_entry)->d_name;
}

bool FileIterator::validateEntry() const
{
    if (m_entry == 0)
        return false;

    auto fileEntry = ((struct dirent*)m_entry);
    auto fileName = fileEntry->d_name;
    if (0 == strcmp(fileName, "."))
        return false;

    if (0 == strcmp(fileName, ".."))
        return false;

    // Skip filtered
    bool isDirectory = (fileEntry->d_type == DT_DIR);
    if ((isDirectory && !m_allowDirs) || (!isDirectory && !m_allowFiles))
        return false;

    // check pattern
    if (m_matchPattern)
        if (!StringView(fileName).matchString(m_searchPattern))
            return false;

    // entry can be used
    return true;
}

bool FileIterator::nextEntry()
{
    if (m_dir == 0)
        return false;

    m_entry = readdir((DIR*)m_dir);
    if (!m_entry)
    {
        closedir((DIR*)m_dir);
        m_dir = nullptr;
    }

    return true;
}

void FileIterator::operator++(int)
{
    while (nextEntry())
        if (validateEntry())
            break;
}

void FileIterator::operator++()
{
    while (nextEntry())
        if (validateEntry())
            break;
}

FileIterator::operator bool() const
{
    return m_entry != 0;
}

//--

ReadFileHandle::ReadFileHandle(int hFile, const StringBuf& origin)
    : m_fileHandle(hFile)
    , m_origin(origin)
{
}

ReadFileHandle::~ReadFileHandle()
{
    close(m_fileHandle);
    m_fileHandle = 0;
}

uint64_t ReadFileHandle::size() const
{
    auto pos = lseek64(m_fileHandle, 0, SEEK_CUR);
    auto size = lseek64(m_fileHandle, 0, SEEK_END);
    lseek64(m_fileHandle, pos, SEEK_SET);
    return size;
}

uint64_t ReadFileHandle::pos() const
{
    return lseek64(m_fileHandle, 0, SEEK_CUR);
}

bool ReadFileHandle::pos(uint64_t newPosition)
{
    auto newPos = lseek64(m_fileHandle, newPosition, SEEK_SET);
    return newPos == newPosition;
}

uint64_t ReadFileHandle::readSync(void* data, uint64_t size)
{
    auto numRead = read(m_fileHandle, data, size);
    if (numRead != size)
    {
        TRACE_ERROR("Read failed for '{}'", m_origin);
        return 0;
    }

    return numRead;
}

//--

WriteFileHandle::WriteFileHandle(int hFile, const StringBuf& origin)
    : m_fileHandle(hFile)
    , m_origin(origin)    
{
}

WriteFileHandle::~WriteFileHandle()
{
    close(m_fileHandle);
    m_fileHandle = 0;
}

void WriteFileHandle::discardContent()
{
}

uint64_t WriteFileHandle::size() const
{
    auto pos = lseek64(m_fileHandle, 0, SEEK_CUR);
    auto size = lseek64(m_fileHandle, 0, SEEK_END);
    lseek64(m_fileHandle, pos, SEEK_SET);
    return size;
}

uint64_t WriteFileHandle::pos() const
{
    return lseek64(m_fileHandle, 0, SEEK_CUR);
}

bool WriteFileHandle::pos(uint64_t newPosition)
{
    auto newPos = lseek64(m_fileHandle, newPosition, SEEK_SET);
    return newPos == newPosition;
}

uint64_t WriteFileHandle::writeSync(const void* data, uint64_t size)
{
    auto numWritten = write(m_fileHandle, data, size);
    if (numWritten != size)
    {
        TRACE_ERROR("Write failed for '{}', written {} instead of {} at {}, size {}: error: {}", 
            m_origin, numWritten, size, pos(), this->size(), errno);
    }

    return numWritten;
}

//--

WriteTempFileHandle::WriteTempFileHandle(const StringBuf& targetPath, const StringBuf& tempFilePath, const FileWriterPtr& tempFileWriter)
    : m_tempFileWriter(tempFileWriter)
    , m_tempFilePath(tempFilePath)
    , m_targetFilePath(targetPath)
{}

WriteTempFileHandle::~WriteTempFileHandle()
{
    if (m_tempFileWriter)
    {
        // close it
        m_tempFileWriter.reset();

        // delete target file
        unlink(m_targetFilePath.c_str());

        // move temp file to the target place
        if (0 == rename(m_tempFilePath.c_str(), m_targetFilePath.c_str()))
        {
            TRACE_INFO("[FILE] Finished staged writing for target '{}'. Temp file '{}' will be delete.", 
                m_targetFilePath, m_tempFilePath);
        }
        else
        {
            TRACE_WARNING("[FILE] Failed to move starged file to '{}'. New content remains saved at '{}'.",
                m_targetFilePath, m_tempFilePath);
        }
    }
}

uint64_t WriteTempFileHandle::size() const
{
    if (m_tempFileWriter)
        return m_tempFileWriter->size();
    return 0;
}

uint64_t WriteTempFileHandle::pos() const
{
    if (m_tempFileWriter)
        return m_tempFileWriter->pos();
    return 0;
}

bool WriteTempFileHandle::pos(uint64_t newPosition)
{
    if (m_tempFileWriter)
        return m_tempFileWriter->pos(newPosition);
    return false;
}

uint64_t WriteTempFileHandle::writeSync(const void* data, uint64_t size)
{
    if (m_tempFileWriter)
        return m_tempFileWriter->writeSync(data, size);
    return 0;
}

void WriteTempFileHandle::discardContent()
{
    if (m_tempFileWriter)
    {
        TRACE_WARNING("[FILE] Discarded file writing for target '{}'. Temp file '{}' will be delete.");
        m_tempFileWriter.reset();
        unlink(m_tempFilePath.c_str());
    }
}

//--

AsyncFileHandle::AsyncFileHandle(int hFile, const StringBuf& origin, uint64_t size, AsyncReadDispatcher* dispatcher)
    : m_fileHandle(hFile)
    , m_origin(origin)
    , m_dispatcher(dispatcher)
{
}

AsyncFileHandle::~AsyncFileHandle()
{
    close(m_fileHandle);
    m_fileHandle = 0;
}

uint64_t AsyncFileHandle::size() const
{
    return m_size;
}

uint64_t AsyncFileHandle::readAsync(uint64_t offset, uint64_t size, void* readBuffer)
{
    return m_dispatcher->readAsync(m_fileHandle, offset, size, readBuffer);
}

//--

DirectoryWatcher::DirectoryWatcher(StringView rootPath)
{
    // create the notify interface
    m_masterHandle = inotify_init1(IN_NONBLOCK);
    if (m_masterHandle >= 0)
    {
        TRACE_SPAM("Created directory watcher for '{}'", rootPath);
    }
    else
    {
        TRACE_ERROR("Failed to create directory watcher");
    }

    // create the watcher thread
    ThreadSetup setup;
    setup.m_name = "DirectoryWatcher";
    setup.m_priority = ThreadPriority::AboveNormal;
    setup.m_function = [this]() { watch(); };
    m_localThread.init(setup);

    // start monitoring the root path
    monitorPath(rootPath);
}

DirectoryWatcher::~DirectoryWatcher()
{
    // close the master handle
    if (m_masterHandle >= 0)
    {
        // unmonitor all paths
        {
            ScopeLock<SpinLock> lock(m_mapLock);

            for (auto watchId : m_handleToPath.keys())
                inotify_rm_watch(m_masterHandle, watchId);

            m_handleToPath.clear();
            m_pathToHandle.clear();
        }

        // close the master handle
        close(m_masterHandle);
        m_masterHandle = -1;
    }

    // stop thread
    m_localThread.close();
}

void DirectoryWatcher::attachListener(IDirectoryWatcherListener* listener)
{
    ScopeLock<Mutex> lock(m_listenersLock);
    m_listeners.pushBack(listener);
}

void DirectoryWatcher::dettachListener(IDirectoryWatcherListener* listener)
{
    ScopeLock<Mutex> lock(m_listenersLock);
    auto index = m_listeners.find(listener);
    if (index != -1)
        m_listeners[index] = nullptr;
}

void DirectoryWatcher::monitorPath(StringView str)
{
    // we can create the watch only if we are initialized properly
    if (m_masterHandle >= 0)
    {
        StringBuf path(str);

        // create the watcher
        auto watcherId = inotify_add_watch(m_masterHandle, path.c_str(), IN_CREATE | IN_CLOSE_WRITE | IN_DELETE | IN_DELETE_SELF | IN_MOVE_SELF | IN_MOVE | IN_MODIFY | IN_ATTRIB);
        if (watcherId != -1)
        {
            TRACE_SPAM("Added directory watch to '{}', handle: {}", path, watcherId);

            // add to map
            {
                ScopeLock<SpinLock> lock(m_mapLock);
                m_handleToPath.set(watcherId, path);
                m_pathToHandle.set(str.calcCRC64(), watcherId);
            }

            // monitor the existing sub directories as well
            for (FileIterator it(path.c_str(), "*.", false, true); it; ++it)
                monitorPath(TempString("{}{}/", str, it.fileName()));
        }
    }
}

void DirectoryWatcher::unmonitorPath(StringView str)
{
    prv::TempPathStringBuffer path(str);

    // un monitor the existing sub directories as well
    for (FileIterator it(path, "*.", false, true); it; ++it)
        unmonitorPath(TempString("{}{}/", path, it.fileName()));

    // remove current watcher
    ScopeLock<SpinLock> lock(m_mapLock);

    int watcherId = 0;
    auto pathHash = str.calcCRC64();
    if (m_pathToHandle.find(pathHash, watcherId))
    {
        // remove from tables
        TRACE_SPAM("Removed directory watcher at '{}' ({})", path, watcherId);
        m_pathToHandle.remove(pathHash);
        m_handleToPath.remove(watcherId);

        // remove from system
        inotify_rm_watch(m_masterHandle, watcherId);
    }
}

void DirectoryWatcher::watch()
{
    for (;;)
    {
        // nothing to read
        if (m_masterHandle <= 0)
            break;

        // read data from the crap
        auto dataSize = read(m_masterHandle, m_buffer, BUF_LEN);
        if (dataSize < 0)
        {
            if (errno == EAGAIN)
            {
                Thread::Sleep(10);
                continue;
            }

            TRACE_ERROR("Read error in the directory watcher data stream: {}", errno);
            break;
        }

        // prepare tables
        m_tempEvents.reset();
        m_tempAddedDirectories.reset();
        m_tempRemovedDirectories.reset();

        // process data
        auto cur = &m_buffer[0];
        auto end = &m_buffer[dataSize];
        while (cur < end)
        {
            // get event
            auto& evt = *(struct inotify_event*)cur;

            // prevent modifications of the map table
            m_mapLock.acquire();

            // identify the target path entry
            auto pathEntry = m_handleToPath.find(evt.wd);
            if (!pathEntry)
            {
                TRACE_INFO("IO event at unrecognized path, ID {}, '{}'", evt.wd, (const char*)evt.name);
                m_mapLock.release();
                break;
            }

            TRACE_INFO("IO watcher: '{}' '{}' {}", pathEntry->c_str(), (const char*)evt.name, Hex(evt.mask));

            // self deleted
            if (evt.mask & (IN_DELETE_SELF | IN_MOVE_SELF))
            {
                // find path
                TRACE_INFO("Observed directory '{}' self deleted", pathEntry->c_str());
                auto& info = m_tempEvents.emplaceBack();
                info.type = DirectoryWatcherEventType::DirectoryRemoved;
                info.path = *pathEntry;

                // unmap
                m_pathToHandle.remove(pathEntry->view().calcCRC64());
                m_handleToPath.remove(evt.wd);
                m_mapLock.release();
            }
            else
            {
                // format full path
                bool isDir = (0 != (evt.mask & IN_ISDIR));
                auto shortPath = &evt.name[0];
                m_mapLock.release();

                // format path
                auto fullPath = isDir 
                    ? StringBuf(TempString("{}{}/", pathEntry->view(), shortPath))
                    : StringBuf(TempString("{}{}", pathEntry->view(), shortPath));

                // stuff was created
                if (evt.mask & IN_CREATE)
                {
                    // if a directory is added make sure to monitor it as well
                    if (isDir)
                    {
                        m_tempAddedDirectories.pushBack(fullPath);

                        auto& evt = m_tempEvents.emplaceBack();
                        evt.type = isDir ? DirectoryWatcherEventType::DirectoryAdded : DirectoryWatcherEventType::FileAdded;
                        evt.path = fullPath;
                    }
                    else
                    {
                        // remember the file around
                        m_filesCreatedButNotYetClosed.pushBackUnique(fullPath);
                    }
                }

                // writable file was closed
                if (evt.mask & IN_CLOSE_WRITE)
                {
                    // report event only if the fle was previously modified
                    if (m_filesCreatedButNotYetClosed.remove(fullPath))
                    {
                        auto& evt = m_tempEvents.emplaceBack();
                        evt.type = DirectoryWatcherEventType::FileAdded;
                        evt.path = fullPath;
                    }

                    // just modified
                    if (m_filesModifiedButNotYetClosed.remove(fullPath))
                    {
                        auto& evt = m_tempEvents.emplaceBack();
                        evt.type = DirectoryWatcherEventType::FileContentChanged;
                        evt.path = fullPath;
                    }
                }

                // file was moved
                if (evt.mask & IN_MOVED_TO)
                {
                    // if a directory is added make sure to monitor it as well
                    if (isDir)
                        m_tempAddedDirectories.pushBack(fullPath);

                    // create direct event
                    auto& evt = m_tempEvents.emplaceBack();
                    evt.type = isDir ? DirectoryWatcherEventType::DirectoryAdded : DirectoryWatcherEventType::FileAdded;
                    evt.path = fullPath;
                }

                // stuff was removed
                if (evt.mask & (IN_DELETE | IN_MOVED_FROM))
                {
                    // if a directory is added make sure to monitor it as well
                    if (isDir)
                        m_tempRemovedDirectories.pushBack(fullPath);

                    // create event
                    auto& evt = m_tempEvents.emplaceBack();
                    evt.type = isDir ? DirectoryWatcherEventType::DirectoryRemoved : DirectoryWatcherEventType::FileRemoved;
                    evt.path = fullPath;
                }

                // stuff was changed
                if (evt.mask & (IN_MODIFY))
                {
                    if (!isDir)
                        m_filesModifiedButNotYetClosed.pushBackUnique(fullPath);
                }

                // metadata changed
                if (evt.mask & (IN_ATTRIB))
                {
                    auto& evt = m_tempEvents.emplaceBack();
                    evt.type = DirectoryWatcherEventType::FileMetadataChanged;
                    evt.path = fullPath;
                }
            }

            // advance
            cur += sizeof(struct inotify_event) + evt.len;
        }

        // unmonitor directories that got removed
        for (auto& path : m_tempRemovedDirectories)
            unmonitorPath(path);

        // start monitoring directories that go added
        for (auto& path : m_tempAddedDirectories)
            monitorPath(path);

        // lock access to this thread only
        ScopeLock<Mutex> lock(m_listenersLock);

        // send events to the listeners
        for (auto& evt : m_tempEvents)
        {
            for (auto listener : m_listeners)
            {
                if (listener != nullptr)
                    listener->handleEvent(evt);
            }
        }

        // remove the empty listeners
        m_listeners.remove(nullptr);
    }
}

//--

AsyncReadDispatcher::AsyncReadDispatcher()
{
}

AsyncReadDispatcher::~AsyncReadDispatcher()
{
}

uint64_t AsyncReadDispatcher::readAsync(int hFile, uint64_t offset, uint64_t size, void* outMemory)
{
    PC_SCOPE_LVL1(AsyncRead, profiler::colors::Red800);

    ScopeLock<> lock(m_lock);

    lseek64(hFile, offset, SEEK_SET);
    return read(hFile, outMemory, size);
}

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

FileReaderPtr FileSystem::openForReading(StringView str, TimeStamp* outTimestamp)
{
    const StringBuf absoluteFilePath(str);

    // get timestamp if request
    if (outTimestamp)
    {
        // get file stats
        struct stat st;
        if (0 != stat(absoluteFilePath.c_str(), &st))
            return nullptr;

        *outTimestamp = TimeStamp::GetFromFileTime(st.st_mtim.tv_sec, st.st_mtim.tv_nsec);
    }

    // open file
    auto handle = open(absoluteFilePath.c_str(), O_RDONLY);
    if (handle <= 0)
    {
        TRACE_ERROR("Failed to create reading handle for '{}'", absoluteFilePath);
        return nullptr;
    }

    // Return file reader
    return RefNew<ReadFileHandle>(handle, absoluteFilePath);
}

FileWriterPtr FileSystem::openForWritingLowLevel(const StringBuf& absoluteFilePath, bool append)
{
    // Create path
    if (!createPath(absoluteFilePath))
    {
        TRACE_ERROR("Failed to create path for '{}'", absoluteFilePath);
        return nullptr;
    }

    // Remove the read only flag
    readOnlyFlag(absoluteFilePath, false);

    // Open file
    auto handle = open(absoluteFilePath.c_str(), O_WRONLY | O_CREAT | (append ? 0 : O_TRUNC), DEFFILEMODE);
    if (handle <= 0)
    {
        auto ret = errno;
        TRACE_ERROR("Failed to create writing handle for '{}': {}", absoluteFilePath, ret);
        return nullptr;
    }

    // Move file pointer
    uint32_t pos = 0;
    if (append)
        pos = lseek64(handle, 0, SEEK_END);

    // Create the wrapper
    return RefNew<WriteFileHandle>(handle, absoluteFilePath);
}

static FileWriterPtr GenerateTempFilePath(StringBuf& outStr)
{
    char filename[64] = "/tmp/inferno-XXXXXX";

    int handle = mkstemp(filename);
    if (handle < 0)
    {
        TRACE_ERROR("Failed to determine temporary file name");
        return nullptr;
    }

    outStr = StringBuf(filename);

    return RefNew<WriteFileHandle>(handle, outStr);
}

FileWriterPtr FileSystem::openForWriting(StringView pathStr, FileWriteMode mode)
{
    const StringBuf absoluteFilePath(pathStr);

    // Create path
    if (!createPath(pathStr))
    {
        TRACE_WARNING("[FILE] Failed to create path for '{}'", absoluteFilePath);
        return nullptr;
    }

    // Remove the read only flag
    if (!readOnlyFlag(pathStr, false))
    {
        TRACE_WARNING("[FILE] Unable to remove read only flag from file '{}', assuming it's protected", absoluteFilePath);
        return nullptr;
    }

    // Staged write
    if (mode == FileWriteMode::StagedWrite)
    {
        StringBuf tempFilePath;

        // create temp file writer
        auto tempFileWriter = GenerateTempFilePath(tempFilePath);
        if (!tempFileWriter)
        {
            TRACE_WARNING("[FILE] Unable to create temp writing file for '{}'", absoluteFilePath);
            return nullptr;
        }

        // create wrapper
        return RefNew<WriteTempFileHandle>(absoluteFilePath, tempFilePath, tempFileWriter);
    }

    // open file
    return openForWritingLowLevel(absoluteFilePath, mode == FileWriteMode::DirectAppend);
}

AsyncFileHandlePtr FileSystem::openForAsyncReading(StringView str, TimeStamp* outTimestamp)
{
    const StringBuf absoluteFilePath(str);

    // get file stats
    struct stat st;
    if (0 != stat(absoluteFilePath.c_str(), &st))
        return nullptr;

    // get timestamp if request
    if (outTimestamp)
        *outTimestamp = TimeStamp::GetFromFileTime(st.st_mtim.tv_sec, st.st_mtim.tv_nsec);

    // get file size
    const auto size = st.st_size;

    // open file
    auto handle = open(absoluteFilePath.c_str(), O_RDONLY);
    if (handle <= 0)
    {
        TRACE_ERROR("Failed to create reading handle for '{}'", absoluteFilePath);
        return nullptr;
    }

    // Return file reader
    return RefNew<AsyncFileHandle>(handle, absoluteFilePath, size, m_asyncDispatcher);
}

Buffer FileSystem::openMemoryMappedForReading(StringView absoluteFilePath, TimeStamp* outTimestamp)
{
    Buffer ret;
    loadFileToBuffer(absoluteFilePath, ret, "MemoryMappedFile", outTimestamp);
    return ret;
}

bool FileSystem::loadFileToBuffer(StringView str, Buffer& outBuffer, const char* tag /*= nullptr*/, TimeStamp* outTimestamp /*= nullptr*/)
{
    const prv::TempPathStringBuffer absoluteFilePath(str);

    // open file
    auto handle = open(absoluteFilePath, O_RDONLY);
    if (handle <= 0)
    {
        TRACE_ERROR("Failed to create reading handle for '{}'", absoluteFilePath);
        return false;
    }

    // get file size
    auto pos = lseek64(handle, 0, SEEK_CUR);
    auto size = lseek64(handle, 0, SEEK_END);
    lseek64(handle, pos, SEEK_SET);

    // create buffer
    auto ret = Buffer::CreateInSystemMemory(size, tag);
    if (!ret)
    {
        TRACE_WARNING("Unable to allocate {} needed to load file '{}'", MemSize(size), absoluteFilePath);
        close(handle);
        return false;
    }

    // load content
    auto numRead = read(handle, ret.data(), size);
    close(handle);

    // make sure we read all
    if (numRead != size)
    {
        TRACE_WARNING("Error reading content of file '{}'", absoluteFilePath);
        return false;
    }

    // return the timestamp
    if (outTimestamp)
    {
        // get file stats
        struct stat st;
        if (0 != stat(absoluteFilePath, &st))
            return false;

        *outTimestamp = TimeStamp::GetFromFileTime(st.st_mtim.tv_sec, st.st_mtim.tv_nsec);
    }

    // done
    outBuffer = ret;
    return true;
}

bool FileSystem::createPath(StringView str)
{
    const prv::TempPathStringBuffer absolutePath(str);

    // Create path
    char* path = absolutePath.pos();
    for (char* pos = path; *pos; pos++)
    {
        if (*pos == '\\' || *pos == '/')
        {
            char was = *pos;
            *pos = 0;

            if (strlen(path) > 0)
            {
                if (0 != mkdir(path, ALLPERMS))
                {
                    auto ret = errno;
                    if (ret != EEXIST)
                    {
                        TRACE_ERROR("Failed to create path '{}': {}", path, ret);
                        return false;
                    }
                }
            }

            *pos = was;
        }
    }

    // Path created
    return true;
}

bool FileSystem::moveFile(StringView srcAbsolutePath, StringView destAbsolutePath)
{
    const prv::TempPathStringBuffer srcFilePath(srcAbsolutePath);
    const prv::TempPathStringBuffer destFilePath(destAbsolutePath);

    // Delete destination file
    if (-1 != access(destFilePath, F_OK))
    {
        TRACE_ERROR("Unable to move file \"{}\" to \"{}\" - destination file already exists", srcAbsolutePath, destAbsolutePath);
        return false;
    }

    // create target path
    if (!createPath(destAbsolutePath))
    {
        TRACE_ERROR("Unable to move file \"{}\" to \"{}\", Unable to create target path", srcAbsolutePath, destAbsolutePath);
        return false;
    }

    // Move the file
    if (0 != rename(srcFilePath, destFilePath))
    {
        auto err = errno;
        TRACE_ERROR("Unable to move file \"{}\" to \"{}\", Error: {}", srcAbsolutePath, destAbsolutePath, err);
        return false;
    }

    // File moved
    return true;
}

bool FileSystem::copyFile(StringView srcAbsolutePath, StringView destAbsolutePath)
{
    const prv::TempPathStringBuffer srcFilePath(srcAbsolutePath);
    const prv::TempPathStringBuffer destFilePath(destAbsolutePath);

    // Delete destination file
    if (-1 != access(destFilePath, F_OK))
    {
        TRACE_ERROR("Unable to move file \"{}\" to \"{}\" - destination file already exists", srcAbsolutePath, destAbsolutePath);
        return false;
    }

    // create target path
    if (!createPath(destAbsolutePath))
    {
        TRACE_ERROR("Unable to move file \"{}\" to \"{}\", Unable to create target path", srcAbsolutePath, destAbsolutePath);
        return false;
    }

    // TODO!

    return false;
}

bool FileSystem::deleteFile(StringView str)
{
    if (!readOnlyFlag(str, false))
        return false;

    // Delete the file
    const prv::TempPathStringBuffer absoluteFilePath(str);
    if (0 != remove(absoluteFilePath))
    {
        auto err = errno;
        TRACE_ERROR("Unable to delete file \"{}\", Error: {}", absoluteFilePath, err);
        return false;
    }

    // File deleted
    return true;
}

bool FileSystem::deleteDir(StringView str)
{
    const prv::TempPathStringBuffer absoluteDirPath(str);

    // Delete the file
    if (0 != remove(absoluteDirPath))
    {
        auto err = errno;
        TRACE_ERROR("Unable to delete directory \"{}\", Error: {}", absoluteDirPath, err);
        return false;
    }

    // Directory deleted
    return true;
}

bool FileSystem::touchFile(StringView str)
{
    const prv::TempPathStringBuffer absoluteFilePath(str);

#if 0
    time_t curTime = 0;
    time(&curTime);

    utimbuf buf;
    buf.modtime = curTime;
    buf.actime = curTime;
    utime(filePath, &buf);
#endif

    return false;
}

bool FileSystem::fileInfo(StringView str, TimeStamp* outTimeStamp /*= nullptr*/, uint64_t* outFileSize /*= nullptr*/)
{
    const prv::TempPathStringBuffer absoluteFilePath(str);

    // get file stats
    struct stat st;
    if (0 != stat(absoluteFilePath, &st))
        return false;

    if (outTimeStamp)
        *outTimeStamp = TimeStamp::GetFromFileTime(st.st_mtim.tv_sec, st.st_mtim.tv_nsec);

    if (outFileSize)
        *outFileSize = st.st_size;

    return true;
}

bool FileSystem::isFileReadOnly(StringView absoluteFilePath)
{
    return false;
}

bool FileSystem::readOnlyFlag(StringView absoluteFilePath, bool flag)
{
    return true;
}

//--

static bool FindFilesInternal(prv::TempPathStringBuffer& dirPath, const char* searchPattern, const std::function<bool(StringView fullPath, StringView fileName)>& enumFunc, bool recurse)
{
    auto* org = dirPath.pos();

    for (FileIterator it(dirPath, searchPattern, true, false); it; ++it)
    {
        dirPath.pop(org);

        const auto* fileName = it.fileName();

        if (dirPath.append(fileName))
            if (enumFunc((const char*)dirPath, fileName))
                return true;
    }

    dirPath.pop(org);

    if (recurse)
    {
        for (FileIterator it(dirPath, "*.", false, true); it; ++it)
        {
            if (dirPath.append(it.fileName()) && dirPath.append("\\"))
            {
                if (FindFilesInternal(dirPath, searchPattern, enumFunc, recurse))
                    return true;
            }
        }
    }

    return false;
}

bool FileSystem::enumFiles(StringView str, StringView str2, const std::function<bool(StringView fullPath, StringView fileName)>& enumFunc, bool recurse)
{
    prv::TempPathStringBuffer absoluteFilePath(str);
    const prv::TempPathStringBuffer searchPattern(str2);

    return FindFilesInternal(absoluteFilePath, searchPattern, enumFunc, recurse);
}

bool FileSystem::enumSubDirs(StringView str, const std::function<bool(StringView name)>& enumFunc)
{
    const prv::TempPathStringBuffer absoluteFilePath(str);

    for (FileIterator it(absoluteFilePath, "*.", false, true); it; ++it)
        if (enumFunc(it.fileName()))
            return true;

    return false;
}

bool FileSystem::enumLocalFiles(StringView str, StringView str2, const std::function<bool(StringView name)>& enumFunc)
{
    const prv::TempPathStringBuffer absoluteFilePath(str);
    const prv::TempPathStringBuffer searchPattern(str2);

    for (FileIterator it(absoluteFilePath, searchPattern, true, false); it; ++it)
        if (enumFunc(it.fileName()))
            return true;

    return false;
}

bool FileSystem::enumFileSystemRoots(const std::function<bool(StringView name)>& enumFunc, bool allowNetworkDrives)
{
    return enumFunc("/");
}

DirectoryWatcherPtr FileSystem::createDirectoryWatcher(StringView path)
{
    return RefNew<DirectoryWatcher>(path);
}

//---

void FileSystem::cacheSystemPaths()
{

}

//--

END_INFERNO_NAMESPACE_EX(posix)

#endif
