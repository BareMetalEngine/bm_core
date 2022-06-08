/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "fileSystem.h"
#include "fileSystemInterface.h"
#include "bm/core/containers/include/path.h"

BEGIN_INFERNO_NAMESPACE()

//--

IFileSystemInterface::~IFileSystemInterface()
{}

bool IFileSystemInterface::findRelativePath(StringView referencePath, StringView searchPath, StringBuf& outFoundAsssetPath, uint32_t maxScanDepth) const
{
    return ScanRelativePaths(referencePath, searchPath, maxScanDepth, outFoundAsssetPath, [this](StringView testPath)
        {
            return queryFileInfo(testPath);
        });
}

StringBuf IFileSystemInterface::loadContentToString(StringView path, bool makeDependency /*= true*/) const
{
    if (auto buffer = loadContentToBuffer(path, makeDependency))
        return StringBuf(buffer);
    else
        return StringBuf::EMPTY();
}

//--

END_INFERNO_NAMESPACE()
