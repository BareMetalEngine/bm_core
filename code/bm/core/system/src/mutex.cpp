/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "mutex.h"
#include "private.h"

BEGIN_INFERNO_NAMESPACE()

Mutex::Mutex()
{
    memzero(&m_data, sizeof(m_data));

#ifdef PLATFORM_WINDOWS
    static_assert(sizeof(m_data) >= sizeof(CRITICAL_SECTION), "Critical section data to small");
    InitializeCriticalSection((CRITICAL_SECTION*)&m_data);
#elif defined(PLATFORM_POSIX)
    static_assert(sizeof(m_data) >= sizeof(pthread_mutex_t), "Critical section data to small");
    memset(&m_data, 0, sizeof(m_data));

    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init((pthread_mutex_t*)&m_data, &attr);
    pthread_mutexattr_destroy(&attr);

    pthread_mutex_lock((pthread_mutex_t*)&m_data);
    pthread_mutex_unlock((pthread_mutex_t*)&m_data);
#elif defined(PLATFORM_PSX)
    static_assert(sizeof(m_data) >= sizeof(ScePthreadMutex), "Critical section data to small");
    ScePthreadMutexattr attr;
    ::scePthreadMutexattrInit(&attr);
    ::scePthreadMutexattrSettype(&attr, SCE_PTHREAD_MUTEX_RECURSIVE);
    ::scePthreadMutexattrSetprotocol(&attr, SCE_PTHREAD_PRIO_INHERIT);
    ::scePthreadMutexInit((ScePthreadMutex*)&m_data, &attr, nullptr);
    ::scePthreadMutexattrDestroy(&attr);
#else
#error "Add platform crap"
#endif
}

Mutex::~Mutex()
{
#ifdef PLATFORM_WINDOWS
    DeleteCriticalSection((CRITICAL_SECTION*)&m_data);
#elif defined(PLATFORM_POSIX)
    pthread_mutex_destroy((pthread_mutex_t*)&m_data);
#elif defined(PLATFORM_PSX)
    ::scePthreadMutexDestroy((ScePthreadMutex*)&m_data);
#else
    #error "Add platform crap"
#endif
}

void Mutex::acquire()
{
#ifdef PLATFORM_WINDOWS
    EnterCriticalSection((CRITICAL_SECTION*)&m_data);
#elif defined(PLATFORM_POSIX)
    pthread_mutex_lock((pthread_mutex_t*)&m_data);
#elif defined(PLATFORM_PSX)
    ::scePthreadMutexLock((ScePthreadMutex*)&m_data);
#else
    #error "Add platform crap"
#endif
}

void Mutex::release()
{
#ifdef PLATFORM_WINDOWS
    LeaveCriticalSection((CRITICAL_SECTION*)&m_data);
#elif defined(PLATFORM_POSIX)
    pthread_mutex_unlock((pthread_mutex_t*)&m_data);
#elif defined(PLATFORM_PSX)
    ::scePthreadMutexUnlock((ScePthreadMutex*)&m_data);
#else
    #error "Add platform crap"
#endif
}

void Mutex::spinCount(uint32_t spinCount)
{
#ifdef PLATFORM_WINDOWS
    SetCriticalSectionSpinCount((CRITICAL_SECTION*)&m_data, spinCount);
#endif
}

END_INFERNO_NAMESPACE()
