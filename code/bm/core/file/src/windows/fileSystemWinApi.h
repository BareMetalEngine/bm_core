/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

#include "fileFormat.h"
#include "fileSystem.h"
#include "fileUtils.h"

#ifdef PLATFORM_WINAPI

BEGIN_INFERNO_NAMESPACE_EX(windows)

//--

class AsyncReadDispatcher;

class FileSystem : public IFileSystem
{
public:
    FileSystem();
    virtual ~FileSystem();

	virtual FileReaderPtr openForReading(StringView absoluteFilePath, FileReadMode mode, TimeStamp* outTimestamp = nullptr) const override final;
	virtual FileWriterPtr openForWriting(StringView absoluteFilePath, FileWriteMode mode) override final;

    virtual bool fileInfo(StringView absoluteFilePath, TimeStamp* outTimeStamp = nullptr, uint64_t* outFileSize = nullptr) const override final;

    virtual bool createPath(StringView absoluteFilePath) override final;
    virtual bool moveFile(StringView srcAbsolutePath, StringView destAbsolutePath) override final;
    virtual bool copyFile(StringView srcAbsolutePath, StringView destAbsolutePath) override final;
    virtual bool deleteFile(StringView absoluteFilePath) override final;
	virtual bool deleteDir(StringView absoluteDirPath) override final;
    virtual bool touchFile(StringView absoluteFilePath) override final;

    virtual bool isFileReadOnly(StringView absoluteFilePath) const override final;
    virtual bool readOnlyFlag(StringView absoluteFilePath, bool flag) override final;

    virtual bool enumFiles(StringView absoluteFilePath, StringView searchPattern, const std::function<bool(StringView fullPath, StringView fileName)>& enumFunc, bool recurse) const override final;
    virtual bool enumSubDirs(StringView absoluteFilePath, const std::function<bool(StringView name)>& enumFunc) const override final;
    virtual bool enumLocalFiles(StringView absoluteFilePath, StringView searchPattern, const std::function<bool(StringView name)>& enumFunc) const override final;
    virtual bool enumFileSystemRoots(const std::function<bool(StringView name)>& enumFunc, bool allowNetworkDrives) const override final;

    virtual DirectoryWatcherPtr createDirectoryWatcher(StringView path) override final;
    
    virtual bool loadFileToBuffer(StringView absoluteFilePath, IPoolUnmanaged& pool, Buffer& outBuffer, TimeStamp* outTimestamp = nullptr, FileReadMode mode = FileReadMode::MemoryMapped) const override final;
    virtual bool saveFileFromBuffer(StringView absoluteFilePath, BufferView data, const TimeStamp* timestampToAssign = nullptr) override final;

    //--

private:
    AsyncReadDispatcher* m_asyncDispatcher;

    bool findFilesInternal(TempPathStringBufferUTF16& dirPath, TempPathStringBufferAnsi& dirPathUTF, StringView searchPattern, const std::function<bool(StringView fullPath, StringView fileName)>& enumFunc, bool recurse) const;
    bool loadFileToBuffer_MemoryMapped(StringView absoluteFilePath, Buffer& outBuffer, TimeStamp* outTimestamp) const;
    bool loadFileToBuffer_ReadWhole(StringView absoluteFilePath, IPoolUnmanaged& pool, Buffer& outBuffer, TimeStamp* outTimestamp) const;

    void cacheSystemPaths();
};

//--

END_INFERNO_NAMESPACE_EX(windows)

#endif
