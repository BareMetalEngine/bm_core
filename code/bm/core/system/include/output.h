/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

#include "debug.h"
#include "format.h"

BEGIN_INFERNO_NAMESPACE()

//-----------------------------------------------------------------------------

//! severity level of the output message
enum class LogOutputLevel : uint8_t
{
    Spam, // deep debug message
    Info, // general info
    Warning, // non critical warning
    Error, // non critical error (bad but we can continue)
    Fatal, // fatal error, we cannot continue, printing anything here stops the app
    Meta, // meta text for log, should not be displayed but may be used by the sinks, NOT FORMATED INTO LINES!!!!!!

    MAX,
};

//-----------------------------------------------------------------------------

// log sink, receives lines and lines only
class BM_CORE_SYSTEM_API ILogSink : public NoCopy
{
public:
    virtual ~ILogSink();

    // process a single (atomic) line of log message
    // NOTE: this function may be called from many threads and must be internally thread safe
    // NOTE: the function returns "consumed" flag and the local sinks may return true to stop propagation to GLOBAL log sink (ie. printing in log window/writing to log file)
    virtual bool print(LogOutputLevel level, const char* file, uint32_t line, const char* text) = 0;
};

//-----------------------------------------------------------------------------

// helper class that implements RAII local log sink
class BM_CORE_SYSTEM_API LocalLogSink : public ILogSink
{
public:
    LocalLogSink();
    virtual ~LocalLogSink();

    // default version of this function just passes the log line to parent local sink and does not consume anything
    virtual bool print(LogOutputLevel level, const char* file, uint32_t line, const char* text) override;

private:
    ILogSink* m_previousLocalSink = nullptr;
};

//-----------------------------------------------------------------------------

// helper class that implements RAII global log sink
class BM_CORE_SYSTEM_API GlobalLogSink : public ILogSink
{
public:
    GlobalLogSink();
    virtual ~GlobalLogSink();
};

//-----------------------------------------------------------------------------

//! logging "system"
class BM_CORE_SYSTEM_API Log : public NoCopy
{
public:
    /// report a log line to all global sink sources, can be called used to integrate better with other log sources that we don't want to route through the line assembler
    static void Print(LogOutputLevel level, const char* file, uint32_t line, const char* text);

    /// get a log stream to print line(s) to the log stream, if text contains multiple lines they are split, current indentation (spaces before first char) is preserved for each line
    static IFormatStream& Stream(LogOutputLevel level = LogOutputLevel::Info, const char* contextFile = nullptr, uint32_t contextLine = 0);

    /// insert a local (thread only) log sink, returns previous sink (that we may choose to ignore or to forward data to)
    static ILogSink* MountLocalSink(ILogSink* sink);

    /// get current local sink
    static ILogSink* GetCurrentLocalSink();

    /// attach global log sink
    static void AttachGlobalSink(ILogSink* sink);

    /// detach global log sink
    static void DetachGlobalSink(ILogSink* sink);

    // throw assertion, returns true if we should break
    static bool Assert(bool isFatal, const char* fileName, uint32_t fileLine, const char* expr, const char* message, bool* isEnabled);

    // report fatal error
    static bool FatalError(const char* fileName, uint32_t fileLine, const char* txt);
};

END_INFERNO_NAMESPACE()

//-----------------------------------------------------------------------------

#define TRACE_MAKE_TEXT(txt) #txt
#define TRACE_MAKE_TEXT2(txt) TRACE_MAKE_TEXT(txt)

#define TRACE_STREAM( lvl ) ::bm::Log::Stream(lvl, __FILE__, __LINE__)
#define TRACE_STREAM_SPAM() TRACE_STREAM(::bm::LogOutputLevel::Spam)
#define TRACE_STREAM_INFO() TRACE_STREAM(::bm::LogOutputLevel::Info)
#define TRACE_STREAM_WARNING() TRACE_STREAM(::bm::LogOutputLevel::Warning)
#define TRACE_STREAM_ERROR() TRACE_STREAM(::bm::LogOutputLevel::Error)

//-----------------------------------------------------------------------------

#ifdef BUILD_FINAL

#ifdef BUILD_CHECKED
    #define TRACE_ERROR( x, ... ) TRACE_STREAM_ERROR().appendf(x, ##__VA_ARGS__).append("\n");
#else
    #define TRACE_ERROR( x, ... ) { }
#endif

    #define TRACE_SPAM( x, ... ) { }
    #define TRACE_INFO( x, ... ) { }
    #define TRACE_WARNING( x, ... ) { }

#else

    #if defined(BUILD_DEBUG) || defined(BUILD_CHECKED)
        #define TRACE_INFO( x, ... ) TRACE_STREAM_INFO().appendf(x, ##__VA_ARGS__).append("\n");
        #define TRACE_SPAM( x, ... ) TRACE_STREAM_SPAM().appendf(x, ##__VA_ARGS__).append("\n");
    #else
        #define TRACE_SPAM( x, ... ) { }
        #define TRACE_INFO( x, ... ) { }
    #endif

    #define TRACE_WARNING( x, ... ) TRACE_STREAM_WARNING().appendf(x, ##__VA_ARGS__).append("\n");
    #define TRACE_ERROR( x, ... ) TRACE_STREAM_ERROR().appendf(x, ##__VA_ARGS__).append("\n");

#endif

#define FATAL_ERROR(x) if (::bm::Log::FatalError(__FILE__, __LINE__, x)) { INFERNO_DEBUG_BREAK(); }

//-----------------------------------------------------------------------------

/// Assertion macros
#if defined(BUILD_FINAL) || defined(BUILD_RELEASE)

#define DEBUG_CHECK(expr)  {}
#define DEBUG_CHECK_EX(expr,msg)  {}
#define ASSERT(expr) {}
#define ASSERT_EX(expr,msg) {}

#define FATAL_CHECK(expr) if (expr) {}

#define DEBUG_CHECK_RETURN( expr ) \
    if (!(expr)) return;

#define DEBUG_CHECK_RETURN_V( expr, ret ) \
    if (!(expr)) return (ret);

#define DEBUG_CHECK_RETURN_EX( expr, message ) \
    if (!(expr)) return;

#define DEBUG_CHECK_RETURN_EX_V( expr, message, ret ) \
    if (!(expr)) return ret;


#else

#define DEBUG_CHECK( expr ) { \
        static bool isEnabled = true; \
        if (!(expr) && isEnabled && ::bm::Log::Assert(false, __FILE__, __LINE__, #expr, NULL, &isEnabled)) INFERNO_DEBUG_BREAK(); }

#define DEBUG_CHECK_EX( expr, msg ) { \
        static bool isEnabled = true; \
        if (!(expr) && isEnabled && ::bm::Log::Assert(false,  __FILE__, __LINE__, #expr, msg, &isEnabled)) INFERNO_DEBUG_BREAK(); }

#define DEBUG_CHECK_RETURN( expr ) { \
        static bool isEnabled = true; \
        if (!(expr)) { \
            if (isEnabled && ::bm::Log::Assert(false,  __FILE__, __LINE__, #expr, "Required condition is not met", &isEnabled)) { INFERNO_DEBUG_BREAK(); } \
            return; } }

#define DEBUG_CHECK_RETURN_V( expr, ret ) { \
        static bool isEnabled = true; \
        if (!(expr)) { \
            if (isEnabled && ::bm::Log::Assert(false,  __FILE__, __LINE__, #expr, "Required condition is not met", &isEnabled)) { INFERNO_DEBUG_BREAK(); } \
            return (ret); } }

#define DEBUG_CHECK_RETURN_EX( expr, msg ) { \
        static bool isEnabled = true; \
        if (!(expr)) { \
            if (isEnabled && ::bm::Log::Assert(false,  __FILE__, __LINE__, #expr, msg, &isEnabled)) { INFERNO_DEBUG_BREAK(); } \
            return; } }

#define DEBUG_CHECK_RETURN_EX_V( expr, msg, ret ) { \
        static bool isEnabled = true; \
        if (!(expr)) { \
            if (isEnabled && ::bm::Log::Assert(false,  __FILE__, __LINE__, #expr, msg, &isEnabled)) { INFERNO_DEBUG_BREAK(); } \
            return (ret); } }

#define ASSERT( expr ) { \
    static bool isEnabled = true; \
    if (isEnabled && !(expr) && ::bm::Log::Assert(true, __FILE__, __LINE__, #expr, nullptr, &isEnabled)) { INFERNO_DEBUG_BREAK(); }}\

#define ASSERT_EX( expr, msg ) { \
    static bool isEnabled = true; \
    if (isEnabled && !(expr) && ::bm::Log::Assert(true, __FILE__, __LINE__, #expr, msg, &isEnabled)) { INFERNO_DEBUG_BREAK(); }}\

#endif

// Slow assertions (debug only)
#if !defined(BUILD_DEBUG)

#define DEBUG_CHECK_SLOW(expr) {}
#define DEBUG_CHECK_SLOW_EX(expr,msg) {}

#else

#define DEBUG_CHECK_SLOW(expr) {}
#define DEBUG_CHECK_SLOW_EX(expr,msg) {}

#endif

//-----------------------------------------------------------------------------

#define VALIDATION_RETURN( expr ) \
    if (!(expr)) return;

#define VALIDATION_RETURN_V( expr, ret ) \
    if (!(expr)) return (ret);

#define VALIDATION_RETURN_EX( expr, message ) \
    if (!(expr)) return;

#define VALIDATION_RETURN_EX_V( expr, message, ret ) \
    if (!(expr)) return ret;

//-----------------------------------------------------------------------------
