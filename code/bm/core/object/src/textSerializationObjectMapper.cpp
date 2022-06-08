/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"

#include "textSerializationObjectMapper.h"
#include "serializationStream.h"
#include "resourceReference.h"
#include "resource.h"

BEGIN_INFERNO_NAMESPACE()

//--

TextSerializationMappedObjects::TextSerializationMappedObjects()
{
	objects.reserve(16);
}

TextSerializationMappedObjects::~TextSerializationMappedObjects()
{}

//--

TextSerializationObjectTable::TextSerializationObjectTable()
{}

TextSerializationObjectTable::~TextSerializationObjectTable()
{}

//--

TextSerializationObjectMapper::TextSerializationObjectMapper(TextSerializationMappedObjects& outObjectMap)
	: m_objectMap(outObjectMap)
{
}

void TextSerializationObjectMapper::writeValueObject(const IObject* obj, bool strong)
{
	if (obj)
	{
		bool known = m_objectMap.objects.contains(obj);

		auto& info = m_objectMap.objects[obj];
		info.ptr = AddRef(obj);

		if (strong)
			info.numStrongRefs += 1;
		else
			info.numWeakRefs += 1;

		if (info.numStrongRefs && (info.numStrongRefs + info.numWeakRefs > 1) && (info.assignedIndex == -1))
			info.assignedIndex = m_nextObjectIndex++;

		if (!known)
			obj->writeText(*this);
	}
}

void TextSerializationObjectMapper::writeValueResource(const BaseReference& rr)
{
	if (rr.inlined())
	{
		writeValueObject(rr.resolveResource(), true);
	}
	else
	{
		if (m_objectMap.extractedResources)
		{
			SerializationResourceKey key;
			key.id = rr.resolveId().guid();
			key.className = rr.resolveClass().name();
			m_objectMap.extractedResources->insert(key);
		}
	}
}

//--

END_INFERNO_NAMESPACE()
