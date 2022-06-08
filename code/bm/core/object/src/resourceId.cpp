/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "resourceId.h"
#include "textSerializationWriter.h"
#include "textSerializationReader.h"

#include "bm/core/object/include/serializationWriter.h"
#include "bm/core/object/include/serializationReader.h"

BEGIN_INFERNO_NAMESPACE()

namespace prv
{

    void WriteResourceIDBinary(TypeSerializationContext& typeContext, SerializationWriter& stream, const void* data, const void* defaultData)
    {
        const auto& id = *(const ResourceID*)data;
        stream.writeTypedData<GUID>(id.guid());
    }

    void ReadResourceIDBinary(TypeSerializationContext& typeContext, SerializationReader& stream, void* data)
    {
        GUID guid;
        stream.readTypedData<GUID>(guid);

        auto& id = *(ResourceID*)data;
        id = guid;
    }

    void WriteResourceIDText(TypeSerializationContext& typeContext, ITextSerializationWriter& writer, const void* data, const void* defaultData)
    {
        const auto& id = *(const ResourceID*)data;
        if (id)
            writer.writeValueText(TempString("{}", id));
        else
            writer.writeValueText("null");
    }

    void ReadResourceIDText(TypeSerializationContext& typeContext, ITextSerializationReader& reader, void* data)
    {
        StringView text;
        reader.readValueText(text);

        ResourceID parsed;
        if (!ResourceID::Parse(text, parsed))
        {
            TRACE_ERROR("{}: unable to parse resource ID from '{}'", typeContext, text);
        }

        auto& id = *(ResourceID*)data;
        id = parsed;
    }

} // prv

RTTI_BEGIN_CUSTOM_TYPE(ResourceID);
    RTTI_BIND_NATIVE_COPY(ResourceID);
    RTTI_BIND_NATIVE_COMPARE(ResourceID);
    RTTI_BIND_NATIVE_CTOR_DTOR(ResourceID);
    RTTI_BIND_CUSTOM_BINARY_SERIALIZATION(&prv::WriteResourceIDBinary, &prv::ReadResourceIDBinary);
    RTTI_BIND_CUSTOM_TEXT_SERIALIZATION(&prv::WriteResourceIDText, &prv::ReadResourceIDText);
RTTI_END_TYPE();

void ResourceID::print(IFormatStream& f) const
{
    m_guid.print(f);
}

bool ResourceID::Parse(StringView path, ResourceID& outPath)
{
    GUID id;
    if (!GUID::Parse(path.data(), path.length(), id))
        return false;

    outPath = ResourceID(id);
    return true;
}

const ResourceID& ResourceID::EMPTY()
{
    static ResourceID theEmptyID;
    return theEmptyID;
}

ResourceID ResourceID::Create()
{
    const auto id = GUID::Create();
    return ResourceID(id);
}

END_INFERNO_NAMESPACE()
