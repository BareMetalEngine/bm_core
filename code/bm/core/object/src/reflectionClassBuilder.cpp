/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
*
* [#filter: reflection #]
***/

#include "build.h"
#include "reflectionClassBuilder.h"
#include "reflectionPropertyBuilder.h"
#include "rttiClassType.h"
#include "rttiMetadata.h"

BEGIN_INFERNO_NAMESPACE()

//--

TypeTraitBuilder::TypeTraitBuilder()
    : m_traitZeroInit(false)
    , m_traitNoConstructor(false)
    , m_traitNoDestructor(false)
    , m_traitFastCopyCompare(false)
{}

void TypeTraitBuilder::apply(TypeRuntimeTraits& traits)
{
    if (m_traitZeroInit)
        traits.initializedFromZeroMem = true;
    if (m_traitFastCopyCompare)
        traits.simpleCopyCompare = true;
    if (m_traitNoConstructor)
        traits.requiresConstructor = false;
    if (m_traitNoDestructor)
        traits.requiresDestructor = false;
}

//--

ClassBuilder::ClassBuilder(NativeClass* classPtr)
    : m_classPtr(classPtr)
    , m_categoryName("Generic")
{
}

ClassBuilder::~ClassBuilder()
{
}

void ClassBuilder::category(const char* category)
{
    m_categoryName = StringBuf(category);
}

void ClassBuilder::submit()
{
    for (auto& prop : m_properties)
        prop.submit(m_classPtr);

    for (const auto& func : m_nativeFunctions)
    {
        auto* nativeFunc = new NativeFunction(m_classPtr, func.rawName, func.sig, func.ptr, func.wrapPtr);
        m_classPtr->addNativeFunction(nativeFunc);
    }

    m_traits.apply(m_classPtr->traits());

    for (auto& oldName : m_oldNames)
        RTTI::GetInstance().registerAlternativeTypeName(m_classPtr, oldName);
}

void ClassBuilder::addOldName(const char* oldName)
{
    m_oldNames.pushBackUnique(StringID(oldName));
}

PropertyBuilder& ClassBuilder::addProperty(const char* rawName, Type type, uint32_t dataOffset)
{
    while (*rawName == '_')
        ++rawName;

    if (type == nullptr)
    {
        static PropertyBuilder theNullProperty(nullptr, "Null", "Null", 0);
        return theNullProperty;
    }

    if (0 == strncmp(rawName, "m_", 2))
        rawName += 2;

    m_properties.emplaceBack(type, rawName, m_categoryName.c_str(), dataOffset);
    return m_properties.back();
}

void ClassBuilder::addNativeFunction(const char* rawName, const FunctionSignature& sig, const FunctionPointer& ptr, TFunctionWrapperPtr wrapPtr)
{
    const auto id = StringID(rawName);

    for (auto& entry : m_nativeFunctions)
    {
        if (entry.rawName == id)
        {
            entry.ptr = ptr;
            entry.sig = sig;
            entry.wrapPtr = wrapPtr;
            return;
        }
    }

    auto& entry = m_nativeFunctions.emplaceBack();
    entry.rawName = id;
    entry.ptr = ptr;
    entry.sig = sig;
    entry.wrapPtr = wrapPtr;
}

IMetadata& ClassBuilder::addMetadata(ClassType classType)
{
    return m_classPtr->addMetadata(classType);
}

//--

CustomTypeBuilder::CustomTypeBuilder(CustomType* customType)
    : m_type(customType)
{}

CustomTypeBuilder::~CustomTypeBuilder()
{}

void CustomTypeBuilder::submit()
{
    m_traits.apply(m_type->traits());
}

//--

END_INFERNO_NAMESPACE()
