/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

#include "textSerializationWriter.h"

BEGIN_INFERNO_NAMESPACE()

///---

struct TextSerializationMappedObjects
{
	TextSerializationMappedObjects();
	~TextSerializationMappedObjects();

	struct ObjectInfo
	{
		uint32_t numStrongRefs = 0;
		uint32_t numWeakRefs = 0;
		int assignedIndex = -1; // index allocated on on second reference
		ObjectPtr ptr;
	};

	HashMap<const IObject*, ObjectInfo> objects;

	HashSet<SerializationResourceKey>* extractedResources = nullptr;
};

///---

struct TextSerializationObjectTable
{
	TextSerializationObjectTable();
	~TextSerializationObjectTable();

	HashMap<uint32_t, ObjectPtr> objects;
};

///---

/// helper class for mapping used objects
class TextSerializationObjectMapper : public ITextSerializationWriter
{
public:
	TextSerializationObjectMapper(TextSerializationMappedObjects& outObjectMap);

	//--

	virtual void beingArray() override final {};
	virtual void endArray() override final {};
	virtual void beingArrayElement() override final {};
	virtual void endArrayElement() override final {};

	virtual void beginStruct(Type type, const void* ptr) override final {};
	virtual void endStruct(Type type, const void* ptr) override final {};
	virtual void beginStructElement(StringView name) override final {};
	virtual void endStructElement(StringView name) override final {};

	virtual void writeValueText(StringView value) override final {};
	virtual void writeValueBytes(BufferView view) override final {};
	virtual void writeValueOwnedBytes(Buffer view) override final {};
	virtual void writeValueObject(const IObject* obj, bool strong) override final;
	virtual void writeValueResource(const BaseReference& rr) override final;

	//--

private:
	TextSerializationMappedObjects& m_objectMap;
	int m_nextObjectIndex = 1;
};

///---

END_INFERNO_NAMESPACE()
