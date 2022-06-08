/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

#ifdef PLATFORM_WINDOWS

#include <Windows.h>
#include <sstream>

BEGIN_INFERNO_NAMESPACE()

// Error handler window
class ErrorHandlerDlg
{
public:
    bool m_canContinue;
    bool m_canDisable;

    ErrorHandlerDlg();
    ~ErrorHandlerDlg();

    void message(const char* txt);
    void assertMessage(const char* file, uint32_t line, const char* expr, const char* msg);

    void callstack(const char* txt);
    void callstack(const DebugCallstack& callstack);

    enum class EResult : uint8_t
    {
        Ignore,
        Disable,
        Break,
        Exit,
    };

    EResult showDialog();

private:
    std::stringstream m_message;
    std::stringstream m_callstack;

    static INT_PTR CALLBACK DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static void ConvertRF(const char *read, std::stringstream& str);
};

END_INFERNO_NAMESPACE()

#endif