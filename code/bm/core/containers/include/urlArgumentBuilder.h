/***
* Inferno Engine v4
* Written by Tomasz "RexDex" Jonarski
***/

#pragma once
#include "inplaceArray.h"

BEGIN_INFERNO_NAMESPACE()

//----

/// param list for general URL arguments
class BM_CORE_CONTAINERS_API URLArgumentBuilder : public MainPoolData<NoCopy>
{
public:
    URLArgumentBuilder();

    //--

    void clear();

    URLArgumentBuilder& param(StringView key, StringView value);
    URLArgumentBuilder& paramInt(StringView key, int64_t value);
    URLArgumentBuilder& paramFloat(StringView key, double value);
    URLArgumentBuilder& paramBool(StringView key, bool value);
    URLArgumentBuilder& paramBase64(StringView key, BufferView buf);
	URLArgumentBuilder& paramHex(StringView key, BufferView buf);

    template< typename T >
    INLINE URLArgumentBuilder& paramAnyBase64(StringView key, const T& data)
    {
        return paramBase64(key, &data, sizeof(data));;
    }

	template< typename T >
	INLINE URLArgumentBuilder& paramAnyHex(StringView key, const T& data)
	{
		return paramHex(key, &data, sizeof(data));;
	}

    //--

    StringBuf toString() const;
    Buffer toBuffer() const;

    //--

    // print to stream, outputs classic ?param=val&param=val string
	// NOTE: all special characters are properly written in URL compatible format
	void print(IFormatStream& f) const;

    //--

private:
    struct Entry
    {
        uint32_t key = 0;
        uint32_t value = 0;
    };

    InplaceArray<Entry, 32> m_params; // parameters with value
    InplaceArray<char, 1024> m_textBuffer; // internal text buffer
    uint32_t m_textLength = 0;

    uint32_t makeString(StringView txt);
    uint32_t makeSafeString(StringView txt);
    uint32_t makeBase64(BufferView data);
    uint32_t makeHex(BufferView data);

    uint32_t measureNeededSize() const;
    void write(char* str, uint32_t length) const;
};

//----

END_INFERNO_NAMESPACE()
