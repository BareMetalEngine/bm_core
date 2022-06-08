/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "taskSignal.h"
#include "taskSignalList.h"
#include "taskEntry.h"

BEGIN_INFERNO_NAMESPACE()

//--

static TaskSignalList* GTaskSignalList = new TaskSignalList();

//--

bool TaskSignal::finished() const
{
	return GTaskSignalList->finished(*this);
}

void TaskSignal::waitSpinInfinite()
{
	DEBUG_CHECK_RETURN_EX(!empty(), "Trying to wait on invalid signal");

	while (!GTaskSignalList->finished(*this))
	{ 
		_mm_pause();
	}
}

bool TaskSignal::waitSpinWithTimeout(uint32_t ms)
{
	auto timeout = NativeTimePoint::Now() + (double)(ms / 1000.0);
	while (!GTaskSignalList->finished(*this))
	{
		if (timeout.reached())
			return false;

		_mm_pause();
	}

	return true;
}

void TaskSignal::waitWithYeild(TaskContext& ctx)
{
	ASSERT(ctx.yielder);
	ctx.yielder->yieldTaskAndWaitForSignal(*this);
}

void TaskSignal::trip(uint32_t count /*= 1*/)
{
	GTaskSignalList->trip(*this, count);
}

void TaskSignal::registerCompletionCallback(TSignalCompletionCallback func)
{
	GTaskSignalList->registerComplectionCallback(*this, std::move(func));	
}

void TaskSignal::registerCompletionSignal(TaskSignal signal, uint32_t count /*= 1*/)
{
	GTaskSignalList->registerComplectionSignal(*this, signal, count);
}

//--

TaskSignal TaskSignal::Create(uint32_t count /*= 1*/, StringID debugName /*= ""*/)
{
	return GTaskSignalList->create(count, debugName);
}

TaskSignal TaskSignal::Merge(std::initializer_list<TaskSignal> signals, uint32_t extraCount)
{
	return GTaskSignalList->merge(ArrayView<TaskSignal>(signals.begin(), signals.end()), extraCount);
}

TaskSignal TaskSignal::Merge(ArrayView<TaskSignal> signals, uint32_t extraCount)
{
	return GTaskSignalList->merge(signals, extraCount);
}

TaskSignal TaskSignal::Steal(TaskContext& tc)
{
	ASSERT_EX(tc.completionSignal, "Task completion signal already stolen");
	auto ret = tc.completionSignal;
	tc.completionSignal = nullptr;
	return ret;
}

//--

END_INFERNO_NAMESPACE()
