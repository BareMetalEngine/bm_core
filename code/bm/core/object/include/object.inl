/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
*
* [#filter: object #]
***/

#pragma once

BEGIN_INFERNO_NAMESPACE()

template< class T >
INLINE bool IObject::is() const
{
    return is(T::GetStaticClass());
}

template< typename T >
INLINE bool IObject::is(SpecificClassType<T> objectClass) const
{
    return is(objectClass.ptr());
}

END_INFERNO_NAMESPACE()
