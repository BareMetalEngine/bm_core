/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "private.h"
#include "mutex.h"
#include "conditionVariable.h"

BEGIN_INFERNO_NAMESPACE()

//---

ConditionVariable::ConditionVariable()
{
	memzero(&m_data, sizeof(m_data));

#ifdef PLATFORM_WINDOWS
	static_assert(sizeof(m_data) >= sizeof(CONDITION_VARIABLE), "Critical section data to small");
	InitializeConditionVariable((CONDITION_VARIABLE*)&m_data);
#elif defined(PLATFORM_POSIX)
	static_assert(sizeof(m_data) >= sizeof(pthread_mutex_t), "Critical section data to small");

#elif defined(PLATFORM_PSX)
	static_assert(sizeof(m_data) >= sizeof(ScePthreadMutex), "Critical section data to small");
#else
#error "Add platform crap"
#endif
}

ConditionVariable::~ConditionVariable()
{
#ifdef PLATFORM_WINDOWS
    // nothing
#elif defined(PLATFORM_POSIX)
    // TODO
#elif defined(PLATFORM_PSX)
	// TODO
#else
    #error "Add platform crap"
#endif
}

bool ConditionVariable::waitInfinite(Mutex& m)
{
#ifdef PLATFORM_WINDOWS
    auto* cv = (CONDITION_VARIABLE*)&m_data;
    auto* cs = (CRITICAL_SECTION*)&m.m_data;
    return SleepConditionVariableCS(cv, cs, INFINITE);
#elif defined(PLATFORM_POSIX)
	// TODO
#elif defined(PLATFORM_PSX)
	// TODO
#else
#error "Add platform crap"
#endif
}

bool ConditionVariable::wait(Mutex& m, uint32_t ms)
{
#ifdef PLATFORM_WINDOWS
	auto* cv = (CONDITION_VARIABLE*)&m_data;
	auto* cs = (CRITICAL_SECTION*)&m.m_data;
	return SleepConditionVariableCS(cv, cs, ms);
#elif defined(PLATFORM_POSIX)
	// TODO
#elif defined(PLATFORM_PSX)
	// TODO
#else
#error "Add platform crap"
#endif
}

void ConditionVariable::wakeAll()
{
#ifdef PLATFORM_WINDOWS
	auto* cv = (CONDITION_VARIABLE*)&m_data;
	return WakeAllConditionVariable(cv);
#elif defined(PLATFORM_POSIX)
	// TODO
#elif defined(PLATFORM_PSX)
	// TODO
#else
#error "Add platform crap"
#endif
}

void ConditionVariable::wakeOne()
{
#ifdef PLATFORM_WINDOWS
	auto* cv = (CONDITION_VARIABLE*)&m_data;
	return WakeConditionVariable(cv);
#elif defined(PLATFORM_POSIX)
	// TODO
#elif defined(PLATFORM_PSX)
	// TODO
#else
#error "Add platform crap"
#endif
}

//---

END_INFERNO_NAMESPACE()
