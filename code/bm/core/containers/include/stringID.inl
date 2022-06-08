/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

BEGIN_INFERNO_NAMESPACE()

///----

INLINE StringID::StringID()
    : indexValue(0)
    , debugTxt("")
{}

INLINE StringID::StringID(const StringID& other)
    : indexValue(other.indexValue)
    , debugTxt(other.debugTxt)
{}

INLINE StringID::StringID(const char* other)
{
	indexValue = Alloc(other);
    debugTxt = c_str();
}

INLINE StringID::StringID(StringView other)
{
	indexValue = Alloc(other);
    debugTxt = c_str();
}

template< uint32_t N >
INLINE StringID::StringID(const BaseTempString<N>& other)
{
	indexValue = Alloc(other.c_str());
    debugTxt = c_str();
}

INLINE bool StringID::operator==(StringID other) const
{
    return indexValue == other.indexValue;
}

INLINE bool StringID::operator!=(StringID other) const
{
    return indexValue != other.indexValue;
}

INLINE bool StringID::operator==(const char* other) const
{
    return view() == other;
}

INLINE bool StringID::operator==(const StringBuf& other) const
{
    return view() == other;
}

INLINE bool StringID::operator!=(const char* other) const
{
    return view() != other;
}

INLINE bool StringID::operator!=(const StringBuf& other) const
{
    return view() != other;
}

INLINE bool StringID::operator<(StringID other) const
{
    return view() < other.view();
}

INLINE StringID& StringID::operator=(StringID other)
{
    indexValue = other.indexValue;
    debugTxt = other.debugTxt;
    return *this;
}

INLINE bool StringID::empty() const
{
    return 0 == indexValue;
}

INLINE StringID::operator bool() const
{
    return 0 != indexValue;
}

INLINE uint32_t StringID::CalcHash(StringID id)
{
    return StringView::CalcHash(id.view());
}

INLINE uint32_t StringID::CalcHash(StringView txt)
{
    return StringView::CalcHash(txt);
}

INLINE uint32_t StringID::CalcHash(const char* txt)
{
    return StringView::CalcHash(txt);
}
        
INLINE const char* StringID::c_str() const
{
    auto pageIndex = indexValue / STRING_TABLE_PAGE_SIZE;
    auto pageOffset = indexValue % STRING_TABLE_PAGE_SIZE;
    return indexValue ? (st_StringTable[pageIndex] + pageOffset) : "";
}

INLINE StringView StringID::view() const
{
	return StringView(c_str());
}

INLINE uint32_t StringID::index() const
{
    return indexValue;
}

END_INFERNO_NAMESPACE()
