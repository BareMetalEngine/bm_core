/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "module.h"
#include "atomic.h"
#include "private.h"

BEGIN_INFERNO_NAMESPACE_EX(modules)

static const uint32_t MAX_MODULES = 512;

static ModuleInfo GModules[MAX_MODULES];
static std::atomic<uint32_t> GModulesCount(0);

static ModuleInfo* GModuleInitOrder[MAX_MODULES];
static std::atomic<uint32_t> GModuleInitOrderCount(0);

void RegisterModule(const char* name, const char* dateCompiled, const char* timeCompiled, uint64_t compilerVer, TModuleInitializationFunc initFunc)
{
    auto index = GModulesCount++;
    if (index < MAX_MODULES)
    {
        //TRACE_INFO("Registered module '{}' at index {}", name, index);
        GModules[index].name = name;
        GModules[index].dateCompiled = dateCompiled;
        GModules[index].timeCompiled = timeCompiled;
        GModules[index].compilerVersion = compilerVer;
        GModules[index].initFunc = initFunc;
        GModules[index].hasOrderedInit = false;
    }
}

void GetRegisteredModules(uint32_t maxEntries, ModuleInfo* outTable, uint32_t& outNumEntries)
{
    auto count = std::min<uint32_t>(maxEntries, GModulesCount.load());
    outNumEntries = count;

    for (uint32_t i = 0; i < count; ++i)
        outTable[i] = GModules[i];
}

void LoadDynamicModule(const char* name)
{
#if !defined(PLATFORM_WINDOWS)
    TRACE_ERROR("Loading dynamic module '{}' is only supported on Windows", name);
#elif defined(BUILD_AS_LIBS)
    TRACE_ERROR("Unable to load dynamic module '{}' when built with static libs");
#else
    char moduleFileName[512];
    strcpy_s(moduleFileName, ARRAYSIZE(moduleFileName), name);
    strcat_s(moduleFileName, ARRAYSIZE(moduleFileName), ".dll");

    auto handle = LoadLibraryA(moduleFileName);
    if (handle == NULL)
    {
        auto errorCode = GetLastError();
        FATAL_ERROR(TempString("Failed to load dynamic module '{}', error code = {}", name, Hex(errorCode)));
    }
    else
    {
        TRACE_SPAM("Dynamic module '{}' loaded at {}", name, Hex(handle));
    }
#endif

    auto count = GModulesCount.load(); // we may add modules from threads
    for (uint32_t i = 0; i < count; ++i)
    {
        if (0 == strcmp(GModules[i].name, name))
        {
            auto index = GModuleInitOrderCount++;
            GModules[i].hasOrderedInit = true;
            GModuleInitOrder[index] = &GModules[i];
        }
    }
}

void InitializePendingModules()
{
    for (;;)
    {
        bool somethingInitialized = false;

        {
            auto count = GModuleInitOrderCount.load(); // we may add modules from threads
            for (uint32_t i = 0; i < count; ++i)
            {
                auto& module = *GModuleInitOrder[i];

                if (module.initFunc)
                {
                    //TRACE_SPAM("Initializing module '{}'", module.name);

                    somethingInitialized = true;
                    module.initFunc();
                    module.initFunc = nullptr;
                }
            }
        }

        {
            auto count = GModulesCount.load(); // we may add modules from threads
            for (uint32_t i = 0; i < count; ++i)
            {
                auto& module = GModules[i];

                if (module.initFunc && !module.hasOrderedInit)
                {
                    //TRACE_SPAM("Initializing module '{}'", module.name);

                    somethingInitialized = true;
                    module.initFunc();
                    module.initFunc = nullptr;
                }
            }
        }

        if (!somethingInitialized)
            break;
    }
}

bool HasModuleLoaded(const char* name)
{
    // look for module
    auto count = GModulesCount.load();
    for (uint32_t i=0; i<count; ++i)
    {
        if (0 == strcmp(GModules[i].name, name))
            return true;
    }

    // module is not loaded
    return false;
}

//--

void* LoadDynamicLibrary(const char* name)
{
    void* ret = nullptr;

#ifdef PLATFORM_WINAPI
    char moduleFileName[512];
    strcpy_s(moduleFileName, ARRAYSIZE(moduleFileName), name);
    strcat_s(moduleFileName, ARRAYSIZE(moduleFileName), ".dll");

    ret = (void*) LoadLibraryA(moduleFileName);
    if (ret)
    {
        TRACE_SPAM("Dynamic library '{}' loaded at {}", name, Hex(ret));
    }
    else
    {
        auto errorCode = GetLastError();
        TRACE_ERROR("Failed to load dynamic library '{}', error code = {}", name, Hex(errorCode));
    }
#else
    TRACE_ERROR("Loading dynamic library '{}' not supported", name);
#endif

    return ret;
}

void UnloadDynamicLibrary(void* ptr)
{
    if (ptr)
    {
#ifdef PLATFORM_WINAPI
        if (FreeLibrary((HMODULE)ptr))
        {
            TRACE_SPAM("Dynamic library at {} unloaded", Hex(ptr));
        }
        else
        {
            auto errorCode = GetLastError();
            TRACE_ERROR("Failed to unload dynamic library at {}, error code = {}", Hex(ptr), Hex(errorCode));
        }
#endif
    }
}

void* FindDynamicLibraryFunction(void* ptr, const char* name)
{
    void* func = nullptr;

    if (ptr)
    {
#ifdef PLATFORM_WINAPI
        func = GetProcAddress((HMODULE)ptr, name);
#endif
    }

    return func;
}

//--

END_INFERNO_NAMESPACE_EX(modules)
