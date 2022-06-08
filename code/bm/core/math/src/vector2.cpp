/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
*
* [# filter: math\vector2 #]
***/

#include "build.h"

BEGIN_INFERNO_NAMESPACE()

//-----

RTTI_BEGIN_TYPE_STRUCT(Vector2);
    RTTI_BIND_NATIVE_COMPARE(Vector2);
    RTTI_TYPE_TRAIT().zeroInitializationValid().noConstructor().noDestructor().fastCopyCompare();

    RTTI_PROPERTY(x).editable().scriptVisible();
    RTTI_PROPERTY(y).editable().scriptVisible();

    RTTI_NATIVE_CLASS_FUNCTION_EX("Abs", abs);
    RTTI_NATIVE_CLASS_FUNCTION_EX("Trunc", trunc);
    RTTI_NATIVE_CLASS_FUNCTION_EX("Frac", frac);
    RTTI_NATIVE_CLASS_FUNCTION_EX("Round", round);
    RTTI_NATIVE_CLASS_FUNCTION_EX("Ceil", ceil);
    RTTI_NATIVE_CLASS_FUNCTION_EX("Floor", floor);
    RTTI_NATIVE_CLASS_FUNCTION_EX("Sum", sum);
    RTTI_NATIVE_CLASS_FUNCTION_EX("Trace", trace);

    RTTI_NATIVE_CLASS_FUNCTION_EX("IsZero", isZero);
    RTTI_NATIVE_CLASS_FUNCTION_EX("IsNearZero", isNearZero);
    RTTI_NATIVE_CLASS_FUNCTION_EX("IsSimilar", isSimilar);

    RTTI_NATIVE_CLASS_FUNCTION_EX("SquareLength", squareLength);
    RTTI_NATIVE_CLASS_FUNCTION_EX("Length", length);
    RTTI_NATIVE_CLASS_FUNCTION_EX("InvLength", invLength);
    RTTI_NATIVE_CLASS_FUNCTION_EX("OneOver", oneOver);
    RTTI_NATIVE_CLASS_FUNCTION_EX("OneOverFast", oneOverFast);
    RTTI_NATIVE_CLASS_FUNCTION_EX("Normalize", normalize);
    RTTI_NATIVE_CLASS_FUNCTION_EX("Normalized", normalized);
    RTTI_NATIVE_CLASS_FUNCTION_EX("Distance", distance);
    RTTI_NATIVE_CLASS_FUNCTION_EX("SquareDistance", squareDistance);

    RTTI_NATIVE_CLASS_FUNCTION_EX("LargestAxis", largestAxis);
    RTTI_NATIVE_CLASS_FUNCTION_EX("SmallestAxis", smallestAxis);
    RTTI_NATIVE_CLASS_FUNCTION_EX("MinValue", minValue);
    RTTI_NATIVE_CLASS_FUNCTION_EX("MaxValue", maxValue);

    RTTI_NATIVE_CLASS_FUNCTION_EX("Min", min);
    RTTI_NATIVE_CLASS_FUNCTION_EX("Max", max);
    RTTI_NATIVE_CLASS_FUNCTION_EX("MinScalar", minScalar);
    RTTI_NATIVE_CLASS_FUNCTION_EX("MaxScalar", maxScalar);
    RTTI_NATIVE_CLASS_FUNCTION_EX("Clamp", clamp);
    RTTI_NATIVE_CLASS_FUNCTION_EX("Clamped", clamped);
    RTTI_NATIVE_CLASS_FUNCTION_EX("ClampScalar", clampScalar);
    RTTI_NATIVE_CLASS_FUNCTION_EX("ClampedScalar", clampedScalar);
    RTTI_NATIVE_CLASS_FUNCTION_EX("Dot", dot);
    RTTI_NATIVE_CLASS_FUNCTION_EX("Snap", snap);
    RTTI_NATIVE_CLASS_FUNCTION_EX("Snapped", snapped);
    RTTI_NATIVE_CLASS_FUNCTION_EX("NormalPart", normalPart);
    RTTI_NATIVE_CLASS_FUNCTION_EX("TangentPart", tangentPart);
    RTTI_NATIVE_CLASS_FUNCTION_EX("ClampLength", clampLength);
    RTTI_NATIVE_CLASS_FUNCTION_EX("ClampedLength", clampedLength);
    RTTI_NATIVE_CLASS_FUNCTION_EX("Lerp", lerp);

RTTI_END_TYPE();

//-----

static Vector2 Vector2_ZERO(0,0);
static Vector2 Vector2_ONE(1,1);
static Vector2 Vector2_EX(1,0);
static Vector2 Vector2_EY(0,1);
static Vector2 Vector2_INF(VERY_LARGE_FLOAT, VERY_LARGE_FLOAT);

const Vector2& Vector2::ZERO()
{
    return Vector2_ZERO;
}

const Vector2& Vector2::ONE()
{
    return Vector2_ONE;
}

const Vector2& Vector2::EX()
{
    return Vector2_EX;
}

const Vector2& Vector2::EY()
{
    return Vector2_EY;
}

const Vector2& Vector2::INF()
{
    return Vector2_INF;
}

//--

void Vector2::print(IFormatStream& f) const
{
    f.appendf("[{},{}]", x, y);
}

//--

static Vector2 opNegV2(const Vector2& a) { return -a; }
static Vector2 opPrepV2(const Vector2& a) { return ~a; }
static Vector2 opAddV2(const Vector2& a, const Vector2& b) { return a + b; }
static Vector2 opSubV2(const Vector2& a, const Vector2& b) { return a - b; }
static Vector2 opMulV2(const Vector2& a, const Vector2& b) { return a * b; }
static Vector2 opDivV2(const Vector2& a, const Vector2& b) { return a / b; }
static float opDotV2(const Vector2& a, const Vector2& b) { return a | b; }

static Vector2 opAsssignAddV2(Vector2& a, const Vector2& b) { return a += b; }
static Vector2 opAsssignSubV2(Vector2& a, const Vector2& b) { return a -= b; }
static Vector2 opAsssignMulV2(Vector2& a, const Vector2& b) { return a *= b; }
static Vector2 opAsssignDivV2(Vector2& a, const Vector2& b) { return a /= b; }

static Vector2 opAsssignAddV2F( Vector2& a, float b) { return a += b; }
static Vector2 opAsssignSubV2F(Vector2& a, float b) { return a -= b; }
static Vector2 opAsssignMulV2F(Vector2& a, float b) { return a *= b; }
static Vector2 opAsssignDivV2F(Vector2& a, float b) { return a /= b; }

static Vector2 opAddV2F(const Vector2& a, float b) { return a + b; }
static Vector2 opSubV2F(const Vector2& a, float b) { return a - b; }
static Vector2 opMulV2F(const Vector2& a, float b) { return a * b; }
static Vector2 opDivV2F(const Vector2& a, float b) { return a / b; }
static Vector2 opMulFV2(float a, const Vector2& b) { return b * a; }

RTTI_SCRIPT_GLOBAL_FUNCTION(opNegV2, "opNegate_ref_Vector2_Vector2");
RTTI_SCRIPT_GLOBAL_FUNCTION(opPrepV2, "opBinaryNot_ref_Vector2_Vector2");
RTTI_SCRIPT_GLOBAL_FUNCTION(opAddV2, "opAdd_ref_Vector2_ref_Vector2_Vector2");
RTTI_SCRIPT_GLOBAL_FUNCTION(opSubV2, "opSubtract_ref_Vector2_ref_Vector2_Vector2");
RTTI_SCRIPT_GLOBAL_FUNCTION(opMulV2, "opMultiply_ref_Vector2_ref_Vector2_Vector2");
RTTI_SCRIPT_GLOBAL_FUNCTION(opDivV2, "opDivide_ref_Vector2_ref_Vector2_Vector2");
RTTI_SCRIPT_GLOBAL_FUNCTION(opDotV2, "opBinaryOr_ref_Vector2_ref_Vector2_float");

RTTI_SCRIPT_GLOBAL_FUNCTION(opAsssignAddV2, "opAddAssign_out_Vector2_ref_Vector2_Vector2");
RTTI_SCRIPT_GLOBAL_FUNCTION(opAsssignSubV2, "opSubAssign_out_Vector2_ref_Vector2_Vector2");
RTTI_SCRIPT_GLOBAL_FUNCTION(opAsssignMulV2, "opMulAssign_out_Vector2_ref_Vector2_Vector2");
RTTI_SCRIPT_GLOBAL_FUNCTION(opAsssignDivV2, "opDivAssign_out_Vector2_ref_Vector2_Vector2");

RTTI_SCRIPT_GLOBAL_FUNCTION(opAddV2F, "opAdd_ref_Vector2_float_Vector2");
RTTI_SCRIPT_GLOBAL_FUNCTION(opSubV2F, "opSubtract_ref_Vector2_float_Vector2");
RTTI_SCRIPT_GLOBAL_FUNCTION(opMulV2F, "opMultiply_ref_Vector2_float_Vector2");
RTTI_SCRIPT_GLOBAL_FUNCTION(opDivV2F, "opDivide_ref_Vector2_float_Vector2");
RTTI_SCRIPT_GLOBAL_FUNCTION(opMulFV2, "opMultiply_float_ref_Vector2_Vector2");

RTTI_SCRIPT_GLOBAL_FUNCTION(opAsssignAddV2F, "opAddAssign_out_Vector2_float_Vector2");
RTTI_SCRIPT_GLOBAL_FUNCTION(opAsssignSubV2F, "opSubAssign_out_Vector2_float_Vector2");
RTTI_SCRIPT_GLOBAL_FUNCTION(opAsssignMulV2F, "opMulAssign_out_Vector2_float_Vector2");
RTTI_SCRIPT_GLOBAL_FUNCTION(opAsssignDivV2F, "opDivAssign_out_Vector2_float_Vector2");

//--

END_INFERNO_NAMESPACE()
