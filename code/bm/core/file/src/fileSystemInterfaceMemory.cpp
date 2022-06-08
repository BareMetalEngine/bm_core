/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "fileSystem.h"
#include "fileSystemInterfaceMemory.h"
#include "embeddedFile.h"

BEGIN_INFERNO_NAMESPACE()

//--

FileSystemMemory::FileSystemMemory()
{}

FileSystemMemory::~FileSystemMemory()
{}

//--

void FileSystemMemory::mountBuffer(StringView path, Buffer data, StringView absolutePath /*= ""*/, TimeStamp timestamp /*= TimeStamp()*/)
{
    DEBUG_CHECK_RETURN_EX(path, "Empty mount path");

    auto pathStr = StringBuf(path);

    auto& entry = m_fileMap[pathStr];
    entry.content = data;
    entry.virtualPath = pathStr;

    if (absolutePath)
    {
        entry.absolutePath = StringBuf(absolutePath);

        if (timestamp.empty())
            FileSystem().fileInfo(absolutePath, &entry.timestamp);
    }

    if (!timestamp.empty())
        entry.timestamp = timestamp;
}

void FileSystemMemory::mountPhysicalFile(StringView path, StringView absolutePath)
{
    if (auto buffer = FileSystem().loadFileToBuffer(absolutePath))
        mountBuffer(path, buffer, absolutePath);
}

/*void FileSystemMemory::mountEmbeddedFile(StringView path, StringView embeddedName)
{
    if (const auto* embeddedFile = static_cast<const EmbeddedFile_Generic*>(EmbeddedFiles().findFile(embeddedName, EmbeddedFileType::Generic)))
    {
        const auto buffer = Buffer::CreateExternal(embeddedFile->size, (void*)embeddedFile->data, [](void*, uint64_t) {});

        const auto* source = embeddedFile->sources;
        mountBuffer(path, buffer, source->sourcePath, source->sourceTimestamp);
    }
}*/

void FileSystemMemory::unmountFile(StringView path)
{
    m_fileMap.remove(path);
}

//--

bool FileSystemMemory::queryFileInfo(StringView path, TimeStamp* outTimestamp /*= nullptr*/, uint64_t* outSize /*= nullptr*/) const
{
    if (const auto* entry = m_fileMap.find(path))
    {
        if (outTimestamp)
            *outTimestamp = entry->timestamp;
        if (outSize)
            *outSize = entry->content.size();
        return true;
    }

    return false;
}

bool FileSystemMemory::queryFileAbsolutePath(StringView path, StringBuf& outAbsolutePath) const
{
    if (const auto* entry = m_fileMap.find(path))
    {
        outAbsolutePath = entry->absolutePath;
        return true;
    }

    return false;
}

Buffer FileSystemMemory::loadContentToBuffer(StringView path, bool makeDependency /*= true*/) const
{
    if (const auto* entry = m_fileMap.find(path))
        return entry->content;

    return Buffer();
}

//--

END_INFERNO_NAMESPACE()
