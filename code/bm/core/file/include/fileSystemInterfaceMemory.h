/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

#include "fileSystemInterface.h"

#include "bm/core/system/include/timestamp.h"
#include "bm/core/containers/include/hashMap.h"

BEGIN_INFERNO_NAMESPACE()

//--

/// abstract file system interface for manually mounted files in memory
class BM_CORE_FILE_API FileSystemMemory : public IFileSystemInterface
{
public:
    FileSystemMemory();
    virtual ~FileSystemMemory();

    //--
    
    /// mount custom file content from memory buffer
    void mountBuffer(StringView path, Buffer data, StringView absolutePath = "", TimeStamp timestamp = TimeStamp());

    /// mount custom file content from file on disk
    void mountPhysicalFile(StringView path, StringView absolutePath);

    /// mount embedded file
    //void mountEmbeddedFile(StringView path, StringView embeddedName);

    /// unmount file content
    void unmountFile(StringView path);

    //--

    // IFileSystemAccessInterface

    virtual bool queryFileInfo(StringView path, TimeStamp* outTimestamp = nullptr, uint64_t* outSize = nullptr) const override;
    virtual bool queryFileAbsolutePath(StringView path, StringBuf& outAbsolutePath) const override;
    virtual Buffer loadContentToBuffer(StringView path, bool makeDependency = true) const override;

    //--

private:
    struct FileInfo
    {
        Buffer content;
        TimeStamp timestamp;
        StringBuf absolutePath;
        StringBuf virtualPath;
    };

    HashMap<StringBuf, FileInfo> m_fileMap;
};

//--

END_INFERNO_NAMESPACE()
