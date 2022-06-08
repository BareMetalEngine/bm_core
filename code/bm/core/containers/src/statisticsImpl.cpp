/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "statistics.h"
#include "statisticsImpl.h"

BEGIN_INFERNO_NAMESPACE()

//---

namespace prv
{
    class StatCounterGroupRegistry : public ISingleton
    {
        DECLARE_SINGLETON(StatCounterGroupRegistry);

    public:
        StatCounterGroupRegistry()
        {}

        StatCounterGroup* get(StringView name)
        {
            auto lock = CreateLock(m_lock);

            for (auto* group : m_groups)
                if (group->name() == name)
                    return group;

            auto* group = new StatCounterGroup(name);
            m_groups.pushBack(group);
            return group;
        }

        StatCounterGroup* find(StringView name) const
        {
            auto lock = CreateLock(m_lock);

            for (auto* group : m_groups)
                if (group->name() == name)
                    return group;

            return nullptr;
        }

        void collect(Array<StatCounterGroup*>& outGroups)
        {
            auto lock = CreateLock(m_lock);
            outGroups = m_groups;
        }

    private:
        SpinLock m_lock;

        Array<StatCounterGroup*> m_groups;

        virtual void deinit() override
        {
            for (auto* group : m_groups)
                delete group;
            m_groups.clear();
        }
    };

} // prv

//---

StatCounterBase::StatCounterBase(StringView name, StatCounterType type, StatCounterGroup& group, StatBlock& block, StringView hint)
    : m_block(&block)
{
    m_index = group.createCounter(name, type);
}

StatCounterGroup* StatCounterBase::CreateStatGroup(StringView name)
{
    return prv::StatCounterGroupRegistry::GetInstance().get(name);
}

//--

StatCounterGroup::StatCounterGroup(StringView name)
    : m_name(name)
{}

StatCounterGroup::~StatCounterGroup()
{}

void StatCounterGroup::collectCounters(Array<StatCounterInfo>& outStats) const
{
    auto lock = CreateLock(m_lock);

    const auto count = m_nextID - 1;

    outStats.reserve(count);
    outStats.pushBackMany(ArrayView<StatCounterInfo>(m_counters + 1, count));
}

StatCounterIndex StatCounterGroup::findCounter(StringView name) const
{
    auto lock = CreateLock(m_lock);

    StatCounterIndex id = 0;
    if (m_map.find(name, id))
        return id;

    return 0;
}

StatCounterIndex StatCounterGroup::createCounter(StringView name, StatCounterType type)
{
    auto lock = CreateLock(m_lock);

    StatCounterIndex id = 0;
    if (m_map.find(name, id))
        return id;

    DEBUG_CHECK_RETURN_EX_V(m_nextID < MAX_STAT_COUNTERS_PER_GROUP, "To many counters in stat group", 0);

    id = m_nextID++;
    m_counters[id].index = id;
    m_counters[id].name = name;
    m_counters[id].type = type;
    m_map[name] = id;

    return id;
}

//--

StatCounterGroup* StatCounterGroup::CreateGroup(StringView name)
{
    return prv::StatCounterGroupRegistry::GetInstance().get(name);
}

StatCounterGroup* StatCounterGroup::FindGroup(StringView name)
{
    return prv::StatCounterGroupRegistry::GetInstance().find(name);
}

void StatCounterGroup::CollectGroup(Array<StatCounterGroup*>& outGroups)
{
    prv::StatCounterGroupRegistry::GetInstance().collect(outGroups);
}

//--

namespace prv
{
    class StatBlockRegistry : public ISingleton
    {
        DECLARE_SINGLETON(StatBlockRegistry);

    public:
        StatBlockRegistry()
        {}

        StatBlock* get(StringView name, const StatCounterGroup& group)
        {
            auto lock = CreateLock(m_lock);

            for (auto* block : m_blocks)
                if (block->name() == name && block->group() == &group)
                    return block;

            auto* block = new StatBlock(name, &group);
            m_blocks.pushBack(block);
            return block;
        }

        void collect(Array<StatBlock*>& outBlocks)
        {
            auto lock = CreateLock(m_lock);
            outBlocks = m_blocks;
        }

    private:
        SpinLock m_lock;

        Array<StatBlock*> m_blocks;

        virtual void deinit() override
        {
            /*for (auto* group : m_blocks)
                delete group;*/
            m_blocks.clear();
        }
    };

} // prv


StatBlock::StatBlock(StringView name, const StatCounterGroup* group)
    : m_name(name)
    , m_group(group)
{
    memzero(&m_values, sizeof(m_values));
}

void StatBlock::clear()
{
    memzero(&m_values, sizeof(m_values));
}

//--

StatBlock* StatBlock::FetchBlock(StringView name, const StatCounterGroup& group)
{
    return prv::StatBlockRegistry::GetInstance().get(name, group);
}

void StatBlock::CollectBlocks(Array<StatBlock*>& outBlocks)
{
    return prv::StatBlockRegistry::GetInstance().collect(outBlocks);
}

//--

/*
DECLARE_STAT_GROUP(Test, "TestGroup");

static TestStatCounter STAT_NUM_CRAP("MyCount");
static TestStatTimer STAT_RENDERING_TIME("TheTime");

void test(StatBlock& stats)
{
    stats.add(STAT_NUM_CRAP, 1);

    {
        StatScopeTimer timer(STAT_RENDERING_TIME, &stats);
        StatScopeCounter numOBjects(STAT_NUM_CRAP, &stats);
    }
}
*/

//--

END_INFERNO_NAMESPACE()