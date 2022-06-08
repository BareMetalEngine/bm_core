/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

#include "bufferOutputStream.h"
#include "bufferInputStream.h"


BEGIN_INFERNO_NAMESPACE()

//--

class BufferSegmentedView;

//--

/// Most fundamental concept of a range of memory addresses composed of [start,end> pointer range
/// NOTE: the pointer range is intended to be passed by VALUE as on good compiler it is passed in 2 registers directly (GCC/CLANG, bad on MSVC but fuck MCVC)
class BM_CORE_MEMORY_API BufferView
{
public:
	ALWAYS_INLINE BufferView() = default;
	ALWAYS_INLINE BufferView(std::nullptr_t) {};
	ALWAYS_INLINE BufferView(const BufferView& other) = default;
	ALWAYS_INLINE BufferView& operator=(const BufferView& other) = default;
	ALWAYS_INLINE BufferView(BufferView&& other);
	ALWAYS_INLINE BufferView& operator=(BufferView&& other);
	ALWAYS_INLINE BufferView(void* data, uint64_t length);
	ALWAYS_INLINE BufferView(void* data, void* dataEnd);
	ALWAYS_INLINE BufferView(const void* data, uint64_t length);
	ALWAYS_INLINE BufferView(const void* data, const void* dataEnd);
	ALWAYS_INLINE ~BufferView() = default;

	template< typename T, uint32_t N>
	ALWAYS_INLINE BufferView(const T data[N])
	{
		m_start = &data[0];
		m_end = &data[N];
	}

	//--

	//! get internal data buffer
	ALWAYS_INLINE uint8_t* data();

	//! get internal data buffer (read-only)
	ALWAYS_INLINE const uint8_t* data() const;

	//! returns number of bytes in the pointer range
	ALWAYS_INLINE uint64_t size() const;

	//! count number of segments of given size
	ALWAYS_INLINE uint32_t segmentCount(uint32_t size) const;

	//! returns true if the array is empty
	ALWAYS_INLINE bool empty() const;

	//! boolean check, false if buffer view is empty
	ALWAYS_INLINE operator bool() const;

	//--

	//! reset the range to empty range
	ALWAYS_INLINE void reset();

	//! check if this memory range contains other memory range
	ALWAYS_INLINE bool containsRange(BufferView other) const;

	//! check if this memory range contains given pointer
	ALWAYS_INLINE bool containsPointer(const void* ptr) const;

	//! validate offset+size range in this view, returns false if range is not fully enclosed
	ALWAYS_INLINE bool validateRange(uint64_t offset, uint64_t size) const;

	//! get number of bytes possible to copy/read from this view at given offset and maxmimum size
	ALWAYS_INLINE uint64_t evaluateCopiableSize(uint64_t offset, uint64_t size) const;

	//--

	//! zero the memory range
	void zeroMemory();

	//! clear memory range to given value, slower than clearToZero
	void fillMemory(uint8_t value);

	//! compare byte-to-byte raw content of memory range with other memory range
	int compareMemory(BufferView other) const;

	//! byte-reverse the content
	void reverseMemory();

	//--

	//! copy memory from one buffer to another, returns number of bytes copied
	static uint64_t Copy(BufferView src, uint64_t srcOffset, BufferView dest, uint64_t destOffset, uint64_t size = std::numeric_limits<uint64_t>::max());

	//--

	//! Get iterator to start of the array
	ALWAYS_INLINE uint8_t* begin();

	//! Get iterator to end of the array
	ALWAYS_INLINE uint8_t* end();

	//! Get read only iterator to start of the array
	ALWAYS_INLINE const uint8_t* begin() const;

	//! Get read only iterator to end of the array
	ALWAYS_INLINE const uint8_t* end() const;

	//--

	//! Create left aligned view
	ALWAYS_INLINE BufferView leftView(uint64_t size) const;

	//! Create right aligned view
	ALWAYS_INLINE BufferView rightView(uint64_t size) const;

	//! Create sub-view of any portion of the buffer
	//! NOTE: selected portion must be completely enclosed by the current view
	ALWAYS_INLINE BufferView subView(uint64_t offset, uint64_t size) const;

	//! Create sub-view of any portion of the buffer, creates a view of data only available in this view 
	//! NOTE: can return empty view if offset/size lies beyond current view
	ALWAYS_INLINE BufferView relaxedSubView(uint64_t offset, uint64_t size) const;

	//! Create segmented view of this buffer, provides chunks of constant size for futher processing
	ALWAYS_INLINE BufferSegmentedView segmentedView(uint32_t chunkSize) const;

	//! Get view of the buffer at n-th segment
	ALWAYS_INLINE BufferView segmentView(uint32_t chunkSize, uint32_t index) const;

	//--

	//! Cut N bytes from the left of the memory region, returns false if cut cannot be done (not enough memory)
	//! Updates this region to start after the cut region
	bool cutLeft(uint64_t size, BufferView& outRegion);

	//! Cut N bytes from the right of the memory region, returns false if cut cannot be done (not enough memory)
	//! Updates this region to end before the cut region
	bool cutRight(uint64_t size, BufferView& outRegion);

	//! Cut N bytes from the left of the memory region but make sure the resulting start of region is aligned, returns false if cut cannot be done (not enough memory)
	//! Updates this region to start after the cut region
	bool cutLeftWithAlignment(uint64_t size, uint32_t alignment, BufferView& outRegion);

	//! Cut N bytes from the right of the memory region but make sure the resulting start of region is aligned, returns false if cut cannot be done (not enough memory)
	//! Updates this region to end before the cut region
	bool cutRightWithAlignment(uint64_t size, uint32_t alignment, BufferView& outRegion);
	
	//--

	//! compute CRC32 value
	//uint32_t crc32() const;

	//! compute CRC64 value
	//uint64_t crc64() const;

	//--

	// estimate upper bound required for compression of this buffer
	uint64_t esitimateCompressedSize(CompressionType ct) const;

	//! measure size needed for encoded (base64/hex, etc) representation
	uint64_t estimateEncodedSize(EncodingType et) const;

	//! measure size needed for decoded representation
	uint64_t estimateDecodedSize(EncodingType et) const;

	//--

	//! encode buffer directly into text representation and output it to printer
	//! NOTE: this is rather slow as it outputs text via the f.append()
	void encode(EncodingType et, IFormatStream& f) const;

	//! encode into other representation, the output buffer must be big enough, actually used size is returned
	bool encode(EncodingType et, BufferOutputStream<char>& output) const;

	//! decode from custom encoding, white spaces can be allowed, returns actual size decoded
	//! NOTE: this function fails on any unexpected character
	bool decode(EncodingType et, BufferOutputStream<uint8_t>& output, bool allowWhiteSpaces = true) const;

	//--

	// Compress data into provided preallocated memory (must be at least size esitimateCompressedSize)
	// NOTE: the compressed data view is updated to point to actual memory
	bool compress(CompressionType ct, BufferOutputStream<uint8_t>& output) const;

	// Decompress data into provided preallocated memory
	bool decompress(CompressionType ct, BufferOutputStream<uint8_t>& output) const;

	//--

protected:
	uint8_t* m_start = nullptr;
	uint8_t* m_end = nullptr;
};

//--

END_INFERNO_NAMESPACE()

#include "bufferView.inl"
