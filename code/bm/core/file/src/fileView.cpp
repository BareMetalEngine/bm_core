/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "fileView.h"

BEGIN_INFERNO_NAMESPACE()

//--

IFileView::IFileView(FileFlags flags, StringBuf info, FileAbsoluteRange range)
	: m_flags(flags)
	, m_info(info)
	, m_range(range)
{}

IFileView::~IFileView()
{}

//--

END_INFERNO_NAMESPACE()
