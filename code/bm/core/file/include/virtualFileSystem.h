/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

BEGIN_INFERNO_NAMESPACE()

//---

/// Dependency of a file in virtual file system
struct BM_CORE_FILE_API VirtualFileDependencyEntry
{
    StringBuf path; // source file path (usually relative to the folder from which VFS was built)
    TimeStamp timestamp; // timestamp of the source file at the moment the dependency was created, used to filter out fingerprint checks if not needed
    uint64_t fingerprint; // fingerprint (CRC64) of the file's content

    //---

    VirtualFileDependencyEntry();
};

/// Dependency information of a virtual file
struct BM_CORE_FILE_API VirtualFileDependencies
{
    Array<VirtualFileDependencyEntry> sourceFiles;

    //---

    VirtualFileDependencies();
};

/// Most VFS are compressed, allow to provide some information about it
struct BM_CORE_FILE_API VirtualFileCompressionInfo
{
    uint64_t uncomressedSize = 0;
    uint64_t compressedSize = 0;
	CompressionType compressionType = CompressionType::Uncompressed;

    //--

    VirtualFileCompressionInfo();
};

//---

/// Virtualized READ-ONLY file system (usually stored in a package)
/// NOTE: each file contains dependency information that could be quickly used to check if it needs rebuilding
class BM_CORE_FILE_API IVirtualFileSystem : public IReferencable, public IFileSystemInterface
{
public:
    virtual ~IVirtualFileSystem();

    //--

    /// open virtual file for reading
    virtual FileReaderPtr openForReading(StringView virtualPath, TimeStamp* outTimestamp = nullptr) = 0;

    /// open virtual file for asynchronous reading
    virtual AsyncFileHandlePtr openForAsyncReading(StringView virtualPath, TimeStamp* outTimestamp = nullptr) = 0;

    // open a read only memory mapped access to file
    virtual Buffer openMemoryMappedForReading(StringView virtualPath, TimeStamp* outTimestamp = nullptr) = 0;

    //--

    // query file dependencies
    virtual bool queryFileDependencies(StringView virtualPath, VirtualFileDependencies& outDependencies) const = 0;

    // query file compression information
    virtual bool queryFileCompressionInfo(StringView virtualPath, VirtualFileCompressionInfo& outCompressionInfo) const = 0;

    //--

    // create a VFS from a compiled package 
    static VirtualFileSystemPtr CreateFromPackage(StringView packageFileAbsolutePath, StringView packageFileAbsolutePath)
};

//---

END_INFERNO_NAMESPACE()
