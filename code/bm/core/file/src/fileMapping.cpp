/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "fileMapping.h"

BEGIN_INFERNO_NAMESPACE()

//--

IFileMapping::~IFileMapping()
{}

IFileMapping::IFileMapping(StringBuf info, const void* data, uint64_t size)
	: m_info(info)
	, m_data((const uint8_t*)data)
	, m_size(size)
{}

Buffer IFileMapping::createBuffer() const
{
	auto selfRef = FileMappingPtr(AddRef(this));
	return Buffer::CreateExternal(view(), [selfRef](void*) {});
}

//--

END_INFERNO_NAMESPACE()
