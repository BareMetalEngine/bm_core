/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

BEGIN_INFERNO_NAMESPACE()

/// TimeStamp for file
class BM_CORE_SYSTEM_API TimeStamp
{
public:
    INLINE TimeStamp()
        : m_timecode(0)
    {}

    explicit INLINE TimeStamp(uint64_t timeCode)
        : m_timecode(timeCode)
    {
    }

    INLINE bool operator==(const TimeStamp &stamp) const
    {
        return m_timecode == stamp.m_timecode;
    }

    INLINE bool operator!=(const TimeStamp &stamp) const
    {
        return m_timecode != stamp.m_timecode;
    }

    INLINE bool operator<(const TimeStamp &stamp) const
    {
        return m_timecode < stamp.m_timecode;
    }

    INLINE bool operator>(const TimeStamp &stamp) const
    {
        return m_timecode > stamp.m_timecode;
    }

    INLINE bool operator<=(const TimeStamp &stamp) const
    {
        return m_timecode <= stamp.m_timecode;
    }

    INLINE bool operator>=(const TimeStamp &stamp) const
    {
        return m_timecode >= stamp.m_timecode;
    }

    INLINE uint64_t hash() const
    {
        return m_timecode;
    }

    INLINE uint64_t value() const
    {
        return m_timecode;
    }

    INLINE bool empty() const
    {
        return m_timecode == 0;
    }

    //--

    //! Get current date/time
    static TimeStamp GetNow();

    //! Get date/time from seconds/nano seconds
    static TimeStamp GetFromFileTime(uint64_t seconds, uint64_t nanoSeconds);

    //---

    // print as string
    void print(IFormatStream& f) const;

    // print if file-system safe format (no strange chars)
    void printSafe(IFormatStream& f) const;

    //--

    // unpack time part
    void unpackTimePart(uint8_t& outHour24, uint8_t& outMinute, uint8_t& outSeconds) const;

    //--

private:
    uint64_t m_timecode = 0;
};

END_INFERNO_NAMESPACE()
