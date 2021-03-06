/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
*
* [# filter: math\matrix #]
***/

#pragma once

BEGIN_INFERNO_NAMESPACE()

//-

INLINE Matrix::Matrix()
{
    identity();
}

INLINE Matrix::Matrix(const Matrix33 &other, const Vector3& t /*= Vector3::ZERO()*/)
{
    m[0][0] = other.m[0][0];
    m[0][1] = other.m[0][1];
    m[0][2] = other.m[0][2];
    m[0][3] = t.x;
    m[1][0] = other.m[1][0];
    m[1][1] = other.m[1][1];
    m[1][2] = other.m[1][2];
    m[1][3] = t.y;
    m[2][0] = other.m[2][0];
    m[2][1] = other.m[2][1];
    m[2][2] = other.m[2][2];
    m[2][3] = t.z;
    m[3][0] = 0.0f;
    m[3][1] = 0.0f;
    m[3][2] = 0.0f;
    m[3][3] = 1.0f;
}

INLINE Matrix::Matrix(float m00, float m01, float m02, float m10, float m11, float m12, float m20, float m21, float m22)
{
    m[0][0] = m00;
    m[0][1] = m01;
    m[0][2] = m02;
    m[0][3] = 0.0f;
    m[1][0] = m10;
    m[1][1] = m11;
    m[1][2] = m12;
    m[1][3] = 0.0f;
    m[2][0] = m20;
    m[2][1] = m21;
    m[2][2] = m22;
    m[2][3] = 0.0f;
    m[3][0] = 0.0f;
    m[3][1] = 0.0f;
    m[3][2] = 0.0f;
    m[3][3] = 1.0f;
}

INLINE Matrix::Matrix(float m00, float m01, float m02, float m03, float m10, float m11, float m12, float m13, float m20, float m21, float m22, float m23, float m30, float m31, float m32, float m33)
{
    m[0][0] = m00;
    m[0][1] = m01;
    m[0][2] = m02;
    m[0][3] = m03;
    m[1][0] = m10;
    m[1][1] = m11;
    m[1][2] = m12;
    m[1][3] = m13;
    m[2][0] = m20;
    m[2][1] = m21;
    m[2][2] = m22;
    m[2][3] = m23;
    m[3][0] = m30;
    m[3][1] = m31;
    m[3][2] = m32;
    m[3][3] = m33;
}

INLINE Matrix::Matrix(const Vector3& x, const Vector3& y, const Vector3& z)
{
    m[0][0] = x.x;
    m[0][1] = x.y;
    m[0][2] = x.z;
    m[0][3] = 0.0f;
    m[1][0] = y.x;
    m[1][1] = y.y;
    m[1][2] = y.z;
    m[1][3] = 0.0f;
    m[2][0] = z.x;
    m[2][1] = z.y;
    m[2][2] = z.z;
    m[2][3] = 0.0f;
    m[3][0] = 0.0f;
    m[3][1] = 0.0f;
    m[3][2] = 0.0f;
    m[3][3] = 1.0f;
}

INLINE Matrix::Matrix(const Vector4& x, const Vector4& y, const Vector4& z, const Vector4& w)
{
    m[0][0] = x.x;
    m[0][1] = x.y;
    m[0][2] = x.z;
    m[0][3] = x.w;
    m[1][0] = y.x;
    m[1][1] = y.y;
    m[1][2] = y.z;
    m[1][3] = y.w;
    m[2][0] = z.x;
    m[2][1] = z.y;
    m[2][2] = z.z;
    m[2][3] = z.w;
    m[3][0] = w.x;
    m[3][1] = w.y;
    m[3][2] = w.z;
    m[3][3] = w.w;
}

INLINE Matrix::Matrix(const float* data)
{
    memcpy(this, data, sizeof(Matrix));
}

INLINE Matrix::Matrix(const double* data)
{
    auto* writePtr = (float*)this;
    const auto* endPtr = data + 16;
    while (data < endPtr)
        *writePtr++ = (float)*data++;
}

INLINE Matrix::Matrix(ETransposedFlag, const float* data)
{
    m[0][0] = *data++;
    m[1][0] = *data++;
    m[2][0] = *data++;
    m[3][0] = *data++;
    m[0][1] = *data++;
    m[1][1] = *data++;
    m[2][1] = *data++;
    m[3][1] = *data++;
    m[0][2] = *data++;
    m[1][2] = *data++;
    m[2][2] = *data++;
    m[3][2] = *data++;
    m[0][3] = *data++;
    m[1][3] = *data++;
    m[2][3] = *data++;
    m[3][3] = *data++;
}

INLINE Matrix::Matrix(ETransposedFlag, const double* data)
{
    m[0][0] = (float)*data++;
    m[1][0] = (float)*data++;
    m[2][0] = (float)*data++;
    m[3][0] = (float)*data++;
    m[0][1] = (float)*data++;
    m[1][1] = (float)*data++;
    m[2][1] = (float)*data++;
    m[3][1] = (float)*data++;
    m[0][2] = (float)*data++;
    m[1][2] = (float)*data++;
    m[2][2] = (float)*data++;
    m[3][2] = (float)*data++;
    m[0][3] = (float)*data++;
    m[1][3] = (float)*data++;
    m[2][3] = (float)*data++;
    m[3][3] = (float)*data++;
}

INLINE void Matrix::zero()
{
    memzero(&m[0][0], sizeof(m));
}

INLINE void Matrix::identity()
{
    zero();
    m[0][0] = 1.0f;
    m[1][1] = 1.0f;
    m[2][2] = 1.0f;
    m[3][3] = 1.0f;
}

INLINE Vector3 Matrix::translation() const
{
    return Vector3(m[0][3], m[1][3], m[2][3]);
}

INLINE void Matrix::translation(const Vector3 &trans)
{
    m[0][3] = trans.x;
    m[1][3] = trans.y;
    m[2][3] = trans.z;
}

INLINE void Matrix::translation(float x, float y, float z)
{
    m[0][3] = x;
    m[1][3] = y;
    m[2][3] = z;
}

INLINE Vector4 Matrix::column(int i) const
{
    DEBUG_CHECK(i >= 0 && i <= 3);
    return Vector4(m[0][i], m[1][i], m[2][i], m[3][i]);
}

INLINE Vector4& Matrix::row(int i)
{
    DEBUG_CHECK(i >= 0 && i <= 3);
    return ((Vector4*)this)[i];
}

INLINE const Vector4& Matrix::row(int i) const
{
    DEBUG_CHECK(i >= 0 && i <= 3);
    return ((const Vector4*)this)[i];
}

INLINE void Matrix::row(int i, const Vector4& v)
{
    DEBUG_CHECK(i >= 0 && i <= 3);
    ((Vector4*)this)[i] = v;
}

INLINE void Matrix::column(int i, const Vector4& v)
{
    DEBUG_CHECK(i >= 0 && i <= 3);
    m[0][i] = v.x;
    m[1][i] = v.y;
    m[2][i] = v.z;
    m[3][i] = v.w;
}

INLINE Matrix Matrix::operator~() const
{
    return inverted();
}

INLINE Matrix Matrix::operator*(const Matrix &other) const
{
    return Concat(*this, other);
}

INLINE Matrix& Matrix::operator*=(const Matrix &other)
{
    *this = Concat(*this, other);
    return *this;
}

INLINE Matrix Matrix::operator*(float other) const
{
    Matrix ret;
    ret.m[0][0] = m[0][0] * other;
    ret.m[0][1] = m[0][1] * other;
    ret.m[0][2] = m[0][2] * other;
    ret.m[0][3] = m[0][3] * other;
    ret.m[1][0] = m[1][0] * other;
    ret.m[1][1] = m[1][1] * other;
    ret.m[1][2] = m[1][2] * other;
    ret.m[1][3] = m[1][3] * other;
    ret.m[2][0] = m[2][0] * other;
    ret.m[2][1] = m[2][1] * other;
    ret.m[2][2] = m[2][2] * other;
    ret.m[2][3] = m[2][3] * other;
    ret.m[3][0] = m[3][0] * other;
    ret.m[3][1] = m[3][1] * other;
    ret.m[3][2] = m[3][2] * other;
    ret.m[3][3] = m[3][3] * other;
    return *this;
}

INLINE Matrix& Matrix::operator*=(float other)
{
    m[0][0] *= other;
    m[0][1] *= other;
    m[0][2] *= other;
    m[0][3] *= other;
    m[1][0] *= other;
    m[1][1] *= other;
    m[1][2] *= other;
    m[1][3] *= other;
    m[2][0] *= other;
    m[2][1] *= other;
    m[2][2] *= other;
    m[2][3] *= other;
    m[3][0] *= other;
    m[3][1] *= other;
    m[3][2] *= other;
    m[3][3] *= other;
    return *this;
}

INLINE bool Matrix::operator==(const Matrix& other) const
{
    return 0 == memcmp(&m, &other.m, sizeof(m));
}

INLINE bool Matrix::operator!=(const Matrix& other) const
{
    return 0 != memcmp(&m, &other.m, sizeof(m));
}

//---

INLINE void Matrix::_setTranslation(const Vector3& trans)
{
    translation(trans);
}

INLINE void Matrix::_setTranslationPerComponent(float x, float y, float z)
{
    translation(x, y, z);
}

INLINE Vector3 Matrix::_getTranslation() const
{
    return translation();
}

INLINE Vector4 Matrix::_getColumn(int i) const
{
    return column(i);
}

INLINE Vector4 Matrix::_getRow(int i) const
{
    return row(i);
}

INLINE void Matrix::_setColumn(int i, const Vector4& v)
{
    column(i, v);
}

INLINE void Matrix::_setRow(int i, const Vector4& v)
{
    row(i, v);
}

//--

END_INFERNO_NAMESPACE()
