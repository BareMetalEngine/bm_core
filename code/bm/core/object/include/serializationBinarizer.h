/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
*
* [#filter: serialization #]
***/

#pragma once

#include "bm/core/containers/include/hashSet.h"

#include "serializationStream.h"
#include "serializationWriter.h"

BEGIN_INFERNO_NAMESPACE()

//--

/// mapped tables for writing
struct BM_CORE_OBJECT_API SerializationMappedReferences : public MainPoolData<NoCopy>
{
    SerializationMappedReferences();

    HashMap<StringID, uint32_t> mappedNames;
    HashMap<Type, uint32_t> mappedTypes;
    HashMap<const IObject*, uint32_t> mappedPointers;
    HashMap<const Property*, uint32_t> mappedProperties;
    HashMap<SerializationResourceKey, uint32_t> mappedResources;
    HashSet<AsyncFileBufferLoaderPtr> mappedAsyncBuffers;
};

//--

// binary data writer for the token stream, provides basic buffering and translation
class BM_CORE_OBJECT_API SerializationBinaryPacker : public MainPoolData<NoCopy>
{
public:
    SerializationBinaryPacker(IFileWriter* outputFile);
    ~SerializationBinaryPacker();

    //--

    // get CRC of written data
    INLINE uint32_t crc() const { return m_crc.crc(); }

    // current position in output stream
    INLINE uint32_t pos() const { return m_pos; }

    //--

    // flush internal buffer
    void flush();

    // write given amount of data to file
    void writeToFile(const void* data, uint64_t size);

    //--

private:
    static const uint32_t BUFFER_SIZE = 328U << 10;

    IFileWriter* m_outputFile;
    uint32_t m_pos = 0;

    uint8_t m_cacheBuffer[BUFFER_SIZE];
    uint32_t m_cacheBufferSize = 0;

    CRC32 m_crc;
};

//--

// write opcode stream to file
extern void BM_CORE_OBJECT_API WriteOpcodes(const SerializationStream& stream, const SerializationMappedReferences& mappedReferences, SerializationBinaryPacker& writer);
            
//--

END_INFERNO_NAMESPACE()
