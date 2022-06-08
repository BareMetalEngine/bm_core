/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "stringBuilder.h"
#include "bm/core/containers/include/utf8StringFunctions.h"

BEGIN_INFERNO_NAMESPACE()

//--

StringBuilder::StringBuilder(IPoolUnmanaged& pool)
    : m_buffer(pool)
{
    writeNullTerminator();
}

StringBuilder::~StringBuilder()
{
    clear();
}

void StringBuilder::clear()
{
    m_buffer.clear();
}

void StringBuilder::reset()
{
}

//--

IFormatStream& StringBuilder::append(StringView view)
{
    if (!view.empty())
    {
        auto requiredCapacity = m_buffer.size() + view.length() + 1;
        if (m_buffer.ensureCapacity(requiredCapacity))
        {
            auto allocateView = m_buffer.allocate(view.length());
            memcpy(allocateView.data(), view.data(), view.length());

            writeNullTerminator();
        }
    }

    return *this;
}

IFormatStream& StringBuilder::append(const wchar_t* str, uint32_t len /*= INDEX_MAX*/)
{
    if (str && *str)
    {
        if (len == INDEX_MAX)
            len = wcslen(str);

        uint32_t convertedLength = 0;
        for (uint32_t i = 0; i < len; ++i)
        {
            char data[6];
            convertedLength += utf8::ConvertChar(data, str[i]);
        }

        // allocate
        auto requiredCapacity = m_buffer.size() + convertedLength + 1;
        if (m_buffer.ensureCapacity(requiredCapacity))
        {
            auto ptr = (char*) m_buffer.allocate(convertedLength).data();

            for (uint32_t i = 0; i < len; ++i)
                ptr += utf8::ConvertChar(ptr, str[i]);
            writeNullTerminator();
        }
    }

    return *this;
}

IFormatStream& StringBuilder::append(const char* str, uint32_t len /*= INDEX_MAX*/)
{
    return append(StringView(str, len));
}

IFormatStream& StringBuilder::append(const StringBuf& str)
{
    return append(str.c_str());
}

IFormatStream& StringBuilder::append(StringID str)
{
    return append(str.c_str());
}

//--

END_INFERNO_NAMESPACE()

