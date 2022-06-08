/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

#include "algorithms.h"
#include "atomic.h"

BEGIN_INFERNO_NAMESPACE()

//-----------------------------------------------------------------------------

/// Simple spin lock
/// NOTE: spin lock can't be acquired between fibers
class BM_CORE_SYSTEM_API SpinLock : public NoCopy
{
public:
    SpinLock();

    void acquire();
    void release();

private:
    std::atomic<bool> lock_;
};

//-----------------------------------------------------------------------------

END_INFERNO_NAMESPACE()
