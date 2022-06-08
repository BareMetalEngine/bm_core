/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "fileView.h"
#include "fileWriter.h"
#include "fileSystem.h"

BEGIN_INFERNO_NAMESPACE()

//--

IFileWriter::IFileWriter(FileFlags flags, StringBuf info)
    : m_info(info)
    , m_flags(flags)
{}

IFileWriter::~IFileWriter()
{}

//--

END_INFERNO_NAMESPACE()
