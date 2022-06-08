/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

#include "configSystem.h"

BEGIN_INFERNO_NAMESPACE()

///----

/// low-level configuration system based on local .ini files
class BM_CORE_OBJECT_API ConfigSystemFileBased : public IConfigSystem
{
public:
    ConfigSystemFileBased();

    virtual void load() override final;
    virtual void save() override final;

private:
    bool loadBaseConfig(ConfigStorage& outStorage) const;
    void loadUserConfig(ConfigStorage& outStorage) const;
    bool saveUserConfig(const ConfigStorage& user, const ConfigStorage& base) const;

    StringBuf m_engineConfigDir;
    StringBuf m_userConfigFile;
};

///---

END_INFERNO_NAMESPACE()
