/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
*
* [#filter: stubs #]
***/

#include "build.h"
#include "stub.h"

#include "bm/core/memory/include/localAllocator.h"

BEGIN_INFERNO_NAMESPACE()

//--

IStub::IStub()
{}

IStub::~IStub()
{}

void IStub::postLoad()
{}

//--

IStubReader::IStubReader(uint32_t version)
: m_version(version)
{}

IStubReader::~IStubReader()
{}

IStubWriter::~IStubWriter()
{}

//--

END_INFERNO_NAMESPACE()
