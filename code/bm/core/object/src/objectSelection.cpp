/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
*
* [# filter: selection #]
***/

#include "build.h"
#include "objectSelection.h"
#include "bm/core/containers/include/stringBuilder.h"

BEGIN_INFERNO_NAMESPACE()

//---

void Selectable::print(IFormatStream& f) const
{
    if (valid())
    {
        f.appendf("ObjectID={}", m_objectID);

        if (m_subObjectID != 0)
            f.appendf(",SubObjectID={}", m_subObjectID);
    }
    else
    {
        f << "None";
    }
}

//---

void EncodedSelectable::print(IFormatStream& f) const
{
    if (valid())
    {
        f << object;
        f.appendf(" at [{},{}], depth {<0.6>}", x, y, depth);
    }
    else
    {
        f << "None";
    }
}

//---

END_INFERNO_NAMESPACE()
