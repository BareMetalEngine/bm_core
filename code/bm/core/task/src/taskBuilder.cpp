/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "taskBuilder.h"
#include "taskSignal.h"
#include "taskScheduler.h"
#include "taskEntry.h"

BEGIN_INFERNO_NAMESPACE()

//--

TaskBuilder::TaskBuilder(StringID name)
	: m_scheduler(&MainScheduler())
{	
	m_entry = TaskEntry::Alloc();
	m_entry->name = name;
	m_entry->group = 0;
	m_entry->signal = TaskSignal::Create(1, name);
}

TaskBuilder::TaskBuilder(TaskContext& c, StringID name)
	: m_scheduler(&MainScheduler())
{
	m_entry = TaskEntry::Alloc();
	m_entry->name = name;
	m_entry->group = c.groupIndex;
	m_entry->signal = TaskSignal::Create(1, name);
}

TaskBuilder::~TaskBuilder()
{
	DEBUG_CHECK_EX(!m_entry, "Task was not finalized with function and was not submitted");

	if (m_entry)
	{
		if (m_entry->signal)
			m_entry->signal.trip(1); // needed so the signal can be freed

		TaskEntry::Free(m_entry);
	}
}

TaskBuilder& TaskBuilder::scheduler(ITaskScheduler& scheduler)
{
	m_scheduler = &scheduler;
	return *this;
}

TaskBuilder& TaskBuilder::instances(uint32_t count)
{
	m_entry->instances = count;
	return *this;
}

TaskBuilder& TaskBuilder::concurency(uint32_t concurency)
{
	m_entry->concurency = concurency;
	return *this;
}

TaskBuilder& TaskBuilder::waitFor(TaskSignal signal)
{
	DEBUG_CHECK_RETURN_EX_V(signal, "Invalid signal", *this);
	ASSERT_EX(m_waitForSignals.contains(signal), "Signal can't be added twice to the wait list");
	m_waitForSignals.pushBack(signal);
	return *this;
}

TaskBuilder& TaskBuilder::waitFor(std::initializer_list<TaskSignal> signals)
{
	for (TaskSignal signal : signals)
	{
		DEBUG_CHECK_RETURN_EX_V(signal, "Invalid signal", *this);
		ASSERT_EX(m_waitForSignals.contains(signal), "Signal can't be added twice to the wait list");
		m_waitForSignals.pushBack(signal);
	}

	return *this;
}

TaskBuilder& TaskBuilder::waitFor(ArrayView<TaskSignal> signals)
{
	for (TaskSignal signal : signals)
	{
		DEBUG_CHECK_RETURN_EX_V(signal, "Invalid signal", *this);
		ASSERT_EX(m_waitForSignals.contains(signal), "Signal can't be added twice to the wait list");
		m_waitForSignals.pushBack(signal);
	}

	return *this;
}

TaskBuilder& TaskBuilder::signal(TaskSignal signal)
{
	DEBUG_CHECK_RETURN_EX_V(signal, "Invalid signal", *this);
	m_entry->signal.registerCompletionSignal(signal);
	return *this;
}

TaskBuilder& TaskBuilder::signal(std::initializer_list<TaskSignal> signals)
{
	for (TaskSignal signal : signals)
		m_entry->signal.registerCompletionSignal(signal);	
	return *this;
}

TaskBuilder& TaskBuilder::signal(ArrayView<TaskSignal> signals)
{
	for (TaskSignal signal : signals)
		m_entry->signal.registerCompletionSignal(signal);
	return *this;
}

//--

TaskSignal TaskBuilder::operator<<(TTaskFunc func)
{
	m_entry->func = [func](TaskContext& t, uint32_t count)
	{
		func(t);
	};

	return submit();
}

TaskSignal TaskBuilder::operator<<(TTaskFuncEx func)
{
	m_entry->func = [func](TaskContext& t, uint32_t count)
	{
		func();
	};

	return submit();
}

TaskSignal TaskBuilder::operator<<(TTaskInstancingFunc func)
{
	m_entry->func = std::move(func);
	return submit();
}

TaskSignal TaskBuilder::operator<<(TTaskInstancingFuncEx func)
{
	m_entry->func = [func](TaskContext& t, uint32_t count)
	{
		func(count);
	};

	return submit();
}

//--

TaskSignal TaskBuilder::submit()
{
	auto ret = m_entry->signal;

	// if we don't have any other conditions submit the task right away
	if (m_waitForSignals.empty())
	{
		m_scheduler ->scheduleTask(m_entry);
		m_entry = nullptr;
	}
	// we have a waiting conditions
	else
	{
		// create merged signal if needed
		auto waitSignal = (m_waitForSignals.size() == 1)
			? m_waitForSignals[0]
			: TaskSignal::Merge(m_waitForSignals.view());

		// once the wait signal is triggered submit the task
		auto* entry = m_entry;
		auto* system = m_scheduler;
		waitSignal.registerCompletionCallback([entry, system]()
			{
				system->scheduleTask(entry);
			});
	}

	return ret;
}

//--

END_INFERNO_NAMESPACE()
