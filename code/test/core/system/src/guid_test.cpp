/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "bm/core/system/include/guid.h"

BEGIN_INFERNO_NAMESPACE()

///--

TEST(GUID, GenetesNonZoro)
{
	auto g = GUID::Create();
	ASSERT_FALSE(g.empty());
}

TEST(GUID, GenetesUnique)
{
	auto a = GUID::Create();
	auto b = GUID::Create();
	auto c = GUID::Create();
	EXPECT_NE(a, b);
	EXPECT_NE(b, c);
	EXPECT_NE(c, a);
}

///--

END_INFERNO_NAMESPACE()