/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "rttiType.h"
#include "rttiProperty.h"

#include "serializationStream.h"
#include "serializationBinarizer.h"

#include "bm/core/file/include/fileReader.h"
#include "bm/core/file/include/fileWriter.h"
#include "asyncBuffer.h"

BEGIN_INFERNO_NAMESPACE()

//--

SerializationMappedReferences::SerializationMappedReferences()
{
    mappedNames.reserve(64);
    mappedTypes.reserve(64);
    mappedPointers.reserve(64);
    mappedProperties.reserve(64);
    mappedResources.reserve(64);
}

//--

SerializationBinaryPacker::SerializationBinaryPacker(IFileWriter* outputFile)
    : m_outputFile(outputFile)
{}

SerializationBinaryPacker::~SerializationBinaryPacker()
{
    flush();
}

void SerializationBinaryPacker::flush()
{
    if (m_cacheBufferSize > 0)
    {
        m_crc.append(m_cacheBuffer, m_cacheBufferSize);
        m_outputFile->writeSync(m_cacheBuffer, m_cacheBufferSize);
        m_cacheBufferSize = 0;
    }
}

void SerializationBinaryPacker::writeToFile(const void* data, uint64_t size)
{
	m_pos += size;

    if (m_cacheBufferSize + size <= BUFFER_SIZE)
    {
        memcpy(m_cacheBuffer + m_cacheBufferSize, data, size);
        m_cacheBufferSize += size;
    }
    else
    {
        flush();

        if (size < (BUFFER_SIZE / 2))
        {
            memcpy(m_cacheBuffer + 0, data, size);
            m_cacheBufferSize = size;
        }
        else
        {
            m_crc.append(data, size);
            m_outputFile->writeSync(data, size);
        }
    }
}

//--

static void WriteIndex(SerializationBinaryPacker& writer, uint64_t index)
{
    uint8_t tempBuffer[16];
    auto size = CalcAdaptiveNumberSize4(index);
    WriteAdaptiveNumber4(tempBuffer, size, index);
    writer.writeToFile(tempBuffer, size);
}

static uint32_t ResolvePropertyIndex(const SerializationMappedReferences& mappedReferences, const Property* prop)
{
    if (!prop)
        return 0;

    uint32_t index = 0;
    if (!mappedReferences.mappedProperties.find(prop, index))
    {
        ASSERT(!"Property not mapped");
    }

    return index;
}

static uint32_t ResolveTypeIndex(const SerializationMappedReferences& mappedReferences, Type type)
{
    if (!type)
        return 0;

    uint32_t index = 0;
    if (!mappedReferences.mappedTypes.find(type, index))
    {
        ASSERT(!"Property not mapped");
    }

    return index;
}

static uint32_t ResolveNameIndex(const SerializationMappedReferences& mappedReferences, StringID name)
{
    if  (!name)
        return 0;

    uint32_t index = 0;
    if (!mappedReferences.mappedNames.find(name, index))
    {
        ASSERT(!"Name not mapped");
    }

    return index;
}

static uint32_t ResolveObjectIndex(const SerializationMappedReferences& mappedReferences, const IObject* obj)
{
    if (!obj)
        return 0;

    uint32_t index = 0;
    if (!mappedReferences.mappedPointers.find(obj, index))
    {
        ASSERT(!"Object not mapped");
    }

    //ASSERT_EX(index != 0, "Valid opcode cannot be mapped to zero");
    return index;
}

static uint32_t ResolveResourceIndex(const SerializationMappedReferences& mappedReferences, const SerializationResourceKey& key)
{
    if (!key.id)
        return 0;

    uint32_t index = 0;
    if (!mappedReferences.mappedResources.find(key, index))
    {
        ASSERT(!"Resource not mapped");
    }

    ASSERT_EX(index != 0, "Valid resource cannot be mapped to zero");
    return index;
}


void EstimateDiskPositions(const SerializationStream& stream, const SerializationMappedReferences& mappedReferences, SerializationBinaryPacker& writer, bool finalized)
{
    uint32_t diskPosition = 0;
	uint32_t bytesWastedOnSkipOffsetEncoding = 0;
	for (auto it = stream.opcodes(); it; ++it)
	{
		if (it->op == SerializationOpcode::NextPage)
			continue; // technical opcode, ignore

		switch (it->op)
		{
		case SerializationOpcode::SkipHeader:
        {
            auto* op = (SerializationOpSkipHeader*)*it;

            if (finalized)
            {
                ASSERT(op->skipSizeValueSize);
                diskPosition += op->skipSizeValueSize;
				op->skipOffset = diskPosition;
            }
            else
            {
                ASSERT(op->skipSizeValueSize == 0);
                op->skipOffset = diskPosition;
				// diskPosition += NOTHING WRITTEN HERE because we don't even have size estimate
            }
            break;
        }

		case SerializationOpcode::SkipLabel:
        {
            auto* op = (SerializationOpSkipLabel*)*it;
            ASSERT(op->header);

            if (finalized)
            {
                const auto finalSkipOffset = diskPosition - op->header->skipOffset;
                const auto finalSkipValueSize = CalcAdaptiveNumberSize4(finalSkipOffset);
                ASSERT(finalSkipValueSize <= op->header->skipSizeValueSize);
                bytesWastedOnSkipOffsetEncoding += (op->header->skipSizeValueSize - finalSkipValueSize);
                op->header->skipOffset = finalSkipOffset;
            }
            else
            {
				ASSERT(op->header->skipSizeValueSize == 0);
                const auto esitmatedSkipOffset = diskPosition - op->header->skipOffset;
                op->header->skipOffset = 0; // do not store in this pass
                op->header->skipSizeValueSize = CalcAdaptiveNumberSize4(esitmatedSkipOffset); // upper bound on size needed
                diskPosition += op->header->skipSizeValueSize; // allocate data here not in the header
            }
            break;
        }

		case SerializationOpcode::Property:
		{
			auto* op = (SerializationOpProperty*)(*it);
			if (!finalized)
				op->shared.index = ResolvePropertyIndex(mappedReferences, op->shared.prop);
			diskPosition += CalcAdaptiveNumberSize4(op->shared.index);
			break;
		}

		case SerializationOpcode::DataBlock1:
		{
			const auto* op = (const SerializationOpDataBlock1*)(*it);
            diskPosition += op->size;
			break;
		}

		case SerializationOpcode::DataBlock2:
		{
			const auto* op = (const SerializationOpDataBlock2*)(*it);
            diskPosition += op->size;
			break;
		}

		case SerializationOpcode::DataBlock4:
		{
			const auto* op = (const SerializationOpDataBlock4*)(*it);
            diskPosition += op->size;
			break;
		}

		case SerializationOpcode::DataTypeRef:
		{
			auto* op = (SerializationOpDataTypeRef*)(*it);
			if (!finalized)
				op->shared.index = ResolveTypeIndex(mappedReferences, op->shared.type);
            diskPosition += CalcAdaptiveNumberSize4(op->shared.index);
			break;
		}

		case SerializationOpcode::DataName:
		{
			auto* op = (SerializationOpDataName*)(*it);
			if (!finalized)
				op->shared.index = ResolveNameIndex(mappedReferences, op->shared.name);
            diskPosition += CalcAdaptiveNumberSize4(op->shared.index);
			break;
		}

		case SerializationOpcode::DataObjectPointer:
		{
			auto* op = (SerializationOpDataObjectPointer*)(*it);
			if (!finalized)
				op->shared.index = ResolveObjectIndex(mappedReferences, op->shared.object);
            diskPosition += CalcAdaptiveNumberSize4(op->shared.index);
			break;
		}

		case SerializationOpcode::DataAdaptiveNumber:
		{
			const auto* op = (const SerializationOpDataAdaptiveNumber*)(*it);
            diskPosition += CalcAdaptiveNumberSize4(op->value);
			break;
		}

		case SerializationOpcode::DataResourceRef:
		{
			auto* op = (SerializationOpDataResourceRef*)(*it);
			if (!finalized)
				op->shared.index = ResolveResourceIndex(mappedReferences, op->shared.key);
            diskPosition += CalcAdaptiveNumberSize4(op->shared.index);
			break;
		}

		case SerializationOpcode::DataInlineBuffer:
		{
			const auto* op = (const SerializationOpDataInlineBuffer*)(*it);

			diskPosition += 1; // type

			if (op->data)
			{
				diskPosition += CalcAdaptiveNumberSize4(op->data.size());
				diskPosition += op->data.size();
			}
			else if (op->asyncLoader)
			{
				diskPosition += sizeof(uint64_t);
			}

			break;
		}

		default:
			ASSERT(!"Unknown opcode");
		}
	}

    if (finalized && bytesWastedOnSkipOffsetEncoding)
        TRACE_WARNING("Wasted {} bytes on skip offset encoding", bytesWastedOnSkipOffsetEncoding);
}

static const uint8_t FLAG_INLINED_BUFFER = 1;
static const uint8_t FLAG_ASYNC_BUFFER = 2;

void WriteFinalizedOpcodes(const SerializationStream& stream, const SerializationMappedReferences& mappedReferences, SerializationBinaryPacker& writer)
{
#ifdef BUILD_DEBUG
	HashMap<const SerializationOpSkipHeader*, uint32_t> skipHeaderPositions;
#endif

	for (auto it = stream.opcodes(); it; ++it)
	{
		if (it->op == SerializationOpcode::NextPage)
			continue; // technical opcode, ignore

		switch (it->op)
		{
		case SerializationOpcode::SkipHeader:
		{
			const auto* op = (const SerializationOpSkipHeader*)(*it);

			uint8_t tempBuffer[16];
			WriteAdaptiveNumber4(tempBuffer, op->skipSizeValueSize, op->skipOffset);
			writer.writeToFile(tempBuffer, op->skipSizeValueSize);

#ifdef BUILD_DEBUG
			skipHeaderPositions[op] = writer.pos();
#endif
			break;
		}

		case SerializationOpcode::SkipLabel:
		{
#ifdef BUILD_DEBUG
			const auto* op = (const SerializationOpSkipLabel*)(*it);
			uint32_t headerOffset = 0;
			ASSERT(skipHeaderPositions.find(op->header, headerOffset));
			const auto actualSkipOffset = writer.pos() - headerOffset;
			ASSERT(op->header->skipOffset == actualSkipOffset);
#endif
			break;
		}

		case SerializationOpcode::Property:
		{
			const auto* op = (const SerializationOpProperty*)(*it);
			WriteIndex(writer, op->shared.index);
			break;
		}

		case SerializationOpcode::DataBlock1:
		{
			const auto* op = (const SerializationOpDataBlock1*)(*it);
			writer.writeToFile(op + 1, op->size);
			break;
		}

		case SerializationOpcode::DataBlock2:
		{
			const auto* op = (const SerializationOpDataBlock2*)(*it);
			writer.writeToFile(op + 1, op->size);
			break;
		}

		case SerializationOpcode::DataBlock4:
		{
			const auto* op = (const SerializationOpDataBlock4*)(*it);
			writer.writeToFile(op + 1, op->size);
			break;
		}

		case SerializationOpcode::DataTypeRef:
		{
			const auto* op = (const SerializationOpDataTypeRef*)(*it);
			WriteIndex(writer, op->shared.index);
			break;
		}

		case SerializationOpcode::DataName:
		{
			const auto* op = (const SerializationOpDataName*)(*it);
			WriteIndex(writer, op->shared.index);
			break;
		}

		case SerializationOpcode::DataObjectPointer:
		{
			const auto* op = (const SerializationOpDataObjectPointer*)(*it);
			WriteIndex(writer, op->shared.index);
			break;
		}

		case SerializationOpcode::DataAdaptiveNumber:
		{
			const auto* op = (const SerializationOpDataAdaptiveNumber*)(*it);
			WriteIndex(writer, op->value);
			break;
		}

		case SerializationOpcode::DataResourceRef:
		{
			const auto* op = (const SerializationOpDataResourceRef*)(*it);
			WriteIndex(writer, op->shared.index);
			break;
		}

		case SerializationOpcode::DataInlineBuffer:
		{
			const auto* op = (const SerializationOpDataInlineBuffer*)(*it);

			uint8_t type = 0;

			if (op->data)
				type = FLAG_INLINED_BUFFER;
			else if (op->asyncLoader)
				type = FLAG_ASYNC_BUFFER;

			writer.writeToFile(&type, sizeof(type));

			if (op->data)
			{
				WriteIndex(writer, op->data.size());
				writer.writeToFile(op->data.data(), op->data.size());
			}
			else if (op->asyncLoader)
			{
				ASSERT(mappedReferences.mappedAsyncBuffers.contains(op->asyncLoader));

				const auto crc = op->asyncLoader->crc();
				writer.writeToFile(&crc, sizeof(crc));
			}

			break;
		}

		default:
			ASSERT(!"Unknown opcode");
		}
	}
}

void WriteOpcodes(const SerializationStream& stream, const SerializationMappedReferences& mappedReferences, SerializationBinaryPacker& writer)
{
    EstimateDiskPositions(stream, mappedReferences, writer, false);
    EstimateDiskPositions(stream, mappedReferences, writer, true);
	WriteFinalizedOpcodes(stream, mappedReferences, writer);
}
        
//--

END_INFERNO_NAMESPACE()
