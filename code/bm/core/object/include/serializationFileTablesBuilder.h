/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

#include "serializationFileTables.h"

#include "bm/core/containers/include/hashMap.h"
#include "bm/core/system/include/guid.h"

BEGIN_INFERNO_NAMESPACE()

//--

// builder of the file tables
class BM_CORE_OBJECT_API SerializationBinaryFileTablesBuilder : public MainPoolData<NoCopy>
{
public:
    SerializationBinaryFileTablesBuilder();
    SerializationBinaryFileTablesBuilder(const SerializationBinaryFileTables& tables); // load existing data

    //--

    // write to physical file
    bool write(IFileWriter* file, uint32_t headerFlags, uint64_t objectEndPos, uint64_t bufferEndPos, const void* prevHeader = nullptr) const;

    //--

    int version = VER_CURRENT;

    Array<char> stringTable;
    Array<SerializationBinaryFileTables::Name> nameTable;
    Array<SerializationBinaryFileTables::Type> typeTable;
    Array<SerializationBinaryFileTables::Property> propertyTable;
    Array<SerializationBinaryFileTables::Import> importTable;
    Array<SerializationBinaryFileTables::Export> exportTable;
    Array<SerializationBinaryFileTables::Buffer> bufferTable;

    //--

    HashMap<SerializationBinaryFileTables::Type, uint32_t> typeMap;
    HashMap<SerializationBinaryFileTables::Property, uint32_t> propertyMap;
    HashMap<SerializationBinaryFileTables::Import, uint32_t> importMap;
    
    HashMap<StringBuf, uint32_t> stringRawMap;
    HashMap<StringID, uint32_t> nameRawMap;
    HashMap<StringID, uint32_t> typeRawMap;
    HashMap<uint64_t, uint32_t> bufferRawMap;

    //--

    struct BufferData
    {
        uint64_t crc = 0;
        Buffer compressedData;
        CompressionType compressionType = CompressionType::Uncompressed;
        uint64_t uncompressedSize = 0;
    };

    Array<BufferData> bufferData;

    //--

    uint32_t mapString(StringView txt);
    uint16_t mapName(StringID name);

    uint16_t mapType(StringID typeName);
    uint16_t mapType(Type type);

    uint16_t mapProperty(StringID classType, StringID propName, StringID propType);
    uint16_t mapProperty(const SerializationBinaryFileTables::Property& prop);
    uint16_t mapProperty(const Property* prop);

    uint16_t mapImport(const SerializationResourceKey& id);
    uint16_t mapImport(const SerializationBinaryFileTables::Import& importInfo);

    StringID typeName(uint32_t index) const;

    void initFromTables(const SerializationBinaryFileTables& tables);
};

//--

END_INFERNO_NAMESPACE()
