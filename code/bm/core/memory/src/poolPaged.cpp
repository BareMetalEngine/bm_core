/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "poolPaged.h"

BEGIN_INFERNO_NAMESPACE()

//--

IPoolPaged::IPoolPaged(const char* name)
	: IPool(name, PoolType::Paged)
{}

IPoolPaged::~IPoolPaged()
{}

//---

END_INFERNO_NAMESPACE()
