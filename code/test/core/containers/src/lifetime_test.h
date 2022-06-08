/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

namespace tests
{
	struct TrackableElement
	{
		uint32_t id = 0;
		uint32_t value = 0;

		uint32_t numConstructed = 0;
		uint32_t numCopyConstructed = 0;
		uint32_t numMoveConstructed = 0;
		uint32_t numCopyAssigned = 0;
		uint32_t numMoveAssigned = 0;
		uint32_t numDestructed = 0;

		TrackableElement();
		TrackableElement(uint32_t payload);
		TrackableElement(const TrackableElement& other);
		TrackableElement(TrackableElement&& other);
		TrackableElement& operator=(const TrackableElement& other);
		TrackableElement& operator=(TrackableElement&& other);
		~TrackableElement();
	};

	struct SimpleTracker
	{
		static uint32_t NumDefaultConstructed;
		static uint32_t NumValueConstructed;
		static uint32_t NumCopyConstructed;
		static uint32_t NumMoveConstructed;
		static uint32_t NumCopyAssigned;
		static uint32_t NumMoveAssigned;
		static uint32_t NumDestructed;

		static void ResetStats();
		static void ResetPopulation();

		static uint32_t AllocUnique();
		static void ReleaseUnique(uint32_t id); // errors if we release something we already did

		static void CheckPopulation(uint32_t expectedLiveObjectCount);
		static uint32_t Population();

		uint32_t m_id;
		uint32_t m_value;

		SimpleTracker();
		SimpleTracker(uint32_t payload);
		SimpleTracker(const SimpleTracker& other);
		SimpleTracker(SimpleTracker&& other);
		SimpleTracker& operator=(const SimpleTracker& other);
		SimpleTracker& operator=(SimpleTracker&& other);
		~SimpleTracker();
	};

	class SimpleTrackerScope
	{
	public:
		SimpleTrackerScope();
		~SimpleTrackerScope();
	};

	class SimpleTrackerTest : public ::testing::Test
	{
	public:
		virtual void SetUp() override;
		virtual void TearDown() override;
	};

} // tests
