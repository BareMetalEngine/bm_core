/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

BEGIN_INFERNO_NAMESPACE()

//--

namespace prv {
    class StatCounterGroupRegistry;
} // prv

// group of counters for statistics
class BM_CORE_CONTAINERS_API StatCounterGroup : public MainPoolData<NoCopy>
{
public:
    //--

    // get name of the group
    INLINE StringView name() const { return m_name; }

    //--

    // collect information about stat entries in this group
    void collectCounters(Array<StatCounterInfo>& outStats) const;

    // find stat by name
    StatCounterIndex findCounter(StringView name) const;

    // create a stat
    StatCounterIndex createCounter(StringView name, StatCounterType type);

    //--

    // create stat group, returns existing one if name matches
    static StatCounterGroup* CreateGroup(StringView name);

    // find stat group by name
    static StatCounterGroup* FindGroup(StringView name);

    // collect all stat groups
    static void CollectGroup(Array<StatCounterGroup*>& outGroups);

    //--

private:
    StatCounterGroup(StringView name);
    ~StatCounterGroup();

    //--

    SpinLock m_lock;

    uint32_t m_nextID = 1;
    StatCounterInfo m_counters[MAX_STAT_COUNTERS_PER_GROUP];

    HashMap<StringView, StatCounterIndex> m_map;

    StringView m_name;

    friend class prv::StatCounterGroupRegistry;
};

//--

END_INFERNO_NAMESPACE()
