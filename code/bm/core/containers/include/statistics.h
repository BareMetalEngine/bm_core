/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

#include "bm/core/containers/include/stringView.h"
#include "bm/core/containers/include/hashMap.h"

BEGIN_INFERNO_NAMESPACE()

//--

// max number of stats counters per stat group
static const uint32_t MAX_STAT_COUNTERS_PER_GROUP = 256;

// index of rendering stat
typedef uint8_t StatCounterIndex;

// type of stats
enum class StatCounterType : uint8_t
{
    Count, // value represents count
    Time, // value represents time interval (as NativeTimeInterval)
};

// information about rendering stat
struct StatCounterInfo
{
    StatCounterType type = StatCounterType::Count;
    StatCounterIndex index = 0;
    StringView name;
};

//--

class StatCounterGroup;

//--

namespace prv { class StatBlockRegistry; }

// block of stat values for one stat group
// NOTE: can be accumulated safely from multiple threads, merged, etc
class BM_CORE_CONTAINERS_API StatBlock : public MainPoolData<NoCopy>
{
public:
    //--

    // reset all values to zero
    void clear();

    //--

    // name of the stat block
    INLINE StringView name() const { return m_name; }

    // counter group
    INLINE const StatCounterGroup* group() const { return m_group; }

    // data values
    INLINE std::atomic<uint64_t>* values() { return m_values; }

    // data values
    INLINE const std::atomic<uint64_t>* values() const { return m_values; }

    //--

    // get a global stat block by name, creates one if needed
    static StatBlock* FetchBlock(StringView name, const StatCounterGroup& group);

    // collect all stat blocks at given time
    static void CollectBlocks(Array<StatBlock*>& outBlocks);

    //--

private:
    StatBlock(StringView name, const StatCounterGroup* group);

    std::atomic<uint64_t> m_values[MAX_STAT_COUNTERS_PER_GROUP];

    StringView m_name;
    const StatCounterGroup* m_group = nullptr;

    //--

    friend class prv::StatBlockRegistry;
};

//--

// base stat counter, represents a single counter/timer in the statistics group
class BM_CORE_CONTAINERS_API StatCounterBase : public MainPoolData<NoCopy>
{
public:
    StatCounterBase(StringView name, StatCounterType type, StatCounterGroup& group, StatBlock& block, StringView hint);

    INLINE StatCounterIndex index() const { return m_index; }
    INLINE StatBlock* block() const { return m_block; }

    INLINE void operator+=(uint64_t val) const
    {
        m_block->values()[m_index] += val;
    }

    INLINE void operator-=(uint64_t val) const
    {
        m_block->values()[m_index] -= val;
    }

protected:
    StatCounterIndex m_index = 0;
    StatBlock* m_block = nullptr;

    static StatCounterGroup* CreateStatGroup(StringView name);
};

//--

// group of stat counters under a common name
// NOTE: there can be only 256 counters in a group
#define DECLARE_STAT_GROUP(_api, _name) \
class _api _name##StatCounter : public bm::StatCounterBase { public: \
_name##StatCounter(bm::StringView name, bm::StringView hint=""); \
static bm::StatCounterGroup& Group(); \
static bm::StatBlock& Block(); }; \
class _api _name##StatTimer : public bm::StatCounterBase { public: \
_name##StatTimer(bm::StringView name, bm::StringView hint=""); \
static bm::StatCounterGroup& Group(); \
static bm::StatBlock& Block(); };

#define DEFINE_STAT_GROUP(_name) \
_name##StatCounter::_name##StatCounter(StringView name, StringView hint) : StatCounterBase(name, StatCounterType::Count, Group(), Block(), hint) {} \
StatCounterGroup& _name##StatCounter::Group() { static auto* theGroup = CreateStatGroup(#_name); return *theGroup; } \
StatBlock& _name##StatCounter::Block() { static auto* theBlock = StatBlock::FetchBlock(#_name, Group()); return *theBlock; } \
_name##StatTimer::_name##StatTimer(StringView name, StringView hint) : StatCounterBase(name, StatCounterType::Time, Group(), Block(),hint) {} \
StatCounterGroup& _name##StatTimer::Group() { static auto* theGroup = CreateStatGroup(#_name); return *theGroup; } \
StatBlock& _name##StatTimer::Block() { static auto* theBlock = StatBlock::FetchBlock(#_name, Group()); return *theBlock; } \

//--

// group of stat counters under a common name
// NOTE: there can be only 256 counters in a group
#define DEFINE_LOCAL_STAT_GROUP(_name) \
class _name##StatCounter : public bm::StatCounterBase { public: \
INLINE _name##StatCounter(StringView name, StringView hint="") : StatCounterBase(name, StatCounterType::Count, Group(), Block(), hint) {} \
static StatCounterGroup& Group()  { static auto* theGroup = CreateStatGroup(#_name); return *theGroup; } \
static StatBlock& Block() { static auto* theBlock = StatBlock::FetchBlock(#_name, Group()); return *theBlock; } }; \
class _name##StatTimer : public bm::StatCounterBase { public: \
INLINE _name##StatTimer(StringView name, StringView hint="") : StatCounterBase(name, StatCounterType::Time, Group(), Block(), hint) {} \
static StatCounterGroup& Group() { static auto* theGroup = CreateStatGroup(#_name); return *theGroup; } \
static StatBlock& Block() { static auto* theBlock = StatBlock::FetchBlock(#_name, Group()); return *theBlock; } };

//--

// helper class to count some stats on stack (when it's much faster than using global atomic counter) and than push it once we are done
class ScopeStatCounter : public MainPoolData<NoCopy>
{
public:
    INLINE ScopeStatCounter(const StatCounterBase& counter)
        : m_counter(counter)
    {}

    INLINE ~ScopeStatCounter()
    {
        if (m_accumulator)
            m_counter += m_accumulator;
    }

    INLINE void operator+=(uint64_t stat)
    {
        m_accumulator += stat;
    }

private:
    const StatCounterBase& m_counter;
    uint64_t m_accumulator = 0;
};

//--

// helper class to automatically count time taken and add it to stats
class ScopeStatTimer : public MainPoolData<NoCopy>
{
public:
    INLINE ScopeStatTimer(const StatCounterBase& counter)
        : m_counter(counter)
    {
        m_start.resetToNow();
    }

    INLINE ~ScopeStatTimer()
    {
        auto timeElapsed = m_start.timeTillNow().rawValue();
        m_counter += timeElapsed;
    }

private:
    const StatCounterBase& m_counter;

    NativeTimePoint m_start;
};

//--

END_INFERNO_NAMESPACE()
