/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

#include "bm/core/memory/include/structureAllocator.h"
#include "bm/core/containers/include/queue.h"

#ifdef PLATFORM_WINAPI

BEGIN_INFERNO_NAMESPACE_EX(windows)

//--

// dispatch for IO jobs
class AsyncReadDispatcher : public MainPoolData<NoCopy>
{
public:
    AsyncReadDispatcher();
    ~AsyncReadDispatcher();

    void scheduleAsync(HANDLE hAsyncFile, FileAbsoluteRange range, void* outMemory, TAsyncReadCallback callback);

private:
    struct Token
    {
        OVERLAPPED m_overlapped; // MUST BE FIRST
		HANDLE m_hAsyncHandle = nullptr;
        void* m_memory = nullptr;
        uint32_t m_size = 0;
        uint32_t m_index = 0;
        AsyncReadDispatcher* m_dispatcher = nullptr;
        TAsyncReadCallback m_callback;

        INLINE Token()
        {
            memzero(&m_overlapped, sizeof(m_overlapped));
        }
    };

	std::atomic<uint32_t> m_tokenCounter;
    StructureAllocator<Token> m_tokenPool;
	SpinLock m_tokenPoolLock;

    Thread m_ioCompletionThread;

    Queue<Token*> m_tokensToExecute;
    SpinLock m_tokensToExecuteLock;

    Semaphore m_tokensCounter;
    std::atomic<uint32_t> m_exiting;

    bool m_opened = true;

    uint32_t m_numRequests = 0;
    uint32_t m_numRequestFailed = 0;
    uint64_t m_totalDataRead = 0;

    Token* popTokenFromQueue();
    Token* allocToken();
	void releaseToken(Token* token);

    void threadFunc();
    void processToken(Token* token);

    static void WINAPI ProcessOverlappedResult(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped);
};

//--

END_INFERNO_NAMESPACE_EX(windows)

#endif
