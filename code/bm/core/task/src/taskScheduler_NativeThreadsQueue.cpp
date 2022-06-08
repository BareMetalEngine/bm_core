/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "taskEntry.h"
#include "taskSignal.h"
#include "taskScheduler_NativeThreadsQueue.h"

BEGIN_INFERNO_NAMESPACE()

//--

TaskScheduler_NativeThreadsQueue::TaskScheduler_NativeThreadsQueue()
	: m_groupCounter(1)
	, m_queueSemaphore(0, 1U << 30)
{}

TaskScheduler_NativeThreadsQueue::~TaskScheduler_NativeThreadsQueue()
{}

//--

void TaskScheduler_NativeThreadsQueue::scheduleTask(TaskEntry* entry)
{
	if (entry->group == 0)
		entry->group = m_groupCounter++;

	entry->remainingJobs = entry->instances;
	entry->activeJobs = 0;
	entry->scheduledJobs = 0;

	{
		auto lock = CreateLock(m_queueLock);
		m_queue.push(entry, entry->group);
	}

	m_queueSemaphore.release(entry->instances);
}

bool TaskScheduler_NativeThreadsQueue::popTask(TaskEntry*& outEntry, uint32_t& outInstanceIndex)
{
	{
		auto lock = CreateLock(m_queueLock);
		if (popTask_NoLock(outEntry, outInstanceIndex))
			return true;
	}

	{
		PC_SCOPE_LVL2(WaitForJobs);
		m_queueSemaphore.wait(5);
	}

	{
		auto lock = CreateLock(m_queueLock);
		return popTask_NoLock(outEntry, outInstanceIndex);
	}
}

bool TaskScheduler_NativeThreadsQueue::popTask_NoLock(TaskEntry*& outEntry, uint32_t& outInstanceIndex)
{
	return m_queue.peek([&outEntry, &outInstanceIndex, this](void* ptr) -> GroupQueue::PeekResult
		{
			auto* entry = (TaskEntry*)ptr;

			std::atomic_thread_fence(std::memory_order_acquire);

			// task is at maximum concurency, ignore and start spawning other tasks
			if (entry->activeJobs >= entry->concurency)
			{
				m_spinCounter += 1;
				return GroupQueue::PeekResult::Continue;
			}

			// we will start work on this task
			outEntry = entry;
			outInstanceIndex = outEntry->scheduledJobs++;
			ASSERT(outInstanceIndex < outEntry->instances);

			outEntry->activeJobs++; // count how many active instances of this task we have (concurency)

			std::atomic_thread_fence(std::memory_order_release);

			const auto lastInstance = outEntry->instances - 1;
			if (outInstanceIndex == lastInstance)
				return GroupQueue::PeekResult::Remove; // remove from queue on last instance reached
			else
				return GroupQueue::PeekResult::Keep; // we haven't picked up all instances of this job yet
		});
}

void TaskScheduler_NativeThreadsQueue::finishTask(TaskEntry* entry, uint32_t instanceIndex)
{
	// unblock other threads when in concurency constrained state
	auto newCount = --entry->activeJobs;
	ASSERT_EX(newCount < entry->concurency, TempString("{} < {}", newCount, entry->concurency));

	// try to pop more work
	if (auto skipped = m_spinCounter.exchange(0))
		m_queueSemaphore.release(skipped);

	// release task back to pool if all instances finished
	// NOTE: keep last !!!
	if (0 == --entry->remainingJobs)
	{
		ASSERT(entry->scheduledJobs == entry->instances);

		// signal that we have finished this job
		// NOTE: this may schedule other jobs
		if (entry->signal)
			entry->signal.trip(1);

		// return entry back to pool
		TaskEntry::Free(entry);
	}
}

//--

END_INFERNO_NAMESPACE()
