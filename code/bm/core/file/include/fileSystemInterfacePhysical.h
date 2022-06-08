/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

#include "fileSystemInterface.h"

BEGIN_INFERNO_NAMESPACE()

//--

/// wraper for physical file system
class BM_CORE_FILE_API FileSystemPhysical : public IFileSystemInterface
{
public:
    FileSystemPhysical();
    virtual ~FileSystemPhysical();

    //--

    // IFileSystemAccessInterface
    virtual bool queryFileInfo(StringView path, TimeStamp* outTimestamp = nullptr, uint64_t* outSize = nullptr) const override;
    virtual bool queryFileAbsolutePath(StringView path, StringBuf& outAbsolutePath) const override;
    virtual Buffer loadContentToBuffer(StringView path, bool makeDependency = true) const override;

    //--
};

//--

END_INFERNO_NAMESPACE()
