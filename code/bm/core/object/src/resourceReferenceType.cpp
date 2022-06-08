/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "resource.h"
#include "resourcePromise.h"
#include "resourceReference.h"
#include "resourceReferenceType.h"
#include "serializationReader.h"
#include "serializationWriter.h"
#include "textSerializationWriter.h"
#include "textSerializationReader.h"

#include "bm/core/containers/include/stringBuilder.h"
#include "bm/core/containers/include/stringParser.h"
#include "bm/core/object/include/rttiProperty.h"
#include "bm/core/object/include/rttiType.h"

BEGIN_INFERNO_NAMESPACE()

//--

const char* ResourceRefType::TypePrefix = "ref<";


StringID FormatRefTypeName(StringID className)
{
    StringBuilder builder;
    builder.append(ResourceRefType::TypePrefix);
    builder.append(className.c_str());
    builder.append(">");
    return StringID(builder.c_str());
}

//--

ResourceRefType::ResourceRefType(SpecificClassType<IResource> classType)
    : IResourceReferenceType(FormatRefTypeName(classType->name()))
    , m_resourceClass(classType)
{
    m_traits.metaType = MetaType::ResourceRef;
    m_traits.convClass = TypeConversionClass::TypeResRef;
    m_traits.alignment = alignof(BaseReference);
    m_traits.size = sizeof(BaseReference);
    m_traits.initializedFromZeroMem = true;
    m_traits.requiresConstructor = true;
    m_traits.requiresDestructor = true;
    m_traits.simpleCopyCompare = true;
    m_traits.hashable = true;
}

ResourceRefType::~ResourceRefType()
{
}

ClassType ResourceRefType::referenceResourceClass() const
{
    return m_resourceClass;
}

bool ResourceRefType::referencePatchResource(void* data, IResource* currentResource, IResource* newResource) const
{
    /*auto& ptr1 = *(BaseReference*)data;
    if (ptr1.resource() != currentResource) // most common case
        return false;

    ptr1 = BaseReference(ptr1.id(), newResource);*/
    return true;
}

void ResourceRefType::readResourceRef(const void* data, BaseReference& outResRef) const
{
    outResRef = *(const BaseReference*)data;
}

void ResourceRefType::writeResourceRef(void* data, const BaseReference& resRef) const
{
    *(BaseReference*)data = resRef;
}

bool ResourceRefType::compare(const void* data1, const void* data2) const
{
    auto& ptr1 = *(const BaseReference*) data1;
    auto& ptr2 = *(const BaseReference*) data2;
    return ptr1 == ptr2;
}

void ResourceRefType::copy(void* dest, const void* src) const
{
    auto& ptrSrc = *(const BaseReference*) src;
    auto& ptrDest = *(BaseReference*) dest;
    ptrDest = ptrSrc;
}

void ResourceRefType::construct(void* object) const
{
    new (object) BaseReference();
}

void ResourceRefType::destruct(void* object) const
{
    ((BaseReference*)object)->~BaseReference();
}

enum ResourceRefBinaryFlag
{
    External = 1,
    Inlined = 2,
};

void ResourceRefType::writeBinary(TypeSerializationContext& typeContext, SerializationWriter& file, const void* data, const void* defaultData) const
{
    auto& ptr = *(const BaseReference*) data;

    uint8_t mask = 0;
    if (ptr.external())
        mask |= External;
	if (ptr.inlined())
		mask |= Inlined;

	file.writeTypedData(mask);

    if (mask & External)
    {
		SerializationResourceKey key;
        key.id = ptr.resolveId().guid();
        key.className = ptr.resolveClass().name();
        file.writeResourceReference(key);
    }

    if (mask & Inlined)
    {
        file.writePointer(ptr.resolveResource(), true);
    }
}

void ResourceRefType::readBinary(TypeSerializationContext& typeContext, SerializationReader& file, void* data) const
{
    BaseReference loadedRef;

    uint8_t mask = 0;
    file.readTypedData(mask);

    if (mask & External)
    {
        if (auto promise = file.readResource())
        {
            if (promise->cls().is(m_resourceClass))
                loadedRef = BaseReference(promise);
        }
    }

	if (mask & Inlined)
	{
        if (auto pointer = file.readPointer())
        {
            if (pointer->cls().is(m_resourceClass))
                loadedRef = BaseReference((const IResource*)pointer);
        }
	}

    *(BaseReference*)data = loadedRef;
}

//--

void ResourceRefType::writeText(TypeSerializationContext& typeContext, ITextSerializationWriter& file, const void* data, const void* defaultData) const
{
    const auto& ptr = *(const BaseReference*)data;
    file.writeValueResource(ptr);
}

void ResourceRefType::readText(TypeSerializationContext& typeContext, ITextSerializationReader& file, void* data) const
{
	auto& ptr = *(BaseReference*)data;
	file.readValueResource(ptr);
}

//--

Type ResourceRefType::ParseType(StringParser& typeNameString, TypeSystem& typeSystem)
{
    StringID innerTypeName;
    if (!typeNameString.parseTypeName(innerTypeName))
        return nullptr;

    if (!typeNameString.parseKeyword(">"))
        return nullptr;

    auto classType = typeSystem.findType(innerTypeName);
    if (!classType)
    {
        TRACE_ERROR("Unable to parse a resource reference type from '{}'", innerTypeName);
        return nullptr;
    }

    return new ResourceRefType((const SpecificClassType<IResource>&) classType);
}

//---

const ResourceRefType* CreateRefType(SpecificClassType<IResource> resourceClass)
{
    DEBUG_CHECK(resourceClass && resourceClass->is<IResource>());
    if (!resourceClass || !resourceClass->is<IResource>())
        return nullptr;

    const auto typeName = FormatRefTypeName(resourceClass->name());
    const auto type = RTTI::GetInstance().findType(typeName);
    ASSERT(type && type->metaType() == MetaType::ResourceRef);

    return static_cast<const ResourceRefType*>(type.ptr());
}

//--

END_INFERNO_NAMESPACE()
