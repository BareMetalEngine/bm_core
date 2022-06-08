/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

#include "taskSignal.h"
#include "bm/core/system/include/locklessPool.h"

BEGIN_INFERNO_NAMESPACE()

//--

/// Internal signal manager
class TaskSignalList : public NoCopy
{
public:
	TaskSignalList();
	~TaskSignalList();

	//--

	/// create standalone signal
	TaskSignal create(uint32_t count, StringID name);

	/// create signal that will be triggered once all given signals complete
	TaskSignal merge(ArrayView<TaskSignal> signal, uint32_t extraCount);

	/// trip the signal
	void trip(TaskSignal signal, uint32_t count);

	/// register a callback to call when signal is tripped, will be called right away if signal is already finished
	void registerComplectionCallback(TaskSignal signal, TSignalCompletionCallback func);

	/// register an other signal trip
	void registerComplectionSignal(TaskSignal signal, TaskSignal otherSignal, uint32_t count);

	/// check if signal has finished
	bool finished(TaskSignal signal) const;

	//--

private:
	static const uint8_t SIGNAL_BIT_COUNT = 16;
	static const uint32_t SIGNAL_MASK = (1U << SIGNAL_BIT_COUNT) - 1;
	static const uint32_t MAX_SIGNALS = 1U << SIGNAL_BIT_COUNT;
	static const uint32_t MAX_CALLBACKS = 65536;
	static const uint32_t MAX_FORWARDING = 65536;

	//--

	struct Callback
	{
		TSignalCompletionCallback func;
		Callback* next = nullptr;
	};

	typedef LockLessPool<Callback, MAX_CALLBACKS, uint16_t> TCallbackPool;
	TCallbackPool m_callbackPool;

	//--

	struct Forwarding
	{
		TaskSignal signal;
		uint32_t count = 0;
		Forwarding* next = nullptr;
	};

	typedef LockLessPool<Forwarding, MAX_FORWARDING, uint16_t> TForwardingPool;
	TForwardingPool m_forwardingPool;

	//--

	TYPE_ALIGN(64, struct) SignalEntry // must be aligned to 64 to prevent false sharing
	{
		SpinLock lock; // locks the whole
		std::atomic<int> counter; // remaining count
		StringID name; // name of the signal, only if it's active
		uint64_t generation = 0; // current generation index, can be used to validate
		Forwarding* forwarding = nullptr; // linked signals
		Callback* callbacks = nullptr; // list of registered callbacks
	};

	static_assert(sizeof(SignalEntry) == 64, "Signal entry must be 64 bytes");

	typedef LockLessPoolAllocator<MAX_SIGNALS, uint16_t> TSignalPoolAllocator;

	TSignalPoolAllocator m_signalPoolAllocator;

	std::atomic<uint64_t> m_signalGenerationCounter;
	SignalEntry m_signalTable[MAX_SIGNALS];

	//--
};

//--

END_INFERNO_NAMESPACE()
