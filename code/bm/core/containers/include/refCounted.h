/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

BEGIN_INFERNO_NAMESPACE()

//---

class IReferencable;

//---

/// helper class for referencable objects that want to have weak refs to them
class BM_CORE_CONTAINERS_API RefWeakContainer : public MainPoolData<NoCopy>
{
public:
    RefWeakContainer(IReferencable* ptr);

    void addRef();
    void releaseRef();

    void drop(); // invalidates all references EVEN if the object has a non-zero reference count (i.e. "you are dead to me")

    IReferencable* lock(); // returns a valid referencable with +1 reference count or NULL

    INLINE IReferencable* unsafe() const { return m_ptr; }

    INLINE bool expired() const { return m_ptr == nullptr; }

private:
    ~RefWeakContainer();

    std::atomic<uint32_t> m_refCount = 1;
    IReferencable* m_ptr; // unreferenced

    SpinLock m_lock;
};

//---

#pragma pack(push, 4)

/// basic implementation of a intrusive reference counting object
class BM_CORE_CONTAINERS_API IReferencable : public MainPoolData<NoCopy>
{
public:
    // Reference counted objects are constructed with initial refcount of 0 so it can be placed on stack or used with manual new/delete
    // To pass newly created object to a RefPtr one must use AddRef() helper
    IReferencable();

    // object can only be destroyed when having 0 reference count
    virtual ~IReferencable();

    //--

    // add a reference
    void addRef();

    // release a reference, when count reaches zero the dispose() function will be called
    void releaseRef();

    //--

    // lock a weak reference
    RefWeakContainer* makeWeakRef() const;

    //--

    // print some object description
    virtual void print(IFormatStream& f) const;

    //--

    // dispose of this object - called when reference count reaches zero
    virtual void dispose();

    // resurrect object that has a refcount of zero, returns a non zero reference
    virtual void resurrect();

private:
    RefWeakContainer* m_weakHolder = nullptr;
    std::atomic<uint32_t> m_refCount = 0;
};

#pragma pack(pop)

static_assert(sizeof(IReferencable) == 20, "Some code makes assumption about size of this class");

//---

// dump all still allocated reference counted objects
extern BM_CORE_CONTAINERS_API void DumpLiveRefCountedObjects();

// enter the default object creation mode on current thread
extern BM_CORE_CONTAINERS_API void EnterDefaultObjectCreation();

// leave the default object creation mode on current thread
extern BM_CORE_CONTAINERS_API void LeaveDefaultObjectCreation();

// are we creating a default object ?
extern BM_CORE_CONTAINERS_API bool IsDefaultObjectCreation();

//---

END_INFERNO_NAMESPACE()
