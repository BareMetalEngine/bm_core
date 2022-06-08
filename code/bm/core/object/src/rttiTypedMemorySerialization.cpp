/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"

#include "bm/core/parser/include/xmlReader.h"
#include "bm/core/parser/include/xmlWriter.h"
//#include "bm/core/parser/include/jsonReader.h"
#include "bm/core/parser/include/jsonWriter.h"

#include "textSerializationObjectMapper.h"
#include "textSerializationWriterXML.h"
#include "textSerializationReaderXML.h"
#include "textSerializationWriterJson.h"
//#include "textSerializationReaderJson.h"

BEGIN_INFERNO_NAMESPACE()

//---

static StringView AutoXMLRootNodeName(Type type, const ObjectSavingContext& ctx)
{
	if (ctx.rootNodeName)
		return ctx.rootNodeName;

	if (type)
	{
		switch (type->metaType())
		{
		case MetaType::Array:
			return "array";
		case MetaType::StrongHandle:
		case MetaType::WeakHandle:
			return "object";
		}
	}

	return "data";
}

bool TypedMemory::SaveData(SerializationFormat format, const ObjectSavingContext& ctx, TypedMemory data, IFormatStream& f)
{
	DEBUG_CHECK_RETURN_EX_V(data, "Nothing to save", false);

	TextSerializationMappedObjects objectMap;
	objectMap.extractedResources = ctx.extractedResources;

	const void* defaultData = nullptr;
	if (auto clsType = data.rawType().toClass())
		defaultData = clsType->defaultObject();

	{
		TextSerializationObjectMapper mapper(objectMap);
		TypeSerializationContext ctx;
		data.rawType()->writeText(ctx, mapper, data.rawData(), nullptr);
	}

	if (format == SerializationFormat::XML)
	{
		auto name = AutoXMLRootNodeName(data.rawType(), ctx);

		auto doc = XMLWriter::Create(name, *ctx.pagedPool);
		DEBUG_CHECK_RETURN_EX_V(doc, "Failed to create XML saver", false);

		TextSerializationWriterXML writer(objectMap, ctx, doc);
		TypeSerializationContext localContext;
		data.rawType()->writeText(localContext, writer, data.rawData(), defaultData);

		XMLWriter::SaveToTextFile(doc, doc->root(), f, ctx.textPrintFlags);

		return true;
	}
	else if (format == SerializationFormat::Json)
	{
		auto name = AutoXMLRootNodeName(data.rawType(), ctx);

		auto doc = JSONWriter::Create(*ctx.pagedPool);
		DEBUG_CHECK_RETURN_EX_V(doc, "Failed to create JSON saver", false);

		TextSerializationWriterJSON writer(objectMap, ctx, doc);
		TypeSerializationContext localContext;
		data.rawType()->writeText(localContext, writer, data.rawData(), defaultData);

		JSONWriter::SaveToTextFile(doc, doc->root(), f, ctx.textPrintFlags);

		return true;
	}
	else
	{
		return false;
	}
}

bool TypedMemory::LoadData(SerializationFormat format, const ObjectLoadingContext& ctx, TypedMemory data, StringView txt, ITextErrorReporter& err /*= ITextErrorReporter::GetDefault()*/)
{
	if (format == SerializationFormat::XML)
	{
		auto doc = XMLReader::LoadFromText(err, ctx.contextPath, txt);
		DEBUG_CHECK_RETURN_EX_V(doc, "XML parsing error", false);

		TextSerializationReaderXML reader(doc, ctx, err);
		TypeSerializationContext localContext;
		data.rawType()->readText(localContext, reader, data.rawData());

		return true;
	}
	else if (format == SerializationFormat::Json)
	{
		return false;
	}

	return false;
}

//---


END_INFERNO_NAMESPACE()
