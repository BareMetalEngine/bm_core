/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

BEGIN_INFERNO_NAMESPACE()

///----

class ConfigGroup;

/// storage entry for configuration values
/// NOTE: once created exists till deletion of the ConfigStorage (but may be empty)
/// NOTE: the ConfigStorageEntry objects owned by the config system are ETERNAL can be held onto by a pointer
class BM_CORE_OBJECT_API ConfigEntry : public MainPoolData<NoCopy>
{
public:
    ConfigEntry(ConfigGroup* group, const StringBuf& name);

    /// get the parent group
    INLINE ConfigGroup* group() const { return m_group;}

    /// get SHORT name of the entry (within a group)
    INLINE const StringBuf& name() const { return m_name; }

    //--

    /// remove all values, returns true if modified
    bool clear();

    /// replace all values with this one, returns true if modified
    bool value(const StringBuf& value);

    /// append new value to the list, returns true if modified
    bool appendValue(const StringBuf& value, bool onlyIfUnique=true);

    /// remove specific value, returns true if removed
    bool removeValue(const StringBuf& value);

    //--

    /// get value (last value from a list or empty string)
    StringBuf value() const;

    /// get all values, NOTE: slow as fuck but needed since we want to make the config entry thread safe
    const Array<StringBuf> values() const;

    /// read as integer, returns default value if not parsed correctly
    int valueInt(int defaultValue = 0) const;

    /// read as a float, returns default value if not parsed correctly
    float valueFloat(float defaultValue = 0) const;

    /// read as a boolean, returns default value if not parsed correctly
    bool valueBool(bool defaultValue = 0) const;

    //--

    /// compare if two entries are equal
    static bool Equal(const ConfigEntry& a, const ConfigEntry& b);

    /// print difference between two entries
    static void PrintDiff(IFormatStream& f, const ConfigEntry& val, const ConfigEntry& base);

    /// print content of the entry
    static void Print(IFormatStream& f, const ConfigEntry& val);

private:
    ConfigGroup* m_group;
    StringBuf m_name;
    Array<StringBuf> m_values;
    SpinLock m_lock;

    void modified();
};

///----

END_INFERNO_NAMESPACE()
