/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "objectBinarySaver.h"
#include "textSerializationObjectMapper.h"

#include "bm/core/parser/include/textFileWriter.h"
#include "bm/core/memory/include/bufferView.h"
#include "bm/core/file/include/fileReader.h"
#include "bm/core/file/include/fileMemoryWriter.h"
#include "objectBinaryLoader.h"
#include "serializationBufferFactory.h"

BEGIN_INFERNO_NAMESPACE()

//--

bool IObject::SaveObject(SerializationFormat format, const ObjectSavingContext& ctx, const IObject* object, IFormatStream& f)
{
	DEBUG_CHECK_RETURN_EX_V(object, "Nothing to write", nullptr);

	if (format == SerializationFormat::RawBinary)
	{
		Buffer buffer;
		if (!SaveObject(format, ctx, object, buffer))
			return false;

		buffer.view().encode(EncodingType::Base64, f);
		return true;
	}
	else
	{
		const auto ptr = ObjectPtr(AddRef(object));
		const auto data = TypedMemory::Wrap(ptr);
		return TypedMemory::SaveData(format, ctx, data, f);
	}
}

bool IObject::SaveObject(SerializationFormat format, const ObjectSavingContext& ctx, const IObject* object, IFileWriter* writer)
{
	DEBUG_CHECK_RETURN_EX_V(object, "Nothing to write", nullptr);

	if (format == SerializationFormat::RawBinary)
	{
		DEBUG_CHECK_RETURN_EX_V(ObjectBinarySaver::SaveObjects(writer, ctx, AddRef(object)), "Object serialization failed", nullptr);
		return true;
	}
	else
	{
		TextFileWriter textWriter(writer);
		return SaveObject(format, ctx, object, textWriter);
	}
}

bool IObject::SaveObject(SerializationFormat format, const ObjectSavingContext& ctx, const IObject* object, Buffer& outBufer)
{
	DEBUG_CHECK_RETURN_EX_V(object, "Nothing to write", nullptr);

	auto writer = RefNew<FileMemoryWriter>("ObjectSerialization", ctx.pagedPool);
	if (!SaveObject(format, ctx, object, writer))
		return false;

	DEBUG_CHECK_RETURN_EX_V(writer->size(), "Nothing written", nullptr);

	auto buffer = Buffer::CreateEmpty(ctx.pool ? *ctx.pool : MainPool(), writer->size());
	DEBUG_CHECK_RETURN_EX_V(buffer, "Out of memory on object serialization - unable to reassemble into one buffer", nullptr);

	BufferOutputStream<uint8_t> exporter(buffer);
	writer->exportDataToMemory(0, writer->size(), exporter);
	DEBUG_CHECK_RETURN_EX_V(exporter.size() == buffer.size(), "Out of memory on object serialization - not all data copied from paged buffer", nullptr);

	outBufer = buffer;
	return true;
}

bool IObject::SaveObject(SerializationFormat format, const ObjectSavingContext& ctx, const IObject* object, StringView absoluteFilePath, IFileSystem& fs /*= FileSystem()*/)
{
	if (auto writer = fs.openForWriting(absoluteFilePath, FileWriteMode::WriteOnly))
		return SaveObject(format, ctx, object, writer);

	return false;
}

//--

Buffer IObject::toBuffer(SerializationFormat format /*= SerializationFormat::RawBinary*/, IPoolUnmanaged& pool /*= MainPool()*/) const
{
	Buffer dataBuffer;
	ObjectSavingContext ctx;
	IObject::SaveObject(format, ctx, this, dataBuffer);
	return dataBuffer;
}

StringBuf IObject::toString(SerializationFormat format /*= SerializationFormat::Json*/) const
{
	StringBuilder txt;
	ObjectSavingContext ctx;
	IObject::SaveObject(format, ctx, this, txt);
	return StringBuf(txt);
}

//--

bool IObject::LocateBufferPlacement(BufferView data, uint64_t crc, uint64_t& outBufferOffset, uint32_t& outBufferCompressedSize, uint32_t& outBufferUncompressedSize, CompressionType& outCompressionType)
{
	ISerializationBufferFactory::BufferInfo info;
	if (!ObjectBinaryLoader::LocateBufferPlacement(data, crc, info))
		return false;

	outBufferOffset = info.fileOffset;
	outBufferCompressedSize = info.compressedSize;
	outBufferUncompressedSize = info.uncompressedSize;
	outCompressionType = info.compressionType;
	return true;
}

//--

ObjectPtr IObject::LoadObject(SerializationFormat format, const ObjectLoadingContext& ctx, BufferView data)
{
	DEBUG_CHECK_RETURN_EX_V(data, "No data to load", nullptr);

	ObjectPtr ptr;

	if (format == SerializationFormat::RawBinary)
	{
		if (!ObjectBinaryLoader::LoadObjects(ctx, data, ptr))
			return nullptr;
	}
	else
	{
		auto wrapped = TypedMemory::Wrap(ptr);
		if (!TypedMemory::LoadData(format, ctx, wrapped, StringView(data)))
			return nullptr;
	}

	return ptr;
}

ObjectPtr IObject::LoadObject(SerializationFormat format, const ObjectLoadingContext& ctx, StringView txt)
{
	return LoadObject(format, ctx, txt.bufferView());
}

ObjectPtr IObject::LoadObject(SerializationFormat format, const ObjectLoadingContext& ctx, IFileReader* file)
{
	auto data = file->loadToBuffer(MainPool(), file->fullRange());
	DEBUG_CHECK_RETURN_EX_V(data, "Failed to load data to buffer", nullptr);

	return LoadObject(format, ctx, data);
}

ObjectPtr IObject::LoadObject(SerializationFormat format, const ObjectLoadingContext& ctx, StringView absoluteFilePath, IFileSystem& fs /*= FileSystem()*/)
{
	auto data = fs.loadFileToBuffer(absoluteFilePath);
	DEBUG_CHECK_RETURN_EX_V(data, "Failed to load data to buffer", nullptr);

	return LoadObject(format, ctx, data);
}

//--

END_INFERNO_NAMESPACE()
