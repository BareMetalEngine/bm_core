/***
* Inferno Engine v4
* Written by Tomasz "RexDex" Jonarski
***/

#pragma once

#include "inplaceArray.h"
#include "bm/core/system/include/guid.h"

BEGIN_INFERNO_NAMESPACE()

//----

/// parser for URL arguments
class BM_CORE_CONTAINERS_API URLArgumentsParser : public MainPoolData<NoCopy>
{
public:
    URLArgumentsParser();
    URLArgumentsParser(StringView txt); // NOTE: stops on errors

    //--

    // check if key exists
    bool has(StringView key) const;

    // get value for key (the last one if multiple are defined)
    StringView raw(StringView key) const; // NOTE: does not transcode the URL text !!
	bool raw(StringView key, StringView& outView) const; // NOTE: does not transcode the URL text !!

    //--
    // SAFE ACCESS, returns false if value does not exist or can't be converted

    bool safeString(StringView key, StringBuf& outValue) const; // NOTE: conversts the URL encoding
    bool safeInt(StringView key, int& outValue) const;
    bool safeInt(StringView key, int64_t& outValue) const;
    bool safeFloat(StringView key, float& outValue) const;
    bool safeFloat(StringView key, double& outValue) const;
	bool safeBuffer(StringView key, Buffer& outValue) const; // BASE64
	bool safeBufferRaw(StringView key, void* outValue, uint32_t valueSize) const; // BASE64
    bool safeBufferHex(StringView key, Buffer& outValue) const;
    bool safeBufferHexRaw(StringView key, void* outValue, uint32_t valueSize) const;
    bool safeGuid(StringView key, GUID& outGUID) const;

    template< typename T >
    INLINE bool safeType(StringView key, T& outValue) const
    {
        return safeBufferRaw(key, &outValue, sizeof(T));
    }

	template< typename T >
	INLINE bool safeBufferHex(StringView key, T& outValue) const
	{
		return safeBufferHexRaw(key, &outValue, sizeof(T));
	}

	//--
	// QUICK ACCESS, returns default value if value does not exist or can't be parsed

    StringBuf quickString(StringView key, StringView defaultValue = "") const;
	int64_t quickInt(StringView key, int64_t defaultValue = 0) const;
	double quickFloat(StringView key, double defaultValue = 0.0) const;
    Buffer quickBuffer(StringView key) const;
    Buffer quickBufferHex(StringView key) const;
    GUID quickGuid(StringView key) const;

    //--
    // MULTI ACCESS, returns collections of items

    bool collectStrings(StringView key, Array<StringBuf>& outValues) const;
	bool collectInts(StringView key, Array<int>& outValues) const;
	bool collectInts(StringView key, Array<int64_t>& outValues) const;
	bool collectFloats(StringView key, Array<float>& outValues) const;
	bool collectFloats(StringView key, Array<double>& outValues) const;

    //--

    // safe parse into URLArgs
    static bool Parse(StringView txt, URLArgumentsParser& outParser);

    //--

private:
    struct Parameter
    {
        StringView key;
        StringView value;
        int next = -1;
        uint32_t hash = 0;
    };

    InplaceArray<Parameter, 64> m_parameters; // parameters with value

	static const uint32_t NUM_BUCKETS = 128;
	int m_buckets[NUM_BUCKETS];

    struct BucketHash
    {
        uint32_t hash = 0;
		uint32_t bucketIndex = 0;
    };

    static BucketHash MakeHash(StringView key);
};

//----

END_INFERNO_NAMESPACE()
