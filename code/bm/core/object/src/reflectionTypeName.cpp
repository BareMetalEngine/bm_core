/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "reflectionTypeName.h"
#include "bm/core/system/include/mutex.h"
#include "bm/core/system/include/scopeLock.h"
#include "bm/core/containers/include/hashMap.h"
#include "rttiVariant.h"

BEGIN_INFERNO_NAMESPACE()

//--

/*
R TTI_BEGIN_TYPE_ENUM(WindowCursorType)
    RTTI_ENUM_OPTION(Hidden);
    RTTI_ENUM_OPTION(Default);
    RTTI_ENUM_OPTION(Cross);
    RTTI_ENUM_OPTION(Hand);
    RTTI_ENUM_OPTION(Help);
    RTTI_ENUM_OPTION(TextBeam);
    RTTI_ENUM_OPTION(No);
    RTTI_ENUM_OPTION(SizeAll);
    RTTI_ENUM_OPTION(SizeNS);
    RTTI_ENUM_OPTION(SizeWE);
    RTTI_ENUM_OPTION(SizeNESW);
    RTTI_ENUM_OPTION(SizeNWSE);
    RTTI_ENUM_OPTION(UpArrow);
    RTTI_ENUM_OPTION(Wait);
RTTI_END_TYPE();

RTT I_BEGIN_TYPE_ENUM(WindowAreaType);
    RTTI_ENUM_OPTION(NotInWindow);
    RTTI_ENUM_OPTION(Client);
    RTTI_ENUM_OPTION(NonSizableBorder);
    RTTI_ENUM_OPTION(BorderBottom);
    RTTI_ENUM_OPTION(BorderBottomLeft);
    RTTI_ENUM_OPTION(BorderBottomRight);
    RTTI_ENUM_OPTION(BorderTop);
    RTTI_ENUM_OPTION(BorderTopLeft);
    RTTI_ENUM_OPTION(BorderTopRight);
    RTTI_ENUM_OPTION(BorderLeft);
    RTTI_ENUM_OPTION(BorderRight);
    RTTI_ENUM_OPTION(Caption);
    RTTI_ENUM_OPTION(Close);
    RTTI_ENUM_OPTION(SizeBox);
    RTTI_ENUM_OPTION(Help);
    RTTI_ENUM_OPTION(HorizontalScroll);
    RTTI_ENUM_OPTION(VerticalScroll);
    RTTI_ENUM_OPTION(Menu);
    RTTI_ENUM_OPTION(Minimize);
    RTTI_ENUM_OPTION(Maximize);
    RTTI_ENUM_OPTION(SysMenu);
RTTI_END_TYPE();
*/

//--

END_INFERNO_NAMESPACE()


