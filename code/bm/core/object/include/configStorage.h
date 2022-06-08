/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

#include "bm/core/containers/include/hashMap.h"

BEGIN_INFERNO_NAMESPACE()

///----

class ConfigGroup;
class ConfigEntry;

/// config storage contains a PERSISTENT group and entries maps
/// NOTE: a group or entry that is once created is not deleted until the storage itself is deleted
class BM_CORE_OBJECT_API ConfigStorage : public MainPoolData<NoCopy>
{
public:
    ConfigStorage();
    ~ConfigStorage();

    //--

    // remove all groups and entries
    void clear();

    //--

    // get group, creates an empty new one if not found
    ConfigGroup& group(StringView name);

    // find group, does not create a new one
    const ConfigGroup* findGroup(StringView name) const;

    // remove group and all stored entries from the storage
    bool removeGroup(StringView name);

    // remove entry in group
    bool removeEntry(StringView groupName, StringView varName);

    // find all groups starting with given start string
    Array<const ConfigGroup*> findAllGroups(StringView groupNameSubString) const;

    //---

    // load from a text content
    static bool Load(StringView txt, ConfigStorage& ret);

    // save settings to string, filter out settings that are the same as in base
    static void Save(IFormatStream& f, const ConfigStorage& cur, const ConfigStorage& base);

    // apply values from other storage
    static void Append(ConfigStorage& target, const ConfigStorage& from);

    //---

    // get the data version, changed every time we are modified
    INLINE uint32_t currentVersion() const { return m_version.load(); }

private:
    friend class ConfigGroup;

    HashMap<StringBuf, ConfigGroup*> m_groups;
    std::atomic<uint32_t> m_version;

    SpinLock m_lock;

    ConfigGroup* group_NoLock(StringView name);
    const ConfigGroup* findGroup_NoLock(StringView name) const;
    Array<const ConfigGroup*> findAllGroups_NoLock(StringView groupNameSubString) const;

    void modified();
};

END_INFERNO_NAMESPACE()
