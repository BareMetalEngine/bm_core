/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "taskScheduler_NativeThreads.h"
#include "taskScheduler_NativeThreadsQueue.h"
#include "taskScheduler_NativeThreadsWorker.h"
#include "taskScheduler_NativeThreadsEventPool.h"

BEGIN_INFERNO_NAMESPACE()

//--

TaskScheduler_NativeThreads::TaskScheduler_NativeThreads(uint32_t numThreads, ThreadPriority priority, bool assignAffinity)
{
	m_queue = new TaskScheduler_NativeThreadsQueue();
	m_events = new TaskScheduler_NativeThreadEventPool();

	uint32_t threadIndex = 0;

	if (assignAffinity)
	{
		InplaceArray<int, 4> reservedAfinities;
		reservedAfinities.pushBack(0); // MAIN THREAD
		reservedAfinities.pushBack(5); // RENDER THREAD

		const auto cores = Thread::NumberOfCores();

		InplaceArray<int, 256> afinities;
		for (uint32_t i = 0; i < cores && afinities.size() < numThreads; ++i)
			if (!reservedAfinities.contains(i))
				afinities.pushBack(i);
		
		for (uint32_t i = 0; i < afinities.size(); ++i)
		{
			auto* thread = new TaskScheduler_NativeThreadWorker(m_queue, m_events, i, priority, afinities[i]);
			m_threads.pushBack(thread);
		}
	}
	else
	{
		for (uint32_t i = 0; i < numThreads; ++i)
		{
			auto* thread = new TaskScheduler_NativeThreadWorker(m_queue, m_events, i, priority, -1);
			m_threads.pushBack(thread);
		}
	}
}

TaskScheduler_NativeThreads::~TaskScheduler_NativeThreads()
{
	m_threads.clearPtr();
	delete m_queue;
	delete m_events;
}

void TaskScheduler_NativeThreads::scheduleTask(TaskEntry* entry)
{
	m_queue->scheduleTask(entry);
}

//--

END_INFERNO_NAMESPACE()
