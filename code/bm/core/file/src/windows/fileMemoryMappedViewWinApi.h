/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

#include "fileMapping.h"

#ifdef PLATFORM_WINAPI

BEGIN_INFERNO_NAMESPACE_EX(windows)


///--

// WinAPI based file handle for sync READ operations
class FileMemoryMappedView : public IFileMapping
{
public:
    FileMemoryMappedView(StringBuf info, HANDLE hMemoryMapped, void* ptr, uint64_t size);
    virtual ~FileMemoryMappedView();
    
protected:
    HANDLE m_hMemoryMappedHandle = nullptr;
    void* m_rawPtr = nullptr;
};

//--

END_INFERNO_NAMESPACE_EX(windows)

#endif
