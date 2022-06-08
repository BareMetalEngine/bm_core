/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "textSerializationObjectMapper.h"
#include "textSerializationReaderXML.h"
#include "serializationStream.h"

#include "resourceReference.h"
#include "resourcePromise.h"
#include "resource.h"
#include "resourceId.h"

#include "bm/core/parser/include/xmlReader.h"
#include "bm/core/parser/include/textToken.h"

BEGIN_INFERNO_NAMESPACE()

//--

TextSerializationReaderXML::TextSerializationReaderXML(const XMLReader* doc, const ObjectLoadingContext& context, ITextErrorReporter& err)
	: m_doc(doc)
	, m_context(context)
	, m_err(err)
{
	m_stack.pushBack(doc->root());
}

TextSerializationReaderXML::~TextSerializationReaderXML()
{
	DEBUG_CHECK_EX(m_stack.size() == 1, "Invalid stack count");
}

void TextSerializationReaderXML::reportError(StringView txt)
{
	auto parentId = m_stack.back();
	
	const auto line = m_doc->nodeLine(parentId);
	TextTokenLocation loc(m_context.contextPath, line);
	m_err.reportError(loc, txt);	
}

//--

bool TextSerializationReaderXML::beingArray(uint32_t& outCount)
{
	auto parentId = m_stack.back();

	uint32_t count = 0;

	{
		auto childId = m_doc->firstChild(parentId);
		while (childId)
		{
			count += 1;
			childId = m_doc->nextChild(childId);
		}
	}

	auto childId = m_doc->firstChild(parentId);
	m_stack.pushBack(childId);

	outCount = count;
	return true;
}

void TextSerializationReaderXML::endArray()
{
	DEBUG_CHECK_RETURN_EX(m_stack.size() >= 2, "Invalid stack");
	m_stack.popBack();
}

bool TextSerializationReaderXML::beingArrayElement()
{
	DEBUG_CHECK_RETURN_EX_V(m_stack.size() >= 2, "Invalid stack", false);

	auto childId = m_stack.back();
	return childId != 0;
}

void TextSerializationReaderXML::endArrayElement()
{
	DEBUG_CHECK_RETURN_EX(m_stack.size() >= 2, "Invalid stack");

	auto childId = m_stack.back();
	if (childId)
		childId = m_doc->nextChild(childId);
	m_stack.back() = childId;
}

bool TextSerializationReaderXML::beginStruct()
{
	auto parentId = m_stack.back();

	auto childId = m_doc->firstChild(parentId);
	if (childId != 0)
	{
		m_stack.pushBack(childId);
		return true;
	}

	return false;
}

void TextSerializationReaderXML::endStruct()
{
	DEBUG_CHECK_RETURN_EX(m_stack.size() >= 2, "Invalid stack");
	m_stack.popBack();
}

bool TextSerializationReaderXML::beginStructElement(StringView& outName)
{
	DEBUG_CHECK_RETURN_EX_V(m_stack.size() >= 2, "Invalid stack", false);
	auto childId = m_stack.back();

	if (!childId)
		return false;

	outName = m_doc->nodeName(childId);
	return true;
}

void TextSerializationReaderXML::endStructElement()
{
	DEBUG_CHECK_RETURN_EX(m_stack.size() >= 2, "Invalid stack");

	auto childId = m_stack.back();
	if (childId != 0)
		childId = m_doc->nextChild(childId);
	m_stack.back() = childId;
}

bool TextSerializationReaderXML::readValueText(StringView& outValue)
{
	auto parentId = m_stack.back();
	if (parentId)
	{
		outValue = m_doc->nodeValueText(parentId);
		return true;
	}

	return false;
}

bool TextSerializationReaderXML::readValueBytes(Buffer& outData)
{
	auto parentId = m_stack.back();
	if (parentId)
	{
		outData = m_doc->nodeValueBuffer(parentId);
		return true;
	}

	return false;
}

bool TextSerializationReaderXML::readValueObject(ObjectPtr& outObject)
{
	auto parentId = m_stack.back();
	if (parentId)
	{
		if (m_doc->nodeValueText(parentId) == "null")
		{
			outObject = nullptr;
			return true;
		}
		else if (const auto txt = m_doc->attributeValue(parentId, "refId"))
		{
			uint32_t id = 0;
			if (!txt.match(id))
			{
				reportError(TempString("Invalid reference id '{}'", txt));
				return false;
			}

			ObjectPtr ptr;
			if (!m_definedObjects.find(id, ptr))
			{
				reportError(TempString("Object reference ID {} is not defined", id));
				return false;
			}

			outObject = ptr;
			return true;				
		}
		else if (const auto txt = m_doc->attributeValue(parentId, "class"))
		{
			auto classType = RTTI::GetInstance().findClass(StringID(txt));

			if (!classType)
			{
				reportError(TempString("Object uses unknown class '{}'", txt));
				return false;
			}

			if (classType->isAbstract())
			{
				reportError(TempString("Object uses abstract class '{}'", txt));
				return false;
			}

			ObjectPtr ptr = classType->create<IObject>();
			if (!ptr)
			{
				reportError(TempString("Failed to create object of class '{}'", txt));
				return false;
			}

			if (const auto txt = m_doc->attributeValue(parentId, "id"))
			{
				uint32_t id = 0;
				if (!txt.match(id))
				{
					reportError(TempString("Object of class '{}' has invalid id '{}'", classType, id));
				}
				else if (m_definedObjects.contains(id))
				{
					reportError(TempString("Object of class '{}' at id '{}' was already defined", classType, id));
				}
				else
				{
					m_definedObjects[id] = ptr;
				}
			}

			if (ptr)
				ptr->readText(*this);

			outObject = ptr;
			return true;
		}
		else
		{
			reportError("Incomplete object definition");
			return false;
		}
	}

	return false;
}

bool TextSerializationReaderXML::readValueResource(BaseReference& outRef)
{
	auto parentId = m_stack.back();
	if (parentId)
	{
		if (m_doc->nodeValueText(parentId) == "null")
		{
			outRef = BaseReference();
			return true;
		}
		else if (auto txt = m_doc->attributeValue(parentId, "class", ""))
		{
			const auto resourceClass = RTTI::GetInstance().findClass(StringID(txt));
			if (!resourceClass)
			{
				reportError(TempString("Resource used unknown class '{}'", txt));
				return false;
			}

			if (!resourceClass->is<IResource>())
			{
				reportError(TempString("Object uses class '{}' that is not a resource class", txt));
				return false;
			}

			if (auto txt = m_doc->attributeValue(parentId, "guid", ""))
			{
				SerializationResourceKey key;
				key.className = resourceClass->name();

				if (!GUID::Parse(txt.data(), txt.length(), key.id))
				{
					reportError(TempString("Failed to parse ID for resource '{}' from '{}'", resourceClass, txt));
					return false;
				}

				auto promise = createResourcePromise(key);
				if (!promise)
				{
					reportError(TempString("Failed to create promise for resource '{}' ID '{}'", resourceClass, key.id));
					return false;
				}

				outRef = BaseReference(promise);
				return true;
			}
			else
			{
				ObjectPtr ptr;
				if (!readValueObject(ptr))
					return false;

				outRef = BaseReference(rtti_cast<IResource>(ptr));
				return true;
			}
		}
		else
		{
			reportError("Incomplete resource definition");
			return false;
		}
	}

	return false;
}

//--

ResourcePromisePtr TextSerializationReaderXML::createResourcePromise(const SerializationResourceKey& key)
{
	DEBUG_CHECK_RETURN_EX_V(key.className && key.id, "Invalid resource key", nullptr);

	ResourcePromisePtr ptr;
	if (m_resourcePromises.find(key, ptr))
		return ptr;

	ptr = ResourcePromise::CreateEmptyPromise(key);
	DEBUG_CHECK_RETURN_EX_V(ptr, "Failed to create promise", nullptr);

	if (m_context.resourcePromises)
		m_context.resourcePromises->pushBack(ptr);
	else
		ptr->fulfill(nullptr); // never going to be fullfilled

	m_resourcePromises[key] = ptr;
	return ptr;
}

//--

END_INFERNO_NAMESPACE();