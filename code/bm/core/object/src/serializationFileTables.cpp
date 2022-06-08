/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "serializationFileTables.h"

BEGIN_INFERNO_NAMESPACE()

const uint32_t SerializationBinaryFileTables::FILE_MAGIC = 0x49524346; // 'IRCF';
const uint32_t SerializationBinaryFileTables::FILE_VERSION_MIN = 1;
const uint32_t SerializationBinaryFileTables::FILE_VERSION_MAX = VER_CURRENT;

//---

bool SerializationBinaryFileTables::ValidateHeader(const Header& header, uint32_t sizeOfData, StringBuf* outError /*= nullptr*/)
{
    if (sizeOfData != sizeof(header))
    {
        if (outError)
            *outError = StringBuf("Invalid header size {}", sizeOfData);
        return false;
    }

    if (header.magic != FILE_MAGIC)
    {
        if (outError)
            *outError = TempString("Invalid header magic {}", Hex(header.magic));
        return false;
    }

    if (header.version < FILE_VERSION_MIN)
    {
        if (outError)
            *outError = TempString("Deprecated file version {}", header.version);
        return false;
    }
    else if (header.version > FILE_VERSION_MAX)
    {
        if (outError)
            *outError = TempString("Unsupported file version {}", header.version);
        return false;
    }

    const auto headerCRC = CalcHeaderCRC(header);
    if (headerCRC != header.crc)
    {
        if (outError)
            *outError = StringBuf("Header consistency check failed");
        return false;
    }
            
    return true;
}

bool SerializationBinaryFileTables::validateHeader(uint64_t avaiableSize) const
{
    if (avaiableSize < sizeof(Header))
        return false;

    const auto* headerData = header();
    if (headerData->magic != FILE_MAGIC)
        return false;

    if (headerData->version < FILE_VERSION_MIN)
        return false;
    else if (headerData->version > FILE_VERSION_MAX)
        return false;

    const auto headerCRC = CalcHeaderCRC(*headerData);
    if (headerCRC != headerData->crc)
        return false;

    if (avaiableSize < headerData->headersEnd)
        return false;

    return true;
}

bool SerializationBinaryFileTables::validateTables(uint64_t memorySize, StringBuf* outError /*= nullptr*/) const
{
    // no data or not enough data
    if (memorySize < sizeof(Header))
    {
        if (outError)
            *outError = TempString("Not enough bytes for a file header ({} < {})", memorySize, sizeof(Header));
        return false;
    }

    // check header magic
    const auto* headerData = header();
    if (headerData->magic != FILE_MAGIC)
    {
        if (outError)
            *outError = TempString("Invalid header magic {} != {}", Hex(headerData->magic), Hex(FILE_MAGIC));
        return false;
    }

    // check version range
    if (headerData->version < FILE_VERSION_MIN)
    {
        if (outError)
            *outError = TempString("Deprecated file version {}", headerData->version);
        return false;
    }
    else if (headerData->version > FILE_VERSION_MAX)
    {
        if (outError)
            *outError = TempString("Unsupported file version {}", headerData->version);
        return false;
    }

    // check header CRC value
    const auto headerCRC = CalcHeaderCRC(*headerData);
    if (headerCRC != headerData->crc)
    {
        if (outError)
            *outError = TempString("Invalid header CRC {} != {}", Hex(headerCRC), Hex(headerData->crc));
        return false;
    }

    // check that data for all chunks if within the block we read
    const auto numChunks = (uint8_t)ChunkType::MAX;
    for (uint32_t i = 0; i < numChunks; ++i)
    {
        const auto& chunk = headerData->chunks[i];
        if ((uint32_t)(chunk.offset + chunk.size) > memorySize)
        {
            if (outError)
                *outError = TempString("Chunk {} spans past the end of the memory block: {} + {} > {}", i, chunk.offset, chunk.size, memorySize);
            return false;
        }

#ifndef BUILD_RELEASE
        {
            const auto* chunkData = (const uint8_t*)this + chunk.offset;
            const auto chunkCRC = CRC32().append(chunkData, chunk.size).crc();
            if (chunkCRC != chunk.crc)
            {
                if (outError)
                    *outError = TempString("Chunk {} has invalid CRC: {} != {}", i, Hex(chunkCRC), Hex(chunk.crc));
                return false;
            }
        }
#endif
    }

    // clear imports
    if (headerData->version < VER_NEW_RESOURCE_ID)
    {
        auto& nonConstChunk = const_cast<Chunk&>(header()->chunks[(int)ChunkType::Imports]);
        nonConstChunk.count = 0;
    }

    // check all name entries
    {
        const auto count = chunkCount(ChunkType::Names);
        const auto stringCount = chunkCount(ChunkType::Strings);
        const auto* ptr = nameTable();
        for (uint32_t i = 0; i < count; ++i, ptr++)
        {
            if (ptr->stringIndex >= stringCount)
            {
                if (outError)
                    *outError = TempString("Name entry {} points to stirng table at {} that is outside it's size {}", i, ptr->stringIndex, stringCount);
                return false;
            }
        }
    }

    // check all type entries
    {
        const auto count = chunkCount(ChunkType::Types);
        const auto nameCount = chunkCount(ChunkType::Names);
        const auto* ptr = typeTable();
        for (uint32_t i = 0; i < count; ++i, ptr++)
        {
            if (ptr->nameIndex >= nameCount)
            {
                if (outError)
                    *outError = TempString("Type entry {} points to name table at {} that is outside it's size {}", i, ptr->nameIndex, nameCount);
                return false;
            }
        }
    }

    // check all properties
    {
        const auto count = chunkCount(ChunkType::Properties);
        const auto nameCount = chunkCount(ChunkType::Names);
        const auto typeCount = chunkCount(ChunkType::Types);
        const auto* ptr = propertyTable();
        for (uint32_t i = 0; i < count; ++i, ptr++)
        {
            if (ptr->classTypeIndex >= typeCount)
            {
                if (outError)
                    *outError = TempString("Property entry {} points to type table at {} that is outside it's size {}", i, ptr->classTypeIndex, typeCount);
                return false;
            }

            /*if (ptr->typeIndex >= typeCount)
            {
                TRACE_WARNING("Property entry {} points to type table at {} that is outside it's size {}", i, ptr->typeIndex, typeCount);
                return false;
            }*/

            if (ptr->nameIndex >= nameCount)
            {
                if (outError)
                    *outError = TempString("Property entry {} points to name table at {} that is outside it's size {}", i, ptr->nameIndex, nameCount);
                return false;
            }
        }
    }

    // check all exports
    {
        const auto count = chunkCount(ChunkType::Exports);
        const auto typeCount = chunkCount(ChunkType::Types);
        const auto* ptr = exportTable();
        for (uint32_t i = 0; i < count; ++i, ptr++)
        {
            if (ptr->classTypeIndex >= typeCount)
            {
                if (outError)
                    *outError = TempString("Export entry {} points to type table at {} that is outside it's size {}", i, ptr->classTypeIndex, typeCount);
                return false;
            }

            /*if (ptr->parentIndex > i)
            {
                if (outError)
                    *outError = TempString("Export entry {} points to export table at {} that is not preceeding it in the file", i, ptr->parentIndex);
                return false;
            }*/
        }
    }
        
    // tables seem valid
    return true;
}

//---

SerializationBinaryFileTables::CRCValue SerializationBinaryFileTables::CalcHeaderCRC(const Header& header)
{
    // tricky bit - the CRC field cannot be included in the CRC calculation - calculate the CRC without it
    Header tempHeader;
    tempHeader = header;
    tempHeader.crc = 0xDEADBEEF; // special hacky stuff

    return CRC32().append(&tempHeader, sizeof(tempHeader)).crc();
}

//--

END_INFERNO_NAMESPACE()
