/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "taskSignalList.h"

BEGIN_INFERNO_NAMESPACE()

//--

TaskSignalList::TaskSignalList()
{
	m_signalGenerationCounter = 1;
	memzero(&m_signalTable, sizeof(m_signalTable));
}

TaskSignalList::~TaskSignalList()
{	
}

TaskSignal TaskSignalList::create(uint32_t count, StringID name)
{
	DEBUG_CHECK_RETURN_EX_V(count > 0, "Counter should be at least 1", TaskSignal());

	auto generation = m_signalGenerationCounter++;
	auto index = m_signalPoolAllocator.allocEntry();

	auto& entry = m_signalTable[index];

	{
		auto lock = CreateLock(entry.lock);
		ASSERT_EX(entry.generation == 0, "Signal slot is in use");
		ASSERT_EX(entry.callbacks == nullptr, "Signal slot is in use");
		ASSERT_EX(entry.forwarding == nullptr, "Signal slot is in use");
		entry.name = name;
		entry.generation = generation;
		entry.forwarding = nullptr;
		entry.callbacks = nullptr;
		entry.counter = count;
	}

	std::atomic_thread_fence(std::memory_order_release);

	TaskSignal ret;
	ret.m_id = index;
	ret.m_id |= generation << SIGNAL_BIT_COUNT;
	return ret;
}

TaskSignal TaskSignalList::merge(ArrayView<TaskSignal> inputSignals, uint32_t extraCount)
{
	// allocate signal that we will return
	auto outputSignal = create(inputSignals.size() + extraCount, "MergedSignal"_id);

	// allocated forwarder
	Forwarding* forwarder = nullptr;

	// create forwarding links for all referenced signals, for every failed forwarding link we will have to 
	uint32_t failedCount = 0;
	for (const auto inputSignal : inputSignals)
	{
		auto index = inputSignal.m_id & SIGNAL_MASK;
		auto generation = inputSignal.m_id >> SIGNAL_BIT_COUNT;

		auto& entry = m_signalTable[index];

		// alloc forwarder
		if (!forwarder)
			forwarder = m_forwardingPool.alloc();

		// inject it into the signal completion list
		{
			auto lock = CreateLock(entry.lock);
			if (entry.generation == generation)
			{
				forwarder->signal = outputSignal;
				forwarder->count = 1;
				forwarder->next = entry.forwarding;
				entry.forwarding = forwarder;
				forwarder = nullptr; // consume
			}
			else
			{
				failedCount += 1; // signal is no longer valid, we still need to trip the merged one as if it was
			}
		}
	}

	// if we are left with unconsumed forwarder return it to the pool
	if (forwarder)
		m_forwardingPool.free(forwarder);

	// account for invalid input fences
	if (failedCount)
	{
		TRACE_INFO("Lost {}/{} signals", failedCount, inputSignals.size());
		trip(outputSignal, failedCount);
	}

	// return output signal that still has "extraCount" to go
	return outputSignal;
}

void TaskSignalList::trip(TaskSignal signal, uint32_t count)
{
	DEBUG_CHECK_RETURN_EX(signal, "Invalid signal");

	auto index = signal.m_id & SIGNAL_MASK;
	auto generation = signal.m_id >> SIGNAL_BIT_COUNT;

	std::atomic_thread_fence(std::memory_order_acquire);

	auto& entry = m_signalTable[index];
	DEBUG_CHECK_RETURN_EX(entry.generation == generation, "Tripping invalid signal");

	const auto newCount = entry.counter -= count;
	ASSERT_EX(newCount >= 0, "Signal counter went below zero");

	if (0 == newCount)
	{		
		Callback* callback = nullptr;
		Forwarding* forwarding = nullptr;

		// reset state
		{
			auto lock = CreateLock(entry.lock);
			callback = entry.callbacks;
			forwarding = entry.forwarding;
			entry.callbacks = nullptr;
			entry.forwarding = nullptr;
			entry.name = StringID();
			entry.generation = 0; // THIS indicates signal is completed
		}

		std::atomic_thread_fence(std::memory_order_release);

		// return to pool
		m_signalPoolAllocator.freeEntry(index);

		// process callbacks
		while (callback)
		{
			auto next = callback->next;
			callback->func();
			m_callbackPool.free(callback);
			callback = next;
		}

		// process forwarding links
		while (forwarding)
		{
			auto next = forwarding->next;
			trip(forwarding->signal, forwarding->count); // RECURSIVE !!
			m_forwardingPool.free(forwarding);
			forwarding = next;
		}
	}
}

bool TaskSignalList::finished(TaskSignal signal) const
{
	VALIDATION_RETURN_V(signal, true); // empty signal is always finished

	auto index = signal.m_id & SIGNAL_MASK;
	auto generation = signal.m_id >> SIGNAL_BIT_COUNT;

	std::atomic_thread_fence(std::memory_order_acquire);

	auto& entry = m_signalTable[index];
	if (entry.generation != generation)
		return true;

	return false;
}

void TaskSignalList::registerComplectionCallback(TaskSignal signal, TSignalCompletionCallback func)
{
	DEBUG_CHECK_RETURN_EX(signal, "Invalid signal");

	auto index = signal.m_id & SIGNAL_MASK;
	auto generation = signal.m_id >> SIGNAL_BIT_COUNT;

	auto& entry = m_signalTable[index];

	// create callback wrapper
	auto* callback = m_callbackPool.alloc();
	callback->func = std::move(func);

	// inject it into the signal completion list
	{
		auto lock = CreateLock(entry.lock);
		if (entry.generation == generation)
		{
			callback->next = entry.callbacks;
			entry.callbacks = callback;
			callback = nullptr;
		}
	}

	// if injection failed execute callback now as the signal is dead
	if (callback)
	{
		callback->func();
		m_callbackPool.free(callback);
	}
}

void TaskSignalList::registerComplectionSignal(TaskSignal signal, TaskSignal otherSignal, uint32_t count)
{
	DEBUG_CHECK_RETURN_EX(signal, "Invalid signal");
	DEBUG_CHECK_RETURN_EX(otherSignal, "Other signal is invalid");
	DEBUG_CHECK_RETURN_EX(!finished(otherSignal), "Other signal has already finished");

	auto index = signal.m_id & SIGNAL_MASK;
	auto generation = signal.m_id >> SIGNAL_BIT_COUNT;

	auto& entry = m_signalTable[index];

	// create forwarding wrapper
	auto* forwarder = m_forwardingPool.alloc();
	forwarder->signal = otherSignal;
	forwarder->count = count;

	// inject it into the signal completion list
	{
		auto lock = CreateLock(entry.lock);
		if (entry.generation == generation)
		{
			forwarder->next = entry.forwarding;
			entry.forwarding = forwarder;
			forwarder = nullptr;
		}
	}

	// if injection failed trip other signal right now
	// NOTE: done outside the locks
	if (forwarder)
	{
		trip(forwarder->signal, forwarder->count);
		m_forwardingPool.free(forwarder);
	}

}

//--

END_INFERNO_NAMESPACE()
