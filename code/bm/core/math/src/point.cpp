/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
*
* [# filter: math\point #]
***/

#include "build.h"

BEGIN_INFERNO_NAMESPACE()


//--

RTTI_BEGIN_TYPE_STRUCT(Point);
    RTTI_BIND_NATIVE_COMPARE(Point);
    RTTI_TYPE_TRAIT().zeroInitializationValid().noConstructor().noDestructor().fastCopyCompare();
    RTTI_PROPERTY(x).editable().scriptVisible();
    RTTI_PROPERTY(y).editable().scriptVisible();
RTTI_END_TYPE();

//--

static Point ZERO_P(0,0);

const Point& Point::ZERO()
{
    return ZERO_P;
}

//--

void Point::print(IFormatStream& f) const
{
    f.appendf("[{}, {}]", x, y);
}

//--

END_INFERNO_NAMESPACE()
