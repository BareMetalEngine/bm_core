/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

#include "textSerializationReader.h"

BEGIN_INFERNO_NAMESPACE()

///---

/// helper class for nice printing of data structures to text
class BM_CORE_OBJECT_API TextSerializationReaderXML : public ITextSerializationReader
{
public:
	TextSerializationReaderXML(const XMLReader* doc, const ObjectLoadingContext& context, ITextErrorReporter& err);
	~TextSerializationReaderXML();

	//--

	virtual bool beingArray(uint32_t& outCount) override final;
	virtual void endArray() override final;

	virtual bool beingArrayElement() override final;
	virtual void endArrayElement() override final;

	virtual bool beginStruct() override final;
	virtual void endStruct() override final;

	virtual bool beginStructElement(StringView& outName) override final;
	virtual void endStructElement() override final;

	virtual bool readValueText(StringView& outValue) override final;
	virtual bool readValueBytes(Buffer& outData) override final;
	virtual bool readValueObject(ObjectPtr& outObject) override final;
	virtual bool readValueResource(BaseReference& outRef) override final;

	virtual void reportError(StringView txt) override final;

	//--

private:
	const XMLReader* m_doc = nullptr;
	const ObjectLoadingContext& m_context;
	ITextErrorReporter& m_err;

	InplaceArray<NodeHandle, 32> m_stack;
	HashMap<uint32_t, ObjectPtr> m_definedObjects;
	HashMap<SerializationResourceKey, ResourcePromisePtr> m_resourcePromises;

	ResourcePromisePtr createResourcePromise(const SerializationResourceKey& key);
};

///---

END_INFERNO_NAMESPACE()
