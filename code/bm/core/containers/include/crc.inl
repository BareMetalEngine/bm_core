/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

BEGIN_INFERNO_NAMESPACE()

//---

INLINE CRC64& operator<<(CRC64& crc, const char* str)
{
    return crc.append(str, strlen(str));
}

INLINE CRC64& operator<<(CRC64& crc, const wchar_t* str)
{
    return crc.append(str, sizeof(wchar_t) * wcslen(str));
}

INLINE CRC32& operator<<(CRC32& crc, const char* str)
{
    return crc.append(str, strlen(str));
}

INLINE CRC32& operator<<(CRC32& crc, const wchar_t* str)
{
    return crc.append(str, sizeof(wchar_t) * wcslen(str));
}

INLINE CRC32& CRC32::appendStatic1(uint8_t data)
{
    auto crc = m_crc;
    crc = (crc >> 8) ^ CRCTable[data ^ (uint8_t)crc];
    m_crc = crc;
    return *this;
}

//--

INLINE CRC64& CRC64::appendStatic1(uint8_t data)
{
    auto crc = m_crc;
    crc = (crc >> 8) ^ CRCTable[data ^ (uint8_t)crc];
    m_crc = crc;
    return *this;
}

//--

END_INFERNO_NAMESPACE()

