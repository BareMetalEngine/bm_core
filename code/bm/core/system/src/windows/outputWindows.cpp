/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "private.h"

#include "errorWindowWindows.h"
#include "bm/core/system/include/debug.h"

#ifdef PLATFORM_WINDOWS

//-----------------------------------------------------------------------------

// Crash report file
#define CRASH_REPORT_FILE       "InfernoCrash.txt"

// Mini dump file
#define CRASH_DUMP_FILE         "InfernoCrash.dmp"

//-----------------------------------------------------------------------------

// GetModuleBaseName
#include <psapi.h>
#include <DbgHelp.h>

#pragma comment ( lib, "dbghelp.lib" )
#pragma comment ( lib, "Psapi.lib" )

//-----------------------------------------------------------------------------

BEGIN_INFERNO_NAMESPACE()

//-----------------------------------------------------------------------------

namespace helper
{
    static bool GetExceptionDesc(EXCEPTION_POINTERS* pExcPtrs, const std::function<void(const char* buffer)>& func)
    {
        DWORD exceptionCode = pExcPtrs->ExceptionRecord->ExceptionCode;
        switch (exceptionCode)
        {
            case EXCEPTION_ACCESS_VIOLATION:
            {
                if (pExcPtrs->ExceptionRecord->NumberParameters >= 2)
                {
                    const char* operation = (pExcPtrs->ExceptionRecord->ExceptionInformation[0] == 0) ? "reading" : "writing";
                    auto address  = pExcPtrs->ExceptionRecord->ExceptionInformation[1];
                    func(TempString("EXCEPTION_ACCESS_VIOLATION, Error {} location {}", operation, address));
                }
                else
                {
                    func("EXCEPTION_ACCESS_VIOLATION");
                }
                break;
            }

            case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
            {
                func("EXCEPTION_ARRAY_BOUNDS_EXCEEDED");
                break;
            }

            case EXCEPTION_BREAKPOINT:
            {
                func("EXCEPTION_BREAKPOINT");
                break;
            }

            case EXCEPTION_DATATYPE_MISALIGNMENT:
            {
                func("EXCEPTION_DATATYPE_MISALIGNMENT");
                break;
            }

            case EXCEPTION_FLT_DENORMAL_OPERAND:
            {
                func("EXCEPTION_FLT_DENORMAL_OPERAND");
                break;
            }

            case EXCEPTION_FLT_DIVIDE_BY_ZERO:
            {
                func("EXCEPTION_FLT_DIVIDE_BY_ZERO");
                break;
            }

            case EXCEPTION_FLT_INEXACT_RESULT:
            {
                func("EXCEPTION_FLT_INEXACT_RESULT");
                break;
            }

            case EXCEPTION_FLT_INVALID_OPERATION:
            {
                func("EXCEPTION_FLT_INVALID_OPERATION");
                break;
            }

            case EXCEPTION_FLT_OVERFLOW:
            {
                func("EXCEPTION_FLT_OVERFLOW");
                break;
            }

            case EXCEPTION_FLT_STACK_CHECK:
            {
                func("EXCEPTION_FLT_STACK_CHECK");
                break;
            }

            case EXCEPTION_FLT_UNDERFLOW:
            {
                func("EXCEPTION_FLT_UNDERFLOW");
                break;
            }

            case EXCEPTION_GUARD_PAGE:
            {
                func("EXCEPTION_GUARD_PAGE");
                break;
            }

            case EXCEPTION_ILLEGAL_INSTRUCTION:
            {
                func("EXCEPTION_ILLEGAL_INSTRUCTION");
                break;
            }

            case EXCEPTION_IN_PAGE_ERROR:
            {
                func("EXCEPTION_IN_PAGE_ERROR");
                break;
            }

            case EXCEPTION_INT_DIVIDE_BY_ZERO:
            {
                func("EXCEPTION_INT_DIVIDE_BY_ZERO");
                break;
            }

            case EXCEPTION_INT_OVERFLOW:
            {
                func("EXCEPTION_INT_OVERFLOW");
                break;
            }

            case EXCEPTION_INVALID_DISPOSITION:
            {
                func("EXCEPTION_INVALID_DISPOSITION");
                break;
            }

            case EXCEPTION_INVALID_HANDLE:
            {
                func("EXCEPTION_INVALID_HANDLE");
                break;
            }

            case EXCEPTION_NONCONTINUABLE_EXCEPTION:
            {
                func("EXCEPTION_NONCONTINUABLE_EXCEPTION");
                break;
            }

            case EXCEPTION_PRIV_INSTRUCTION:
            {
                func("EXCEPTION_PRIV_INSTRUCTION");
                break;
            }

            case EXCEPTION_SINGLE_STEP:
            {
                func("EXCEPTION_SINGLE_STEP");
                break;
            }

            case EXCEPTION_STACK_OVERFLOW:
            {
                func("EXCEPTION_STACK_OVERFLOW");
                break;
            }

            default:
            {
                func("<unknown-exception>");
                return false;
            }
        }

        return true;
    }

    class ScopeLock
    {
    public:
        INLINE ScopeLock(CRITICAL_SECTION& section)
            : m_section(&section)
        {
            EnterCriticalSection(m_section);
        }

        INLINE ~ScopeLock()
        {
            LeaveCriticalSection(m_section);
        }
                        
    private:
        CRITICAL_SECTION*   m_section;
    };
} // helper

//-----------------------------------------------------------------------------

static void FormatLineForDebugOutputPrinting(IFormatStream& f, const LogOutputLevel level, const char* file, uint32_t line, const char* text)
{
    if (file && *file)
    {
        f.append(file);

        if (line != 0)
            f.appendf("({}): ", line);
        else
            f.append(": ");
    }

    switch (level)
    {
        case LogOutputLevel::Spam: f.append("spam"); break;
        case LogOutputLevel::Info: f.append("info"); break;
        case LogOutputLevel::Warning: f.append("warning"); break;
        case LogOutputLevel::Error: f.append("error"); break;
        case LogOutputLevel::Fatal: f.append("fatal"); break;
    }

    f.append(": ");
    f.append(text);
}

static void FormatLineForConsolePrinting(IFormatStream& f, const LogOutputLevel level, const char* file, uint32_t line, const char* text)
{
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
    f.append("\r\n");
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

class WindowsDebugOutput : public ILogSink
{
public:
    WindowsDebugOutput()
    {
        Log::AttachGlobalSink(this);
        m_stdOut = ::GetStdHandle(STD_OUTPUT_HANDLE);
        m_debugEcho = ::IsDebuggerPresent();
    }

    bool print(LogOutputLevel level, const char* file, uint32_t line, const char* text) override
    {
        if (level == LogOutputLevel::Meta)
            return false;

        if (level == LogOutputLevel::Spam && !m_verbose)
            return false;

        if (!GPrintBuffer)
            GPrintBuffer = new LocalStringBuffer();

        if (m_debugEcho)
        {
            GPrintBuffer->reset();
            FormatLineForDebugOutputPrinting(*GPrintBuffer, level, file, line, text);
            OutputDebugStringA(GPrintBuffer->c_str());
        }

#if 0
        fprintf(stdout, "%hs", GPrintBuffer->c_str());
#else
        if (m_stdOut != INVALID_HANDLE_VALUE)
        {
            if (level == LogOutputLevel::Fatal)
                SetConsoleTextAttribute(m_stdOut, BACKGROUND_RED | 0); // black text on red background
            else if (level == LogOutputLevel::Error)
                SetConsoleTextAttribute(m_stdOut, 12);
            else if (level == LogOutputLevel::Warning)
                SetConsoleTextAttribute(m_stdOut, 14);
            else if (level == LogOutputLevel::Spam)
                SetConsoleTextAttribute(m_stdOut, 8); // dark gray text
            else
                SetConsoleTextAttribute(m_stdOut, 7); // gray text
        }

        {
            if (m_stdOut != INVALID_HANDLE_VALUE)
            {
                GPrintBuffer->reset();
                FormatLineForConsolePrinting(*GPrintBuffer, level, file, line, text);

                // Write to stdout
                DWORD written = 0;
                WriteFile(m_stdOut, GPrintBuffer->c_str(), GPrintBuffer->length(), &written, NULL);
            }
        }

        if (m_stdOut != INVALID_HANDLE_VALUE)
        {
            if ((uint8_t)level >= (uint8_t)LogOutputLevel::Warning)
                FlushFileBuffers(m_stdOut);

            if (level != LogOutputLevel::Info)
                SetConsoleTextAttribute(m_stdOut, 7); // gray text
        }
#endif

        return false;
    }

    HANDLE m_stdOut = NULL;
    bool m_debugEcho = false;
    bool m_verbose = false;
};

static WindowsDebugOutput GWindowsOutput;

//--

class WindowsErrorDialog : public NoCopy
{
public:
    WindowsErrorDialog()
    {
        ::InitializeCriticalSection(&m_lock);

        AddVectoredExceptionHandler(1, &VectoredExceptionHandler);
    }

    static LONG VectoredExceptionHandler(_EXCEPTION_POINTERS* ExceptionInfo);
    
    bool handleDialog(ErrorHandlerDlg& dlg, EXCEPTION_POINTERS* pExcPtrs, bool* isEnabledFlag)
    {
        // We cannot disable the assert if we have no flag
        if (isEnabledFlag == nullptr)
            dlg.m_canDisable = false;

        // Show dialog
        auto res = dlg.showDialog();

        // Process result
        switch (res)
        {
            case ErrorHandlerDlg::EResult::Disable:
            {
                if (isEnabledFlag) *isEnabledFlag = false;
                return false;
            }

            case ErrorHandlerDlg::EResult::Ignore:
            {
                return false;
            }

            case ErrorHandlerDlg::EResult::Break:
            {
                if (!IsDebuggerPresent())
                {
                    ToggleWERPopup(true);
                    if (pExcPtrs)
                        ::RaiseFailFastException(pExcPtrs->ExceptionRecord, pExcPtrs->ContextRecord, 0);
                    else
                        ::RaiseFailFastException(nullptr, nullptr, FAIL_FAST_GENERATE_EXCEPTION_ADDRESS);

                    return false;
                }
                else
                {
                    return true;
                }
            }

            case ErrorHandlerDlg::EResult::Exit:
            {
                crash(pExcPtrs, false);
                return true;
            }
        }

        return false;
    }

    void crash(EXCEPTION_POINTERS* pExcPtrs, bool panic)
    {
        if (panic)
        {
            if (pExcPtrs)
            {
                __try
                {
                    ::RaiseFailFastException(pExcPtrs->ExceptionRecord, pExcPtrs->ContextRecord, 0);
                }
                __except (EXCEPTION_EXECUTE_HANDLER)
                {
                }
            }

            ::RaiseFailFastException(nullptr, nullptr, FAIL_FAST_GENERATE_EXCEPTION_ADDRESS);
        }
        else
        {
            ::SetErrorMode(::GetErrorMode() | SEM_NOGPFAULTERRORBOX);
            ::TerminateProcess(::GetCurrentProcess(), 1);
        }
    }

    bool handleAssert(bool isFatal, const char* fileName, uint32_t fileLine, const char* expr, const char* msg, bool* isEnabled)
    {
        // make sure only one assertion dialog is displayed
        helper::ScopeLock lock(m_lock);
        ErrorHandlerDlg dlg;

        // print to log
        if (fileName && *fileName)
        {
            Log::Print(::bm::LogOutputLevel::Fatal, fileName, fileLine, "Assertion failed!");
            Log::Print(::bm::LogOutputLevel::Fatal, fileName, fileLine, expr);

            if (msg && *msg)
                Log::Print(::bm::LogOutputLevel::Fatal, fileName, fileLine, msg);
        }
        else
        {
            TRACE_STREAM(::bm::LogOutputLevel::Fatal).appendf("Assertion failed: {}\n", expr);
        }

        // we cannot disable fatal asserts
        dlg.m_canContinue = !isFatal;
        dlg.m_canDisable = !isFatal;

        // Set message
        dlg.assertMessage(fileName, fileLine, expr, msg);

        // Set callstack
        DebugCallstack callstack;
        if (GrabCallstack(3, nullptr, callstack))
            dlg.callstack(callstack);

        // Display and process the dialog
        return handleDialog(dlg, nullptr, isEnabled);
    }

    bool handleFatalError(const char* fileName, uint32_t fileLine, const char* msg)
    {
        // make sure only one assertion dialog is displayed
        helper::ScopeLock lock(m_lock);

        ErrorHandlerDlg dlg;

        // print to log
        if (fileName && *fileName)
        {
            Log::Print(::bm::LogOutputLevel::Fatal, fileName, fileLine, "Fatal error!");
            Log::Print(::bm::LogOutputLevel::Fatal, fileName, fileLine, msg);
        }
        else
        {
            TRACE_STREAM(::bm::LogOutputLevel::Fatal).appendf("Fatal error: {}\n", msg);
        }

#ifdef BUILD_RELEASE
        dlg.m_canContinue = false;
        dlg.m_canDisable = false;
#else
        dlg.m_canContinue = true;
        dlg.m_canDisable = false;
#endif

        // Format message
        dlg.message(msg);

        // Set callstack
        DebugCallstack callstack;
        if (GrabCallstack(2, nullptr, callstack))
            dlg.callstack(callstack);

        // Display and process the dialog
        return handleDialog(dlg, nullptr, nullptr);
    }

    int handleException(EXCEPTION_POINTERS* exception)
    {
        // no exception information
        if (exception == nullptr)
            return EXCEPTION_CONTINUE_SEARCH;

        helper::ScopeLock lock(m_lock);

        ErrorHandlerDlg dlg;
        dlg.m_canDisable = false;

        // Format message based on exception rules
        if (!helper::GetExceptionDesc(exception, [&dlg](const char* txt) { dlg.message(txt); }))
            return EXCEPTION_CONTINUE_EXECUTION;

#ifdef BUILD_RELEASE
        dlg.m_canContinue = false;
#else
        dlg.m_canContinue = true;
#endif

        // Set callstack
        DebugCallstack callstack;
        if (GrabCallstack(6, exception, callstack))
            dlg.callstack(callstack);

        // Additional log to console
        TempString locationInfo;
        if (callstack.printFrame(0, locationInfo))
        {
            helper::GetExceptionDesc(exception, [&locationInfo](const char* txt) {
                Log::Print(::bm::LogOutputLevel::Fatal, locationInfo.c_str(), 0, txt);
                });
        }

        // Display and process the dialog
        if (handleDialog(dlg, exception, nullptr))
            return EXCEPTION_EXECUTE_HANDLER;
        else
            return EXCEPTION_CONTINUE_EXECUTION;
    }

    //-----------------------------------------------------------------------------

    bool generateDump(EXCEPTION_POINTERS* pExcPtrs)
    {
        // Open dump file
        HANDLE hFile = CreateFileA(CRASH_DUMP_FILE, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE)
        {
            // SetupMetadata exception info
            MINIDUMP_EXCEPTION_INFORMATION exceptionInfo;
            exceptionInfo.ThreadId = GetCurrentThreadId();
            exceptionInfo.ExceptionPointers = pExcPtrs;
            exceptionInfo.ClientPointers = FALSE;

            // Write mini dump of the crash :)
            bool state = 0 != MiniDumpWriteDump(
                GetCurrentProcess(),
                GetCurrentProcessId(),
                hFile,
                MiniDumpWithDataSegs,
                pExcPtrs ? &exceptionInfo : NULL,
                NULL, NULL);

            // Close dump file
            CloseHandle(hFile);

            // Dump saved
            return state;
        }

        // Dump not saved
        return false;
    }

    static void ToggleWERPopup(bool value)
    {
        HKEY hWerKey = nullptr;
        if (::RegCreateKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\Windows Error Reporting", 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, nullptr, &hWerKey, nullptr) == ERROR_SUCCESS)
        {
            DWORD regVal = value ? 0 : 1; // if enable, then zero to NOT disable
            ::RegSetValueExW(hWerKey, L"DontShowUI", 0, REG_DWORD, reinterpret_cast<const BYTE*>(&regVal), sizeof(regVal));
            ::CloseHandle(hWerKey);
            hWerKey = nullptr;
        }
    }

private:
    CRITICAL_SECTION m_lock;
};

static WindowsErrorDialog GWindowsError;

LONG WindowsErrorDialog::VectoredExceptionHandler(_EXCEPTION_POINTERS* ExceptionInfo)
{
    return GWindowsError.handleException(ExceptionInfo);
}

bool Log::Assert(bool isFatal, const char* fileName, uint32_t fileLine, const char* expr, const char* message, bool* isEnabled)
{
    return GWindowsError.handleAssert(isFatal, fileName, fileLine, expr, message, isEnabled);
}

bool Log::FatalError(const char* fileName, uint32_t fileLine, const char* text)
{
    return GWindowsError.handleFatalError(fileName, fileLine, text);
}

//-----------------------------------------------------------------------------

END_INFERNO_NAMESPACE()

#endif