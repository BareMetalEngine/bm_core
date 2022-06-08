/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

#include "bm/core/containers/include/array.h"
#include "bm/core/containers/include/stringView.h"

BEGIN_INFERNO_NAMESPACE()

//--


//--

// type of location we want to retrieve from the system
enum class FileSystemGlobalPath : uint8_t
{
    // path to the executable file currently running
    ExecutableFile,

    // path to the bin/ directory we are running from
    ExecutableDir,

    // engine directory (contains engine's src, data and config)
    EngineDir,

    // project directory (only if valid)
    //ProjectDir,

    // path to the system wide temporary directory when temporary files may be stored
    // NOTE: temp directory may be totally purged between runs
    SystemTempDir,

    // project local temp directory that has higher tendency to stay around
    LocalTempDir,

    // path to the user config directory - a directory where we can store user specific configuration files (like settings)
    UserConfigDir,

    // path to "My Documents" or /home/XXX/ or something similar
    UserDocumentsDir,

    MAX,
};

//--

/// low-level IO system handler
class BM_CORE_FILE_API IFileSystem : public IReferencable
{
public:
    IFileSystem();
    virtual ~IFileSystem();

    //----

    // get a path to some specific shit
    INLINE const StringBuf& globalPath(FileSystemGlobalPath category) const { return m_globalPaths[(int)category]; }

    //----

    // open physical file for reading
    virtual FileReaderPtr openForReading(StringView absoluteFilePath, FileReadMode mode, TimeStamp* outTimestamp = nullptr) const = 0;

    // open physical file for writing
    virtual FileWriterPtr openForWriting(StringView absoluteFilePath, FileWriteMode mode) = 0;

    //--

    //! Make sure all directories along the way exist
    virtual bool createPath(StringView absoluteFilePath) = 0;

    //! Move file
    virtual bool moveFile(StringView srcAbsolutePath, StringView destAbsolutePath) = 0;

    //! Copy file
    virtual bool copyFile(StringView srcAbsolutePath, StringView destAbsolutePath) = 0;

    //! Delete file from disk
    virtual bool deleteFile(StringView absoluteFilePath) = 0;

    //! Delete folder from disk, MUST BE EMPTY (for safety)
    virtual bool deleteDir(StringView absoluteDirPath) = 0;

    //--

    //! Check if file exists and if so, get some file info
    virtual bool fileInfo(StringView absoluteFilePath, TimeStamp* outTimeStamp = nullptr, uint64_t* outFileSize = nullptr) const = 0;

    //! Update modification date on the file
    virtual bool touchFile(StringView absoluteFilePath) = 0;

    //! Check if file is read only
    virtual bool isFileReadOnly(StringView absoluteFilePath) const = 0;

    //! Change read only attribute on file
    virtual bool readOnlyFlag(StringView absoluteFilePath, bool flag) = 0;

    //--

    //! Enumerate files in given directory, NOTE: slow as fuck
    virtual bool enumFiles(StringView absoluteFilePath, StringView searchPattern, const std::function<bool(StringView fullPath, StringView fileName)>& enumFunc, bool recurse) const = 0;

    //! Enumerate directories in given directory, NOTE: slow as fuck
    virtual bool enumSubDirs(StringView absoluteFilePath, const std::function<bool(StringView name)>& enumFunc) const = 0;

    //! Enumerate file in given directory, not recursive version, NOTE: slow as fuck
    virtual bool enumLocalFiles(StringView absoluteFilePath, StringView searchPattern, const std::function<bool(StringView name)>& enumFunc) const = 0;

    //! Enumerate system root paths (drive letters, etc)
    virtual bool enumFileSystemRoots(const std::function<bool(StringView name)>& enumFunc, bool allowNetworkDrives) const = 0;

    //--

    //! Collect list of files in given directory (can be recursive)
    void collectFiles(StringView absoluteFilePath, StringView searchPattern, Array<StringBuf>& outAbsoluteFiles, bool recurse) const;

    //! Collect list of direct child directories
    void collectSubDirs(StringView absoluteFilePath, Array<StringBuf>& outDirectoryNames) const;

    //! Collect list of local files
    void collectLocalFiles(StringView absoluteFilePath, StringView searchPattern, Array<StringBuf>& outFileNames) const;

    //--

    //! Create asynchronous directory watcher
    virtual DirectoryWatcherPtr createDirectoryWatcher(StringView path) = 0;

    //--

/*    //! Show the given file in the file explorer
    virtual void showFileExplorer(StringView path) = 0;

    //! Show the system "Open File" dialog
    //! This pops up the native system window in which user can select a file(s) to be opened
    virtual bool showFileOpenDialog(uint64_t nativeWindowHandle, bool allowMultiple, const Array<FileFormat>& formats, Array<StringBuf>& outPaths, OpenSavePersistentData& persistentData) = 0;

    //! Show the system "Save File" dialog
    //! This pops up the native system window in which user can select a file(s) to be opened
    virtual bool showFileSaveDialog(uint64_t nativeWindowHandle, const StringBuf& currentFileName, const Array<FileFormat>& formats, StringBuf& outPath, OpenSavePersistentData& persistentData) = 0;*/

    //--

    // load content of an absolute file on disk to a string buffer
    // returns true if content was loaded, false if there were errors (file does not exist, etc)
    // NOTE: both ANSI UTF-8 and UTF-16 files are supported, the UTF-16 files are automatically converted
    bool loadFileToString(StringView absoluteFilePath, StringBuf& outString, TimeStamp* outTimestamp = nullptr);

    // load content of an absolute file on disk to a string buffer, returns empty string if file does not exist
    // NOTE: both ANSI UTF-8 and UTF-16 files are supported, the UTF-16 files are automatically converted
    StringBuf loadFileToString(StringView absoluteFilePath, TimeStamp* outTimestamp = nullptr);

    //--

    // load file content into a memory buffer
    // returns true if content was loaded, false if there were errors (file does not exist, etc)
    // NOTE: buffer is usually allocated outside of the normal memory pools
    virtual bool loadFileToBuffer(StringView absoluteFilePath, IPoolUnmanaged& pool, Buffer& outBuffer, TimeStamp* outTimestamp = nullptr, FileReadMode mode = FileReadMode::MemoryMapped) const = 0;

    // load file content into a memory buffer, uses OS dependent implementation for maximum efficiency
    // NOTE: buffer is usually allocated outside of the normal memory pools
    Buffer loadFileToBuffer(StringView absoluteFilePath, TimeStamp* outTimestamp = nullptr) const;

    //----

    // encoding of the file saved
    enum class StringEncoding : uint8_t
    {
        Ansi, // all chars > 255 are saved as ?
        UTF8, // native, no data conversion
        UTF16, // expands to 16-bits, larger values are written as ?
    };

    // save string (ANSI / UTF16) to file on disk
    // returns true if content was saved, false if there were errors (file could not be created, etc)
    // NOTE: uses safe replacement method via a temp file
    bool saveFileFromString(StringView absoluteFilePath, StringView str, StringEncoding encoding = StringEncoding::UTF8, const TimeStamp* timestampToAssign = nullptr);

    // save block of memory to file on disk
    // returns true if content was saved, false if there were errors (file could not be created, etc)
    // NOTE: uses safe replacement method via a temp file
    virtual bool saveFileFromBuffer(StringView absoluteFilePath, BufferView data, const TimeStamp* timestampToAssign = nullptr) = 0;

    //--

    // calculate CRC of the file's content
    virtual bool calculateFileCRC64(StringView absoluteFilePath, uint64_t& outCRC64, TimeStamp* outTimestamp = nullptr, uint64_t* outSize = nullptr) const;

    //--

protected:
    StringBuf m_globalPaths[(int)FileSystemGlobalPath::MAX];
};

//--

END_INFERNO_NAMESPACE()
