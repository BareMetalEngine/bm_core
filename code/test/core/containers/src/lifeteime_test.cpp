/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "lifetime_test.h"

#include <unordered_set>

namespace tests
{

	//--

	uint32_t SimpleTracker::NumDefaultConstructed = 0;
	uint32_t SimpleTracker::NumValueConstructed = 0;
	uint32_t SimpleTracker::NumCopyConstructed = 0;
	uint32_t SimpleTracker::NumMoveConstructed = 0;
	uint32_t SimpleTracker::NumCopyAssigned = 0;
	uint32_t SimpleTracker::NumMoveAssigned = 0;
	uint32_t SimpleTracker::NumDestructed = 0;

	void SimpleTracker::ResetStats()
	{
		NumDefaultConstructed = 0;
		NumValueConstructed = 0;
		NumCopyConstructed = 0;
		NumMoveConstructed = 0;
		NumCopyAssigned = 0;
		NumMoveAssigned = 0;
		NumDestructed = 0;
	}

	std::unordered_set<uint32_t> AllocatedObjects;

	uint32_t NextObjectIndex = 1;

	uint32_t SimpleTracker::AllocUnique()
	{
		auto id = NextObjectIndex++;
		AllocatedObjects.insert(id);
		return id;
	}

	void SimpleTracker::ReleaseUnique(uint32_t id)
	{
		auto it = AllocatedObjects.find(id);
		EXPECT_TRUE(it != AllocatedObjects.end());

		if (it != AllocatedObjects.end())
			AllocatedObjects.erase(id);
	}

	void SimpleTracker::ResetPopulation()
	{
		AllocatedObjects.clear();
	}

	uint32_t SimpleTracker::Population()
	{
		return AllocatedObjects.size();
	}

	void SimpleTracker::CheckPopulation(uint32_t expectedLiveObjectCount)
	{
		EXPECT_EQ(AllocatedObjects.size(), expectedLiveObjectCount);
	}

	SimpleTracker::SimpleTracker()
		: m_value(0)
	{
		m_id = AllocUnique();
		NumDefaultConstructed += 1;
	}

	SimpleTracker::SimpleTracker(uint32_t payload)
		: m_value(payload)
	{
		m_id = AllocUnique();
		NumValueConstructed += 1;
	}

	SimpleTracker::SimpleTracker(const SimpleTracker& other)
		: m_value(other.m_value)
	{
		m_id = AllocUnique();
		NumCopyConstructed += 1;
	}

	SimpleTracker::SimpleTracker(SimpleTracker&& other)
		: m_id(other.m_id)
        , m_value(other.m_value)
	{
		other.m_value = 0;
		other.m_id = 0;
		NumMoveConstructed += 1;
	}

	SimpleTracker& SimpleTracker::operator=(const SimpleTracker& other)
	{
		if (this != &other)
		{
			m_value = other.m_value;			
			NumCopyAssigned += 1;
		}
		return *this;
	}

	SimpleTracker& SimpleTracker::operator=(SimpleTracker&& other)
	{
		if (this != &other)
		{
			if (0 != m_id)
			{
				NumDestructed += 1;
				ReleaseUnique(m_id);
				m_id = 0;
			}

			m_value = other.m_value;
			m_id = other.m_id;

			other.m_id = 0;
			other.m_value = 0;

			NumMoveAssigned += 1;
		}
		return *this;
	}

	SimpleTracker::~SimpleTracker()
	{
		if (0 != m_id)
		{
			NumDestructed += 1;
			ReleaseUnique(m_id);
			m_id = 0;
		}

		m_value = 0;
	}

	//--

	static std::atomic<uint32_t> NextTrackableElementIndex = 1;

	TrackableElement::TrackableElement()
	{
		id = NextTrackableElementIndex + 1;
		numConstructed += 1;
	}

	TrackableElement::TrackableElement(uint32_t payload)
	{
		id = NextTrackableElementIndex + 1;
		value = payload;
		numConstructed += 1;
	}

	TrackableElement::TrackableElement(const TrackableElement& other)
		: numConstructed(other.numConstructed)
		, numCopyConstructed(other.numCopyConstructed)
		, numMoveConstructed(other.numMoveConstructed)
		, numCopyAssigned(other.numCopyAssigned)
		, numMoveAssigned(other.numMoveAssigned)
		, numDestructed(other.numDestructed)
		, id(other.id)
		, value(other.value)
	{
		numCopyConstructed += 1;
	}

	TrackableElement::TrackableElement(TrackableElement&& other)
		: numConstructed(other.numConstructed)
		, numCopyConstructed(other.numCopyConstructed)
		, numMoveConstructed(other.numMoveConstructed)
		, numCopyAssigned(other.numCopyAssigned)
		, numMoveAssigned(other.numMoveAssigned)
		, numDestructed(other.numDestructed)
		, id(other.id)
		, value(other.value)
	{
		numMoveConstructed += 1;
		other.id = 0;
		other.value = 0;
	}

	TrackableElement& TrackableElement::operator=(const TrackableElement& other)
	{
		if (this != &other)
		{
			numConstructed = other.numConstructed;
			numCopyConstructed = other.numCopyConstructed;
			numMoveConstructed = other.numMoveConstructed;
			numCopyAssigned = other.numCopyAssigned;
			numMoveAssigned = other.numMoveAssigned;
			numDestructed = other.numDestructed;
			id = other.id;
			value = other.value;
			
			numCopyConstructed += 1;
		}

		return *this;
	}

	TrackableElement& TrackableElement::operator=(TrackableElement&& other)
	{
		if (this != &other)
		{
			numConstructed = other.numConstructed;
			numCopyConstructed = other.numCopyConstructed;
			numMoveConstructed = other.numMoveConstructed;
			numCopyAssigned = other.numCopyAssigned;
			numMoveAssigned = other.numMoveAssigned;
			numDestructed = other.numDestructed;
			id = other.id;
			value = other.value;

			numMoveAssigned += 1;
			other.numConstructed = 0;
			other.numCopyConstructed = 0;
			other.numMoveConstructed = 0;
			other.numCopyAssigned = 0;
			other.numMoveAssigned = 0;
			other.numDestructed = 0;
			other.id = 0;
			other.value = 0;
		}

		return *this;
	}

	TrackableElement::~TrackableElement()
	{
		numDestructed += 1;
	}

	//--

	void SimpleTrackerTest::SetUp()
	{
		SimpleTracker::ResetStats();
		SimpleTracker::ResetPopulation();
	}

	void SimpleTrackerTest::TearDown()
	{
		SimpleTracker::CheckPopulation(0);
	}

	//--

	SimpleTrackerScope::SimpleTrackerScope()
	{
		SimpleTracker::ResetStats();
		SimpleTracker::ResetPopulation();
	}

	SimpleTrackerScope::~SimpleTrackerScope()
	{
		SimpleTracker::CheckPopulation(0);
	}

	//--

} // tests
