/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

#include "fileAbsoluteRange.h"

BEGIN_INFERNO_NAMESPACE()

//----

// view of readable file, contains distinctive read position
class BM_CORE_FILE_API IFileView : public IReferencable
{
public:
    virtual ~IFileView();

    //--
    
	//! get debug information about source if this file (usually the path and offset information)
	INLINE const StringBuf& info() const { return m_info; }

	//! view flags
	INLINE FileFlags flags() const { return m_flags; }

	//! absolute range in the parent file this view represents
	INLINE const FileAbsoluteRange& range() const { return m_range; }

	//! get size of the file's view
	INLINE uint64_t size() const { return m_range.size(); }

    //--


	//! get offset (in view)
	virtual uint64_t offset() const = 0;

    //! change current offset in view
    //! NOTE: setting incorrect offset will result in readSync() returning 0 bytes read
    virtual void seek(uint64_t offset) = 0;

    //--

	//! Read data from file at specified offset, returns number of bytes read
	//! NOTE: interface is inherently slow, should be used only when reading small amount of data (headers) when memory-mapping is not worth it
	virtual uint64_t readSync(void* readBuffer, uint64_t size) = 0;

    //--

	// create a file view from memory buffer in a Buffer, will keep the buffer alive
	static FileViewPtr CreateFromBuffer(Buffer buffer, StringBuf info="");

	// create a file view of an arbitrary memory
	// TRICKY: memory obviously can't be freed so it's rather unsafe 
	static FileViewPtr CreateFromMemoryView(BufferView view, StringBuf info = "");

	//--

protected:
	IFileView(FileFlags flags, StringBuf info, FileAbsoluteRange range);

	FileFlags m_flags;
	StringBuf m_info;
	FileAbsoluteRange m_range;
};

//----

END_INFERNO_NAMESPACE()
