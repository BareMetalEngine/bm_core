/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "scopeTimingBlock.h"

BEGIN_INFERNO_NAMESPACE()

//--

ScopeTimerTable::ScopeTimerTable()
{
    m_start = NativeTimePoint::Now();
    m_blockStart = NativeTimePoint::Now();
}

void ScopeTimerTable::printToLog() const
{
    print(TRACE_STREAM_INFO());
}

void ScopeTimerTable::print(IFormatStream& f) const
{
    auto totalTime = m_start.timeTillNow();

    if (!m_entries.empty())
    {
        f << "-------------------------------\n";

        for (const auto& entry : m_entries)
        {
            const auto timeElapsed = entry.finish - entry.start;
            f.appendf("  {}: {}\n", entry.name, timeElapsed);
        }
    }

    f << "-------------------------------\n";
    f.appendf("  TOTAL: {}\n", totalTime);
    f << "-------------------------------\n";
}

void ScopeTimerTable::finishRegion(StringView name)
{
    auto& entry = m_entries.emplaceBack();
    entry.start = m_blockStart;
    
    m_blockStart = NativeTimePoint::Now();
    entry.finish = m_blockStart;
}

//--

END_INFERNO_NAMESPACE()
