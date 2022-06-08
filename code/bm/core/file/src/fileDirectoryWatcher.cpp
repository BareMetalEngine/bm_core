/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "fileDirectoryWatcher.h"

BEGIN_INFERNO_NAMESPACE()

//--

IDirectoryWatcherListener::~IDirectoryWatcherListener()
{}

//--

IDirectoryWatcher::~IDirectoryWatcher()
{}

IDirectoryWatcher::IDirectoryWatcher()
{}

void IDirectoryWatcher::attachListener(IDirectoryWatcherListener* listener)
{
    auto lock = CreateLock(m_listenersLock);
    m_listeners.pushBackUnique(listener);
}

void IDirectoryWatcher::dettachListener(IDirectoryWatcherListener* listener)
{
    auto lock = CreateLock(m_listenersLock);

    auto index = m_listeners.find(listener);
    if (INDEX_NONE != index)
        m_listeners[index] = nullptr;
}

void IDirectoryWatcher::dispatchEvent(const DirectoryWatcherEvent& evt)
{
    // dispatch events
    {
        bool hasEmptyEntry = false;

        auto lock = CreateLock(m_listenersLock);
        for (auto listener : m_listeners)
            if (listener)
                listener->handleEvent(evt);
            else
                hasEmptyEntry = true;

        if (hasEmptyEntry)
            m_listeners.removeAll(nullptr); // preserve order (strange bugs otherwise..)
    }
}

//--

END_INFERNO_NAMESPACE()
