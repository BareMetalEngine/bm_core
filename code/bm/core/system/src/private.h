/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

#ifdef PLATFORM_WINDOWS

    #ifdef _WIN32_WINNT
        #undef _WIN32_WINNT
    #endif
    #ifdef NTDDI_VERSION
        #undef NTDDI_VERSION
    #endif
    #ifdef _WIN32_WINDOWS
        #undef _WIN32_WINDOWS
    #endif

    // Define windows versions
    #define _WIN32_WINNT _WIN32_WINNT_VISTA
    #define NTDDI_VERSION NTDDI_VISTASP1
    #define _WIN32_WINDOWS 0x0501

    // Suppress stuff from windows.h
    #define _TCHAR_DEFINED
    
    // Security stuff
    #ifndef _CRT_NON_CONFORMING_SWPRINTFS
        #define _CRT_NON_CONFORMING_SWPRINTFS
    #endif

    // Security stuff
    #ifndef _CRT_SECURE_NO_DEPRECATE
        #define _CRT_SECURE_NO_DEPRECATE
    #endif

    // Security stuff
    #ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
        #define _WINSOCK_DEPRECATED_NO_WARNINGS
    #endif

    // Windows specific headers
    #include <winsock2.h>
    #include <windows.h>
    #include <locale.h>
    #include <process.h>
    #include <DbgHelp.h>
    #include <windows.h>
    #include <Lmcons.h>

#elif defined(PLATFORM_LINUX)

    #include <pthread.h>
    #include <syscall.h>
    #include <unistd.h>
    #include <semaphore.h>
    #include <sys/time.h>
    #include <sys/signal.h>
    #include <sys/utsname.h>
    #include <sys/syscall.h>
    #include <sched.h>
    #include <time.h>

#elif defined(PLATFORM_PROSPERO)

    #include <scebase_common.h>
    #include <scebase.h>
    #include <semaphore.h>
    #include <kernel.h>

#else

    #error "Please define the platform"

#endif


