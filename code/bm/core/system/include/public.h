/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

// Glue headers and logic
#include "bm_core_system_glue.inl"

// Disable the bullshit
#define  _CRT_SECURE_NO_WARNINGS

// Disable exceptions
#undef _HAS_EXCEPTIONS
#define _HAS_EXCEPTIONS 0

// STL
#include <memory>
#include <algorithm>
#include <utility>
#include <atomic>
#include <functional>
#include <inttypes.h>
#include <iostream>
#include <string.h>

// Expose some base headers
#include "settings.h"
#include "types.h"
#include "algorithms.h" 
#include "singleton.h"
#include "format.h"
#include "output.h"
#include "module.h"
#include "scope.h"
#include "scopeLock.h"
#include "staticCRC.h"
#include "profiling.h"
#include "systemInfo.h"
#include "formatBuffer.h"
#include "atomic.h"
#include "event.h"
#include "thread.h"
#include "semaphoreEx.h"
#include "spinLock.h"
#include "timestamp.h"
#include "threadSafe.h"
#include "readWriteLock.h"

BEGIN_INFERNO_NAMESPACE()

class CommandLine;

END_INFERNO_NAMESPACE()

