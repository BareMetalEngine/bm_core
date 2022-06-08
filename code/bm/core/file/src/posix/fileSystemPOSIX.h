/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

#ifdef PLATFORM_POSIX

#include "fileFormat.h"
#include "fileSystem.h"
#include "fileReader.h"
#include "fileAsyncHandle.h"
#include "fileDirectoryWatcher.h"

#include "bm/core/memory/include/structurePool.h"
#include "bm/core/containers/include/queue.h"
#include "bm/core/containers/include/hashMap.h"
#include "bm/core/system/include/thread.h"
#include "bm/core/containers/include/array.h"
#include "bm/core/system/include/mutex.h"

BEGIN_INFERNO_NAMESPACE_EX(posix)

//--

class AsyncReadDispatcher;

// POSIX based file handle
class ReadFileHandle : public IFileReader
{
public:
    ReadFileHandle(int hFile, const StringBuf& path);
    virtual ~ReadFileHandle();

    INLINE int handle() const { return m_fileHandle; }

    // IFileReader implementation
    virtual uint64_t size() const override final;
    virtual uint64_t pos() const override final;
    virtual bool pos(uint64_t newPosition) override final;
    virtual uint64_t readSync(void* data, uint64_t size) override final;

protected:
    int m_fileHandle = -1;
    StringBuf m_origin;
};

// POSIX based file handle
class WriteFileHandle : public IFileWriter
{
public:
    WriteFileHandle(int hFile, const StringBuf& path);
    virtual ~WriteFileHandle();

    INLINE int handle() const { return m_fileHandle; }

    // IFileReader implementation
    virtual uint64_t size() const override final;
    virtual uint64_t pos() const override final;
    virtual bool pos(uint64_t newPosition) override final;
    virtual uint64_t writeSync(const void* data, uint64_t size) override final;
    virtual void discardContent() override final;

protected:
    int m_fileHandle = -1;
    StringBuf m_origin;
};

// WinAPI based file handle for sync WRITE operations
class WriteTempFileHandle : public IFileWriter
{
public:
    WriteTempFileHandle(const StringBuf& targetPath, const StringBuf& tempFilePath, const FileWriterPtr& tempFileWriter);
    virtual ~WriteTempFileHandle();

    // IFileHandle implementation
    virtual uint64_t size() const override final;
    virtual uint64_t pos() const override final;
    virtual bool pos(uint64_t newPosition) override final;
    virtual uint64_t writeSync(const void* data, uint64_t size) override final;
    virtual void discardContent() override final;

protected:
    StringBuf m_tempFilePath;
    StringBuf m_targetFilePath;
    FileWriterPtr m_tempFileWriter;
};

//--

class AsyncReadDispatcher;

// WinAPI based file handle
class AsyncFileHandle : public IAsyncFileHandle
{
public:
    AsyncFileHandle(int hAsyncFile, const StringBuf& origin, uint64_t size, AsyncReadDispatcher* dispatcher);
    virtual ~AsyncFileHandle();

    INLINE int handle() const { return m_fileHandle; }

    // IAsyncFileHandle
    virtual uint64_t size() const override final;
    virtual CAN_YIELD uint64_t readAsync(uint64_t offset, uint64_t size, void* readBuffer) override final;

protected:
    AsyncReadDispatcher* m_dispatcher = nullptr;

    int m_fileHandle = -1;
    uint64_t m_size = 0; // at the time file was opened

    StringBuf m_origin;
};

//--

// dispatch for IO jobs
class AsyncReadDispatcher : public MainPoolData<NoCopy>
{
public:
    AsyncReadDispatcher();
    ~AsyncReadDispatcher();

    uint64_t readAsync(int file, uint64_t offset, uint64_t size, void* outMemory);

private:
    Mutex m_lock;
};

//--

 /// POSIX implementation of the watcher
class DirectoryWatcher : public IDirectoryWatcher
{
public:
    DirectoryWatcher(StringView rootPath);
    virtual ~DirectoryWatcher();

    //! attach listener
    virtual void attachListener(IDirectoryWatcherListener* listener) override;
    virtual void dettachListener(IDirectoryWatcherListener* listener) override;

private:
    static const uint32_t BUF_LEN = 1024 * 64;

    int m_masterHandle;

    Mutex m_listenersLock;
    Array<IDirectoryWatcherListener*> m_listeners;

    SpinLock m_mapLock;
    HashMap<int, StringBuf> m_handleToPath;
    HashMap<uint64_t, int> m_pathToHandle;

    Thread m_localThread;

    Array<DirectoryWatcherEvent> m_tempEvents;
    Array<StringBuf> m_tempAddedDirectories;
    Array<StringBuf> m_tempRemovedDirectories;

    Array<StringBuf> m_filesCreatedButNotYetClosed;
    Array<StringBuf> m_filesModifiedButNotYetClosed;

    uint8_t m_buffer[BUF_LEN];

    void monitorPath(StringView path);
    void unmonitorPath(StringView path);

    void watch();
};

//--

class FileIterator
{
public:
    FileIterator(const char* searchAbsolutePath, const char* pattern, bool allowFiles, bool allowDirs);
    ~FileIterator();

    void operator++(int);
    void operator++();

    operator bool() const;

    const char* fileName() const;

private:
    bool validateEntry() const;
    bool nextEntry();

    void* m_dir;
    void* m_entry;

    bool m_allowDirs;
    bool m_allowFiles;

    bool m_matchPattern = false;
    const char* m_searchPath = nullptr;
    const char* m_searchPattern = nullptr;
};

//--

class FileSystem : public IFileSystem
{
public:
    FileSystem();
    virtual ~FileSystem();

    virtual FileReaderPtr openForReading(StringView absoluteFilePath, TimeStamp* outTimestamp) override final;
    virtual FileWriterPtr openForWriting(StringView absoluteFilePath, FileWriteMode mode = FileWriteMode::StagedWrite) override final;
    virtual AsyncFileHandlePtr openForAsyncReading(StringView absoluteFilePath, TimeStamp* outTimestamp) override final;
    virtual Buffer openMemoryMappedForReading(StringView absoluteFilePath, TimeStamp* outTimestamp) override final;

    virtual bool fileInfo(StringView absoluteFilePath, TimeStamp* outTimeStamp = nullptr, uint64_t* outFileSize = nullptr) override final;

    virtual bool createPath(StringView absoluteFilePath) override final;
    virtual bool moveFile(StringView srcAbsolutePath, StringView destAbsolutePath) override final;
    virtual bool copyFile(StringView srcAbsolutePath, StringView destAbsolutePath) override final;
    virtual bool deleteFile(StringView absoluteFilePath) override final;
	virtual bool deleteDir(StringView absoluteDirPath) override final;
    virtual bool touchFile(StringView absoluteFilePath) override final;

    virtual bool isFileReadOnly(StringView absoluteFilePath) override final;
    virtual bool readOnlyFlag(StringView absoluteFilePath, bool flag) override final;

    virtual bool enumFiles(StringView absoluteFilePath, StringView searchPattern, const std::function<bool(StringView fullPath, StringView fileName)>& enumFunc, bool recurse) override final;
    virtual bool enumSubDirs(StringView absoluteFilePath, const std::function<bool(StringView name)>& enumFunc) override final;
    virtual bool enumLocalFiles(StringView absoluteFilePath, StringView searchPattern, const std::function<bool(StringView name)>& enumFunc) override final;
    virtual bool enumFileSystemRoots(const std::function<bool(StringView name)>& enumFunc, bool allowNetworkDrives) override final;

    virtual DirectoryWatcherPtr createDirectoryWatcher(StringView path) override final;
    
    virtual bool loadFileToBuffer(StringView absoluteFilePath, Buffer& outBuffer, const char* tag = nullptr, TimeStamp* outTimestamp = nullptr) override final;

    //--

    FileWriterPtr openForWritingLowLevel(const StringBuf& path, bool append);

private:
    AsyncReadDispatcher* m_asyncDispatcher;

    void cacheSystemPaths();
};

//--

END_INFERNO_NAMESPACE_EX(posix)

#endif
