/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

#include "array.h"
#include "inplaceArray.h"
#include "bitUtils.h"

BEGIN_INFERNO_NAMESPACE()

/// set of bits
class BM_CORE_CONTAINERS_API BitSet
{
public:
	ALWAYS_INLINE BitSet();
	BitSet(const BitSet& other);
	ALWAYS_INLINE BitSet(BitSet&& other);
	BitSet& operator=(const BitSet& other);
	ALWAYS_INLINE BitSet& operator=(BitSet&& other);
	ALWAYS_INLINE BitSet(uint32_t bitCount, EBitStateZero);
	ALWAYS_INLINE BitSet(uint32_t bitCount, EBitStateOne);
	~BitSet();

	//--

	// is the bit set empty ? (no bit)
	ALWAYS_INLINE bool empty() const;

	// get number of bits in use
	ALWAYS_INLINE uint32_t bitCount() const;

	//--

	// get bit data
	ALWAYS_INLINE BitWord* words();

	// get bit data
	ALWAYS_INLINE const BitWord* words() const;

    // get number of words in use
    ALWAYS_INLINE uint32_t wordCount() const;

	//--

	// clear container
	void clear();

	// resize container to hold X bits, all bits are initialized to 0
	// NOTE: previous content is NOT kept
	void resizeWithZeros(uint32_t bitCount);

	// resize container to hold X bits, all bits are initialized to 1
	// NOTE: previous content is NOT kept
	void resizeWithOnes(uint32_t bitCount);

	//--

	// clear all bits
	ALWAYS_INLINE void clearAll();

	// set all bits
	ALWAYS_INLINE void enableAll();

	//--

	// get state of n-th bit
	ALWAYS_INLINE bool operator[](uint32_t index) const;

	// set bit to 1, returns previous bit state
	ALWAYS_INLINE bool set(uint32_t index);

	// clear bit to 0, returns previous bit state
	ALWAYS_INLINE bool clear(uint32_t index);

	// set bit to given state, returns previous bit state
	ALWAYS_INLINE bool toggle(uint32_t index, bool state);

	//--

	// or with other set
	BitSet& operator|=(const BitSet& otherMask);

	// and the bits between sets
	BitSet& operator&=(const BitSet& otherMask);

	// xor the bits between sets
	BitSet& operator^=(const BitSet& otherMask);

	// remove the bits between sets
	BitSet& operator-=(const BitSet& otherMask);

    //---

    // are all bits in the bit set set ? true for empty bitset
    bool isAllSet() const;

    // is any bits from given test mask set in here ? false for empty bitset
    bool isAnySet() const;

    // are none bit set ? true for empty bitset
    bool isNoneSet() const;

    //----

    // compare if two bitsets are equal
    bool operator==(const BitSet& other) const;
		
    // compare if two bitsets are not equal
    bool operator!=(const BitSet& other) const;

    //----

    // iterate over all bits that are set
	INLINE bool iterateSetBits(const std::function<bool(uint32_t index)>& enumFunc) const;

    // iterate over all bits that are clear
	INLINE bool iterateClearBits(const std::function<bool(uint32_t index)>& enumFunc) const;

	// 

private:
	uint32_t m_bitSize = 0;
	BitWord* m_words = nullptr;

	static const uint32_t NUM_BITS_IN_WORD = sizeof(BitWord) * 8;
	static const uint32_t BIT_WORD_SHIFT = 5;
	static const uint32_t BIT_INDEX_MASK = (1U << BIT_WORD_SHIFT) - 1;
	static const BitWord ALL_ONES = ~(BitWord)0;
	static const BitWord ALL_ZEROS = (BitWord)0;	
};


END_INFERNO_NAMESPACE()

#include "bitSet.inl"