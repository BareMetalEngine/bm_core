/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "poolManaged.h"

BEGIN_INFERNO_NAMESPACE()

//--

IMemoryBlockStateResolver::~IMemoryBlockStateResolver()
{}

//--

IPoolManaged::IPoolManaged(const char* name)
	: IPool(name, PoolType::Managed)
{}

IPoolManaged::~IPoolManaged()
{}

//---

END_INFERNO_NAMESPACE()
