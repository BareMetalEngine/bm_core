/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "configStorage.h"
#include "configSystemFileBased.h"
#include "bm/core/containers/include/path.h"

BEGIN_INFERNO_NAMESPACE()

ConfigSystemFileBased::ConfigSystemFileBased()
{
    const auto& engineDir = FileSystem().globalPath(FileSystemGlobalPath::EngineDir);
    m_engineConfigDir = TempString("{}data/config/", engineDir);
    ConformPathSeparators(m_engineConfigDir);
    TRACE_INFO("Engine config directory: '{}'", m_engineConfigDir);

    const auto& configDir = FileSystem().globalPath(FileSystemGlobalPath::UserConfigDir);
    m_userConfigFile = TempString("{}UserConfig.ini", configDir);
    ConformPathSeparators(m_userConfigFile);
    TRACE_INFO("User config file: '{}'", m_userConfigFile);

#if 0
    const auto& projectDir = fs->globalPath(FileSystemGlobalPath::ProjectDir);
    if (projectDir)
    {
        m_projectConfigDir = TempString("{}config/", projectDir);
        ConformPathSeparators(m_projectConfigDir);

        TRACE_INFO("Project config directory: '{}'", m_projectConfigDir);
    }
#endif
}

void ConfigSystemFileBased::load()
{
    m_baseStorage->clear();
    loadBaseConfig(*m_baseStorage);

    m_userStorage->clear();
    ConfigStorage::Append(*m_userStorage, *m_baseStorage);
    loadUserConfig(*m_userStorage);

    visitConfigProperties([this](IConfigProperty* prop)
        {
            if (!prop->readFromStorage())
                prop->sendToStorage(); // if reading failed it means we have a missing entry
        });
}

void ConfigSystemFileBased::save()
{
    visitConfigProperties([](IConfigProperty* prop)
        {
            prop->sendToStorage();
        });

    saveUserConfig(*m_userStorage, *m_baseStorage);
}

//---

static void CollectConfigFiles(StringView path, Array<StringBuf>& outConfigList)
{
    InplaceArray<StringBuf, 10> localList;
    FileSystem().collectLocalFiles(path, "*.ini", localList);

    std::sort(localList.begin(), localList.end()); // ensure determined order, allows files to be named 10_config, 20_config etc

    for (const auto& localFileName : localList)
        outConfigList.pushBack(TempString("{}{}", path, localFileName));
}

bool ConfigSystemFileBased::loadBaseConfig(ConfigStorage& outStorage) const
{
    InplaceArray<StringBuf, 30> paths;
    CollectConfigFiles(m_engineConfigDir, paths);

#ifdef PLATFORM_WINDOWS
    CollectConfigFiles(TempString("{}platform/windows/", m_engineConfigDir), paths);
#elif defined(PLATFORM_LINUX)
    CollectConfigFiles(TempString("{}platform/linux/", m_engineConfigDir), paths);
#elif defined(PLATFORM_PROSPERO)
    CollectConfigFiles(TempString("{}platform/prospero/", m_engineConfigDir), paths);
#elif defined(PLATFORM_SCARLETT)
    CollectConfigFiles(TempString("{}platform/scarlett/", m_engineConfigDir), paths);
#endif

    /*if (!m_projectConfigDir.empty())
        CollectConfigFiles(m_fileSystem, m_projectConfigDir, paths);*/

    TRACE_INFO("Found {} config files", paths.size());

    for (const auto& path : paths)
    {
        StringBuf configContent;
        if (FileSystem().loadFileToString(path, configContent))
        {
            ConfigStorage tempStorage;
            if (ConfigStorage::Load(configContent, tempStorage))
            {
                TRACE_INFO("Applied config file '{}'", path);
                ConfigStorage::Append(outStorage, tempStorage); // append atomically - all or nothing
            }
            else
            {
                TRACE_WARNING("Unable to load config file '{}'", path);
            }
        }
        else
        {
            TRACE_WARNING("Unable to open config file '{}'", path);
        }
    }

    return true;
}

void ConfigSystemFileBased::loadUserConfig(ConfigStorage& outStorage) const
{
    outStorage.clear();

    if (FileSystem().fileInfo(m_userConfigFile))
    {
        StringBuf configContent;
        if (FileSystem().loadFileToString(m_userConfigFile, configContent))
        {
            if (ConfigStorage::Load(configContent, outStorage))
            {
                TRACE_INFO("Loaded user config from '{}'", m_userConfigFile);
            }
            else
            {
                TRACE_WARNING("Failed to parse user config from '{}'", m_userConfigFile);
            }
        }
        else
        {
            TRACE_WARNING("Failed to load user config from '{}'", m_userConfigFile);
        }
    }
    else
    {
        TRACE_INFO("No user config at '{}'", m_userConfigFile);
    }
}

bool ConfigSystemFileBased::saveUserConfig(const ConfigStorage& user, const ConfigStorage& base) const
{
    StringBuilder txt;
    ConfigStorage::Save(txt, user, base);

    return FileSystem().saveFileFromString(m_userConfigFile, txt.view());
}

//---

static IConfigSystem* CreateConfigSystem()
{
    return new ConfigSystemFileBased();
}

IConfigSystem& ConfigSystem()
{
    static IConfigSystem* GConfigSystem = CreateConfigSystem();
    return *GConfigSystem;
}

//---

END_INFERNO_NAMESPACE()