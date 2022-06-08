/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

/// this "Module declarations" are used by the generated code to glue everything together in case of static libs and DLLs

BEGIN_INFERNO_NAMESPACE_EX(modules)

//--

typedef void (*TModuleInitializationFunc)();

struct ModuleInfo
{
    const char* name = nullptr;
    const char* dateCompiled = nullptr;
    const char* timeCompiled = nullptr;
    uint64_t compilerVersion = 0;
    TModuleInitializationFunc initFunc = nullptr; // cleared after initialization
    bool hasOrderedInit = false;
};

/// register information about a module being initialized
extern BM_CORE_SYSTEM_API void RegisterModule(const char* name, const char* dateCompiled, const char* timeCompiled, uint64_t compilerVer, TModuleInitializationFunc initFunc);

/// get list of initialized modules
extern BM_CORE_SYSTEM_API void GetRegisteredModules(uint32_t maxEntries, ModuleInfo* outTable, uint32_t& outNumEntries);

/// load a dynamic module if its not yet loaded
extern BM_CORE_SYSTEM_API void LoadDynamicModule(const char* name);

/// check if we have module loaded
extern BM_CORE_SYSTEM_API bool HasModuleLoaded(const char* name);

/// call initialization on all not yet initialized modules that were just loaded
extern BM_CORE_SYSTEM_API void InitializePendingModules();

//--

/// load a dynamic library
extern BM_CORE_SYSTEM_API void* LoadDynamicLibrary(const char* name);

/// unload dynamic library
extern BM_CORE_SYSTEM_API void UnloadDynamicLibrary(void* ptr);

/// find function in dynamic library
extern BM_CORE_SYSTEM_API void* FindDynamicLibraryFunction(void* ptr, const char* name);

//--

END_INFERNO_NAMESPACE_EX(modules)
