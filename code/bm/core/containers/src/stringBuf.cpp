/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "inplaceArray.h"
#include "utf8StringFunctions.h"
#include "bm/core/memory/include/implDynamicNativeAllocator.h"

BEGIN_INFERNO_NAMESPACE()

namespace prv
{

    //---

    StringDataHolder* StringDataHolder::CreateAnsi(const char* txt, uint32_t length/* = INDEX_MAX*/)
    {
        if (!txt || !*txt)
            return nullptr;

        if (length == INDEX_MAX)
            length = strlen(txt);

        auto data = (StringDataHolder*) StringBuf::StringPool().allocateMemory(sizeof(StringDataHolder) + length);
        data->m_refs = 1;
        data->m_length = length;
        data->m_unicodeLength = utf8::Length(txt, txt+length);
        memcpy(data->m_txt, txt, length);
        data->m_txt[length] = 0;
        return data;
    }

    StringDataHolder* StringDataHolder::CreateUnicode(const wchar_t* txt, uint32_t uniLength /*= INDEX_MAX*/)
    {
        if (!txt || !*txt)
            return nullptr;

        if (uniLength == INDEX_MAX)
            uniLength = wcslen(txt);

        auto length  = utf8::CalcSizeRequired(txt, uniLength);

        auto data = (StringDataHolder*)StringBuf::StringPool().allocateMemory(sizeof(StringDataHolder) + length);
        data->m_refs = 1;
        data->m_length = length;
        data->m_unicodeLength = uniLength;
        utf8::FromUniChar(data->m_txt, length + 1, txt, uniLength);
        data->m_txt[length] = 0;
        return data;
    }

    StringDataHolder* StringDataHolder::CreateEmpty(uint32_t length)
    {
        if (!length)
            return nullptr;

        auto data = (StringDataHolder*)StringBuf::StringPool().allocateMemory(sizeof(StringDataHolder) + length);
        data->m_refs = 1;
        data->m_length = length;
        memset(data->m_txt, 0, length + 1);
        return data;
    }

    StringDataHolder* StringDataHolder::copy() const
    {
        auto ret  = StringDataHolder::CreateEmpty(m_length);
        memcpy(ret->m_txt, m_txt, m_length + 1);
        return ret;
    }

    void StringDataHolder::ReleaseToPool(void* mem, uint32_t length)
    {
        StringBuf::StringPool().freeMemory(mem);
    }

    //---

} // prv

//--

StringBuf EmptyString;

const StringBuf& StringBuf::EMPTY()
{
    return EmptyString;
}

//--

StringBuf::StringBuf(BufferView buffer)
    : m_data(nullptr)
{
    if (buffer)
    {
        auto rawDataSize  = buffer.size();
        auto uniData  = (const wchar_t*)buffer.data();
        if (rawDataSize >= 2 && (*uniData == 0xFFFE || *uniData == 0xFFFE))
        {
            auto stringLength  = (rawDataSize - 2) / 2;
            m_data = prv::StringDataHolder::CreateUnicode(uniData + 1, stringLength);
        }
        else
        {
            m_data = prv::StringDataHolder::CreateAnsi((const char*)buffer.data(), rawDataSize);
        }
    }
}

//--

StringBuf StringBuf::transcode(std::function<uint32_t(uint32_t)> func) const
{
	StringBuilder txt;
	bool replacementDone = false;

	const auto* str = view().data();
	const auto* strEnd = view().data() + length();
	while (str < strEnd)
	{
		const auto ch = utf8::NextChar(str, strEnd);
		if (!ch)
			break;

        const auto newCh = func(ch);
        if (newCh == ch)
        {
            txt.appendUTF32(ch);
        }
        else
        {
            if (newCh) // do not output empty chars
			    txt.appendUTF32(newCh);
			replacementDone = true;
		}
	}

	if (replacementDone)
		return StringBuf(txt);
	else
		return *this;
}

StringBuf StringBuf::removeChar(uint32_t ch) const
{
	return transcode([ch](uint32_t x)
		{
			return (ch == x) ? 0 : x;
		});
}

static bool IsSafeFileNameCharacter(uint32_t ch, bool keepWhitespaces)
{
    switch (ch)
    {
    case ' ':
        return keepWhitespaces;
	case '<':
    case '>':
    case ':':
	case '\"':
	case '/':
    case '\\':
    case '|':
    case '?':
    case '*':
        return false;
    default:
        if (ch < 32) return false;
        return true;
    }
}

StringBuf StringBuf::sanitizeFileName(bool keepWhitespaces /*= true*/) const
{
	return transcode([keepWhitespaces](uint32_t ch) -> uint32_t
		{
            if (!IsSafeFileNameCharacter(ch, keepWhitespaces))
                return '_';
            return ch;
		});    
}

StringBuf StringBuf::replaceChar(uint32_t oldCh, uint32_t newCh) const
{
    return transcode([oldCh, newCh](uint32_t ch)
        {
            return (ch == oldCh) ? newCh : ch;
        });
}

StringBuf StringBuf::toLower() const
{
	return transcode([](uint32_t ch)
		{
            return StringView::MapUpperToLowerCaseUTF32(ch);
		});
}

StringBuf StringBuf::toUpper() const
{
	return transcode([](uint32_t ch)
		{
			return StringView::MapLowerToUpperCaseUTF32(ch);
		});
}

//---

static IPoolUnmanaged* GStringPool = new PoolUnmanaged_DynamicNativeAllocator("StringPool");

IPoolUnmanaged& StringBuf::StringPool()
{
    return *GStringPool;
}

//---

END_INFERNO_NAMESPACE()
