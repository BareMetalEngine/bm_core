/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

#include "taskScheduler.h"
#include "bm/core/containers/include/groupQueue.h"

BEGIN_INFERNO_NAMESPACE()

//--

class TaskScheduler_NativeThreadsQueue;
class TaskScheduler_NativeThreadEventPool;
class TaskScheduler_NativeThreadWorker;

/// Scheduler based on native threads
class TaskScheduler_NativeThreads : public ITaskScheduler
{
public:
	TaskScheduler_NativeThreads(uint32_t numThreads, ThreadPriority priority, bool assignAffinity);
	virtual ~TaskScheduler_NativeThreads();

	virtual void scheduleTask(TaskEntry* entry) override final;

private:
	TaskScheduler_NativeThreadsQueue* m_queue = nullptr;
	TaskScheduler_NativeThreadEventPool* m_events = nullptr;
	Array<TaskScheduler_NativeThreadWorker*> m_threads;
};

//--

END_INFERNO_NAMESPACE()
