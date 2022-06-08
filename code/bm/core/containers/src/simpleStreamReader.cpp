/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "simpleStreamReader.h"

BEGIN_INFERNO_NAMESPACE()

//--

SimpleStreamReader::SimpleStreamReader() = default;
SimpleStreamReader::SimpleStreamReader(const SimpleStreamReader& other) = default;
SimpleStreamReader& SimpleStreamReader::operator=(const SimpleStreamReader & other) = default;

SimpleStreamReader::SimpleStreamReader(std::nullptr_t)
{}

SimpleStreamReader::SimpleStreamReader(const void* data, uint32_t size)
{
    m_start = (const uint8_t*)data;
    m_pos = m_start;
    m_end = m_start + size;
}

SimpleStreamReader::SimpleStreamReader(const Buffer& data)
{
    m_start = data.data();
    m_pos = m_start;
    m_end = m_start + data.size();
}

SimpleStreamReader::SimpleStreamReader(const Array<uint8_t>& data)
{
    m_start = data.typedData();
    m_pos = m_start;
    m_end = m_start + data.dataSize();
}

SimpleStreamReader::~SimpleStreamReader()
{
}

bool SimpleStreamReader::failStream(StringView message)
{
    if (!m_failed)
    {
        const auto offset = m_pos - m_start;
        const auto size = m_end - m_start;
        TRACE_ERROR("Stream reading failed at offset {}/{}: {}", offset, size, message);
        m_failed = true;
    }

    return false;
}

#define ENSURE_SPACE_V(size, ret) if (!ensureStreamData(size)) return ret;

bool SimpleStreamReader::readCompressedInt_Safe(int& outValue)
{
    const auto* start = m_pos;

    int32_t ret = 0;

    ENSURE_SPACE_V(1, 0);
    const auto byte = *m_pos++;
    ret = byte & 0x3F;

    if (byte & 0x80)
    {
        ENSURE_SPACE_V(1, 0);
        const auto byte = *m_pos++;
        ret |= (byte & 0x7F) << 6;

        if (byte & 0x80)
        {
            ENSURE_SPACE_V(1, 0);
            const auto byte = *m_pos++;
            ret |= (byte & 0x7F) << 13;

            if (byte & 0x80)
            {
                ENSURE_SPACE_V(1, 0);
                const auto byte = *m_pos++;
                ret |= (byte & 0x7F) << 20;

                ENSURE_SPACE_V(1, 0);
                if (byte & 0x80)
                {
                    const auto byte = *m_pos++;
                    ret |= (byte & 0x7F) << 27;
                }
            }
        }
    }

    if (start[0] & 0x40)
        ret = -ret;

    outValue = ret;
    return true;
}

bool SimpleStreamReader::readBuffer_Safe(uint32_t elementSize, BufferView& outView)
{
    int length = 0;
    if (!readCompressedInt_Safe(length))
        return false;

    if (length < 0)
        return failStream("Negative buffer count");

    if (length > MAX_STREAM_BUFFER_LENGTH)
        return failStream("Buffer size to big");

    if (m_pos + length > m_end)
        return failStream(TempString("Not enough data left in stream for a buffer of size {}", length));

    if ((length % elementSize) != 0)
        return failStream(TempString("Buffer length {} does not match element size {}", length, elementSize));

    outView = BufferView(m_pos, length / elementSize);

    m_pos += length;
    return true;
}

bool SimpleStreamReader::readBuffer_Safe(uint32_t elementSize, Buffer& outBuffer)
{
    BufferView view;
    if (!readBuffer_Safe(elementSize, view))
        return false;

    outBuffer = Buffer::CreateFromCopy(MainPool(), view);
	return true;
}

bool SimpleStreamReader::readBuffer_Safe(BufferView& outBuffer)
{
    return readBuffer_Safe(1, outBuffer);
}

bool SimpleStreamReader::readBuffer_Safe(Buffer& outBuffer)
{
    return readBuffer_Safe(1, outBuffer);
}

bool SimpleStreamReader::readString_Safe(StringView& outView)
{
    int length = 0;
    if (!readCompressedInt_Safe(length))
        return false;

    if (length < 0)
        return failStream("Negative string length"); // TODO: use this for something ?

    if (length > MAX_STREAM_STRING_LENGTH)
        return failStream("String size to big");

    if (m_pos + length > m_end)
        return failStream(TempString("Not enough data left in stream for a text of size {}", length));

    outView = StringView((const char*)m_pos, length);

    m_pos += length;
    return true;
}

bool SimpleStreamReader::readStringArray_Safe(Array<StringID>& outArr)
{
    int count = 0;
    if (!readCompressedInt_Safe(count))
        return false;

    if (count < 0)
        return failStream("Negative array size");

    if (count > MAX_STREAM_ARRAY_SIZE)
        return failStream("Array to big");

    if (!outArr.resize(count))
        return failStream("Out of memory");

    auto* write = outArr.typedData();
    auto* writeEnd = write + count;

    while (write < writeEnd)
    {
        StringView txt;
        if (!readString_Safe(txt))
            return false;

        *write++ = StringID(txt);
    }

    return true;
}

bool SimpleStreamReader::readStringArray_Safe(Array<StringBuf>& outArr)
{
    int count = 0;
    if (!readCompressedInt_Safe(count))
        return false;

    if (count < 0)
        return failStream("Negative array size");

    if (count > MAX_STREAM_ARRAY_SIZE)
        return failStream("Array to big");

    if (!outArr.resize(count))
        return failStream("Out of memory");

    auto* write = outArr.typedData();
    auto* writeEnd = write + count;

    while (write < writeEnd)
    {
        StringView txt;
        if (!readString_Safe(txt))
            return false;

        *write++ = StringBuf(txt);
    }

    return true;
}

//--

END_INFERNO_NAMESPACE()
