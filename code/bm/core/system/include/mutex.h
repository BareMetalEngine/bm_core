/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

BEGIN_INFERNO_NAMESPACE()

//-----------------------------------------------------------------------------

class ConditionVariable;

/// Simple critical section
class BM_CORE_SYSTEM_API Mutex : public NoCopy
{
public:
	Mutex();
	~Mutex();

	//! Lock section
	void acquire();

	//! Releases the lock on the critical section
	void release();

	//! Set spin count for critical section
	void spinCount(uint32_t spinCount);

private:
	union {
		void* handle;
		uint8_t data[64];
	} m_data;

	friend class ConditionVariable;
};

//-----------------------------------------------------------------------------

END_INFERNO_NAMESPACE()
