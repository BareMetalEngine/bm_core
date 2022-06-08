/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "configEntry.h"
#include "configGroup.h"
#include "configStorage.h"
#include "configProperty.h"
#include "configSystem.h"

BEGIN_INFERNO_NAMESPACE()

//---

IConfigSystem::IConfigSystem()
{
    m_userStorage = new ConfigStorage();
    m_baseStorage = new ConfigStorage();
}

IConfigSystem::~IConfigSystem()
{
    delete m_userStorage;
    m_userStorage = nullptr;

    delete m_baseStorage;
    m_baseStorage = nullptr;
}

//--

ConfigGroup& IConfigSystem::makeGroup(StringView name)
{
    return m_userStorage->group(name);
}

const ConfigGroup* IConfigSystem::findGroup(StringView name) const
{
    return const_cast<ConfigGroup*>(m_userStorage->findGroup(name));
}

ConfigEntry& IConfigSystem::makeEntry(StringView groupName, StringView entryName)
{
    return m_userStorage->group(groupName).entry(entryName);
}

const ConfigEntry* IConfigSystem::findEntry(StringView groupName, StringView entryName) const
{
    if (auto group = m_userStorage->findGroup(groupName))
        return group->findEntry(entryName);
    else
        return nullptr;
}

Array<const ConfigGroup*> IConfigSystem::findAllGroups(StringView groupNameSubString) const
{
    return m_userStorage->findAllGroups(groupNameSubString);
}

//--

StringBuf IConfigSystem::readValue(StringView groupName, StringView entryName, const StringBuf& defaultValue /*= StringBuf::EMPTY()*/) const
{
    if (const auto* entry = findEntry(groupName, entryName))
        return entry->value();
    else
        return defaultValue;
}

const Array<StringBuf> IConfigSystem::readAllValues(StringView groupName, StringView entryName) const
{
    if (const auto* entry = findEntry(groupName, entryName))
        return entry->values();
    else
        return Array<StringBuf>();
}

int IConfigSystem::readInt(StringView groupName, StringView entryName, int defaultValue) const
{
    if (const auto* entry = findEntry(groupName, entryName))
        return entry->valueInt();
    else
        return defaultValue;
}

float IConfigSystem::readFloat(StringView groupName, StringView entryName, float defaultValue) const
{
    if (const auto* entry = findEntry(groupName, entryName))
        return entry->valueFloat();
    else
        return defaultValue;
}

bool IConfigSystem::readBool(StringView groupName, StringView entryName, bool defaultValue) const
{
    if (const auto* entry = findEntry(groupName, entryName))
        return entry->valueBool();
    else
        return defaultValue;
}

//--

bool IConfigSystem::removeGroup(StringView groupName)
{
    return m_userStorage->removeGroup(groupName);
}

bool IConfigSystem::removeEntry(StringView groupName, StringView entryName)
{
    return m_userStorage->removeEntry(groupName, entryName);
}

bool IConfigSystem::removeValue(StringView groupName, StringView entryName, StringView value)
{
    if (auto* entry = const_cast<ConfigEntry*>(findEntry(groupName, entryName)))
    {
        if (entry->removeValue(StringBuf(value)))
        {
            if (auto* prop = findConfigProperties(groupName, entryName))
                prop->readFromStorage();

            return true;
        }
    }

    return false;
}

//---

static bool CompareValues(const Array<StringBuf>& a, const Array<StringBuf>& b)
{
    if (a.size() == b.size())
    {
        for (uint32_t i = 0; i < a.size(); ++i)
            if (a[i] != b[i])
                return false;

        return true;
    }

    return false;
}

void IConfigSystem::writeValues(StringView groupName, StringView entryName, const Array<StringBuf>& values)
{
    auto& entry = makeEntry(groupName, entryName);

    if (!CompareValues(entry.values(), values))
    {
        entry.clear();

        for (const auto& val : values)
            entry.appendValue(val);

        if (auto* prop = findConfigProperties(groupName, entryName))
            prop->readFromStorage();
    }
}

void IConfigSystem::writeValue(StringView groupName, StringView entryName, StringView value)
{
    Array<StringBuf> values;
    if (value)
        values.emplaceBack(value);

    writeValues(groupName, entryName, values);
}

void IConfigSystem::writeInt(StringView groupName, StringView entryName, int value)
{
    writeValue(groupName, entryName, TempString("{}", value));
}

void IConfigSystem::writeBool(StringView groupName, StringView entryName, bool value)
{
    writeValue(groupName, entryName, TempString("{}", value));
}

void IConfigSystem::writeFloat(StringView groupName, StringView entryName, float value)
{
    writeValue(groupName, entryName, TempString("{}", value));
}

//---

void IConfigSystem::registerConfigProperty(IConfigProperty* prop)
{
    DEBUG_CHECK_RETURN(prop);
    DEBUG_CHECK_RETURN(!prop->group().empty());
    DEBUG_CHECK_RETURN(!prop->name().empty());

    auto lock = CreateLock(m_propertiesLock);

    if (m_properties.insert(prop))
    {
        PropertyGroup* group = nullptr;
        if (!m_propertiesGroups.find(prop->group(), group))
        {
            group = new PropertyGroup();
            group->name = StringBuf(prop->group());
            group->entries.reserve(30);
        }

        group->entries[prop->name()] = prop;
    }
}

void IConfigSystem::unregisterConfigProperty(IConfigProperty* prop)
{
    auto lock = CreateLock(m_propertiesLock);

    if (m_properties.remove(prop))
    {
        PropertyGroup* group = nullptr;
        m_propertiesGroups.find(prop->group(), group);

        if (group)
            group->entries.remove(prop->name());
    }
}

Array<IConfigProperty*> IConfigSystem::configPropertyListCopy() const
{
    auto lock = CreateLock(m_propertiesLock);
    return m_properties.keys();
}

void IConfigSystem::visitConfigProperties(const std::function<void(IConfigProperty* prop)>& func) const
{
    auto lock = CreateLock(m_propertiesLock);

    for (auto* prop : m_properties.keys())
        func(prop);
}

IConfigProperty* IConfigSystem::findConfigProperties(StringView groupName, StringView entryName) const
{
    auto lock = CreateLock(m_propertiesLock);

    PropertyGroup* group = nullptr;
    if (m_propertiesGroups.find(groupName, group))
    {
        if (auto name = StringID::Find(entryName))
        {
            IConfigProperty* prop = nullptr;
            group->entries.find(name, prop);
            return prop;
        }
    }

    return nullptr;
}

//--

END_INFERNO_NAMESPACE()