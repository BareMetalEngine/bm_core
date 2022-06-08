/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "object.h"
#include "objectBinarySaver.h"

#include "asyncBuffer.h"

#include "serializationStream.h"
#include "serializationWriter.h"
#include "serializationBinarizer.h"
#include "serializationFileTables.h"
#include "serializationFileTablesBuilder.h"

#include "bm/core/file/include/fileReader.h"
#include "bm/core/file/include/fileWriter.h"
#include "bm/core/containers/include/queue.h"
#include "bm/core/memory/include/localAllocator.h"

BEGIN_INFERNO_NAMESPACE()

//--

struct SerializedObject : public MainPoolData<NoCopy>
{
    ObjectPtr object;
    SerializationStream stream;
	bool root = false;
};

//--

class SerializedObjectCollection : public MainPoolData<NoCopy>
{
public:
    SerializedObjectCollection(LocalAllocator& mem);
    ~SerializedObjectCollection();

    INLINE const Array< SerializedObject*>& objects() const { return m_objectMap.values(); }

    SerializedObject* mapObject(const IObject* object);

private:
    LocalAllocator& m_mem;
    HashMap<const IObject*, SerializedObject*> m_objectMap;
};

//--

class SerializedObjectQueue : public MainPoolData<NoCopy>
{
public:
    SerializedObjectQueue();
    ~SerializedObjectQueue();

    void push(SerializedObject* object);

    SerializedObject* pop();

private:
    Queue<SerializedObject*> m_objectsToSave;
    HashSet<const SerializedObject*> m_visitedObjects;
};

//--

SerializedObjectCollection::SerializedObjectCollection(LocalAllocator& mem)
    : m_mem(mem)
{}

SerializedObjectCollection::~SerializedObjectCollection()
{
    for (auto* val : m_objectMap.values())
        val->~SerializedObject();
    m_objectMap.clear();
}

SerializedObject* SerializedObjectCollection::mapObject(const IObject* object)
{
    if (!object)
        return nullptr;

    SerializedObject* ret = nullptr;
    if (!m_objectMap.find(object, ret))
    {
        ret = m_mem.createWithoutDestruction<SerializedObject>();
        ret->object = AddRef(object);
        m_objectMap[object] = ret;
    }

    return ret;
}

//---

SerializedObjectQueue::SerializedObjectQueue()
{}

SerializedObjectQueue::~SerializedObjectQueue()
{}

void SerializedObjectQueue::push(SerializedObject* object)
{
    if (object && m_visitedObjects.insert(object))
        m_objectsToSave.push(object);
}
                
SerializedObject* SerializedObjectQueue::pop()
{
    if (m_objectsToSave.empty())
        return nullptr;

    auto* ret = m_objectsToSave.top();
    m_objectsToSave.pop();
    return ret;
}

//----

bool ObjectBinarySaver::CollectObjects(LocalAllocator& mem, const ObjectSavingContext& context, ObjectPtr object, SerializedObjectCollection& outCollection, SerializationWriterReferences& outReferences)
{
    ScopeTimer timer;
    SerializedObjectQueue objectQueue;

    // start saving with the root object
    if (object)
    {
		auto* obj = outCollection.mapObject(object);
		obj->root = true;
		objectQueue.push(obj);
    }
#if 0
    for (const auto* rootObject : context.rootObjects)
    {
        auto* obj = outCollection.mapObject(rootObject);
        obj->root = true;
        objectQueue.push(obj);
    }
#endif

    // local set for tracking strong-referenced objects
	HashSet<IObject*> localStrongReferences;
    localStrongReferences.reserve(256);

    // save objects
    uint32_t numSavedObjects = 0;
    SerializationStreamAllocator streamAllocator(mem);    
    while (auto* obj = objectQueue.pop())
    {
        if (localStrongReferences.size() <= 256)
        {
            localStrongReferences.reset();
        }
        else
        {
            localStrongReferences.clear();
            localStrongReferences.reserve(256);
        }

        // serialize object to opcodes
        {
            SerializationWriter writer(obj->stream, streamAllocator, outReferences, localStrongReferences);
            obj->object->onWriteBinary(writer);

            // if we had serialization errors exit now
            if (writer.errors())
            {
                TRACE_WARNING("Opcode stream corruption at object '{}' 0x{}. Possible OOM.", obj->object->cls()->name(), Hex(obj->object.get()));
                return false;
            }
        }

        // all objects referenced by a strong pointers must also be serialized
		for (const auto& referencedObject : localStrongReferences.keys())
            objectQueue.push(outCollection.mapObject(referencedObject));

        // count saved object
        numSavedObjects += 1;
    }

    // final stats
    TRACE_SPAM("Serialized {} objects in {} ({} opcodes, {} in opcodes)",
        numSavedObjects, timer,  streamAllocator.totalBlocksAllocated(), MemSize(streamAllocator.totalBytesAllocator()));
    return true;
}

void ObjectBinarySaver::BuildFileTables(const Array<SerializedObject*>& objects, const SerializationWriterReferences& collectedReferences, SerializationBinaryFileTablesBuilder& outTables, SerializationMappedReferences& outMappedReferences)
{
    // NOTE: maintain determinism

    // map strings
    for (const auto& name : collectedReferences.stringIds.keys())
    {
        const auto index = outTables.mapName(name);
        outMappedReferences.mappedNames[name] = index;
    }

    // map type references
    for (const auto& type : collectedReferences.types.keys())
    {
        const auto index = outTables.mapType(type);
        outMappedReferences.mappedTypes[type] = index;
    }

    // map property references    
    for (const auto* prop : collectedReferences.properties.keys())
    {
        const auto index = outTables.mapProperty(prop);
        outMappedReferences.mappedProperties[prop] = index;
    }

    // map resource reference
    for (const auto& id : collectedReferences.resources.keys())
    {
        const auto index = outTables.mapImport(id);
        DEBUG_CHECK(index != 0);
        outMappedReferences.mappedResources[id] = index;
    }

    // map buffers
    for (const auto& buf : collectedReferences.asyncBuffers.keys())
    {
        outMappedReferences.mappedAsyncBuffers.insert(buf);
        // NOTE: buffers are indexed by data CRC64 so no index
    }

    // map objects
    for (uint32_t i=0; i<objects.size(); ++i)
    {
        const auto* obj = objects[i];

        auto& exportInfo = outTables.exportTable.emplaceBack();
        exportInfo.classTypeIndex = outTables.mapType(obj->object->cls());

        exportInfo.flags = 0;
        if (obj->root)
            exportInfo.flags |= 1;

        outMappedReferences.mappedPointers[obj->object.get()] = i + 1;
    }

    // sort buffers by the CRC to allow for interpolation search
    auto bufferKeys = outMappedReferences.mappedAsyncBuffers.keys();
    std::sort(bufferKeys.begin(), bufferKeys.end(), [](const auto& a, const auto& b) { return a->crc() < b->crc(); });

    // build buffer table
    for (const auto& buffer : bufferKeys)
    {
        uint64_t crc = buffer->crc();
        if (!outTables.bufferRawMap.contains(crc))
        {
            SerializationBinaryFileTablesBuilder::BufferData data;
            if (buffer->extract(NoTask(), MainPool(), data.compressedData, data.compressionType) && data.compressedData)
            {
                data.crc = crc;
                data.uncompressedSize = buffer->size();
                outTables.bufferData.pushBack(data);

                auto& entry = outTables.bufferTable.emplaceBack();
                entry.crc = crc;
                entry.compressionType = (uint8_t)data.compressionType;
                entry.compressedSize = data.compressedData.size();
                entry.uncompressedSize = buffer->size();
                entry.dataOffset = 0; // not yet saved
            }
        }
    }
}

void ObjectBinarySaver::WriteObjects(const ObjectSavingContext& context, const SerializedObjectCollection& objects, const SerializationMappedReferences& mappedReferences, SerializationBinaryFileTablesBuilder& tables, uint64_t baseOffset, IFileWriter* file)
{
    ScopeTimer timer;

    // write all objects
    uint64_t totalDataSize = 0;
    for (auto i : objects.objects().indexRange())
    {
        const auto* object = objects.objects()[i];

        // write binary opcode stream to file
        const auto objectStartPos = file->pos();
        {
            SerializationBinaryPacker fileWriter(file);
            WriteOpcodes(object->stream, mappedReferences, fileWriter);
            fileWriter.flush();

            tables.exportTable[i].crc = fileWriter.crc();
        }

        // count data size
        totalDataSize += file->pos() - objectStartPos;

        // patch object entry
        tables.exportTable[i].dataOffset = objectStartPos - baseOffset;
        tables.exportTable[i].dataSize = file->pos() - objectStartPos;
    }

    TRACE_SPAM("Written {} objects in {} ({} written)", objects.objects().size(), timer, MemSize(totalDataSize));    
}

uint32_t ObjectBinarySaver::HeaderFlags(const ObjectSavingContext& context)
{
    uint32_t flags = 0;

    if (context.extractedBuffers)
        flags |= SerializationBinaryFileTables::FileFlag_ExtractedBuffers;

    return flags;
}

bool ObjectBinarySaver::SaveObjects(IFileWriter* file, const ObjectSavingContext& context, ObjectPtr object)
{
    ScopeTimer timer;

    // NOTE: keep allocator outside
    LocalAllocator mem;

    {
		// collect objects to save and build local serialization streams
        SerializedObjectCollection objectCollection(mem);
        SerializationWriterReferences objectReferences;
        if (!CollectObjects(mem, context, object, objectCollection, objectReferences))
            return false;

        // merge reference tables
        SerializationBinaryFileTablesBuilder fileTables;
        SerializationMappedReferences mappedReferences;
        BuildFileTables(objectCollection.objects(), objectReferences, fileTables, mappedReferences);

        // store the file header to reserve space in the final file
        // NOTE: header size will not change since we gathered and mapped all references
        const auto baseOffset = file->pos();
        if (!fileTables.write(file, HeaderFlags(context), 0, 0))
            return false;

        // write objects
        WriteObjects(context, objectCollection, mappedReferences, fileTables, baseOffset, file);

        // extract dependencies
        if (context.extractedResources)
        {
            for (const auto& info : fileTables.importTable)
            {
                SerializationResourceKey key;
                key.id = GUID(info.guid[0], info.guid[1], info.guid[2], info.guid[3]);
                key.className = fileTables.typeName(info.classTypeIndex);
                context.extractedResources->insert(key);
            }
        }

        // remember file position after all objects were written
        const auto objectEnd = file->pos() - baseOffset;

        // write or extract buffers
        ASSERT(fileTables.bufferData.size() == fileTables.bufferTable.size());
        for (auto i : fileTables.bufferData.indexRange())
        {
            const auto& data = fileTables.bufferData[i];

            if (context.extractedBuffers)
            {
                auto& entry = (*context.extractedBuffers)[data.crc];
                entry.compressedData = data.compressedData;
                entry.compressionType = data.compressionType;
                entry.uncompressedSize = data.uncompressedSize;
                entry.uncompressedCRC = data.crc;
            }
            else
            {
                auto& entry = fileTables.bufferTable[i];
                entry.dataOffset = file->pos() - baseOffset;
                entry.compressedSize = data.compressedData.size();

                const auto numWritten = data.compressedData.size();
                if (file->writeSync(data.compressedData.data(), data.compressedData.size()) != numWritten)
                {
                    TRACE_WARNING("Failed to save {} bytes of async buffer {}, saved {} only", data.compressedData.size(), data.crc, numWritten);
                    return false;
                }
            }
        }

        // remember file position after all buffers were written
        const auto buffersEnd = file->pos() - baseOffset;
        const auto bufferDataSize = buffersEnd - objectEnd;

        // write the final header again
        file->seek(baseOffset);
        if (!fileTables.write(file, HeaderFlags(context), objectEnd, buffersEnd))
        {
            file->seek(buffersEnd);
            return false;
        }

        // go back
        file->seek(buffersEnd);

        // done
        TRACE_SPAM("Saved {} objects ({} of objects, {} of buffers) in {}",
            objectCollection.objects().size(), MemSize(objectEnd), MemSize(bufferDataSize), timer);
    }

    return true;
}

void ObjectBinarySaver::ExtractUsedResources(const IObject* object, HashMap<ResourceID, uint32_t>& outResourceCounts)
{
    /*if (object)
    {
        ObjectSavingContext context;
        context.rootObjects.pushBack(object);

        LocalAllocator mem;
        SerializedObjectCollection objectCollection;
        if (CollectObjects(mem, context, objectCollection))
        {
            SerializationBinaryFileTablesBuilder fileTables;
            SerializationMappedReferences mappedReferences;
            BuildFileTables(objectCollection.orderedObjects(), fileTables, mappedReferences);

            for (const auto& id : mappedReferences.mappedResources.keys())
                outResourceCounts[id] += 1;
        }
    }*/
}

//--

END_INFERNO_NAMESPACE()
