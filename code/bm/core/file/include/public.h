/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

#include "bm_core_file_glue.inl"

BEGIN_INFERNO_NAMESPACE()

//--

static const char WINDOWS_PATH_SEPARATOR = '\\';
static const char UNIX_PATH_SEPARATOR = '/';

#if defined(PLATFORM_WINDOWS)
static const char SYSTEM_PATH_SEPARATOR = WINDOWS_PATH_SEPARATOR;
static const char WRONG_SYSTEM_PATH_SEPARATOR = UNIX_PATH_SEPARATOR;
#else
static const char SYSTEM_PATH_SEPARATOR = UNIX_PATH_SEPARATOR;
static const char WRONG_SYSTEM_PATH_SEPARATOR = WINDOWS_PATH_SEPARATOR;
#endif

class IFileSystem;

class IFileView;
typedef RefPtr<IFileView> FileViewPtr;

class IFileReader;
typedef RefPtr<IFileReader> FileReaderPtr;

class IFileMapping;
typedef RefPtr<IFileMapping> FileMappingPtr;

class IFileWriter;
typedef RefPtr<IFileWriter> FileWriterPtr;

typedef std::function<void(int actualReadSize)> TAsyncReadCallback; // negative read size on errors
typedef std::function<void(FileMappingPtr mappedView)> TAsyncMappingCallback;

class IDirectoryWatcher;
class IDirectoryWatcherListener;
typedef RefPtr<IDirectoryWatcher> DirectoryWatcherPtr;

class FileFormat;
struct FileAbsoluteRange;
struct FileRelativeOffsetSize;

//--

class IFileSystemMonitoring;

typedef int64_t TFileSystemMonitoringToken; // internal ID carried by operations to link the together

//--

/// generic flags for file reading
enum class FileFlagBit : uint32_t
{
    FileBacked, // there's an actual file on disk we are reading from
	MemoryBacked, // view is memory mapped, it's possible to get direct pointer to data
	MemoryMapped, // (only for MemoryBacked) - memory comes from memory mapping and is not loaded
    Buffered, // view is buffered with additional internal buffer, reads may not reflect current file state
    Detached, // view does NOT own the memory it operates on
    Packaged, // content is coming from a package
};

/// file reader flags
typedef BitFlags<FileFlagBit> FileFlags;

//--

/// mode for opening file for writing
enum class FileReadMode : uint8_t
{
	DirectNonBuffered, // create a raw (non memory mapped) interface with unbuffered reads
	DirectBuffered, // create a raw (non memory mapped) interface with buffered reads
	MemoryMapped, // open as a memory mapped file
};

/// mode for opening file for writing
enum class FileWriteMode : uint8_t
{
	WriteOnly, // write only access, replaces content
	ReadWrite, // read/write access (append)
};

//--

class IVirtualFileSystem;
typedef RefPtr<IVirtualFileSystem> VirtualFileSystemPtr;

//--

struct EmbeddedFile;
struct EmbeddedFile_Shader;
struct EmbeddedFile_Generic;
struct EmbeddedFile_Image;

//--

class IFileSystemInterface;

//--

/// global file system
extern BM_CORE_FILE_API IFileSystem& FileSystem();

//--

END_INFERNO_NAMESPACE()

#include "fileSystem.h"
