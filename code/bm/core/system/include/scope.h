/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

BEGIN_INFERNO_NAMESPACE()

//--

class NativeTimeInterval;

//--

/// a timer that starts at the beginning of the scope
/// does not print anything
class BM_CORE_SYSTEM_API ScopeTimer
{
public:
    ScopeTimer();

    /// get time elapsed since the timer started
    double timeElapsed() const;

    /// get time elapsed since the timer started in mili seconds
    float milisecondsElapsed() const;

    /// elapsed time so far
    NativeTimeInterval timeElapsedInterval() const;

    //
        
    // prints as time since start
    void print(IFormatStream& f) const;

private:
    uint64_t m_startTime;
};

//--

/// simple boolean guard flag
struct ScopeBool : public NoCopy
{
public:
    INLINE ScopeBool(bool& flag)
        : m_flag(flag)
    {
        DEBUG_CHECK_EX(flag == false, "Flag expected to be false");
        flag = true;
    }

    INLINE ~ScopeBool()
    {
        DEBUG_CHECK_EX(m_flag == true, "Flag expected to be true");
        m_flag = false;
    }

private:
    bool& m_flag;
};

//--

END_INFERNO_NAMESPACE()
