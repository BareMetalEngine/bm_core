/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "timing.h"
#include "scope.h"

BEGIN_INFERNO_NAMESPACE()

//---

ScopeTimer::ScopeTimer()
{
    (NativeTimePoint&)m_startTime = NativeTimePoint::Now();
}

NativeTimeInterval ScopeTimer::timeElapsedInterval() const
{
    return ((NativeTimePoint&)m_startTime).timeTillNow();
}

double ScopeTimer::timeElapsed() const
{
    return ((NativeTimePoint&)m_startTime).timeTillNow().toSeconds();
}

float ScopeTimer::milisecondsElapsed() const
{
	return ((NativeTimePoint&)m_startTime).timeTillNow().toMiliSeconds();
}

void ScopeTimer::print(IFormatStream& f) const
{
    f << TimeInterval(timeElapsed());
}

//--

END_INFERNO_NAMESPACE()
