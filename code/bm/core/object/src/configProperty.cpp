/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "configEntry.h"
#include "configGroup.h"
#include "configProperty.h"
#include "configSystem.h"

#include "bm/core/containers/include/hashMap.h"
#include "bm/core/containers/include/hashSet.h"
#include "bm/core/containers/include/stringView.h"
#include "bm/core/containers/include/inplaceArray.h"

BEGIN_INFERNO_NAMESPACE()

//---

IConfigProperty::IConfigProperty(StringView groupName, StringView name, StringView flags)
    : m_group(groupName)
    , m_name(name)
{
    m_developer = (flags.findFirstChar('D') != INDEX_NONE);
    m_readOnly = (flags.findFirstChar('R') != INDEX_NONE);

    ConfigSystem().registerConfigProperty(this);
}

IConfigProperty::~IConfigProperty()
{
    ConfigSystem().unregisterConfigProperty(this);
}

/*void IConfigProperty::IterateAll(const std::function<void(IConfigProperty* prop)>& func)
{
    prv::ConfigPropertyRegistry::GetInstance().iterate(func);
}

IConfigProperty* IConfigProperty::FindProperty(StringView groupName, StringView entryName)
{
    return prv::ConfigPropertyRegistry::GetInstance().find(groupName, entryName);
}*/

//---

void IConfigProperty::sendToStorage()
{
    InplaceArray<StringBuf, 10> values;
    captureToString(values);

    auto& entry = ConfigSystem().makeEntry(m_group, m_name.view());
    entry.clear();

    for (const auto& val : values)
        entry.appendValue(val);
}

bool IConfigProperty::readFromStorage()
{
    if (const auto* entry = ConfigSystem().findEntry(m_group, m_name.view()))
        return applyFromString(entry->values());

    return false;
}

//---

END_INFERNO_NAMESPACE()
