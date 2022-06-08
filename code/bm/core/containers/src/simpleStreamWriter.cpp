/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "simpleStreamWriter.h"

BEGIN_INFERNO_NAMESPACE()

//--

SimpleStreamWriter::SimpleStreamWriter(uint8_t* writePtr, uint32_t maxSize)
    : m_start(writePtr)
    , m_pos(writePtr)
    , m_end(writePtr + maxSize)
{}

SimpleStreamWriter::SimpleStreamWriter(Buffer& buffer)
{
    m_start = buffer.data();
    m_pos = buffer.data();
    m_end = buffer.data() + buffer.size();
}

SimpleStreamWriter::~SimpleStreamWriter()
{
}

bool SimpleStreamWriter::ensureSize(uint32_t size)
{
    if (m_pos + size <= m_end)
        return true;

    if (!m_failed)
    {
        const auto offset = m_pos - m_start;
        TRACE_ERROR("Stream writing failed at offset {}: not enough space to write {} bytes", offset, size);
        m_failed = true;
    }

    return false;
}

bool SimpleStreamWriter::writeRaw(const void* data, uint32_t size)
{
    VALIDATION_RETURN_V(ensureSize(size), false);

    if (size)
    {
        memcpy(m_pos, data, size);
        m_pos += size;
    }

    return true;
}

bool SimpleStreamWriter::writeBuffer(const Buffer& buffer)
{
    VALIDATION_RETURN_V(writeCompressedInt(buffer.size()), false);
    VALIDATION_RETURN_V(writeRaw(buffer.data(), buffer.size()), false);
    return true;
}

bool SimpleStreamWriter::writeBuffer(const void* data, uint32_t size)
{
    VALIDATION_RETURN_V(writeCompressedInt(size), false);
    VALIDATION_RETURN_V(writeRaw(data, size), false);
    return true;
}

bool SimpleStreamWriter::writeCompressedInt(int32_t x)
{
    uint8_t signMask = 0;
    if (x < 0)
    {
        signMask = 0x40;
        x = -x;
    }

    uint32_t length = 0;
    uint8_t bytes[10];

    bytes[length] = (x & 0x3F) | signMask;
    x >>= 6;
    length += 1;

    while (x)
    {
        bytes[length-1] |= 0x80;
        bytes[length] = x & 0x7F;
        length += 1;
        x >>= 7;
    }

    return writeRaw(bytes, length);
}

bool SimpleStreamWriter::writeString(StringView txt)
{
    VALIDATION_RETURN_V(writeBuffer(txt.data(), txt.length()), false);
    return true;
}

bool SimpleStreamWriter::writeString(StringID txt)
{
    return writeString(txt.view());
}

bool SimpleStreamWriter::writeStringArray(const Array<StringID>& arr)
{
    VALIDATION_RETURN_V(writeCompressedInt(arr.size()), false);
    for (const auto& str : arr)
    {
        VALIDATION_RETURN_V(writeString(str), false);
    }

    return true;
}

bool SimpleStreamWriter::writeStringArray(const Array<StringBuf>& arr)
{
    VALIDATION_RETURN_V(writeCompressedInt(arr.size()), false);

    for (const auto& str : arr)
    {
        VALIDATION_RETURN_V(writeString(str), false);
    }

    return true;
}

//--

END_INFERNO_NAMESPACE()
