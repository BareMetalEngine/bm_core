/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "serializationFileTables.h"
#include "serializationFileTablesBuilder.h"

#include "bm/core/file/include/fileReader.h"
#include "bm/core/file/include/fileWriter.h"
#include "bm/core/object/include/rttiProperty.h"
#include "serializationStream.h"

BEGIN_INFERNO_NAMESPACE()

//--

SerializationBinaryFileTablesBuilder::SerializationBinaryFileTablesBuilder()
{
    stringTable.reserve(4096);
    stringTable.pushBack(0);

    nameTable.reserve(64);
    nameTable.emplaceBack();

    typeTable.reserve(64);
    typeTable.emplaceBack();

	propertyTable.reserve(64);
    propertyTable.emplaceBack();

    importTable.reserve(64);
    exportTable.reserve(64);

    stringRawMap.reserve(64);
    nameRawMap.reserve(64);
    typeMap.reserve(64);
    importMap.reserve(64);
}

template< typename T >
static void ExtractChunk(Array<T>& localData, const SerializationBinaryFileTables& tables, SerializationBinaryFileTables::ChunkType chunkType)
{
    const auto count = tables.chunkCount(chunkType);
    localData.resize(count);
    memcpy(localData.data(), tables.chunkData(chunkType), sizeof(T) * count);
};

SerializationBinaryFileTablesBuilder::SerializationBinaryFileTablesBuilder(const SerializationBinaryFileTables& tables)
{
    ExtractChunk(stringTable, tables, SerializationBinaryFileTables::ChunkType::Strings);
    ExtractChunk(nameTable, tables, SerializationBinaryFileTables::ChunkType::Names);
    ExtractChunk(propertyTable, tables, SerializationBinaryFileTables::ChunkType::Properties);
    ExtractChunk(importTable, tables, SerializationBinaryFileTables::ChunkType::Imports);
    ExtractChunk(exportTable, tables, SerializationBinaryFileTables::ChunkType::Exports);

    //--

    if (stringTable.empty())
        stringTable.pushBack(0);

    const char* strPtr = stringTable.typedData() + 1;
    const char* strEnd = stringTable.typedData() + stringTable.size();
    while (strPtr < strEnd)
    {
        const auto length = strlen(strPtr);
        stringRawMap[StringBuf(strPtr)] = strPtr - stringTable.typedData();
        strPtr += (length + 1);
    }

    //--

    for (uint32_t i = 0; i < nameTable.size(); ++i)
    {
        const auto& nameEntry = nameTable[i];
        const auto* nameString = stringTable.typedData() + nameEntry.stringIndex;
        const auto name = StringID(nameString);
        nameRawMap[name] = i;
    }

    //--
}

//--

uint32_t SerializationBinaryFileTablesBuilder::mapString(StringView txt)
{
    if (txt.empty())
        return 0;

    uint32_t index = 0;
    if (stringRawMap.find(txt, index))
        return index;

    index = stringTable.size();

    {
        uint32_t size = txt.length() + 1;
        auto ptr = stringTable.allocateUninitialized(size);
        memcpy(ptr, txt.data(), txt.length());
        ptr[txt.length()] = 0;
    }

    stringRawMap.set(StringBuf(txt), index);
    return index;
}

//--

uint16_t SerializationBinaryFileTablesBuilder::mapName(StringID name)
{
    if (!name)
        return 0;

    uint32_t index = 0;
    if (nameRawMap.find(name, index))
        return index;

    index = nameTable.size();

    auto& nameEntry = nameTable.emplaceBack();
    nameEntry.stringIndex = mapString(name.view());
    nameRawMap[name] = index;

    return index;
}

//--

StringID SerializationBinaryFileTablesBuilder::typeName(uint32_t index) const
{
    if (!index || index >= typeTable.size())
        return StringID();

    const auto& typeEntry = typeTable[index];
    const auto& nameEntry = nameTable[typeEntry.nameIndex];

    const auto* str = stringTable.typedData() + nameEntry.stringIndex;
    return StringID::Find(str);
}

uint16_t SerializationBinaryFileTablesBuilder::mapType(Type type)
{
    if (!type)
        return 0;

    return mapType(type->name());
}

uint16_t SerializationBinaryFileTablesBuilder::mapType(StringID typeName)
{
    if (!typeName)
        return 0;

    uint32_t ret = 0;
    if (typeRawMap.find(typeName, ret))
        return ret;

    SerializationBinaryFileTables::Type info;
    info.nameIndex = mapName(typeName);

    if (typeMap.find(info, ret))
        return ret;

    ret = typeTable.size();
    typeTable.pushBack(info);
    typeRawMap[typeName] = ret;
    typeMap[info] = ret;

    // TRACE_INFO("Internal Type '{}' mapped to {}", typeName, ret);
    return ret;
}

uint16_t SerializationBinaryFileTablesBuilder::mapProperty(StringID classType, StringID propName, StringID propType)
{
    if (!classType || !propName)
        return 0;

    SerializationBinaryFileTables::Property prop;
    prop.classTypeIndex = mapType(classType);
    prop.typeIndex = mapType(propType);
    prop.nameIndex = mapName(propName);
    return mapProperty(prop);
}

uint16_t SerializationBinaryFileTablesBuilder::mapProperty(const Property* prop)
{
    if (!prop)
        return 0;

    return mapProperty(prop->parent()->name(), prop->name(), prop->type()->name());
}

uint16_t SerializationBinaryFileTablesBuilder::mapProperty(const SerializationBinaryFileTables::Property& prop)
{
    if (!prop.classTypeIndex || !prop.nameIndex)
        return 0;

    uint32_t ret = 0;
    if (propertyMap.find(prop, ret))
        return ret;

    ret = propertyTable.size();
    propertyTable.pushBack(prop);
    propertyMap[prop] = ret;
    return ret;
}

uint16_t SerializationBinaryFileTablesBuilder::mapImport(const SerializationResourceKey& id)
{
    if (!id.id)
        return 0;

    SerializationBinaryFileTables::Import importInfo;
    importInfo.classTypeIndex = mapType(id.className);
    importInfo.guid[0] = id.id.data()[0];
    importInfo.guid[1] = id.id.data()[1];
    importInfo.guid[2] = id.id.data()[2];
    importInfo.guid[3] = id.id.data()[3];
    return mapImport(importInfo);
}

uint16_t SerializationBinaryFileTablesBuilder::mapImport(const SerializationBinaryFileTables::Import& importInfo)
{
    if (!importInfo.guid[0] && !importInfo.guid[1] && !!importInfo.guid[2] && !importInfo.guid[3])
        return 0;

    uint32_t ret = 0;
    if (importMap.find(importInfo, ret))
    {
        ASSERT(ret != 0);
        auto& entry = importTable[ret - 1];
        return ret;
    }

    importTable.pushBack(importInfo);
    ret = importTable.size();

    importMap[importInfo] = ret;
    return ret;
}

//--

void SerializationBinaryFileTablesBuilder::initFromTables(const SerializationBinaryFileTables& tables)
{
    stringTable.reset();
    nameTable.reset();
    typeTable.reset();
    importTable.reset();
    exportTable.reset();
    propertyTable.reset();

    stringTable.pushBack(0);
    nameTable.emplaceBack();
    typeTable.emplaceBack();

    {
        const auto numNames = tables.chunkCount(SerializationBinaryFileTables::ChunkType::Names);
        for (uint32_t i = 1; i < numNames; ++i)
        {
            const auto* str = tables.stringTable() + tables.nameTable()[i].stringIndex;

            auto& entry = nameTable.emplaceBack();
            entry.stringIndex = mapString(str);

            nameRawMap[StringID(str)] = i;
        }
    }

    {
        const auto numTypes = tables.chunkCount(SerializationBinaryFileTables::ChunkType::Types);
        for (uint32_t i = 1; i < numTypes; ++i)
        {
            auto& entry = typeTable.emplaceBack();
            entry.nameIndex = tables.typeTable()[i].nameIndex;

            const auto* str = tables.stringTable() + tables.nameTable()[entry.nameIndex].stringIndex;
            typeRawMap[StringID(str)] = i;
        }
    }

    {
        const auto numProperties = tables.chunkCount(SerializationBinaryFileTables::ChunkType::Properties);
        for (uint32_t i = 0; i < numProperties; ++i)
        {
            auto& entry = propertyTable.emplaceBack();
            entry.classTypeIndex = tables.propertyTable()[i].classTypeIndex;
            entry.nameIndex = tables.propertyTable()[i].nameIndex;
            propertyMap[entry] = i;
        }
    }

    {
        const auto numExports = tables.chunkCount(SerializationBinaryFileTables::ChunkType::Exports);
        for (uint32_t i = 0; i < numExports; ++i)
        {
            const auto& src = tables.exportTable()[i];
            auto& entry = exportTable.emplaceBack();
            entry.classTypeIndex = src.classTypeIndex;
            entry.crc = src.crc;
            entry.flags = src.flags;
            entry.dataOffset = src.dataOffset;
            entry.dataSize = src.dataSize;
        }
    }

    {
        const auto numImports = tables.chunkCount(SerializationBinaryFileTables::ChunkType::Imports);
        for (uint32_t i = 0; i < numImports; ++i)
        {
            const auto& src = tables.importTable()[i];

            auto& entry = importTable.emplaceBack();
            entry.guid[0] = src.guid[0];
            entry.guid[1] = src.guid[1];
            entry.guid[2] = src.guid[2];
            entry.guid[3] = src.guid[3];
        }
    }
}

//--

template< typename T >
bool WriteChunk(IFileWriter* file, uint64_t baseOffset, const Array<T>& entries, SerializationBinaryFileTables::Chunk& outChunk)
{
    outChunk.count = entries.size();
    outChunk.size = entries.dataSize();
    outChunk.offset = file->pos() - baseOffset;
    outChunk.crc = CRC32().append(entries.data(), entries.dataSize()).crc();

    if (entries.empty())
        return true;

    return file->writeSync(entries.data(), entries.dataSize()) == entries.dataSize();
}

bool SerializationBinaryFileTablesBuilder::write(IFileWriter* file, uint32_t headerFlags, uint64_t objectEndPos, uint64_t bufferEndPos, const void* prevHeader) const
{
    const uint64_t baseOffset = file->pos();

    // write the header (placeholder)
    SerializationBinaryFileTables::Header writeHeader;
    memset(&writeHeader, 0, sizeof(writeHeader));
    if (file->writeSync(&writeHeader, sizeof(writeHeader)) != sizeof(writeHeader))
        return false;

    // write chunks
    if (!WriteChunk(file, baseOffset, stringTable, writeHeader.chunks[(int)SerializationBinaryFileTables::ChunkType::Strings]))
        return false;
    if (!WriteChunk(file, baseOffset, nameTable, writeHeader.chunks[(int)SerializationBinaryFileTables::ChunkType::Names]))
        return false;
    if (!WriteChunk(file, baseOffset, typeTable, writeHeader.chunks[(int)SerializationBinaryFileTables::ChunkType::Types]))
        return false;
    if (!WriteChunk(file, baseOffset, propertyTable, writeHeader.chunks[(int)SerializationBinaryFileTables::ChunkType::Properties]))
        return false;
    if (!WriteChunk(file, baseOffset, importTable, writeHeader.chunks[(int)SerializationBinaryFileTables::ChunkType::Imports]))
        return false;
    if (!WriteChunk(file, baseOffset, exportTable, writeHeader.chunks[(int)SerializationBinaryFileTables::ChunkType::Exports]))
        return false;
	if (!WriteChunk(file, baseOffset, bufferTable, writeHeader.chunks[(int)SerializationBinaryFileTables::ChunkType::Buffers]))
		return false;

    // update header
    writeHeader.magic = SerializationBinaryFileTables::FILE_MAGIC;
    writeHeader.headersEnd = file->pos() - baseOffset;
    writeHeader.buffersEnd = bufferEndPos;
    writeHeader.objectsEnd = objectEndPos;

    // setup version and flags - if we had previous header use values from it
    if (prevHeader)
    {
        const auto& prevHeaderData = *(const SerializationBinaryFileTables::Header*)prevHeader;
        writeHeader.version = prevHeaderData.version;
        writeHeader.flags = prevHeaderData.flags;
    }
    else
    {
        writeHeader.version = SerializationBinaryFileTables::FILE_VERSION_MAX;
        writeHeader.flags = headerFlags;
    }

    writeHeader.crc = SerializationBinaryFileTables::CalcHeaderCRC(writeHeader);

    // write patched up header again
    {
        auto pos = file->pos();
        file->seek(baseOffset);
        if (file->writeSync(&writeHeader, sizeof(writeHeader)) != sizeof(writeHeader))
            return false;
        file->seek(pos);
    }

    // file tables saved
    return true;
}

//--

END_INFERNO_NAMESPACE()
