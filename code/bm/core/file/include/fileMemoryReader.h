/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

#include "fileReader.h"

BEGIN_INFERNO_NAMESPACE()

//--

// a simple file handle that's fully baked by memory
class BM_CORE_FILE_API FileMemoryReader : public IFileReader
{
public:
    FileMemoryReader(Buffer buffer, BufferView view, FileFlags flags, StringBuf info = "");
    virtual ~FileMemoryReader();

    //----
    /// IFileHandler

    virtual void readAsync(FileAbsoluteRange range, void* ptr, TAsyncReadCallback callback) override final;
    virtual bool readAsync(TaskContext& tc, FileAbsoluteRange range, void* ptr, uint32_t& outNumRead) override final;

    virtual Buffer loadToBuffer(IPoolUnmanaged& pool, FileAbsoluteRange range) override final;

	virtual FileMappingPtr createMapping(FileAbsoluteRange range) override final;
	virtual FileViewPtr createView(FileAbsoluteRange range) override final;

    //---

protected:
	Buffer m_buffer;
    BufferView m_view;
};

//--

END_INFERNO_NAMESPACE()
