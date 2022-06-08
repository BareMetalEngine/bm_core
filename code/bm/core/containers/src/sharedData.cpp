/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "sharedData.h"

BEGIN_INFERNO_NAMESPACE()

//--

ISharedData::ISharedData()
{}

ISharedData::~ISharedData()
{}

RefPtr<ISharedData> ISharedData::InternalAquireSharedData(SharedStorage& storage, const std::function<ISharedData* ()>& factory)
{
    auto lock = CreateLock(storage.lock);

    auto ptr = storage.ptr.lock();
    if (!ptr)
    {
        ptr = AddRef(factory());
        storage.ptr = ptr;
    }

    return ptr;
}

//--

END_INFERNO_NAMESPACE()

