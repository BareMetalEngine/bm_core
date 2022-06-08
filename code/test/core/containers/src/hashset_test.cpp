/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "bm/core/containers/include/hashSet.h"

DECLARE_TEST_FILE(HashSet);

BEGIN_INFERNO_NAMESPACE()

TEST(HashSet, SimpleTest)
{
    HashSet<int> x;

    for (int i = 0; i < 300; ++i)
    {
        EXPECT_TRUE(x.insert(i));
    }

    for (int i = 0; i < 300; ++i)
    {
        EXPECT_TRUE(x.contains(i));
    }

    for (int i = 0; i < 300; i += 3)
    {
        EXPECT_TRUE(x.remove(i));
    }

    for (int i = 0; i < 300; i += 7)
    {
        if (i % 3 == 0)
            continue;
        EXPECT_TRUE(x.remove(i));
    }

    for (int i = 0; i < 300; i += 11)
    {
        if (i % 3 == 0)
            continue;
        if (i % 7 == 0)
            continue;
        EXPECT_TRUE(x.remove(i));
    }

    for (int i = 0; i < 300; i++)
    {
        if (i % 3 == 0)
            continue;
        if (i % 7 == 0)
            continue;
        if (i % 11 == 0)
            continue;
        EXPECT_TRUE(x.contains(i));
    }

    for (int i = 3; i < 300; i+=3)
    {
        EXPECT_TRUE(x.insert(i));
    }

    for (int i = 0; i < 300; i++)
    {
        if (i % 7 == 0)
            continue;
        if (i % 11 == 0)
            continue;
        EXPECT_TRUE(x.contains(i));
    }

    for (int i = 0; i < 300; i++)
    {
        if (i % 7 == 0)
            continue;
        if (i % 11 == 0)
            continue;
        EXPECT_TRUE(x.remove(i));
    }

    for (int i = 0; i < 300; i++)
    {
        x.remove(i);
    }

    EXPECT_TRUE(x.empty());

}

END_INFERNO_NAMESPACE()