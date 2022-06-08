/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
*
* [# filter: math\transform #]
***/

#include "build.h"
#include "bm/core/containers/include/stringParser.h"
#include "bm/core/containers/include/stringBuilder.h"
#include "bm/core/object/include/serializationWriter.h"
#include "bm/core/object/include/serializationReader.h"
#include "bm/core/system/include/format.h"

BEGIN_INFERNO_NAMESPACE()

RTTI_BEGIN_TYPE_STRUCT(EulerTransform);
    RTTI_TYPE_TRAIT().noDestructor().fastCopyCompare();
    RTTI_BIND_NATIVE_COMPARE(EulerTransform);

    RTTI_PROPERTY(T).editable().scriptVisible();
    RTTI_PROPERTY(R).editable().scriptVisible();
    RTTI_PROPERTY(S).editable().scriptVisible();

    RTTI_SCRIPT_CLASS_FUNCTION_EX("Identity", identity);
    RTTI_SCRIPT_CLASS_FUNCTION_EX("IsIdentity", isIdentity);

    RTTI_SCRIPT_CLASS_FUNCTION_EX("ToTransform", toTransform);
    RTTI_SCRIPT_CLASS_FUNCTION_EX("ToMatrix", toMatrix);
    RTTI_SCRIPT_CLASS_FUNCTION_EX("ToMatrixNoScale", toMatrixNoScale);
RTTI_END_TYPE();

void EulerTransform::identity()
{
    T = Translation::ZERO();
    R = Rotation::ZERO();
    S = Scale::ONE();
}

bool EulerTransform::isIdentity() const
{
    return (T == Translation::ZERO()) && (R == Rotation::ZERO()) && (S == Scale::ONE());
}

Transform EulerTransform::toTransform() const
{
    const auto quat = R.toQuat();
    return Transform(T, quat, S);
}

static EulerTransform IDENTITY_TRANSFORM;

const EulerTransform& EulerTransform::IDENTITY()
{
    return IDENTITY_TRANSFORM;
}

Matrix EulerTransform::toMatrix() const
{
    Matrix ret = R.toMatrix();
    ret.translation(T);
    ret.scaleColumns(S);
    return ret;
}

Matrix EulerTransform::toMatrixNoScale() const
{
    Matrix ret = R.toMatrix();
    ret.translation(T);
    return ret;
}

///----

void EulerTransform::print(IFormatStream& f) const
{
    bool hasSomething = false;

    f << "{";

    if (T != Translation::ZERO())
    {
        f.appendf("T={}", T);
        hasSomething = true;
    }

    if (R != Rotation::ZERO())
    {
        if (hasSomething)
            f << ", ";
        f.appendf("R={}", R);
        hasSomething = true;
    }

    if (S != Scale::ZERO())
    {
        if (hasSomething)
            f << ", ";
        f.appendf("S={}", S);
        hasSomething = true;
    }

    if (!hasSomething)
        f << "Identity";

    f << "}";
}

///----

/*void EulerTransform::writeBinary(SerializationWriter& stream) const
{
    uint8_t flags = 0;

    if (T != Translation::ZERO())
        flags |= 1;
    if (R != Rotation::ZERO())
        flags |= 2;
    if (S != Scale::ONE())
        flags |= 4;

    stream.writeTypedData(flags);

    if (flags & 1)
        stream.writeTypedData(T);

    if (flags & 2)
        stream.writeTypedData(R);

    if (flags & 4)
        stream.writeTypedData(S);
}

void EulerTransform::readBinary(SerializationReader& stream)
{
    uint8_t flags = 0;
    stream.readTypedData(flags);

    if (flags & 1)
        stream.readTypedData(T);
    else
        T = Translation::ZERO();

    if (flags & 2)
        stream.readTypedData(R);
    else
        R = Rotation::ZERO();

    if (flags & 4)
        stream.readTypedData(S);
    else
        S = Scale::ONE();
}*/

//--

END_INFERNO_NAMESPACE()
