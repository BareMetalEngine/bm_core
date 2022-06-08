/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "bm/core/task/include/taskSignal.h"

BEGIN_INFERNO_NAMESPACE()

//--

TEST(Signal, DefaultSignalIsEmpty)
{
	TaskSignal sig;
	EXPECT_TRUE(sig.empty());
}

TEST(Signal, DefaultSignalReportsAsFinished)
{
	TaskSignal sig;
	EXPECT_TRUE(sig.finished());
}

TEST(Signal, SignalWithCountIsValid)
{
	TaskSignal sig = TaskSignal::Create(1);
	EXPECT_FALSE(sig.empty());
	sig.trip();
}

TEST(Signal, SignalReportsAsNotFinished)
{
	TaskSignal sig = TaskSignal::Create(1);
	EXPECT_FALSE(sig.finished());
	sig.trip();
}

TEST(Signal, SignalReportsAsFinishedAfterTripping)
{
	TaskSignal sig = TaskSignal::Create(1);
	sig.trip();
	EXPECT_TRUE(sig.finished());
}

TEST(Signal, SignalWithCountTwoReportsAsNotFinishedAfterOneTripping)
{
	TaskSignal sig = TaskSignal::Create(2);
	sig.trip();
	EXPECT_FALSE(sig.finished());
	sig.trip();
}

TEST(Signal, SignalWithCountTwoReportsAsFinishedAfterTwoTripping)
{
	TaskSignal sig = TaskSignal::Create(2);
	sig.trip();
	sig.trip();
	EXPECT_TRUE(sig.finished());
}

TEST(Signal, SignalCompletionCallbackNotCalledBeforeSignalling)
{
	TaskSignal sig = TaskSignal::Create(1);

	bool called = false;
	sig.registerCompletionCallback([&called]()
		{
			called = true;
		});

	EXPECT_FALSE(called);
	sig.trip();
}

TEST(Signal, SignalCompletionCallbackNotCalledAfterSignalling)
{
	TaskSignal sig = TaskSignal::Create(1);

	bool called = false;
	sig.registerCompletionCallback([&called]()
		{
			called = true;
		});

	sig.trip();
	EXPECT_TRUE(called);
}


TEST(Signal, SignalCompletionCallbackAttachedToFinishedSignalCalledRightAway)
{
	TaskSignal sig = TaskSignal::Create(1);

	sig.trip();

	bool called = false;
	bool returned = false;
	sig.registerCompletionCallback([&called, &returned]()
		{
			EXPECT_FALSE(returned);
			called = true;
		});

	returned = true;
	EXPECT_TRUE(called);
}

//--

END_INFERNO_NAMESPACE()