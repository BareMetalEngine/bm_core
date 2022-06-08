/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
*
* [#filter: rtti\types\arrays #]
***/

#include "build.h"
#include "rttiArrayType.h"
#include "rttiNativeArrayType.h"
#include "rttiTypeSystem.h"

#include "bm/core/containers/include/stringBuilder.h"
#include "bm/core/containers/include/stringParser.h"

BEGIN_INFERNO_NAMESPACE()

namespace prv
{

    const char* NativeArrayType::TypePrefix = "[";

    NativeArrayType::NativeArrayType(Type innerType, uint32_t size)
        : IArrayType(FormatNativeArrayTypeName(innerType->name(), size), innerType)
        , m_elementCount(size)
    {
        m_traits.alignment = innerType->alignment();

        auto innerSize = innerType->size();
        auto innerAlignment = innerType->alignment();
        auto innerSeparation = std::max<uint32_t>(innerSize, innerAlignment);
        m_traits.size = innerSeparation * m_elementCount;

        m_traits.simpleCopyCompare = innerType->traits().simpleCopyCompare;
        m_traits.requiresDestructor = innerType->traits().requiresDestructor;
        m_traits.requiresConstructor = innerType->traits().requiresConstructor;
        m_traits.initializedFromZeroMem = innerType->traits().initializedFromZeroMem;
    }

    NativeArrayType::~NativeArrayType()
    {}

    void NativeArrayType::construct(void *object) const
    {
        for (uint32_t i = 0; i < m_elementCount; ++i)
        {
            auto ptr  = arrayElementData(object, i);
            innerType()->construct(ptr);
        }
    }

    void NativeArrayType::destruct(void *object) const
    {
        for (uint32_t i = 0; i < m_elementCount; ++i)
        {
            auto ptr  = arrayElementData(object, i);
            innerType()->destruct(ptr);
        }
    }

    ArrayMetaType NativeArrayType::arrayMetaType() const
    {
        return ArrayMetaType::Native;
    }

    uint32_t NativeArrayType::arraySize(const void* data) const
    {
        return m_elementCount;
    }

    uint32_t NativeArrayType::arrayCapacity(const void* data) const
    {
        return m_elementCount;
    }

	uint32_t NativeArrayType::maxArrayCapacity(const void* data) const
	{
		return m_elementCount;
	}

    bool NativeArrayType::canArrayBeResized() const
    {
        return false;
    }

    bool NativeArrayType::clearArrayElements(void* data) const
    {
        return true;
    }

    bool NativeArrayType::resizeArrayElements(void* data, uint32_t count) const
    {
        return (count == m_elementCount);
    }

    bool NativeArrayType::removeArrayElement(const void* data, uint32_t index) const
    {
        return true;
    }

    bool NativeArrayType::createArrayElement(void* data, uint32_t index) const
    {
        return true;
    }

    const void* NativeArrayType::arrayElementData(const void* data, uint32_t index) const
    {
        if (index >= m_elementCount)
            return nullptr;

        auto elemSize = innerType()->size();
        auto elemAlign = innerType()->alignment();
        auto elemSeparation = std::max<uint32_t>(elemSize, elemAlign);

        return OffsetPtr<void>(data, index * elemSeparation);
    }

    void* NativeArrayType::arrayElementData(void* data, uint32_t index) const
    {
        if (index >= m_elementCount)
            return nullptr;

        auto elemSize = innerType()->size();
        auto elemAlign = innerType()->alignment();
        auto elemSeparation = std::max<uint32_t>(elemSize, elemAlign);

        return OffsetPtr<void>(data, index * elemSeparation);
    }

    //---

    Type NativeArrayType::ParseType(StringParser& typeNameString, TypeSystem& typeSystem)
    {
        /*if (!typeNameString.parseKeyword("["))
            return nullptr;*/

        uint32_t maxSize = 0;
        if (!typeNameString.parseUint32(maxSize))
            return nullptr;

        if (!typeNameString.parseKeyword("]"))
            return nullptr;

        StringID innerTypeName;
        if (!typeNameString.parseTypeName(innerTypeName))
            return nullptr;

        if (auto innerType = typeSystem.findType(innerTypeName))
            return new NativeArrayType(innerType, maxSize);

        TRACE_ERROR("Unable to parse a array inner type from '{}'", innerTypeName);
        return nullptr;
    }

    //---

} // prv

extern StringID FormatNativeArrayTypeName(StringID innerTypeName, uint32_t maxSize)
{
    StringBuilder builder;
    builder.appendf("[{}]", maxSize);
    builder.append(innerTypeName.c_str());
    return StringID(builder.c_str());
}

END_INFERNO_NAMESPACE()
