/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
*
* [#filter: io\system #]
***/

#pragma once

#include "bm/core/containers/include/stringStream.h"

BEGIN_INFERNO_NAMESPACE()

//--

// helper class to write large amount of text to a file
class BM_CORE_PARSER_API TextFileWriter : public IStringStream
{
public:
    TextFileWriter(IFileWriter* file);
    virtual ~TextFileWriter();

    // output file
    INLINE IFileWriter* file() const { return m_file; }

    // did we have any writing errors ?
    INLINE bool hasErrors() const { return m_hasErrors; }

    // flush current content
    virtual void flush() override;

protected:
    virtual void writeData(const void* data, uint32_t size) override final;

    FileWriterPtr m_file;
    bool m_hasErrors = false;
};

///--

END_INFERNO_NAMESPACE()
