/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
*
* [#filter: rtti\metadata #]
***/

#include "build.h"
#include "rttiClassType.h"
#include "rttiMetadata.h"
#include "rttiTypeSystem.h"
#include "rttiNativeClassType.h"

BEGIN_INFERNO_NAMESPACE()

//---

IMetadata::~IMetadata()
{}

void IMetadata::RegisterType(TypeSystem& typeSystem)
{
    Type classType = new NativeClass("IMetadata", sizeof(IMetadata), alignof(IMetadata), typeid(IMetadata).hash_code());
    //auto baseClassType = typeSystem.findClass("IObject"_id);
    //classType->baseClass(baseClassType);
    typeSystem.registerType(classType);
}

ClassType IMetadata::cls() const
{
    return GetStaticClass();
}

SpecificClassType<IMetadata> IMetadata::GetStaticClass()
{
    static ClassType objectType = RTTI::GetInstance().findClass("IMetadata"_id);
    return SpecificClassType<IMetadata>(*objectType.ptr());
}

//---

ShortTypeNameMetadata::~ShortTypeNameMetadata()
{}

ClassType ShortTypeNameMetadata::cls() const
{
    return GetStaticClass();
}

void ShortTypeNameMetadata::RegisterType(TypeSystem& typeSystem)
{
    auto classType = new NativeClass("ShortTypeNameMetadata", sizeof(ShortTypeNameMetadata), alignof(ShortTypeNameMetadata), typeid(ShortTypeNameMetadata).hash_code());
    classType->bindCtorDtor<ShortTypeNameMetadata>();
    auto baseClassType = typeSystem.findClass("IMetadata"_id);
    classType->baseClass(baseClassType.ptr());
    typeSystem.registerType(Type(classType));
}

SpecificClassType<ShortTypeNameMetadata> ShortTypeNameMetadata::GetStaticClass()
{
    static ClassType objectType = RTTI::GetInstance().findClass("ShortTypeNameMetadata"_id);
    return SpecificClassType<ShortTypeNameMetadata>(*objectType.ptr());
}

//---

ClassType TestOrderMetadata::cls() const
{
    return GetStaticClass();
}

void TestOrderMetadata::RegisterType(TypeSystem& typeSystem)
{
    auto classType = new NativeClass("TestOrderMetadata", sizeof(TestOrderMetadata), alignof(TestOrderMetadata), typeid(TestOrderMetadata).hash_code());
    classType->bindCtorDtor<TestOrderMetadata>();
    auto baseClassType = typeSystem.findClass("IMetadata"_id);
    classType->baseClass(baseClassType.ptr());
    typeSystem.registerType(Type(classType));
}

SpecificClassType<TestOrderMetadata> TestOrderMetadata::GetStaticClass()
{
    static ClassType objectType = RTTI::GetInstance().findClass("TestOrderMetadata"_id);
    return SpecificClassType<TestOrderMetadata>(*objectType.ptr());
}

//---

ClassType ClassVersionMetadata::cls() const
{
    return GetStaticClass();
}

void ClassVersionMetadata::RegisterType(TypeSystem& typeSystem)
{
    auto classType = new NativeClass("ClassVersionMetadata", sizeof(ClassVersionMetadata), alignof(ClassVersionMetadata), typeid(ClassVersionMetadata).hash_code());
    classType->bindCtorDtor<ClassVersionMetadata>();
    auto baseClassType = typeSystem.findClass("IMetadata"_id);
    classType->baseClass(baseClassType.ptr());
    typeSystem.registerType(Type(classType));
}

SpecificClassType<ClassVersionMetadata> ClassVersionMetadata::GetStaticClass()
{
    static ClassType objectType = RTTI::GetInstance().findClass("ClassVersionMetadata"_id);
    return SpecificClassType<ClassVersionMetadata>(*objectType.ptr());
}

//---

MetadataContainer::MetadataContainer()
{
}

MetadataContainer::~MetadataContainer()
{
    removeAllMetadata();
}

void MetadataContainer::removeAllMetadata()
{
    m_metadata.clearPtr(); // crashes :(
}

IMetadata& MetadataContainer::addMetadata(ClassType metadataType)
{
    for (auto ptr  : m_metadata)
        if (ptr->cls()->is(metadataType))
            return *ptr;

    DEBUG_CHECK_EX(!metadataType->isAbstract(), TempString("Cannot create metadata from abstract class '{}'", metadataType));

    auto metadata = metadataType->createPointer<IMetadata>();
    ASSERT(metadata->cls() == metadataType);
    m_metadata.pushBack(metadata);
    return *metadata;
}

void MetadataContainer::attachMetadata(IMetadata* metadata)
{
    ASSERT(metadata != nullptr);
    ASSERT(this->metadata(metadata->cls()) == nullptr);
    m_metadata.pushBack(metadata);
}

const IMetadata* MetadataContainer::metadata(ClassType metadataType) const
{
    for (auto ptr  : m_metadata)
        if (ptr->cls()->is(metadataType))
            return ptr;

    return nullptr;
}

void MetadataContainer::collectMetadataList(Array<const IMetadata*>& outMetadataList) const
{
    if (!m_metadata.empty())
    {
        outMetadataList.reserve(outMetadataList.size() + m_metadata.size());

        for (auto localMetadata : localMetadataList())
            outMetadataList.pushBack(localMetadata);
    }
}

//---

END_INFERNO_NAMESPACE()
