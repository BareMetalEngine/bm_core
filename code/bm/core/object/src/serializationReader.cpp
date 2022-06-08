/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "rttiType.h"
#include "rttiProperty.h"
#include "serializationReader.h"
#include "asyncBuffer.h"
#include "serializationErrorReporter.h"
#include "serializationBufferFactory.h"

BEGIN_INFERNO_NAMESPACE()

//--

SerializationResolvedReferences::SerializationResolvedReferences()
{}

SerializationResolvedReferences::~SerializationResolvedReferences()
{}

//--

SerializationSkipBlock::SerializationSkipBlock(SerializationReader& reader)
    : m_reader(&reader)
{
    auto offset = reader.readCompressedNumber();
    m_skipOffset = offset;
    m_skipPosition = reader.m_cur + offset;
    ASSERT(m_skipPosition <= reader.m_end);
}

SerializationSkipBlock::~SerializationSkipBlock()
{
    ASSERT(m_reader->m_cur <= m_skipPosition);
    m_reader->m_cur = m_skipPosition;
}

//--

SerializationReader::SerializationReader(IPoolUnmanaged& pool, const SerializationResolvedReferences& refs, BufferView data, uint32_t version, StringBuf context, ISerializationErrorReporter& errors)
    : m_version(version)
    , m_refs(refs)
    , m_pool(pool)
    , m_err(errors)
    , m_context(context)
{
    m_base = (const uint8_t*)data.data();
    m_cur = m_base;
    m_end = m_base + data.size();
}

SerializationReader::~SerializationReader()
{}

Buffer SerializationReader::readBufferData(uint64_t size, bool makeCopy)
{
    ASSERT_EX(m_cur + size <= m_end, "Read past buffer end");

    const auto* start = m_cur;
    m_cur += size;

    if (makeCopy)
        return Buffer::CreateFromCopy(MainPool(), BufferView(start, size), 16);
    else
        return Buffer::CreateExternal(BufferView(start, size), [](void*) {});
}

static const uint8_t FLAG_INLINED_BUFFER = 1;
static const uint8_t FLAG_ASYNC_BUFFER = 2;

AsyncFileBufferLoaderPtr SerializationReader::readAsyncBuffer()
{
    uint8_t type = 0;
    readTypedData(type);

    if (type == FLAG_INLINED_BUFFER)
    {
        const auto size = readCompressedNumber();
		const auto data = readBufferData(size, true);
        const auto crc = CRC64().append(data.data(), data.size()).crc();

		DEBUG_CHECK_RETURN_EX_V(data, "Failed to read buffer data", nullptr); // NOTE: empty buffers are saved as "None" so this should not happen
        return IAsyncFileBufferLoader::CreateResidentBufferFromUncompressedData(data, CompressionType::Uncompressed, crc);
    }

    else if (type == FLAG_ASYNC_BUFFER)
    {
        uint64_t crc = 0;
        readTypedData(crc);

        AsyncFileBufferLoaderPtr ret;
        DEBUG_CHECK_RETURN_EX_V(m_refs.buffers.find(crc, ret), "Async buffer not resolved", nullptr);
		DEBUG_CHECK_RETURN_EX_V(ret, "Failed to retrieve async buffer", nullptr);

		return ret;
    }

    return nullptr;
}

Buffer SerializationReader::readInlinedBuffer()
{
	uint8_t type = 0;
	readTypedData(type);

	if (type == FLAG_INLINED_BUFFER)
	{
		const auto size = readCompressedNumber();
		const auto data = readBufferData(size, true);
		return data;
	}

	else if (type == FLAG_ASYNC_BUFFER)
	{
		uint64_t crc = 0;
		readTypedData(crc);

		AsyncFileBufferLoaderPtr ret;
		DEBUG_CHECK_RETURN_EX_V(m_refs.buffers.find(crc, ret), "Async buffer not resolved", nullptr);
		DEBUG_CHECK_RETURN_EX_V(ret, "Failed to retrieve async buffer", nullptr);

        auto data = ret->load(NoTask(), m_pool, DEFAULT_BUFFER_ALIGNMENT);
		DEBUG_CHECK_RETURN_EX_V(data, "Failed to load async content", nullptr);

		return data;		
	}

	return nullptr;
}

//--

END_INFERNO_NAMESPACE()
