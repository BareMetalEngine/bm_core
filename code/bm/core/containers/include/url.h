/***
* Inferno Engine v4
* Written by Tomasz "RexDex" Jonarski
***/

#pragma once

BEGIN_INFERNO_NAMESPACE()

//----

/// general URL
struct BM_CORE_CONTAINERS_API URL
{
public:
    StringBuf protocol; // https
    StringBuf host; // example.com
    StringBuf path; // /test/bar
    StringBuf args; // foo=1&bar=5

    //--

    URL();
    URL(const URL& other);
    URL(URL&& other);
    URL& operator=(const URL& other);
    URL& operator=(URL&& other);

	URL(StringView host, StringView path, StringView args = "");

    //--

    // valid url ?
    INLINE operator bool() const { return !host.empty() && !path.empty(); }

	// URL is empty if host and path are empty
	INLINE bool empty() const { return host.empty() || path.empty(); }
    
    //--
    
    // make a formal URL: https://example.com/test/bar?foo=1&bar=5
    void print(IFormatStream& f) const;

    // parse from URL string
    static bool Parse(StringView txt, URL& outURL);

    //--

    // convert to string
    StringBuf toString(bool includeProtocol = true, bool includeArgs = true) const;

    //--
};

//----

END_INFERNO_NAMESPACE()
