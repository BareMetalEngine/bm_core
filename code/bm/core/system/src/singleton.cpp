/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "singleton.h"
#include "mutex.h"
#include "scopeLock.h"

BEGIN_INFERNO_NAMESPACE()

namespace prv
{

    /// the registry of all ISingleton implementations
    class SigletonRegistry : public NoCopy
    {
    public:
        SigletonRegistry()
        {}

        void registerInstance(ISingleton* singleton)
        {
            ScopeLock<Mutex> lock(m_lock);

            if (m_numSingletons < MAX_SINGLETONS)
                m_singletons[m_numSingletons++] = singleton;
        }

        void deinit()
        {
            ScopeLock<Mutex> lock(m_lock);

            for (int i = (int)(m_numSingletons - 1); i >= 0; --i)
            {
                auto singleton  = m_singletons[i];
                singleton->deinit();
            }
        }

        void deinitOnServicesClose()
        {
            ScopeLock<Mutex> lock(m_lock);

            for (int i = (int)(m_numSingletons - 1); i >= 0; --i)
            {
                auto singleton = m_singletons[i];
                singleton->deinitOnServicesClose();
            }
        }

        static SigletonRegistry& GetInstance()
        {
            static SigletonRegistry theInstance;
            return theInstance;
        }

    private:
        static const uint32_t MAX_SINGLETONS = 256;

        Mutex m_lock;
        ISingleton* m_singletons[MAX_SINGLETONS];
        uint32_t m_numSingletons;
    };

} // prv

//--

ISingleton::ISingleton()
{
    prv::SigletonRegistry::GetInstance().registerInstance(this);
}

ISingleton::~ISingleton()
{
    FATAL_ERROR("Calling destructor on ISingleton is not legal");
}

void ISingleton::deinit()
{
}

void ISingleton::deinitOnServicesClose()
{
}

void ISingleton::DeinitializeAll()
{
    prv::SigletonRegistry::GetInstance().deinit();
}

void ISingleton::NotifyServicesClosing()
{
    prv::SigletonRegistry::GetInstance().deinitOnServicesClose();
}

//--

END_INFERNO_NAMESPACE()
