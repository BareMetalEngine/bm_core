/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "rttiType.h"
#include "rttiTypeSystem.h"

#include "serializationWriter.h"
#include "serializationReader.h"
#include "textSerializationWriter.h"
#include "textSerializationReader.h"

BEGIN_INFERNO_NAMESPACE()

/// simple type definition
template < class T >
class SimpleValueType : public IType
{
public:
    SimpleValueType(const char* typeName, TypeConversionClass typeConversionClass)
        : IType(StringID(typeName))
    {
        m_traits.metaType = MetaType::Simple;
        m_traits.convClass = typeConversionClass;
        m_traits.size = sizeof(T);
        m_traits.alignment = __alignof(T);
        m_traits.nativeHash = typeid(T).hash_code();
        m_traits.requiresConstructor = false;
        m_traits.requiresDestructor = false;
        m_traits.initializedFromZeroMem = true;
        m_traits.simpleCopyCompare = true;
    }

    virtual void construct(void* object) const override final
    {
        new (object) T();
    }

    virtual void destruct(void* object) const override final
    {
        ((T*)object)->~T();
    }

    virtual bool compare(const void* data1, const void* data2) const override final
    {
        return *(const T*)data1 == *(const T*)data2;
    }

    virtual void copy(void* dest, const void* src) const override final
    {
        *(T*)dest = *(const T*)src;
    }

    virtual void writeBinary(TypeSerializationContext& typeContext, SerializationWriter& file, const void* data, const void* defaultData) const override
    {
        file.writeTypedData(*(const T*)data);
    }

    virtual void readBinary(TypeSerializationContext& typeContext, SerializationReader& file, void* data) const override
    {
        file.readTypedData(*(T*)data);
    }

    virtual void writeText(TypeSerializationContext& typeContext, ITextSerializationWriter& file, const void* data, const void* defaultData) const override
    {
		TempString txt;
        if (std::is_same<T, float>::value)
		{
            const auto value = *(const float*)data;
            const auto valueU = (uint32_t&)value;
			txt << PreciseFloat(value);
			file.writeValueText(txt.c_str());

            const auto test = (float)atof(txt.c_str());
            const auto testU = (uint32_t&)test;
            ASSERT(test == value);
            ASSERT(testU == valueU);
        }
		else if (std::is_same<T, double>::value)
		{
            const auto value = *(const double*)data;
            const auto valueU = (uint64_t&)value;
            txt << PreciseDouble(value);
			file.writeValueText(txt.c_str());

			const auto test = atof(txt.c_str());
			const auto testU = (uint64_t&)test;
			ASSERT(test == value);
			ASSERT(testU == valueU);
		}
        else
        {
            txt << *(const T*)data;
            file.writeValueText(txt.c_str());
        }
    }

    virtual void readText(TypeSerializationContext& typeContext, ITextSerializationReader& file, void* data) const override
    {
        StringView value;
        file.readValueText(value);
        if (!value.match(*(T*)data))
        {
            file.reportError(TempString("Type conversion to '{}' failed", name()));
        }
    }
};

class SimpleTypeStringBuf : public SimpleValueType<StringBuf>
{
public:
    SimpleTypeStringBuf()
        : SimpleValueType<StringBuf>("StringBuf", TypeConversionClass::TypeStringBuf)
    {
        m_traits.requiresConstructor = true;
        m_traits.initializedFromZeroMem = true;
        m_traits.requiresDestructor = true;
        m_traits.simpleCopyCompare = false;
    }

    virtual void writeBinary(TypeSerializationContext& typeContext, SerializationWriter& file, const void* data, const void* defaultData) const override final
    {
        const auto& str = *(const StringBuf*)data;

        const uint32_t length = str.length();
        file.writeTypedData(length);

        if (length)
            file.writeData(str.c_str(), length);
    }

    virtual void readBinary(TypeSerializationContext& typeContext, SerializationReader& file, void* data) const override final
    {
        auto& str = *(StringBuf*)data;

        uint32_t length = 0;
        file.readTypedData(length);

        if (length)
        {
            const auto* chr = (const char*)file.readData(length);
            str = StringBuf(StringView(chr, chr + length));
        }
        else
        {
            str = "";
        }
    }

	virtual void writeText(TypeSerializationContext& typeContext, ITextSerializationWriter& file, const void* data, const void* defaultData) const override
	{
		const auto& str = *(const StringBuf*)data;
		file.writeValueText(str);
	}

	virtual void readText(TypeSerializationContext& typeContext, ITextSerializationReader& file, void* data) const override
	{
        StringView text;
        file.readValueText(text);

		auto& str = *(StringBuf*)data;
        str = StringBuf(text);
	}
};

class SimpleTypeStringID : public SimpleValueType<StringID>
{
public:
    SimpleTypeStringID()
        : SimpleValueType<StringID>("StringID", TypeConversionClass::TypeStringID)
    {
        m_traits.requiresConstructor = true;
        m_traits.initializedFromZeroMem = true;
        m_traits.requiresDestructor = false;
        m_traits.simpleCopyCompare = true;
    }

    virtual void writeBinary(TypeSerializationContext& typeContext, SerializationWriter& file, const void* data, const void* defaultData) const override final
    {
        const auto& str = *(const StringID*)data;
        file.writeStringID(str);;
    }

    virtual void readBinary(TypeSerializationContext& typeContext, SerializationReader& file, void* data) const override final
    {
        const auto str = file.readStringID();
        *(StringID*)data = str;
    }

	virtual void writeText(TypeSerializationContext& typeContext, ITextSerializationWriter& file, const void* data, const void* defaultData) const override
	{
		const auto& str = *(const StringID*)data;
		file.writeValueText(str.view());
	}

	virtual void readText(TypeSerializationContext& typeContext, ITextSerializationReader& file, void* data) const override
	{
        StringView text;
        file.readValueText(text);

		auto& str = *(StringID*)data;
        str = StringID(text);
	}
};

#define REGISTER_FUNDAMENTAL_TYPE( type )   typeSystem.registerType(new SimpleValueType<type>(#type, TypeConversionClass::Type##type));

void RegisterFundamentalTypes(TypeSystem& typeSystem)
{
    REGISTER_FUNDAMENTAL_TYPE(bool);
    REGISTER_FUNDAMENTAL_TYPE(uint8_t);
    REGISTER_FUNDAMENTAL_TYPE(char);
    REGISTER_FUNDAMENTAL_TYPE(uint16_t);
    REGISTER_FUNDAMENTAL_TYPE(short);
    REGISTER_FUNDAMENTAL_TYPE(uint32_t);
    REGISTER_FUNDAMENTAL_TYPE(int);
    REGISTER_FUNDAMENTAL_TYPE(float);
    REGISTER_FUNDAMENTAL_TYPE(double);
    REGISTER_FUNDAMENTAL_TYPE(int64_t);
    REGISTER_FUNDAMENTAL_TYPE(uint64_t);

    typeSystem.registerType(new SimpleTypeStringBuf);
    typeSystem.registerType(new SimpleTypeStringID);
}

END_INFERNO_NAMESPACE()
