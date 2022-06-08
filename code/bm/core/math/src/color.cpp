/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
*
* [# filter: math\color #]
***/

#include "build.h"

BEGIN_INFERNO_NAMESPACE()

//---

RTTI_BEGIN_TYPE_STRUCT(Color);
    RTTI_BIND_NATIVE_COMPARE(Color);
    RTTI_PROPERTY(r).editable().range(0, 255).widgetSlider().scriptVisible();
    RTTI_PROPERTY(g).editable().range(0, 255).widgetSlider().scriptVisible();
    RTTI_PROPERTY(b).editable().range(0, 255).widgetSlider().scriptVisible();
    RTTI_PROPERTY(a).editable().range(0, 255).widgetSlider().scriptVisible();

    RTTI_SCRIPT_CLASS_FUNCTION_EX("ReplaceAlpha", replaceAlpha);
    RTTI_SCRIPT_CLASS_FUNCTION_EX("ScaleColor", scaleColor);
    RTTI_SCRIPT_CLASS_FUNCTION_EX("ScaleAlpha", scaleAlpha);
    RTTI_SCRIPT_CLASS_FUNCTION_EX("ToNative", toNative);
    RTTI_SCRIPT_CLASS_FUNCTION_EX("ToABGR", toABGR);
    RTTI_SCRIPT_CLASS_FUNCTION_EX("ToARGB", toARGB);
    RTTI_SCRIPT_CLASS_FUNCTION_EX("ToRGBA", toRGBA);
    RTTI_SCRIPT_CLASS_FUNCTION_EX("ToBGRA", toBGRA);
    RTTI_SCRIPT_CLASS_FUNCTION_EX("ToRGB565", toRGB565);
    RTTI_SCRIPT_CLASS_FUNCTION_EX("ToBGR565", toBGR565);
    RTTI_SCRIPT_CLASS_FUNCTION_EX("ToARGB4444", toARGB4444);
    RTTI_SCRIPT_CLASS_FUNCTION_EX("ToABGR4444", toABGR4444);

    RTTI_SCRIPT_CLASS_FUNCTION_EX("ToVector", toVector);
    RTTI_SCRIPT_CLASS_FUNCTION_EX("ToVectorFromSRGB", toVectorRaw);

    RTTI_SCRIPT_CLASS_FUNCTION_EX("Luminance", luminance);
    RTTI_SCRIPT_CLASS_FUNCTION_EX("Inverted", inverted);
    RTTI_SCRIPT_CLASS_FUNCTION_EX("LuminanceRaw", luminance);
    RTTI_SCRIPT_CLASS_FUNCTION_EX("InvertedRaw", inverted);

    RTTI_SCRIPT_STATIC_FUNCTION(FromARGB);
    RTTI_SCRIPT_STATIC_FUNCTION(FromABGR);
    RTTI_SCRIPT_STATIC_FUNCTION(FromRGBA);
    RTTI_SCRIPT_STATIC_FUNCTION(FromBGRA);
    RTTI_SCRIPT_STATIC_FUNCTION(FromRGB565);
    RTTI_SCRIPT_STATIC_FUNCTION(FromBGR565);
    RTTI_SCRIPT_STATIC_FUNCTION(FromARGB4444);
    RTTI_SCRIPT_STATIC_FUNCTION(FromABGR4444);
    RTTI_SCRIPT_STATIC_FUNCTION(FromVector);
    RTTI_SCRIPT_STATIC_FUNCTION(FromVectorRaw);

RTTI_END_TYPE();

//---

Color::Color(StringView txt)
    : r(255), g(255), b(255), a(255)
{
    Parse(txt, *this);
}

//--

Color Color::operator*(float scale) const
{
    return FromVector(toVector() * scale);
}

Color& Color::operator*=(float scale)
{
    *this = FromVector(toVector() * scale);
    return *this;
}

Color Color::replaceAlpha(uint8_t newAlpha) const
{
    return Color(r, g, b, newAlpha);
}

Color Color::scaleColor(float scale) const
{
    return Color(
        FloatTo255_SRGB(FloatFrom255_SRGB(r) * scale),
        FloatTo255_SRGB(FloatFrom255_SRGB(g) * scale),
        FloatTo255_SRGB(FloatFrom255_SRGB(b) * scale),
        a);
}

Color Color::scaleColorRaw(float scale) const
{
    return Color(
        FloatTo255(FloatFrom255(r) * scale),
        FloatTo255(FloatFrom255(g) * scale),
        FloatTo255(FloatFrom255(b) * scale),
        a);
}

Color Color::scaleAlpha(float scale) const
{
    return Color(r, g, b, FloatTo255(FloatFrom255(a) * scale));
}

// https://stackoverflow.com/questions/1102692/how-to-alpha-blend-rgba-unsigned-byte-color-fast
Color Lerp256(const Color &a, const Color &b, uint32_t alpha) // alpha is from 0 to 256
{
    uint32_t invA = 0x100 - alpha;
    uint32_t rb1 = (invA * (a.toNative() & 0xFF00FF)) >> 8;
    uint32_t rb2 = (alpha * (b.toNative() & 0xFF00FF)) >> 8;
    uint32_t g1 = (invA * (a.toNative() & 0x00FF00)) >> 8;
    uint32_t g2 = (alpha * (b.toNative() & 0x00FF00)) >> 8;
    return ((rb1 | rb2) & 0xFF00FF) + ((g1 | g2) & 0x00FF00) + 0xFF000000;
}

Vector4 Color::toVectorRaw() const
{
    Vector4 ret;

#ifdef PLATFORM_SSE2
    static const __m128 alphaOne = _mm_setr_ps(0.0f, 0.0f, 0.0f, 1.0f);
    static const __m128 scalar = _mm_set_ps(1.0f / 255.0f, 1.0f / 255.0f, 1.0f / 255.0f, 1.0f / 255.0f);
    __m128i xmm0 = _mm_set1_epi32(*(const uint32_t*)this);
    xmm0 = _mm_unpacklo_epi8(xmm0, _mm_setzero_si128()); // unpack to 16 bits
    xmm0 = _mm_unpacklo_epi16(xmm0, _mm_setzero_si128());  // unpack to 32 bits
    __m128 xmm1 = _mm_cvtepi32_ps(xmm0); // convert to floats
    xmm1 = _mm_mul_ps(xmm1, scalar); // convert to 0-1 range
    //xmm1 = _mm_and_ps(xmm1, _mm_castsi128_ps(packMask)); // zero out invalid entries
    _mm_store_ps((float*)&ret, xmm1);
    return ret;
#else
    auto div = 1.0f / 255.0f;
    ret.x = r * div;
    ret.y = g * div;
    ret.z = b * div;
    ret.w = a * div;
    return ret;
#endif
}

Color Color::FromVectorRaw(const Vector4 &other)
{
    struct
    {
        Color ret;
        uint8_t padding[12];
    } data;
#ifdef PLATFORM_SSE2
    static const __m128i packMask = _mm_setr_epi8(-1, -1, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    static const __m128 scalar = _mm_set_ps(255.0f, 255.0f, 255.0f, 255.0f);
    __m128 c = _mm_mul_ps(_mm_load_ps((const float*)&other), scalar);
    __m128i ci = _mm_cvtps_epi32(c);
    __m128i packedW = _mm_packs_epi32(ci, ci); // to 16-bit signed with saturation
    __m128i packedB = _mm_packus_epi16(packedW, packedW); // to 8-bit unsigned with saturation
    _mm_maskmoveu_si128(packedB, packMask, (char*)&data);
#else
    data.ret.r = FloatTo255(other.x);
    data.ret.g = FloatTo255(other.y);
    data.ret.b = FloatTo255(other.z);
    data.ret.a = FloatTo255(other.w);
#endif
    return data.ret;
}

//--

ColorLookupTablesSRGB::ColorLookupTablesSRGB()
{
    for (int i = 0; i < 256; ++i)
    {
        float alpha = i / 255.0f;
        Uint8ToLinear[i] = alpha;
        Uint8SrgbToLinear[i] = FloatFromSRGBToLinear(alpha);
    }

    for (int i = 0; i < 65536; ++i)
    {
        float alpha = i / 65535.0f;
        Uint16SrgbToLinear[i] = FloatFromSRGBToLinear(alpha);
    }

    for (int i = 0; i <= 65536; ++i)
    {
        float alpha = i / 65536.0f;
        Uint8LinearToSrgb[i] = (uint8_t)std::clamp<float>(FloatFromLinearToSRGB(alpha) * 255.0f, 0.0f, 255.0f);
        Uint16LinearToSrgb[i] = (uint16_t)std::clamp<float>(FloatFromLinearToSRGB(alpha) * 65535.0f, 0.0f, 65535.0f);
    }
}

static ColorLookupTablesSRGB GColorLookup;

const ColorLookupTablesSRGB& ColorLookupTablesSRGB::GetInstance()
{
    return GColorLookup;
}

//--

uint8_t FloatTo255_SRGB(float linear)
{
    int index = std::clamp((int)(linear * 65536.0f), 0, 65536);
    return GColorLookup.Uint8LinearToSrgb[index];
}

float FloatFrom255_SRGB(uint8_t col)
{
    return GColorLookup.Uint8SrgbToLinear[col];
}

//--

uint16_t FloatTo65535_SRGB(float linear)
{
    int index = std::clamp((int)(linear * 65536.0f), 0, 65536);
    return GColorLookup.Uint16LinearToSrgb[index];
}

float FloatFrom65535_SRGB(uint16_t col)
{
    return GColorLookup.Uint16SrgbToLinear[col];
}

//--

Vector4 Color::toVector() const
{
    return Vector4(
        GColorLookup.Uint8SrgbToLinear[r],
        GColorLookup.Uint8SrgbToLinear[g],
        GColorLookup.Uint8SrgbToLinear[b],
        GColorLookup.Uint8ToLinear[a]);
}

StringBuf Color::toHexString(bool withAlpha /*= false*/) const
{
    if (withAlpha)
        return TempString("#{}{}{}{}", Hex(r), Hex(g), Hex(b), Hex(a));
    else
        return TempString("#{}{}{}", Hex(r), Hex(g), Hex(b));
}

float Color::luminance() const
{
    float lr = GColorLookup.Uint8SrgbToLinear[r] * 0.299f;
    float lg = GColorLookup.Uint8SrgbToLinear[g] * 0.587f;
    float lb = GColorLookup.Uint8SrgbToLinear[b] * 0.114f;
    return lr + lg + lb;
}

float Color::luminanceRaw() const
{
    const float invR = 0.299f / 255.0f;
    const float invG = 0.587f / 255.0f;
    const float invB = 0.114f / 255.0f;
    float lr = r * invR;
    float lg = g * invG;
    float lb = b * invB;
    return lr + lg + lb;
}

Color Color::inverted() const
{
    auto values = toVector();
    values.x = 1.0f - values.x;
    values.y = 1.0f - values.y;
    values.z = 1.0f - values.z;
    return Color::FromVector(values);
}

Color Color::invertedRaw() const
{
    return Color(255 - r, 255 - g, 255 - b, a);
}

void Color::print(IFormatStream& f) const
{
    f << "#" << Hex(r) << Hex(g) << Hex(b);
    if (a != 255)
        f << Hex(a);
}

Color Color::FromVector(const Vector4 &other)
{
    int ra = std::clamp((int)(other.x * 65536.0f), 0, 65536);
    int rg = std::clamp((int)(other.y * 65536.0f), 0, 65536);
    int rb = std::clamp((int)(other.z * 65536.0f), 0, 65536);

    return Color(
        GColorLookup.Uint8LinearToSrgb[ra],
        GColorLookup.Uint8LinearToSrgb[rg],
        GColorLookup.Uint8LinearToSrgb[rb],
        (uint8_t)std::clamp((int)(other.w * 255.0f), 0, 255));
}

///---

Color Color::lerp(Color target, float frac) const
{
    return FromVector(toVector().lerp(target.toVector(), frac));
}

Color Color::lerpRaw(Color target, float frac) const
{
    auto alpha = std::clamp((int)(frac * 256.0f), 0, 256);
    return Lerp256(*this, target, alpha);
}

///---

bool Color::Parse(StringView txt, Color& outColor)
{
    StringParser p(txt);

    p.parseKeyword("#");

    uint32_t length = 0;
    uint64_t value = 0;
    if (p.parseHex(value, 0, &length))
    {
        if (length == 3)
        {
            uint8_t r = (value >> 8) & 0xF;
            uint8_t g = (value >> 4) & 0xF;
            uint8_t b = (value >> 0) & 0xF;

            r = r | (r << 4);
            g = g | (g << 4);
            b = b | (b << 4);

            outColor = Color(r, g, b, 255);
            return true;
        }
        else if (length == 4)
        {
            uint8_t r = (value >> 12) & 0xF;
            uint8_t g = (value >> 8) & 0xF;
            uint8_t b = (value >> 4) & 0xF;
            uint8_t a = (value >> 0) & 0xF;

            r = r | (r << 4);
            g = g | (g << 4);
            b = b | (b << 4);
            a = a | (a << 4);

            outColor = Color(r, g, b, a);
            return true;
        }
        else if (length == 6)
        {
            uint8_t r = (value >> 16) & 0xFF;
            uint8_t g = (value >> 8) & 0xFF;
            uint8_t b = (value >> 0) & 0xFF;

            outColor = Color(r, g, b, 255);
            return true;
        }
        else if (length == 8)
        {
            uint8_t r = (value >> 24) & 0xFF;
            uint8_t g = (value >> 16) & 0xFF;
            uint8_t b = (value >> 8) & 0xFF;
            uint8_t a = (value >> 0) & 0xFF;

            outColor = Color(r, g, b, a);
            return true;
        }
    }

    return false;
}

///---

const Color::ColorVal Color::NONE = 0x00FFFFFF; // alpha zero
const Color::ColorVal Color::MAROON = 0xFF800000;
const Color::ColorVal Color::DARKRED = 0xFF8B0000;
const Color::ColorVal Color::RED = 0xFFFF0000;
const Color::ColorVal Color::LIGHTPINK = 0xFFFFB6C1;
const Color::ColorVal Color::CRIMSON = 0xFFDC143C;
const Color::ColorVal Color::PALEVIOLETRED = 0xFFDB7093;
const Color::ColorVal Color::HOTPINK = 0xFFFF69B4;
const Color::ColorVal Color::DEEPPINK = 0xFFFF1493;
const Color::ColorVal Color::MEDIUMVIOLETRED = 0xFFC71585;
const Color::ColorVal Color::PURPLE = 0xFF800080;
const Color::ColorVal Color::DARKMAGENTA = 0xFF8B008B;
const Color::ColorVal Color::ORCHID = 0xFFDA70D6;
const Color::ColorVal Color::THISTLE = 0xFFD8BFD8;
const Color::ColorVal Color::PLUM = 0xFFDDA0DD;
const Color::ColorVal Color::VIOLET = 0xFFEE82EE;
const Color::ColorVal Color::FUCHSIA = 0xFFFF00FF;
const Color::ColorVal Color::MAGENTA = 0xFFFF00FF;
const Color::ColorVal Color::MEDIUMORCHID = 0xFFBA55D3;
const Color::ColorVal Color::DARKVIOLET = 0xFF9400D3;
const Color::ColorVal Color::DARKORCHID = 0xFF9932CC;
const Color::ColorVal Color::BLUEVIOLET = 0xFF8A2BE2;
const Color::ColorVal Color::INDIGO = 0xFF4B0082;
const Color::ColorVal Color::MEDIUMPURPLE = 0xFF9370DB;
const Color::ColorVal Color::SLATEBLUE = 0xFF6A5ACD;
const Color::ColorVal Color::MEDIUMSLATEBLUE = 0xFF7B68EE;
const Color::ColorVal Color::DARKBLUE = 0xFF00008B;
const Color::ColorVal Color::MEDIUMBLUE = 0xFF0000CD;
const Color::ColorVal Color::BLUE = 0xFF0000FF;
const Color::ColorVal Color::NAVY = 0xFF000080;
const Color::ColorVal Color::MIDNIGHTBLUE = 0xFF191970;
const Color::ColorVal Color::DARKSLATEBLUE = 0xFF483D8B;
const Color::ColorVal Color::ROYALBLUE = 0xFF4169E1;
const Color::ColorVal Color::CORNFLOWERBLUE = 0xFF6495ED;
const Color::ColorVal Color::LIGHTSTEELBLUE = 0xFFB0C4DE;
const Color::ColorVal Color::ALICEBLUE = 0xFFF0F8FF;
const Color::ColorVal Color::GHOSTWHITE = 0xFFF8F8FF;
const Color::ColorVal Color::LAVENDER = 0xFFE6E6FA;
const Color::ColorVal Color::DODGERBLUE = 0xFF1E90FF;
const Color::ColorVal Color::STEELBLUE = 0xFF4682B4;
const Color::ColorVal Color::DEEPSKYBLUE = 0xFF00BFFF;
const Color::ColorVal Color::SLATEGRAY = 0xFF708090;
const Color::ColorVal Color::LIGHTSLATEGRAY = 0xFF778899;
const Color::ColorVal Color::LIGHTSKYBLUE = 0xFF87CEFA;
const Color::ColorVal Color::SKYBLUE = 0xFF87CEEB;
const Color::ColorVal Color::LIGHTBLUE = 0xFFADD8E6;
const Color::ColorVal Color::TEAL = 0xFF008080;
const Color::ColorVal Color::DARKCYAN = 0xFF008B8B;
const Color::ColorVal Color::DARKTURQUOISE = 0xFF00CED1;
const Color::ColorVal Color::CYAN = 0xFF00FFFF;
const Color::ColorVal Color::MEDIUMTURQUOISE = 0xFF48D1CC;
const Color::ColorVal Color::CADETBLUE = 0xFF5F9EA0;
const Color::ColorVal Color::PALETURQUOISE = 0xFFAFEEEE;
const Color::ColorVal Color::LIGHTCYAN = 0xFFE0FFFF;
const Color::ColorVal Color::AZURE = 0xFFF0FFFF;
const Color::ColorVal Color::LIGHTSEAGREEN = 0xFF20B2AA;
const Color::ColorVal Color::TURQUOISE = 0xFF40E0D0;
const Color::ColorVal Color::POWDERBLUE = 0xFFB0E0E6;
const Color::ColorVal Color::DARKSLATEGRAY = 0xFF2F4F4F;
const Color::ColorVal Color::AQUAMARINE = 0xFF7FFFD4;
const Color::ColorVal Color::MEDIUMSPRINGGREEN = 0xFF00FA9A;
const Color::ColorVal Color::MEDIUMAQUAMARINE = 0xFF66CDAA;
const Color::ColorVal Color::SPRINGGREEN = 0xFF00FF7F;
const Color::ColorVal Color::MEDIUMSEAGREEN = 0xFF3CB371;
const Color::ColorVal Color::SEAGREEN = 0xFF2E8B57;
const Color::ColorVal Color::LIMEGREEN = 0xFF32CD32;
const Color::ColorVal Color::DARKGREEN = 0xFF006400;
const Color::ColorVal Color::GREEN = 0xFF00FF00;
const Color::ColorVal Color::LIME = 0xFF008000;
const Color::ColorVal Color::FORESTGREEN = 0xFF228B22;
const Color::ColorVal Color::DARKSEAGREEN = 0xFF8FBC8F;
const Color::ColorVal Color::LIGHTGREEN = 0xFF90EE90;
const Color::ColorVal Color::PALEGREEN = 0xFF98FB98;
const Color::ColorVal Color::MINTCREAM = 0xFFF5FFFA;
const Color::ColorVal Color::HONEYDEW = 0xFFF0FFF0;
const Color::ColorVal Color::CHARTREUSE = 0xFF7FFF00;
const Color::ColorVal Color::LAWNGREEN = 0xFF7CFC00;
const Color::ColorVal Color::OLIVEDRAB = 0xFF6B8E23;
const Color::ColorVal Color::DARKOLIVEGREEN = 0xFF556B2F;
const Color::ColorVal Color::YELLOWGREEN = 0xFF9ACD32;
const Color::ColorVal Color::GREENYELLOW = 0xFFADFF2F;
const Color::ColorVal Color::BEIGE = 0xFFF5F5DC;
const Color::ColorVal Color::LINEN = 0xFFFAF0E6;
const Color::ColorVal Color::LIGHTGOLDENRODYELLOW = 0xFFFAFAD2;
const Color::ColorVal Color::OLIVE = 0xFF808000;
const Color::ColorVal Color::YELLOW = 0xFFFFFF00;
const Color::ColorVal Color::LIGHTYELLOW = 0xFFFFFFE0;
const Color::ColorVal Color::IVORY = 0xFFFFFFF0;
const Color::ColorVal Color::DARKKHAKI = 0xFFBDB76B;
const Color::ColorVal Color::KHAKI = 0xFFF0E68C;
const Color::ColorVal Color::PALEGOLDENROD = 0xFFEEE8AA;
const Color::ColorVal Color::WHEAT = 0xFFF5DEB3;
const Color::ColorVal Color::GOLD = 0xFFFFD700;
const Color::ColorVal Color::LEMONCHIFFON = 0xFFFFFACD;
const Color::ColorVal Color::PAPAYAWHIP = 0xFFFFEFD5;
const Color::ColorVal Color::DARKGOLDENROD = 0xFFB8860B;
const Color::ColorVal Color::GOLDENROD = 0xFFDAA520;
const Color::ColorVal Color::ANTIQUEWHITE = 0xFFFAEBD7;
const Color::ColorVal Color::CORNSILK = 0xFFFFF8DC;
const Color::ColorVal Color::OLDLACE = 0xFFFDF5E6;
const Color::ColorVal Color::MOCCASIN = 0xFFFFE4B5;
const Color::ColorVal Color::NAVAJOWHITE = 0xFFFFDEAD;
const Color::ColorVal Color::ORANGE = 0xFFFFA500;
const Color::ColorVal Color::BISQUE = 0xFFFFE4C4;
const Color::ColorVal Color::TAN = 0xFFD2B48C;
const Color::ColorVal Color::DARKORANGE = 0xFFFF8C00;
const Color::ColorVal Color::BURLYWOOD = 0xFFDEB887;
const Color::ColorVal Color::SADDLEBROWN = 0xFF8B4513;
const Color::ColorVal Color::SANDYBROWN = 0xFFF4A460;
const Color::ColorVal Color::BLANCHEDALMOND = 0xFFFFEBCD;
const Color::ColorVal Color::LAVENDERBLUSH = 0xFFFFF0F5;
const Color::ColorVal Color::SEASHELL = 0xFFFFF5EE;
const Color::ColorVal Color::FLORALWHITE = 0xFFFFFAF0;
const Color::ColorVal Color::SNOW = 0xFFFFFAFA;
const Color::ColorVal Color::PERU = 0xFFCD853F;
const Color::ColorVal Color::PEACHPUFF = 0xFFFFDAB9;
const Color::ColorVal Color::CHOCOLATE = 0xFFD2691E;
const Color::ColorVal Color::SIENNA = 0xFFA0522D;
const Color::ColorVal Color::LIGHTSALMON = 0xFFFFA07A;
const Color::ColorVal Color::CORAL = 0xFFFF7F50;
const Color::ColorVal Color::DARKSALMON = 0xFFE9967A;
const Color::ColorVal Color::MISTYROSE = 0xFFFFE4E1;
const Color::ColorVal Color::ORANGERED = 0xFFFF4500;
const Color::ColorVal Color::SALMON = 0xFFFA8072;
const Color::ColorVal Color::TOMATO = 0xFFFF6347;
const Color::ColorVal Color::ROSYBROWN = 0xFFBC8F8F;
const Color::ColorVal Color::PINK = 0xFFFFC0CB;
const Color::ColorVal Color::INDIANRED = 0xFFCD5C5C;
const Color::ColorVal Color::LIGHTCORAL = 0xFFF08080;
const Color::ColorVal Color::BROWN = 0xFFA52A2A;
const Color::ColorVal Color::FIREBRICK = 0xFFB22222;
const Color::ColorVal Color::BLACK = 0xFF000000;
const Color::ColorVal Color::DIMGRAY = 0xFF696969;
const Color::ColorVal Color::GRAY = 0xFF808080;
const Color::ColorVal Color::DARKGRAY = 0xFFA9A9A9;
const Color::ColorVal Color::SILVER = 0xFFC0C0C0;
const Color::ColorVal Color::LIGHTGREY = 0xFFD3D3D3;
const Color::ColorVal Color::GAINSBORO = 0xFFDCDCDC;
const Color::ColorVal Color::WHITESMOKE = 0xFFF5F5F5;
const Color::ColorVal Color::WHITE = 0xFFFFFFFF;
const Color::ColorVal Color::GREY       = 0xff888888;
const Color::ColorVal Color::GREY25 = 0xff404040;
const Color::ColorVal Color::GREY50 = 0xff808080;
const Color::ColorVal Color::GREY75 = 0xffc0c0c0;

const Color::ColorVal Color::TROVE0 = Color("#51574A").toABGR();
const Color::ColorVal Color::TROVE1 = Color("#447C69").toABGR();
const Color::ColorVal Color::TROVE2 = Color("#74C493").toABGR();
const Color::ColorVal Color::TROVE3 = Color("#8E8C6D").toABGR();
const Color::ColorVal Color::TROVE4 = Color("#E4BF80").toABGR();
const Color::ColorVal Color::TROVE5 = Color("#E9D78E").toABGR();
const Color::ColorVal Color::TROVE6 = Color("#E2975D").toABGR();
const Color::ColorVal Color::TROVE7 = Color("#F19670").toABGR();
const Color::ColorVal Color::TROVE8 = Color("#E16552").toABGR();
const Color::ColorVal Color::TROVE9 = Color("#C94A53").toABGR();
const Color::ColorVal Color::TROVE10 = Color("#BE5168").toABGR();
const Color::ColorVal Color::TROVE11 = Color("#A34974").toABGR();
const Color::ColorVal Color::TROVE12 = Color("#993767").toABGR();
const Color::ColorVal Color::TROVE13 = Color("#65387D").toABGR();
const Color::ColorVal Color::TROVE14 = Color("#4E2472").toABGR();
const Color::ColorVal Color::TROVE15 = Color("#9163B6").toABGR();
const Color::ColorVal Color::TROVE16 = Color("#E279A3").toABGR();
const Color::ColorVal Color::TROVE17 = Color("#E0598B").toABGR();
const Color::ColorVal Color::TROVE18 = Color("#7C9FB0").toABGR();
const Color::ColorVal Color::TROVE19 = Color("#5698C4").toABGR();
const Color::ColorVal Color::TROVE20 = Color("#9ABF88").toABGR();

///---

END_INFERNO_NAMESPACE()
