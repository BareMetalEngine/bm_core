/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

#include "bm/core/file/include/fileSystem.h"
#include "bm/core/file/include/fileDirectoryWatcher.h"
#include "bm/core/memory/include/structureAllocator.h"

BEGIN_INFERNO_NAMESPACE()

//--

class MemoryFileSystemWatcher;

//--

// Fully in-memory file system with ALL features of a normal file system, including file notifications
// Mostly used for testing as a mock file system
class BM_CORE_FILE_API MemoryFileSystem : public IFileSystem
{
public:
    MemoryFileSystem();
    virtual ~MemoryFileSystem();

    //--

    //-----
    // IFileSystem 
    virtual FileReaderPtr openForReading(StringView absoluteFilePath, FileReadMode mode, TimeStamp* outTimestamp = nullptr) const override final;
    virtual FileWriterPtr openForWriting(StringView absoluteFilePath, FileWriteMode mode) override final;

    virtual bool createPath(StringView absoluteFilePath) override final;
    virtual bool moveFile(StringView srcAbsolutePath, StringView destAbsolutePath) override final;
    virtual bool copyFile(StringView srcAbsolutePath, StringView destAbsolutePath) override final;
    virtual bool deleteFile(StringView absoluteFilePath) override final;
    virtual bool deleteDir(StringView absoluteDirPath) override final;

    virtual bool fileInfo(StringView absoluteFilePath, TimeStamp* outTimeStamp = nullptr, uint64_t* outFileSize = nullptr) const override final;
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

    void testCollectFiles(StringView absoluteFilePath, StringView searchPattern, Array<StringBuf>& outPaths, bool recursive=true) const
    {
        enumFiles(absoluteFilePath, searchPattern, [&outPaths](StringView fullPath, StringView fileName)
            {
                outPaths.emplaceBack(fullPath);
                return false;
            }, recursive);
    }

    void testCollectSubDirs(StringView absoluteFilePath, Array<StringBuf>& outNames) const
    {
        enumSubDirs(absoluteFilePath, [&outNames](StringView name)
            {
                outNames.emplaceBack(name);
                return false;
            });
    }

    void testCollectLocalFiles(StringView absoluteFilePath, StringView searchPattern, Array<StringBuf>& outNames) const
    {
		enumLocalFiles(absoluteFilePath, searchPattern, [&outNames](StringView name)
			{
				outNames.emplaceBack(name);
				return false;
			});
    }

    void testCollectFileSystemRoots(Array<StringBuf>& outDrivers) const
    {
        enumFileSystemRoots([&outDrivers](StringView name)
            {
                outDrivers.emplaceBack(name);
                return false;
            }, true);
    }

    //--

	bool testStoreFileContent(StringView path, Buffer data, TimeStamp timestamp = TimeStamp::GetNow());
    bool testStoreFileContentText(StringView path, StringView data, TimeStamp timestamp = TimeStamp::GetNow());

	bool testHasFile(StringView path) const;
    bool testHasDirectory(StringView path) const;

	Buffer testLoadFileContent(StringView path) const;
	StringBuf testLoadFileContentText(StringView path) const;
	
    bool testUpdateFileContentExternal(StringView path, Buffer data, TimeStamp timestamp = TimeStamp::GetNow());

    //--

protected:
	//--

	struct Directory;

	struct File : public MainPoolData<NoCopy>
	{
		Directory* parent = nullptr;
		StringBuf name;
		Buffer content;
		TimeStamp timestamp;
		bool readonly = false;
		bool deleted = false;
        File* next = nullptr;
	};

	struct Directory : public MainPoolData<NoCopy>
	{
		Directory* parent = nullptr;
		StringBuf name;
		//StringBuf path;

		File* firstFile = nullptr;
        File* lastFile = nullptr;

		Directory* firstDir = nullptr;
        Directory* lastDir = nullptr;

        Directory* next = nullptr;
		bool deleted = false;
	};

	Directory* m_root = nullptr;
    StructureAllocator<Directory> m_directoryPool;
    StructureAllocator<File> m_filePool;	

	Directory* testCreatePath(StringView absoluteFilePath, bool notify);

	Directory* testDirectoryEntryFind(StringView path);
	const Directory* testDirectoryEntryFind(StringView path) const;

	File* testFileEntryFind(StringView path);
	const File* testFileEntryFind(StringView path) const;

    void cleanupDirectory(Directory* dir);

    //--

    Array<RefWeakPtr<MemoryFileSystemWatcher>> m_watchers;

	StringBuf fileAbsolutePath(const File* file) const;
	StringBuf dirAbsolutePath(const Directory* dir) const;
    void dirAbsolutePath(const Directory* dir, IFormatStream& f) const;

	void notiftFileAdded(const File* file);
	void notiftFileRemoved(const File* file);
	void notiftFileChanged(const File* file);
	void notiftDirectoryAdded(const Directory* dir);
	void notiftDirectoryRemoved(const Directory* dir);

	void dispatchFileSystemEvent(const DirectoryWatcherEvent& evt);

    //--
};

//--

END_INFERNO_NAMESPACE()
