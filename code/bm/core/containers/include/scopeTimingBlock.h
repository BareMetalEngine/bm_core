/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

#include "inplaceArray.h"

BEGIN_INFERNO_NAMESPACE()

//--

/// simple table-printing helper function that times block of code
class BM_CORE_CONTAINERS_API ScopeTimerTable : public MainPoolData<NoCopy>
{
public:
    ScopeTimerTable();

    void finishRegion(StringView name);

    void print(IFormatStream& f) const;
    void printToLog() const;

private:
    struct Entry
    {
        StringBuf name;
        NativeTimePoint start;
        NativeTimePoint finish;
    };

    InplaceArray<Entry, 32> m_entries;
    NativeTimePoint m_start;
    NativeTimePoint m_blockStart;
};

//--

END_INFERNO_NAMESPACE()
