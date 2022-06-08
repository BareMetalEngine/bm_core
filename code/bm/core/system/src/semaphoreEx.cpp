/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "semaphoreEx.h"
#include "private.h"

BEGIN_INFERNO_NAMESPACE()

#ifdef PLATFORM_POSIX
namespace prv
{
    struct SemaphoreData
    {
        sem_t data;

        inline SemaphoreData(uint32_t initialCount)
        {
            memset(&data, 0, sizeof(data));
            sem_init(&data, 0, initialCount);
        }

        inline ~SemaphoreData()
        {
            sem_destroy(&data);
        }

        inline void signal()
        {
            sem_post(&data);
        }

        inline bool wait(uint32_t timeout)
        {
            if (timeout == INFINITE_TIME)
            {
                sem_wait(&data);
                return true;
            }
            else
            {
                timeval now;
                gettimeofday(&now, NULL);

                timespec ts;
                ts.tv_sec = now.tv_sec + (timeout / 1000);
                ts.tv_nsec = (now.tv_usec * 1000) + ((timeout % 1000) * 1000000ULL);
                ts.tv_sec += ts.tv_nsec / 1000000000ULL;
                ts.tv_nsec %= 1000000000ULL;

                if (-1 == sem_timedwait(&data, &ts))
                {
                    if (errno == ETIMEDOUT)
                        return false;

                    TRACE_ERROR("Semaphore error: {}", errno);
                    return false;
                }

                return true;
            }
        }
    };

} // prv
#endif

Semaphore::Semaphore(uint32_t initialCount, uint32_t maxCount)
{
#ifdef PLATFORM_WINDOWS
    m_handle = CreateSemaphoreA(NULL, initialCount, maxCount, NULL);
#elif defined(PLATFORM_POSIX)
    m_handle = new prv::SemaphoreData(initialCount);
#endif
}

Semaphore::~Semaphore()
{
#ifdef PLATFORM_WINDOWS
    CloseHandle((HANDLE)m_handle);
#elif defined(PLATFORM_POSIX)
    delete (prv::SemaphoreData*)m_handle;
#endif
}

void Semaphore::release(uint32_t count)
{
#ifdef PLATFORM_WINDOWS
    ReleaseSemaphore(m_handle, count, NULL);
#elif defined(PLATFORM_POSIX)
    auto data = (prv::SemaphoreData*)m_handle;
    data->signal();
#endif
}

bool Semaphore::wait(const uint32_t waitTime /*INFINITE_TIME*/, bool alterable /*=false*/)
{
#ifdef PLATFORM_WINDOWS
    for (;;)
    {
        auto ret = WaitForSingleObjectEx(m_handle, waitTime, alterable);
        if (ret == WAIT_IO_COMPLETION)
            continue;

        DEBUG_CHECK_EX(ret == WAIT_OBJECT_0 || ret == WAIT_TIMEOUT, "Semaphore lost while waiting for it");
        return (ret == WAIT_OBJECT_0);
    }
#elif defined(PLATFORM_POSIX)
    auto data = (prv::SemaphoreData*)m_handle;
    return data->wait(waitTime);
#else 
    #error "Implement!"
#endif
}

//--

END_INFERNO_NAMESPACE()
