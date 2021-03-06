/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
*
* [# filter: event #]
***/

#include "build.h"
#include "globalEventFunction.h"
#include "globalEventDispatch.h"

BEGIN_INFERNO_NAMESPACE()
    
//--

void GlobalEventFunctionBinder::publish()
{
    if (m_entry)
        RegisterGlobalEventListener(m_entry);
}

//--

END_INFERNO_NAMESPACE()



