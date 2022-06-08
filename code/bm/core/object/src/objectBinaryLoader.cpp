/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/
#include "build.h"
#include "asyncBuffer.h"
#include "serializationFileTables.h"
#include "serializationErrorReporter.h"

#include "resource.h"
#include "resourcePromise.h"

#include "object.h"
#include "objectBinaryLoader.h"

#include "bm/core/file/include/fileReader.h"
#include "bm/core/file/include/fileView.h"
#include "bm/core/parser/include/xmlReader.h"
#include "bm/core/object/include/serializationReader.h"
#include "bm/core/object/include/object.h"
#include "serializationBufferFactory.h"
#include "serializationStream.h"

BEGIN_INFERNO_NAMESPACE()

//--

void ObjectBinaryLoader::ResolveStringIDs(const SerializationBinaryFileTables& tables, const ObjectLoadingContext& context, SerializationResolvedReferences& resolvedReferences)
{
    PC_SCOPE_LVL2(ResolveStringIDs);

    const auto numStringIds = tables.chunkCount(SerializationBinaryFileTables::ChunkType::Names);
    resolvedReferences.stringIds.resize(numStringIds);

    const auto* strings = tables.stringTable();
    const auto* ptr = tables.nameTable();
    for (uint32_t i = 0; i < numStringIds; ++i, ++ptr)
    {
        const auto* str = strings + ptr->stringIndex;
        //TRACE_INFO("Name[{}]: '{}'", i, str);
        auto stringID = StringID(str);
        resolvedReferences.stringIds[i] = stringID;
    }
}

void ObjectBinaryLoader::ResolveTypes(const SerializationBinaryFileTables& tables, const ObjectLoadingContext& context, SerializationResolvedReferences& resolvedReferences)
{
    PC_SCOPE_LVL2(ResolveTypes);

    const auto numTypes = tables.chunkCount(SerializationBinaryFileTables::ChunkType::Types);
    resolvedReferences.types.resize(numTypes);
    resolvedReferences.typeNames.resize(numTypes);

    resolvedReferences.types[0] = Type();
    resolvedReferences.typeNames[0] = StringID();

    const auto* ptr = tables.typeTable();
    for (uint32_t i = 0; i < numTypes; ++i, ++ptr)
    {
        auto typeName = resolvedReferences.stringIds[ptr->nameIndex];
        resolvedReferences.typeNames[i] = typeName;

        if (typeName)
        {
            auto type = RTTI::GetInstance().findType(typeName);
            if (!type)
            {
                if (context.collectedErrors)
                    context.collectedErrors->reportTypeMissing(context.contextPath, typeName);
            }

            //TRACE_INFO("Type[{}]: '{}'", i, type);
            resolvedReferences.types[i] = type;
        }
    }
}

void ObjectBinaryLoader::ResolveProperties(const SerializationBinaryFileTables& tables, const ObjectLoadingContext& context, SerializationResolvedReferences& resolvedReferences)
{
    PC_SCOPE_LVL2(ResolveProperties);

    const auto numProperties = tables.chunkCount(SerializationBinaryFileTables::ChunkType::Properties);
    resolvedReferences.properties.resize(numProperties);

    const auto* ptr = tables.propertyTable();
    for (uint32_t i = 0; i < numProperties; ++i, ++ptr)
    {
        auto classType = resolvedReferences.types[ptr->classTypeIndex].toClass();

		auto& info = resolvedReferences.properties[i];
        info.name = resolvedReferences.stringIds[ptr->nameIndex];
        info.type = resolvedReferences.types[ptr->typeIndex];
		info.typeName = resolvedReferences.stringIds[ptr->typeIndex];

		if (classType)
			info.prop = classType->findProperty(info.name);

        /*// report errors
        // TODO: details!
		if (context.errorColletor)
		{
            if (!info.prop)
            {
                if (!errorFileInfo)
                    errorFileInfo = context.errorColletor->fileInfo(context.debugLoadPath);

                ObjectLoadingErrorCollector::PropertyInfo prop;
                prop.propertyName = info.name;
                prop.className = classType->name();
                errorFileInfo->missingProperties.insert(prop);
            }
            else if (info.prop->type() != info.type)
            {
                if (info.type)
                {
					if (!errorFileInfo)
						errorFileInfo = context.errorColletor->fileInfo(context.debugLoadPath);

					ObjectLoadingErrorCollector::PropertyInfo prop;
					prop.propertyName = info.name;
					prop.className = classType->name();
					errorFileInfo->missingProperties.insert(prop);
                }
                else
                {
					if (!errorFileInfo)
						errorFileInfo = context.errorColletor->fileInfo(context.debugLoadPath);

					ObjectLoadingErrorCollector::PropertyInfo prop;
					prop.propertyName = info.name;
					prop.className = classType->name();
					errorFileInfo->missingProperties.insert(prop);
                }
            }
        }*/
    }
}

void ObjectBinaryLoader::ResolveImports(const SerializationBinaryFileTables& tables, const ObjectLoadingContext& context, SerializationResolvedReferences& resolvedReferences)
{
    if (tables.header()->version < VER_NEW_RESOURCE_ID)
        return;

    const auto numImports = tables.chunkCount(SerializationBinaryFileTables::ChunkType::Imports);
    resolvedReferences.resources.resize(numImports);

    const auto* ptr = tables.importTable();
    for (uint32_t i = 0; i < numImports; ++i, ++ptr)
    {
		auto& res = resolvedReferences.resources[i];

        res.cls = resolvedReferences.types[ptr->classTypeIndex].toClass().cast<IResource>();
        res.id = ResourceID(GUID(ptr->guid[0], ptr->guid[1], ptr->guid[2], ptr->guid[3]));

		res.promise = ResourcePromise::CreateEmptyPromise(res.id, res.cls);
		DEBUG_CHECK(res.promise);

        if (res.promise)
        {
            if (context.resourcePromises)
                context.resourcePromises->pushBack(res.promise);
            else
                res.promise->fulfill(nullptr); // never going to be resolved
        }        
    }
}

void ObjectBinaryLoader::ResolveExports(const SerializationBinaryFileTables& tables, const ObjectLoadingContext& context, SerializationResolvedReferences& resolvedReferences, ObjectPtr& outRoot)
{
    const auto numExports = tables.chunkCount(SerializationBinaryFileTables::ChunkType::Exports);
    resolvedReferences.objects.resize(numExports);

    const auto* ptr = tables.exportTable();
    for (uint32_t i = 0; i < numExports; ++i, ++ptr)
    {
        auto classTypeName = resolvedReferences.typeNames[ptr->classTypeIndex];
        auto classType = resolvedReferences.types[ptr->classTypeIndex].toClass();

        if (!classType || !classType->is<IObject>() || !classType->isConstructible() || classType->isAbstract())
        {
            TRACE_WARNING("Object '{}' is using invalid type '{}' that is not a creatable class", i, classTypeName);

            /*if (context.errorColletor)
            {
                if (!errorFileInfo)
                    errorFileInfo = context.errorColletor->fileInfo(context.debugLoadPath);
                errorFileInfo->missingClasses.insert(classTypeName);
            }*/

            continue;
        }

        /*/ObjectPtr parentObject;
        if (ptr->parentIndex != 0)
        {
            parentObject = resolvedReferences.objects[ptr->parentIndex - 1];
            if (!parentObject) // skip loading if parent object failed to load
                continue;
        }*/

		// selective loading
		if (!context.selectiveLoadingClasses.empty())
		{
            if (!context.selectiveLoadingClasses.contains(classType))
                continue;
		}

        auto obj = classType->create<IObject>();
        resolvedReferences.objects[i] = obj;

        // remember selective loaded objects in a separate list
        if (!context.selectiveLoadingClasses.empty() && context.collectedSelectiveObjects)
            context.collectedSelectiveObjects->pushBack(obj);

        if (i == 0 && context.contextPath)
            if (auto* resource = rtti_cast<IResource>(obj.get()))
                resource->bindLoadPath(context.contextPath);
    
        // root ?
        if (ptr->flags & 1) 
        {
            if (!outRoot)
            {
                if (!context.expectedRootClass || obj->is(context.expectedRootClass))
                    outRoot = obj;
            }

            if (context.collectedRootObjects)
                context.collectedRootObjects->pushBack(obj);
        }
    }
}

void ObjectBinaryLoader::ResolveBuffers(const SerializationBinaryFileTables& tables, const ObjectLoadingContext& context, SerializationResolvedReferences& resolvedReferences, BufferView data)
{
	const auto numBuffers = tables.chunkCount(SerializationBinaryFileTables::ChunkType::Buffers);
	//resolvedReferences.buffers.resize(numBuffers);

	const auto* ptr = tables.bufferTable();
	for (uint32_t i = 0; i < numBuffers; ++i, ++ptr)
	{
        ISerializationBufferFactory::BufferInfo info;
        info.crc = ptr->crc;
        info.compressedSize = ptr->compressedSize;
        info.compressionType = (CompressionType)ptr->compressionType;
        info.uncompressedSize = ptr->uncompressedSize;
        info.fileOffset = ptr->dataOffset;

        AsyncFileBufferLoaderPtr loader;
        if (context.bufferFactory)
        {
            // use external factory
            loader = context.bufferFactory->createAsyncBufferLoader(info);
        }
        else if (context.dataOwnerToInheritOtherUncompressedBuffers)
        {
			// create a SUB BUFFER of the original buffer (this will keep the original buffer alive as well but that's what we wanted)
			const auto compressedData = context.dataOwnerToInheritOtherUncompressedBuffers.createSubBuffer(ptr->dataOffset, ptr->compressedSize);
			DEBUG_CHECK_EX(compressedData, "Failed to create sub-buffer");

			// wrap it in a async loader that will decompress the data on request
			loader = IAsyncFileBufferLoader::CreateResidentBufferFromAlreadyCompressedData(compressedData, ptr->uncompressedSize, ptr->crc, (CompressionType)ptr->compressionType);
        }
        else
        {
			// we don't own the source data so we need to make a copy of it
			const auto compressedDataView = data.subView(ptr->dataOffset, ptr->compressedSize);
			DEBUG_CHECK_EX(compressedDataView, "Failed to create source view of the data");

			// copy 
			const auto compressedData = Buffer::CreateFromCopy(*context.pool, compressedDataView, DEFAULT_BUFFER_ALIGNMENT);
            DEBUG_CHECK_EX(compressedData, "Buffer copy failed, OOM?");

			// wrap it in a async loader that will decompress the data on request
			loader = IAsyncFileBufferLoader::CreateResidentBufferFromAlreadyCompressedData(compressedData, ptr->uncompressedSize, ptr->crc, (CompressionType)ptr->compressionType);
        }

		DEBUG_CHECK_EX(loader, "Invalid buffer loader");
        resolvedReferences.buffers[ptr->crc] = loader;
	}
}

#if 0
uint64_t ObjectBinaryLoader::DetermineLoadBufferSize(IAsyncFileHandle* file, const SerializationBinaryFileTables& tables, const ResourceLoadingContext& context, const SerializationResolvedReferences& resolvedReferences)
{
    // whole file is smaller than the load buffer
    // NOTE: use this ONLY if we indeed want to load the whole file
    if (tables.header()->objectsEnd <= DefaultLoadBufferSize)
        return tables.header()->objectsEnd;

    // file is bigger than the load buffer - we can't load it all at once
    // we will load it in chunks but we can't have smaller chunk than the biggest object
    uint64_t maxObjectSize = 0;
    const auto* ptr = tables.exportTable();
    const auto numObjects = tables.chunkCount(SerializationBinaryFileTables::ChunkType::Exports);
    for (uint32_t i = 0; i < numObjects; ++i, ++ptr)
    {
        // do not consider objects that were disabled from loading
        if (!resolvedReferences.objects[i])
            continue;

        // get size of the object to load
        const auto objectDataSize = ptr->dataSize;
        if (objectDataSize > maxObjectSize)
            maxObjectSize = objectDataSize;
    }

    // account for misalignment
    maxObjectSize += BlockSize;
    return std::max<uint32_t>(maxObjectSize, DefaultLoadBufferSize);
}
#endif

//--

class ObjectBinaryLoaderFallbackAsyncBufferFactory : public ISerializationBufferFactory
{
public:
    ObjectBinaryLoaderFallbackAsyncBufferFactory(const ObjectLoadingContext& context, const SerializationBinaryFileTables& tables, BufferView data)
    {
        const auto numBuffers = tables.chunkCount(SerializationBinaryFileTables::ChunkType::Buffers);
        const auto* bufferTable = tables.bufferTable();

        m_bufferMap.reserve(numBuffers);
        if (numBuffers)
        {
            uint64_t totalCompressedSize = 0;
            uint64_t totalUncompressedSize = 0;

            for (uint32_t i = 0; i < numBuffers; ++i)
            {
                const auto& bufferEntry = bufferTable[i];
                DEBUG_CHECK_EX(bufferEntry.crc, "Buffer with invalid CRC");

                // make ULTRA sure we don't have duplicates
                AsyncFileBufferLoaderPtr existingEntry;
                if (m_bufferMap.find(bufferEntry.crc, existingEntry))
                {
                    TRACE_ERROR("Async buffer {} (at entry {}, size {}) registered again", bufferEntry.crc, i, bufferEntry.uncompressedSize);
                    continue;
                }

                // create the async loader for the buffer
                if (const auto localData = createLoaderForBufferEntry(context, tables, bufferEntry, data))
                {
                    // store
                    m_bufferMap[bufferEntry.crc] = localData;

                    // stats
                    totalUncompressedSize += bufferEntry.uncompressedSize;
                    totalCompressedSize += bufferEntry.compressedSize;
                }
            }

            TRACE_INFO("Loading data with async buffers from MEMORY, {} buffers, {} of compresed data, {} of uncompressed data",
                m_bufferMap.size(), MemSize(totalCompressedSize), MemSize(totalUncompressedSize));
        }
    }

    AsyncFileBufferLoaderPtr createLoaderForBufferEntryFromOwnedBuffer(const ObjectLoadingContext& context, const SerializationBinaryFileTables& tables, const SerializationBinaryFileTables::Buffer& bufferEntry) const
    {
        DEBUG_CHECK_RETURN_EX_V(context.dataOwnerToInheritOtherUncompressedBuffers.size() >= tables.header()->buffersEnd, "Owned buffer data is smaller than needed for all buffers", nullptr);

        // create a SUB BUFFER of the original buffer (this will keep the original buffer alive as well but that's what we wanted)
        const auto compressedData = context.dataOwnerToInheritOtherUncompressedBuffers.createSubBuffer(bufferEntry.dataOffset, bufferEntry.compressedSize);
        DEBUG_CHECK_RETURN_EX_V(compressedData, "Failed to create sub-buffer", nullptr);

        // wrap it in a async loader that will decompress the data on request
        auto ret = IAsyncFileBufferLoader::CreateResidentBufferFromAlreadyCompressedData(compressedData, bufferEntry.uncompressedSize, bufferEntry.crc, (CompressionType)bufferEntry.compressionType);
        DEBUG_CHECK_RETURN_EX_V(ret, "Failed to create wrapping token", nullptr);

        return ret;
    }

	AsyncFileBufferLoaderPtr createLoaderForBufferEntry(const ObjectLoadingContext& context, const SerializationBinaryFileTables& tables, const SerializationBinaryFileTables::Buffer& bufferEntry, BufferView data) const
	{
        // if we have the owned buffer it would be best just to keep it
        if (context.dataOwnerToInheritOtherUncompressedBuffers)
            if (auto ret = createLoaderForBufferEntryFromOwnedBuffer(context, tables, bufferEntry))
                return nullptr;

        // we don't own the source data so we need to make a copy of it
        const auto compressedDataView = data.subView(bufferEntry.dataOffset, bufferEntry.compressedSize);
        DEBUG_CHECK_RETURN_EX_V(compressedDataView, "Failed to create source view of the data", nullptr);

        // copy 
        const auto compressedData = Buffer::CreateFromCopy(*context.pool, compressedDataView, DEFAULT_BUFFER_ALIGNMENT);
        DEBUG_CHECK_RETURN_EX_V(compressedData, "Buffer copy failed, OOM?", nullptr);

		// wrap it in a async loader that will decompress the data on request
		auto ret = IAsyncFileBufferLoader::CreateResidentBufferFromAlreadyCompressedData(compressedData, bufferEntry.uncompressedSize, bufferEntry.crc, (CompressionType)bufferEntry.compressionType);
		DEBUG_CHECK_RETURN_EX_V(ret, "Failed to create wrapping token", nullptr);

		return ret;
	}

    virtual AsyncFileBufferLoaderPtr createAsyncBufferLoader(const BufferInfo& info) const override
    {
        // look for already created wrapper
        AsyncFileBufferLoaderPtr ret;
        if (m_bufferMap.find(info.crc, ret))
            return ret;

        TRACE_WARNING("Async buffer {} was requested but it does not exist in file", info.crc);
        return nullptr;
    }

private:
    HashMap<uint64_t, AsyncFileBufferLoaderPtr> m_bufferMap;
};

//--

bool ObjectBinaryLoader::LoadObjects(const ObjectLoadingContext& context, BufferView data, ObjectPtr& outRoot)
{
    if (data.size() < sizeof(SerializationBinaryFileTables::Header))
        return false;

    DEBUG_CHECK_RETURN_EX_V(context.pool, "Memory pool can't be empty, use MainPool() at least", false);

    // try the XML format
    /*const auto maxCompare = std::min<uint32_t>(strlen(XMLReader::HEADER_TEXT()), sizeof(SerializationBinaryFileTables::Header));
    if (loadedHeader.size() >= maxCompare)
    {
        // TODO: try to load from XML!
    }*/

    // validate the file tables
    const auto& tables = *(const SerializationBinaryFileTables*)data.data();
    {
        StringBuf tablesError;
        if (!tables.validateTables(data.size(), &tablesError))
        {
            return false;
        }
    }

    // we have no buffer factory yet we have not enough data for buffer
    DEBUG_CHECK_RETURN_EX_V(context.bufferFactory || data.size() >= tables.header()->buffersEnd, "No buffer loader factory yet given data does not contain buffers", false);

    // resolve references
    SerializationResolvedReferences resolvedReferences;
    ResolveStringIDs(tables, context, resolvedReferences);
    ResolveTypes(tables, context, resolvedReferences);
    ResolveProperties(tables, context, resolvedReferences);
    ResolveImports(tables, context, resolvedReferences);
    ResolveExports(tables, context, resolvedReferences, outRoot);
    ResolveBuffers(tables, context, resolvedReferences, data);

    // load data and process it
    const auto* objectTable = tables.exportTable();
    const auto numObjects = tables.chunkCount(SerializationBinaryFileTables::ChunkType::Exports);
    for (uint32_t i=0; i<numObjects; ++i)
    {
        // do not consider objects that were disabled from loading
        if (auto object = resolvedReferences.objects[i])
        {
            // get the memory range in the loaded buffer where the object content is
            const auto& objectEntry = objectTable[i];

            // get object data in the buffer
            const auto* objectData = OffsetPtr<void>(data.data(), objectEntry.dataOffset);
            const auto objectDataView = BufferView(objectData, objectEntry.dataSize);

            // invalid data loaded ?
#ifndef BUILD_RELEASE
            {
                const auto crc = CRC32().append(objectData, objectEntry.dataSize).crc();
                DEBUG_CHECK_RETURN_EX_V(crc == objectEntry.crc, "Object data corruption", false);
            }
#endif

            // read the crap
            {
                SerializationReader reader(*context.pool, resolvedReferences, objectDataView, tables.header()->version, context.contextPath, *context.collectedErrors);
                object->onReadBinary(reader);
            }
        }
    }

    // post load objects
    //for (auto i : resolvedReferences.objects.indexRange().reversed())
    for (auto i : resolvedReferences.objects.indexRange())
        if (auto obj = resolvedReferences.objects[i])
            obj->onPostLoad();

    // loaded
    return true;
}

bool ObjectBinaryLoader::LoadFileContent(IFileView* file, Buffer& outData, FileLoadMode loadMode)
{
    DEBUG_CHECK_RETURN_EX_V(file, "Invalid file", false);

    SerializationBinaryFileTables::Header header;
    memzero(&header, sizeof(header));

    {
        const auto numRead = file->readSync(&header, sizeof(header));
        if (numRead != sizeof(header))
            return false;
    }

    if (header.magic != SerializationBinaryFileTables::FILE_MAGIC)
        return false;
    if (header.version < SerializationBinaryFileTables::FILE_VERSION_MIN)
        return false;
    if (header.version > SerializationBinaryFileTables::FILE_VERSION_MAX)
        return false;

    if (loadMode == FileLoadMode::DepsOnly)
    {
        if (header.headersEnd > file->size())
            return false;

        outData = Buffer::CreateEmpty(MainPool(), header.headersEnd);
    }
    else if (loadMode == FileLoadMode::NoBuffers)
    {
        if (header.objectsEnd > file->size())
            return false;

        outData = Buffer::CreateEmpty(MainPool(), header.headersEnd);
    }
    else
    {
        outData = Buffer::CreateEmpty(MainPool(), file->size());
    }

    DEBUG_CHECK_RETURN_EX_V(outData, "Unable to allocate memory for file loading", false);
    memcpy(outData.data(), &header, sizeof(header));

    {
        const auto expectedSize = outData.size() - sizeof(header);
        const auto numRead = file->readSync(outData.data() + sizeof(header), expectedSize);
        DEBUG_CHECK_RETURN_EX_V(numRead == expectedSize, "Unexpected IO error loading data", false);
    }

    return true;
}

bool ObjectBinaryLoader::LoadDependencies(const ObjectLoadingContext& context, BufferView data, HashSet<ResourceID>& outDependencies)
{
    // validate the file tables
    const auto& tables = *(const SerializationBinaryFileTables*)data.data();
    if (!tables.validateHeader(data.size()))
        return false;

    // extract imports
    const auto numImports = tables.chunkCount(SerializationBinaryFileTables::ChunkType::Imports);
    for (uint32_t i=0; i<numImports; ++i)
    {
        const auto& info = tables.importTable()[i];
        const auto guid = GUID(info.guid[0], info.guid[1], info.guid[2], info.guid[3]);
        outDependencies.insert(guid);
    }

    return true;
}

//--

bool ObjectBinaryLoader::LocateBufferPlacement(BufferView data, uint64_t crc, ISerializationBufferFactory::BufferInfo& outInfo)
{
	// validate the file tables
	const auto& tables = *(const SerializationBinaryFileTables*)data.data();
	if (!tables.validateHeader(data.size()))
		return false;

    // look for buffers
    // TODO: better search
	const auto numBuffers = tables.chunkCount(SerializationBinaryFileTables::ChunkType::Buffers);
    for (uint32_t i = 0; i < numBuffers; ++i)
    {
        const auto& info = tables.bufferTable()[i];
        if (info.crc == crc)
        {
			outInfo.fileOffset = info.dataOffset;
            outInfo.compressedSize = info.compressedSize;
            outInfo.compressionType = (CompressionType) info.compressionType;
			outInfo.uncompressedSize = info.uncompressedSize;
            return true;
        }
    }

    // buffer not found in file
    return false;
}

//--

END_INFERNO_NAMESPACE()
