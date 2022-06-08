/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

#include "fileView.h"

BEGIN_INFERNO_NAMESPACE()

//----

// Remory backed readable view
class BM_CORE_FILE_API FileMemoryView : public IFileView
{
public:
	FileMemoryView(FileFlags flags, StringBuf info, FileAbsoluteRange range, Buffer buffer, BufferView view);
	virtual ~FileMemoryView();

    //--
    
	/// IFileView
	virtual uint64_t offset() const override final { return m_offset; }
	virtual void seek(uint64_t offset) override final { m_offset = offset; }
	
	virtual uint64_t readSync(void* readBuffer, uint64_t size) override final;

	//--

private:
	uint64_t m_offset = 0;

	BufferView m_view;
	Buffer m_buffer; // owner of the memory
};

//----

END_INFERNO_NAMESPACE()
