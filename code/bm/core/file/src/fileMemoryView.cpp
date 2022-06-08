/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "fileMemoryView.h"

BEGIN_INFERNO_NAMESPACE()

//--

FileMemoryView::FileMemoryView(FileFlags flags, StringBuf info, FileAbsoluteRange range, Buffer buffer, BufferView view)
	: IFileView(flags | FileFlagBit::MemoryBacked, info, range)
	, m_buffer(buffer)
	, m_view(view)
{
}

FileMemoryView::~FileMemoryView()
{
}

//--

uint64_t FileMemoryView::readSync(void* readBuffer, uint64_t size)
{
	const auto copySize = m_view.evaluateCopiableSize(m_offset, size);

	memcpy(readBuffer, m_view.data() + m_offset, copySize);
	m_offset += copySize;

	return copySize;
}

//--

FileViewPtr IFileView::CreateFromBuffer(Buffer buffer, StringBuf info)
{
	FileFlags flags;
	flags |= FileFlagBit::MemoryBacked;

	return RefNew<FileMemoryView>(flags, info, FileAbsoluteRange(0, buffer.size()), buffer, buffer.view());
}

FileViewPtr IFileView::CreateFromMemoryView(BufferView view, StringBuf info)
{
	FileFlags flags;
	flags |= FileFlagBit::MemoryBacked;
	flags |= FileFlagBit::Detached;

	return RefNew<FileMemoryView>(flags, info, FileAbsoluteRange(0, view.size()), nullptr, view);
}

//--

END_INFERNO_NAMESPACE()
