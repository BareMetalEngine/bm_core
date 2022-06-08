/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "bm/core/task/include/taskBuilder.h"
#include "bm/core/task/include/taskSignal.h"

BEGIN_INFERNO_NAMESPACE()

//--

TEST(TaskTest, ConcurencyOne)
{
	const double spinTime = 0.002;
	const uint32_t taskCount = 100;

	ScopeTimer timer;

	std::atomic<uint32_t> simultanousTasks;
	 
	auto sig = TaskBuilder("Spin"_id).instances(taskCount).concurency(1) << [spinTime, &simultanousTasks]
	{
		auto count = ++simultanousTasks;
		EXPECT_GE(1, count);

		auto waitTil = NativeTimePoint::Now() + spinTime;
		while (!waitTil.reached())
			_mm_pause();

		--simultanousTasks;
	};

	sig.waitSpinInfinite();
}

TEST(TaskTest, ConcurencyTwo)
{
	const double spinTime = 0.002;
	const uint32_t taskCount = 200;

	ScopeTimer timer;

	std::atomic<uint32_t> simultanousTasks;

	auto sig = TaskBuilder("Spin"_id).instances(taskCount).concurency(2) << [spinTime, &simultanousTasks]
	{
		auto count = ++simultanousTasks;
		EXPECT_GE(2, count);

		auto waitTil = NativeTimePoint::Now() + spinTime;
		while (!waitTil.reached())
			_mm_pause();

		--simultanousTasks;
	};

	sig.waitSpinInfinite();
}

TEST(TaskTest, ConcurencyFour)
{
	const double spinTime = 0.002;
	const uint32_t taskCount = 400;

	ScopeTimer timer;

	std::atomic<uint32_t> simultanousTasks;

	auto sig = TaskBuilder("Spin"_id).instances(taskCount).concurency(4) << [spinTime, &simultanousTasks]
	{
		auto count = ++simultanousTasks;
		EXPECT_GE(4, count);

		auto waitTil = NativeTimePoint::Now() + spinTime;
		while (!waitTil.reached())
			_mm_pause();

		--simultanousTasks;
	};

	sig.waitSpinInfinite();
}

TEST(TaskTest, ConcurencyEight)
{
	const double spinTime = 0.002;
	const uint32_t taskCount = 800;

	ScopeTimer timer;

	std::atomic<uint32_t> simultanousTasks;

	auto sig = TaskBuilder("Spin"_id).instances(taskCount).concurency(8) << [spinTime, &simultanousTasks]
	{
		auto count = ++simultanousTasks;
		EXPECT_GE(8, count);

		auto waitTil = NativeTimePoint::Now() + spinTime;
		while (!waitTil.reached())
			_mm_pause();

		--simultanousTasks;
	};

	sig.waitSpinInfinite();
}


TEST(TaskTest, ConcurencySixteen)
{
	const double spinTime = 0.002;
	const uint32_t taskCount = 1600;

	ScopeTimer timer;

	std::atomic<uint32_t> simultanousTasks;

	auto sig = TaskBuilder("Spin"_id).instances(taskCount).concurency(16) << [spinTime, &simultanousTasks]
	{
		auto count = ++simultanousTasks;
		EXPECT_GE(16, count);

		auto waitTil = NativeTimePoint::Now() + spinTime;
		while (!waitTil.reached())
			_mm_pause();

		--simultanousTasks;
	};

	sig.waitSpinInfinite();
}

#if 0
TEST(TaskTest, FullSpin)
{
	const double spinTime = 0.0001;
	const uint32_t taskCount = 100000000;

	ScopeTimer timer;

	std::atomic<uint32_t> simultanousTasks;

	auto sig = TaskBuilder("Spin"_id).instances(taskCount) << [spinTime, &simultanousTasks]
	{
		auto count = ++simultanousTasks;
		EXPECT_GE(14, count);

		auto waitTil = NativeTimePoint::Now() + spinTime;
		while (!waitTil.reached())
			_mm_pause();

		--simultanousTasks;
	};

	while (!sig.waitSpinWithTimeout(33))
	{
		MarkProfilingFrame();
	}
}
#endif

//--

END_INFERNO_NAMESPACE()