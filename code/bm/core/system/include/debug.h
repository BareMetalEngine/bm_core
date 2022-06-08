/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

#ifdef PLATFORM_WINDOWS
extern "C" {
    __declspec(dllimport) void __stdcall DebugBreak(void);
};
#endif

BEGIN_INFERNO_NAMESPACE()

//---

class BM_CORE_SYSTEM_API DebugSymbolName
{
public:
    static const uint32_t MAX_SIZE = 512;

    DebugSymbolName();

    INLINE const char* c_str() const { return m_txt; }
    INLINE bool unknown() const { return m_unknown; }

    void set(const char* txt);
    void append(const char* txt);

private:
    char m_txt[MAX_SIZE + 1];
    bool m_unknown = true;
};

class BM_CORE_SYSTEM_API DebugFileName
{
public:
	static const uint32_t MAX_SIZE = 512;

    DebugFileName();

	INLINE const char* c_str() const { return m_txt; }
    INLINE uint32_t line() const { return m_line; }
	INLINE bool unknown() const { return m_unknown; }

    void set(const char* txt, uint32_t line);
	void append(const char* txt);

private:
	char m_txt[MAX_SIZE + 1];
    uint32_t m_line = 0;
	bool m_unknown = true;
};

//---

class BM_CORE_SYSTEM_API DebugCallstack
{
public:
    static const uint32_t MAX_FRAMES = 64;

    DebugCallstack();
            
    void reset();
    void push(uint64_t address);

    INLINE bool empty() const
    {
        return (m_size == 0);
    }

    INLINE uint32_t size() const
    {
        return m_size;
    }

    INLINE const uint64_t operator[](const uint32_t index) const
    {
        if (index < MAX_FRAMES)
            return m_frames[index];
        return 0;
    }

	void print(IFormatStream& f, const char* lineSearator = "\n") const;

    bool printFrame(uint32_t frameIndex, IFormatStream& f) const;

    uint64_t uniqueHash() const;

private:
    uint32_t m_size;
    uint64_t m_frames[MAX_FRAMES];
};

//---

// Debug break function
#ifdef PLATFORM_WINDOWS
    #define INFERNO_DEBUG_BREAK() ::DebugBreak()
#elif defined(PLATFORM_POSIX)
    #define INFERNO_DEBUG_BREAK() raise(SIGTRAP);
#elif defined(PLATFORM_PSX)
    #define INFERNO_DEBUG_BREAK() _SCE_BREAK()
#else
    #define INFERNO_DEBUG_BREAK() __nop()
#endif


// Grab current callstack, returns number of function grabbed (0 if error)
extern BM_CORE_SYSTEM_API bool GrabCallstack(uint32_t skipInitialFunctions, const void* exptr, DebugCallstack& outCallstack);

// Get human readable function name
extern BM_CORE_SYSTEM_API bool TranslateSymbolName(uint64_t functionAddress, DebugSymbolName& outSymbolName, DebugFileName& outFileName);

// Check if debugger is present (windows mostly)
extern BM_CORE_SYSTEM_API bool DebuggerPresent();

//--

// Get indexed callstack (as an easy to save number)
extern BM_CORE_SYSTEM_API uint32_t CaptureCallstack(uint32_t skipInitialFunctions);

// Resolve callstack by ID, can be used to print it
extern BM_CORE_SYSTEM_API const DebugCallstack& ResolveCapturedCallstack(uint32_t id);

//--

END_INFERNO_NAMESPACE()
