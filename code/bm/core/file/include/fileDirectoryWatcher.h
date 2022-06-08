/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

BEGIN_INFERNO_NAMESPACE()

///--

// type of observed file system event
enum class DirectoryWatcherEventType : uint8_t
{
    FileAdded,
    DirectoryAdded,
    FileRemoved,
    DirectoryRemoved,
    FileContentChanged,
    FileMetadataChanged,
};

// directory watcher event, sent to the listener interface
struct DirectoryWatcherEvent
{
    DirectoryWatcherEventType type;
    StringBuf path;
};

///--

// directory watcher callback
// NOTE: this may be called at any time and from any thread (sometimes a deep OS thread)
class BM_CORE_FILE_API IDirectoryWatcherListener : public MainPoolData<NoCopy>
{
public:
    virtual ~IDirectoryWatcherListener();

    /// handle file event
    virtual void handleEvent(const DirectoryWatcherEvent& evt) = 0;
};

///--

// an abstract directory watcher interface
// NOTE: the directory watcher watches the specified directory and all sub directories
class BM_CORE_FILE_API IDirectoryWatcher : public IReferencable
{
public:
    virtual ~IDirectoryWatcher();

    //! attach listener
    virtual void attachListener(IDirectoryWatcherListener* listener);

    //! detach event listener
    virtual void dettachListener(IDirectoryWatcherListener* listener);

protected:
    IDirectoryWatcher();

    //--

    Mutex m_listenersLock;
    Array<IDirectoryWatcherListener*> m_listeners;
    bool m_listenersIterated = false;

    //--

    void dispatchEvent(const DirectoryWatcherEvent& evt);

    //--
};

///--

END_INFERNO_NAMESPACE()
