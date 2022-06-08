/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

#include "fileAbsoluteRange.h"

BEGIN_INFERNO_NAMESPACE()

//----

// An abstract file with read access
// NOTE: by default only async reads are possible without creating a view
class BM_CORE_FILE_API IFileReader : public IReferencable
{
public:
    virtual ~IFileReader();

    //----

    //! get debug information about source if this file (usually the path)
    INLINE const StringBuf& info() const { return m_info; }

    //! file flags
    INLINE FileFlags flags() const { return m_flags; }

    //! get size of the file
    INLINE uint64_t size() const { return m_size; }

    //! get the full absolute file range
    INLINE FileAbsoluteRange fullRange() const { return FileAbsoluteRange(0, m_size); }

	//----

	//! Start async read of file's content, calls the callback when completed, does not block calling thread
	//! Returns true if read was successful and returns number of bytes read
    virtual void readAsync(FileAbsoluteRange range, void* ptr, TAsyncReadCallback callback) = 0;

	//! Start async read of file's content, calls the callback when completed, does not block calling thread
	//! Returns true if read was successful and returns number of bytes read
    INLINE void readAsync(uint64_t offset, uint32_t size, void* ptr, TAsyncReadCallback callback)
    {
        return readAsync(FileAbsoluteRange(offset, offset + size), ptr, callback);
    }

    //---

    //! Start async read of file's content, will yield current task until read is finished
    //! Returns true if read was successful and returns number of bytes read
    virtual bool readAsync(TaskContext& tc, FileAbsoluteRange range, void* ptr, uint32_t& outNumRead);

	//! Start async read of file's content, calls the callback when completed
	//! Blocks the calling task (yields) until the read arrives
	//! Returns true if read was successful and returns number of bytes read
    INLINE bool readAsync(TaskContext& tc, uint64_t offset, uint32_t size, void* ptr, uint32_t& outNumRead)
    {
        return readAsync(tc, FileAbsoluteRange(offset, offset + size), ptr, outNumRead);
    }

    //---
     
    //! Load part of file to buffer, may use memory mapping
    virtual Buffer loadToBuffer(IPoolUnmanaged& pool, FileAbsoluteRange range) = 0;

    //----

    //! Create view of part of the view, allows the classic seek/read interface
	virtual FileViewPtr createView(FileAbsoluteRange range) = 0;

	//--

	//! Map part of the file for direct memory access, for files without memory mapping it just allocates a block of memory and loads data there
    //! NOTE: it's should be considered a blocking call
	virtual FileMappingPtr createMapping(FileAbsoluteRange range) = 0;

	//----

    // create a file handle from memory buffer
    // NOTE: memory cannot be freed
    static FileReaderPtr CreateFromMemoryView(BufferView view, StringBuf info = "");

    // create a file handle from memory buffer in a Buffer, will keep the buffer alive
    static FileReaderPtr CreateFromBuffer(Buffer buffer, StringBuf info = "");
        
    //----

protected:
    IFileReader(FileFlags flags, StringBuf info, uint64_t size);

    uint64_t m_size;
	StringBuf m_info;
    FileFlags m_flags;
};

//--

END_INFERNO_NAMESPACE()
