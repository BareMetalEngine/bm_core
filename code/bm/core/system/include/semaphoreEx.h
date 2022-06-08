/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

#include "algorithms.h"

BEGIN_INFERNO_NAMESPACE()

//-----------------------------------------------------------------------------

/// Semaphore
class BM_CORE_SYSTEM_API Semaphore : public NoCopy
{
public:
    Semaphore(uint32_t initialCount, uint32_t maxCount);
    ~Semaphore();

    //! Release semaphore, will wake up one random thread waiting on it
    void release(uint32_t count = 1);

    //! Wait for semaphore, optionally a timeout can be specified (function will return false if timeout passed)
    bool wait(uint32_t waitTime = INFINITE_TIME, bool alterable = false);

private:
    INLINE Semaphore() {};

    void* m_handle = nullptr;     // Internal handle
};

//-----------------------------------------------------------------------------

END_INFERNO_NAMESPACE()
