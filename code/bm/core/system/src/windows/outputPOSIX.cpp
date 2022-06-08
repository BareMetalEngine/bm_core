/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "private.h"
#include "output.h"
#include "outputStream.h"

#ifdef PLATFORM_POSIX

#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

BEGIN_INFERNO_NAMESPACE()

//-----------------------------------------------------------------------------

static void FormatLineForConsolePrinting(IFormatStream& f, const LogOutputLevel level, const char* file, uint32_t line, const char* text)
{
    if (level == LogOutputLevel::Error || level == LogOutputLevel::Fatal)
        f << "\x1B[1;31m";
    else if (level == LogOutputLevel::Spam)
        f << "\x1B[30;1m";
    else if (level == LogOutputLevel::Warning)
        f << "\x1B[1;33m";

    switch (level)
    {
        case LogOutputLevel::Spam: f.append("[S]"); break;
        case LogOutputLevel::Info: f.append("[I]"); break;
        case LogOutputLevel::Warning: f.append("[W]"); break;
        case LogOutputLevel::Error: f.append("[E]"); break;
        case LogOutputLevel::Fatal: f.append("[F]"); break;
    }

    // (GPrintTimestamp)
    {
        uint8_t h = 0, m = 0, s = 0;

        const auto now = TimeStamp::GetNow();
        now.unpackTimePart(h, m, s);
        f.appendf("[{}:{}:{}]", h, m ,s);
    }

    f.append(" ");
    f.append(text);
    f.append("\x1B[0m");
}

class LocalStringBuffer : public IFormatStream
{
public:
    static const auto MAX_SIZE = 1200; // max log + file name

    INLINE void reset()
    {
        m_index = 0;
    }

    INLINE const char* c_str() const
    {
        return m_buffer;
    }

    INLINE uint32_t length() const
    {
        return m_index;
    }

    virtual IFormatStream& append(const char* str, uint32_t len /*= INDEX_MAX*/) override
    {
        if (str && *str)
        {
            if (len == INDEX_MAX)
                len = strlen(str);

            auto maxWrite = std::min<uint32_t>(MAX_SIZE - m_index, len);
            memcpy(m_buffer + m_index, str, maxWrite);
            m_index += maxWrite;
            m_buffer[m_index + 0] = '\r';
            m_buffer[m_index + 1] = '\n';
            m_buffer[m_index + 2] = 0;
        }

        return *this;
    }

private:
    char m_buffer[MAX_SIZE+4];
    int m_index = 0;
};

//-----------------------------------------------------------------------------

TYPE_TLS LocalStringBuffer* GPrintBuffer = nullptr;

class PosixDebugOutput : public ILogSink
{
public:
    PosixDebugOutput()
    {
        Log::AttachGlobalSink(this);
    }

    virtual bool print(LogOutputLevel level, const char* file, uint32_t line, const char* text) override
    {
        if (level == LogOutputLevel::Meta)
            return false;

        if (level == LogOutputLevel::Spam && !m_verbose)
            return false;

        if (!GPrintBuffer)
            GPrintBuffer = new LocalStringBuffer();

        GPrintBuffer->reset();
        FormatLineForConsolePrinting(*GPrintBuffer, level, file, line, text);
        fprintf(stderr, "%s", GPrintBuffer->c_str());

        return false;
    }

    bool m_verbose = false;
};

static PosixDebugOutput GPosixOutput;

//-----------------------------------------------------------------------------

bool Log::Assert(bool isFatal, const char* fileName, uint32_t fileLine, const char* expr, const char* message, bool* isEnabled)
{
    SinkTable::GetInstance().print(LogOutputLevel::Fatal, fileName, fileLine, TempString("Assertion failed in '{}'", expr));
    SinkTable::GetInstance().print(LogOutputLevel::Fatal, fileName, fileLine, message);
    return true;
}

bool Log::FatalError(const char* fileName, uint32_t fileLine, const char* text)
{
    SinkTable::GetInstance().print(LogOutputLevel::Fatal, fileName, fileLine, text);
    return true;
}

//-----------------------------------------------------------------------------

END_INFERNO_NAMESPACE()

#endif