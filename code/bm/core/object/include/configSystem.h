/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

BEGIN_INFERNO_NAMESPACE()

///----

class IConfigProperty;
class ConfigStorage;
class ConfigGroup;
class ConfigEntry;

///----

/// low-level configuration system
class BM_CORE_OBJECT_API IConfigSystem : public MainPoolData<NoCopy>
{
public:
    ///---

    // user storage for config vars
    INLINE ConfigStorage& storage() const { return *m_userStorage; }

    ///---

    // load/reload config
    virtual void load() = 0;

    // save the differences between base config and current config to user config file
    virtual void save() = 0;

    ///---

    // make/find config group with given name
    ConfigGroup& makeGroup(StringView name);

    // find config group, returns NULL if not found
    const ConfigGroup* findGroup(StringView name) const;

    // get config entry, creates an empty new one if not found
    ConfigEntry& makeEntry(StringView groupName, StringView entryName);

    // find config entry, return NULL if not found
    const ConfigEntry* findEntry(StringView groupName, StringView entryName) const;

    // find all groups starting with given start string
    Array<const ConfigGroup*> findAllGroups(StringView groupNameSubString) const;

    //--

    /// get all values
    const Array<StringBuf> readAllValues(StringView groupName, StringView entryName) const;

    /// get value (last value from a list or empty string)
    StringBuf readValue(StringView groupName, StringView entryName, const StringBuf& defaultValue = StringBuf::EMPTY()) const;

    /// read as integer, returns default value if not parsed correctly
    int readInt(StringView groupName, StringView entryName, int defaultValue = 0) const;

    /// read as a float, returns default value if not parsed correctly
    float readFloat(StringView groupName, StringView entryName, float defaultValue = 0) const;

    /// read as a boolean, returns default value if not parsed correctly
    bool readBool(StringView groupName, StringView entryName, bool defaultValue = 0) const;

    //--

    /// remove whole group
    bool removeGroup(StringView groupName);

    /// remove whole entry
    bool removeEntry(StringView groupName, StringView entryName);

    /// remove specific value from entry
    bool removeValue(StringView groupName, StringView entryName, StringView value);

    //---

    /// write value to config storage
    void writeValues(StringView groupName, StringView entryName, const Array<StringBuf>& values);

    /// write value to config storage
    void writeValue(StringView groupName, StringView entryName, StringView value);

    /// write integer value to config storage
    void writeInt(StringView groupName, StringView entryName, int value);

    /// write boolean value to config storage
    void writeBool(StringView groupName, StringView entryName, bool value);

    /// write floating point value to config storage
    void writeFloat(StringView groupName, StringView entryName, float value);

    //---

    // get all config properties
    Array<IConfigProperty*> configPropertyListCopy() const;

    // iterate over config properties
    void visitConfigProperties(const std::function<void(IConfigProperty*)>& func) const;

    //--

protected:
    IConfigSystem();
    ~IConfigSystem();

    ConfigStorage* m_userStorage = nullptr;
    ConfigStorage* m_baseStorage = nullptr;

    //--

    friend class IConfigProperty;

    struct PropertyGroup : public MainPoolData<NoCopy>
    {
        StringBuf name;
        HashMap<StringID, IConfigProperty*> entries;
    };

    SpinLock m_propertiesLock;
    HashMap<StringBuf, PropertyGroup*> m_propertiesGroups;
    HashSet<IConfigProperty*> m_properties;

    void registerConfigProperty(IConfigProperty* prop);
    void unregisterConfigProperty(IConfigProperty* prop);

    IConfigProperty* findConfigProperties(StringView groupName, StringView entryName) const;
};

//--

/// global configuration system
extern BM_CORE_OBJECT_API IConfigSystem& ConfigSystem();

//--

END_INFERNO_NAMESPACE()
