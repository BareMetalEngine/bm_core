/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

#include "bm/core/system/include/guid.h"

BEGIN_INFERNO_NAMESPACE()

//--

/// a simple "manual" stream writer, usually for simple binary based network messaging
class BM_CORE_CONTAINERS_API SimpleStreamWriter : public MainPoolData<NoCopy>
{
public:
    SimpleStreamWriter(uint8_t* writePtr, uint32_t maxSize);
    SimpleStreamWriter(Buffer& buffer);
    ~SimpleStreamWriter();

    INLINE bool hasFailed() const { return m_failed; }
    INLINE uint32_t size() const { return m_pos - m_start; }

    bool writeCompressedInt(int32_t x); // packed value
    bool writeBuffer(const Buffer& buffer);
    bool writeBuffer(const void* data, uint32_t size); // write buffer of given size, size is stored
    bool writeString(StringView txt);
    bool writeString(StringID txt);
    bool writeRaw(const void* data, uint32_t size); // write raw data directly

    template< typename T >
    INLINE bool writeRaw(const T& data) { return writeRaw(&data, sizeof(data)); }

    INLINE bool writeUint8(uint8_t x) { return writeRaw(&x, sizeof(x)); }
    INLINE bool writeUint16(uint16_t x) { return writeRaw(&x, sizeof(x)); }
    INLINE bool writeUint32(uint32_t x) { return writeRaw(&x, sizeof(x)); }
    INLINE bool writeUint64(uint64_t x) { return writeRaw(&x, sizeof(x)); }
    INLINE bool writeInt8(int8_t x) { return writeRaw(&x, sizeof(x)); }
    INLINE bool writeInt16(int16_t x) { return writeRaw(&x, sizeof(x)); }
    INLINE bool writeInt32(int32_t x) { return writeRaw(&x, sizeof(x)); }
    INLINE bool writeInt64(int64_t x) { return writeRaw(&x, sizeof(x)); }
    INLINE bool writeFloat(float x) { return writeRaw(&x, sizeof(x)); }
    INLINE bool writeDouble(double x) { return writeRaw(&x, sizeof(x)); }
    INLINE bool writeBool(bool x) { return writeUint8(x ? 1 : 0); }
    INLINE bool writeGuid(const GUID& x) { return writeRaw(&x, sizeof(x)); }

    bool writeStringArray(const Array<StringID>& arr);
    bool writeStringArray(const Array<StringBuf>& arr);

    template< typename T >
    INLINE bool writeArray(const Array<T>& arr)
    {
        if (!writeCompressedInt(arr.size()))
            return false;
        return writeRaw(arr.data(), arr.dataSize());
    }
    
private:
    bool ensureSize(uint32_t size);

    uint8_t* m_start = nullptr;
    uint8_t* m_pos = nullptr;
    uint8_t* m_end = nullptr;

    bool m_failed = false;
};

//--

END_INFERNO_NAMESPACE()
