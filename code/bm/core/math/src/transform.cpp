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

BEGIN_INFERNO_NAMESPACE()

RTTI_BEGIN_TYPE_STRUCT(Transform);
    RTTI_TYPE_TRAIT().noDestructor().fastCopyCompare();
    RTTI_PROPERTY(T).scriptVisible().transient();
    RTTI_PROPERTY(R).scriptVisible().transient();
    RTTI_PROPERTY(S).scriptVisible().transient();
    RTTI_BIND_NATIVE_COPY(Transform);
    RTTI_BIND_NATIVE_COMPARE(Transform);
    RTTI_BIND_NATIVE_BINARY_SERIALIZATION(Transform);
    RTTI_SCRIPT_CLASS_FUNCTION_EX("Reset", reset);
    RTTI_SCRIPT_CLASS_FUNCTION_EX("IsIdentity", checkIdentity);
    RTTI_SCRIPT_CLASS_FUNCTION_EX("Inverted", inverted);
    RTTI_SCRIPT_CLASS_FUNCTION_EX("InvertedNoScale", invertedWithNoScale);
    RTTI_SCRIPT_CLASS_FUNCTION_EX("ToMatrix", toMatrix);
    RTTI_SCRIPT_CLASS_FUNCTION_EX("ToMatrixNoScale", toMatrixNoScale);
    RTTI_SCRIPT_CLASS_FUNCTION_EX("ToMatrixInverse", toInverseMatrix);
    RTTI_SCRIPT_CLASS_FUNCTION_EX("ToMatrixInverseNoScale", toInverseMatrixNoScale);
    //RTTI_SCRIPT_CLASS_FUNCTION_EX("ToAngles", toEulerTransform);
RTTI_END_TYPE();

bool Transform::checkIdentity() const
{
    return (T == Vector3::ZERO()) && (R == Quat::IDENTITY()) && (S == Vector3::ONE());
}

Transform Transform::invertedWithNoScale() const
{
    const auto invRotation = R.inverted();
    auto invTranslation = -invRotation.toMatrix().transformPosition(T);
    return Transform(invTranslation, invRotation);
}

Transform Transform::inverted() const
{
    const auto invRotation = R.inverted();
    auto invScale = S.oneOver();
    auto invTranslation = -invRotation.toMatrix().transformPosition(T) * invScale;
    return Transform(invTranslation, invRotation, invScale);
}

Transform Transform::operator*(const Transform& baseTransform) const
{
    // x2 = R1*x1 + T1
    // x3 = R2*x2 + T2 = R2(R1*x1 + T1) + T2
    // = R2*R1*x1 + T2 + T1*R2

    return Transform(R.toMatrix().transformVector(S * baseTransform.T) + T,
        R * baseTransform.R,
        S * baseTransform.S);
}

Transform Transform::operator/(const Transform& baseTransform) const
{
    return *this * baseTransform.inverted();
}

const Transform& Transform::IDENTITY()
{
    static Transform IDENTITY_TRANSFORM;
    return IDENTITY_TRANSFORM;
}

Matrix Transform::toMatrix() const
{
    Matrix ret = R.toMatrix();
    ret.translation(T);
    ret.scaleColumns(S);
    return ret;
}

Matrix Transform::toInverseMatrix() const
{
    return inverted().toMatrix();
}

Matrix Transform::toMatrixNoScale() const
{
    Matrix ret = R.toMatrix();
    ret.translation(T);
    return ret;
}

Matrix Transform::toInverseMatrixNoScale() const
{
    return inverted().toMatrixNoScale();
}

EulerTransform Transform::toEulerTransform() const
{
    EulerTransform ret;
    ret.T = T;
    ret.S = S;
    ret.R = R.toRotator();
    return ret;
}

///----

void Transform::writeBinary(SerializationWriter& stream) const
{
    uint8_t flags = 0;

    if (T != Vector3::ZERO())
        flags |= 1;
    if (R != Quat::IDENTITY())
        flags |= 2;
    if (S != Vector3::ONE())
        flags |= 4;

    stream.writeTypedData(flags);

    if (flags & 1)
        stream.writeTypedData(T);

    if (flags & 2)
        stream.writeTypedData(R);

    if (flags & 4)
        stream.writeTypedData(S);
}

void Transform::readBinary(SerializationReader& stream)
{
    uint8_t flags = 0;
    stream.readTypedData(flags);

    if (flags & 1)
        stream.readTypedData(T);
    else
        T = Vector3::ZERO();

    if (flags & 2)
        stream.readTypedData(R);
    else
        R = Quat::IDENTITY();

    if (flags & 4)
        stream.readTypedData(S);
    else
        S = Vector3::ONE();
}

//--

static Transform opMulTT(const Transform& a, const Transform& b) { return a * b; }
static Transform opDivTT(const Transform& a, const Transform& b) { return a / b; }
//static Transform opAsssignMulTT(Transform& a, const Transform& b) { return a *= b; }

RTTI_SCRIPT_GLOBAL_FUNCTION(opMulTT, "opMultiply_ref_Transform_ref_Transform_Transform");
RTTI_SCRIPT_GLOBAL_FUNCTION(opDivTT, "opDivide_ref_Transform_ref_Transform_Transform");
//RTTI_SCRIPT_GLOBAL_FUNCTION(opAsssignMulTT, "opMulAssign_out_Transform_ref_Transform_Transform");

END_INFERNO_NAMESPACE()
