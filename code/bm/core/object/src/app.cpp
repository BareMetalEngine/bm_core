/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "app.h"

//----

#ifdef PLATFORM_WINDOWS
#include <Windows.h> 
#include <stdio.h> 

static BOOL WINAPI ConsoleHandler(DWORD signal)
{
    if (signal == CTRL_C_EVENT)
    {
        TRACE_WARNING("Ctrl-C detected");
        bm::App().requestExit();
    }

    return TRUE;
}

static void RegisterBreakHandler()
{
    if (SetConsoleCtrlHandler(&ConsoleHandler, TRUE))
    {
        TRACE_INFO("Ctrl-C handler registered");
    }
    else
    {
        TRACE_WARNING("Ctrl-C handler failed to register");
    }
}

static void UnregisterBreakHandler()
{
    if (SetConsoleCtrlHandler(&ConsoleHandler, FALSE))
    {
        TRACE_INFO("Ctrl-C handler unregistered");
    }
}
#else
static void SigHandler(int signo)
{
    TRACE_WARNING("Received SIGINT");
    bm::App().requestExit();
}

static void SigPipehandler(int sig)
{
    TRACE_INFO("Caught SIGPIPE");
}

static void RegisterBreakHandler()
{
    signal(SIGPIPE, &SigPipehandler);
    signal(SIGINT, &SigHandler);
}

static void UnregisterBreakHandler()
{
}
#endif

//----

BEGIN_INFERNO_NAMESPACE()

//----

static IApplication* GApplication = nullptr;

RTTI_BEGIN_TYPE_ABSTRACT_CLASS(IApplication);
RTTI_END_TYPE();

IApplication::IApplication()
    : m_exitRequested(false)
{
    TRACE_INFO("Applicataion starting");
    GApplication = this;
    RegisterBreakHandler();
}

IApplication::~IApplication()
{
    UnregisterBreakHandler();
    GApplication = nullptr;
    TRACE_INFO("Applicataion finished");
}

void IApplication::requestExit()
{
    if (0 == m_exitRequested.exchange(1))
    {
        TRACE_WARNING("Exit requested for application");
    }
}

void IApplication::registerInterface(void* ptr, ClassType cls)
{
    DEBUG_CHECK_RETURN_EX(ptr, "Invalid interface pointer");

    auto index = cls->assignUserIndex();
    m_interfaces.resizeWith(index + 1, nullptr);

    DEBUG_CHECK_RETURN_EX(m_interfaces[index] == nullptr, "Interface already registered");
    m_interfaces[index] = ptr;
}

void IApplication::unregisterInterface(void* ptr, ClassType cls)
{
    DEBUG_CHECK_RETURN_EX(ptr, "Invalid interface pointer");

    auto index = cls->peekUserIndex();
    DEBUG_CHECK_RETURN_EX(index != INDEX_NONE, "Class not used as interface before");
    DEBUG_CHECK_RETURN_EX(index >= 0 && index <= m_interfaces.lastValidIndex(), "Invalid class interface index");
    DEBUG_CHECK_RETURN_EX(m_interfaces[index] == ptr, "Class not registered");

    m_interfaces[index] = nullptr;
}

void* IApplication::internalQueryInterface(ClassType cls) const
{
    auto index = cls->peekUserIndex();
    if (index >= 0 && index <= m_interfaces.lastValidIndex())
        return m_interfaces[index];
    return nullptr;
}

//----

IApplication& App()
{
    return *GApplication;
}

//----

END_INFERNO_NAMESPACE()

