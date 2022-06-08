/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "taskScheduler.h"
#include "taskScheduler_NativeThreads.h"
#include <thread>

#include "bm/core/containers/include/commandLine.h"

BEGIN_INFERNO_NAMESPACE()

//--

ITaskScheduler::~ITaskScheduler()
{}

//--

static ITaskScheduler* GMainScheduler = nullptr;
static ITaskScheduler* GBackgroundScheduler = nullptr;
static uint32_t GMaxCocurency = 1;

ITaskScheduler& MainScheduler()
{
	return *GMainScheduler;
}

ITaskScheduler& BackgroundScheduler()
{
	if (GBackgroundScheduler)
		return *GBackgroundScheduler;
	else
		return *GMainScheduler;
}

//--

bool InitTaskThreads(const CommandLine& cmdLine) 
{
	if (GMainScheduler)
		return true;

	const auto nunMainThreadsDefault = std::max<int>(1, std::thread::hardware_concurrency() / 2);
	const auto numMainThreads = std::max<int>(1, cmdLine.singleValueInt("taskThreads", nunMainThreadsDefault));
	const auto useAffinities = !cmdLine.singleValueBool("taskNoAffinities", false);
	TRACE_INFO("Task main scheduler using {} threads ({})", numMainThreads, useAffinities ? "with affinites" : "no affinities");

	if (useAffinities)
		Thread::SetThreadAffinity(0);

	GMaxCocurency = numMainThreads;
	GMainScheduler = new TaskScheduler_NativeThreads(numMainThreads, ThreadPriority::Normal, useAffinities);

	const auto useBackgroundScheduler = !cmdLine.singleValueBool("taskNoBackgroundScheduler", false);
	if (useBackgroundScheduler)
	{
		const auto nunBackgroundThreadsDefault = std::max<int>(1, std::thread::hardware_concurrency() / 4);
		const auto nunBackgroundThreads = std::max<int>(1, cmdLine.singleValueInt("taskBackgroundThreads", nunBackgroundThreadsDefault));
		TRACE_INFO("Task background scheduler using {} threads", nunBackgroundThreads);

		GBackgroundScheduler = new TaskScheduler_NativeThreads(nunBackgroundThreads, ThreadPriority::BelowNormal, false);
	}
	else
	{
		GBackgroundScheduler = nullptr;
	}

	return true;
}

uint32_t MaxTaskConcurency()
{
	return GMaxCocurency;
}

void CloseTaskThreads()
{
	ScopeTimer timer;

	TRACE_INFO("Closing task system");

	if (GBackgroundScheduler)
	{
		delete GBackgroundScheduler;
		GBackgroundScheduler = nullptr;
	}

	if (GMainScheduler)
	{
		delete GMainScheduler;
		GMainScheduler = nullptr;
	}

	if (timer.timeElapsed() > 0.1)
	{
		TRACE_WARNING("Task system closed in {}, some threads were dangling", timer);
	}
	else
	{
		TRACE_INFO("Task system closed in {}", timer);
	}
}

//--

END_INFERNO_NAMESPACE()
