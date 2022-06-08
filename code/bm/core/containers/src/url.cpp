/***
* Inferno Engine v4
* Written by Tomasz "RexDex" Jonarski
***/

#include "build.h"
#include "url.h"
#include <locale>

BEGIN_INFERNO_NAMESPACE()

//---

static StringBuf theBasePath("/");
static StringBuf theBaseProtocol("https");

URL::URL()
{
    protocol = theBaseProtocol;
    path = theBasePath;
}

URL::URL(const URL& other) = default;
URL::URL(URL&& other) = default;
URL& URL::operator=(const URL& other) = default;
URL& URL::operator=(URL&& other) = default;

URL::URL(StringView host_, StringView path_, StringView args_)
    : host(host_)
    , path(path_)
    , args(args_)
{
}

//--

void URL::print(IFormatStream& f) const
{
    if (!empty())
    {
        if (protocol)
            f << protocol << "://";

        f << host;
        f << path;

        if (args)
        {
            f << "?";
            f << args;
        }
    }
}

StringBuf URL::toString(bool includeProtocol, bool includeArgs) const
{
    StringBuilder txt;

	if (!empty())
	{
		if (protocol && includeProtocol)
            txt << protocol << "://";

		txt << host;
		txt << path;

        if (includeArgs && !args.empty())
        {
            txt << "?";
            txt << args;
        }
	}

    return StringBuf(txt);
}

bool URL::Parse(StringView txt, URL& outURL)
{
    // TODO!!
    return false;
}

//---

END_INFERNO_NAMESPACE()