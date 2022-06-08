/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

BEGIN_INFERNO_NAMESPACE()

//--

class TaskSignalList;

/// synchronization signal
struct BM_CORE_TASK_API TaskSignal
{
public:
	ALWAYS_INLINE TaskSignal() = default;
	ALWAYS_INLINE TaskSignal(std::nullptr_t) {};
	ALWAYS_INLINE TaskSignal(const TaskSignal& other) = default;
	ALWAYS_INLINE TaskSignal(TaskSignal&& other) = default;
	ALWAYS_INLINE TaskSignal& operator=(const TaskSignal& other) = default;
	ALWAYS_INLINE TaskSignal& operator=(TaskSignal&& other) = default;
	ALWAYS_INLINE ~TaskSignal() = default;

	//--

	// valid signal ?
	ALWAYS_INLINE operator bool() const { return m_id != 0; }

	// empty signal ?
	ALWAYS_INLINE bool empty() const { return m_id == 0; }

	//--

	// Wait for signal to be triggered, busy spins (basic wait)
	void waitSpinInfinite();

	// Wait for signal to be triggered within given time window (in ms)
	// Returns true if signal was triggered within that timeout or false it it was not
	bool waitSpinWithTimeout(uint32_t ms);

	// Wait via task yielder - the current task will be yielded and resumed once the signal has finished
	void waitWithYeild(TaskContext& ctx);

	// Check if signal has finished (NOTE: may return false just before signal finishes, use only for optimizations)
	bool finished() const;

	//--

	// Trip the signal, if the internal signal counter reaches zero the signal is released
	// NOTE: tripping dead signal is not allowed, tripping it more than initially allocated count as well
	void trip(uint32_t count = 1);

	//--

	// Register completion callback on signal, callback will be called THE INSTANT the signal count reaches zero
	// NOTE: the callback may be called from ANY thread
	// If signal is already tripped callback is called right away - be prepared
	void registerCompletionCallback(TSignalCompletionCallback func);

	// Register a signal to trip automatically once this signal is has finished
	// This avoid creating callbacks and is in general much faster allowing to implement "WaitForMultipleObjects" easily
	// NOTE: as with the registerCompletionCallback the signal may be already tripped - if so then we immediatelly subtract the count (which may finish us and trigger other signals, etc)
	void registerCompletionSignal(TaskSignal signal, uint32_t count = 1);

	//--

	// Create signal with internal counter at given value, once the counter reaches zero the signal is tripped
	static TaskSignal Create(uint32_t count = 1, StringID debugName = StringID());

	// Create a completion signal that will be triggered when all other signals are triggered (WaitForMultipleObjects style)
	// NOTE: may return empty signal if all given signals are already triggered
	static TaskSignal Merge(std::initializer_list<TaskSignal> signals, uint32_t extraCount=0);

	// Create a completion signal that will be triggered when all other signals are triggered (WaitForMultipleObjects style)
	// NOTE: may return empty signal if all given signals are already triggered
	static TaskSignal Merge(ArrayView<TaskSignal> signals, uint32_t extraCount = 0);

	// Pull completion signal from task context - it basically means we take responsibility for notifying other parties about taks completion
	static TaskSignal Steal(TaskContext& tc);

	//--

private:
	uint64_t m_id = 0; // internal ID + generation

	friend class TaskSignalList;
};

//--

END_INFERNO_NAMESPACE()

