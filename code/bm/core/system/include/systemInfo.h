/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

BEGIN_INFERNO_NAMESPACE()

// Get name of the user
#undef GetUserName
extern BM_CORE_SYSTEM_API const char* GetUserName();

// Get computer name
extern BM_CORE_SYSTEM_API const char* GetHostName();

// Get string that describes the system
extern BM_CORE_SYSTEM_API const char* GetSystemName();

// Get value of the environment variable
extern BM_CORE_SYSTEM_API const char* GetEnv(const char* key);

// Get value from the registry
extern BM_CORE_SYSTEM_API bool GetRegistryKey(const char* path, const char* key, char* outBuffer, uint32_t& outBufferSize);

END_INFERNO_NAMESPACE()
