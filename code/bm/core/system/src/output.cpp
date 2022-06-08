/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "output.h"
#include "outputStream.h"

BEGIN_INFERNO_NAMESPACE()

//--

static TYPE_TLS LineAssembler* GLogLineStream = nullptr;

//--

ILogSink::~ILogSink()
{}

//--

LocalLogSink::LocalLogSink()
{
    m_previousLocalSink = Log::MountLocalSink(this);
}

LocalLogSink::~LocalLogSink()
{
    auto* prev = Log::MountLocalSink(m_previousLocalSink);
    DEBUG_CHECK_EX(prev == this, "Something went wrong with local log sinks attach/detach order");
}

bool LocalLogSink::print(LogOutputLevel level, const char* file, uint32_t line, const char* text)
{
    if (m_previousLocalSink)
        return m_previousLocalSink->print(level, file, line, text);
    else
        return false;
}

//--

GlobalLogSink::GlobalLogSink()
{
    Log::AttachGlobalSink(this);
}

GlobalLogSink::~GlobalLogSink()
{
    Log::DetachGlobalSink(this);
}

//--

IFormatStream& Log::Stream(LogOutputLevel level /*= LogOutputLevel::Info*/, const char* contextFile /*= nullptr*/, uint32_t contextLine /*= 0*/)
{
    if (nullptr == GLogLineStream)
        GLogLineStream = new LineAssembler(); // never released, but it's a global log FFS, don't bother

    GLogLineStream->takeOwnership(level, contextFile, contextLine);
    return *GLogLineStream;

    // TODO: return proxy object that will call "release ownership"
}

ILogSink* Log::MountLocalSink(ILogSink* sink)
{
    if (nullptr == GLogLineStream)
        GLogLineStream = new LineAssembler(); // never released, but it's a global log FFS, don't bother

    return GLogLineStream->mountLocalSink(sink);
}

ILogSink* Log::GetCurrentLocalSink()
{
    if (nullptr != GLogLineStream)
        return GLogLineStream->localSink();
    return nullptr;
}

void Log::AttachGlobalSink(ILogSink* sink)
{
    SinkTable::GetInstance().attach(sink);
}

void Log::DetachGlobalSink(ILogSink* sink)
{
    SinkTable::GetInstance().detach(sink);
}

void Log::Print(LogOutputLevel level, const char* file, uint32_t line, const char* text)
{
    SinkTable::GetInstance().print(level, file, line, text);
}

//--

END_INFERNO_NAMESPACE()
