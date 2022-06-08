/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
*
* [#filter: rtti\types #]
***/

#include "build.h"
#include "rttiType.h"
#include "rttiDataView.h"
#include "rttiArrayType.h"
#include "serializationWriter.h"
#include "rttiDataHolder.h"
#include "rttiClassType.h"
#include "dataView.h"

#include "bm/core/containers/include/stringBuilder.h"

BEGIN_INFERNO_NAMESPACE()

///---

IType::IType(StringID name)
    : m_name(name)
{}

IType::~IType()
{}

void IType::cacheTypeData()
{}

void IType::releaseTypeReferences()
{}

//--

DataViewResult IType::describeDataView(StringView viewPath, const void* viewData, DataViewInfo& outInfo) const
{
    if (viewPath.empty())
    {
        outInfo.dataPtr = viewData;
        outInfo.dataType = this; // that's all we know :D

        if (outInfo.requestFlags.test(DataViewRequestFlagBit::TypeMetadata))
            collectMetadataList(outInfo.typeMetadata);

        outInfo.flags |= DataViewInfoFlagBit::LikeValue;
        return DataViewResultCode::OK;
    }

    StringView propertyName;
    if (ParsePropertyName(viewPath, propertyName))
        return DataViewResultCode::ErrorUnknownProperty;

    return DataViewResultCode::ErrorIllegalAccess;
}

DataViewResult IType::readDataView(StringView viewPath, const void* viewData, void* targetData, Type targetType) const
{
    if (viewPath.empty())
    {
        if (!ConvertData(viewData, this, targetData, targetType))
            return DataViewResultCode::ErrorTypeConversion;
        return DataViewResultCode::OK;
    }

    StringView propertyName;
    if (ParsePropertyName(viewPath, propertyName))
    {
        if (propertyName == "__type")
        {
                // TODO
        }
        else if (propertyName == "__text")
        {
            StringBuf text;
            {
                StringBuilder f;
                IType::PrintToString(this, viewData, f);
                text = StringBuf(f);
            }

            static const auto textType = RTTI::GetInstance().findType("StringBuf"_id);

            if (!ConvertData(&text, textType, targetData, targetType))
                return DataViewResultCode::ErrorTypeConversion;

            return DataViewResultCode::OK;
                    
        }

        return DataViewResultCode::ErrorUnknownProperty;
    }

    return DataViewResultCode::ErrorIllegalAccess;
}

DataViewResult IType::writeDataView(StringView viewPath, void* viewData, const void* sourceData, Type sourceType) const
{
    if (viewPath.empty())
    {
        if (!ConvertData(sourceData, sourceType, viewData, this))
            return DataViewResultCode::ErrorTypeConversion;
        return DataViewResultCode::OK;
    }

    StringView propertyName;
    if (ParsePropertyName(viewPath, propertyName))
    {
        return DataViewResultCode::ErrorUnknownProperty;
    }

    return DataViewResultCode::ErrorIllegalAccess;
}

///---

StringID TypeSerializationContext::classContextName() const
{
    if (classContext)
        return classContext->name();
    else
        return StringID();
}

StringID TypeSerializationContext::propertyContextName() const
{
	if (propertyContext)
		return propertyContext->name();
	else
		return StringID();
}

StringID TypeSerializationContext::propertyContextType() const
{
	if (propertyContext)
		return propertyContext->type()->name();
	else
		return StringID();
}

void TypeSerializationContext::print(IFormatStream& f) const
{
    bool written = false;

    if (directObjectContext)
    {
        f.appendf("object {}", *directObjectContext);
        written = true;
    }

    if (classContext)
    {
        if (written) f.append(", ");
        f.appendf("class '{}'", classContext->name());
        written = true;
    }

    if (propertyContext)
    {
        if (written) f.append(", ");
        f.appendf("property '{}'", propertyContext->name());
    }
}

///---

END_INFERNO_NAMESPACE()
