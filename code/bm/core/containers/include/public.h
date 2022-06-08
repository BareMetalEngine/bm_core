/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

//----

// Glue headers and logic
#include "bm_core_containers_glue.inl"

// All shared pointer stuff is always exposed
#include "uniquePtr.h"
#include "refCounted.h"
#include "refPtr.h"
#include "refWeakPtr.h"

// Common shit
#include "flags.h"
#include "crc.h"
#include "stringView.h"
#include "stringID.h"
#include "stringBuf.h"
#include "stringBuilder.h"
#include "stringParser.h"

// Inlined stuff
#include "stringID.inl"
#include "stringBuf.inl"
#include "stringView.inl"
#include "crc.inl"

#include "bm/core/system/include/timing.h"

//----

BEGIN_INFERNO_NAMESPACE()

class RectAllocator;

struct CompileTimeCRC32;
struct CompileTimeCRC64;
class IClipboardHandler;

class SimpleStreamWriter;
class SimpleStreamReader;

static const auto MAX_STREAM_BUFFER_LENGTH = 1U << 20;
static const auto MAX_STREAM_STRING_LENGTH = 1U << 16;
static const auto MAX_STREAM_ARRAY_SIZE = 1U << 16;

//----

template< typename T, typename... Args >
INLINE RefPtr<T> RefNew(Args&& ... args)
{
    static_assert(std::is_base_of<IReferencable, T>::value, "RefNew can only be used with IReferencables");
    return AddRef(new T(std::forward< Args >(args)...));
}

//----

// basic progress reporter
class BM_CORE_CONTAINERS_API IProgressTracker : public MainPoolData<NoCopy>
{
public:
    virtual ~IProgressTracker();

    /// check if we were canceled, if so we should exit any inner loops we are in
    virtual bool checkCancelation() const = 0;

    /// post status update, will replace the previous status update
    virtual void reportProgress(uint64_t currentCount, uint64_t totalCount, StringView text) = 0;

    /// post status update without any numerical information (just a sliding bar)
    INLINE void reportProgress(StringView text) { reportProgress(0, 0, text); }

    //----

    // get an empty progress tracker - will not print anything
    static IProgressTracker& DevNull();
};

//--

// fiber job helper for progress
class BM_CORE_CONTAINERS_API FiberProgressTracker : public IProgressTracker
{
public:
    FiberProgressTracker(IProgressTracker& progres, uint32_t total, const char* text);

    virtual bool checkCancelation() const override;
    virtual void reportProgress(uint64_t currentCount, uint64_t totalCount, StringView text) override;

    void advance(uint32_t count = 1);

private:
    IProgressTracker& m_progress;

    uint32_t m_total = 0;
    std::atomic<uint32_t> m_counter = 0;
    NativeTimePoint m_nextAllowedUpdate;
    const char* m_text = "";
};

//--

template< typename T >
INLINE static NoAddRefWrapper<T> NoAddRef(T* ptr)
{
    //static_assert(std::is_base_of<IReferencable, T>::value, "Type should be based on IReferencable");
    return NoAddRefWrapper(ptr);
}

template< typename T >
INLINE static NoAddRefWrapper<T> NoAddRef(const T* ptr)
{
    //static_assert(std::is_base_of<IReferencable, T>::value, "Type should be based on IReferencable");
    return NoAddRefWrapper((T*)ptr);
}

template< typename T >
INLINE static AddRefWrapper<T> AddRef(T* ptr)
{
    //static_assert(std::is_base_of<IReferencable, T>::value, "Type should be based on IReferencable");
    return AddRefWrapper<T>(ptr);
}

template< typename T >
INLINE static AddRefWrapper<T> AddRef(const T* ptr)
{
    //static_assert(std::is_base_of<IReferencable, T>::value, "Type should be based on IReferencable");
    return AddRefWrapper<T>((T*)ptr);
}

//--

static const int INVALID_ID = -1;

//--

END_INFERNO_NAMESPACE()



