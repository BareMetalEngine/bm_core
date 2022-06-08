/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

#include "bm/core/system/include/guid.h"

BEGIN_INFERNO_NAMESPACE()

//--

/// a simple "manual" stream reader, usually for simple binary based network messaging
class BM_CORE_CONTAINERS_API SimpleStreamReader
{
public:
    SimpleStreamReader();
    SimpleStreamReader(std::nullptr_t);
    SimpleStreamReader(const void* data, uint32_t size);
    SimpleStreamReader(const Buffer& data);
    SimpleStreamReader(const Array<uint8_t>& data);
    SimpleStreamReader(const SimpleStreamReader& other);
    SimpleStreamReader& operator=(const SimpleStreamReader& other);
    ~SimpleStreamReader();

    INLINE bool hasFailed() const { return m_failed; }

    template< typename T >
    INLINE ArrayView<T> readBuffer()
    {
        uint32_t count = 0;
        void* start = nullptr;
        if (readBuffer_Safe(sizeof(T), start, count))
            return ArrayView<T>((const T*)start, count);

        return ArrayView<T>();
    }

    INLINE int32_t readCompressedInt()
    {
        int32_t ret = 0;
        readCompressedInt_Safe(ret);
        return ret;
    }

    INLINE StringView readString()
    {
        StringView ret;
        readString_Safe(ret);
        return ret;
    }

    template< typename T >
    INLINE bool readSafe(T& data)
    {
        if (!ensureStreamData(sizeof(T)))
            return false;

        data = *(const T*)m_pos;
        m_pos += sizeof(T);
        return true;
    }

    INLINE uint8_t readUint8() { uint8_t x = 0; readSafe(x); return x; }
    INLINE uint16_t readUint16() { uint16_t x = 0; readSafe(x); return x; }
    INLINE uint32_t readUint32() { uint32_t x = 0; readSafe(x); return x; }
    INLINE uint64_t readUint64() { uint64_t x = 0; readSafe(x); return x; }
    INLINE int8_t readInt8() { int8_t x = 0; readSafe(x); return x; }
    INLINE int16_t readInt16() { int16_t x = 0; readSafe(x); return x; }
    INLINE int32_t readInt32() { int32_t x = 0; readSafe(x); return x; }
    INLINE int64_t readInt64() { int64_t x = 0; readSafe(x); return x; }
    INLINE float readFloat() { float x = 0.0f; readSafe(x); return x; }
    INLINE double readDouble() { double x = 0.0; readSafe(x); return x; }
    INLINE GUID readGuid() { GUID x; readSafe(x); return x; }

    bool readCompressedInt_Safe(int& outValue);
    bool readString_Safe(StringView& outView);
    bool readBuffer_Safe(uint32_t elementSize, BufferView& outView);
    bool readBuffer_Safe(uint32_t elementSize, Buffer& outView);
    bool readBuffer_Safe(BufferView& outBuffer);
    bool readBuffer_Safe(Buffer& outBuffer);

    bool readStringArray_Safe(Array<StringID>& outArr);
    bool readStringArray_Safe(Array<StringBuf>& outArr);

    template< typename T >
    INLINE bool readArray_Safe(Array<T>& outArr)
    {
        int count = 0;
        if (!readCompressedInt_Safe(count))
            return false;

        if (count < 0)
            return failStream("Negative array count");

        if ((count * sizeof(T)) > MAX_STREAM_BUFFER_LENGTH)
            return failStream("Array size to big");

        const auto dataSize = sizeof(T) * count;
        if (m_pos + dataSize > m_end)
            return failStream(TempString("Not enough data left in stream for a array of size {}", count));

        if (!outArr.resize(count))
            return failStream("Out of memory");

        memcpy(outArr.data(), m_pos, dataSize);
        m_pos += dataSize;
        return true;
    }

    INLINE bool readString_Safe(StringBuf& outBuf)
    {
        StringView txt;
        if (!readString_Safe(txt))
            return false;

        outBuf = StringBuf(txt);
        return true;
    }

    INLINE bool readString_Safe(StringID& outBuf)
    {
        StringView txt;
        if (!readString_Safe(txt))
            return false;

        outBuf = StringID(txt);
        return true;
    }

private:
    const uint8_t* m_start = nullptr;
    const uint8_t* m_pos = nullptr;
    const uint8_t* m_end = nullptr;
    bool m_failed = false;

    bool failStream(StringView message);

    INLINE bool ensureStreamData(uint32_t size)
    {
        if (m_failed)
            return false;
        if (m_pos + size > m_end)
            return failStream("Reading past stream end");
        return true;
    }
};

//--

END_INFERNO_NAMESPACE()
