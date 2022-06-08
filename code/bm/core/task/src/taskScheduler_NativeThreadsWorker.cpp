/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "taskEntry.h"
#include "taskScheduler_NativeThreadsQueue.h"
#include "taskScheduler_NativeThreadsWorker.h"
#include "taskScheduler_NativeThreadsEventPool.h"

BEGIN_INFERNO_NAMESPACE()

//--

static StringID FormatThreadName(uint32_t index, ThreadPriority prio)
{
	const char* prefix = (prio == ThreadPriority::Normal)
		? "WorkerThead"
		: ((prio == ThreadPriority::BelowNormal) ? "BackgroundThread" : "HighPriorityThread");

	return StringID(TempString("{}{}", prefix, index));
}

TaskScheduler_NativeThreadWorker::TaskScheduler_NativeThreadWorker(TaskScheduler_NativeThreadsQueue* taskQueue, TaskScheduler_NativeThreadEventPool* eventPool, uint32_t index, ThreadPriority prio, int affinity)
	: m_queue(taskQueue)
	, m_eventPool(eventPool)
	, m_name(FormatThreadName(index, prio))
	, m_requestExit(false)
{
	auto name = m_name;

	ThreadSetup setup;
	setup.m_priority = prio;
	setup.m_name = m_name.c_str();
	setup.m_stackSize = 1U << 20;

	setup.m_function = [this, name, affinity, prio]()
	{ 
		InitProfilingThread(name.c_str(), Thread::CurrentThreadID(), affinity, (int)prio);
		threadFunc();
	};

	if (affinity >= 0)
		setup.m_affinity = affinity;

	m_thread.init(setup);
}

TaskScheduler_NativeThreadWorker::~TaskScheduler_NativeThreadWorker()
{
	TRACE_SPAM("Exit requested for {}", m_name);
	m_requestExit = true;
	m_thread.close();
}

//--

void TaskScheduler_NativeThreadWorker::yieldTaskAndWaitForSignal(TaskSignal signal)
{
	auto* evt = m_eventPool->alloc();

	signal.registerCompletionCallback([evt]()
		{
			evt->trigger();
		});

	evt->waitInfinite(); // YIELDS THE THREAD
	m_eventPool->free(evt);
}

//--

void TaskScheduler_NativeThreadWorker::threadFunc()
{
	ScopeTimer timer;

	TRACE_SPAM("Started thread '{}'", m_name);

	uint32_t totalTaskCount = 0;
	NativeTimeInterval totalTaskTime;

	uint32_t taskSkip = 0;
	while (!m_requestExit)
	{
		uint32_t taskIndex = 0;
		TaskEntry* taskEntry = nullptr;

		if (m_queue->popTask(taskEntry, taskIndex))
		{
			ScopeTimer timer;

			{
				TaskContext localContext;
				localContext.groupIndex = taskEntry->group;
				localContext.yielder = this;
				localContext.completionSignal = taskEntry->signal;

				{
					PC_SCOPE_DYNAMIC(taskEntry->name.c_str());
					taskEntry->func(localContext, taskIndex);
				}

				taskEntry->signal = localContext.completionSignal;

				m_queue->finishTask(taskEntry, taskIndex);
			}

			totalTaskTime += timer.timeElapsedInterval();
			totalTaskCount += 1;
		}
	}

	TRACE_SPAM("Finished thead {} after {}, processing time {} ({} tasks), utilization {}",
		m_name, timer, totalTaskTime, totalTaskCount, Percent(totalTaskTime.toSeconds(), timer.timeElapsed()));

	CloseProfilingThread();
}

//--

END_INFERNO_NAMESPACE()
