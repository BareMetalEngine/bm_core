/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

BEGIN_INFERNO_NAMESPACE()

//-----------------------------------------------------------------------------

/// Thread ID
typedef uint32_t ThreadID;

//-----------------------------------------------------------------------------

/// Thread priority
enum class ThreadPriority : uint8_t
{
    Normal,
    AboveNormal,
    BelowNormal,
};

//-----------------------------------------------------------------------------

/// Thread function
typedef std::function<void()> TThreadFunc;

//-----------------------------------------------------------------------------

// thread setup
struct BM_CORE_SYSTEM_API ThreadSetup : public NoCopy
{
    const char* m_name;  // debug name of the thread

    ThreadPriority m_priority; //  thread priority
    uint32_t m_stackSize; // thead stack size
    int m_affinity = -1; // affinity to CPU cores on the system

    TThreadFunc m_function; // thread function to run

    ThreadSetup();
};

//-----------------------------------------------------------------------------

/// This is the base thread interface for all runnable thread classes
class BM_CORE_SYSTEM_API Thread : public NoCopy
{
public:
    Thread();
    Thread(Thread&& other);
    Thread& operator=(Thread&& other);
    ~Thread(); // NOTE: closes automatically

    //! initialize thread
    void init(const ThreadSetup& setup);

    //! close this thread, waits for the thread to finish, allows to call init again
    void close(bool terminateIfNeeded = false);

    //--

    //! Get ID of current thread
    static ThreadID CurrentThreadID();

    //! Get number of cores in the system
    static uint32_t NumberOfCores();

    //! Change name of current thread (even not created by us)
    static void SetThreadName(const char* name);

    //! Set affinity of the thread
    static void SetThreadAffinity(int core);

    //! Wait some time, time is in ms
    static void Sleep(uint32_t ms);

    //! Yield current thread
    static void YieldThread();

    //--

protected:
    void* m_systemThreadHandle = nullptr;
};

//--

END_INFERNO_NAMESPACE()
