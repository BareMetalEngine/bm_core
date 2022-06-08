/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"

#include "serializationWriter.h"
#include "serializationReader.h"
#include "bm/core/system/include/timestamp.h"
#include "textSerializationWriter.h"
#include "textSerializationReader.h"

BEGIN_INFERNO_NAMESPACE()

namespace prv
{

    void WriteBinary(TypeSerializationContext& typeContext, SerializationWriter& stream, const void* rawData, const void* defaultData)
    {
        const auto& data = *(const TimeStamp*)rawData;
        stream.writeTypedData<uint64_t>(data.value());
    }

    void ReadBinary(TypeSerializationContext& typeContext, SerializationReader& stream, void* rawData)
    {
        uint64_t value = 0;
        stream.readTypedData(value);

        auto& data = *(TimeStamp*)rawData;
        data = TimeStamp(value);
    }

    void WriteText(TypeSerializationContext& typeContext, ITextSerializationWriter& writer, const void* rawData, const void* defaultData)
    {
        const auto& data = *(const TimeStamp*)rawData;
        const auto value = data.value();
        writer.writeValueText(TempString("{}", value));
    }

    void ReadText(TypeSerializationContext& typeContext, ITextSerializationReader& reader, void* rawData)
    {
        StringView text;
        reader.readValueText(text);

        uint64_t value = 0;
        text.match(value);

        auto& data = *(TimeStamp*)rawData;
        data = TimeStamp(value);
    }

} // prv

RTTI_BEGIN_CUSTOM_TYPE(TimeStamp);
    RTTI_BIND_NATIVE_CTOR_DTOR(TimeStamp);
    RTTI_BIND_NATIVE_COPY(TimeStamp);
    RTTI_BIND_NATIVE_COMPARE(TimeStamp);
    RTTI_BIND_CUSTOM_BINARY_SERIALIZATION(&prv::WriteBinary, &prv::ReadBinary);
    RTTI_BIND_CUSTOM_TEXT_SERIALIZATION(&prv::WriteText, &prv::ReadText);
RTTI_END_TYPE();

END_INFERNO_NAMESPACE()
