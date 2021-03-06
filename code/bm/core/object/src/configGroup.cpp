/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "configStorage.h"
#include "configGroup.h"
#include "configEntry.h"

BEGIN_INFERNO_NAMESPACE()

//---

ConfigGroup::ConfigGroup(ConfigStorage* storage, const StringBuf& name)
    : m_storage(storage)
    , m_name(name)
{
}

ConfigGroup::~ConfigGroup()
{
    m_entries.clearPtr();
}

bool ConfigGroup::clear()
{
    auto lock  = CreateLock(m_lock);

    bool wasModified = false;
    for (auto entry  : m_entries.values())
        wasModified |= entry->clear();

    return wasModified;
}

StringBuf ConfigGroup::entryValue(StringView name, const StringBuf& defaultValue/*= ""*/) const
{
    if (auto entry  = findEntry(name))
        return entry->value();
    return defaultValue;
}

const ConfigEntry* ConfigGroup::findEntry(StringView name) const
{
    auto lock  = CreateLock(m_lock);
    return findEntry_NoLock(name);
}

const ConfigEntry* ConfigGroup::findEntry_NoLock(StringView name) const
{
    return m_entries.findSafe(name, nullptr);
}

ConfigEntry& ConfigGroup::entry(StringView name)
{
    auto lock  = CreateLock(m_lock);

    ASSERT_EX(!name.empty(), "Config entry name cannot be empty");

    auto entry  = m_entries.findSafe(name, nullptr);
    if (!entry)
    {
        auto nameStr = StringBuf(name);
        entry = new ConfigEntry(this, nameStr);
        m_entries[nameStr] = entry;
        // NOTE: we do not mark a group as modified since empty entry will NOT be saved any way
    }

    return *entry;
}

bool ConfigGroup::removeEntry(StringView name)
{
    auto lock  = CreateLock(m_lock);

    auto entry  = m_entries.findSafe(name, nullptr);
    if (!entry)
    {
        if (entry->clear())
        {
            modified();
            return true;
        }
    }

    return false;
}

void ConfigGroup::modified()
{
    m_storage->modified();
}

void ConfigGroup::SaveDiff(IFormatStream& f, const ConfigGroup& cur, const ConfigGroup& base)
{
    auto lockA  = CreateLock(cur.m_lock);
    auto lockB  = CreateLock(base.m_lock);

    bool headerPrinted = false;

    for (auto entry  : cur.m_entries.values())
    {
        // compare against the base
        auto baseEntry  = base.findEntry_NoLock(entry->name());
        if (baseEntry)
        {
            if (ConfigEntry::Equal(*entry, *baseEntry))
                continue;
        }

        // we will be writing the value, print the group header
        if (!headerPrinted)
        {
            headerPrinted = true;
            f.appendf("[{}]\n", cur.m_name);
        }

        // print the value difference
        if (baseEntry)
            ConfigEntry::PrintDiff(f, *entry, *baseEntry);
        else
            ConfigEntry::Print(f, *entry);
    }

    // print separator
    if (headerPrinted)
        f.append("\n");
}

void ConfigGroup::Save(IFormatStream& f, const ConfigGroup& cur)
{
    auto lockA  = CreateLock(cur.m_lock);

    bool headerPrinted = false;
    for (auto entry  : cur.m_entries.values())
    {
        // we will be writing the value, print the group header
        if (!headerPrinted)
        {
            headerPrinted = true;
            f.appendf("[{}]\n", cur.m_name);
        }

        // print the value difference
        ConfigEntry::Print(f, *entry);
    }

    // print separator
    if (headerPrinted)
        f.append("\n");
}

//---

END_INFERNO_NAMESPACE()
