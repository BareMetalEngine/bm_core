/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "rttiVariant.h"
#include "rttiType.h"
#include "rttiTypedMemory.h"

#include "serializationWriter.h"
#include "serializationReader.h"
#include "textSerializationWriter.h"
#include "textSerializationReader.h"

#include "bm/core/containers/include/stringBuilder.h"

BEGIN_INFERNO_NAMESPACE()

//--

static bool MustAllocateVariantMemory(Type type)
{
    if (type->traits().requiresConstructor || type->traits().requiresDestructor)
        return true;
    if (type->alignment() > 4 || type->size() > Variant::INTERNAL_STORAGE_SIZE)
        return true;

    //return false;
    return true;
}

//--

Variant::Variant(const Variant& other)
{
    if (other)
        reset(other.type(), other.data());
}

Variant::Variant(Variant&& other)
    : m_type(other.m_type)
    , m_data(other.m_data)
{
    other.m_type = nullptr;
    other.m_data = nullptr;
}

Variant::~Variant()
{
    reset();
}

Variant& Variant::operator=(const Variant& other)
{
    if (this != &other)
        reset(other.type(), other.data());
    return *this;
}

Variant& Variant::operator=(Variant&& other)
{
    if (this != &other)
    {
        m_data = other.m_data;
        m_type = other.m_type;
		other.m_type = nullptr;
		other.m_data = nullptr;
    }

    return *this;
}

bool Variant::operator==(const Variant& other) const
{
    if (type() == other.type())
    {
        if (data() && other.data())
            return type()->compare(data(), other.data());
        else
            return !!data() && !!other.data();
    }

    return false;
}

bool Variant::operator!=(const Variant& other) const
{
    return !operator==(other);
}

void* Variant::AllocateVariantMemory(Type type)
{
    // TODO: custom pool for variants
    return Memory::AllocateBlock(type->size(), type->alignment(), "Variant");
}

void Variant::FreeVariantMemory(void* data)
{
    Memory::FreeBlock(data);
}

void Variant::reset()
{
    if (m_data)
    {
        m_type->destruct(m_data);

        FreeVariantMemory(m_data);
        m_data = nullptr;
    }
}

static bool IsAligned(const void* ptr, uint32_t alignemnt)
{
    return AlignPtr(ptr, alignemnt) == ptr;
}

bool Variant::reset(Type type, const void* data)
{
    ASSERT_EX(!m_type, "Variant should be empty before initialization");
    DEBUG_CHECK_EX(type, "Trying to create variant from non existing type");

    // change type of the container
    if (type != m_type)
    {
        // destroy current value
        if (m_type)
        {
            m_type->destruct(m_data);
            m_type = m_type;

            FreeVariantMemory(m_data);
            m_data = nullptr;
        }

        // change type
        ASSERT(!m_data)
        m_type = type;

        // allocate new storage
        if (type)
        {
            auto* data = AllocateVariantMemory(type);
            DEBUG_CHECK_RETURN_EX_V(data, "OOM in variant", false);

            m_data = data;
            m_type->construct(m_data);
        }
    }

    // copy new value
    ASSERT(m_type == type);
    if (data)
        m_type->copy(m_data, data);

    return true;
}

bool Variant::set(Type srcType, const void* srcData)
{
    return ConvertData(srcData, srcType, data(), type());
}

bool Variant::get(Type destType, void* destData) const
{
    return ConvertData(data(), type(), destData, destType);
}

void Variant::print(IFormatStream& f) const
{
    IType::PrintToString(m_type, m_data, f);
}

bool Variant::fromString(StringView txt)
{
    return IType::ParseFromString(m_type, m_data, txt);
}

StringBuf Variant::toString() const
{
    StringBuilder txt;
    IType::PrintToString(m_type, m_data, txt);
    return StringBuf(txt);
}

static const Variant theEmpty;

const Variant& Variant::EMPTY()
{
    return theEmpty;
}

//---

namespace prv
{
    static void VariantWriteBinary(TypeSerializationContext& typeContext, SerializationWriter& stream, const void* data, const void* defaultData)
    {
        const auto& v = *(const Variant*)data;

        if (v.empty())
        {
            stream.writeTypedData<uint8_t>(0);
        }
        else
        {
            stream.writeTypedData<uint8_t>(1);
            stream.writeType(v.type());

            stream.beginSkipBlock();
            v.type()->writeBinary(typeContext, stream, v.data(), nullptr);
            stream.endSkipBlock();
        }
    }

    static void VariantReadBinary(TypeSerializationContext& typeContext, SerializationReader& stream, void* data)
    {
        auto& v = *(Variant*)data;

        // load type name, if its empty variant had no value
		StringID typeName;
		const auto type = stream.readType(typeName);

        // had value ?
        if (typeName)
		{
            SerializationSkipBlock block(stream);

            if (type)
            {
                v.reset(type);
                type->readBinary(typeContext, stream, v.data());
            }
            else
            {
                v.reset();
            }
        }
        else
        {
            // no value
            v.reset();
        }
    }

} // prv

RTTI_BEGIN_CUSTOM_TYPE(Variant);
    RTTI_BIND_NATIVE_CTOR_DTOR(Variant);
    RTTI_BIND_NATIVE_COPY(Variant);
    RTTI_BIND_NATIVE_COMPARE(Variant);
    RTTI_BIND_CUSTOM_BINARY_SERIALIZATION(&prv::VariantWriteBinary, &prv::VariantReadBinary);
RTTI_END_TYPE();

//--

namespace prv
{

    static void WriteTypeBinary(TypeSerializationContext& typeContext, SerializationWriter& stream, const void* data, const void* defaultData)
    {
        auto v = *(const Type*)data;
        stream.writeType(v);
    }

    static void ReadTypeBinary(TypeSerializationContext& typeContext, SerializationReader& stream, void* data)
    {
        StringID typeName;
        *(Type*)data = stream.readType(typeName);
    }

	static void WriteTypeText(TypeSerializationContext& typeContext, ITextSerializationWriter& stream, const void* data, const void* defaultData)
	{
		auto v = *(const Type*)data;

        if (v)
            stream.writeValueText(v.name().view());
        else
            stream.writeValueText("null");
	}

	static void ReadTypeText(TypeSerializationContext& typeContext, ITextSerializationReader& stream, void* data)
	{
        StringView text;
        stream.readValueText(text);

		StringID typeName;
        *(Type*)data = RTTI::GetInstance().findType(StringID(text));
	}


} // prv

RTTI_BEGIN_CUSTOM_TYPE(Type);
    RTTI_BIND_NATIVE_CTOR_DTOR(Type);
    RTTI_BIND_NATIVE_COPY(Type);
    RTTI_BIND_NATIVE_COMPARE(Type);
    RTTI_BIND_CUSTOM_BINARY_SERIALIZATION(&prv::WriteTypeBinary, &prv::ReadTypeBinary);
    RTTI_BIND_CUSTOM_TEXT_SERIALIZATION(&prv::WriteTypeText, &prv::ReadTypeText);
RTTI_END_TYPE();

//--

namespace prv
{

    void WriteClassTypeBinary(TypeSerializationContext& typeContext, SerializationWriter& stream, const void* data, const void* defaultData)
    {
        const auto& cls = *(const ClassType*)data;
        stream.writeType(cls.ptr());
    }

    void ReadClassTypeBinary(TypeSerializationContext& typeContext, SerializationReader& stream, void* data)
    {
        StringID typeName;
        const auto clsType = stream.readType(typeName);

        auto& cls = *(ClassType*)data;
        cls = clsType.toClass();
    }

	static void WriteClassTypeText(TypeSerializationContext& typeContext, ITextSerializationWriter& stream, const void* data, const void* defaultData)
	{
        const auto& cls = *(const ClassType*)data;

		if (cls)
			stream.writeValueText(cls.name().view());
		else
			stream.writeValueText("null");
	}

	static void ReadClassTypeText(TypeSerializationContext& typeContext, ITextSerializationReader& stream, void* data)
	{
		StringView text;
		stream.readValueText(text);

		StringID typeName;
		*(ClassType*)data = RTTI::GetInstance().findClass(StringID(text));
	}

} // prv

RTTI_BEGIN_CUSTOM_TYPE(ClassType);
RTTI_BIND_NATIVE_CTOR_DTOR(ClassType);
RTTI_BIND_NATIVE_COMPARE(ClassType);
RTTI_BIND_NATIVE_COPY(ClassType);
RTTI_BIND_CUSTOM_BINARY_SERIALIZATION(&prv::WriteClassTypeBinary, &prv::ReadClassTypeBinary);
RTTI_BIND_CUSTOM_TEXT_SERIALIZATION(&prv::WriteClassTypeText, &prv::ReadClassTypeText);
RTTI_END_TYPE();

END_INFERNO_NAMESPACE()

