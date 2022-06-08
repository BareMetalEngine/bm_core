/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"

#include "serializationWriter.h"
#include "serializationReader.h"
#include "textSerializationWriter.h"
#include "textSerializationReader.h"

#include "asyncBuffer.h"

BEGIN_INFERNO_NAMESPACE()

namespace prv
{
    void WriteBufferBinary(TypeSerializationContext& typeContext, SerializationWriter& stream, const void* data, const void* defaultData)
    {
        auto& buffer = *(Buffer*)data;
        stream.writeInlinedBuffer(buffer);
    }

    void ReadBufferBinary(TypeSerializationContext& typeContext, SerializationReader& stream, void* data)
    {
        auto& buffer = *(Buffer*)data;
        buffer = stream.readInlinedBuffer();
    }

	void WriteBufferText(TypeSerializationContext& typeContext, ITextSerializationWriter& file, const void* data, const void* defaultData)
	{
		auto& buffer = *(Buffer*)data;
        file.writeValueOwnedBytes(buffer);
	}

	void ReadBufferText(TypeSerializationContext& typeContext, ITextSerializationReader& file, void* data)
	{
        auto& buffer = *(Buffer*)data;
        if (!file.readValueBytes(buffer))
            buffer = Buffer();
	}    	

} // prv

RTTI_BEGIN_CUSTOM_TYPE(Buffer);
    RTTI_BIND_NATIVE_CTOR_DTOR(Buffer);
    RTTI_BIND_NATIVE_COPY(Buffer);
    RTTI_BIND_NATIVE_COMPARE(Buffer);
    RTTI_BIND_CUSTOM_BINARY_SERIALIZATION(&prv::WriteBufferBinary, &prv::ReadBufferBinary);
    RTTI_BIND_CUSTOM_TEXT_SERIALIZATION(&prv::WriteBufferText, &prv::ReadBufferText);
RTTI_END_TYPE();

//--

namespace prv
{

    void WriteAsyncBufferBinary(TypeSerializationContext& typeContext, SerializationWriter& stream, const void* data, const void* defaultData)
    {
        const auto& asyncBuffer = *(const AsyncFileBuffer*)data;

        if (auto loader = asyncBuffer.loader())
            stream.writeAsyncBuffer(loader);
        else
            stream.writeInlinedBuffer(Buffer());
    }

    void ReadAsyncBufferBinary(TypeSerializationContext& typeContext, SerializationReader& stream, void* data)
    {
        auto& asyncBuffer = *(AsyncFileBuffer*)data;

        const auto loader = stream.readAsyncBuffer();
        asyncBuffer = AsyncFileBuffer(loader);
    }

	void WriteAsyncBufferText(TypeSerializationContext& typeContext, ITextSerializationWriter& file, const void* data, const void* defaultData)
	{
		const auto& asyncBuffer = *(const AsyncFileBuffer*)data;
        if (asyncBuffer)
        {
            const auto content = asyncBuffer.load(NoTask(), MainPool());
            file.writeValueOwnedBytes(content);
        }
        else
        {
            file.writeValueOwnedBytes(Buffer());
        }
	}

	void ReadAsyncBufferText(TypeSerializationContext& typeContext, ITextSerializationReader& file, void* data)
	{
		auto& asyncBuffer = *(AsyncFileBuffer*)data;

        Buffer buffer;
        file.readValueBytes(buffer);
        asyncBuffer.setup(buffer);
	}

} // prv

//--

RTTI_BEGIN_CUSTOM_TYPE(AsyncFileBuffer);
    RTTI_BIND_NATIVE_CTOR_DTOR(AsyncFileBuffer);
    RTTI_BIND_NATIVE_COMPARE(AsyncFileBuffer);
    RTTI_BIND_NATIVE_COPY(AsyncFileBuffer);
    RTTI_BIND_CUSTOM_BINARY_SERIALIZATION(&prv::WriteAsyncBufferBinary, &prv::ReadAsyncBufferBinary);
    RTTI_BIND_CUSTOM_TEXT_SERIALIZATION(&prv::WriteAsyncBufferText, &prv::ReadAsyncBufferText);
RTTI_END_TYPE();

//--

namespace prv
{

    void WriteGUIDBinary(TypeSerializationContext& typeContext, SerializationWriter& stream, const void* data, const void* defaultData)
    {
        const auto& id = *(const GUID*)data;
        stream.writeTypedData<GUID>(id);
    }

    void ReadGUIDBinary(TypeSerializationContext& typeContext, SerializationReader& stream, void* data)
    {
        GUID guid;
        stream.readTypedData<GUID>(guid);

        auto& id = *(GUID*)data;
        id = guid;
    }

    void WriteGUIDText(TypeSerializationContext& typeContext, ITextSerializationWriter& writer, const void* data, const void* defaultData)
    {
        const auto& id = *(const GUID*)data;
        if (id)
            writer.writeValueText(TempString("{}", id));
        else
            writer.writeValueText("null");
    }

    void ReadGUIDText(TypeSerializationContext& typeContext, ITextSerializationReader& reader, void* data)
    {
        GUID parsed;

        StringView value;
        if (!reader.readValueText(value))
        {
            reader.reportError(TempString("Missing GUID value at {}", typeContext));
            return;
        }

        if (!value.empty())
        {
            if (!GUID::Parse(value.data(), value.length(), parsed))
            {
                reader.reportError(TempString("Unable to parse GUID from '{}' at {}", value, typeContext));
                return;
            }
        }

        auto& id = *(GUID*)data;
        id = parsed;
    }

} // prv

RTTI_BEGIN_CUSTOM_TYPE(GUID);
    RTTI_BIND_NATIVE_COPY(GUID);
    RTTI_BIND_NATIVE_COMPARE(GUID);
    RTTI_BIND_NATIVE_CTOR_DTOR(GUID);
    RTTI_BIND_CUSTOM_BINARY_SERIALIZATION(&prv::WriteGUIDBinary, &prv::ReadGUIDBinary);
    RTTI_BIND_CUSTOM_TEXT_SERIALIZATION(&prv::WriteGUIDText, &prv::ReadGUIDText);
RTTI_END_TYPE();

//--

END_INFERNO_NAMESPACE()

