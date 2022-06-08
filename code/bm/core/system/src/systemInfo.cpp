/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "systemInfo.h"
#include "private.h"

BEGIN_INFERNO_NAMESPACE()

#ifdef PLATFORM_WINAPI

class UserNameString
{
public:
    UserNameString()
    {
        DWORD size = UNLEN;
        memset(m_text, 0, sizeof(m_text));
        GetUserNameA(m_text, &size);
        m_text[size] = 0;
            
        for (uint32_t i=0; i<size; ++i)
            if (m_text[i] && m_text[i] <= ' ')
                m_text[i] = '_';
    }

    INLINE const char* c_str() const
    {
        return m_text;
    }

private:
    char m_text[UNLEN+1];
};

#undef GetUserName
const char* GetUserName()
{
    static UserNameString theData;
    return theData.c_str();
}

//--

class HostNameString
{
public:
    HostNameString()
    {
        DWORD size = 256;
        memset(m_text, 0, sizeof(m_text));
        GetComputerNameExA(ComputerNameDnsHostname, m_text, &size);
        m_text[size] = 0;

        for (uint32_t i=0; i<size; ++i)
            if (m_text[i] && m_text[i] <= ' ')
                m_text[i] = '_';
    }

    INLINE const char* c_str() const
    {
        return m_text;
    }

private:
    char m_text[256+1];
};

const char* GetHostName()
{
    static HostNameString theData;
    return theData.c_str();
}

//--

const char* GetSystemName()
{
    return "Windows";
}

const char* GetEnv(const char* key)
{
    auto value = getenv(key);
    return value ? value : "";
}

bool GetRegistryKey(const char* path, const char* key, char* outBuffer, uint32_t& outBufferSize)
{
    HKEY hKey;
    LONG lRes = RegOpenKeyExA(HKEY_LOCAL_MACHINE, path, 0, KEY_READ, &hKey);
    if (lRes == ERROR_SUCCESS)
    {
        auto nError = RegQueryValueExA(hKey, key, 0, NULL, (LPBYTE)outBuffer, (DWORD*)&outBufferSize);
        return ERROR_SUCCESS == nError;
    }

    return false;
}

#elif defined(PLATFORM_POSIX)

    class UserNameString
{
public:
    UserNameString()
    {
        if (0 != getlogin_r(m_text, ARRAY_COUNT(m_text)))
        {
            strcpy_s(m_text, "unknown");
        }
    }

    INLINE const char* c_str() const
    {
        return m_text;
    }

private:
    char m_text[64];
};

const char* GetUserName()
{
    static UserNameString theData;
    return theData.c_str();
}

//--

class HostNameString
{
public:
    HostNameString()
    {
        if (0 != gethostname(m_text, ARRAY_COUNT(m_text)))
        {
            strcpy_s(m_text, "unknown");
        }
    }

    INLINE const char* c_str() const
    {
        return m_text;
    }

private:
    char m_text[64];
};

const char* GetHostName()
{
    static HostNameString theData;
    return theData.c_str();
}

//--

class SystemNameString
{
public:
    SystemNameString()
    {
        utsname name;
        if (0 != uname(&name))
        {
            strcpy_s(m_text, "unknown");
        }
        else
        {
            sprintf(m_text, "%s %s %s", name.sysname, name.version, name.version);
        }
    }

    INLINE const char* c_str() const
    {
        return m_text;
    }

private:
    char m_text[256];
};

const char* GetSystemName()
{
    static SystemNameString theData;
    return theData.c_str();
}

const char* GetEnv(const char* key)
{
    auto value = getenv(key);
    return value ? value : "";
}

bool GetRegistryKey(const char* path, const char* key, char* outBuffer, uint32_t& outBufferSize)
{
    return false;
}

#else

const char* GetUserName()
{
    return "GenericUser";
}

const char* GetHostName()
{
    return "GenericComputer";
}

const char* GetSystemName()
{
    return "GenericSystem";
}

const char* GetEnv(const char* key)
{
    return "";
}

bool GetRegistryKey(const char* path, const char* key, char* outBuffer, uint32_t& outBufferSize)
{
    return false;
}

#endif

END_INFERNO_NAMESPACE()
