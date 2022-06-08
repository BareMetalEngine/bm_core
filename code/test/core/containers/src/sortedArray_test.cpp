/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "bm/core/containers/include/array.h"

BEGIN_INFERNO_NAMESPACE()

TEST(SortedArray, InitFromDataUnsorted)
{
	int numbers[10] = { 7,5,3,2,6,9,4,0,1,8 };

	Array<int> data(numbers);
	EXPECT_EQ(10, data.size());
	EXPECT_FALSE(data.view().checkSortedOrder());
	EXPECT_FALSE(data.view().checkStrictSortedOrder());
}

TEST(SortedArray, InitFromDataSorted)
{
	int numbers[10] = { 7,5,3,2,6,9,4,0,1,8 };

	Array<int> data(numbers);
	data.sort();

	EXPECT_TRUE(data.view().checkSortedOrder());
	EXPECT_TRUE(data.view().checkStrictSortedOrder());
}

TEST(SortedArray, InitFromMultipleCopiesSorted)
{
	int numbers[20] = { 7,5,3,2,6,9,4,0,1,8,5,9,6,3,4,0,2,7,8,1 };

	Array<int> data(numbers);
	data.sort();

	EXPECT_TRUE(data.view().checkSortedOrder());
	EXPECT_FALSE(data.view().checkStrictSortedOrder());
}

TEST(SortedArray, InitFromMultipleCopiesSortedRemoveDuplicates)
{
	int numbers[20] = { 7,5,3,2,6,9,4,0,1,8,5,9,6,3,4,0,2,7,8,1 };

	Array<int> data(numbers);
	data.sort();

	EXPECT_TRUE(data.view().checkSortedOrder());
	EXPECT_FALSE(data.view().checkStrictSortedOrder());

	auto removed = data.sortedArrayRemoveDuplicates();
	EXPECT_EQ(10, removed);
	EXPECT_EQ(10, data.size());

	EXPECT_TRUE(data.view().checkSortedOrder());
	EXPECT_TRUE(data.view().checkStrictSortedOrder());
}

TEST(SortedArray, InitFromMultipleCopiesSortedRemoveDuplicatesInOneGo)
{
	int numbers[20] = { 7,5,3,2,6,9,4,0,1,8,5,9,6,3,4,0,2,7,8,1 };

	Array<int> data(numbers);

	EXPECT_FALSE(data.view().checkSortedOrder());
	EXPECT_FALSE(data.view().checkStrictSortedOrder());

	auto removed = data.removeDuplicatesAndSort();
	EXPECT_EQ(10, removed);
	EXPECT_EQ(10, data.size());

	EXPECT_TRUE(data.view().checkSortedOrder());
	EXPECT_TRUE(data.view().checkStrictSortedOrder());
}

TEST(SortedArray, RemoveExisting)
{
	Array<int> data({0,1,2,3,4,5,6,7,8,9});
	EXPECT_EQ(10, data.size());

	auto removed = data.sortedArrayRemove(5);
	EXPECT_TRUE(removed);
	EXPECT_EQ(9, data.size());
	EXPECT_FALSE(data.contains(5));
}

TEST(SortedArray, RemovePreservedOrder)
{
	Array<int> data({ 0,1,2,3,4,5,6,7,8,9 });
	EXPECT_EQ(10, data.size());
	EXPECT_TRUE(data.view().checkStrictSortedOrder());

	auto removed = data.sortedArrayRemove(5);
	EXPECT_TRUE(data.view().checkStrictSortedOrder());
}

TEST(SortedArray, ContainsTest)
{
	Array<int> data({ 0,1,2,3,4,5,6,7,8,9 });
	EXPECT_EQ(10, data.size());

	EXPECT_TRUE(data.sortedArrayContains(5));
	EXPECT_FALSE(data.sortedArrayContains(-1));
	EXPECT_FALSE(data.sortedArrayContains(10));
}

TEST(SortedArray, RemovingNonExistingFails)
{
	Array<int> data({ 0,1,2,3,4,6,7,8,9 });
	EXPECT_EQ(9, data.size());

	EXPECT_FALSE(data.sortedArrayRemove(-1));
	EXPECT_FALSE(data.sortedArrayRemove(5));
	EXPECT_FALSE(data.sortedArrayRemove(10));
	EXPECT_EQ(9, data.size());
}

TEST(SortedArray, InsertAtFrontWorks)
{
	Array<int> data({ 1,2,3,4,5,6,7,8,9 });
	EXPECT_EQ(9, data.size());

	data.sortedArrayInsert(0);
	EXPECT_EQ(10, data.size());
	EXPECT_EQ(0, data[0]);
}

TEST(SortedArray, InsertAtEndWorks)
{
	Array<int> data({ 1,2,3,4,5,6,7,8,9 });
	EXPECT_EQ(9, data.size());

	data.sortedArrayInsert(10);
	EXPECT_EQ(10, data.size());
	EXPECT_EQ(10, data[9]);
}

TEST(SortedArray, InsertAtMiddleWorks)
{
	Array<int> data({ 0,1,2,3,4,6,7,8,9 });
	EXPECT_EQ(9, data.size());

	data.sortedArrayInsert(5);
	EXPECT_EQ(10, data.size());
	EXPECT_EQ(5, data[5]);
}

TEST(SortedArray, InsertDuplicateWorks)
{
	Array<int> data({ 0,1,2,3,4,5,6,7,8,9 });
	EXPECT_EQ(10, data.size());

	data.sortedArrayInsert(5);
	EXPECT_EQ(11, data.size());
}

TEST(SortedArray, InsertDuplicatdElementsAreClose)
{
	Array<int> data({ 0,1,2,3,4,5,6,7,8,9 });
	EXPECT_EQ(10, data.size());

	data.sortedArrayInsert(5);
	EXPECT_EQ(5, data[5]);
	EXPECT_EQ(5, data[6]);
}

TEST(SortedArray, InsertDuplicateFiltered)
{
	Array<int> data({ 0,1,2,3,4,5,6,7,8,9 });
	EXPECT_EQ(10, data.size());

	EXPECT_FALSE(data.sortedArrayInsertUnique(5));
	EXPECT_EQ(10, data.size());
}

TEST(SortedArray, FindFindsFirstAtStart)
{
	Array<int> data({ 0,1,2,3,4,5,6,7,8,9 });

	EXPECT_EQ(0, data.view().sortedFindFirst(0));
}

TEST(SortedArray, FindFindsFirstAtEnd)
{
	Array<int> data({ 0,1,2,3,4,5,6,7,8,9 });
	EXPECT_EQ(9, data.view().sortedFindFirst(9));
}

TEST(SortedArray, FindFindsFindsAtMiddle)
{
	Array<int> data({ 0,1,2,3,4,5,6,7,8,9 });
	EXPECT_EQ(5, data.view().sortedFindFirst(5));
}

TEST(SortedArray, FindFindsFirstFailsBeforeStart)
{
	Array<int> data({ 0,1,2,3,4,5,6,7,8,9 });

	EXPECT_EQ(-1, data.view().sortedFindFirst(-1));
}

TEST(SortedArray, FindFindsFirstFailsAfterEnd)
{
	Array<int> data({ 0,1,2,3,4,5,6,7,8,9 });

	EXPECT_EQ(-1, data.view().sortedFindFirst(11));
}

TEST(SortedArray, FindFindsFirstFailsMiddle)
{
	Array<int> data({ 0,1,2,3,4,6,7,8,9 });
	EXPECT_EQ(-1, data.view().sortedFindFirst(5));
}

TEST(SortedArray, FindFindsFindsFirst)
{
	Array<int> data({ 0,1,2,3,3,3,4,5,6 });
	EXPECT_EQ(3, data.view().sortedFindFirst(3));
}

//--

END_INFERNO_NAMESPACE()