/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "refCounted.h"
#include "refPtr.h"
#include "refWeakPtr.h"

#include <unordered_set>

//#define TRACK_REF_PTR

BEGIN_INFERNO_NAMESPACE()

///--

RefWeakContainer::RefWeakContainer(IReferencable* ptr)
    : m_ptr(ptr)
{}

RefWeakContainer::~RefWeakContainer()
{
    if (m_ptr != nullptr)
    {
        TRACE_ERROR("Object still referenced");
    }

    DEBUG_CHECK_EX(m_refCount.load() == 0, "Invalid ref count");
    DEBUG_CHECK_EX(m_ptr == nullptr, "Object still referenced");
}

void RefWeakContainer::addRef()
{
    ++m_refCount;
}

void RefWeakContainer::releaseRef()
{
    if (0 == --m_refCount)
        delete this;
}

void RefWeakContainer::drop()
{
    auto lock = CreateLock(m_lock);
    m_ptr = nullptr;
}

IReferencable* RefWeakContainer::lock()
{
    IReferencable* ret = nullptr;

    {
        auto lock = CreateLock(m_lock);
        if (m_ptr)
        {
            m_ptr->addRef();
            ret = m_ptr;
        }
    }

    return ret;
}

///--
    
#ifdef TRACK_REF_PTR
class PointerTrackingRegistry
{
public:
    PointerTrackingRegistry()
    {}

    static PointerTrackingRegistry& GetInstance()
    {
        static PointerTrackingRegistry theInstance;
        return theInstance;
    }

    //--

    Mutex m_lock;

    struct TrackingPointer;

    struct TrackingRef
    {
        uint32_t callstack = 0;
    };

    std::atomic<uint32_t> m_activeRefIndex = 1;
    std::unordered_map<uint32_t, TrackingRef*> m_activeRefs;

    struct TrackingPointer
    {
        IReferencable* ptr = nullptr;
        uint32_t creationCallstack = 0;
        std::unordered_set<uint32_t> refs;
    };

    std::unordered_map<void*, TrackingPointer*> m_activePointers;

    //--

    uint32_t CreateRef()
    {
        auto index = m_activeRefIndex++;
        auto callstackIndex = debug::CaptureCallstack(2);

        {
            auto lock = CreateLock(m_lock);
            auto* entry = new TrackingRef;
            entry->callstack = callstackIndex;
            m_activeRefs[index] = entry;
        }

        return index;
    }

    void ReleaseRef(uint32_t index)
    {
        auto lock = CreateLock(m_lock);

        auto ptr = m_activeRefs.find(index);
        if (ptr != m_activeRefs.end())
            delete ptr->second;
        m_activeRefs.erase(ptr);
    }

    void TrackPointer(IReferencable* ptr)
    {
        auto lock = CreateLock(m_lock);

        auto it = m_activePointers.find(ptr);
        ASSERT_EX(it == m_activePointers.end(), "Pointer already tracked");
        if (it != m_activePointers.end())
            return;

        auto* entry = new TrackingPointer;
        entry->ptr = ptr;
        entry->creationCallstack = debug::CaptureCallstack(2);
        m_activePointers[ptr] = entry;
    }

    void UntrackPointer(IReferencable* ptr)
    {
        auto lock = CreateLock(m_lock);

        auto it = m_activePointers.find(ptr);
        DEBUG_CHECK_EX(it != m_activePointers.end(), "Pointer not tracked");
        if (it != m_activePointers.end())
        {
            DEBUG_CHECK_EX(it->second->refs.empty(), "Pointer has tracking refs");
            delete it->second;
            m_activePointers.erase(it);
        }
    }

    void PrintActivePointer()
    {
        auto lock = CreateLock(m_lock);

        TRACE_INFO("There are still {} active IReferencables", m_activePointers.size());

        uint32_t index = 0;
        for (const auto& it : m_activePointers)
        {
            TRACE_INFO("  [{}]: 0x{} ({})", index, Hex((uint64_t)it.second->ptr), typeid(*(it.second->ptr)).name());

            const auto& stack = debug::ResolveCapturedCallstack(it.second->creationCallstack);
            if (!stack.empty())
                TRACE_INFO("     Creation callstack: {}", stack);

            index += 1;
        }
    }
};
#endif
///--

IReferencable::IReferencable()
    : m_refCount(0)
{
    m_weakHolder = new RefWeakContainer(this);
#ifdef TRACK_REF_PTR
    if (!IsDefaultObjectCreation())
        PointerTrackingRegistry::GetInstance().TrackPointer(this);
#endif
}

IReferencable::~IReferencable()
{
    ASSERT_EX(m_refCount.load() == 0, "Deleting object with non zero reference count");

    if (m_weakHolder)
    {
        m_weakHolder->drop();
        m_weakHolder->releaseRef();
        m_weakHolder = nullptr;
    }

#ifdef TRACK_REF_PTR
    PointerTrackingRegistry::GetInstance().UntrackPointer(this);
#endif
}

void IReferencable::addRef()
{
    auto refCount = m_refCount++;
    if (refCount == 0)
        resurrect();
}

void IReferencable::releaseRef()
{
    DEBUG_CHECK_EX(m_refCount.load() > 0, "Cannot release reference of disposed object");

    auto refCount = --m_refCount;
    DEBUG_CHECK_EX(refCount >= 0, "Invalid reference count on release");
    if (0 == refCount)
        dispose();
}

void IReferencable::print(IFormatStream& f) const
{
    f.appendf("0x{}", Hex(this));
}

RefWeakContainer* IReferencable::makeWeakRef() const
{
    auto* ret = m_weakHolder;
    if (ret)
        ret->addRef();
    return ret;
}

void IReferencable::resurrect()
{
}

void IReferencable::dispose()
{
    DEBUG_CHECK_EX(m_refCount.load() == 0, "Invalid refcount for a dispose() call");

    if (m_weakHolder)
    {
        m_weakHolder->drop();
        m_weakHolder->releaseRef();
        m_weakHolder = nullptr;
    }

    delete this;
}

///--

BaseRefPtr::BaseRefPtr()
{}

void BaseRefPtr::addRefInternal(void* ptr)
{
    DEBUG_CHECK_EX(m_currentTrackingId == 0, "Ref already tracking something");

    if (ptr)
    {
        //m_currentTrackingId =
        ((IReferencable*)ptr)->addRef();
    }
}

void BaseRefPtr::printRefInternal(void* ptr, IFormatStream& f) const
{
    if (ptr)
        ((IReferencable*)ptr)->print(f);
    else
        f.append("null");
}

void BaseRefPtr::swapRefInternal(void** ptr, void* newPtr)
{
    if (*ptr != newPtr)
    {
        auto oldPtr = *(IReferencable**)ptr;
        m_currentTrackingId = 0;
        *ptr = nullptr;

        if (newPtr)
        {
            *ptr = newPtr;
            (*(IReferencable**)ptr)->addRef();
        }

        if (oldPtr)
            oldPtr->releaseRef();
    }
}

void BaseRefPtr::releaseRefInternal(void** ptr)
{
    if (*ptr)
    {
        auto old = *(IReferencable**)ptr;
        *ptr = nullptr;
        old->releaseRef();
    }
}

///--

void DumpLiveRefCountedObjects()
{
#ifdef TRACK_REF_PTR 
    PointerTrackingRegistry::GetInstance().PrintActivePointer();
#endif
}

///--

TYPE_TLS volatile uint32_t GDefaultObjectCreationDepth = 0;

void EnterDefaultObjectCreation()
{
    GDefaultObjectCreationDepth += 1;
}

void LeaveDefaultObjectCreation()
{
    ASSERT(GDefaultObjectCreationDepth > 0);
    GDefaultObjectCreationDepth -= 1;
}

bool IsDefaultObjectCreation()
{
    return GDefaultObjectCreationDepth > 0;
}

//--

END_INFERNO_NAMESPACE()

