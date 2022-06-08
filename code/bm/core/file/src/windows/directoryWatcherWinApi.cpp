/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "private.h"
#include "directoryWatcherWinApi.h"

#include "bm/core/containers/include/inplaceArray.h"

#ifdef PLATFORM_WINAPI

BEGIN_INFERNO_NAMESPACE_EX(windows)

//--

DirectoryWatcher::DirectoryWatcher(Array<wchar_t> rootPath)
    : m_watchedPath(rootPath)
{
    memset(&m_waitCondition, 0, sizeof(m_waitCondition));
    m_waitCondition.hEvent = ::CreateEvent(NULL, false, false, NULL);

    m_directoryHandle = ::CreateFileW(
        m_watchedPath.typedData(),
        FILE_LIST_DIRECTORY,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
        NULL
    );

    if (m_directoryHandle != INVALID_HANDLE_VALUE)
    {
        {
            ThreadSetup setup;
            setup.m_function = [this]() { watch(); };
            setup.m_priority = ThreadPriority::AboveNormal;
            setup.m_name = "IODirectoryWatcher";
            m_localThread.init(setup);
        }

        {
            ThreadSetup setup;
            setup.m_function = [this]() { dispatch(); };
            setup.m_priority = ThreadPriority::AboveNormal;
            setup.m_name = "IODirectoryWatcherDispatch";
            m_dispatchThread.init(setup);
        }
    }
    else
    {
        TRACE_WARNING("Cannot create a file system watcher for absolute path '{}'", (const wchar_t*)rootPath.typedData());
    }
}

DirectoryWatcher::~DirectoryWatcher()
{
    ::CancelIoEx(m_directoryHandle, &m_waitCondition);
    m_localThread.close();

    m_requestExit = true;
    m_dispatchThread.close();

    ::CloseHandle(m_waitCondition.hEvent);
}

void DirectoryWatcher::dispatch()
{
    InplaceArray<DirectoryWatcherEvent, 64> events;

    while (!m_requestExit.load())
    {
        events.reset();

        // dispatch normal events
        {
            m_pendingEventsLock.acquire();
            auto pendingEvents = std::move(m_pendingEvents);
            m_pendingEventsLock.release();

            for (const auto& evt : pendingEvents)
                events.pushBack(evt);
        }

        // dispatch file modification events
        {
            auto lock = CreateLock(m_pendingModificationsLock);

            auto pendingEvents = std::move(m_pendingModifications);
            for (auto& evt : pendingEvents)
            {
                if (evt.expires.reached())
                {
                    TRACE_WARNING("IO: Modification event for '{}' expired without resolution", evt.path);
                    continue;
                }

                if (evt.time.reached())
                {
                    TRACE_ERROR("Modified reached '{}'", evt.path);
                    bool emitEvent = true;
                    //if (CheckIfFileFinishedWriting(evt.path.c_str(), emitEvent))
                    {
                        if (emitEvent)
                        {
                            TRACE_ERROR("Modified writing finished '{}'", evt.path);
                            TRACE_SPAM("IO: Modification event for '{}' will be issued since file is no longer touched", evt.path);
                            auto& pe = events.emplaceBack();
                            pe.type = DirectoryWatcherEventType::FileContentChanged;
                            pe.path = std::move(evt.path);
                        }
                        else
                        {
                            TRACE_WARNING("File '{}' was lost", evt.path);
                        }
                    }
                    /*else
                    {
                        TRACE_ERROR("Modified writing NOT finished '{}'", evt.path);
                        TRACE_WARNING("IO: Modification event for '{}' posponed since file is still inacessible", evt.path);
                        evt.time = NativeTimePoint::Now() + 0.5;
                        m_pendingModifications.emplaceBack(std::move(evt));
                    }*/
                }
                else
                {
                    m_pendingModifications.emplaceBack(std::move(evt));
                }
            }
        }

        // dispatch events
        for (const auto& evt : events)
            dispatchEvent(evt);

        // limit CPU usage
        ::Sleep(200);
    }
}

void DirectoryWatcher::watch()
{
    DWORD maxEventsAtTime = 2U;
    DWORD bufSize = maxEventsAtTime * (sizeof(FILE_NOTIFY_EXTENDED_INFORMATION) + MAX_PATH * sizeof(wchar_t));

    auto info = (FILE_NOTIFY_EXTENDED_INFORMATION*)malloc(bufSize);
    while (true)
    {
        DWORD bytesReturned = 0U;
        DWORD filter = FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE;
        memset(info, 0, bufSize);

        if (::ReadDirectoryChangesExW(m_directoryHandle, info, bufSize, true, filter, &bytesReturned, &m_waitCondition, NULL, ReadDirectoryNotifyExtendedInformation))
        {
            // canceled ?
            if (!::GetOverlappedResult(m_directoryHandle, &m_waitCondition, &bytesReturned, true))
            {
                TRACE_SPAM("Failed GetOverlappedResult, exiting watch thread");
                break;
            }

            auto curInfo  = info;
            while (curInfo) // there may be more than one info returned in the buffer
            {
                auto action = 0U;

                StringBuilder fullPathBuilder;
                fullPathBuilder.append(m_watchedPath.typedData());
                fullPathBuilder.append(curInfo->FileName, curInfo->FileNameLength / sizeof(wchar_t));

                auto fullPath = StringBuf(fullPathBuilder.view());

                //TRACE_INFO("Event {} for '{}'", curInfo->Action, fullPath);
                            
                bool send = true;
                bool directory = 0 != (curInfo->FileAttributes & FILE_ATTRIBUTE_DIRECTORY);

                if (curInfo->Action == FILE_ACTION_ADDED || curInfo->Action == FILE_ACTION_RENAMED_NEW_NAME)
                {
                    auto lock = CreateLock(m_pendingEventsLock);
                    auto& evt = m_pendingEvents.emplaceBack();
                    evt.path = std::move(fullPath);

                    if (directory)
                        evt.type = DirectoryWatcherEventType::DirectoryAdded;
                    else
                        evt.type = DirectoryWatcherEventType::FileAdded;
                }
                else if (curInfo->Action == FILE_ACTION_REMOVED || curInfo->Action == FILE_ACTION_RENAMED_OLD_NAME)
                {
                    auto lock = CreateLock(m_pendingEventsLock);
                    auto& evt = m_pendingEvents.emplaceBack();
                    evt.path = std::move(fullPath); 

                    if (directory)
                        evt.type = DirectoryWatcherEventType::DirectoryRemoved;
                    else
                        evt.type = DirectoryWatcherEventType::FileRemoved;
                }
                else if (curInfo->Action == FILE_ACTION_MODIFIED)
                {
                    if (!directory)
                    {
                        auto lock = CreateLock(m_pendingModificationsLock);

                        bool newEntry = true;
                        for (auto& entry : m_pendingModifications)
                        {
                            if (entry.path == fullPath)
                            {
                                //TRACE_ERROR("Modified known '{}'", fullPath);
                                entry.time = NativeTimePoint::Now() + 0.5;
                                entry.expires = NativeTimePoint::Now() + 60.0;
                                newEntry = false;
                                break;
                            }
                        }

                        if (newEntry)
                        {
                            //TRACE_ERROR("Modified new '{}'", fullPath);
                            auto& entry = m_pendingModifications.emplaceBack();
                            entry.path = std::move(fullPath);
                            entry.time = NativeTimePoint::Now() + 0.5;
                            entry.expires = NativeTimePoint::Now() + 60.0;
                        }
                    }
                }

                // Go to the next info
                curInfo = curInfo->NextEntryOffset ? OffsetPtr<FILE_NOTIFY_EXTENDED_INFORMATION>(curInfo, curInfo->NextEntryOffset) : nullptr;
            }
        }
    }

    free(info);
}

//--

END_INFERNO_NAMESPACE_EX(windows)

#endif

