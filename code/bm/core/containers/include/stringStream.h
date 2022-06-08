/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

#include "stringBuf.h"

BEGIN_INFERNO_NAMESPACE()

/// a helper class that writes text to an output stream (usually a file) and does not retain anything in the memory
class BM_CORE_CONTAINERS_API IStringStream : public IFormatStream
{
public:
    IStringStream(); // TODO: encoding!
    virtual ~IStringStream(); // flushes

    //--

    // flush not yet written content
    virtual void flush();

    //---

    IFormatStream& append(StringView view);
    IFormatStream& append(const StringBuf& str);
    IFormatStream& append(StringID str);

    //---

    // append number of chars to the stream
    virtual IFormatStream& append(const char* str, uint32_t len = INDEX_MAX) override final;

    // append wide-char stream
    virtual IFormatStream& append(const wchar_t* str, uint32_t len = INDEX_MAX) override final;

    //---

protected:
    // write block of data to the output file/stream
    virtual void writeData(const void* data, uint32_t size) = 0;

private:
    static const uint32_t INTERNAL_BUFFER_SIZE = 8192;

    char m_internalBuffer[INTERNAL_BUFFER_SIZE];
    uint32_t m_size = 0;
};

END_INFERNO_NAMESPACE()
