/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

BEGIN_INFERNO_NAMESPACE()

/// basic singleton interface
/// NOTE: we try to cleanup all singletons before closing the app
class BM_CORE_SYSTEM_API ISingleton : public NoCopy
{
public:
    /// called to cleanup the singleton before app shutdown
    virtual void deinit();

    /// called just before systems are closed
    virtual void deinitOnServicesClose();

    ///---

    /// deinitialize all singletons
    static void DeinitializeAll();

    /// deinitialize all singletons
    static void NotifyServicesClosing();

protected:
    ISingleton();
    virtual ~ISingleton();
};

#define DECLARE_SINGLETON(T) \
    public: static T& GetInstance() { static T* instance = new T(); return *instance; } \
    private: virtual ~T() {}

END_INFERNO_NAMESPACE()
