/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

#include "fileReader.h"
#include "bm/core/containers/include/inplaceArray.h"

#ifdef PLATFORM_WINAPI

BEGIN_INFERNO_NAMESPACE_EX(windows)

//--

class AsyncReadDispatcher;

///--

// WinAPI based file handle for sync READ operations
class FileReader : public IFileReader
{
public:
    FileReader(FileFlags flags, StringBuf info, const wchar_t* openFilePath, HANDLE hAsyncFile, uint64_t size, AsyncReadDispatcher* dispatcher, HANDLE hMemoryMapped);
    virtual ~FileReader();

    // NOTE: handle is always opened in async mode
    INLINE HANDLE handle() const { return m_hAsyncHandle; }

	//----
    // IFileReader

	virtual void readAsync(FileAbsoluteRange range, void* ptr, TAsyncReadCallback callback) override;

    virtual Buffer loadToBuffer(IPoolUnmanaged& pool, FileAbsoluteRange range) override final;

	virtual FileViewPtr createView(FileAbsoluteRange range) override;
	virtual FileMappingPtr createMapping(FileAbsoluteRange range) override;

    //--
    
    void releaseSyncHandle(HANDLE handle);

protected:
    HANDLE m_hAsyncHandle = nullptr;
    HANDLE m_hMemoryMappedHandle = nullptr;
    AsyncReadDispatcher* m_asyncDispatcher = nullptr;

    //--

	SpinLock m_syncHandlesLock;
	InplaceArray<HANDLE, 2> m_syncHandles;

    std::wstring m_syncFilePath;

	bool aquireSyncHandle(HANDLE& outHandle);
	
};

//--

END_INFERNO_NAMESPACE_EX(windows)

#endif
