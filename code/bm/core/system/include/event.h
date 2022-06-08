/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

BEGIN_INFERNO_NAMESPACE()

//-----------------------------------------------------------------------------

/// Type of event
enum class EventType : uint8_t
{
    ManualReset,
    AutomaticReset,
};

/// Synchronization event
class BM_CORE_SYSTEM_API Event : public NoCopy
{
public:
    Event(EventType type = EventType::ManualReset);
    ~Event();

    //! Triggers the event so any waiting threads are activated
    void trigger();

    //! Resets the event to an idle state
    void reset();

    //! Waits for the event to be triggered with timeout
    bool wait(uint32_t waitTime);

    //! Waits for the event to be triggered (forever)
    void waitInfinite();

private:
    void* m_event;          // Internal handle
};

//-----------------------------------------------------------------------------

END_INFERNO_NAMESPACE()
