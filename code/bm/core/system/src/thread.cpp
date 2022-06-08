/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "thread.h"
#include "private.h"

#include <thread>

BEGIN_INFERNO_NAMESPACE()

//---

ThreadSetup::ThreadSetup()
    : m_name("InfernoThread")
    , m_priority(ThreadPriority::Normal)
    , m_stackSize(64 * 1024)
{
}

//---

Thread::Thread()
    : m_systemThreadHandle(0)
{}

Thread::Thread(Thread&& other)
    : m_systemThreadHandle(other.m_systemThreadHandle)
{
    other.m_systemThreadHandle = 0;
}

Thread& Thread::operator=(Thread&& other)
{
    if (this != &other)
    {
        close();
        m_systemThreadHandle = other.m_systemThreadHandle;
        other.m_systemThreadHandle = 0;
    }

    return *this;
}

Thread::~Thread()
{
    close(true);
}

//--

void Thread::SetThreadName(const char* name)
{
#ifdef PLATFORM_WINDOWS
    // Name this thread
    struct THREADNAME_INFO
    {
        uint32_t dwType; // must be 0x1000
        LPCSTR szName; // pointer to name (in user addr space)
        uint32_t dwThreadID; // thread ID (-1=caller thread)
        uint32_t dwFlags; // reserved for future use, must be zero
    };

    THREADNAME_INFO info;
    info.dwType = 0x1000;
    info.szName = name;
    info.dwThreadID = ::GetCurrentThreadId();
    info.dwFlags = 0;

    __try
    {
        RaiseException(0x406D1388, 0, sizeof(info) / sizeof(uint32_t), (const ULONG_PTR*)&info);
    }
    __except (EXCEPTION_CONTINUE_EXECUTION)
    {
    }
#elif defined(PLATFORM_POSIX)
    pthread_setname_np(pthread_self(), name);
#elif defined(PLATFORM_PSX)
#else
    #error "Add platform crap"
#endif
}

void Thread::SetThreadAffinity(int core)
{
#ifdef PLATFORM_WINDOWS
    DWORD_PTR mask = 0;
    if (core < 0)
        mask = (DWORD_PTR)-1;
    else
        mask = 1ULL << core;
	::SetThreadAffinityMask(::GetCurrentThread(), mask);
#elif defined(PLATFORM_POSIX)
#elif defined(PLATFORM_PSX)
#else
#error "Add platform crap"
#endif
}

ThreadID Thread::CurrentThreadID()
{
#ifdef PLATFORM_WINDOWS
    return ::GetCurrentThreadId();
#elif defined(PLATFORM_POSIX)
    return syscall(SYS_gettid);
#elif defined(PLATFORM_PSX)
    return ::scePthreadGetthreadid();
#else
    #error "Add platform crap"
    return 1;
#endif
}

uint32_t Thread::NumberOfCores()
{
#if defined(PLATFORM_PSX)
    return 13;
#else
    return  std::thread::hardware_concurrency();
#endif
}

void Thread::Sleep(uint32_t ms)
{
#ifdef PLATFORM_WINDOWS
    ::Sleep(ms);
#elif defined(PLATFORM_POSIX)
    usleep(ms * 1000);
#elif defined(PLATFORM_PSX)
    ::sceKernelUsleep(ms * 1000);
#else
    #error "Add platform crap"
#endif
}

void Thread::YieldThread()
{
#ifdef PLATFORM_WINDOWS
    ::SwitchToThread();
#elif defined(PLATFORM_POSIX)
    sched_yield();
#elif defined(PLATFORM_PSX)
    ::scePthreadYield();
#else
    #error "Add platform crap"
#endif
}

//----

struct InitPayload
{
    TThreadFunc func;
    char name[50];
};

#ifdef PLATFORM_WINDOWS

static DWORD __stdcall ThreadStaticEntry(LPVOID lpThreadParameter)
{
    auto initData = (InitPayload*)lpThreadParameter;

    Thread::SetThreadName(initData->name);

    initData->func();

    delete initData;
    return 0;
}

void Thread::init(const ThreadSetup& setup)
{
    auto initData = new InitPayload;
    initData->func = setup.m_function;
    strcpy_s(initData->name, setup.m_name);

    INT systemPriority = THREAD_PRIORITY_NORMAL;
    switch (setup.m_priority)
    {
    case ThreadPriority::AboveNormal:
        systemPriority = THREAD_PRIORITY_ABOVE_NORMAL;
        break;

    case ThreadPriority::BelowNormal:
        systemPriority = THREAD_PRIORITY_BELOW_NORMAL;
        break;
    }

    // Create thread
    DWORD threadID = 0;
    HANDLE handle = CreateThread(NULL, setup.m_stackSize, &ThreadStaticEntry, initData, CREATE_SUSPENDED, &threadID);
    ASSERT_EX(NULL != handle, "Failed to create a thread. Whoops.");

    SetThreadPriority(handle, systemPriority);

    if (setup.m_affinity != -1)
    {
        DWORD_PTR mask = 1ULL << setup.m_affinity;
        ::SetThreadAffinityMask(handle, mask);
    }
        
    m_systemThreadHandle = handle;
    TRACE_SPAM("Thread '{}' created (ID {}), starting...", setup.m_name, threadID);

    ResumeThread(handle);
}

void Thread::close(bool terminateIfNeeded)
{
    if (m_systemThreadHandle)
    {
        if (terminateIfNeeded)
        {
            auto ret = WaitForSingleObject((HANDLE)m_systemThreadHandle, 10000);
            if (WAIT_OBJECT_0 != ret)
            {
                TRACE_WARNING("Failed to close thread gracefully (0x{}), so it was terminated without grace.", Hex(ret));
#ifdef PLATFORM_WINAPI
                TerminateThread((HANDLE)m_systemThreadHandle, 0);
#endif
            }
            else
            {
                CloseHandle((HANDLE)m_systemThreadHandle);
            }
        }
        else
        {
            WaitForSingleObject((HANDLE)m_systemThreadHandle, INFINITE);
            CloseHandle((HANDLE)m_systemThreadHandle);
        }

        m_systemThreadHandle = nullptr;
    }
}

#elif defined(PLATFORM_POSIX)

static void* ThreadStaticEntry(void* ptr)
{
    auto initData = (InitPayload*)ptr;

    pthread_setname_np(pthread_self(), initData->name);

    initData->func();
    delete initData;

    pthread_exit(0);
    return 0;
}

void Thread::init(const ThreadSetup& setup)
{
    auto initData = new InitPayload();
    initData->func = setup.m_function;
    strcpy_s(initData->name, setup.m_name);

    pthread_attr_t attrs;
    pthread_attr_init(&attrs);
    pthread_attr_setstacksize(&attrs, setup.m_stackSize);
    pthread_create((pthread_t*)&m_systemThreadHandle, &attrs, &ThreadStaticEntry, initData);
    pthread_attr_destroy(&attrs);
}

void Thread::close(bool terminate)
{
    pthread_join((pthread_t)m_systemThreadHandle, NULL);
    m_systemThreadHandle = 0;
}

#elif defined(PLATFORM_PSX)

static void* ThreadStaticEntry(void* ptr)
{
    auto initData = (InitPayload*)ptr;

    initData->func();
    delete initData;

    ::scePthreadExit(0);
    return 0;
}

void Thread::init(const ThreadSetup& setup)
{
    auto initData = new InitPayload();
    initData->func = setup.m_function;
    strcpy_s(initData->name, setup.m_name);

    SceKernelSchedParam schedParam;
    schedParam.sched_priority = SCE_KERNEL_PRIO_FIFO_DEFAULT;
    if (setup.m_priority == ThreadPriority::AboveNormal)
        schedParam.sched_priority = SCE_KERNEL_PRIO_FIFO_DEFAULT - 10;
    else if (setup.m_priority == ThreadPriority::BelowNormal)
        schedParam.sched_priority = SCE_KERNEL_PRIO_FIFO_DEFAULT + 10;

    ScePthreadAttr attrs;
    ::scePthreadAttrInit(&attrs);
    ::scePthreadAttrSetstacksize(&attrs, setup.m_stackSize);
    ::scePthreadAttrSetinheritsched(&attrs, SCE_PTHREAD_EXPLICIT_SCHED);
    ::scePthreadAttrSetschedpolicy(&attrs, SCE_KERNEL_SCHED_RR);
    ::scePthreadAttrSetschedparam(&attrs, &schedParam);
    ::scePthreadAttrSetdetachstate(&attrs, SCE_PTHREAD_CREATE_JOINABLE);

    const int err = ::scePthreadCreate((ScePthread*)&m_systemThreadHandle, &attrs, ThreadStaticEntry, initData, setup.m_name);
    ASSERT_EX(err == SCE_OK, "Failed to create pthread");

    ::scePthreadAttrDestroy(&attrs);
}

void Thread::close(bool terminate)
{
    ::scePthreadJoin((ScePthread)m_systemThreadHandle, nullptr);
    m_systemThreadHandle = 0;
}


#else

    #error "Add platform crap"

#endif

//---

END_INFERNO_NAMESPACE()
