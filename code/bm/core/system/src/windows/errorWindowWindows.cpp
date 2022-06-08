/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "private.h"
#include "errorWindowWindows.h"

#ifdef PLATFORM_WINDOWS

#include "../res/resources.h"

extern void* GModuleHandle;

BEGIN_INFERNO_NAMESPACE()

ErrorHandlerDlg::ErrorHandlerDlg()
    : m_canContinue(false)
    , m_canDisable(false)
{}

ErrorHandlerDlg::~ErrorHandlerDlg()
{}

void ErrorHandlerDlg::message(const char* txt)
{
    ConvertRF(txt, m_message);
}

static const char* ShorterFilePath(const char* file)
{
    if (!file)
        return nullptr;

    if (const auto* str = strstr(file, "/src/"))
        return str + 5;

	if (const auto* str = strstr(file, "\\src\\"))
		return str + 5;

    return file;
}

void ErrorHandlerDlg::assertMessage(const char* file, uint32_t line, const char* expr, const char* msg)
{
	if (msg && *msg)
		m_message << "ERROR:\r\n    " << msg << "\r\n";

    m_message << "EXPRESSION:\r\n    " << expr << "\r\n";

    if (file && *file)
	{
        file = ShorterFilePath(file);
        m_message << "FILE:\r\n    " << file << "(" << line << ")";
    }
}

void ErrorHandlerDlg::callstack(const char   * txt)
{
    ConvertRF(txt, m_callstack);
}

class StringStreamWrapper : public IFormatStream
{
public:
    StringStreamWrapper(std::stringstream& str)
        : m_ss(str)
    {}
        
    virtual IFormatStream& append(const char* str, uint32_t len) override
    {
        if (len == INDEX_MAX)
            len = strlen(str);
        m_ss << std::string_view(str, len);
        return *this;
    }

private:
    std::stringstream& m_ss;
};

void ErrorHandlerDlg::callstack(const DebugCallstack& callstack)
{
    StringStreamWrapper wrap(m_callstack);
	callstack.print(wrap, "\r\n");
}

ErrorHandlerDlg::EResult ErrorHandlerDlg::showDialog()
{
    auto retVal = DialogBoxParam((HINSTANCE)GModuleHandle, MAKEINTRESOURCE(IDD_ASSERT), NULL, &DlgProc, reinterpret_cast<LPARAM>(this));

    // Not shown - missing dialog
    if (retVal == -1)
    {
        auto errCode = GetLastError();
        TRACE_ERROR("Failed to display assert dialog, error code ={}", errCode);
        return EResult::Break;
    }

    return (EResult)retVal;
}

INT_PTR ErrorHandlerDlg::DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_INITDIALOG:
        {
            ErrorHandlerDlg* params = reinterpret_cast<ErrorHandlerDlg*>(lParam);

            HWND hExpression = GetDlgItem(hwndDlg, IDC_ERROR_TEXT);
            SetWindowTextA(hExpression, params->m_message.str().c_str());

            HWND hCallstack = GetDlgItem(hwndDlg, IDC_ERROR_CALLSTACK);
            SetWindowTextA(hCallstack, params->m_callstack.str().c_str());

            if (!params->m_canContinue)
            {
                HWND hBreakBtn = GetDlgItem(hwndDlg, IDC_IGNORE);
                EnableWindow(hBreakBtn, FALSE);
            }

            if (!params->m_canContinue || !params->m_canDisable)
            {
                HWND hBreakBtn = GetDlgItem(hwndDlg, IDC_DISABLE);
                EnableWindow(hBreakBtn, FALSE);
            }

            ShowWindow(hwndDlg, SW_SHOW);
            SetForegroundWindow(hwndDlg);
            return TRUE;
        }

        case WM_COMMAND:
        {
            switch (LOWORD(wParam))
            {
                case IDC_IGNORE: EndDialog(hwndDlg, (int)EResult::Ignore); return TRUE;
                case IDC_DISABLE: EndDialog(hwndDlg, (int)EResult::Disable); return TRUE;
                case IDC_BREAK: EndDialog(hwndDlg, (int)EResult::Break); return TRUE;
                case IDC_EXIT: EndDialog(hwndDlg, (int)EResult::Exit); return TRUE;
            }
        }
    }

    return FALSE;
}

void ErrorHandlerDlg::ConvertRF(const char *read, std::stringstream& str)
{
    while (*read)
    {
        auto ch = *read++;
		if (ch == '\n')
		{
			str << "\xd\xa";
		}
		else
		{
			char temp[2] = { ch, 0 };
			str << temp;
		}
    }
}

END_INFERNO_NAMESPACE()

#endif