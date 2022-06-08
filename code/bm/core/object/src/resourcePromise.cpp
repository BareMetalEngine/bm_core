/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "resource.h"
#include "resourcePromise.h"
#include "serializationStream.h"

BEGIN_INFERNO_NAMESPACE()

//---

ResourcePromise::ResourcePromise(ResourceID id, ResourceClass cls, StringBuf path, IResource* ptr)
	: m_id(id)
	, m_cls(cls)
	, m_path(path)
{
	if (ptr)
	{
		ptr->addRef();
		m_ptr = ptr;
	}
}

ResourcePromise::~ResourcePromise()
{
	if (auto ptr = m_ptr.exchange(nullptr))
		ptr->releaseRef();
}

ResourcePtr ResourcePromise::resolve() const
{
	if (auto* ptr = m_ptr.load())
		return ResourcePtr(AddRef(ptr));
	return nullptr;
}

void ResourcePromise::fulfill(IResource* resource)
{
	ASSERT(false == m_fullfilled.exchange(true));

	if (resource)
		resource->addRef();

	auto* oldPtr = m_ptr.exchange(resource);

	if (oldPtr)
		oldPtr->releaseRef();
}

//---

ResourcePromisePtr ResourcePromise::CreateEmptyPromise(const SerializationResourceKey& key)
{
	DEBUG_CHECK_RETURN_EX_V(key.id, "Invalid resource ID", nullptr);
	DEBUG_CHECK_RETURN_EX_V(key.className, "Invalid resource class", nullptr);

	auto classType = RTTI::GetInstance().findClass(key.className).cast<IResource>();
	DEBUG_CHECK_RETURN_EX_V(classType, "Invalid resource class", nullptr);

	return RefNew<ResourcePromise>(key.id, classType);
}

ResourcePromisePtr ResourcePromise::CreateEmptyPromise(ResourceID id, ResourceClass cls, StringBuf path)
{
	DEBUG_CHECK_RETURN_EX_V(id, "Invalid resource ID", nullptr);
	DEBUG_CHECK_RETURN_EX_V(cls, "Invalid resource class", nullptr);
	DEBUG_CHECK_RETURN_EX_V(cls.is<IResource>(), "Invalid resource class", nullptr);

	return RefNew<ResourcePromise>(id, cls, path);
}

//---

END_INFERNO_NAMESPACE()
