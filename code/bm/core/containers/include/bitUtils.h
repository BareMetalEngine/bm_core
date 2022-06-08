/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

BEGIN_INFERNO_NAMESPACE()

/// bit word for the all of the bit storages
typedef uint64_t BitWord;

/// special bit state enums
enum class EBitStateZero { ZERO };
enum class EBitStateOne { ONE };

// number of words needed for given bit count
ALWAYS_INLINE uint32_t WorldCountForBitCount(uint32_t bitCount);

// test if single bit is set
ALWAYS_INLINE bool TestBit(const BitWord* bits, uint32_t index);

// clear single bit, returns previous value
ALWAYS_INLINE bool ClearBit(const BitWord* bits, uint32_t index);

// set single bit, returns previous value
ALWAYS_INLINE bool SetBit(const BitWord* bits, uint32_t index);

//--

// Find (linear bit search) index of first bit set in the set of bits, returns index>=maxBits if bit not found
ALWAYS_INLINE uint32_t FindFirstBitSet(const BitWord* bits, uint32_t maxBits);

// Find (linear bit search) index of first bit cleared in the set of bits, returns index>=maxBits if bit not found
ALWAYS_INLINE uint32_t FindFirstBitCleared(const BitWord* bits, uint32_t maxBits);

// Find (linear bit search) index of next bit set in the set of bits starting at given starting point, returns index>=maxBits if bit not found
ALWAYS_INLINE uint32_t FindNextBitSet(const BitWord* bits, uint32_t maxBits, uint32_t searchStart);

// Find (linear bit search) index of next bit cleared in the set of bits starting at given starting point, returns index>=maxBits if bit not found
ALWAYS_INLINE uint32_t FindNextBitCleared(const BitWord* bits, uint32_t maxBits, uint32_t searchStart);

//--

// compare bits between two bit sets
extern BM_CORE_CONTAINERS_API bool CompareBits(const BitWord* bitsA, const BitWord* bitsB, uint32_t firstBit, uint32_t numBits);

// returns true if ALL bits in the range is set
extern BM_CORE_CONTAINERS_API bool AllBitsSet(const BitWord* bits, uint32_t firstBit, uint32_t numBits);

// returns true if ANY bits in the range is set
extern BM_CORE_CONTAINERS_API bool AnyBitsSet(const BitWord* bits, uint32_t firstBit, uint32_t numBits);

// returns true if ALL bits in the range is set
extern BM_CORE_CONTAINERS_API bool AllBitsSet(const BitWord* bits, const BitWord* maskBits, uint32_t firstBit, uint32_t numBits);

// returns true if ANY bits in the range is set
extern BM_CORE_CONTAINERS_API bool AnyBitsSet(const BitWord* bits, const BitWord* maskBits, uint32_t firstBit, uint32_t numBits);

// or the bits from the given masks and in given range
extern BM_CORE_CONTAINERS_API void OrBits(BitWord* bits, const BitWord* maskBits, uint32_t firstBit, uint32_t numBits);

// and the bits from the given masks and in given range
extern BM_CORE_CONTAINERS_API void AndBits(BitWord* bits, const BitWord* maskBits, uint32_t firstBit, uint32_t numBits);

// xor the bits from the given masks and in given range
extern BM_CORE_CONTAINERS_API void XorBits(BitWord* bits, const BitWord* maskBits, uint32_t firstBit, uint32_t numBits);

// negate the bits from the given masks and in given range
extern BM_CORE_CONTAINERS_API void ClearBits(BitWord* bits, const BitWord* maskBits, uint32_t firstBit, uint32_t numBits);

// clear range of bits
extern BM_CORE_CONTAINERS_API void ClearBits(BitWord* bits, uint32_t firstBit, uint32_t numBits);

// set range of bits
extern BM_CORE_CONTAINERS_API void SetBits(BitWord* bits, uint32_t firstBit, uint32_t numBits);

//--

// calculate storage needed for the variable-length encoding number, up to 5 bytes are needed
ALWAYS_INLINE static uint8_t CalcAdaptiveNumberSize4(uint32_t number);

// calculate storage needed for the variable-length encoding number, up to 10 bytes are needed
ALWAYS_INLINE static uint8_t CalcAdaptiveNumberSize8(uint64_t number);

// encode variable length number into the memory
ALWAYS_INLINE static void WriteAdaptiveNumber4(void* ptr, uint8_t size, uint32_t number);

// encode variable length number into the memory
ALWAYS_INLINE static void WriteAdaptiveNumber8(void* ptr, uint8_t size, uint64_t number);

// read a 'compressed' uint64_t value with UTF-8 style encoding, returns the value and advances the stream pointer
ALWAYS_INLINE uint32_t ReadAdaptiveNumber4(const uint8_t* ptr, uint8_t& outValueSize);

// read a 'compressed' uint64_t value with UTF-8 style encoding, returns the value and advances the stream pointer
ALWAYS_INLINE uint64_t ReadAdaptiveNumber8(const uint8_t* ptr, uint8_t& outValueSize);

//--

END_INFERNO_NAMESPACE()

#include "bitUtils.inl"