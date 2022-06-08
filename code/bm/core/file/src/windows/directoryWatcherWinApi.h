/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

#include "fileDirectoryWatcher.h"

#ifdef PLATFORM_WINAPI

BEGIN_INFERNO_NAMESPACE_EX(windows)

//--

class DirectoryWatcher : public IDirectoryWatcher
{
public:
    DirectoryWatcher(Array<wchar_t> rootPath);
    virtual ~DirectoryWatcher();

private:
    Array<wchar_t> m_watchedPath;

    Thread m_localThread;
    Thread m_dispatchThread;

    OVERLAPPED m_waitCondition;
    HANDLE m_directoryHandle;

    std::atomic<bool> m_requestExit = false;

    Array<DirectoryWatcherEvent> m_pendingEvents;
    SpinLock m_pendingEventsLock;

    struct PendingModification
    {
        StringBuf path;
        NativeTimePoint time;
        NativeTimePoint expires;
    };

    Array<PendingModification> m_pendingModifications;
    SpinLock m_pendingModificationsLock;

    void watch();
    void dispatch();
};

//--

END_INFERNO_NAMESPACE_EX(windows)

#endif
