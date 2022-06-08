/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "textSerializationObjectMapper.h"
#include "textSerializationWriterXML.h"
#include "resourceReference.h"
#include "resource.h"
#include "bm/core/parser/include/xmlWriter.h"

BEGIN_INFERNO_NAMESPACE()

//--

TextSerializationWriterXML::TextSerializationWriterXML(const TextSerializationMappedObjects& objects, const ObjectSavingContext& ctx, XMLWriter* doc)
	: m_doc(AddRef(doc))
	, m_objects(objects)
	, m_context(ctx)
{
	m_stack.pushBack(doc->root());
}

TextSerializationWriterXML::~TextSerializationWriterXML()
{}

//--

void TextSerializationWriterXML::beingArray()
{

}

void TextSerializationWriterXML::endArray()
{

}

void TextSerializationWriterXML::beingArrayElement()
{
	auto parentId = m_stack.back();
	auto node = m_doc->createNode(parentId, "element");
	m_stack.pushBack(node);
}

void TextSerializationWriterXML::endArrayElement()
{
	DEBUG_CHECK_RETURN(m_stack.size() >= 1);
	m_stack.popBack();
}

void TextSerializationWriterXML::beginStruct(Type type, const void* ptr)
{
	auto parentId = m_stack.back();

	/*if (const auto* info = m_objects.objects.find((const IObject*)ptr))
	{
		m_doc->createAttribute(parentId, "class", type.name().view());

		if (info->assignedIndex != -1)
			m_doc->createAttribute(parentId, "id", TempString("{}", info->assignedIndex));
	}*/
}

void TextSerializationWriterXML::endStruct(Type type, const void* ptr)
{
}

void TextSerializationWriterXML::beginStructElement(StringView name)
{
	auto parentId = m_stack.back();
	auto node = m_doc->createNode(parentId, name);
	m_stack.pushBack(node);
}

void TextSerializationWriterXML::endStructElement(StringView name)
{
	DEBUG_CHECK_RETURN(m_stack.size() >= 1);
	m_stack.popBack();
}

void TextSerializationWriterXML::writeValueText(StringView value)
{
	auto parentId = m_stack.back();
	m_doc->changeNodeText(parentId, value);
}

void TextSerializationWriterXML::writeValueBytes(BufferView view)
{
	auto parentId = m_stack.back();
	m_doc->changeNodeDataCopy(parentId, view);
}

void TextSerializationWriterXML::writeValueOwnedBytes(Buffer view)
{
	auto parentId = m_stack.back();
	m_doc->changeNodeData(parentId, view);
}

void TextSerializationWriterXML::writeValueObject(const IObject* obj, bool strong)
{
	auto parentId = m_stack.back();

	if (!obj)
	{
		m_doc->changeNodeText(parentId, "null");
		return;
	}

	const auto info = m_objects.objects.find(obj);
	DEBUG_CHECK_RETURN_EX(info, "Unknown object encountered");

	if (info)
	{
		if (m_definedObjects.insert(obj) && strong) // only strong ref may define
		{
			m_doc->createAttribute(parentId, "class", obj->cls().name().view());

			if (info->assignedIndex != -1)
				m_doc->createAttribute(parentId, "id", TempString("{}", info->assignedIndex));

			obj->writeText(*this);
		}
		else if (info->numStrongRefs)
		{
			ASSERT(info->assignedIndex != -1);
			m_doc->createAttribute(parentId, "refId", TempString("{}", info->assignedIndex));
		}
		else
		{
			m_doc->changeNodeText(parentId, "null"); // lost object
		}
	}
	else
	{
		m_doc->changeNodeText(parentId, "null");
	}
}

void TextSerializationWriterXML::writeValueResource(const BaseReference& rr)
{
	auto parentId = m_stack.back();

	if (rr.inlined())
	{
		writeValueObject(rr.resolveResource(), true);
	}
	else if (rr.external())
	{
		//auto nodeId = m_doc->createNode(parentId, "resource");
		m_doc->createAttribute(parentId, "class", rr.resolveClass().name().view());
		m_doc->createAttribute(parentId, "guid", TempString("{}", rr.resolveId()));
	}
	else
	{
		m_doc->changeNodeText(parentId, "null");
	}
}

//--

END_INFERNO_NAMESPACE();