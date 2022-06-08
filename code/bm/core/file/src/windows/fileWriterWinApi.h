/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

#include "fileWriter.h"

#ifdef PLATFORM_WINAPI

BEGIN_INFERNO_NAMESPACE_EX(windows)

//--

class FileWriter : public IFileWriter
{
public:
	FileWriter(FileFlags flags, StringBuf info, HANDLE hSyncHandle);
	virtual ~FileWriter();

	//--
	// IFileWriter

	virtual uint64_t size() const override final;
	virtual uint64_t pos() const override final;
	virtual void seek(uint64_t offset) override final;

	virtual uint64_t readSync(void* ptr, uint64_t size) override final;
	virtual uint64_t writeSync(const void* ptr, uint64_t size) override final;

	//--

private:
	HANDLE m_hSyncHandle;
};

//--

END_INFERNO_NAMESPACE_EX(windows)

#endif
