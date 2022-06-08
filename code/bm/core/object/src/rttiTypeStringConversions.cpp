/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
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

bool IType::ApplyFromString(Type type, void* data, const Array<StringBuf>& values)
{
    DEBUG_CHECK_RETURN_EX_V(type, "Invalid type", false);

    if (type->metaType() == MetaType::Array)
    {
        const auto* arrayType = static_cast<const IArrayType*>(type.ptr());
        arrayType->clearArrayElements(data);
        
        // try to parse each element
        uint32_t numValidElements = 0;
        for (const auto& str : values)
        {
            // parse into temporary storage (so we don't add to array unless its valid)
            DataHolder tempData(arrayType->innerType());
            if (ParseFromString(arrayType->innerType(), tempData.data(), str))
            {
                // try to allocate new element from array (may fail for static arrays)
                if (arrayType->canArrayBeResized())
                {
                    if (arrayType->createArrayElement(data, numValidElements))
                    {
                        void* arrayElementData = arrayType->arrayElementData(data, numValidElements++);
                        arrayType->innerType()->copy(arrayElementData, tempData.data());
                    }
                }
                else
                {
                    // get n-th element (fails if out of bounds for static array)
                    if (void* arrayElementData = arrayType->arrayElementData(data, numValidElements))
                    {
                        arrayType->innerType()->copy(arrayElementData, tempData.data());
                        numValidElements += 1;
                    }
                }
            }
        }

        return true;
    }
    else if (!values.empty())
    {
        // just parse value directly
        return ParseFromString(type, data, values.back().view());
    }

    // no data
    return false;
}

void IType::CaptureToString(Type type, const void* data, Array<StringBuf>& outValues)
{
    DEBUG_CHECK_RETURN_EX(type, "Invalid type");

    if (type->metaType() == MetaType::Array)
    {
        const auto* arrayType = static_cast<const IArrayType*>(type.ptr());
        const auto size = arrayType->arraySize(data);

        // always reset output, no stacking
        outValues.reset();
        outValues.reserve(size);

        // convert each element to string
        for (uint32_t i = 0; i < size; ++i)
        {
            // get n-th element (fails if out of bounds for static array)
            if (const void* arrayElementData = arrayType->arrayElementData(data, i))
            {
                // store array element in text
                StringBuilder txt;
                PrintToString(arrayType->innerType().ptr(), arrayElementData, txt);
                outValues.emplaceBack(txt.view());
            }
        }
    }
    else
    {
		outValues.reset();

		StringBuilder value;
        if (PrintToString(type, data, value))
            outValues.emplaceBack(value.view());
    }
}

///---

template< typename T >
static void PrintToStringSimple(const void* data, IFormatStream& f)
{
    f << *(const T*)data;
}

bool IType::PrintToString(Type type, const void* data, IFormatStream& f)
{
    if (type)
    {
        switch (type->traits().convClass)
        {
            case TypeConversionClass::Typebool:
                PrintToStringSimple<bool>(data, f);
                break;
		    case TypeConversionClass::Typechar:
			    PrintToStringSimple<char>(data, f);
			    break;
		    case TypeConversionClass::Typeshort:
			    PrintToStringSimple<short>(data, f);
			    break;
		    case TypeConversionClass::Typeint:
			    PrintToStringSimple<int>(data, f);
			    break;
		    case TypeConversionClass::Typeint64_t:
			    PrintToStringSimple<int64_t>(data, f);
			    break;
		    case TypeConversionClass::Typeuint8_t:
			    PrintToStringSimple<uint8_t>(data, f);
			    break;
		    case TypeConversionClass::Typeuint16_t:
			    PrintToStringSimple<uint16_t>(data, f);
			    break;
		    case TypeConversionClass::Typeuint32_t:
			    PrintToStringSimple<uint32_t>(data, f);
			    break;
		    case TypeConversionClass::Typeuint64_t:
			    PrintToStringSimple<uint64_t>(data, f);
			    break;
		    case TypeConversionClass::Typefloat:
			    PrintToStringSimple<float>(data, f);
			    break;
		    case TypeConversionClass::Typedouble:
			    PrintToStringSimple<double>(data, f);
			    break;
		    case TypeConversionClass::TypeStringBuf:
			    PrintToStringSimple<StringBuf>(data, f);
			    break;
		    case TypeConversionClass::TypeStringID:
			    PrintToStringSimple<StringID>(data, f);
			    break;
			case TypeConversionClass::TypeClassRef:
				PrintToStringSimple<ClassType>(data, f);
				break;
			case TypeConversionClass::TypeEnum:
            {
                const auto* enumType = static_cast<const EnumType*>(type.ptr());
                StringID name;
                if (!enumType->toStringID(data, name))
                    return false;
                f << name;
                return true;
            }
            default:
            {
                return false;
            }				
        }
    }

    return false;
}

template< typename T >
static bool ParseFromStringSimple(void* data, StringView txt)
{
	return txt.match(*(T*)data);
}

bool IType::ParseFromString(Type type, void* data, StringView txt)
{
	if (type)
	{
		switch (type->traits().convClass)
		{
		case TypeConversionClass::Typebool:
			return ParseFromStringSimple<bool>(data, txt);
		case TypeConversionClass::Typechar:
			return ParseFromStringSimple<char>(data, txt);
		case TypeConversionClass::Typeshort:
			return ParseFromStringSimple<short>(data, txt);
		case TypeConversionClass::Typeint:
			return ParseFromStringSimple<int>(data, txt);
		case TypeConversionClass::Typeint64_t:
			return ParseFromStringSimple<int64_t>(data, txt);
		case TypeConversionClass::Typeuint8_t:
			return ParseFromStringSimple<uint8_t>(data, txt);
		case TypeConversionClass::Typeuint16_t:
			return ParseFromStringSimple<uint16_t>(data, txt);
		case TypeConversionClass::Typeuint32_t:
			return ParseFromStringSimple<uint32_t>(data, txt);
		case TypeConversionClass::Typeuint64_t:
			return ParseFromStringSimple<uint64_t>(data, txt);
		case TypeConversionClass::Typefloat:
			return ParseFromStringSimple<float>(data, txt);
		case TypeConversionClass::Typedouble:
			return ParseFromStringSimple<double>(data, txt);
		case TypeConversionClass::TypeStringBuf:
            *(StringBuf*)data = StringBuf(txt);
            return true;
		case TypeConversionClass::TypeStringID:
			*(StringID*)data = StringID(txt.trim());
			return true;
		case TypeConversionClass::TypeClassRef:
        {
            if (txt == "null")
            {
                *(ClassType*)data = nullptr;
                return true;
            }
            else
            {
                auto cls = RTTI::GetInstance().findClass(txt.trim());
                if (!cls)
                    return false;

                *(ClassType*)data = cls;
                return true;
            }
            break;
        }
		case TypeConversionClass::TypeEnum:
		{
			const auto* enumType = static_cast<const EnumType*>(type.ptr());
            return enumType->fromString(txt.trim(), data);
		}
		default:
		{
			return false;
		}
		}
	}

    return false;
}

///---

END_INFERNO_NAMESPACE()
