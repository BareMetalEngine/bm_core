/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

#include "fileReader.h"
#include "fileView.h"

#ifdef PLATFORM_WINAPI

BEGIN_INFERNO_NAMESPACE_EX(windows)

//--

class FileReader;

// A disk-backed view of a file, unbuffered - all reads go straight through
class FileUnbufferedDiskView : public IFileView
{
public:
    FileUnbufferedDiskView(FileFlags flags, StringBuf info, FileAbsoluteRange range, HANDLE hSyncHandle, FileReader* owner);
    virtual ~FileUnbufferedDiskView();

	//----
    // IFileReader

	virtual uint64_t offset() const override;
	virtual void seek(uint64_t offset) override;
    virtual uint64_t readSync(void* readBuffer, uint64_t size) override;

    //--
    
protected:
    bool m_absoluteOffsetChanged = false;
    uint64_t m_absoluteOffset = 0;

    bool syncFileOffset();

    HANDLE m_hSyncHandle = nullptr;

    RefWeakPtr<FileReader> m_owner;
};

//--

END_INFERNO_NAMESPACE_EX(windows)

#endif
