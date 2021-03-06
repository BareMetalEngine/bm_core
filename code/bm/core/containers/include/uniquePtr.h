/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

BEGIN_INFERNO_NAMESPACE()

/// auto-delete, single ownership pointer wrapper
template< typename T >
class UniquePtr : public MainPoolData<NoCopy>
{
public:
    INLINE UniquePtr();
    INLINE UniquePtr(std::nullptr_t);
    INLINE explicit UniquePtr(T* ptr);
    INLINE UniquePtr(UniquePtr&& other);
    INLINE ~UniquePtr();

    template< typename U >
    INLINE UniquePtr(UniquePtr< U > && other);

    INLINE UniquePtr& operator=(UniquePtr&& other);

    template<typename U>
    INLINE UniquePtr& operator=(UniquePtr<U> && other);

    //---
        
    // extract the held pointer
    INLINE T* get() const;

    // pointer/reference access
    INLINE T& operator*() const;
    INLINE T* operator->() const;

    // restore ownership as a raw pointer
    INLINE T* release();

    // reset or replace 
    INLINE void reset(T* newPtr=nullptr);

    // replace with new object
    template<typename... Args>
    INLINE void create(Args && ... args);

    // boolean expression
    INLINE explicit operator bool() const;
    INLINE bool operator!() const;

protected:
    T* m_ptr;

    void destroy();
};

template< typename T, typename... Args >
UniquePtr< T > CreateUniquePtr(Args && ... args);

template< typename LeftType, typename RightType >
INLINE bool operator==(const UniquePtr< LeftType >& leftPtr, const UniquePtr< RightType >& rightPtr);

template< typename LeftType, typename RightType >
INLINE bool operator!=(const UniquePtr< LeftType >& leftPtr, const UniquePtr< RightType >& rightPtr);

template< typename LeftType, typename RightType >
INLINE bool operator<(const UniquePtr< LeftType >& leftPtr, const UniquePtr< RightType >& rightPtr);

template< typename T, typename... Args >
INLINE UniquePtr< T > CreateUniquePtr(Args && ... args)
{
    return UniquePtr<T>(new T(std::forward< Args >(args)...));
}

END_INFERNO_NAMESPACE()

#include "uniquePtr.inl"
