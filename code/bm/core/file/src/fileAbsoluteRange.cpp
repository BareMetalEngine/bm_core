/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "fileAbsoluteRange.h"

BEGIN_INFERNO_NAMESPACE()

//--

FileAbsoluteRange FileAbsoluteRange::rangeAtStart(uint64_t size_) const
{
	DEBUG_CHECK_RETURN_EX_V(size_ < size(), TempString("Requested left file range for {} bytes from range {} that has size {}", size_, *this, size()), nullptr);
	return FileAbsoluteRange(m_absoluteStart, m_absoluteStart + size_);
}

FileAbsoluteRange FileAbsoluteRange::rangeAtEnd(uint64_t size_) const
{
	DEBUG_CHECK_RETURN_EX_V(size_ < size(), TempString("Requested right file range for {} bytes from range {} that has size {}", size_, *this, size()), nullptr);
	return FileAbsoluteRange(m_absoluteEnd - size_, m_absoluteEnd);
}

FileAbsoluteRange FileAbsoluteRange::rangeFromRelativeOffsetSize(uint64_t relativeOffset, uint64_t relativeSize) const
{
	DEBUG_CHECK_RETURN_EX_V(validateRelativeOffsetSize(relativeOffset, relativeSize), TempString("Requested relative sube range of {} bytes at {} from range {} that has size {}",
		relativeSize, relativeOffset, *this, size()), nullptr);

	return FileAbsoluteRange(m_absoluteStart + relativeOffset,
		m_absoluteStart + relativeOffset + relativeSize);
}

//--

bool FileAbsoluteRange::validateRelativeOffsetSize(uint64_t relativeOffset, uint64_t relativeSize) const
{
	// NOTE: special care to handle overflows
	const auto allowedSize = size();
	return (relativeOffset <= allowedSize) && (relativeSize <= allowedSize) && (relativeOffset + relativeSize <= allowedSize);
}

//--

bool FileAbsoluteRange::contains(FileAbsoluteRange range) const
{
	if (empty() || range.empty())
		return false; // empty ranges are not contained anywhere nor do the contain anything - by definition

	return (range.absoluteStart() >= absoluteStart()) && (range.absoluteEnd() <= absoluteEnd());
}

bool FileAbsoluteRange::overlaps(FileAbsoluteRange range) const
{
	if (empty() || range.empty())
		return false; // empty ranges are not contained anywhere nor do the contain anything - by definition

	if (range.absoluteEnd() <= absoluteStart())
		return false;
	if (range.absoluteStart() >= absoluteEnd())
		return false;

	return true;
}

uint64_t FileAbsoluteRange::calculateReadSize(FileAbsoluteRange readRange) const
{
	const auto leftInBufer = (readRange.absoluteStart() <= absoluteEnd()) ? (absoluteEnd() - readRange.absoluteStart()) : 0;
	return leftInBufer;
}

//--

void FileAbsoluteRange::print(IFormatStream& f) const
{
	if (empty())
		f << "[empty]";
	else
		f.appendf("[{} - {}]", m_absoluteStart, m_absoluteEnd);
}

//--

END_INFERNO_NAMESPACE()
