/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

BEGIN_INFERNO_NAMESPACE()

//----

// simple monitoring interface that gets notified about ALL file operations being performed
// NOTE: not used in final build hence a little bit virtualized
class BM_CORE_FILE_API IFileSystemMonitoring : public IReferencable
{
public:
    virtual ~IFileSystemMonitoring();

    // TODO!    
};

//----

END_INFERNO_NAMESPACE()
