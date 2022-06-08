/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "resource.h"
#include "resourceReference.h"
#include "resourcePromise.h"

BEGIN_INFERNO_NAMESPACE()

//---

BaseReference::BaseReference(const IResource* ptr)
    : m_ptr(AddRef(ptr))
{}

BaseReference::BaseReference(const ResourcePromise* promise)
	: m_promise(AddRef(promise))
{}

BaseReference::BaseReference(const BaseReference& other)
    : m_ptr(other.m_ptr)
    , m_promise(other.m_promise)
{}

BaseReference::BaseReference(BaseReference&& other) 
    : m_ptr(other.m_ptr)    
    , m_promise(other.m_promise)
{
    other.m_promise.reset();
    other.m_ptr.reset();
}

BaseReference& BaseReference::operator=(const BaseReference& other)
{
	if (this != &other)
	{
		m_ptr = other.m_ptr;
		m_promise = other.m_promise;
	}

	return *this;
}

BaseReference& BaseReference::operator=(BaseReference&& other)
{
	if (this != &other)
	{
		m_ptr = other.m_ptr;
		m_promise = other.m_promise;

		other.m_promise.reset();
		other.m_ptr.reset();
	}

	return *this;
}

ResourceID BaseReference::resolveId() const
{
    if (m_promise)
        return m_promise->id();
    return nullptr;
}

ResourceClass BaseReference::resolveClass() const
{
    if (m_ptr)
        return m_ptr->cls().cast<IResource>();
	if (m_promise)
		return m_promise->cls();
	return nullptr;
}

ResourcePtr BaseReference::resolveResource() const
{
    if (m_ptr)
        return m_ptr;
    else if (m_promise)
        return m_promise->resolve();

    return nullptr;
}

StringBuf BaseReference::resolvePath() const
{
    if (m_promise)
        return m_promise->path();
    else if (m_ptr)
        return m_ptr->loadPath();
    else
        return "";
}

bool BaseReference::operator==(const BaseReference& other) const
{
    return (m_ptr == other.m_ptr) && (m_promise == other.m_promise);
}

void BaseReference::reset()
{
    m_ptr = nullptr;
	m_promise = nullptr;
}

void BaseReference::print(IFormatStream& f) const
{
    if (m_promise)
        f.appendf("{{}: {}}", m_promise->cls(), m_promise->id());
    else if (m_ptr)
        f.appendf("{Inlined: {}}", m_ptr->cls());
    else
        f << "null";
}

//--

uint32_t BaseReference::CalcHash(const BaseReference& ref)
{
    if (ref.m_promise)
        return (uint64_t)(ref.m_promise.get()) >> 8;

    else if (ref.inlined())
        return (uint64_t)(ref.m_ptr.get()) >> 8;

    return 0;
}

//--

END_INFERNO_NAMESPACE()
