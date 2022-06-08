/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

BEGIN_INFERNO_NAMESPACE()

//--

template< typename T, typename LockT = SpinLock >
class ThreadSafeProxy : public NoCopy
{
public:
	INLINE ThreadSafeProxy(T* data, LockT* lock)
		: m_data(data)
		, m_lock(lock)
	{
		m_lock->acquire();
	}

	INLINE ~ThreadSafeProxy()
	{
		if (m_lock)
			m_lock->release();
	}

	INLINE ThreadSafeProxy(ThreadSafeProxy&& other)
	{
		m_data = other.m_data;
		m_lock = other.m_lock;
		other.m_data = nullptr;
		other.m_lock = nullptr;
	}

	INLINE ThreadSafeProxy& operator=(ThreadSafeProxy&& other)
	{
		if (this != &other)
		{
			m_data = other.m_data;
			m_lock = other.m_lock;
			other.m_data = nullptr;
			other.m_lock = nullptr;
		}

		return *this;
	}

	INLINE T* operator->()
	{
		return m_data;
	}

	INLINE T& operator*()
	{
		return *m_data;
	}

private:
	T* m_data = nullptr;
	LockT* m_lock = nullptr;
};

//--

// simple thread safe wrapper
template< typename T, typename LockT = SpinLock >
class ThreadSafe : public NoCopy
{
public:
	INLINE ThreadSafe() = default;
	INLINE ~ThreadSafe() = default;

	INLINE ThreadSafeProxy<T, LockT> lock()
	{
		return ThreadSafeProxy<T, LockT>(&m_data, &m_lock);
	}

private:
	LockT m_lock;
	T m_data;
};

//--

END_INFERNO_NAMESPACE()
