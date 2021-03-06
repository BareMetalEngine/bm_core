/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "configEntry.h"
#include "configGroup.h"
#include "bm/core/containers/include/hashSet.h"

BEGIN_INFERNO_NAMESPACE()

//---

ConfigEntry::ConfigEntry(ConfigGroup* group, const StringBuf& name)
    : m_group(group)
    , m_name(name)
{}

bool ConfigEntry::clear()
{
    auto lock  = CreateLock(m_lock);

    if (m_values.empty())
        return false;

    m_values.clear();
    modified();
    return true;
}

bool ConfigEntry::value(const StringBuf& value)
{
    auto lock  = CreateLock(m_lock);

    // same value ?
    if (m_values.size() == 1 && m_values[0] == value)
        return false;

    // clear existing values
    m_values.clear();

    // set new value, NOTE: empty values are not pushed by default
    if (!value.empty())
        m_values.pushBack(value);

    modified();
    return true;
}

bool ConfigEntry::appendValue(const StringBuf& value, bool onlyIfUnique/*=true*/)
{
    auto lock  = CreateLock(m_lock);

    // empty value, not pushed
    if (value.empty())
        return false;

    // check if already exists
    if (onlyIfUnique && m_values.contains(value))
        return false;

    // append
    m_values.pushBack(value);
    modified();
    return true;
}

bool ConfigEntry::removeValue(const StringBuf& value)
{
    auto lock  = CreateLock(m_lock);

    if (!m_values.remove(value))
        return false;

    modified();
    return true;
}

StringBuf ConfigEntry::value() const
{
    auto lock  = CreateLock(m_lock);
    return m_values.empty() ? StringBuf::EMPTY() : m_values.back();
}

const Array<StringBuf> ConfigEntry::values() const
{
    auto lock  = CreateLock(m_lock);
    return m_values;
}

int ConfigEntry::valueInt(const int defaultValue/* = 0*/) const
{
    auto lock  = CreateLock(m_lock);

    if (!m_values.empty())
    {
        int ret = defaultValue;
        if (m_values.back().view().match(ret))
            return ret;
    }

    return defaultValue;
}

float ConfigEntry::valueFloat(float defaultValue /*= 0*/) const
{
    auto lock  = CreateLock(m_lock);

    if (!m_values.empty())
    {
        float ret = defaultValue;
        if (m_values.back().view().match(ret))
            return ret;
    }

    return defaultValue;
}

bool ConfigEntry::valueBool(bool defaultValue /*= 0*/) const
{
    auto lock  = CreateLock(m_lock);

    if (!m_values.empty())
    {
        bool ret = defaultValue;
        if (m_values.back().view().match(ret))
            return ret;
    }

    return defaultValue;
}

void ConfigEntry::modified()
{
    m_group->modified();
}

bool ConfigEntry::Equal(const ConfigEntry& a, const ConfigEntry& b)
{
    // NOTE: potential deadlock if we do Equal(a,b) and Equal(b,a)
    auto lockA  = CreateLock(a.m_lock);
    auto lockB  = CreateLock(b.m_lock);

    // different sizes
    if (a.m_values.size() != b.m_values.size())
        return false;

    // compare values
    for (uint32_t i=0; i<a.m_values.size(); ++i)
        if (a.m_values[i] != b.m_values[i])
            return false;

    return true;
}

/*static bool HasCharsThatNeedEscaping(StringView str)
{
    for (auto ch  : str)
    {
        if (ch <= ' ' || ch >= 127)
            return true;
        if (ch >= 'A' && ch <= 'Z')
            continue;
        if (ch >= 'a' && ch <= 'z')
            continue;
        if (ch >= '0' && ch <= '9')
            continue;
        if (ch == '.' || ch == '_' || ch == '-' || ch == '+' || ch == '[' || ch == ']'|| ch == '('|| ch == ')' || ch == ','|| ch == '=')
            continue;
        return true;
    }

    return false;
}

static void WriteEscapedConfigString(IFormatStream& f, StringView str)
{
    if (HasCharsThatNeedEscaping(str))
    {
        f.append("\"");
        f << str; // TOOD: new line support
        f.append("\"");
    }
    else
    {
        f << str;
    }
}*/

void ConfigEntry::PrintDiff(IFormatStream& f, const ConfigEntry& cur, const ConfigEntry& base)
{
    // NOTE: potential dead lock
    auto lockA  = CreateLock(cur.m_lock);
    auto lockB  = CreateLock(base.m_lock);

    // we DONT have values but we had in the base, write as empty shit
    if (cur.m_values.empty())
    {
        f.appendf("{}=\n", cur.m_name);
    }

    // we only have one value, write it as a replacement
    else if (cur.m_values.size() == 1)
    {
        f.appendf("{}={}\n", cur.m_name);
    }

    // we have more than one value, make a proper diff
    else
    {
        HashSet<StringBuf> baseValues;
        HashSet<StringBuf> curValues;

        baseValues.reserve(base.m_values.size());
        for (auto& it : base.m_values)
            baseValues.insert(it);

        baseValues.reserve(cur.m_values.size());
        for (auto& it : cur.m_values)
            curValues.insert(it);

        // remove all entries from base config that we no longer have
        // NOTE: if we removed ALL values from the base than we can just use the replace
        bool hasDeltaValue = false;
        for (auto& it : base.m_values)
        {
            if (curValues.contains(it))
            {
                hasDeltaValue = true;
                break;
            }
        }

        // if have some base values remove the ones that we don't have
        if (hasDeltaValue)
        {
            for (auto& it : base.m_values)
                if (curValues.contains(it))
                    f.appendf("{}-={}\n", cur.m_name, it);
        }

        // print values we have that are not in the base
        for (auto& it : cur.m_values)
        {
            if (baseValues.contains(it))
            {
                if (hasDeltaValue)
                {
                    f.appendf("{}+={}\n", cur.m_name, it);
                }
                else
                {
                    f.appendf("{}={}\n", cur.m_name, it);
                    hasDeltaValue = true;
                }
            }
        }
    }
}

void ConfigEntry::Print(IFormatStream& f, const ConfigEntry& cur)
{
    bool hasDeltaValue = false;
    for (auto& it : cur.m_values)
    {
        if (hasDeltaValue)
        {
            f.appendf("{}+={}\n", cur.m_name, it);
        }
        else
        {
            f.appendf("{}={}\n", cur.m_name, it);
            hasDeltaValue = true;
        }
    }
}

//---

END_INFERNO_NAMESPACE()
