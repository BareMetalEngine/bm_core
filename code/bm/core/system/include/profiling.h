/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

#ifndef BUILD_FINAL
#define WITH_PROFILING
#endif

#ifdef WITH_PROFILING
#include "FramePro/FramePro.h"
#endif

BEGIN_INFERNO_NAMESPACE()

#if defined(WITH_PROFILING)

#define PC_SCOPE_LVL0(name, ...) FRAMEPRO_NAMED_SCOPE(#name)
#define PC_SCOPE_LVL1(name, ...) FRAMEPRO_NAMED_SCOPE(#name)
#define PC_SCOPE_LVL2(name, ...) FRAMEPRO_NAMED_SCOPE(#name)
#define PC_SCOPE_LVL3(name, ...) FRAMEPRO_HIRES_SCOPE(#name)
#define PC_SCOPE_DYNAMIC(name)   FRAMEPRO_DYNAMIC_SCOPE(name)

#else


// profiling macros
#define PC_SCOPE_LVL0(name, ...)
#define PC_SCOPE_LVL1(name, ...)
#define PC_SCOPE_LVL2(name, ...)
#define PC_SCOPE_LVL3(name, ...)
#define PC_SCOPE_DYNAMIC(name) 

#endif

extern BM_CORE_SYSTEM_API void InitProfiling();
extern BM_CORE_SYSTEM_API void CloseProfiling();

extern BM_CORE_SYSTEM_API void InitProfilingThread(const char* name, int id, int affinity, int priority);
extern BM_CORE_SYSTEM_API void CloseProfilingThread();

extern BM_CORE_SYSTEM_API void MarkProfilingFrame();

END_INFERNO_NAMESPACE()