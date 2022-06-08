/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "rttiType.h"
#include "rttiArrayType.h"
#include "rttiClassType.h"
#include "rttiHandleType.h"
#include "rttiTypedMemory.h"
#include "rttiVariant.h"
#include "rttiVariantTable.h"
#include "rttiTextFormater.h"

#include "bm/core/containers/include/stringBuilder.h"
#include "bm/core/containers/include/refCounted.h"
#include "object.h"

BEGIN_INFERNO_NAMESPACE()

///---

void TypedMemoryResult::print(IFormatStream& f) const
{
	switch (status)
	{
		case TypedMemoryResultStatus::OK: f << "OK"; break;
		case TypedMemoryResultStatus::ErrorReadOnly: f << "ErrorReadOnly"; break;
		case TypedMemoryResultStatus::ErrorIndexOutOfRange: f << "ErrorIndexOutOfRange"; break;
		case TypedMemoryResultStatus::ErrorMissingChildren: f << "ErrorMissingChildren"; break;
		case TypedMemoryResultStatus::ErrorInvalidType: f << "ErrorInvalidType"; break;
		case TypedMemoryResultStatus::ErrorInvalidOperation: f << "ErrorInvalidOperation"; break;
		default: f << "UnknownStatus"; break;
	}
}

///---

bool TypedMemory::isVariant() const
{
	return m_type == GetTypeObject<Variant>();
}

TypedMemoryResult TypedMemory::variantChangeType(Type otherType)
{
	if (m_readOnly)
		return TypedMemoryResultStatus::ErrorReadOnly;

	if (!m_data)
		return TypedMemoryResultStatus::ErrorInvalidOperation;

	if (m_type != GetTypeObject<Variant>())
		return TypedMemoryResultStatus::ErrorInvalidType;

	auto* ptr = (Variant*)m_data;
	if (ptr->type() != otherType)
		ptr->reset(otherType);

	return TypedMemoryResultStatus::OK;
}

void* TypedMemory::unwrappedData() const
{
	if (m_type == GetTypeObject<Variant>())
	{
		const auto* var = (const Variant*)m_data;
		return (void*)var->data();
	}

	return m_data;
}

Type TypedMemory::unwrappedType() const
{
	if (m_type == GetTypeObject<Variant>())
	{
		const auto* var = (const Variant*)m_data;
		return var->type();
	}

	return m_type;
}

///--

bool TypedMemory::operator==(const TypedMemory& other) const
{
	const auto typeA = unwrappedType();
	const auto typeB = other.unwrappedType();

	if (typeA == typeB)
	{
		const auto* dataA = unwrappedData();
		const auto* dataB = other.unwrappedData();

		if (typeA)
			return typeA->compare(dataA, dataB);
		else
			return !!dataA == !!dataB;
	}

	return false;
}

bool TypedMemory::operator!=(const TypedMemory& other) const
{
	return !operator==(other);
}

///--

bool TypedMemory::isSimple() const
{
	const auto ut = unwrappedType();

	switch (ut.metaType())
	{
	case MetaType::Simple:
		return true;
	}

	return false;
}

bool TypedMemory::isPointer() const
{
	const auto ut = unwrappedType();

	switch (ut.metaType())
	{
	case MetaType::StrongHandle:
	case MetaType::WeakHandle:
		return true;
	}

	return false;
}

ClassType TypedMemory::pointedClass() const
{
	const auto ut = unwrappedType();
	return ut.referencedClass();
}

bool TypedMemory::isObject() const
{
	return pointedClass().is<IObject>();
}

void* TypedMemory::pointedObjectPtrRaw(ClassType expectedType) const
{
	if (const auto* data = unwrappedData())
	{
		const auto ut = unwrappedType();
		switch (ut.metaType())
		{
			case MetaType::StrongHandle:
			{
				const auto* handle = (const RefPtr<IReferencable>*)data;
				return handle->get();
			}

			case MetaType::WeakHandle:
			{
				const auto* handle = (const RefWeakPtr<IReferencable>*)data;
				return handle->unsafe();
			}
		}
	}

	return nullptr;
}

//--

bool TypedMemory::isArray() const
{
	const auto ut = unwrappedType();
	return ut.metaType() == MetaType::Array;
}

uint32_t TypedMemory::arraySize() const
{
	if (const auto* data = unwrappedData())
	{
		const auto ut = unwrappedType();
		if (ut.isArray())
		{
			const auto* arrayType = (const IArrayType*)ut.ptr();
			return arrayType->arraySize(m_data);
		}
	}

	return 0;
}

Type TypedMemory::arrayInnerType() const
{
	const auto ut = unwrappedType();
	if (ut.isArray())
		return ut.innerType();

	return nullptr;
}

TypedMemoryResult TypedMemory::arrayElement(uint32_t index) const
{
	auto* data = unwrappedData();
	if (!data)
		return TypedMemoryResultStatus::ErrorInvalidOperation;

	const auto ut = unwrappedType();
	if (!ut.isArray())
		return TypedMemoryResultStatus::ErrorInvalidType;

	const auto* arrayType = (const IArrayType*)ut.ptr();
	void* ptr = arrayType->arrayElementData(data, index);
	if (!ptr)
		return TypedMemoryResultStatus::ErrorIndexOutOfRange;

	return TypedMemory(arrayType->innerType(), ptr, m_readOnly);
}

TypedMemoryResult TypedMemory::arrayClear()
{
	if (m_readOnly)
		return TypedMemoryResultStatus::ErrorReadOnly;

	auto* data = unwrappedData();
	if (!data)
		return TypedMemoryResultStatus::ErrorInvalidOperation;

	const auto ut = unwrappedType();
	if (!ut.isArray())
		return TypedMemoryResultStatus::ErrorInvalidType;

	const auto* arrayType = (const IArrayType*)ut.ptr();
	if (!arrayType->canArrayBeResized())
		return TypedMemoryResultStatus::ErrorReadOnly;

	if (!arrayType->clearArrayElements(data))
		return TypedMemoryResultStatus::ErrorInvalidOperation;

	return TypedMemoryResultStatus::OK;
}

TypedMemoryResult TypedMemory::arrayRemove(uint32_t index)
{
	if (m_readOnly)
		return TypedMemoryResultStatus::ErrorReadOnly;

	auto* data = unwrappedData();
	if (!data)
		return TypedMemoryResultStatus::ErrorInvalidOperation;

	const auto ut = unwrappedType();
	if (!ut.isArray())
		return TypedMemoryResultStatus::ErrorInvalidType;

	const auto* arrayType = (const IArrayType*)ut.ptr();
	if (!arrayType->canArrayBeResized())
		return TypedMemoryResultStatus::ErrorReadOnly;

	if (index >= arrayType->arraySize(data))
		return TypedMemoryResultStatus::ErrorIndexOutOfRange;

	if (!arrayType->removeArrayElement(data, index))
		return TypedMemoryResultStatus::ErrorInvalidOperation;

	return TypedMemoryResultStatus::OK;
}

TypedMemoryResult TypedMemory::arrayResize(uint32_t size)
{
	if (m_readOnly)
		return TypedMemoryResultStatus::ErrorReadOnly;

	auto* data = unwrappedData();
	if (!data)
		return TypedMemoryResultStatus::ErrorInvalidOperation;

	const auto ut = unwrappedType();
	if (!ut.isArray())
		return TypedMemoryResultStatus::ErrorInvalidType;

	const auto* arrayType = (const IArrayType*)ut.ptr();
	if (!arrayType->canArrayBeResized())
		return TypedMemoryResultStatus::ErrorReadOnly;

	if (!arrayType->resizeArrayElements(data, size))
		return TypedMemoryResultStatus::ErrorInvalidOperation;

	return TypedMemoryResultStatus::OK;
}

TypedMemoryResult TypedMemory::arrayInsert(uint32_t index)
{
	if (m_readOnly)
		return TypedMemoryResultStatus::ErrorReadOnly;

	auto* data = unwrappedData();
	if (!data)
		return TypedMemoryResultStatus::ErrorInvalidOperation;

	const auto ut = unwrappedType();
	if (!ut.isArray())
		return TypedMemoryResultStatus::ErrorInvalidType;

	const auto* arrayType = (const IArrayType*)ut.ptr();
	if (!arrayType->canArrayBeResized())
		return TypedMemoryResultStatus::ErrorReadOnly;

	if (index > arrayType->arraySize(data))
		return TypedMemoryResultStatus::ErrorIndexOutOfRange;

	if (!arrayType->createArrayElement(data, index))
		return TypedMemoryResultStatus::ErrorInvalidOperation;

	return arrayElement(index);
}

TypedMemoryResult TypedMemory::arrayPushBack()
{
	if (m_readOnly)
		return TypedMemoryResultStatus::ErrorReadOnly;

	auto* data = unwrappedData();
	if (!data)
		return TypedMemoryResultStatus::ErrorInvalidOperation;

	const auto ut = unwrappedType();
	if (!ut.isArray())
		return TypedMemoryResultStatus::ErrorInvalidType;

	const auto* arrayType = (const IArrayType*)ut.ptr();
	if (!arrayType->canArrayBeResized())
		return TypedMemoryResultStatus::ErrorReadOnly;

	const auto index = arrayType->arraySize(data);
	if (!arrayType->createArrayElement(data, index))
		return TypedMemoryResultStatus::ErrorInvalidOperation;

	return arrayElement(index);
}

TypedMemoryResult TypedMemory::arrayIterate(const std::function<bool(uint32_t, TypedMemory)>& func) const
{
	auto* data = unwrappedData();
	if (!data)
		return TypedMemoryResultStatus::ErrorInvalidOperation;

	const auto ut = unwrappedType();
	if (!ut.isArray())
		return TypedMemoryResultStatus::ErrorInvalidType;

	const auto innerType = ut.innerType();
	if (!innerType)
		return TypedMemoryResultStatus::ErrorInvalidType;

	const auto* arrayType = (const IArrayType*)ut.ptr();
	const auto size = arrayType->arraySize(data);

	for (auto i : IndexRange(size))
	{
		void* ptr = arrayType->arrayElementData(data, i);

		auto elem = TypedMemory(innerType, ptr, m_readOnly);
		if (func(i, elem))
			return elem;
	}

	return TypedMemoryResultStatus::ErrorNotFound;
}

//--

bool TypedMemory::isDictionary() const
{
	const auto ut = unwrappedType();
	switch (ut.metaType())
	{
	case MetaType::StrongHandle:
	case MetaType::WeakHandle:
	case MetaType::Class:
		return true;
	}

	if (ut == GetTypeObject<VariantTable>())
		return true;

	return false;
}

bool TypedMemory::isDynamicDictionary() const
{
	const auto ut = unwrappedType();
	return !m_readOnly && (ut == GetTypeObject<VariantTable>());
}

Type TypedMemory::dictionaryType() const
{
	auto* data = unwrappedData();
	if (!data)
		return nullptr;

	const auto ut = unwrappedType();
	const auto mut = ut.metaType();
	if (mut == MetaType::Class)
		return ut;

	if (mut == MetaType::StrongHandle || mut == MetaType::WeakHandle)
		return ut.referencedClass();

	return nullptr;
}

bool TypedMemory::dictionaryHasElements() const
{
	auto* data = unwrappedData();
	if (!data)
		return false;

	const auto ut = unwrappedType();
	const auto mut = ut.metaType();
	if (mut == MetaType::Class)
		return true;

	if (mut == MetaType::StrongHandle || mut == MetaType::WeakHandle)
	{
		auto* innerData = pointedObjectPtrRaw(ut.referencedClass());
		return innerData != nullptr;
	}

	else if (ut == GetTypeObject<VariantTable>())
	{
		const auto* table = (VariantTable*)unwrappedData();
		return !table->empty();
	}

	return false;
}

TypedMemoryResult TypedMemory::dictionaryElement(StringID name) const
{
	auto* data = unwrappedData();
	if (!data)
		return TypedMemoryResultStatus::ErrorInvalidOperation;

	const auto ut = unwrappedType();
	const auto mut = ut.metaType();
	if (mut == MetaType::Class)
	{
		const auto cls = ut.referencedClass();
		if (!cls)
			return TypedMemoryResultStatus::ErrorInvalidOperation;

		return DictionaryElementClass(cls, data, m_readOnly, name);
	}

	else if (mut == MetaType::StrongHandle || mut == MetaType::WeakHandle)
	{
		const auto cls = ut.referencedClass();
		if (!cls)
			return TypedMemoryResultStatus::ErrorInvalidOperation;

		auto* innerData = pointedObjectPtrRaw(cls);
		if (!innerData)
			return TypedMemoryResultStatus::ErrorInvalidOperation;

		return DictionaryElementClass(cls, innerData, m_readOnly, name);
	}

	else if (ut == GetTypeObject<VariantTable>())
	{
		const auto* table = (VariantTable*)unwrappedData();
		return table->internalElementView(name, m_readOnly);
	}
	
	return TypedMemoryResultStatus::ErrorInvalidType;
}

TypedMemoryResult TypedMemory::dictionaryElement(StringView name) const
{
	const auto id = StringID::Find(name);
	return dictionaryElement(id);
}

TypedMemoryResult TypedMemory::dictionaryRemove(StringID name)
{
	return TypedMemoryResultStatus::ErrorReadOnly;
}

TypedMemoryResult TypedMemory::dictionaryRemove(StringView name)
{
	const auto id = StringID::Find(name);
	return dictionaryRemove(id);
}

TypedMemoryResult TypedMemory::dictionaryInsert(StringID name, Type type)
{
	return TypedMemoryResultStatus::ErrorReadOnly;
}

TypedMemoryResult TypedMemory::dictionaryInsert(StringView name, Type type)
{
	const auto id = StringID(name);
	return dictionaryInsert(id, type);
}

TypedMemoryResult TypedMemory::dictionaryIterate(const std::function<bool(StringID, TypedMemory)>& func) const
{
	auto* data = unwrappedData();
	if (!data)
		return TypedMemoryResultStatus::ErrorInvalidOperation;

	const auto ut = unwrappedType();
	const auto mut = ut.metaType();
	if (mut == MetaType::Class)
		return DictionaryIterateClass(ut.toClass(), data, m_readOnly, func);

	else if (mut == MetaType::StrongHandle || mut == MetaType::WeakHandle)
	{
		const auto cls = ut.referencedClass();
		if (!cls)
			return TypedMemoryResultStatus::ErrorInvalidOperation;

		auto* innerData = pointedObjectPtrRaw(cls);
		if (!innerData)
			return TypedMemoryResultStatus::ErrorInvalidOperation;

		return DictionaryIterateClass(cls, innerData, m_readOnly, func);
	}

	else if (ut == GetTypeObject<VariantTable>())
	{
		auto* table = (VariantTable*)unwrappedData();
		return DictionaryIterateTable(table, m_readOnly, func);
	}

	return TypedMemoryResultStatus::ErrorInvalidType;
}

//--

void TypedMemory::print(IFormatStream& f, DataTextFormatterFlags flags) const
{
	DataTextFormatter_JSON ctx(flags);
	format(f, ctx);
}

bool TypedMemory::format(IFormatStream& f, IDataTextFormatter& ctx) const
{
	// print array
	if (isArray())
	{
		const auto count = arraySize();
		ctx.beingArray(f);
		for (auto i : IndexRange(count))
		{
			ctx.beingArrayElement(f);

			const auto elem = arrayElement(i);
			if (!elem)
				return false;

			if (!elem.ptr.format(f, ctx))
				return false;
			
			ctx.endArrayElement(f);
		}
		ctx.endArray(f);
	}

	// print dictionary
	else if (isDictionary())
	{
		const auto type = dictionaryType();
		const auto name = type ? type->name().view() : "struct";

		const auto pointer = isPointer();
		const auto pointerClass = pointedClass();

		auto* ptr = pointedObjectPtrRaw(nullptr);
		if (pointer && !ptr)
		{
			ctx.printValue(f, "null");
		}
		else
		{
			ctx.beginStruct(f);

			if (pointer && pointerClass)
			{
				ctx.beginStructElement(f, "_class");
				ctx.printValue(f, pointerClass->name().view());
				ctx.endStructElement(f, "_class");
			}

			dictionaryIterate([&ctx, &f](StringID name, TypedMemory value)
				{
					ctx.beginStructElement(f, name.view());
					if (!value.format(f, ctx))
						return false;
					ctx.endStructElement(f, name.view());

					return false;
				});

			ctx.endStruct(f);
		}
	}

	// print simple value
	else
	{
		auto* data = unwrappedData();
		const auto ut = unwrappedType();
		if (!data || !ut)
			return false;

		StringBuilder txt;
		IType::PrintToString(ut, data, txt);
		ctx.beginValue(f, ut);
		ctx.printValue(f, txt.view());
		ctx.endValue(f, ut);
	}
	
	// done
	return true;
}

//--

bool TypedMemory::get(Type destType, void* destData) const
{
	if (auto* data = unwrappedData())
		if (const auto ut = unwrappedType())
			return ConvertData(data, ut, destData, destType);

	return false;
}

bool TypedMemory::set(Type srcType, const void* srcData) const
{
	if (auto* data = unwrappedData())
		if (const auto ut = unwrappedType())
			return ConvertData(srcData, srcType, data, ut);

	return false;
}

//--

StringBuf TypedMemory::toString(DataTextFormatterFlags flags) const
{
	DataTextFormatter_JSON ctx(flags);
	return toString(ctx);
}

StringBuf TypedMemory::toString(IDataTextFormatter& ctx) const
{
	StringBuilder txt;
	format(txt, ctx);
	return StringBuf(txt);
}

DataHolder TypedMemory::toDataHolder() const
{
	if (auto* data = unwrappedData())
		if (const auto ut = unwrappedType())
			return DataHolder(ut, data);

	return nullptr;
}

///---

TypedMemoryResult TypedMemory::DictionaryIterateClass(ClassType type, void* data, bool readOnly, const std::function<bool(StringID, TypedMemory)>& func)
{
	if (!type)
		return TypedMemoryResultStatus::ErrorInvalidType;

	for (const auto* prop : type->allProperties().values())
	{
		const auto* propData = prop->offsetPtr(data);
		if (!propData)
			return TypedMemoryResultStatus::ErrorInvalidOperation;

		const auto propView = TypedMemory(prop->type(), propData, prop->readonly() | readOnly);
		if (func(prop->name(), propView))
			return propView;
	}

	return TypedMemoryResultStatus::ErrorNotFound;
}

TypedMemoryResult TypedMemory::DictionaryIterateTable(VariantTable* table, bool readOnly, const std::function<bool(StringID, TypedMemory)>& func)
{
	if (!table)
		return TypedMemoryResultStatus::ErrorInvalidType;

	if (table->iterate(func, readOnly))
		return TypedMemoryResultStatus::OK;

	return TypedMemoryResultStatus::ErrorNotFound;
}

//--

TypedMemoryResult TypedMemory::DictionaryElementClass(ClassType type, void* data, bool readOnly, StringID name)
{
	if (!type)
		return TypedMemoryResultStatus::ErrorInvalidType;

	const auto* prop = type->findProperty(name);
	if (!prop)
		return TypedMemoryResultStatus::ErrorMissingChildren;

	const auto* propData = prop->offsetPtr(data);
	if (!propData)
		return TypedMemoryResultStatus::ErrorInvalidOperation;

	return TypedMemory(prop->type(), propData, prop->readonly() | readOnly);
}

//---

END_INFERNO_NAMESPACE()
