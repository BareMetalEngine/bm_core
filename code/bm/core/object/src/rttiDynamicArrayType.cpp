/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
*
* [#filter: rtti\types\arrays #]
***/

#include "build.h"
#include "rttiArrayType.h"
#include "rttiDynamicArrayType.h"
#include "rttiTypeSystem.h"
#include "rttiArrayHelpers.h"
#include "rttiDataView.h"

#include "bm/core/containers/include/stringBuilder.h"
#include "bm/core/containers/include/stringParser.h"

BEGIN_INFERNO_NAMESPACE()

extern bool SkipString(const char*& ptr, const char* endPtr);

namespace prv
{

    const char* DynamicArrayType::TypePrefix = "array<";

    DynamicArrayType::DynamicArrayType(Type innerType)
        : IArrayType(FormatDynamicArrayTypeName(innerType->name()), innerType)
		, m_helper(nullptr)
    {
        m_traits.size = sizeof(BaseArray);
        m_traits.alignment = __alignof(BaseArray);
        m_traits.initializedFromZeroMem = true; // BaseArray is fine with zero memory init
        m_traits.requiresConstructor = true;
        m_traits.requiresDestructor = true;
    }

    DynamicArrayType::~DynamicArrayType()
    {}

    void DynamicArrayType::cacheTypeData()
    {
        IArrayType::cacheTypeData();

        ASSERT(innerType());
        ASSERT(innerType()->size() != 0);
        ASSERT(innerType()->alignment() != 0);

        m_helper = IArrayHelper::GetHelperForType(innerType());
    }

    void DynamicArrayType::construct(void *object) const
    {
        new (object) BaseArray();
    }

    DataViewResult DynamicArrayType::describeDataView(StringView viewPath, const void* viewData, DataViewInfo& outInfo) const
    {
        if (viewPath.empty())
            outInfo.flags |= DataViewInfoFlagBit::DynamicArray;

        return IArrayType::describeDataView(viewPath, viewData, outInfo);
    }

    DataViewResult DynamicArrayType::writeDataView(StringView viewPath, void* viewData, const void* sourceData, Type sourceType) const
    {
        return IArrayType::writeDataView(viewPath, viewData, sourceData, sourceType);
    }

    void DynamicArrayType::destruct(void *object) const
    {
        auto arr  = (BaseArray*)object;

        m_helper->destroy(arr, innerType());

        clearArrayElements(object);
    }
            
    ArrayMetaType DynamicArrayType::arrayMetaType() const
    {
        return ArrayMetaType::Dynamic;
    }

    uint32_t DynamicArrayType::arraySize(const void* data) const
    {
        auto arr  = (const BaseArray*)data;
        return arr->size();
    }

    uint32_t DynamicArrayType::arrayCapacity(const void* data) const
    {
		auto arr  = (const BaseArray*)data;
		return arr->capacity();
    }

	uint32_t DynamicArrayType::maxArrayCapacity(const void* data) const
	{
		return INDEX_MAX;
	}

    bool DynamicArrayType::canArrayBeResized() const
    {
        return true;
    }

    bool DynamicArrayType::clearArrayElements(void* data) const
    {
        auto arr  = (BaseArray*)data;
        m_helper->destroy(arr, innerType());
        return true;
    }

    bool DynamicArrayType::resizeArrayElements(void* data, uint32_t count) const
    {
        auto arr = (BaseArray*)data;
        m_helper->resize(arr, innerType(), count);
        return true;
    }

    bool DynamicArrayType::removeArrayElement(const void* data, uint32_t index) const
    {
        auto arr  = (BaseArray*)data;
        auto size = arr->size();
        DEBUG_CHECK_RETURN_EX_V(index < size, "Invalid array index", false);

        m_helper->eraseRange(arr, innerType(), index, 1);
        return true;
    }

    bool DynamicArrayType::createArrayElement(void* data, uint32_t index) const
    {
        auto arr  = (BaseArray*)data;
        auto size = arr->size();
        DEBUG_CHECK_RETURN_EX_V(index <= size, "Invalid array index", false);

        m_helper->insertRange(arr, innerType(), index, 1);
        return true;
    }

    const void* DynamicArrayType::arrayElementData(const void* data, uint32_t index) const
    {
        auto arr  = (const BaseArray*)data;

        auto size = arr->size();
        DEBUG_CHECK_RETURN_EX_V(index < size, "Invalid array index", nullptr);

        auto elemSize = innerType()->size();
        return (const uint8_t*)arr->data() + (index * elemSize);
    }

    void* DynamicArrayType::arrayElementData(void* data, uint32_t index) const
    {
        auto arr  = (BaseArray*)data;

        auto size = arr->size();
        DEBUG_CHECK_RETURN_EX_V(index < size, "Invalid array index", nullptr);

        auto elemSize = innerType()->size();
        return (uint8_t*)arr->data() + (index * elemSize);
    }

    //---

    Type DynamicArrayType::ParseType(StringParser& typeNameString, TypeSystem& typeSystem)
    {
        StringID innerTypeName;
        if (!typeNameString.parseTypeName(innerTypeName))
            return nullptr;

        if (!typeNameString.parseKeyword(">"))
            return nullptr;

        if (auto innerType = typeSystem.findType(innerTypeName))
            return new DynamicArrayType(innerType);

        TRACE_ERROR("Unable to parse a array inner type from '{}'", innerTypeName);
        return nullptr;
    }

    //---

} // prv

extern StringID FormatDynamicArrayTypeName(StringID innerTypeName)
{
    StringBuilder builder;
    builder.append(prv::DynamicArrayType::TypePrefix);
    builder.append(innerTypeName.c_str());
    builder.append(">");
    return StringID(builder.c_str());
}

END_INFERNO_NAMESPACE()
