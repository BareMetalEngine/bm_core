/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "textFileWriter.h"
#include "bm/core/file/include/fileWriter.h"

BEGIN_INFERNO_NAMESPACE()

//--

TextFileWriter::TextFileWriter(IFileWriter* file)
    : m_file(AddRef(file))
{}

TextFileWriter::~TextFileWriter()
{
    flush();
}

void TextFileWriter::flush()
{
    IStringStream::flush();

    //m_file->flush(); //TODO
}

void TextFileWriter::writeData(const void* data, uint32_t size)
{
    if (!m_hasErrors)
    {
        if (!m_file->writeSync(data, size))
            m_hasErrors = true;
    }
}

//--

END_INFERNO_NAMESPACE()
