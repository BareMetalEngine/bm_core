/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"

BEGIN_INFERNO_NAMESPACE()

//--

#pragma section(".text")
__declspec(allocate(".text")) static uint8_t ASM_Test[] = {
	0x31, 0xc0,         // xor eax, eax
	0xc3,				// ret
};

typedef int (*TAsmCall)(void);
static TAsmCall GAsmTest = (TAsmCall)&ASM_Test[0];

TEST(AsmTest, SimpleCall)
{
	int result = GAsmTest();
	EXPECT_EQ(0, result);
}

//--

END_INFERNO_NAMESPACE()