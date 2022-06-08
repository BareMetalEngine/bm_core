/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "bm/core/system/include/formatBuffer.h"

BEGIN_INFERNO_NAMESPACE()

//--

IProgressTracker::~IProgressTracker()
{}

//--

FiberProgressTracker::FiberProgressTracker(IProgressTracker& progres, uint32_t total, const char* text)
    : m_progress(progres)
    , m_total(total)
    , m_counter(0)
    , m_text((text && *text) ? text : "Processing {}/{}...")
{
    m_nextAllowedUpdate.resetToNow();
    m_progress.reportProgress(0, m_total, TempString(m_text, 0, m_total));
}

void FiberProgressTracker::reportProgress(uint64_t currentCount, uint64_t totalCount, StringView text)
{
    // nothing happens here
}

bool FiberProgressTracker::checkCancelation() const
{
    return m_progress.checkCancelation();
}

void FiberProgressTracker::advance(uint32_t count /*= 1*/)
{
    auto finished = (m_counter += count) + count;
    if (finished >= m_total || m_nextAllowedUpdate.reached())
    {
        m_progress.reportProgress(finished, m_total, TempString(m_text, finished, m_total));
        m_nextAllowedUpdate = NativeTimePoint::Now() + 0.05;
    }
}

//--

class DevNullProgressTracker : public IProgressTracker
{
public:
    DevNullProgressTracker() {};
    virtual bool checkCancelation() const override final { return false; }
    virtual void reportProgress(uint64_t currentCount, uint64_t totalCount, StringView text) override final { };
};

//--

static DevNullProgressTracker theNullProgressTracker;

IProgressTracker& IProgressTracker::DevNull()
{
    return theNullProgressTracker;
}

//--

END_INFERNO_NAMESPACE()
