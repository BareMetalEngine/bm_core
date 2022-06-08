/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "fileSystem.h"
#include "fileSystemInterfacePhysical.h"

BEGIN_INFERNO_NAMESPACE()

//--

FileSystemPhysical::FileSystemPhysical()
{}

FileSystemPhysical::~FileSystemPhysical()
{}

bool FileSystemPhysical::queryFileInfo(StringView path, TimeStamp* outTimestamp /*= nullptr*/, uint64_t* outSize /*= nullptr*/) const
{
    return FileSystem().fileInfo(path, outTimestamp, outSize);
}

bool FileSystemPhysical::queryFileAbsolutePath(StringView path, StringBuf& outAbsolutePath) const
{
    outAbsolutePath = StringBuf(path);
    return true;
}

Buffer FileSystemPhysical::loadContentToBuffer(StringView path, bool makeDependency /*= true*/) const
{
    if (auto data = FileSystem().loadFileToBuffer(path))
        return data;

    return nullptr;
}


//--

IFileSystemInterface& IFileSystemInterface::GetPhysicalFileSystemInterface()
{
    static FileSystemPhysical thePhysicalFileSystem;
    return thePhysicalFileSystem;
}

//--

END_INFERNO_NAMESPACE()
