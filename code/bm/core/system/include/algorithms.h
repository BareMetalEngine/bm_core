/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

BEGIN_INFERNO_NAMESPACE()

//--

typedef int32_t Index;
typedef uint32_t Count;

//--

/// Class that makes the copy constructor and assignment operator private
class NoCopy
{
public:
    ALWAYS_INLINE NoCopy() = default;
    ALWAYS_INLINE ~NoCopy() = default;
    ALWAYS_INLINE NoCopy& operator=(const NoCopy&) = delete;
    ALWAYS_INLINE NoCopy( const NoCopy& ) = delete;
};

//--

// align value to given any value
// NOTE: values does not have to be a power of two
template< typename T >
static ALWAYS_INLINE T Align(T offset, uint32_t alignment)
{
    T mask = (alignment - 1);
    return ((offset + mask) / alignment) * alignment;
}

// check if value is aligned
template< typename T >
static ALWAYS_INLINE bool IsAligned(T value, uint32_t alignment)
{
    return (((uint64_t)value) % alignment) == 0;
}

// check if value is aligned
template< typename T >
static ALWAYS_INLINE bool IsAligned(const T* ptr)
{
	return (((uint64_t)ptr) % alignof(T)) == 0;
}

// align pointer
template<typename T>
static ALWAYS_INLINE T* AlignPtr(T* ptr, size_t alignment)
{
    return (T*)(void*)Align<size_t>((size_t)ptr, alignment);
}

// compute number of required groups of given size to cover the given count
template< typename T >
static ALWAYS_INLINE T GroupCount(T count, T groupSize)
{
    return (count + (groupSize-1)) / groupSize;
}

// compute number of FULL groups required to cover given size, returns the remainder as well
template< typename T >
static ALWAYS_INLINE uint32_t GroupCountWithRemainder(T count, T groupSize, T& outRemainder)
{
    auto groups = (count / groupSize);
    outRemainder = count - (groups * groupSize);
    return groups;
}

// get difference (in bytes) between pointers
template<typename T>
static ALWAYS_INLINE ptrdiff_t PtrDirr(T* a, T* b)
{
    return (char*)a - (char*)b;
}

//! Offset a pointer by given amount of bytes
template <typename T>
static INLINE T* OffsetPtr(void* ptr, ptrdiff_t offset)
{
    return (T*)((char*)ptr + offset);
}

//! Offset a pointer by given amount of bytes
template <typename T>
static INLINE const T* OffsetPtr(const void* ptr, ptrdiff_t offset)
{
    return (T*)((const char*)ptr + offset);
}

//! Fash 32-bit hash
static INLINE uint32_t FastHash32(const void* data, uint32_t size)
{
    auto ptr  = (const uint8_t*)data;
    auto end  = ptr + size;
    uint32_t hval = UINT32_C(0x811c9dc5); // FNV-1a
    while (ptr< end)
    {
        hval ^= (uint32_t)*ptr++;
        hval *= UINT32_C(0x01000193);
    }
    return hval;
}

//! Atomically update maximum value
template<typename T>
static INLINE void UpdateMaximum(std::atomic<T>& maximum_value, T const& value) noexcept
{
	T prev_value = maximum_value;
	while (prev_value < value && !maximum_value.compare_exchange_weak(prev_value, value)) { /*loop*/ }
}

//! Check if we are power of two
template< typename T >
static ALWAYS_INLINE bool IsPowerOf2(T val)
{
    return (val & (val - 1)) == 0;
}

//! get next power of two value
template< typename T >
static ALWAYS_INLINE T NextPowerOf2(T x)
{
    // https://graphics.stanford.edu/~seander/bithacks.html#DetermineIfPowerOf2
    --x;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    ++x;
    return x;
}

namespace prv
{
    static const int MultiplyDeBruijnBitPosition[32] =
    {
      0, 9, 1, 10, 13, 21, 2, 29, 11, 14, 16, 18, 22, 25, 3, 30,
      8, 12, 20, 28, 15, 17, 24, 7, 19, 27, 23, 6, 26, 5, 4, 31
    };
} // prv

//! calculate Log2 (rounded down of given value 
//! e.g. log(2)->1, log(3)->1, log(4)->2, 
static ALWAYS_INLINE uint8_t FloorLog2(uint32_t v)
{
    // https://graphics.stanford.edu/~seander/bithacks.html#IntegerLogDeBruijn
	v |= v >> 1; // first round down to one less than a power of 2 
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
    return prv::MultiplyDeBruijnBitPosition[(uint32_t)(v * 0x07C4ACDDU) >> 27];
}

//--

END_INFERNO_NAMESPACE()

//-----------------------------------------------------------------------------

template< typename Dest, typename Src >
static INLINE Dest range_cast(Src x)
{
    return (Dest)x;
}

//-----------------------------------------------------------------------------
