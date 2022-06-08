/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "resourceReference.h"
#include "resource.h"
#include "textSerializationObjectMapper.h"
#include "textSerializationWriterJson.h"
#include "bm/core/parser/include/jsonWriter.h"

BEGIN_INFERNO_NAMESPACE()

//--

TextSerializationWriterJSON::TextSerializationWriterJSON(const TextSerializationMappedObjects& objects, const ObjectSavingContext& ctx, JSONWriter* doc)
	: m_doc(AddRef(doc))
	, m_objects(objects)
	, m_context(ctx)
{
	m_stack.pushBack(doc->root());
}

TextSerializationWriterJSON::~TextSerializationWriterJSON()
{}

//--

void TextSerializationWriterJSON::beingArray()
{
	auto nodeId = m_stack.back();
	m_doc->changeNodeType(nodeId, JSONNodeType::Array);
}

void TextSerializationWriterJSON::endArray()
{
}

void TextSerializationWriterJSON::beingArrayElement()
{
	auto parentId = m_stack.back();
	auto nodeId = m_doc->createNode(parentId);
	m_stack.pushBack(nodeId);
}

void TextSerializationWriterJSON::endArrayElement()
{
	m_stack.popBack();
}

void TextSerializationWriterJSON::beginStruct(Type type, const void* ptr)
{
	auto nodeId = m_stack.back();
	m_doc->changeNodeType(nodeId, JSONNodeType::Compound);

	if (type.toClass().is<IObject>() && ptr && m_nextStructIsObject)
	{
		if (const auto info = m_objects.objects.find((const IObject*)ptr))
		{
			{
				auto attributeId = m_doc->createNode(nodeId, JSONNodeType::Value);
				m_doc->changeNodeName(attributeId, "#class");
				m_doc->changeNodeText(attributeId, type.name().view());
			}

			if (info->assignedIndex != -1)
			{
				auto attributeId = m_doc->createNode(nodeId, JSONNodeType::Value);
				m_doc->changeNodeName(attributeId, "#id");
				m_doc->changeNodeText(attributeId, TempString("{}", info->assignedIndex));
			}
		}
	}

	m_nextStructIsObject = false;
}

void TextSerializationWriterJSON::endStruct(Type type, const void* ptr)
{
}

void TextSerializationWriterJSON::beginStructElement(StringView name)
{
	auto parentId = m_stack.back();
	auto nodeId = m_doc->createNode(parentId);
	m_doc->changeNodeName(nodeId, name);
	m_stack.pushBack(nodeId);
}

void TextSerializationWriterJSON::endStructElement(StringView name)
{
	m_stack.popBack();
}

void TextSerializationWriterJSON::writeValueText(StringView value)
{
	auto nodeId = m_stack.back();

	m_doc->changeNodeType(nodeId, JSONNodeType::Value);
	m_doc->changeNodeText(nodeId, value);
}

void TextSerializationWriterJSON::writeValueBytes(BufferView view)
{
	auto nodeId = m_stack.back();

	m_doc->changeNodeType(nodeId, JSONNodeType::Value);
	m_doc->changeNodeDataCopy(nodeId, view);
}

void TextSerializationWriterJSON::writeValueOwnedBytes(Buffer view)
{
	auto nodeId = m_stack.back();

	m_doc->changeNodeType(nodeId, JSONNodeType::Value);
	m_doc->changeNodeData(nodeId, view);
}

void TextSerializationWriterJSON::writeValueObject(const IObject* obj, bool strong)
{
	auto parentId = m_stack.back();

	if (!obj)
	{
		writeValueText("null");
		return;
	}

	const auto info = m_objects.objects.find(obj);
	DEBUG_CHECK_RETURN_EX(info, "Unknown object encountered");

	if (info)
	{
		if (m_definedObjects.insert(obj) && strong) // only strong ref may define
		{
			ASSERT(!m_nextStructIsObject);
			m_nextStructIsObject = true;

			obj->writeText(*this);
		}
		else if (info->numStrongRefs)
		{
			writeValueText(TempString("#ref({})", info->assignedIndex));
		}
		else
		{
			writeValueText("null");
		}
	}
	else
	{
		writeValueText("null");
	}
}

void TextSerializationWriterJSON::writeValueResource(const BaseReference& rr)
{
	auto parentId = m_stack.back();

	if (rr.inlined())
		writeValueObject(rr.resolveResource(), true);
	else if (rr.external())
		m_doc->changeNodeText(parentId, TempString("{}:{}", rr.resolveClass().name().view(), rr.resolveId()));
	else
		m_doc->changeNodeText(parentId, "null");
}

//--

END_INFERNO_NAMESPACE();