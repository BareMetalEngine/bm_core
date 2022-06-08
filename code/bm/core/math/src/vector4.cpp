/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
*
* [# filter: math\vector4 #]
***/

#include "build.h"

BEGIN_INFERNO_NAMESPACE()

//-----------------------------------------------------------------------------

RTTI_BEGIN_TYPE_STRUCT(Vector4);
    RTTI_BIND_NATIVE_COMPARE(Vector4);
    RTTI_TYPE_TRAIT().zeroInitializationValid().noConstructor().noDestructor().fastCopyCompare();

    RTTI_PROPERTY(x).editable().scriptVisible();
    RTTI_PROPERTY(y).editable().scriptVisible();
    RTTI_PROPERTY(z).editable().scriptVisible();
    RTTI_PROPERTY(w).editable().scriptVisible();

    RTTI_NATIVE_CLASS_FUNCTION_EX("Abs",abs);
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
    RTTI_NATIVE_CLASS_FUNCTION_EX("Normalize", normalize);
    RTTI_NATIVE_CLASS_FUNCTION_EX("Normalized", normalized);
    RTTI_NATIVE_CLASS_FUNCTION_EX("Distance", distance);
    RTTI_NATIVE_CLASS_FUNCTION_EX("SquareDistance", squareDistance);
    RTTI_NATIVE_CLASS_FUNCTION_EX("OneOver", oneOver);
    RTTI_NATIVE_CLASS_FUNCTION_EX("OneOverFast", oneOverFast);

    RTTI_NATIVE_CLASS_FUNCTION_EX("Project", project);
    RTTI_NATIVE_CLASS_FUNCTION_EX("Projected", projected);
    RTTI_NATIVE_CLASS_FUNCTION_EX("ProjectFast", projectFast);
    RTTI_NATIVE_CLASS_FUNCTION_EX("ProjectedFast", projectedFast);

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
    RTTI_NATIVE_CLASS_FUNCTION_EX("Lerp", lerp);

RTTI_END_TYPE();

//--

static Vector4 ZERO_V4(0,0,0,0);
static Vector4 ZEROH_V4(0,0,0,1);
static Vector4 ONE_V4(1,1,1,1);
static Vector4 EX_V4(1,0,0,0);
static Vector4 EY_V4(0,1,0,0);
static Vector4 EZ_V4(0,0,1,0);
static Vector4 EW_V4(0,0,0,1);
static Vector4 INF_V4(VERY_LARGE_FLOAT, VERY_LARGE_FLOAT, VERY_LARGE_FLOAT, VERY_LARGE_FLOAT);

const Vector4& Vector4::ZERO() { return ZERO_V4; }
const Vector4& Vector4::ZEROH() { return ZEROH_V4; }
const Vector4& Vector4::ONE() { return ONE_V4; }
const Vector4& Vector4::EX() { return EX_V4; }
const Vector4& Vector4::EY() { return EY_V4; }
const Vector4& Vector4::EZ() { return EZ_V4; }
const Vector4& Vector4::EW() { return EW_V4; }
const Vector4& Vector4::INF() { return INF_V4; }

//--

void Vector4::print(IFormatStream& f) const
{
    f.appendf("[{},{},{},{}]", x, y, z, w);
}

//--

static Vector4 opNegV4(const Vector4& a) { return -a; }
static Vector4 opAddV4(const Vector4& a, const Vector4& b) { return a + b; }
static Vector4 opSubV4(const Vector4& a, const Vector4& b) { return a - b; }
static Vector4 opMulV4(const Vector4& a, const Vector4& b) { return a * b; }
static Vector4 opDivV4(const Vector4& a, const Vector4& b) { return a / b; }
static float opDotV4(const Vector4& a, const Vector4& b) { return a | b; }

static Vector4 opAsssignAddV4(Vector4& a, const Vector4& b) { return a += b; }
static Vector4 opAsssignSubV4(Vector4& a, const Vector4& b) { return a -= b; }
static Vector4 opAsssignMulV4(Vector4& a, const Vector4& b) { return a *= b; }
static Vector4 opAsssignDivV4(Vector4& a, const Vector4& b) { return a /= b; }

static Vector4 opAsssignAddV4F( Vector4& a, float b) { return a += b; }
static Vector4 opAsssignSubV4F(Vector4& a, float b) { return a -= b; }
static Vector4 opAsssignMulV4F(Vector4& a, float b) { return a *= b; }
static Vector4 opAsssignDivV4F(Vector4& a, float b) { return a /= b; }

static Vector4 opAddV4F(const Vector4& a, float b) { return a + b; }
static Vector4 opSubV4F(const Vector4& a, float b) { return a - b; }
static Vector4 opMulV4F(const Vector4& a, float b) { return a * b; }
static Vector4 opDivV4F(const Vector4& a, float b) { return a / b; }
static Vector4 opMulFV4(float a, const Vector4& b) { return b * a; }

RTTI_SCRIPT_GLOBAL_FUNCTION(opNegV4, "opNegate_ref_Vector4_Vector4");
RTTI_SCRIPT_GLOBAL_FUNCTION(opAddV4, "opAdd_ref_Vector4_ref_Vector4_Vector4");
RTTI_SCRIPT_GLOBAL_FUNCTION(opSubV4, "opSubtract_ref_Vector4_ref_Vector4_Vector4");
RTTI_SCRIPT_GLOBAL_FUNCTION(opMulV4, "opMultiply_ref_Vector4_ref_Vector4_Vector4");
RTTI_SCRIPT_GLOBAL_FUNCTION(opDivV4, "opDivide_ref_Vector4_ref_Vector4_Vector4");
RTTI_SCRIPT_GLOBAL_FUNCTION(opDotV4, "opBinaryOr_ref_Vector4_ref_Vector4_float");
RTTI_SCRIPT_GLOBAL_FUNCTION(opAsssignAddV4, "opAddAssign_out_Vector4_ref_Vector4_Vector4");
RTTI_SCRIPT_GLOBAL_FUNCTION(opAsssignSubV4, "opSubAssign_out_Vector4_ref_Vector4_Vector4");
RTTI_SCRIPT_GLOBAL_FUNCTION(opAsssignMulV4, "opMulAssign_out_Vector4_ref_Vector4_Vector4");
RTTI_SCRIPT_GLOBAL_FUNCTION(opAsssignDivV4, "opDivAssign_out_Vector4_ref_Vector4_Vector4");
RTTI_SCRIPT_GLOBAL_FUNCTION(opAddV4F, "opAdd_ref_Vector4_float_Vector4");
RTTI_SCRIPT_GLOBAL_FUNCTION(opSubV4F, "opSubtract_ref_Vector4_float_Vector4");
RTTI_SCRIPT_GLOBAL_FUNCTION(opMulV4F, "opMultiply_ref_Vector4_float_Vector4");
RTTI_SCRIPT_GLOBAL_FUNCTION(opDivV4F, "opDivide_ref_Vector4_float_Vector4");
RTTI_SCRIPT_GLOBAL_FUNCTION(opMulFV4, "opMultiply_float_ref_Vector4_Vector4");
RTTI_SCRIPT_GLOBAL_FUNCTION(opAsssignAddV4F, "opAddAssign_out_Vector4_float_Vector4");
RTTI_SCRIPT_GLOBAL_FUNCTION(opAsssignSubV4F, "opSubAssign_out_Vector4_float_Vector4");
RTTI_SCRIPT_GLOBAL_FUNCTION(opAsssignMulV4F, "opMulAssign_out_Vector4_float_Vector4");
RTTI_SCRIPT_GLOBAL_FUNCTION(opAsssignDivV4F, "opDivAssign_out_Vector4_float_Vector4");

//--

END_INFERNO_NAMESPACE()
