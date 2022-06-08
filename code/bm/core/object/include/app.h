/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

BEGIN_INFERNO_NAMESPACE()

//---

/// Application interface, used when the auto-main option is used for a project
/// Also the app is a good a singleton to expose
class BM_CORE_OBJECT_API IApplication : public MainPoolData<NoCopy>
{
    RTTI_DECLARE_VIRTUAL_ROOT_CLASS(IApplication);

public:
    IApplication();
    virtual ~IApplication();

    //--

    // is the exit requested ?
    INLINE bool exitRequested() const { return m_exitRequested; }

    //--

    // initialize application with given commandline
    virtual bool init(const CommandLine& cmdLine) = 0;

    // update application state, called in a loop until this function returns false
    virtual bool update() = 0;

    // external exit request (Ctrl+C)
    virtual void requestExit();

    //--

    // query application interface
    template< typename T >
    INLINE T* queryInterface() const
    {
        return (T*)internalQueryInterface(T::GetStaticClass());
    }

protected:
    void registerInterface(void* ptr, ClassType cls);
    void unregisterInterface(void* ptr, ClassType cls);
    void* internalQueryInterface(ClassType cls) const;

private:
    Array<void*> m_interfaces;
    std::atomic<bool> m_exitRequested;
};

//---

extern BM_CORE_OBJECT_API IApplication& App();

//--

END_INFERNO_NAMESPACE()
