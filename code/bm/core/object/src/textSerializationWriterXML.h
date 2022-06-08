/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

#include "textSerializationWriter.h"

BEGIN_INFERNO_NAMESPACE()

///---

struct TextSerializationMappedObjects;

///---

/// helper class for nice printing of data structures to text
class BM_CORE_OBJECT_API TextSerializationWriterXML : public ITextSerializationWriter
{
public:
	TextSerializationWriterXML(const TextSerializationMappedObjects& objects, const ObjectSavingContext& ctx, XMLWriter* doc);
	~TextSerializationWriterXML();

	//--

	virtual void beingArray() override;
	virtual void endArray() override;
	virtual void beingArrayElement() override;
	virtual void endArrayElement() override;

	virtual void beginStruct(Type type, const void* ptr) override;
	virtual void endStruct(Type type, const void* ptr) override;
	virtual void beginStructElement(StringView name) override;
	virtual void endStructElement(StringView name) override;

	virtual void writeValueText(StringView value) override;
	virtual void writeValueBytes(BufferView view) override;
	virtual void writeValueOwnedBytes(Buffer view) override;
	virtual void writeValueObject(const IObject* obj, bool strong) override;
	virtual void writeValueResource(const BaseReference& rr) override;

	//--

private:
	const TextSerializationMappedObjects& m_objects;
	const ObjectSavingContext& m_context;
	XMLWriterPtr m_doc;

	InplaceArray<NodeHandle, 32> m_stack;
	HashSet<const IObject*> m_definedObjects;
};

///---

END_INFERNO_NAMESPACE()
