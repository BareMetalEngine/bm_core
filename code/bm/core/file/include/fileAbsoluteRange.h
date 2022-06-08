/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

BEGIN_INFERNO_NAMESPACE()

//----

// absolute range in file specified by start and end absolute positions
struct BM_CORE_FILE_API FileAbsoluteRange
{
public:
	INLINE FileAbsoluteRange() = default;
	INLINE FileAbsoluteRange(std::nullptr_t) {};
	INLINE FileAbsoluteRange(const FileAbsoluteRange& other) = default;
	INLINE FileAbsoluteRange(FileAbsoluteRange&& other) = default;
	INLINE FileAbsoluteRange(uint64_t absoluteStart, uint64_t absoluteEnd) : m_absoluteStart(absoluteStart), m_absoluteEnd(absoluteEnd) {};
	INLINE FileAbsoluteRange& operator=(const FileAbsoluteRange& other) = default;
	INLINE FileAbsoluteRange& operator=(FileAbsoluteRange && other) = default;
	INLINE ~FileAbsoluteRange() {};

	INLINE bool empty() const { return m_absoluteEnd <= m_absoluteStart; }
	INLINE operator bool() const { return m_absoluteEnd > m_absoluteStart; }

	INLINE uint64_t absoluteStart() const { return m_absoluteStart; }
	INLINE uint64_t absoluteEnd() const { return m_absoluteEnd; }

	INLINE uint64_t size() const { return m_absoluteEnd - m_absoluteStart; }

	//--

	// get left part of the file range (asserts on errors)
	FileAbsoluteRange rangeAtStart(uint64_t size) const;

	// get right part of the file range (asserts on errors)
	FileAbsoluteRange rangeAtEnd(uint64_t size) const;

	// get sub range of this range using relative offset and size (asserts on errors)
	FileAbsoluteRange rangeFromRelativeOffsetSize(uint64_t relativeOffset, uint64_t relativeSize) const;

	//--

	// validate relative offset/size if it is fully contained within this file view
	bool validateRelativeOffsetSize(uint64_t relativeOffset, uint64_t relativeSize) const;

	// check if given absolute file range is fully contained within this file range
	bool contains(FileAbsoluteRange range) const;

	// check if given absolute file range overlaps this file range
	bool overlaps(FileAbsoluteRange range) const;

	//--

	// calculate how much bytes are actually readable for a placed read, returns value from 0 to readRange.size() based on safe overlap
	uint64_t calculateReadSize(FileAbsoluteRange readRange) const;

	//--

	// check for strict equality
	INLINE bool operator==(FileAbsoluteRange other) const { return (m_absoluteStart == other.m_absoluteStart) && (m_absoluteEnd == other.m_absoluteEnd); }
	INLINE bool operator!=(FileAbsoluteRange other) const { return !operator==(other); }

	// ordering by starting offset
	INLINE bool operator<(FileAbsoluteRange other) const { return m_absoluteStart < other.m_absoluteStart; }
	
	//--

	// debug print ([start - end])
	void print(IFormatStream& f) const;

	//--

private:
	uint64_t m_absoluteStart = 0;
	uint64_t m_absoluteEnd = 0;
};

//----

END_INFERNO_NAMESPACE()
