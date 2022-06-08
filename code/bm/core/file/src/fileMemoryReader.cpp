/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "fileMemoryReader.h"
#include "fileMemoryView.h"
#include "fileMapping.h"

BEGIN_INFERNO_NAMESPACE()

//--

FileMemoryReader::FileMemoryReader(Buffer buffer, BufferView view, FileFlags flags, StringBuf info)
    : IFileReader(flags | FileFlagBit::MemoryBacked | FileFlagBit::Buffered, info, view.size())
	, m_view(view)
    , m_buffer(buffer)
{
}

FileMemoryReader::~FileMemoryReader()
{}

void FileMemoryReader::readAsync(FileAbsoluteRange readRange, void* ptr, TAsyncReadCallback callback)
{
    const auto readSize = fullRange().calculateReadSize(readRange);
    memcpy(ptr, m_view.data() + readRange.absoluteStart(), readSize);
    callback(readSize);
}

bool FileMemoryReader::readAsync(TaskContext& tc, FileAbsoluteRange readRange, void* ptr, uint32_t& outNumRead)
{
	const auto readSize = fullRange().calculateReadSize(readRange);
	memcpy(ptr, m_view.data() + readRange.absoluteStart(), readSize);
    outNumRead = readSize;
    return true;
}

Buffer FileMemoryReader::loadToBuffer(IPoolUnmanaged& pool, FileAbsoluteRange range)
{
    DEBUG_CHECK_RETURN_EX_V(fullRange().contains(range), "Invalid file range", nullptr);
    return m_buffer.createSubBuffer(range.absoluteStart(), range.size());
}

class FileMappingFromBuffer : public IFileMapping
{
public:
    FileMappingFromBuffer(StringBuf info, Buffer buffer, uint64_t offset, uint64_t size)
        : IFileMapping(info, buffer.data() + offset, size)
        , m_buffer(buffer)
    {}

private:
    Buffer m_buffer;
};

FileViewPtr FileMemoryReader::createView(FileAbsoluteRange range)
{
    VALIDATION_RETURN_V(range, nullptr); // empty range yields to no view
    DEBUG_CHECK_RETURN_EX_V(fullRange().contains(range), TempString("Requested range {} does not lie within the file {}", range, fullRange()), nullptr);

    const auto subView = m_view.subView(range.absoluteStart(), range.size());
    return RefNew<FileMemoryView>(flags(), m_info, range, m_buffer, subView);
}

FileMappingPtr FileMemoryReader::createMapping(FileAbsoluteRange range)
{
    DEBUG_CHECK_RETURN_EX_V(fullRange().contains(range), "Invalid mapping range", nullptr);

    const auto view = m_buffer.view().subView(range.absoluteStart(), range.absoluteEnd());
    return RefNew<FileMappingFromBuffer>(info(), m_buffer, range.absoluteStart(), range.size());
}

/*bool FileMemoryReader::createMappingAsync(FileAbsoluteRange range, TAsyncMappingCallback callback)
{
    DEBUG_CHECK_RETURN_EX_V(fullRange().contains(range), "Invalid mapping range", false);

    auto mapping = createMapping(range);
    DEBUG_CHECK_RETURN_EX_V(mapping, "Mapping failed", false);

    callback(mapping);
    return true;
}*/

//--

FileReaderPtr IFileReader::CreateFromMemoryView(BufferView view, StringBuf info)
{
    return RefNew<FileMemoryReader>(nullptr, view, FileFlags(), info);
}

FileReaderPtr IFileReader::CreateFromBuffer(Buffer buffer, StringBuf info)
{
	return RefNew<FileMemoryReader>(buffer, buffer.view(), FileFlags(), info);
}

//--


END_INFERNO_NAMESPACE()
