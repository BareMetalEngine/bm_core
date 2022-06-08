/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "bm/core/containers/include/inplaceArray.h"

BEGIN_INFERNO_NAMESPACE()

//--

TEST(InplaceArray, ConstructedArrayReportsEmpty)
{
	InplaceArray<int, 10> ar;
	EXPECT_TRUE(ar.empty());
}

TEST(InplaceArray, ConstructedArrayReportsZeroSize)
{
	InplaceArray<int, 10> ar;
	EXPECT_EQ(0, ar.size());
}

TEST(InplaceArray, ConstructedArrayReportsInplaceCapacity)
{
	InplaceArray<int, 10> ar;
	EXPECT_EQ(10, ar.capacity());
}

TEST(InplaceArray, ConstructedArrayReportsAsInplace)
{
	InplaceArray<int, 10> ar;
	EXPECT_TRUE(ar.inplace());
}

TEST(InplaceArray, PushingToEmptyArrayPreservesInplace)
{
	InplaceArray<int, 10> ar;
	ar.pushBack(1);
	EXPECT_TRUE(ar.inplace());
}

struct TYPE_ALIGN(16, AlignedTest)
{
	int x;
};

TEST(InplaceArray, InplaceElementsAreAligned)
{
	static_assert(alignof(AlignedTest) == 16, "Invalid alignment");

	uint32_t placeholder = 0;
	(void)placeholder;

	InplaceArray<AlignedTest, 10> ar;

	auto* ptr = ar.allocate(1);
	EXPECT_TRUE(IsAligned(ptr));
}

TEST(InplaceArray, ClearingArrayPreservesInplace)
{
	InplaceArray<int, 10> ar;
	ar.pushBack(1);
	ar.clear();
	EXPECT_TRUE(ar.inplace());
}

TEST(InplaceArray, ClearingArrayPreservesCapacity)
{
	InplaceArray<int, 10> ar;
	ar.pushBack(1);
	ar.clear();
	EXPECT_EQ(10, ar.capacity());
}

TEST(InplaceArray, ResettingsArrayPreservesInplace)
{
	InplaceArray<int, 10> ar;
	ar.pushBack(1);
	ar.reset();
	EXPECT_TRUE(ar.inplace());
}

TEST(InplaceArray, ResettingsArrayPreservesCapacity)
{
	InplaceArray<int, 10> ar;
	ar.pushBack(1);
	ar.reset();
	EXPECT_EQ(10, ar.capacity());
}

TEST(InplaceArray, ReserveSmallerTheCapacityPreservesInplace)
{
	InplaceArray<int, 10> ar;
	ar.reserve(10);
	EXPECT_TRUE(ar.inplace());
}

TEST(InplaceArray, ResizeSmallerTheCapacityPreservesInplace)
{
	InplaceArray<int, 10> ar;
	ar.resize(10);
	EXPECT_TRUE(ar.inplace());
}

TEST(InplaceArray, ArrayRelocatesWhenInplaceBufferFull)
{
	InplaceArray<int, 3> ar;
	ar.pushBack(1);
	ar.pushBack(2);
	ar.pushBack(3);
	EXPECT_TRUE(ar.inplace());
	ar.pushBack(4);
	EXPECT_FALSE(ar.inplace());
}

TEST(InplaceArray, RelocatedArrayPreservesElements)
{
	InplaceArray<int, 3> ar;
	ar.pushBack(1);
	ar.pushBack(2);
	ar.pushBack(3);
	ar.pushBack(4);
	EXPECT_EQ(1, ar[0]);
	EXPECT_EQ(2, ar[1]);
	EXPECT_EQ(3, ar[2]);
	EXPECT_EQ(4, ar[3]);
}

TEST(InplaceArray, CopyConstructorUsesInplaceMemory)
{
	Array<int> src = { 1,2,3 };
	InplaceArray<int, 4> ar;
	ar = src;
	EXPECT_EQ(3, ar.size());
	EXPECT_TRUE(ar.inplace());
}

TEST(InplaceArray, CopyConstructorUsesHeapIfToBig)
{
	Array<int> src = { 1,2,3,4,5 };
	InplaceArray<int, 4> ar;
	ar = src;
	EXPECT_EQ(5, ar.size());
	EXPECT_FALSE(ar.inplace());
}

TEST(InplaceArray, MoveConstructorUsesInplaceMemoryIfSourceSmaller)
{
	Array<int> src;
	src.resize(3);
	src[0] = 1;
	src[1] = 2;
	src[2] = 3;

	InplaceArray<int, 20> ar(std::move(src));
	EXPECT_EQ(3, ar.size());
	EXPECT_TRUE(ar.inplace());
	EXPECT_EQ(1, ar[0]);
	EXPECT_EQ(2, ar[1]);
	EXPECT_EQ(3, ar[2]);
}

TEST(InplaceArray, MoveConstructorUsesHeapMemoryIfSourceBigger)
{
	Array<int> src = { 1,2,3 };
	src.resize(3);
	src[0] = 1;
	src[1] = 2;
	src[2] = 3;

	InplaceArray<int, 4> ar(std::move(src));
	EXPECT_EQ(3, ar.size());
	EXPECT_FALSE(ar.inplace());
	EXPECT_EQ(1, ar[0]);
	EXPECT_EQ(2, ar[1]);
	EXPECT_EQ(3, ar[2]);
}

TEST(InplaceArray, MoveConstructorUsesHeapMemoryIfToBig)
{
	Array<int> src = { 1,2,3,4,5 };
	InplaceArray<int, 4> ar(std::move(src));
	EXPECT_EQ(5, ar.size());
	EXPECT_FALSE(ar.inplace());
}

TEST(InplaceArray, MoveUsesInplaceMemoryIfSmall)
{
	Array<int> src = { 1,2,3 };
	InplaceArray<int, 100> ar;
	ar = std::move(src);
	EXPECT_EQ(3, ar.size());
	EXPECT_TRUE(ar.inplace());
}

TEST(InplaceArray, MoveUsesHeapMemoryIfToBig)
{
	Array<int> src = { 1,2,3,4,5 };
	InplaceArray<int, 4> ar;
	ar = std::move(src);
	EXPECT_EQ(5, ar.size());
	EXPECT_FALSE(ar.inplace());
}

TEST(InplaceArray, AssignmentUsesInplaceMemory)
{
	Array<int> src = { 1,2,3 };
	InplaceArray<int, 4> ar;
	ar = src;
	EXPECT_EQ(3, ar.size());
	EXPECT_TRUE(ar.inplace());
}

TEST(InplaceArray, AssignmentUsesHeapMemoryIfToBig)
{
	Array<int> src = { 1,2,3,4,5 };
	InplaceArray<int, 4> ar;
	ar = src;
	EXPECT_EQ(5, ar.size());
	EXPECT_FALSE(ar.inplace());
}

TEST(InplaceArray, ClearingBigArrayRestoresInplace)
{
	InplaceArray<int, 4> ar;
	ar.resize(5);
	EXPECT_FALSE(ar.inplace());
	ar.clear();
	EXPECT_TRUE(ar.inplace());
	EXPECT_EQ(4, ar.capacity());
}

TEST(InplaceArray, ResettingBigArrayDoesNotRestoresInplace)
{
	InplaceArray<int, 4> ar;
	ar.resize(5);
	EXPECT_FALSE(ar.inplace());
	ar.reset();
	EXPECT_FALSE(ar.inplace());
	EXPECT_NE(4, ar.capacity());
}

//--

TEST(InplaceArray, MoveFromHeapArrayToInplaceArrayPreserveInplaceIfInplaceHasBiggerCapacity)
{
	Array<int> src;
	src.resizeWith(3, 1);

	InplaceArray<int, 100> dest;
	dest = std::move(src);
	EXPECT_TRUE(dest.inplace());
	EXPECT_EQ(100, dest.capacity());	

	EXPECT_EQ(0, src.size()); // original elements should be moved
	EXPECT_NE(0, src.capacity()); // original capacity should be preserve
}

TEST(InplaceArray, MoveFromHeapArrayToInplaceArrayDropsInplaceIfInplaceHasSmallerCapacity)
{
	Array<int> src;
	src.resizeWith(100, 1);

	InplaceArray<int, 4> dest;
	dest = std::move(src);
	EXPECT_FALSE(dest.inplace());
	EXPECT_NE(4, dest.capacity());

	EXPECT_EQ(0, src.size()); // original elements should be moved
	EXPECT_EQ(0, src.capacity()); // original array should now be empty
}

//--

TEST(InplaceArray, SwappingInplaceWorks)
{
	InplaceArray<int, 4> a = { 1,2,3 };
	InplaceArray<int, 4> b = { 4,5,6,7 };
	std::swap(a, b);
	EXPECT_EQ(4, a.size());
	EXPECT_EQ(3, b.size());
	EXPECT_EQ(4, a[0]);
	EXPECT_EQ(5, a[1]);
	EXPECT_EQ(6, a[2]);
	EXPECT_EQ(7, a[3]);
	EXPECT_EQ(1, b[0]);
	EXPECT_EQ(2, b[1]);
	EXPECT_EQ(3, b[2]);
}

TEST(InplaceArray, SwappingInplaceDoesNotSwapPointers)
{
	InplaceArray<int, 4> a = { 1,2,3 };
	InplaceArray<int, 4> b = { 4,5,6,7 };
	void* ptrA = a.data();
	void* ptrB = b.data();
	std::swap(a, b);
	EXPECT_EQ(a.data(), ptrA);
	EXPECT_EQ(b.data(), ptrB);
}

TEST(InplaceArray, SwappingInplaceKeepsInplaceIfThereSize)
{
	InplaceArray<int, 4> a = { 1 };
	InplaceArray<int, 4> b = { 4 };
	EXPECT_TRUE(a.inplace());
	EXPECT_TRUE(b.inplace());
	std::swap(a, b);
	EXPECT_TRUE(a.inplace());
	EXPECT_TRUE(b.inplace());
}

TEST(InplaceArray, SwappingInplaceKeepsInplaceIfThereSizeEvenIfCapacitySmaller)
{
	InplaceArray<int, 4> a = { 1 };
	InplaceArray<int, 1> b = { 4 };
	EXPECT_TRUE(a.inplace());
	EXPECT_TRUE(b.inplace());
	std::swap(*(Array<int>*)&a, *(Array<int>*)&b);
	EXPECT_TRUE(a.inplace());
	//EXPECT_TRUE(b.inplace()); // b should not be resized because despite having 15% of capacity it has 50% of elements
}

//--

END_INFERNO_NAMESPACE();