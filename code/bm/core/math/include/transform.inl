/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
*
* [# filter: math\transform #]
***/

#pragma once

BEGIN_INFERNO_NAMESPACE()

//--

INLINE Transform::Transform()
    : R(Quat::IDENTITY())
    , T(ExactPosition::ZERO())
    , S(Vector3::ONE())
{}

INLINE Transform::Transform(const ExactPosition& pos)
    : R(Quat::IDENTITY())
    , T(pos)
    , S(Vector3::ONE())
{}

INLINE Transform::Transform(const ExactPosition& pos, const Quat& rot)
    : R(rot)
    , T(pos)
    , S(Vector3::ONE())
{}

INLINE Transform::Transform(const ExactPosition& pos, const Quat& rot, const Vector3& scale)
    : R(rot)
    , T(pos)
    , S(scale)
{}

INLINE void Transform::reset()
{
    T = ExactPosition::ZERO();
    R = Quat::IDENTITY();
    S = Vector3::ONE();
}

INLINE bool Transform::operator==(const Transform& other) const
{
    return (T == other.T) && (R == other.R) && (S == other.S);
}

INLINE bool Transform::operator!=(const Transform& other) const
{
    return !operator==(other);
}

//--

END_INFERNO_NAMESPACE()
