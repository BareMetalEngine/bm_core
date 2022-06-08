#pragma once

BEGIN_INFERNO_NAMESPACE()

//---

/// Base interface for classes that want to be constructed on demand and shared as long as they are in use
/// Typical use: shared rendering objects, un-managed resources, whenever data needs to be shared but adding a service would be an overkill
/// NOTE: shared data is also a singleton so it gets the deinit() to help with clearing memory at exit
class BM_CORE_CONTAINERS_API ISharedData : public IReferencable, public ISingleton
{
public:
    ISharedData();
    virtual ~ISharedData();

protected:
    struct SharedStorage
    {
        Mutex lock;
        RefWeakPtr<ISharedData> ptr;
    };

    static RefPtr<ISharedData> InternalAquireSharedData(SharedStorage& storage, const std::function<ISharedData* ()>& factory);
};

#define DECLARE_SHARED_DATA(_cls) \
private: static SharedStorage& GetSharedStorage(); \
public: static RefPtr<_cls> AquireSharedData() { return RefPtr<_cls>(AddRef(static_cast<_cls*>(InternalAquireSharedData(GetSharedStorage(), []() { return new _cls(); }).get()))); }

#define IMPLEMENT_SHARED_DATA(_cls) \
static SharedStorage& cls_::GetSharedStorage() { \
static SharedStorage theStorage; return theStorage; }

//---

END_INFERNO_NAMESPACE()
