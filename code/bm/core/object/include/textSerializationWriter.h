/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

BEGIN_INFERNO_NAMESPACE()

///---

/// helper class for nice printing of data structures to text
class BM_CORE_OBJECT_API ITextSerializationWriter : public NoCopy
{
public:
	virtual ~ITextSerializationWriter();

	//--

	virtual void beingArray() = 0;
	virtual void endArray() = 0;
	virtual void beingArrayElement() = 0;
	virtual void endArrayElement() = 0;

	virtual void beginStruct(Type type, const void* ptr) = 0;
	virtual void endStruct(Type type, const void* ptr) = 0;
	virtual void beginStructElement(StringView name) = 0;
	virtual void endStructElement(StringView name) = 0;

	virtual void writeValueText(StringView value) = 0;
	virtual void writeValueBytes(BufferView view) = 0;
	virtual void writeValueOwnedBytes(Buffer view) = 0;
	virtual void writeValueObject(const IObject* obj, bool strong) = 0;
	virtual void writeValueResource(const BaseReference& rr) = 0;

	//--
};

///---

END_INFERNO_NAMESPACE()
