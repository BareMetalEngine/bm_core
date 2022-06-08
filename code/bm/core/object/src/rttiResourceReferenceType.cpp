/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
*
* [#filter: rtti\types #]
***/

#include "build.h"
#include "rttiResourceReferenceType.h"

BEGIN_INFERNO_NAMESPACE()

//--

IResourceReferenceType::IResourceReferenceType(StringID name)
    : IType(name)
{}

IResourceReferenceType::~IResourceReferenceType()
{}

//--

END_INFERNO_NAMESPACE()
