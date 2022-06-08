/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "timing.h"
#include "private.h"

#if defined( PLATFORM_PSX )
#include "rtc.h"
#pragma comment (lib, "libSceRtc_stub_weak.a" )
#endif

BEGIN_INFERNO_NAMESPACE()

//-----

NativeTimePoint NativeTimePoint::Now()
{
    NativeTimePoint ret;
#if defined(PLATFORM_WINDOWS)
    QueryPerformanceCounter((LARGE_INTEGER*)&ret.m_value);
#elif defined(PLATFORM_LINUX)
    timespec sp;
    clock_gettime(CLOCK_MONOTONIC_RAW, &sp);
    ret.m_value = sp.tv_nsec  + sp.tv_sec * 1000000000LLU;
#elif defined(PLATFORM_PSX)
    ::sceRtcGetCurrentTick((SceRtcTick*) &ret.m_value);
#else
    #error "Add platform crap"
#endif
    return ret;
}

NativeTimeInterval NativeTimePoint::timeTillNow() const
{
    return Now() - *this;
}

NativeTimeInterval NativeTimePoint::timeTillNowAndReset()
{
    const auto now = Now();
    const auto ret = now - *this;
    *this = now;
    return ret;
}

void NativeTimePoint::print(IFormatStream& f) const
{
    f << TimeInterval(timeTillNow().toSeconds());
}

void NativeTimePoint::resetToNow()
{
    *this = Now();
}

bool NativeTimePoint::reached() const
{
    return Now() > *this;
}

//-----

#if defined(PLATFORM_WINDOWS)
static NativeTimeInterval::TDelta GetNativeTimeFreq()
{
    NativeTimeInterval::TDelta ret = 1;
    QueryPerformanceFrequency((LARGE_INTEGER*)&ret);
    return ret;
}
#elif defined(PLATFORM_LINUX)
static NativeTimeInterval::TDelta GetNativeTimeFreq()
{
    return NativeTimeInterval::TDelta(1000000000LLU);
}
#elif defined(PLATFORM_PSX)
static NativeTimeInterval::TDelta GetNativeTimeFreq()
{
    return ::sceRtcGetTickResolution();
}
#else
#error "Add platform crap"
#endif

static NativeTimeInterval::TDelta GNativeTimeFreq = GetNativeTimeFreq();
static double GNativeTimeFreqInv = 1.0 / (double)GetNativeTimeFreq();

NativeTimeInterval::NativeTimeInterval(double delta)
{
    m_delta = (TDelta)(delta  * (double)GNativeTimeFreq);
}

double NativeTimeInterval::toSeconds() const
{
    return (double)m_delta * GNativeTimeFreqInv;
}

float NativeTimeInterval::toMiliSeconds() const
{
    return (float)(toSeconds() * 1000.0);
}

void NativeTimeInterval::print(IFormatStream& f) const
{
    f << TimeInterval(toSeconds());
}

END_INFERNO_NAMESPACE()
