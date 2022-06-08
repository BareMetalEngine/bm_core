/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "fileSystem.h"
#include "fileFormat.h"

#ifdef PLATFORM_WINAPI
    #include "windows/fileSystemWinApi.h"
    typedef bm::windows::FileSystem FileSystemClass;
#elif defined(PLATFORM_POSIX)
    #include "posix/fileSystemPOSIX.h"
    typedef bm::posix::FileSystem FileSystemClass;
#else
    #error "Please implement file system"
#endif

BEGIN_INFERNO_NAMESPACE()

//--

void FileFormat::print(IFormatStream& f) const
{
    if (m_ext.empty())
    {
        f.appendf("none");
    }
    else
    {
        if (m_desc.empty())
            f.appendf("*.{}", m_ext);
        else
            f.appendf("*.{} [{]]", m_ext, m_desc);
    }
}

//--

IFileSystem::IFileSystem()
{}

IFileSystem::~IFileSystem()
{}

//--

void IFileSystem::collectFiles(StringView absoluteFilePath, StringView searchPattern, Array<StringBuf>& outAbsoluteFiles, bool recurse) const
{
    enumFiles(absoluteFilePath, searchPattern, [&outAbsoluteFiles](StringView fullPath, StringView name)
        {
            outAbsoluteFiles.emplaceBack(StringBuf(fullPath));
            return false;
        }, recurse);
}

void IFileSystem::collectSubDirs(StringView absoluteFilePath, Array<StringBuf>& outDirectoryNames) const
{
    enumSubDirs(absoluteFilePath, [&outDirectoryNames](StringView name)
        {
            outDirectoryNames.emplaceBack(name);
            return false;
        });
}

void IFileSystem::collectLocalFiles(StringView absoluteFilePath, StringView searchPattern, Array<StringBuf>& outFileNames) const
{
    enumLocalFiles(absoluteFilePath, searchPattern, [&outFileNames](StringView name)
        {
            outFileNames.emplaceBack(name);
            return false;
        });
}

//--

bool IFileSystem::loadFileToString(StringView absoluteFilePath, StringBuf& outString, TimeStamp* outTimestamp)
{
    // TODO: use only one allocation

    // load content to memory buffer
    Buffer buffer;
    if (!loadFileToBuffer(absoluteFilePath, StringBuf::StringPool(), buffer, outTimestamp))
        return false;

    // create string
    outString = StringBuf(buffer.view());
    return true;
}

StringBuf IFileSystem::loadFileToString(StringView absoluteFilePath, TimeStamp* outTimestamp)
{
    StringBuf txt;
    loadFileToString(absoluteFilePath, txt, outTimestamp);
    return txt;
}

//--

Buffer IFileSystem::loadFileToBuffer(StringView absoluteFilePath, TimeStamp* outTimestamp /* = nullptr */) const
{
    Buffer buffer;
    loadFileToBuffer(absoluteFilePath, MainPool(), buffer, outTimestamp, FileReadMode::MemoryMapped);
    return buffer;
}

//----

// encoding of the file saved
enum class StringEncoding : uint8_t
{
    Ansi, // all chars > 255 are saved as ?
    UTF8, // native, no data conversion
    UTF16, // expands to 16-bits, larger values are written as ?
};

bool IFileSystem::saveFileFromString(StringView absoluteFilePath, StringView str, StringEncoding encoding, const TimeStamp* timestampToAssign /*= nullptr*/)
{
    return saveFileFromBuffer(absoluteFilePath, str.bufferView(), timestampToAssign);
}

//--

bool IFileSystem::calculateFileCRC64(StringView absoluteFilePath, uint64_t& outCRC64, TimeStamp* outTimestamp /*= nullptr*/, uint64_t* outSize /*= nullptr*/) const
{
    Buffer fileContent;
    if (!loadFileToBuffer(absoluteFilePath, MainPool(), fileContent, outTimestamp, FileReadMode::MemoryMapped))
        return false;

    if (outSize)
        *outSize = fileContent.size();

    outCRC64 = CRC64().append(fileContent.data(), fileContent.size()).crc();
    return true;
}

//--

static IFileSystem* CreateFileSystem()
{
    return new FileSystemClass();
}

IFileSystem& FileSystem()
{
    static IFileSystem* GFileSystem = CreateFileSystem();
    return *GFileSystem;
}

//--

END_INFERNO_NAMESPACE()
