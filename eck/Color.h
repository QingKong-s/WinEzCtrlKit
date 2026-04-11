#pragma once
#include "Utility.h"

ECK_NAMESPACE_BEGIN
namespace Colorref
{
    inline constexpr COLORREF
        Red = 0x0000FF,			// 红色
        Green = 0x00FF00,		// 绿色
        Blue = 0xFF0000,		// 蓝色
        Yellow = 0x00FFFF,		// 黄色
        Magenta = 0xFF00FF,		// 品红/洋红
        Cyan = 0xFFFF00,		// 艳青/青色
        Aqua = Cyan,

        Maroon = 0x000080,		// 红褐/暗红
        OfficeGreen = 0x008000,	// 墨绿/暗绿
        Olive = 0x008080,		// 褐绿/暗黄
        NavyBlue = 0x800000,	// 藏青/暗蓝
        Patriarch = 0x800080,	// 紫红/暗洋红
        Teal = 0x808000,		// 深青/暗青

        Silver = 0xC0C0C0,		// 浅灰/亮灰
        MoneyGreen = 0xC0DCC0,	// 美元绿
        LightBlue = 0xF0CAA6,	// 浅蓝/天蓝

        Gray = 0x808080,		// 灰色/暗灰
        NeutralGray = 0xA4A0A0,	// 中性灰
        MilkyWhite = 0xF0FBFF,	// 乳白

        Black = 0x000000,		// 黑色
        White = 0xFFFFFF,		// 白色

        BlueGray = 0xFF8080,	// 蓝灰
        PurplishBlue = 0xE03058,// 藏蓝
        TenderGreen = 0x00E080,	// 嫩绿
        Turquoise = 0x80E000,	// 青绿
        YellowishBrown = 0x0060C0,// 黄褐
        Pink = 0xFFA8FF,		// 粉红
        BrightYellow = 0x00D8D8,// 嫩黄
        JadeWhite = 0xECECEC,	// 银白
        Purple = 0xFF0090,		// 紫色
        Azure = 0xFF8800,		// 天蓝
        Celadon = 0x80A080,		// 灰绿
        CyanBlue = 0xC06000,	// 青蓝
        Orange = 0x0080FF,		// 橙黄
        Peachblow = 0x8050FF,	// 桃红
        HibiscusRed = 0xC080FF,	// 芙红
        DeepGray = 0x606060		// 深灰
        ;
}

EckInlineNdCe UINT ReverseColorref(COLORREF cr) noexcept
{
    return BytesToInteger<UINT>(
        GetIntegerByte<2>(cr),
        GetIntegerByte<1>(cr),
        GetIntegerByte<0>(cr),
        0);
}

EckInlineNdCe ARGB ColorrefToArgb(COLORREF cr, BYTE byAlpha = 0xFF) noexcept
{
    return ReverseColorref(cr) | (byAlpha << 24);
}
EckInlineNdCe COLORREF ArgbToColorref(ARGB argb, BYTE* pbyAlpha = nullptr) noexcept
{
    if (pbyAlpha)
        *pbyAlpha = GetIntegerByte<3>(argb);
    return ReverseColorref(argb);
}

#ifdef _D2D1_H_
EckInlineNdCe D2D1_COLOR_F ArgbToD2DColorF(ARGB argb) noexcept
{
    return D2D1_COLOR_F
    {
        GetIntegerByte<2>(argb) / 255.f,
        GetIntegerByte<1>(argb) / 255.f,
        GetIntegerByte<0>(argb) / 255.f,
        GetIntegerByte<3>(argb) / 255.f
    };
}
EckInlineNdCe ARGB D2DColorFToArgb(const D2D1_COLOR_F& cr) noexcept
{
    return BytesToInteger<ARGB>(
        BYTE(cr.r * 255.f),
        BYTE(cr.g * 255.f),
        BYTE(cr.b * 255.f),
        BYTE(cr.a * 255.f));
}
EckInlineNdCe D2D1_COLOR_F RgbToD2DColorF(UINT rgb, float fAlpha = 1.f) noexcept
{
    return D2D1_COLOR_F
    {
        GetIntegerByte<2>(rgb) / 255.f,
        GetIntegerByte<1>(rgb) / 255.f,
        GetIntegerByte<0>(rgb) / 255.f,
        fAlpha
    };
}

EckInlineNdCe COLORREF D2DColorFToColorref(const D2D1_COLOR_F& cr) noexcept
{
    return BytesToInteger<COLORREF>(
        BYTE(cr.r * 255.f),
        BYTE(cr.g * 255.f),
        BYTE(cr.b * 255.f),
        0);
}
EckInlineNdCe D2D1_COLOR_F ColorrefToD2DColorF(COLORREF cr, float fAlpha = 1.f) noexcept
{
    return D2D1_COLOR_F
    {
        GetRValue(cr) / 255.f,
        GetGValue(cr) / 255.f,
        GetBValue(cr) / 255.f,
        fAlpha
    };
}
#endif// _D2D1_H_

EckInlineNdCe COLORREF ColorrefAlphaBlend(COLORREF cr, COLORREF crBK, BYTE byAlpha) noexcept
{
    return BytesToInteger<COLORREF>(
        GetIntegerByte<0>(cr) * byAlpha / 0xFF + GetIntegerByte<0>(crBK) * (0xFF - byAlpha) / 0xFF,
        GetIntegerByte<1>(cr) * byAlpha / 0xFF + GetIntegerByte<1>(crBK) * (0xFF - byAlpha) / 0xFF,
        GetIntegerByte<2>(cr) * byAlpha / 0xFF + GetIntegerByte<2>(crBK) * (0xFF - byAlpha) / 0xFF,
        0);
}
EckInlineNdCe ARGB ArgbAlphaBlend(ARGB cr, ARGB crBK) noexcept
{
    const BYTE byAlpha = GetIntegerByte<3>(cr);
    return BytesToInteger<ARGB>(
        GetIntegerByte<0>(cr) * byAlpha / 0xFF + GetIntegerByte<0>(crBK) * (0xFF - byAlpha) / 0xFF,
        GetIntegerByte<1>(cr) * byAlpha / 0xFF + GetIntegerByte<1>(crBK) * (0xFF - byAlpha) / 0xFF,
        GetIntegerByte<2>(cr) * byAlpha / 0xFF + GetIntegerByte<2>(crBK) * (0xFF - byAlpha) / 0xFF,
        GetIntegerByte<3>(cr) * byAlpha / 0xFF + GetIntegerByte<3>(crBK) * (0xFF - byAlpha) / 0xFF);
}

EckInlineNdCe BOOL IsColorLight(BYTE r, BYTE g, BYTE b) noexcept
{
    return 5 * g + 2 * r + b > 8 * 128;
}
EckInlineNdCe BOOL IsColorLightArgb(ARGB argb) noexcept
{
    return IsColorLight(
        GetIntegerByte<2>(argb),
        GetIntegerByte<1>(argb),
        GetIntegerByte<0>(argb));
}
EckInlineNdCe BOOL IsColorLightColorref(COLORREF cr) noexcept
{
    return IsColorLight(
        GetIntegerByte<0>(cr),
        GetIntegerByte<1>(cr),
        GetIntegerByte<2>(cr));
}

EckInlineNdCe BYTE GetArgbR(ARGB argb) noexcept
{
    return GetIntegerByte<2>(argb);
}
EckInlineNdCe BYTE GetArgbG(ARGB argb) noexcept
{
    return GetIntegerByte<1>(argb);
}
EckInlineNdCe BYTE GetArgbB(ARGB argb) noexcept
{
    return GetIntegerByte<0>(argb);
}
EckInlineNdCe BYTE GetArgbA(ARGB argb) noexcept
{
    return GetIntegerByte<3>(argb);
}

template<class TOut>
inline constexpr void RgbToYuv(BYTE r, BYTE g, BYTE b, TOut& y, TOut& u, TOut& v) noexcept
{
    y = TOut(0.299f * r + 0.587f * g + 0.114f * b);
    u = TOut(-0.14713f * r - 0.28886f * g + 0.436f * b);
    v = TOut(0.615f * r - 0.51499f * g - 0.10001f * b);
}

EckNfInlineNd float CalculateColorDifference(BYTE r1, BYTE g1, BYTE b1,
    BYTE r2, BYTE g2, BYTE b2) noexcept
{
    float y1, u1, v1, y2, u2, v2;
    RgbToYuv(r1, g1, b1, y1, u1, v1);
    RgbToYuv(r2, g2, b2, y2, u2, v2);
    return sqrtf((y1 - y2) * (y1 - y2) + (u1 - u2) * (u1 - u2) + (v1 - v2) * (v1 - v2));
}
EckInlineNd float CalculateColorrefDifference(COLORREF cr1, COLORREF cr2) noexcept
{
    return CalculateColorDifference(GetRValue(cr1), GetGValue(cr1), GetBValue(cr1),
        GetRValue(cr2), GetGValue(cr2), GetBValue(cr2));
}
EckInlineNd float CalculateArgbDifference(ARGB argb1, ARGB argb2) noexcept
{
    return CalculateColorDifference(GetArgbR(argb1), GetArgbG(argb1), GetArgbB(argb1),
        GetArgbR(argb2), GetArgbG(argb2), GetArgbB(argb2));
}

EckInlineNdCe COLORREF AdjustColorrefLuma(COLORREF cr, int iPrecent) noexcept
{
    return RGB(
        std::min(GetRValue(cr) * iPrecent / 100, 0xFF),
        std::min(GetGValue(cr) * iPrecent / 100, 0xFF),
        std::min(GetBValue(cr) * iPrecent / 100, 0xFF));
}
EckInlineNdCe COLORREF DeltaColorrefLuma(COLORREF cr, int d) noexcept
{
    return RGB(
        std::clamp(GetRValue(cr) + d, 0, 0xFF),
        std::clamp(GetGValue(cr) + d, 0, 0xFF),
        std::clamp(GetBValue(cr) + d, 0, 0xFF));
}
EckInlineNdCe COLORREF DeltaColorrefLuma(COLORREF cr, float d) noexcept
{
    return RGB(
        std::clamp(int(GetRValue(cr) + d * 255), 0, 0xFF),
        std::clamp(int(GetGValue(cr) + d * 255), 0, 0xFF),
        std::clamp(int(GetBValue(cr) + d * 255), 0, 0xFF));
}

EckInlineNdCe D2D1_COLOR_F LerpD2DColorF(
    const D2D1_COLOR_F& c1,
    const D2D1_COLOR_F& c2,
    float fLerp) noexcept
{
    return {
        c1.r + (c2.r - c1.r) * fLerp,
        c1.g + (c2.g - c1.g) * fLerp,
        c1.b + (c2.b - c1.b) * fLerp,
        c1.a + (c2.a - c1.a) * fLerp,
    };
}
EckInlineNdCe ARGB LerpArgb(ARGB c1, ARGB c2, float fLerp) noexcept
{
    return BytesToInteger<ARGB>(
        BYTE(GetArgbB(c1) + (GetArgbB(c2) - GetArgbB(c1)) * fLerp),
        BYTE(GetArgbG(c1) + (GetArgbG(c2) - GetArgbG(c1)) * fLerp),
        BYTE(GetArgbR(c1) + (GetArgbR(c2) - GetArgbR(c1)) * fLerp),
        BYTE(GetArgbA(c1) + (GetArgbA(c2) - GetArgbA(c1)) * fLerp));
}
ECK_NAMESPACE_END