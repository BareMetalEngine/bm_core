/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

BEGIN_INFERNO_NAMESPACE()

///---

class BaseReference;

///---

/// helper class for nice printing of data structures to text
class BM_CORE_OBJECT_API ITextSerializationReader : public NoCopy
{
public:
	virtual ~ITextSerializationReader();

	//--

	virtual bool beingArray(uint32_t& outCount) = 0;
	virtual void endArray() = 0;

	virtual bool beingArrayElement() = 0;
	virtual void endArrayElement() = 0;

	virtual bool beginStruct() = 0;
	virtual void endStruct() = 0;

	virtual bool beginStructElement(StringView& outName) = 0;
	virtual void endStructElement() = 0;

	virtual bool readValueText(StringView& outValue) = 0;
	virtual bool readValueBytes(Buffer& outData) = 0;
	virtual bool readValueObject(ObjectPtr& outObject) = 0;
	virtual bool readValueResource(BaseReference& outRef) = 0;

	virtual void reportError(StringView txt) = 0;

	//--
};

///---

END_INFERNO_NAMESPACE()
