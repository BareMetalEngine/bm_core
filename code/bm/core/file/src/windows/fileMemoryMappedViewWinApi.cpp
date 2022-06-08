/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "private.h"

#include "fileMemoryMappedViewWinApi.h"

#ifdef PLATFORM_WINAPI

BEGIN_INFERNO_NAMESPACE_EX(windows)

//--

FileMemoryMappedView::FileMemoryMappedView(StringBuf info, HANDLE hMemoryMapped, void* ptr, uint64_t size)
	: IFileMapping(info, ptr, size)
	, m_hMemoryMappedHandle(hMemoryMapped)
	, m_rawPtr(ptr)
{}

FileMemoryMappedView::~FileMemoryMappedView()
{
	UnmapViewOfFile(m_rawPtr);
}

//--

END_INFERNO_NAMESPACE_EX(windows)

#endif
