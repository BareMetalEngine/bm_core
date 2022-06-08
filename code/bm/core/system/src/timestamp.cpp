/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "timestamp.h"
#include "private.h"

#if defined( PLATFORM_PSX )
    #include "rtc.h"
    #pragma comment (lib, "libSceRtc_stub_weak.a" )
#endif

BEGIN_INFERNO_NAMESPACE()

void TimeStamp::unpackTimePart(uint8_t& outHour24, uint8_t& outMinute, uint8_t& outSeconds) const
{
#ifdef PLATFORM_WINDOWS
    // Convert to file time
    SYSTEMTIME stUTC, stLocal;

    // Convert the last-write time to local time.
    FileTimeToSystemTime((FILETIME*)&m_timecode, &stUTC);
    SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);

    // Build a string showing the date and time.
    outHour24 = stLocal.wHour;
    outMinute = stLocal.wMinute;
    outSeconds = stLocal.wSecond;
#elif defined(PLATFORM_POSIX)
    // get the time64_t
    time_t t = m_timecode / 10000000ULL - 11644473600ULL;

    // get string representation
    tm timeData;
    auto timeDataPtr = gmtime_r(&t, &timeData);

    // Build a string showing the date and time.
    outHour24 = timeDataPtr->tm_hour;
    outMinute = timeDataPtr->tm_min;
    outSeconds = timeDataPtr->tm_sec;
#elif defined(PLATFORM_PSX)
    SceRtcDateTime dt;
    sceRtcSetTick(&dt, (const SceRtcTick*)&m_timecode);
    outHour24 = dt.hour;
    outMinute = dt.minute;
    outSeconds = dt.second;
#else
    #error "Add platform crap"
#endif
}

void TimeStamp::print(IFormatStream& f) const
{
#ifdef PLATFORM_WINDOWS

    // Convert to file time
    SYSTEMTIME stUTC, stLocal;

    // Convert the last-write time to local time.
    FileTimeToSystemTime((FILETIME*)&m_timecode, &stUTC);
    SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);

    // Build a string showing the date and time.
    f.appendf("{}/{}/{} {}:{}:{}", stLocal.wYear, stLocal.wMonth, stLocal.wDay, stLocal.wHour, stLocal.wMinute, stLocal.wSecond);
#elif defined(PLATFORM_POSIX)
    // get the time64_t
    time_t t = m_timecode / 10000000ULL - 11644473600ULL;

    // get string representation
    tm timeData;
    auto timeDataPtr = gmtime_r(&t, &timeData);

    // build string
    f.appendf("{}/{}/{} {}:{}:{}", timeDataPtr->tm_year + 1900, timeDataPtr->tm_mon + 1, timeDataPtr->tm_mday, timeDataPtr->tm_hour, timeDataPtr->tm_min, timeDataPtr->tm_sec);
#elif defined(PLATFORM_PSX)
    SceRtcDateTime dt;
    sceRtcSetTick(&dt, (const SceRtcTick*)&m_timecode);
    f.appendf("{}/{}/{} {}:{}:{}.{}", dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second, dt.microsecond);
#else
    f.append("{}", m_timecode);
#endif
}

void TimeStamp::printSafe(IFormatStream& f) const
{
#ifdef PLATFORM_WINDOWS

    // Convert to file time
    SYSTEMTIME stUTC, stLocal;

    // Convert the last-write time to local time.
    FileTimeToSystemTime((FILETIME*)&m_timecode, &stUTC);
    SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);

    // Build a string showing the date and time.
    f.appendf("{}_{}_{}-{}_{}_{}", stLocal.wYear, stLocal.wMonth, stLocal.wDay, stLocal.wHour, stLocal.wMinute, stLocal.wSecond);

#elif defined(PLATFORM_POSIX)

    // get the time64_t
    time_t t = m_timecode / 10000000ULL - 11644473600ULL;

    // get string representation
    tm timeData;
    auto timeDataPtr  = gmtime_r(&t, &timeData);

    // build string
    f.appendf("{}_{}_{}-{}_{}_{}", timeDataPtr->tm_year + 1900, timeDataPtr->tm_mon + 1, timeDataPtr->tm_mday, timeDataPtr->tm_hour, timeDataPtr->tm_min, timeDataPtr->tm_sec);

#elif defined(PLATFORM_PSX)
    SceRtcDateTime dt;
    sceRtcSetTick(&dt, (const SceRtcTick*)&m_timecode);
    f.appendf("{}_{}_{}-{}_{}_{}", dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second);
#else
    f.append("{}", m_timecode);
#endif
}

TimeStamp TimeStamp::GetNow()
{
#ifdef PLATFORM_WINDOWS
    SYSTEMTIME stUTC;
    GetSystemTime(&stUTC);

    FILETIME fileTime;
    SystemTimeToFileTime(&stUTC, &fileTime);

    ULARGE_INTEGER ull;
    ull.LowPart = fileTime.dwLowDateTime;
    ull.HighPart = fileTime.dwHighDateTime;

    uint64_t val = *(uint64_t*)&fileTime;
    return TimeStamp(val);
#elif defined(PLATFORM_POSIX)
    time_t t;
    time(&t);

    uint64_t val = (uint64_t)t * 10000000 + 116444736000000000;
    return TimeStamp(val);
#elif defined(PLATFORM_PSX)
    SceRtcTick tick;
    sceRtcGetCurrentTick(&tick);
    return TimeStamp(tick.tick);
#else
    #error "Add platform crap"
#endif
}

TimeStamp TimeStamp::GetFromFileTime(uint64_t seconds, uint64_t nanoSeconds)
{
    uint64_t val = (uint64_t)seconds * 10000000 + 116444736000000000;
    return TimeStamp(val);
}

END_INFERNO_NAMESPACE()
