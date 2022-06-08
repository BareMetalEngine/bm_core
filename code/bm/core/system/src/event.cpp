/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "event.h"
#include "private.h"

BEGIN_INFERNO_NAMESPACE()

#ifdef PLATFORM_POSIX
namespace prv
{
    struct EventData
    {
        bool signalled = false;
        bool manualReset = false;
        pthread_mutex_t m_mutex;
        pthread_cond_t m_cond;

        inline EventData(bool manulReset)
            : manualReset(manulReset)
        {
            pthread_mutex_init(&m_mutex, NULL);
            pthread_cond_init(&m_cond, NULL);
        }

        inline ~EventData()
        {
            pthread_mutex_destroy(&m_mutex);
            pthread_cond_destroy(&m_cond);
        }

        inline void signal()
        {
            pthread_mutex_lock(&m_mutex);

            if (manualReset)
            {
                signalled = true;
                pthread_cond_broadcast(&m_cond);
            }
            else if (!signalled)
            {
                signalled = true;
                pthread_cond_signal(&m_cond);
            }

            pthread_mutex_unlock(&m_mutex);
        }

        inline void reset()
        {
            if (manualReset)
            {
                pthread_mutex_lock(&m_mutex);
                signalled = false;
                pthread_mutex_unlock(&m_mutex);
            }
        }

        inline bool wait(uint32_t waitTime)
        {
            pthread_mutex_lock(&m_mutex);
            if (!signalled)
            {
                timeval now;
                gettimeofday(&now, NULL);

                timespec ts;
                ts.tv_sec = now.tv_sec + (waitTime / 1000);
                ts.tv_nsec = (now.tv_usec * 1000) + ((waitTime % 1000) * 1000000ULL);
                ts.tv_sec += ts.tv_nsec / 1000000000ULL;
                ts.tv_nsec %= 1000000000ULL;

                auto ret = pthread_cond_timedwait(&m_cond, &m_mutex, &ts);
                if (ret != 0)
                {
                    pthread_mutex_unlock(&m_mutex);
                    if (ret != ETIMEDOUT)
                    {
                        TRACE_ERROR("Error waiting for event: {}", errno);
                    }

                    return false;
                }
            }

            // reset after wait
            if (!manualReset)
                signalled = false;

            pthread_mutex_unlock(&m_mutex);
            return true;
        }

        inline void waitInfinite()
        {
            pthread_mutex_lock(&m_mutex);
            while (!signalled)
            {
                pthread_cond_wait(&m_cond, &m_mutex);
            }
            if (!manualReset)
                signalled = false;
            pthread_mutex_unlock(&m_mutex);
        }
    };
}
#endif

Event::Event(EventType type)
{
    bool manualReset = (type == EventType::ManualReset);
#ifdef PLATFORM_WINDOWS
    m_event = CreateEvent(NULL, manualReset, 0, NULL);
#elif defined(PLATFORM_POSIX)
    m_event = new prv::EventData(manualReset);
#elif defined(PLATFORM_PROSPERO)
    SceKernelEventFlag flag;
    sceKernelCreateEventFlag(&flag, "Event", SCE_KERNEL_EVF_ATTR_MULTI | SCE_KERNEL_EVF_ATTR_TH_PRIO, 0, nullptr);
    m_event = flag;
#else
    #error "Add platform crap"
#endif

}

Event::~Event()
{
#ifdef PLATFORM_WINDOWS
    CloseHandle((HANDLE)m_event);
#elif defined(PLATFORM_POSIX)
    delete (prv::EventData*)m_event;
#elif defined(PLATFORM_PROSPERO)
    sceKernelDeleteEventFlag((SceKernelEventFlag)m_event);
#else
    #error "Add platform crap"
#endif
}

void Event::trigger()
{
#ifdef PLATFORM_WINDOWS
    SetEvent((HANDLE)m_event);
#elif defined(PLATFORM_POSIX)
    auto data = (prv::EventData*)m_event;
    data->signal();
#elif defined(PLATFORM_PROSPERO)
    auto data = (SceKernelEventFlag)m_event;
    sceKernelSetEventFlag(data, 1);
#else
    #error "Add platform crap"
#endif
}

void Event::reset()
{
#ifdef PLATFORM_WINDOWS
    ResetEvent((HANDLE)m_event);
#elif defined(PLATFORM_POSIX)
    auto data = (prv::EventData*)m_event;
    data->reset();
#elif defined(PLATFORM_PROSPERO)
    auto data = (SceKernelEventFlag)m_event;
    sceKernelClearEventFlag(data, 0);
#else
    #error "Add platform crap"
#endif
}

bool Event::wait(uint32_t waitTime)
{
#ifdef PLATFORM_WINDOWS
    DEBUG_CHECK_EX(waitTime != INFINITE, "Invalid wait time");
    auto ret = WaitForSingleObject((HANDLE)m_event, waitTime);

    DEBUG_CHECK_EX(ret == WAIT_OBJECT_0 || ret == WAIT_TIMEOUT, "Event lost while waiting for it");
    return (ret == WAIT_OBJECT_0);
#elif defined(PLATFORM_POSIX)
    auto data = (prv::EventData*)m_event;
    return data->wait(waitTime);
#elif defined(PLATFORM_PROSPERO)
    auto data = (SceKernelEventFlag)m_event;
    SceKernelUseconds timeout = waitTime * 1000; // in us
    return SCE_OK == sceKernelWaitEventFlag(data, 1, SCE_KERNEL_EVF_WAITMODE_AND, nullptr, &timeout);
#else
    #error "Add platform crap"
#endif
}

void Event::waitInfinite()
{
#ifdef PLATFORM_WINDOWS
    auto ret = WaitForSingleObject((HANDLE)m_event, INFINITE);
    DEBUG_CHECK_EX(ret == WAIT_OBJECT_0, "Event lost while waiting for it");
#elif defined(PLATFORM_POSIX)
    auto data = (prv::EventData*)m_event;
    data->waitInfinite();
#elif defined(PLATFORM_PROSPERO)
    auto data = (SceKernelEventFlag)m_event;
    sceKernelWaitEventFlag(data, 1, SCE_KERNEL_EVF_WAITMODE_AND, nullptr, nullptr);
#else
    #error "Add platform crap"
#endif
}

END_INFERNO_NAMESPACE()
