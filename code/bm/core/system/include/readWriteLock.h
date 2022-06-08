/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

BEGIN_INFERNO_NAMESPACE()

//--

class RWLock;

//--

/// call forwarder for "shared" side so RWLock can be used with CreateLock
class RWLockReadProxy : public NoCopy
{
public:
	INLINE RWLockReadProxy(RWLock& lock) : m_lock(lock) {};
	INLINE RWLockReadProxy(RWLockReadProxy&& other) : m_lock(other.m_lock) {};
	INLINE ~RWLockReadProxy() {}

    INLINE void acquire();
    INLINE void release();

private:
    RWLock& m_lock;
};

//--

/// call forwarder for "exclusive" side so RWLock can be used with CreateLock
class RWLockWriteProxy : public NoCopy
{
public:
	INLINE RWLockWriteProxy(RWLock& lock) : m_lock(lock) {};
	INLINE RWLockWriteProxy(RWLockWriteProxy&& other) : m_lock(other.m_lock) {};
	INLINE ~RWLockWriteProxy() {}

	INLINE void acquire();
	INLINE void release();

private:
	RWLock& m_lock;
};

//--

/// RW lock, platform specific implementation
class BM_CORE_SYSTEM_API RWLock : public NoCopy
{
public:
    RWLock();
    ~RWLock();

    void acquireShared();
    void releaseShared();

	void acquireExclusive();
	void releaseExclusive();

	INLINE RWLockReadProxy shared() const { return RWLockReadProxy((RWLock&)* this); }
	INLINE RWLockWriteProxy exclusive() const { return RWLockWriteProxy((RWLock&)*this); }


private:
	union {
		void* handle;
		uint8_t data[64];
	} m_data;

	friend class ConditionVariable;
};

//--

INLINE void RWLockReadProxy::acquire()
{
	m_lock.acquireShared();
}

INLINE void RWLockReadProxy::release()
{
	m_lock.releaseShared();
}

//--

INLINE void RWLockWriteProxy::acquire()
{
	m_lock.acquireExclusive();
}

INLINE void RWLockWriteProxy::release()
{
	m_lock.acquireExclusive();
}

//--

END_INFERNO_NAMESPACE()

