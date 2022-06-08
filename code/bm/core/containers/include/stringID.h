/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

#include "stringBuf.h"

BEGIN_INFERNO_NAMESPACE()

typedef uint32_t StringIDIndex;

namespace prv
{
	class StringIDDataStorage;
	class StringIDMap;
}

/// String class
class BM_CORE_CONTAINERS_API StringID
{
public:
    INLINE StringID();
    INLINE StringID(const StringID& other);
    INLINE StringID(StringView other);
    INLINE ~StringID() = default;

    INLINE explicit StringID(const char* other);

    template< uint32_t N >
    INLINE explicit StringID(const BaseTempString<N>& other);

    INLINE bool operator==(StringID other) const;
    INLINE bool operator!=(StringID other) const;

    INLINE bool operator==(const char* other) const;
    INLINE bool operator==(const StringBuf& other) const;
    INLINE bool operator!=(const char* other) const;
    INLINE bool operator!=(const StringBuf& other) const;

    INLINE bool operator<(StringID other) const; // compares STRINGS, not numerical IDs as this is more stable

    INLINE StringID& operator=(StringID other);

    //---

    //! true if the ID is empty (== EMPTY()
    INLINE bool empty() const;

    //! get the ordering value for this name, can be used if we want to map stuff directly
    INLINE uint32_t index() const;

    //! get the printable C string
    INLINE const char* c_str() const;

    //! get the view of the string
    INLINE StringView view() const;

    //---

    //! check if not empty
    INLINE operator bool() const;

    //---

    //! get empty name
    static StringID EMPTY();

    //! find name without allocating string
    static StringID Find(StringView txt);

    //---

    INLINE static uint32_t CalcHash(StringID id);
    INLINE static uint32_t CalcHash(StringView txt);
    INLINE static uint32_t CalcHash(const char* txt);

    //--

#ifdef WITH_GTEST
	friend std::ostream& operator<<(std::ostream& os, const StringID& txt) {
        return os << "'" << txt.c_str() << "' #" << txt.index();
	}
#endif

private:
    StringIDIndex indexValue;
    const char* debugTxt = 0;

	//--

	static StringIDIndex Alloc(StringView txt);

	//--

    static const uint32_t STRING_TABLE_PAGE_SIZE = 1U << 20;
	static const char* st_StringTable[1024]; // global string table

	friend class prv::StringIDDataStorage;
	friend class prv::StringIDMap;

	//--

	explicit ALWAYS_INLINE StringID(StringIDIndex index)
		: indexValue(index)
	{
        debugTxt = c_str();
    }

	//--

};

END_INFERNO_NAMESPACE()

INLINE const bm::StringID operator"" _id(const char* str, size_t len)
{
    return bm::StringID(str);
}