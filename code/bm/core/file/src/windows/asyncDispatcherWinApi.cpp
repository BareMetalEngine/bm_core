/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "private.h"
#include "asyncDispatcherWinApi.h"

#include "bm/core/containers/include/utf8StringFunctions.h"
#include "bm/core/containers/include/path.h"
#include "bm/core/containers/include/inplaceArray.h"
#include "fileAbsoluteRange.h"

#ifdef PLATFORM_WINAPI

BEGIN_INFERNO_NAMESPACE_EX(windows)

//--

AsyncReadDispatcher::AsyncReadDispatcher()
    : m_tokensCounter(0, 1U << 30)
    , m_tokenCounter(1)
    , m_exiting(0)
{
    // create the io completion thread
    ThreadSetup setup;
    setup.m_function = [this]() { threadFunc(); };
    setup.m_priority = ThreadPriority::AboveNormal;
    m_ioCompletionThread.init(setup);
}

AsyncReadDispatcher::~AsyncReadDispatcher()
{
    TRACE_INFO("Closing async IO dispatcher");

    m_exiting = 1;
    m_tokensCounter.release(1000);
    m_ioCompletionThread.close();

    TRACE_INFO("Closed async IO dispatcher, {} total requests serviced ({} failed), {} bytes read", 
        m_numRequests, m_numRequestFailed, MemSize(m_totalDataRead));
}

AsyncReadDispatcher::Token* AsyncReadDispatcher::allocToken()
{
	auto lock  = CreateLock(m_tokenPoolLock);
	return m_tokenPool.create();
}

void AsyncReadDispatcher::releaseToken(Token* token)
{
	auto lock  = CreateLock(m_tokenPoolLock);
	m_tokenPool.free(token);
}

void AsyncReadDispatcher::scheduleAsync(HANDLE hAsyncFile, FileAbsoluteRange range, void* outMemory, TAsyncReadCallback callback)
{
    ASSERT_EX(hAsyncFile != INVALID_HANDLE_VALUE, "Invalid file handle");
    DEBUG_CHECK_RETURN_EX(!m_exiting, "Sending IO requestes during exit");

    // nothing to read
    if (!range)
    {
        callback(0);
        return;
    }

    // create request, if this fails it means we are already full, do idle spin until we are free again
    auto token = allocToken();
    memzero(token, sizeof(Token));

    // setup 
    token->m_hAsyncHandle = hAsyncFile;
    token->m_memory = outMemory;
    token->m_callback = std::move(callback);
    token->m_size = (uint32_t)range.size(); // TODO: async reads of more than 4GB!
    token->m_overlapped.Offset = (uint32_t)(range.absoluteStart() >> 0);
    token->m_overlapped.OffsetHigh = (uint32_t)(range.absoluteStart() >> 32);
    token->m_dispatcher = this;
    token->m_index = m_tokenCounter++;
    
    //TRACE_INFO("[ASYNC READ] #{} at {} ({}) scheduled for {}", token->m_index, range.absoluteStart(), range.size(), Hex(hAsyncFile));

    // send to the thread
    {
        auto lock  = CreateLock(m_tokensToExecuteLock);
        m_tokensToExecute.push(token);
    }

    // wake up thread
    m_tokensCounter.release(1);
}

void WINAPI AsyncReadDispatcher::ProcessOverlappedResult(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped)
{
    auto token  = (Token*)lpOverlapped;

    //TRACE_INFO("[ASYNC READ] #{} finished with {}/{} and code {}", token->m_index, dwNumberOfBytesTransfered, token->m_size, dwErrorCode);

    // IO ERROR ?
    if (dwErrorCode != 0)
    {
        TRACE_ERROR("AsyncRead failed with %08X", dwErrorCode);
        token->m_dispatcher->m_numRequestFailed += 1;
    }
    else
    {
        if (dwNumberOfBytesTransfered < token->m_size)
        {
            TRACE_WARNING("AsyncRead read {} instead of {}", dwNumberOfBytesTransfered, token->m_size);
        }

        token->m_dispatcher->m_totalDataRead += dwNumberOfBytesTransfered;
    }

    // get the callback to call
    auto callback = std::move(token->m_callback);

    // release token to pool
    auto dispatcher = token->m_dispatcher;
    dispatcher->releaseToken(token);

    // call the callback now, once the token has been returned
    if (dwErrorCode == 0)
        callback(dwNumberOfBytesTransfered);
    else
        callback(-1);
}

AsyncReadDispatcher::Token* AsyncReadDispatcher::popTokenFromQueue()
{
    auto lock  = CreateLock(m_tokensToExecuteLock);
    if (m_tokensToExecute.empty())
        return nullptr;

    AsyncReadDispatcher::Token* ret = nullptr;
    m_tokensToExecute.popIfNotEmpty(ret);
    return ret;
}

void AsyncReadDispatcher::threadFunc()
{
    while (!m_exiting)
    {
        m_tokensCounter.wait(10, true);

        if (auto token = popTokenFromQueue())
        {
            //TRACE_INFO("[ASYNC READ] #{} popped", token->m_index);
            processToken(token);
        }
    }
}

void AsyncReadDispatcher::processToken(Token* token)
{
	ASSERT(token != nullptr);

	// count stats
	m_numRequests += 1;

	// this function must be called from this thread because only here we are waiting in the alert alertable
	if (!ReadFileEx(token->m_hAsyncHandle, token->m_memory, token->m_size, &token->m_overlapped, &ProcessOverlappedResult))
	{
		auto errorCode = GetLastError();
		TRACE_ERROR("AsyncRead failed with {}", errorCode);
		m_numRequestFailed += 1;

		// inform user that read failed
		token->m_callback(-1);

		// release token here since the callback will not be called
		m_tokenPool.free(token);
	}
}

//--

END_INFERNO_NAMESPACE_EX(windows)

#endif
