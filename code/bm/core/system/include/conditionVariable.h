/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

BEGIN_INFERNO_NAMESPACE()

//-----------------------------------------------------------------------------

/// Simple condition variable
class BM_CORE_SYSTEM_API ConditionVariable : public NoCopy
{
public:
	ConditionVariable();
	~ConditionVariable();

	// Sleeps on the specified condition variable and releases the specified critical section as an atomic operation.
	bool waitInfinite(Mutex& m);

	// Sleeps on the specified condition variable and releases the specified critical section as an atomic operation.
	bool wait(Mutex& m, uint32_t ms);

	// Wakeup all threads waiting for this condition variable
	void wakeAll();

	// Wake up single thread
	void wakeOne();

private:
	union {
		void* handle;
		uint8_t data[64];
	} m_data;
};

//-----------------------------------------------------------------------------

END_INFERNO_NAMESPACE()
