/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

#include "array.h"

BEGIN_INFERNO_NAMESPACE()

//--

namespace prv
{

    // holder for string data (all strings are interned)
    // for "STL" style strings, use StringVector
    class BM_CORE_CONTAINERS_API StringDataHolder : public MainPoolData<NoCopy>
    {
    public:
        // add internal reference
        INLINE void addRef() { ++m_refs; }

        /// release a reference
        INLINE void release() { if (0 == --m_refs) ReleaseToPool(this, m_length); }

        // get the zero-terminated C style string representation of the data stored in the storage buffer
        INLINE const char* c_str() const { return m_txt; }

        // get length of the current data
        INLINE uint32_t length() const { return m_length; }

		// number of actual unicode chars (<=length()
		INLINE uint32_t unicodeLength() const { return m_unicodeLength; }

        //--

        // create from ansi string
        static StringDataHolder* CreateAnsi(const char* txt, uint32_t length = INDEX_MAX);

        // create from string
        static StringDataHolder* CreateUnicode(const wchar_t* txt, uint32_t length = INDEX_MAX);

        // create empty
        static StringDataHolder* CreateEmpty(uint32_t length); // empty buffer with preallocated length

        //--

        // make a un-shared copy
        StringDataHolder* copy() const;

    private:
        std::atomic<uint32_t> m_refs;
        uint32_t m_length = 0;
        uint32_t m_unicodeLength = 0;
        char m_txt[1];

        static void ReleaseToPool(void* mem, uint32_t length);
    };

} // prv

//--

/// general string buffer
class BM_CORE_CONTAINERS_API StringBuf
{
public:
    INLINE StringBuf()
        : m_data(nullptr)
    {};

    INLINE StringBuf(StringBuf&& other)
        : m_data(other.m_data)
    {
        other.m_data = nullptr;
    }

    // TODO: make explicit!
    INLINE StringBuf(const char* str, uint32_t length = INDEX_MAX)
    {
        m_data = prv::StringDataHolder::CreateAnsi(str, length);
    }

    INLINE explicit StringBuf(const wchar_t* str, uint32_t length = INDEX_MAX)
    {
        m_data = prv::StringDataHolder::CreateUnicode(str, length);
    }

    INLINE explicit StringBuf(StringView view)
    {
        m_data = prv::StringDataHolder::CreateAnsi(view.data(), view.length());
    }

	INLINE explicit StringBuf(ArrayView<char> view)
	{
		m_data = prv::StringDataHolder::CreateAnsi(view.typedData(), view.size());
	}

	INLINE explicit StringBuf(ArrayView<wchar_t> view)
	{
		m_data = prv::StringDataHolder::CreateUnicode(view.typedData(), view.size());
	}

    INLINE explicit StringBuf(uint32_t length)
    {
        m_data = prv::StringDataHolder::CreateEmpty(length);
    }

    INLINE ~StringBuf()
    {
        if (m_data)
            m_data->release();
    }

    template< uint32_t N >
    INLINE StringBuf(const BaseTempString<N>& str)
    {
        m_data = prv::StringDataHolder::CreateAnsi(str.c_str(), str.length());
    }

    template< uint32_t N >
    INLINE StringBuf& operator=(const BaseTempString<N>& str)
    {
        *this = StringBuf(str.c_str());
        return *this;
    }

    INLINE StringBuf(const StringBuf& other)
        : m_data(other.m_data)
    {
        if (m_data)
            m_data->addRef();
    }

	StringBuf(BufferView buffer); // automatic detection of Ansi vs UTF-16

    INLINE StringBuf& operator=(const StringBuf& other);
    INLINE StringBuf& operator=(StringBuf&& other);

    //---

    // compare to other string
    INLINE int compare(StringView other, StringCaseComparisonMode caseMode = StringCaseComparisonMode::WithCase) const;

    // standard comparison operators
	INLINE bool operator==(StringView other) const;
	INLINE bool operator!=(StringView other) const;
	INLINE bool operator<(StringView other) const;
	INLINE bool operator<=(StringView other) const;
	INLINE bool operator>(StringView other) const;
	INLINE bool operator>=(StringView other) const;

	// standard comparison operators
	/*INLINE bool operator==(const StringBuf& other) const;
	INLINE bool operator!=(const StringBuf& other) const;
	INLINE bool operator<(const StringBuf& other) const;
	INLINE bool operator<=(const StringBuf& other) const;
	INLINE bool operator>(const StringBuf& other) const;
	INLINE bool operator>=(const StringBuf& other) const;*/

	// standard comparison operators
	friend INLINE static bool operator==(const char* first, const StringBuf& other);
    friend INLINE static bool operator!=(const char* first, const StringBuf& other);
    friend INLINE static bool operator<(const char* first, const StringBuf& other);
    friend INLINE static bool operator<=(const char* first, const StringBuf& other);
    friend INLINE static bool operator>(const char* first, const StringBuf& other);
    friend INLINE static bool operator>=(const char* first, const StringBuf& other);

    //--

    // transcode string in UTF-32 space
    StringBuf transcode(std::function<uint32_t(uint32_t)> func) const;

	// sanitize file name string - remove all characters that are not valid for a file name
	// NOTE: unicode aware - string is decoded!
	StringBuf sanitizeFileName(bool keepWhitespaces = true) const;

    // replace given character, allocates new string if needed on first replacement
    // NOTE: unicode aware - string is decoded!
    StringBuf replaceChar(uint32_t oldCh, uint32_t newCh) const;

	// remove given character, allocates new string if needed on first replacement
	// NOTE: unicode aware - string is decoded!
	StringBuf removeChar(uint32_t ch) const;

    // convert to lower case string, allocates string only if any change must happen
    // NOTE: unicode aware - string is decoded!
	StringBuf toLower() const;

    // convert to lower case string, allocates string only if any change must happen
    // NOTE: unicode aware - string is decoded!
	StringBuf toUpper() const;

    //---

    // check if string begins with other string
    INLINE bool beginsWith(StringView pattern, StringCaseComparisonMode caseMode = StringCaseComparisonMode::WithCase) const;

    // check if string begins with other string
    INLINE bool endsWith(StringView pattern, StringCaseComparisonMode caseMode = StringCaseComparisonMode::WithCase) const;

    //---

    // reset string content
    INLINE void clear();

    // check if string is empty
    INLINE bool empty() const;

    // quick emptiness check
	INLINE explicit operator bool() const;

    // length of the string
    INLINE uint32_t length() const;

    // number of actual unicode character (unicodeLength() <= length())
    INLINE uint32_t unicodeLength() const;

    // C-style zero terminated string buffer
    INLINE const char* c_str() const;

	// C-style string cast
	// INLINE operator const char*() const;

    // view of the content of the string
    INLINE StringView view() const;

    // cast to view of the content of the string
    INLINE operator StringView() const;

    //---

    // evaluate 32-bit standalone CRC
    INLINE uint32_t evaluateCRC32(uint32_t crc = CRC32Init) const;

    // evaluate 64-bit standalone CRC
    INLINE uint64_t evaluateCRC64(uint64_t crc = CRC64Init) const;

    //---

    // hashing for hashmaps
    INLINE static uint32_t CalcHash(const StringBuf& txt);

    // hashing for hashmaps - the StringView and StringBuf have same hashing so they can be replaced as keys
    INLINE static uint32_t CalcHash(StringView txt);

    // hashing for hashmaps - the char* and StringBuf have same hashing so they can be replaced as keys
    INLINE static uint32_t CalcHash(const char* txt);

    //---

    // the empty string
    static const StringBuf& EMPTY();

    //--

    // a memory pool to be used for strings
    static IPoolUnmanaged& StringPool();

    //--

#ifdef WITH_GTEST
	friend std::ostream& operator<<(std::ostream& os, const StringBuf& txt) {
		return os << "\"" << txt.c_str() << "\"";
	}
#endif

private:
    prv::StringDataHolder* m_data = nullptr;
};

//--

END_INFERNO_NAMESPACE()
