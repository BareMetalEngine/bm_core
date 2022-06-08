/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

BEGIN_INFERNO_NAMESPACE()

///---

class ISerializationErrorReporter;

//---

struct SerializationResourceInfo;

struct BM_CORE_OBJECT_API SerializationResolvedReferences : public MainPoolData<NoCopy>
{
    Array<StringID> stringIds;

    Array<Type> types;
    Array<StringID> typeNames;

    Array<SerializationPropertyInfo> properties;

    Array<ObjectPtr> objects;
    Array<SerializationResourceInfo> resources;

    HashMap<uint64_t, AsyncFileBufferLoaderPtr> buffers;

    //--

    SerializationResolvedReferences();
    ~SerializationResolvedReferences();
};

///---

class BM_CORE_OBJECT_API SerializationSkipBlock : public NoCopy
{
public:
    SerializationSkipBlock(SerializationReader& reader);
    ~SerializationSkipBlock(); // automatically skips the stream to the final position

private:
    SerializationReader* m_reader = nullptr;
    const uint8_t* m_skipPosition = nullptr;
    uint32_t m_skipOffset = 0; // as read, for debug
};

///---

struct BM_CORE_OBJECT_API SerializationPropertyInfo
{
    StringID name; // always resolved
    StringID typeName; // always resolved
    Type type; // resolved if type still exists
    const Property* prop = nullptr; // not resolved on missing properties
};

///---

struct BM_CORE_OBJECT_API SerializationResourceInfo
{
    ResourceID id;
    ResourceClass cls;
    ResourcePromisePtr promise; // if created
};

///---

class BM_CORE_OBJECT_API SerializationReader : public MainPoolData<NoCopy>
{
public:
    SerializationReader(IPoolUnmanaged& pool, const SerializationResolvedReferences& refs, BufferView data, uint32_t version, StringBuf context, ISerializationErrorReporter& errors);
    ~SerializationReader();

    //--

    // stream version
    INLINE uint32_t version() const { return m_version; }

    // context (usually file we are reading)
    INLINE const StringBuf& context() const { return m_context; }

    // internal memory pool, can be used to make allocations related to loading
    INLINE IPoolUnmanaged& pool() const { return m_pool; }

    // get error reporter for the stream
    INLINE ISerializationErrorReporter& errors() const { return m_err; }

    //--

    /// get current data pointer and skip bytes (direct memory access)
    INLINE const uint8_t* readData(uint32_t dataSize);

    /// copy data to another place
    INLINE void readData(void* data, uint64_t size);

    /// load typed data
    template< typename T >
    INLINE void readTypedData(T& data);

    ///--

    /// read StringID
    INLINE StringID readStringID();

    /// read type reference
    INLINE Type readType(StringID& outTypeName);

    /// read property reference
    INLINE bool readProperty(SerializationPropertyInfo& outInfo);

    /// read pointer to other object
    INLINE IObject* readPointer();

    /// read buffer for use in async buffer
    AsyncFileBufferLoaderPtr readAsyncBuffer();

    /// read an inlined buffer, memory is usually allocated from the serialization pool
    Buffer readInlinedBuffer();

	/// read resource reference
    ResourcePromisePtr readResource();

    /// read generate compressed number
	INLINE uint64_t readCompressedNumber();

    ///---

private:
    const uint8_t* m_cur = nullptr;
    const uint8_t* m_end = nullptr;
    const uint8_t* m_base = nullptr;

	IPoolUnmanaged& m_pool;

    uint32_t m_version = 0;
    ISerializationErrorReporter& m_err;

    const SerializationResolvedReferences& m_refs;

    StringBuf m_context;

    Buffer readBufferData(uint64_t size, bool makeCopy);

    friend class SerializationSkipBlock;
};

///---

INLINE const uint8_t* SerializationReader::readData(uint32_t size)
{
    ASSERT_EX(m_cur + size <= m_end, "Read past buffer end");
    auto* ptr = m_cur;
    m_cur += size;
    return ptr;
}

INLINE void SerializationReader::readData(void* data, uint64_t size)
{
    ASSERT_EX(m_cur + size <= m_end, "Read past buffer end");
    memcpy(data, m_cur, size);
    m_cur += size;
}

template< typename T >
INLINE void SerializationReader::readTypedData(T& data)
{
    ASSERT_EX(m_cur + sizeof(T) <= m_end, "Read past buffer end");
    if (alignof(T) < 16)
        data = *(const T*)m_cur;
    else
        memcpy(&data, m_cur, sizeof(T));
    m_cur += sizeof(T);
}
       
INLINE StringID SerializationReader::readStringID()
{
    const auto index = readCompressedNumber();
    return m_refs.stringIds[index];
}

INLINE Type SerializationReader::readType(StringID& outTypeName)
{
    const auto index = readCompressedNumber();
    outTypeName = m_refs.typeNames[index];
    return m_refs.types[index];
}

INLINE bool SerializationReader::readProperty(SerializationPropertyInfo& outInfo)
{
    const auto index = readCompressedNumber();
    if (!index)
        return false;

    outInfo = m_refs.properties[index];
    return true;
}

INLINE IObject* SerializationReader::readPointer()
{
    const auto index = readCompressedNumber();
    if (index == 0 || index > (int)m_refs.objects.size())
        return nullptr;

    return m_refs.objects[index - 1];
}

INLINE ResourcePromisePtr SerializationReader::readResource()
{
    const auto index = readCompressedNumber();
    if (index <= 0 && index > (int)m_refs.resources.size())
        return nullptr;
    
    return m_refs.resources[index - 1].promise;
}

INLINE uint64_t SerializationReader::readCompressedNumber()
{
    ASSERT_EX(m_cur < m_end, "Reading past the end of the stream");

    auto singleByte = *m_cur++;
    uint64_t ret = singleByte & 0x7F;
    uint32_t offset = 7;

    while (singleByte & 0x80)
    {
        ASSERT_EX(m_cur < m_end, "Reading past the end of the stream");
        singleByte = *m_cur++;
        ret |= ((uint64_t)(singleByte & 0x7F)) << offset;
        offset += 7;
    }

    return ret;
}

//---

END_INFERNO_NAMESPACE()
