/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
*
* [# filter: math\matrix #]
***/

#include "build.h"

BEGIN_INFERNO_NAMESPACE()

//--

RTTI_BEGIN_TYPE_STRUCT(Matrix);
    RTTI_BIND_NATIVE_COMPARE(Matrix);
    RTTI_TYPE_TRAIT().noDestructor().fastCopyCompare(); // we must be constructed to identity

    RTTI_PROPERTY_VIRTUAL("x", Vector4, 0).editable().scriptVisible();
    RTTI_PROPERTY_VIRTUAL("y", Vector4, 16).editable().scriptVisible();
    RTTI_PROPERTY_VIRTUAL("z", Vector4, 32).editable().scriptVisible();
    RTTI_PROPERTY_VIRTUAL("w", Vector4, 48).editable().scriptVisible();

    RTTI_SCRIPT_CLASS_FUNCTION_EX("Zero", zero);
    RTTI_SCRIPT_CLASS_FUNCTION_EX("Identity", identity);
    RTTI_SCRIPT_CLASS_FUNCTION_EX("Translation", _getTranslation);
    RTTI_SCRIPT_CLASS_FUNCTION_EX("SetTranslation", _setTranslation);
    RTTI_SCRIPT_CLASS_FUNCTION_EX("Column", _getColumn);
    RTTI_SCRIPT_CLASS_FUNCTION_EX("SetColumn", _setColumn);
    RTTI_SCRIPT_CLASS_FUNCTION_EX("Row", _getRow);
    RTTI_SCRIPT_CLASS_FUNCTION_EX("SetRow", _setRow);
    RTTI_SCRIPT_CLASS_FUNCTION_EX("RowLengths", rowLengths);
    RTTI_SCRIPT_CLASS_FUNCTION_EX("ColumnLengths", columnLengths);

    RTTI_SCRIPT_CLASS_FUNCTION_EX("Det", det);
    RTTI_SCRIPT_CLASS_FUNCTION_EX("Det3", det3);
    RTTI_SCRIPT_CLASS_FUNCTION_EX("Transpose", transpose);
    RTTI_SCRIPT_CLASS_FUNCTION_EX("Transposed", transposed);
    RTTI_SCRIPT_CLASS_FUNCTION_EX("Invert", invert);
    RTTI_SCRIPT_CLASS_FUNCTION_EX("Inverted", inverted);

    RTTI_SCRIPT_STATIC_FUNCTION(Concat);
    RTTI_SCRIPT_STATIC_FUNCTION(BuildScale);
    RTTI_SCRIPT_STATIC_FUNCTION(BuildScaleUniform);
    RTTI_SCRIPT_STATIC_FUNCTION(BuildTranslation);
    RTTI_SCRIPT_STATIC_FUNCTION(BuildRotation);
    RTTI_SCRIPT_STATIC_FUNCTION(BuildRotationX);
    RTTI_SCRIPT_STATIC_FUNCTION(BuildRotationY);
    RTTI_SCRIPT_STATIC_FUNCTION(BuildRotationZ);
    RTTI_SCRIPT_STATIC_FUNCTION(BuildRotationXYZ);
    RTTI_SCRIPT_STATIC_FUNCTION(BuildRotationQuat);
    RTTI_SCRIPT_STATIC_FUNCTION(BuildReflection);
    RTTI_SCRIPT_STATIC_FUNCTION(BuildTranslationAnglesScale);
    RTTI_SCRIPT_STATIC_FUNCTION(BuildTranslationQuatScale);
    RTTI_SCRIPT_STATIC_FUNCTION(BuildTranslationAnglesUniformScale);
    RTTI_SCRIPT_STATIC_FUNCTION(BuildTranslationQuatUniformScale);
    RTTI_SCRIPT_STATIC_FUNCTION(BuildTranslation33);
    //RTTI_SCRIPT_STATIC_FUNCTION(BuildPerspective);
    //RTTI_SCRIPT_STATIC_FUNCTION(BuildPerspectiveFOV);
    RTTI_SCRIPT_STATIC_FUNCTION(BuildOrtho);
    //RTTI_SCRIPT_STATIC_FUNCTION(BuildPerspective);
    //RTTI_SCRIPT_STATIC_FUNCTION(BuildPerspectiveFOV);
    RTTI_SCRIPT_STATIC_FUNCTION(BuildOrtho);

    RTTI_SCRIPT_CLASS_FUNCTION_EX("ToAngles", toRotator);
    RTTI_SCRIPT_CLASS_FUNCTION_EX("ToQuat", toQuat);
    RTTI_SCRIPT_CLASS_FUNCTION_EX("ToTransform", toTransform);

    // TODO: unpack/pack into float array

    RTTI_SCRIPT_CLASS_FUNCTION_EX("TransformPoint", transformPoint);
    RTTI_SCRIPT_CLASS_FUNCTION_EX("TransformPointWithW", transformPointWithW);
    RTTI_SCRIPT_CLASS_FUNCTION_EX("TransformInvPoint", transformInvPoint);
    RTTI_SCRIPT_CLASS_FUNCTION_EX("TransformVector", transformVector);
    RTTI_SCRIPT_CLASS_FUNCTION_EX("TransformInvVector", transformInvVector);
    RTTI_SCRIPT_CLASS_FUNCTION_EX("TransformVector4", transformVector4);
    RTTI_SCRIPT_CLASS_FUNCTION_EX("TransformPosition", transformPosition);
    RTTI_SCRIPT_CLASS_FUNCTION_EX("TransformInvPosition", transformInvPosition);
    RTTI_SCRIPT_CLASS_FUNCTION_EX("TransformPlane", transformInvPosition);
    RTTI_SCRIPT_CLASS_FUNCTION_EX("TransformBox", transformBox);
RTTI_END_TYPE();

//--

Vector3 Matrix::rowLengths() const
{
#ifdef PLATFORM_PROSPERO
    return Vector3(
        std::hypot3(m[0][0], m[0][1], m[0][2]),
        std::hypot3(m[1][0], m[1][1], m[1][2]),
        std::hypot3(m[2][0], m[2][1], m[2][2])
    );
#else
    return Vector3(
            std::hypot(m[0][0], m[0][1], m[0][2]),
            std::hypot(m[1][0], m[1][1], m[1][2]),
            std::hypot(m[2][0], m[2][1], m[2][2])
            );
#endif
}

Vector3 Matrix::columnLengths() const
{
#ifdef PLATFORM_PROSPERO
    return Vector3(
        std::hypot3(m[0][0], m[1][0], m[2][0]),
        std::hypot3(m[0][1], m[1][1], m[2][1]),
        std::hypot3(m[0][2], m[1][2], m[2][2])
    );
#else
    return Vector3(
            std::hypot(m[0][0], m[1][0], m[2][0]),
            std::hypot(m[0][1], m[1][1], m[2][1]),
            std::hypot(m[0][2], m[1][2], m[2][2])
    );
#endif
}

Matrix& Matrix::scaleColumns(const Vector3 &scale)
{
    m[0][0] *= scale.x;
    m[1][0] *= scale.x;
    m[2][0] *= scale.x;
    m[0][1] *= scale.y;
    m[1][1] *= scale.y;
    m[2][1] *= scale.y;
    m[0][2] *= scale.z;
    m[1][2] *= scale.z;
    m[2][2] *= scale.z;
    return *this;
}

Matrix& Matrix::scaleRows(const Vector3 &scale)
{
    m[0][0] *= scale.x;
    m[0][1] *= scale.x;
    m[0][2] *= scale.x;
    m[1][0] *= scale.y;
    m[1][1] *= scale.y;
    m[1][2] *= scale.y;
    m[2][0] *= scale.z;
    m[2][1] *= scale.z;
    m[2][2] *= scale.z;
    return *this;
}

Matrix& Matrix::scaleInner(float scale)
{
    m[0][0] *= scale;
    m[0][1] *= scale;
    m[0][2] *= scale;
    m[1][0] *= scale;
    m[1][1] *= scale;
    m[1][2] *= scale;
    m[2][0] *= scale;
    m[2][1] *= scale;
    m[2][2] *= scale;
    return *this;
}

Matrix& Matrix::normalizeRows(Vector3* outLengths)
{
    auto  scale = rowLengths();

    if (det3() < 0.0f)
        scale.x = -scale.x;

    if (outLengths)
        *outLengths = scale;

    scale = scale.oneOver();

    m[0][0] *= scale.x;
    m[0][1] *= scale.x;
    m[0][2] *= scale.x;
    m[1][0] *= scale.y;
    m[1][1] *= scale.y;
    m[1][2] *= scale.y;
    m[2][0] *= scale.z;
    m[2][1] *= scale.z;
    m[2][2] *= scale.z;
    return *this;
}

Matrix& Matrix::normalizeColumns(Vector3* outLengths)
{
    auto scale = columnLengths();

    if (det3() < 0.0f)
        scale.x = -scale.x;

    if (outLengths)
        *outLengths = scale;

    scale = scale.oneOver();

    m[0][0] *= scale.x;
    m[1][0] *= scale.x;
    m[2][0] *= scale.x;
    m[0][1] *= scale.y;
    m[1][1] *= scale.y;
    m[2][1] *= scale.y;
    m[0][2] *= scale.z;
    m[1][2] *= scale.z;
    m[2][2] *= scale.z;
    return *this;
}

Matrix& Matrix::scaleTranslation(const Vector3 &scale)
{
    m[0][3] *= scale.x;
    m[1][3] *= scale.y;
    m[2][3] *= scale.z;
    return *this;
}

Matrix& Matrix::scaleTranslation(float scale)
{
    m[0][3] *= scale;
    m[1][3] *= scale;
    m[2][3] *= scale;
    return *this;
}

double Matrix::coFactor(int row, int col) const
{
#define M(i,j) (double)(m[(row+i)&3][(col+j)&3])
    double val = 0.0;
    val += M(1, 1) * M(2, 2) * M(3, 3);
    val += M(1, 2) * M(2, 3) * M(3, 1);
    val += M(1, 3) * M(2, 1) * M(3, 2);
    val -= M(3, 1) * M(2, 2) * M(1, 3);
    val -= M(3, 2) * M(2, 3) * M(1, 1);
    val -= M(3, 3) * M(2, 1) * M(1, 2);
    val *= ((row + col) & 1) ? -1.0f : 1.0f;
    return val;
#undef M
}

double Matrix::det() const
{
    return  m[0][0] * coFactor(0, 0) +
            m[0][1] * coFactor(0, 1) +
            m[0][2] * coFactor(0, 2) +
            m[0][3] * coFactor(0, 3);
}

double Matrix::det3() const
{
    double det = 0.0;
    det += m[0][0] * m[1][1] * m[2][2]; // 123
    det += m[0][1] * m[1][2] * m[2][0]; // 231
    det += m[0][2] * m[1][0] * m[2][1]; // 312
    det -= m[0][2] * m[1][1] * m[2][0]; // 321
    det -= m[0][1] * m[1][0] * m[2][2]; // 213
    det -= m[0][0] * m[1][2] * m[2][1]; // 132
    return det;
}

void Matrix::transpose()
{
    using std::swap;
    swap(m[1][0], m[0][1]);
    swap(m[2][0], m[0][2]);
    swap(m[3][0], m[0][3]);
    swap(m[3][1], m[1][3]);
    swap(m[3][2], m[2][3]);
    swap(m[1][2], m[2][1]);
}

Matrix Matrix::transposed() const
{
    Matrix out;
    out.m[0][0] = m[0][0];
    out.m[0][1] = m[1][0];
    out.m[0][2] = m[2][0];
    out.m[0][3] = m[3][0];
    out.m[1][0] = m[0][1];
    out.m[1][1] = m[1][1];
    out.m[1][2] = m[2][1];
    out.m[1][3] = m[3][1];
    out.m[2][0] = m[0][2];
    out.m[2][1] = m[1][2];
    out.m[2][2] = m[2][2];
    out.m[2][3] = m[3][2];
    out.m[3][0] = m[0][3];
    out.m[3][1] = m[1][3];
    out.m[3][2] = m[2][3];
    out.m[3][3] = m[3][3];
    return out;
}

void Matrix::invert()
{
    *this = inverted();
}

Matrix Matrix::inverted() const
{
    auto d = det();

    Matrix out;
    out.m[0][0] = (float)(coFactor(0, 0) / d);
    out.m[0][1] = (float)(coFactor(1, 0) / d);
    out.m[0][2] = (float)(coFactor(2, 0) / d);
    out.m[0][3] = (float)(coFactor(3, 0) / d);
    out.m[1][0] = (float)(coFactor(0, 1) / d);
    out.m[1][1] = (float)(coFactor(1, 1) / d);
    out.m[1][2] = (float)(coFactor(2, 1) / d);
    out.m[1][3] = (float)(coFactor(3, 1) / d);
    out.m[2][0] = (float)(coFactor(0, 2) / d);
    out.m[2][1] = (float)(coFactor(1, 2) / d);
    out.m[2][2] = (float)(coFactor(2, 2) / d);
    out.m[2][3] = (float)(coFactor(3, 2) / d);
    out.m[3][0] = (float)(coFactor(0, 3) / d);
    out.m[3][1] = (float)(coFactor(1, 3) / d);
    out.m[3][2] = (float)(coFactor(2, 3) / d);
    out.m[3][3] = (float)(coFactor(3, 3) / d);
    return out;
};

//--

Angles Matrix::toRotatorOthoNormal() const
{
    Angles ret;

    float xy = std::hypot(m[0][0], m[1][0]);
    if (xy > 0.01f)
    {
        ret.yaw = RAD2DEG * std::atan2(m[1][0], m[0][0]);
        ret.pitch = RAD2DEG * std::atan2(-m[2][0], xy);
        ret.roll = RAD2DEG * std::atan2(m[2][1], m[2][2]);
    }
    else
    {
        ret.roll = RAD2DEG * std::atan2(-m[1][2], m[1][1]);
        ret.pitch = RAD2DEG * std::atan2(-m[2][0], xy);
        ret.yaw = 0.0f;
    }

    return ret;
}

Angles Matrix::toRotator(Vector3* outScale) const
{
    Matrix copy(*this);
    copy.normalizeColumns(outScale);
    return copy.toRotatorOthoNormal();
}

Quat Matrix::toQuatOthoNormal() const
{
    Quat q;

    float trace = m[0][0] + m[1][1] + m[2][2];
    if (trace > 0)
    {
        float s = 0.5f / std::sqrt(trace + 1.0f);
        q.w = 0.25f / s;
        q.x = (m[2][1] - m[1][2]) * s;
        q.y = (m[0][2] - m[2][0]) * s;
        q.z = (m[1][0] - m[0][1]) * s;
    }
    else
    {
        if (m[0][0] > m[1][1] && m[0][0] > m[2][2])
        {
            float s = 2.0f * std::sqrt(1.0f + m[0][0] - m[1][1] - m[2][2]);
            q.w = (m[2][1] - m[1][2]) / s;
            q.x = 0.25f * s;
            q.y = (m[0][1] + m[1][0]) / s;
            q.z = (m[0][2] + m[2][0]) / s;
        }
        else if (m[1][1] > m[2][2])
        {
            float s = 2.0f * std::sqrt(1.0f + m[1][1] - m[0][0] - m[2][2]);
            q.w = (m[0][2] - m[2][0]) / s;
            q.x = (m[0][1] + m[1][0]) / s;
            q.y = 0.25f * s;
            q.z = (m[1][2] + m[2][1]) / s;
        }
        else
        {
            float s = 2.0f * std::sqrt(1.0f + m[2][2] - m[0][0] - m[1][1]);
            q.w = (m[1][0] - m[0][1]) / s;
            q.x = (m[0][2] + m[2][0]) / s;
            q.y = (m[1][2] + m[2][1]) / s;
            q.z = 0.25f * s;
        }
    }

    q.normalize();
    return q;
}

Quat Matrix::toQuat(Vector3* outScale) const
{
    Matrix copy(*this);
    copy.normalizeColumns(outScale);
    return copy.toQuatOthoNormal();
}

//--

Matrix Matrix::BuildScale(const Vector3 &scale)
{
    return Matrix(scale.x, 0.0f, 0.0f, 0.0f,
                    0.0f, scale.y, 0.0f, 0.0f,
                    0.0f, 0.0f, scale.z, 0.0f,
                    0.0f, 0.0f, 0.0f, 1.0);
}

Matrix Matrix::BuildScaleUniform(float scale)
{
    return Matrix(scale, 0.0f, 0.0f, 0.0f,
    0.0f, scale, 0.0f, 0.0f,
    0.0f, 0.0f, scale, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0);
}

Matrix Matrix::BuildTranslation(const Vector3 &trans)
{
    return Matrix(1.0f, 0.0f, 0.0f, trans.x,
                    0.0f, 1.0f, 0.0f, trans.y,
                    0.0f, 0.0f, 1.0f, trans.z,
                    0.0f, 0.0f, 0.0f, 1.0);
}

Matrix Matrix::BuildRotation(const Angles &rotation)
{
    return rotation.toMatrix();
}

Matrix Matrix::BuildRotationX(float degress)
{
    auto s = std::sin(DEG2RAD * degress);
    auto c = std::cos(DEG2RAD * degress);
    return Matrix(1,    0.0f, 0.0f, 0.0f,
                    0.0f, c,    s,    0.0f,
                    0.0f, -s,   c, 0.0f,
                    0.0f, 0.0f, 0.0f, 1.0);
}

Matrix Matrix::BuildRotationY(float degress)
{
    auto s = std::sin(DEG2RAD * degress);
    auto c = std::cos(DEG2RAD * degress);
    return Matrix(c,    0.0f, -s,   0.0f,
                    0.0f, 1.0f, 0.0f, 0.0f,
                    s,    0.0f, c,    0.0f,
                    0.0f, 0.0f, 0.0f, 1.0);
}

Matrix Matrix::BuildRotationZ(float degress)
{
    auto s = std::sin(DEG2RAD * degress);
    auto c = std::cos(DEG2RAD * degress);
    return Matrix(c,    s,    0.0f, 0.0f,
                    -s,   c,    0.0f, 0.0f,
                    0.0f, 0.0f, 1.0f, 0.0f,
                    0.0f, 0.0f, 0.0f, 1.0);
}

Matrix Matrix::BuildRotationXYZ(float pitch, float yaw, float roll)
{
    return Angles(pitch, yaw, roll).toMatrix();
}

Matrix Matrix::BuildRotationQuat(const Quat& rotation)
{
    return rotation.toMatrix();
}

Matrix Matrix::BuildReflection(const Plane &plane)
{
    Plane copy = plane;
    copy.normalize();

    Matrix ret;
    ret.m[0][0] = -2.0f * plane.n.x * plane.n.x + 1;
    ret.m[0][1] = -2.0f * plane.n.x * plane.n.y;
    ret.m[0][2] = -2.0f * plane.n.x * plane.n.z;
    ret.m[0][3] = -2.0f * plane.n.x * plane.d;
    ret.m[1][0] = -2.0f * plane.n.y * plane.n.x;
    ret.m[1][1] = -2.0f * plane.n.y * plane.n.y + 1;
    ret.m[1][2] = -2.0f * plane.n.y * plane.n.z;
    ret.m[1][3] = -2.0f * plane.n.y * plane.d;
    ret.m[2][0] = -2.0f * plane.n.z * plane.n.x;
    ret.m[2][1] = -2.0f * plane.n.z * plane.n.y;
    ret.m[2][2] = -2.0f * plane.n.z * plane.n.z + 1;
    ret.m[2][3] = -2.0f * plane.n.z * plane.d;
    ret.m[3][0] = 0.0f;
    ret.m[3][1] = 0.0f;
    ret.m[3][2] = 0.0f;
    ret.m[3][3] = 1.0f;
    return ret;
}

Matrix Matrix::BuildTranslationAnglesScale(const Vector3& t, const Angles& r, const Vector3& s /*= Vector3::ONE()*/)
{
    auto ret = r.toMatrix();
    ret.scaleColumns(s);
    ret.translation(t);
    return ret;
}

Matrix Matrix::BuildTranslationQuatScale(const Vector3& t, const Quat& r, const Vector3& s /*= Vector3::ONE()*/)
{
    auto ret = r.toMatrix();
    ret.scaleColumns(s);
    ret.translation(t);
    return ret;
}

Matrix Matrix::BuildTranslationAnglesUniformScale(const Vector3& t, const Angles& r, float s)
{
    auto ret = r.toMatrix();
    ret.scaleColumns(Vector3(s,s,s));
    ret.translation(t);
    return ret;
}

Matrix Matrix::BuildTranslationQuatUniformScale(const Vector3& t, const Quat& r, float s)
{
    auto ret = r.toMatrix();
    ret.scaleColumns(Vector3(s,s,s));
    ret.translation(t);
    return ret;
}

Matrix Matrix::BuildTranslation33(const Vector3& t, const Matrix33& rs)
{
    return Matrix(rs, t);
}

Matrix Matrix::BuildPerspective(float xScale, float yScale, float zNear, float zFar, float offsetX /*= 0.0f*/, float offsetY /*= 0.0f*/, bool reverseProjection /*=false*/)
{
    Matrix ret;
    ret.m[0][0] = xScale;
    ret.m[0][1] = 0.0f;
    ret.m[0][2] = offsetX; // note: get's divided by W to get the proper offset
    ret.m[0][3] = 0.0f;

    ret.m[1][0] = 0.0f;
    ret.m[1][1] = yScale;
    ret.m[1][2] = offsetY; // note: get's divided by W to get the proper offset
    ret.m[1][3] = 0.0f;

    if (reverseProjection)
    {
        auto a = -zNear / (zFar - zNear);
        auto b = zNear * zFar / (zFar - zNear);

        // z = zn
        // z' = (-zn*zn  + zn*zf) / (zf-zn) = zn(zf-zn) / (zf-zn) = zn 
        // w' = zn
        // z'/w' = zn/zn = 1

        // z = zf
        // z' = -zn*zf + zn*zf / (zf-zn) = 0

        ret.m[2][0] = 0.0f;
        ret.m[2][1] = 0.0f;
        ret.m[2][2] = a;
        ret.m[2][3] = b;
    }
    else
    {
        auto a = zFar / (zFar - zNear);
        auto b = -zNear * zFar / (zFar - zNear);

        // (zFar - zNear*zFar) / (zFar - zNear) = (zFar * (1 - zNear)) / (zFar - zNear) * zFar = (1 - zNear) / (zFar - zNear)

        ret.m[2][0] = 0.0f;
        ret.m[2][1] = 0.0f;
        ret.m[2][2] = a;
        ret.m[2][3] = b;
    }

    ret.m[3][0] = 0.0f;
    ret.m[3][1] = 0.0f;
    ret.m[3][2] = 1.0f;
    ret.m[3][3] = 0.0f;
    return ret;
}

Matrix Matrix::BuildPerspectiveFOV(float fovDeg, float aspectWidthToHeight, float zNear, float zFar, float offsetX /*= 0.0f*/, float offsetY /*= 0.0f*/, bool reverseProjection /*=false*/)
{
    float xScale = 1.0f / std::tan(DEG2RAD * (fovDeg*0.5f));
    float yScale = xScale * aspectWidthToHeight;
    return BuildPerspective(xScale, yScale, zNear, zFar, offsetX, offsetY, reverseProjection);
}

Matrix Matrix::BuildOrtho(float width, float height, float zNear, float zFar)
{
    auto a = 1.0f / (zFar - zNear);
    auto b = zNear / (zNear - zFar);

    // at z = zNear
    // n / (f-n) + -n / (f-n) = n-n / (f-n) = 0

    // at z = zFar
    // f / (f-n) - n / (f-n) = f-n / f-n = 1

    Matrix ret;
    ret.m[0][0] = 2.0f / width;
    ret.m[0][1] = 0.0f;
    ret.m[0][2] = 0.0f;
    ret.m[0][3] = 0.0f;

    ret.m[1][0] = 0.0f;
    ret.m[1][1] = 2.0f / height;
    ret.m[1][2] = 0.0f;
    ret.m[1][3] = 0.0f;

    ret.m[2][0] = 0.0f;
    ret.m[2][1] = 0.0f;
    ret.m[2][2] = a;
    ret.m[2][3] = b;

    ret.m[3][0] = 0.0f;
    ret.m[3][1] = 0.0f;
    ret.m[3][2] = 0.0f;
    ret.m[3][3] = 1.0f;
    return ret;
}

Matrix Matrix::Concat(const Matrix &a, const Matrix &b)
{
    Matrix ret;
    ret.m[0][0] = b.m[0][0] * a.m[0][0] + b.m[0][1] * a.m[1][0] + b.m[0][2] * a.m[2][0] + b.m[0][3] * a.m[3][0];
    ret.m[0][1] = b.m[0][0] * a.m[0][1] + b.m[0][1] * a.m[1][1] + b.m[0][2] * a.m[2][1] + b.m[0][3] * a.m[3][1];
    ret.m[0][2] = b.m[0][0] * a.m[0][2] + b.m[0][1] * a.m[1][2] + b.m[0][2] * a.m[2][2] + b.m[0][3] * a.m[3][2];
    ret.m[0][3] = b.m[0][0] * a.m[0][3] + b.m[0][1] * a.m[1][3] + b.m[0][2] * a.m[2][3] + b.m[0][3] * a.m[3][3];
    ret.m[1][0] = b.m[1][0] * a.m[0][0] + b.m[1][1] * a.m[1][0] + b.m[1][2] * a.m[2][0] + b.m[1][3] * a.m[3][0];
    ret.m[1][1] = b.m[1][0] * a.m[0][1] + b.m[1][1] * a.m[1][1] + b.m[1][2] * a.m[2][1] + b.m[1][3] * a.m[3][1];
    ret.m[1][2] = b.m[1][0] * a.m[0][2] + b.m[1][1] * a.m[1][2] + b.m[1][2] * a.m[2][2] + b.m[1][3] * a.m[3][2];
    ret.m[1][3] = b.m[1][0] * a.m[0][3] + b.m[1][1] * a.m[1][3] + b.m[1][2] * a.m[2][3] + b.m[1][3] * a.m[3][3];
    ret.m[2][0] = b.m[2][0] * a.m[0][0] + b.m[2][1] * a.m[1][0] + b.m[2][2] * a.m[2][0] + b.m[2][3] * a.m[3][0];
    ret.m[2][1] = b.m[2][0] * a.m[0][1] + b.m[2][1] * a.m[1][1] + b.m[2][2] * a.m[2][1] + b.m[2][3] * a.m[3][1];
    ret.m[2][2] = b.m[2][0] * a.m[0][2] + b.m[2][1] * a.m[1][2] + b.m[2][2] * a.m[2][2] + b.m[2][3] * a.m[3][2];
    ret.m[2][3] = b.m[2][0] * a.m[0][3] + b.m[2][1] * a.m[1][3] + b.m[2][2] * a.m[2][3] + b.m[2][3] * a.m[3][3];
    ret.m[3][0] = b.m[3][0] * a.m[0][0] + b.m[3][1] * a.m[1][0] + b.m[3][2] * a.m[2][0] + b.m[3][3] * a.m[3][0];
    ret.m[3][1] = b.m[3][0] * a.m[0][1] + b.m[3][1] * a.m[1][1] + b.m[3][2] * a.m[2][1] + b.m[3][3] * a.m[3][1];
    ret.m[3][2] = b.m[3][0] * a.m[0][2] + b.m[3][1] * a.m[1][2] + b.m[3][2] * a.m[2][2] + b.m[3][3] * a.m[3][2];
    ret.m[3][3] = b.m[3][0] * a.m[0][3] + b.m[3][1] * a.m[1][3] + b.m[3][2] * a.m[2][3] + b.m[3][3] * a.m[3][3];
    return ret;
}

//--

Transform Matrix::toTransform() const
{
    return Transform(translation(), toQuat(), columnLengths());
}

void Matrix::toFloats(float* outData) const
{
    memcpy(outData, this, sizeof(Matrix));
}

void Matrix::toDoubles(double* outData) const
{
    auto* endData = outData + 16;
    const auto* readData = (const float*)this;
    while (outData < endData)
        *outData++ = *readData++;
}

void Matrix::toFloatsTransposed(float* outData) const
{
    *outData++ = m[0][0];
    *outData++ = m[1][0];
    *outData++ = m[2][0];
    *outData++ = m[3][0];
    *outData++ = m[0][1];
    *outData++ = m[1][1];
    *outData++ = m[2][1];
    *outData++ = m[3][1];
    *outData++ = m[0][2];
    *outData++ = m[1][2];
    *outData++ = m[2][2];
    *outData++ = m[3][2];
    *outData++ = m[0][3];
    *outData++ = m[1][3];
    *outData++ = m[2][3];
    *outData++ = m[3][3];
}

void Matrix::toDoublesTransposed(float* outData) const
{
    *outData++ = m[0][0];
    *outData++ = m[1][0];
    *outData++ = m[2][0];
    *outData++ = m[3][0];
    *outData++ = m[0][1];
    *outData++ = m[1][1];
    *outData++ = m[2][1];
    *outData++ = m[3][1];
    *outData++ = m[0][2];
    *outData++ = m[1][2];
    *outData++ = m[2][2];
    *outData++ = m[3][2];
    *outData++ = m[0][3];
    *outData++ = m[1][3];
    *outData++ = m[2][3];
    *outData++ = m[3][3];
}

//--

Vector3 Matrix::transformPoint(const Vector3& point) const
{
    float x = point.x * m[0][0] + point.y * m[0][1] + point.z * m[0][2] + m[0][3];
    float y = point.x * m[1][0] + point.y * m[1][1] + point.z * m[1][2] + m[1][3];
    float z = point.x * m[2][0] + point.y * m[2][1] + point.z * m[2][2] + m[2][3];
    return Vector3(x, y, z);
}

Vector4 Matrix::transformPointWithW(const Vector3& point) const
{
    float x = point.x * m[0][0] + point.y * m[0][1] + point.z * m[0][2] + m[0][3];
    float y = point.x * m[1][0] + point.y * m[1][1] + point.z * m[1][2] + m[1][3];
    float z = point.x * m[2][0] + point.y * m[2][1] + point.z * m[2][2] + m[2][3];
    float w = point.x * m[3][0] + point.y * m[3][1] + point.z * m[3][2] + m[3][3];
    return Vector4(x, y, z, w);
}

Vector3 Matrix::transformInvPoint(const Vector3& point) const
{
    float px = point.x - m[0][3];
    float py = point.y - m[1][3];
    float pz = point.z - m[2][3];
    float x = px * m[0][0] + py * m[1][0] + pz * m[2][0];
    float y = px * m[0][1] + py * m[1][1] + pz * m[2][1];
    float z = px * m[0][2] + py * m[1][2] + pz * m[2][2];
    return Vector3(x, y, z);
}

Vector3 Matrix::transformVector(const Vector3& point) const
{
    float x = point.x * m[0][0] + point.y * m[0][1] + point.z * m[0][2];
    float y = point.x * m[1][0] + point.y * m[1][1] + point.z * m[1][2];
    float z = point.x * m[2][0] + point.y * m[2][1] + point.z * m[2][2];
    return Vector3(x, y, z);
}

Vector3 Matrix::transformInvVector(const Vector3& point) const
{
    float x = point.x * m[0][0] + point.y * m[1][0] + point.z * m[2][0];
    float y = point.x * m[0][1] + point.y * m[1][1] + point.z * m[2][1];
    float z = point.x * m[0][2] + point.y * m[1][2] + point.z * m[2][2];
    return Vector3(x, y, z);
}

Vector4 Matrix::transformVector4(const Vector4& point) const
{
    float x = point.x * m[0][0] + point.y * m[0][1] + point.z * m[0][2] + point.w * m[0][3];
    float y = point.x * m[1][0] + point.y * m[1][1] + point.z * m[1][2] + point.w * m[1][3];
    float z = point.x * m[2][0] + point.y * m[2][1] + point.z * m[2][2] + point.w * m[2][3];
    float w = point.x * m[3][0] + point.y * m[3][1] + point.z * m[3][2] + point.w * m[3][3];
    return Vector4(x, y, z, w);
}

ExactPosition Matrix::transformPosition(const ExactPosition& point) const
{
    double x = point.x * m[0][0] + point.y * m[0][1] + point.z * m[0][2] + m[0][3];
    double y = point.x * m[1][0] + point.y * m[1][1] + point.z * m[1][2] + m[1][3];
    double z = point.x * m[2][0] + point.y * m[2][1] + point.z * m[2][2] + m[2][3];
    return ExactPosition(x, y, z);
}

ExactPosition Matrix::transformInvPosition(const ExactPosition& point) const
{
    double px = point.x - m[0][3];
    double py = point.y - m[1][3];
    double pz = point.z - m[2][3];
    double x = px * m[0][0] + py * m[1][0] + pz * m[2][0];
    double y = px * m[0][1] + py * m[1][1] + pz * m[2][1];
    double z = px * m[0][2] + py * m[1][2] + pz * m[2][2];
    return ExactPosition(x, y, z);
}

Plane Matrix::transformPlane(const Plane& plane) const
{
    auto n = transformVector(plane.n);
    auto p = transformPoint(plane.n * -plane.d);
    n.normalize();
    return Plane(n, -(n | p));
}

Box Matrix::transformBox(const Box& box) const
{
    Box out;

    if (!box.empty())
    {
        out.min = out.max = transformPoint(box.corner(0));
        // Transform ret of the corners
        out.updateWithPoint(transformPoint(box.corner(1)));
        out.updateWithPoint(transformPoint(box.corner(2)));
        out.updateWithPoint(transformPoint(box.corner(3)));
        out.updateWithPoint(transformPoint(box.corner(4)));
        out.updateWithPoint(transformPoint(box.corner(5)));
        out.updateWithPoint(transformPoint(box.corner(6)));
        out.updateWithPoint(transformPoint(box.corner(7)));
    }

    return out;
}

//--

static Matrix IDENTITY_M(1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1);
static Matrix ZERO_M(0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0);

const Matrix& Matrix::IDENTITY()
{
    return IDENTITY_M;
}

const Matrix& Matrix::ZERO()
{
    return ZERO_M;
}

//--

void Matrix::print(IFormatStream& f) const
{
    f.appendf("X:[{},{},{}]  ", m[0][0], m[0][1], m[0][2]);
    f.appendf("Y:[{},{},{}]  ", m[1][0], m[1][1], m[1][2]);
    f.appendf("Z:[{},{},{}]  ", m[2][0], m[2][1], m[2][2]);
    f.appendf("T:[{},{},{}]  ", m[0][3], m[1][3], m[2][3]);

    f.appendf("SH:[{},{},{}]  ", row(0).xyz().length(), row(1).xyz().length(), row(2).xyz().length());
    f.appendf("SV:[{},{},{}]  ", column(0).xyz().length(), column(1).xyz().length(), column(2).xyz().length());

    const auto r = this->toRotator();
    f.appendf("R:[{},{},{}]  ", r.pitch, r.yaw, r.roll);

    f.appendf("Det:{}  Det33: {}", det(), det3());
}

//--

static Matrix opMulMVM(const Matrix& a, const Matrix& b) { return a * b; }
static Matrix opMulMVF(const Matrix& a, float b) { return a * b; }
static Matrix opAssignMulMVM(Matrix& a, const Matrix& b) { a *= b; return a; }
static Matrix opAssignMulMVF(Matrix& a, float b) { a *= b; return a; }

RTTI_SCRIPT_GLOBAL_FUNCTION(opMulMVM, "opMultiply_ref_Matrix_ref_Matrix_Matrix");
RTTI_SCRIPT_GLOBAL_FUNCTION(opMulMVF, "opMultiply_ref_Matrix_float_Matrix")
RTTI_SCRIPT_GLOBAL_FUNCTION(opAssignMulMVM, "opMulAssign_out_Matrix_ref_Matrix_Matrix")
RTTI_SCRIPT_GLOBAL_FUNCTION(opAssignMulMVF, "opMulAssign_out_Matrix_float_Matrix")

//--

END_INFERNO_NAMESPACE()
