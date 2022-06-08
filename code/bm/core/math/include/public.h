/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
*
***/

#pragma once

// STD headers
#include <math.h>

// Glue code
#include "bm_core_math_glue.inl"

// Math is global
#include "mathConstants.h" // TODO: remove!
#include "mathCommon.h"

// Math classes
#include "rotation.h"
#include "vector3.h"
#include "position.h"
#include "vector2.h"
#include "vector4.h"
#include "box.h"
#include "plane.h"
#include "matrix.h"
#include "quat.h"
#include "color.h"
#include "point.h"
#include "rect.h"
#include "range.h"
#include "float16.h"
#include "xform2D.h"
#include "transform.h"
#include "eulerTransform.h"
#include "matrix33.h"
#include "simd.h"
#include "mathRandom.h"
#include "obb.h"
#include "sphere.h"
#include "camera.h"
#include "lerp.h"

// Inlined part of math classes
#include "vector2.inl"
#include "vector3.inl"
#include "vector4.inl"
#include "color.inl"
#include "box.inl"
#include "matrix.inl"
#include "rotation.inl"
#include "quat.inl"
#include "plane.inl"
#include "point.inl"
#include "rect.inl"
#include "range.inl"
#include "xform2D.inl"
#include "transform.inl"
#include "eulerTransform.inl"
#include "matrix33.inl"
#include "position.inl"
#include "simd.inl"
#include "lerp.inl"
#include "mathCommon.inl"
#include "mathRandom.inl"

//--

BEGIN_INFERNO_NAMESPACE();

class Curve;

class UnpackedCurve;
typedef RefPtr<UnpackedCurve> UnpackedCurvePtr;

class UnpackedCurvePoint;
typedef RefPtr<UnpackedCurvePoint> UnpackedCurvePointPtr;

class ColorGradient;

END_INFERNO_NAMESPACE();

//--
