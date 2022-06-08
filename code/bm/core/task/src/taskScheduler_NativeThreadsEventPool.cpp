/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "taskScheduler_NativeThreadsEventPool.h"

BEGIN_INFERNO_NAMESPACE()

//--

TaskScheduler_NativeThreadEventPool::TaskScheduler_NativeThreadEventPool()
	: m_allocator(sizeof(Event) * 1024)
{
	for (uint32_t i = 0; i < 64; ++i)
	{
		auto* evt = m_allocator.create(EventType::AutomaticReset);
		m_freeEvents.pushBack(evt);
	}
}

TaskScheduler_NativeThreadEventPool::~TaskScheduler_NativeThreadEventPool()
{
	for (auto* evt : m_freeEvents)
		m_allocator.free(evt);
}

Event* TaskScheduler_NativeThreadEventPool::alloc()
{
	{
		auto lock = CreateLock(m_lock);

		Event* ret = nullptr;
		if (m_freeEvents.popBackIfExists(ret))
			return ret;
	}

	return m_allocator.create(EventType::AutomaticReset);
}

void TaskScheduler_NativeThreadEventPool::free(Event* evt)
{
	auto lock = CreateLock(m_lock);
	m_freeEvents.pushBack(evt);
}

//--

END_INFERNO_NAMESPACE()
