/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "spinLock.h"
#include "private.h"
#include <assert.h>

BEGIN_INFERNO_NAMESPACE()

//---

SpinLock::SpinLock()
    : lock_(false)
{
}

void SpinLock::acquire()
{
	for (;;)
	{
		// Optimistically assume the lock is free on the first try
		if (!lock_.exchange(true, std::memory_order_acquire))
			return;

		// Wait for lock to be released without generating cache misses
		while (lock_.load(std::memory_order_relaxed))
			// Issue X86 PAUSE or ARM YIELD instruction to reduce contention between
			_mm_pause();
	}
}

void SpinLock::release()
{
	lock_.store(false, std::memory_order_release);
}

///---

END_INFERNO_NAMESPACE()
