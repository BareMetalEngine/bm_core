/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

#include "bm/core/containers/include/array.h"
#include "bm/core/memory/include/buffer.h"

BEGIN_INFERNO_NAMESPACE()

//--

static const uint32_t CRC32Init = 0x0;
static const uint64_t CRC64Init = 0xCBF29CE484222325;

//--

class StringID;
class StringBuf;

//--

enum class MatchResultStatus : uint8_t
{
    OK = 0,
    EmptyString = 1,
    InvalidCharacter = 2,
    Overflow = 3,
};

struct MatchResult
{
    MatchResultStatus status = MatchResultStatus::OK;

    INLINE MatchResult() = default;
    INLINE MatchResult(const MatchResult& other) = default;
    INLINE MatchResult& operator=(const MatchResult& other) = default;

    INLINE MatchResult(MatchResultStatus status_) : status(status_) {};

	INLINE operator bool() const { return status == MatchResultStatus::OK; }
};

//--

enum class StringCaseComparisonMode : uint8_t
{
    WithCase,
    NoCase,
};

enum class StringFindFallbackMode : uint8_t
{
	Empty,
	Full,
};

enum class StringSliceBit : uint8_t
{
    KeepEmpty,
    IgnoreQuotes,
    IgnoreTrim,
};

typedef BitFlags<StringSliceBit> StringSliceFlags;

//--

struct StringReplaceSetup;

//--

/// View of a string buffer, extends array view with stuff typical to string
class BM_CORE_CONTAINERS_API StringView : protected ArrayView<char>
{
public:
    INLINE StringView() = default;
    INLINE StringView(std::nullptr_t) {};
    INLINE StringView(const char* start, uint32_t length = std::numeric_limits<uint32_t>::max()); // computes the length automatically if not provided
    INLINE StringView(const char* start, const char* end);
    INLINE StringView(ConstArrayIterator<char> start, ConstArrayIterator<char> end);
    INLINE StringView(const StringView& other) = default;
    INLINE StringView(StringView&& other) = default;
    INLINE StringView(BufferView rawData);
    INLINE StringView(ArrayView<char> rawData);

    template< uint32_t N >
    INLINE StringView(const BaseTempString<N>& tempString);

    INLINE StringView& operator=(const StringView& other) = default;
    INLINE StringView& operator=(StringView&& other) = default;

    INLINE ~StringView() = default;

    //--------------------------

    // internal data (NOTE: will NOT be zero terminated!!!!)
    INLINE const char* data() const;

    // is the view representing empty string
    INLINE bool empty() const;

    // length of the string (same as array view's size()
    INLINE uint32_t length() const;

    // number of actual unicode character
    uint32_t unicodeLength() const;

    // reset the view to empty state 
    INLINE void reset();

    // print to printer
    INLINE void print(IFormatStream& p) const;

    //---
    
	// compare content of string view with other string view
	int compare(StringView other, StringCaseComparisonMode caseMode = StringCaseComparisonMode::WithCase) const;

	// compare content of string view with other string view but only N first characters
	int compareN(StringView other, uint32_t count, StringCaseComparisonMode caseMode = StringCaseComparisonMode::WithCase) const;

    //---

    // specific text comparison
    INLINE bool operator==(StringView other) const;
    INLINE bool operator!=(StringView other) const;
    INLINE bool operator<(StringView other) const;
    INLINE bool operator<=(StringView other) const;
    INLINE bool operator>(StringView other) const;
    INLINE bool operator>=(StringView other) const;

	// standard comparison operators
	friend INLINE static bool operator==(const char* first, StringView other);
	friend INLINE static bool operator!=(const char* first, StringView other);
	friend INLINE static bool operator<(const char* first, StringView other);
	friend INLINE static bool operator<=(const char* first, StringView other);
	friend INLINE static bool operator>(const char* first, StringView other);
	friend INLINE static bool operator>=(const char* first, StringView other);

    //--

    // quick emptiness test
    INLINE operator bool() const;

	// quick cast to buffer
	INLINE operator BufferView() const;

    //--

    // get N left characters, at most to the length of the string
    INLINE StringView leftPart(uint32_t count) const;

    // get N right characters, at most to the length of the string
    INLINE StringView rightPart(uint32_t count) const;

    // get N characters starting at given position, at most till the end of the string
    INLINE StringView subString(uint32_t first, uint32_t count = INDEX_MAX) const;

	// get inner string triming from left and right
    INLINE StringView innerString(uint32_t frontTrim, uint32_t backTrim) const;

    // split string into left and right part
    INLINE void split(uint32_t index, StringView& outLeft, StringView& outRight) const;

    // split string at the occurrence of the pattern into left and right part
    INLINE bool splitAt(StringView str, StringView& outLeft, StringView& outRight) const;

	/// slice string into parts using the chars from the set
	void slice(char splitChar, Array< StringView >& outTokens, StringSliceFlags flags = StringSliceFlags()) const;

    /// slice string into parts using the chars from the set
    void slice(const char* splitChars, Array< StringView >& outTokens, StringSliceFlags flags = StringSliceFlags()) const;

    //--

    /// trim whitespaces at the start of the string, may return empty string
    StringView trimLeft() const;

    /// trim whitespaces at the end of the string, may return empty string
    StringView trimRight() const;

    /// trim whitespaces at both ends
    StringView trim() const;

    /// trim tailing numbers (Mesh01 -> Mesh), may return an empty string
    StringView trimTailNumbers(uint32_t* outNumber=nullptr) const;

    /// remove quotes if present ("test" -> test, 'test' -> test)
    StringView trimQuotes() const;

    //--

    // find first substring occurrence
    INLINE Index findStr(StringView pattern, StringCaseComparisonMode caseMode = StringCaseComparisonMode::WithCase, int firstPosition = 0) const;

    // find last substring occurrence
    INLINE Index findRevStr(StringView pattern, StringCaseComparisonMode caseMode = StringCaseComparisonMode::WithCase, int firstPosition = std::numeric_limits<int>::max()) const;

    // find first character occurrence
    INLINE Index findFirstChar(char ch) const;

    // find last character 
    INLINE Index findLastChar(char ch) const;

    //--

    // check if string begins with other string
    INLINE bool beginsWith(StringView pattern, StringCaseComparisonMode caseMode = StringCaseComparisonMode::WithCase) const;

	// check if string end with other string
    INLINE bool endsWith(StringView pattern, StringCaseComparisonMode caseMode = StringCaseComparisonMode::WithCase) const;

    //--

    // find text after first occurrence of given pattern
    INLINE StringView afterFirst(StringView pattern, StringCaseComparisonMode caseMode = StringCaseComparisonMode::WithCase, StringFindFallbackMode fallback = StringFindFallbackMode::Empty) const;

	// find text before first occurrence of given pattern
	INLINE StringView beforeFirst(StringView pattern, StringCaseComparisonMode caseMode = StringCaseComparisonMode::WithCase, StringFindFallbackMode fallback = StringFindFallbackMode::Empty) const;

	// find text after first occurrence of given pattern
	INLINE StringView afterLast(StringView pattern, StringCaseComparisonMode caseMode = StringCaseComparisonMode::WithCase, StringFindFallbackMode fallback = StringFindFallbackMode::Empty) const;

	// find text before first occurrence of given pattern
	INLINE StringView beforeLast(StringView pattern, StringCaseComparisonMode caseMode = StringCaseComparisonMode::WithCase, StringFindFallbackMode fallback = StringFindFallbackMode::Empty) const;

    //--

    // returns filename with extension:    Z:\test\files\lena.png.bak -> "lena.png.bak"
    INLINE StringView pathFileName() const; 

    // returns filename stem with no extension:  Z:\test\files\lena.png.bak -> "lena"
    INLINE StringView pathFileStem() const;

    // returns file extensions WITH the dot:   Z:\test\files\lena.png.bak -> "png.bak"
    INLINE StringView pathFileExtensions() const;

    // returns file LAST extension WITH the dot:   Z:\test\files\lena.png.bak -> "bak"
    INLINE StringView pathFileLastExtension() const;

    // strip any extensions from path:  Z:\test\files\lena.png -> "Z:\test\files\lena"
	INLINE StringView pathStripExtensions() const;

    // return path to the parent directory:   Z:\test\files\lena.png.bak -> "Z:\test\files\", Z:\test\files\ -> "Z:\test\"
    INLINE StringView pathParent() const;

    // return name of the last directory in the path  Z:\test\files\ -> "files"
    INLINE StringView pathDirectoryName() const; 


    //--

    // match boolean from string ("true", "false" or from integer)
    MatchResult match(bool& outValue) const;

    // match 8-bit unsigned integer (values out of range are not converted)
    // NOTE: hex values starting from 0x are also decoded
    MatchResult match(uint8_t& outValue) const;

    // match 16-bit unsigned integer (values out of range are not converted)
    // NOTE: hex values starting from 0x are also decoded
    MatchResult match(uint16_t& outValue) const;

    // match 32-bit unsigned integer (values out of range are not converted)
    // NOTE: hex values starting from 0x are also decoded
    MatchResult match(uint32_t& outValue) const;

    // match 64-bit unsigned integer (values out of range are not converted)
    // NOTE: hex values starting from 0x are also decoded
    MatchResult match(uint64_t& outValue) const;

    // match 8-bit signed integer (values out of range are not converted)
    MatchResult match(char& outValue) const;

    // match 16-bit signed integer (values out of range are not converted)
    MatchResult match(short& outValue) const;

    // match 32-bit signed integer (values out of range are not converted)
    MatchResult match(int& outValue) const;

    // match 64-bit signed integer (values out of range are not converted)
    MatchResult match(int64_t& outValue) const;

    // match 32-bit floating point value (including scientific notation)
    MatchResult match(float& outValue) const;

    // match 64-bit floating point value (including scientific notation)
    MatchResult match(double& outValue) const;

    // match StringID value (trims + removes quotes)
    MatchResult match(StringID& outValue) const;

    // match StringBuf value (trims + removes quotes)
    MatchResult match(StringBuf& outValue) const;

    //--

    // match string to pattern
    bool matchString(StringView pattern, StringCaseComparisonMode caseMode = StringCaseComparisonMode::WithCase) const;

    // match a pattern or sub string
    bool matchPattern(StringView pattern, StringCaseComparisonMode caseMode = StringCaseComparisonMode::WithCase) const;

    //--

	/// slice string into parts using the chars from the set and parse the values, pushes only valid values
	/// NOTE: returns number of parsing errors
	template< typename T >
	INLINE uint32_t sliceAndMatch(char splitChar, Array< T >& outTokens) const;

	//--

    /// replace all entries of strings from replacement list, new string is printed to the format stream    
	void replaceText(IFormatStream& f, ArrayView<StringReplaceSetup> patterns) const;

    //--

    /// Encode string content in another transport format (URL, C-scaped, JSON-escaped, Hexadecimal, BASE64, etc)
    /// Prints the output directly to the formatter, does not create temporary representation
    void encode(EncodingType et, IFormatStream& f) const;

    /// Encode string content in another transport format (URL, C-scaped, JSON-escaped, Hexadecimal, BASE64, etc)
    /// NOTE: can fail under rare circumstances like OOM but in general never fails
    bool encode(EncodingType et, StringBuf& outString) const;

	/// Encode string content in another transport format (URL, C-scaped, JSON-escaped, Hexadecimal, BASE64, etc)
	/// NOTE: can fail if output buffer is to small
	bool encode(EncodingType et, BufferOutputStream<char>& outputs) const;

	/// Encode string content in another transport format (URL, C-scaped, JSON-escaped, Hexadecimal, BASE64, etc)
    /// NOTE: can fail under rare circumstances like OOM but in general never fails
	bool encode(EncodingType et, Buffer& outBuffer, IPoolUnmanaged& pool = MainPool()) const;

    /// Encode string content in another transport format (URL, C-scaped, JSON-escaped, Hexadecimal, BASE64, etc)
    StringBuf encode(EncodingType et) const;

    //---

	/// decode this transport string into data buffer
	bool decode(IPoolUnmanaged& pool, Buffer& outBuffer, EncodingType et, bool allowWhiteSpaces = true, uint32_t alignment = 16) const;

    /// decode this transport string into data buffer
    Buffer decode(IPoolUnmanaged& pool, EncodingType et, bool allowWhiteSpaces = true, uint32_t alignment = 16) const;

    /// decode this transport string into text (URLs, JSON, etc)
    /// NOTE: returns false on decoding failure 
    bool decode(EncodingType et, StringBuf& outString, bool allowWhiteSpaces = true) const;

    /// decode this transport string into text (URLs, JSON, etc)
    /// NOTE: returns empty string on decoding failure (but the input string might have been empty already so there's no clean way to detect errors)
    StringBuf decode(EncodingType et, bool allowWhiteSpaces = true) const;

    //--

    // compute hash of the string
    static uint32_t CalcHash(StringView txt);

    //--

    // evaluate 32-bit CRC
    uint32_t evaluateCRC32(uint32_t crc = CRC32Init) const;

    // compute 64-bit crr
    uint64_t evaluateCRC64(uint64_t crc = CRC64Init) const;

	//--

	// create a memory buffer, NOTE: no null termination
	BufferView bufferView() const;

	// create a memory buffer, NOTE: no null termination
	ArrayView<char> arrayView() const;

    //--

    //! Get read only iterator to start of the array
    ConstArrayIterator<char> begin() const;

    //! Get read only iterator to end of the array
    ConstArrayIterator<char> end() const;

    //--
    
    // decode and iterate UTF32 characters
    // NOTE: not used a lot so it's optimized more for convenience
    void iterateUTF32(const std::function<void(uint32_t)>& func) const;

    // convert into UTF-16
	Array<wchar_t> exportUTF16() const;

	// convert into UTF-32
	Array<uint32_t> exportUTF32() const;

    //--

    // upper case UTF-32 char for given lower-case one
    static uint32_t MapUpperToLowerCaseUTF32(uint32_t ch);

	// upper case UTF-32 char for given lower-case one
	static uint32_t MapLowerToUpperCaseUTF32(uint32_t ch);

    //--

    // global "empty" string view, can be returned when returning something via const reference is needed by there's no data
    static StringView& EMPTY();

    //--

#ifdef WITH_GTEST
	friend std::ostream& operator<<(std::ostream& os, const StringView& txt) {
		return os << "\"" << std::string_view(txt.data(), txt.length()) << "\"";
	}
#endif

    //--

	/// Convert string content to standalone buffer
    Buffer toBuffer(IPoolUnmanaged& pool = MainPool()) const;

	/// Convert string content to standalone buffer WITH EXTRA ZERO BYTE at the end
	Buffer toBufferZeroTerminated(IPoolUnmanaged& pool = MainPool()) const;

    /// Encode string content into a buffer
    Buffer toBufferEncoded(EncodingType encoding, IPoolUnmanaged& pool = MainPool()) const;

    //---
};

//--

struct StringReplaceSetup
{
	StringView name;
	StringView data;
	StringCaseComparisonMode caseMode = StringCaseComparisonMode::WithCase;
};

//--

END_INFERNO_NAMESPACE()
