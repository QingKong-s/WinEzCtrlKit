#pragma once
#include "RectTraits.h"

ECK_NAMESPACE_BEGIN
#pragma region 地址
/// <summary>
/// 计算下一对齐边界
/// </summary>
/// <param name="pStart">起始地址</param>
/// <param name="pCurr">当前地址</param>
/// <param name="cbAlign">对齐尺寸</param>
/// <returns>当前地址到下一对齐边界的距离，如果当前地址已经落在对齐边界上，则返回0</returns>
EckInlineNdCe SIZE_T CalculateNextAlignmentBoundaryDistance(
    const void* pStart, const void* pCurr, SIZE_T cbAlign) noexcept
{
    SIZE_T uDistance = (SIZE_T)pCurr - (SIZE_T)pStart;
    return (((uDistance - 1u) / cbAlign + 1u) * cbAlign - uDistance);
}

/// <summary>
/// 步进到下一对齐边界
/// </summary>
/// <param name="pStart">起始指针</param>
/// <param name="pCurr">当前指针</param>
/// <param name="cbAlign">对齐尺寸</param>
/// <returns>步进后的指针，如果当前指针已经落在对齐边界上，则指针不变</returns>
template<class T>
EckInlineNdCe T* StepToNextAlignmentBoundary(T* pStart, T* pCurr, size_t cbAlign) noexcept
{
    return (T*)((BYTE*)pCurr + CalculateNextAlignmentBoundaryDistance(pStart, pCurr, cbAlign));
}

EckInlineNdCe void* PointerSkipType(auto* p) noexcept { return (void*)((BYTE*)p + sizeof(*p)); }
EckInlineNdCe const void* PointerSkipType(const auto* p) noexcept { return (const void*)((PCBYTE)p + sizeof(*p)); }

// 计算对齐后内存尺寸
EckInlineNdCe size_t AlignedSize(size_t cbSize, size_t cbAlign) noexcept
{
    if (cbSize / cbAlign * cbAlign == cbSize)
        return cbSize;
    else
        return (cbSize / cbAlign + 1) * cbAlign;
}

template<class T>
EckInlineNdCe T* PointerStepBytes(T* p, SSIZE_T d) noexcept
{
    return (T*)((BYTE*)p + d);
}
#pragma endregion 地址

#pragma region 位
template<std::integral T>
EckInlineNdCe BYTE GetIntegerByte(T i, int idxByte) noexcept
{
    EckAssert(idxByte >= 0 && idxByte < sizeof(T));
    return (BYTE)((i >> (idxByte * 8)) & 0b11111111);
}

template<std::integral T>
EckInlineCe void SetIntegerByte(T& i, int idxByte, BYTE by) noexcept
{
    EckAssert(idxByte >= 0 && idxByte < sizeof(T));
    i &= ((~(T)0b11111111) << (idxByte * 8));
}

template<int N, std::integral T>
EckInlineNdCe BYTE GetIntegerByte(T i) noexcept
{
    static_assert(N >= 0 && N < sizeof(T));
    return (BYTE)((i >> (N * 8)) & 0b11111111);
}

template<int N, std::integral T>
EckInlineCe void SetIntegerByte(T& i, BYTE by) noexcept
{
    static_assert(N >= 0 && N < sizeof(T));
    i &= ((~(T)0b11111111) << (N * 8));
}

/// <summary>
/// 字节组到整数
/// </summary>
/// <typeparam name="TRet"></typeparam>
/// <typeparam name="...T"></typeparam>
/// <param name="...by">字节组，个数必须等于sizeof(TRet)</param>
/// <returns>整数</returns>
template<std::integral TRet, class... T>
EckInlineNdCe TRet BytesToInteger(T... by) noexcept
{
    static_assert(sizeof...(T) == sizeof(TRet));
    int i = 0;
    auto fn = [&i](BYTE by)
        {
            ++i;
            return by << (8 * (i - 1));
        };
    return (TRet)(... | fn((BYTE)by));
}

/// <summary>
/// 反转整数字节序
/// </summary>
/// <param name="i">输入</param>
/// <returns>转换结果</returns>
template<std::integral T>
EckInlineNdCe T ReverseInteger(T i) noexcept
{
    if constexpr (sizeof(T) == 8)
        return (T)_byteswap_uint64((UINT64)i);
    else if constexpr (sizeof(T) == 4)
        return (T)_byteswap_ulong((ULONG)i);
    else if constexpr (sizeof(T) == 2)
        return (T)_byteswap_ushort((USHORT)i);
    else
        return i;
}

EckInlineCe void ReverseByteOrder(BYTE* p, size_t cb) noexcept
{
    std::reverse(p, p + cb);
}
template<std::integral T>
EckInlineNdCe T ReverseByteOrder(T& i) noexcept
{
    auto p = (BYTE*)&i;
    std::reverse(p, p + sizeof(T));
    return i;
}

EckInlineNdCe BOOL IsBitSet(auto dw1, auto dw2) noexcept
{
    return (dw1 & dw2) == dw2;
}

template<std::integral T>
EckInlineNdCe T GetLowNBits(T x, size_t n) noexcept
{
    return x & ((T{ 1 } << n) - T{ 1 });
}
template<std::integral T>
EckInlineNdCe T GetHighNBits(T x, size_t n) noexcept
{
    return (x >> n) & ((T{ 1 } << n) - T{ 1 });
}

template<std::integral T>
EckInlineNdCe T ClearLowNBits(T x, size_t n) noexcept
{
    return x & ~((T{ 1 } << n) - T{ 1 });
}
template<std::integral T>
EckInlineNdCe T ClearHighNBits(T x, size_t n) noexcept
{
    return x & ((T{ 1 } << (sizeof(T) * 8 - n)) - T{ 1 });
}
#pragma endregion 位

#pragma region 颜色
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
#pragma endregion 颜色

#pragma region 图形结构
EckInlineNdCe BOOL EquRect(const RECT& rc1, const RECT& rc2) noexcept
{
    return rc1.left == rc2.left && rc1.top == rc2.top &&
        rc1.right == rc2.right && rc1.bottom == rc2.bottom;
}

EckNfInlineNdCe RECT MakeRect(POINT pt1, POINT pt2) noexcept
{
    RECT rc;
    if (pt1.x >= pt2.x)
        rc.left = pt2.x, rc.right = pt1.x;
    else
        rc.left = pt1.x, rc.right = pt2.x;

    if (pt1.y >= pt2.y)
        rc.top = pt2.y, rc.bottom = pt1.y;
    else
        rc.top = pt1.y, rc.bottom = pt2.y;
    return rc;
}

EckInlineNdCe BOOL PtInRect(const RECT& rc, POINT pt) noexcept
{
    return ((pt.x >= rc.left) && (pt.x < rc.right) &&
        (pt.y >= rc.top) && (pt.y < rc.bottom));
}
EckInlineNdCe BOOL PtInRect(const D2D1_RECT_F& rc, D2D1_POINT_2F pt) noexcept
{
    return ((pt.x >= rc.left) && (pt.x < rc.right) &&
        (pt.y >= rc.top) && (pt.y < rc.bottom));
}
EckInlineNdCe BOOL PtInRect(const D2D1_RECT_F& rc, POINT pt) noexcept
{
    return ((pt.x >= rc.left) && (pt.x < rc.right) &&
        (pt.y >= rc.top) && (pt.y < rc.bottom));
}

EckInlineNdCe D2D1_RECT_F MakeD2DRectF(const RECT& rc) noexcept
{
    return { (float)rc.left, (float)rc.top, (float)rc.right, (float)rc.bottom };
}
EckInlineNdCe D2D1_RECT_F MakeD2DRectF(float x, float y, float cx, float cy) noexcept
{
    return { x, y, x + cx, y + cy };
}

EckInlineNdCe RECT MakeRect(const D2D1_RECT_F& rc) noexcept
{
    return { (LONG)rc.left, (LONG)rc.top, (LONG)rc.right, (LONG)rc.bottom };
}
EckInlineNdCe RECT MakeRect(int x, int y, int cx, int cy) noexcept
{
    return { x, y, x + cx, y + cy };
}

#if !ECK_OPT_NO_GDIPLUS
EckInline GpRectF MakeGpRectF(const RECT& rc) noexcept
{
    return { (REAL)rc.left,(REAL)rc.top,(REAL)(rc.right - rc.left),(REAL)(rc.bottom - rc.top) };
}
#endif

EckInlineNdCe RCWH MakeRcwh(const RECT& rc) noexcept
{
    return RCWH{ rc.left,rc.top,rc.right - rc.left,rc.bottom - rc.top };
}

EckInlineNdCe D2D1_POINT_2F MakeD2DPointF(POINT pt) noexcept
{
    return { (float)pt.x,(float)pt.y };
}

template<CcpRect T>
EckInlineNdCe BOOL IsRectsIntersect(const T& rc1, const T& rc2) noexcept
{
    if constexpr (CRectTraits<T>::IsRcwh)
    {
        if (std::max(rc1.x, rc2.x) < std::min(rc1.x + rc1.cx, rc2.x + rc2.cx))
            return std::max(rc1.y, rc2.y) < std::min(rc1.y + rc1.cy, rc2.y + rc2.cy);
        return FALSE;
    }
    else
    {
        if (std::max(rc1.left, rc2.left) < std::min(rc1.right, rc2.right))
            return std::max(rc1.top, rc2.top) < std::min(rc1.bottom, rc2.bottom);
        return FALSE;
    }
}

template<CcpRect T>
EckInlineCe void InflateRect(_Inout_ T& rc, auto dx, auto dy) noexcept
{
    if constexpr (CRectTraits<T>::IsRcwh)
    {
        rc.x -= dx;
        rc.y -= dy;
        rc.cx += (dx * 2);
        rc.cy += (dy * 2);
    }
    else
    {
        rc.left -= dx;
        rc.top -= dy;
        rc.right += dx;
        rc.bottom += dy;
    }
}

template<CcpRect T>
EckInlineCe BOOL IntersectRect(_Out_ T& rcDst, const T& rcSrc1, const T& rcSrc2) noexcept
{
    if constexpr (CRectTraits<T>::IsRcwh)
    {
        rcDst.x = std::max(rcSrc1.x, rcSrc2.x);
        rcDst.cx = std::min(rcSrc1.x + rcSrc1.cx, rcSrc2.x + rcSrc2.cx) - rcDst.x;
        if (rcDst.cx > 0)
        {
            rcDst.y = std::max(rcSrc1.y, rcSrc2.y);
            rcDst.cy = std::min(rcSrc1.y + rcSrc1.cy, rcSrc2.y + rcSrc2.cy) - rcDst.y;
            if (rcDst.cy > 0)
                return TRUE;
        }
        rcDst = {};
        return FALSE;
    }
    else
    {
        rcDst.left = std::max(rcSrc1.left, rcSrc2.left);
        rcDst.right = std::min(rcSrc1.right, rcSrc2.right);
        if (rcDst.left < rcDst.right)
        {
            rcDst.top = std::max(rcSrc1.top, rcSrc2.top);
            rcDst.bottom = std::min(rcSrc1.bottom, rcSrc2.bottom);
            if (rcDst.top < rcDst.bottom)
                return TRUE;
        }
        rcDst = {};
        return FALSE;
    }
}

template<CcpRect T>
EckInlineCe void OffsetRect(_Inout_ T& rc, auto dx, auto dy) noexcept
{
    if constexpr (CRectTraits<T>::IsRcwh)
    {
        rc.x += dx;
        rc.y += dy;
    }
    else
    {
        rc.left += dx;
        rc.top += dy;
        rc.right += dx;
        rc.bottom += dy;
    }
}

template<CcpRect T>
EckInlineNdCe BOOL IsRectEmpty(const T& rc) noexcept
{
    if constexpr (CRectTraits<T>::IsRcwh)
        return rc.cx <= 0 || rc.cy <= 0;
    else
        return rc.left >= rc.right || rc.top >= rc.bottom;
}

template<CcpRect T>
EckNfInlineCe void UnionRect(T& rcDst, const T& rcSrc1, const T& rcSrc2) noexcept
{
    if constexpr (CRectTraits<T>::IsRcwh)
    {
        const BOOL b1 = IsRectEmpty(rcSrc1), b2 = IsRectEmpty(rcSrc2);
        if (b1)
        {
            if (b2)
                rcDst = {};
            else
                rcDst = rcSrc2;
        }
        else if (b2)
            rcDst = rcSrc1;
        else
        {
            rcDst.x = std::min(rcSrc1.x, rcSrc2.x);
            rcDst.y = std::min(rcSrc1.y, rcSrc2.y);
            rcDst.cx = std::max(rcSrc1.x + rcSrc1.cx, rcSrc2.x + rcSrc2.cx) - rcDst.x;
            rcDst.cy = std::max(rcSrc1.y + rcSrc1.cy, rcSrc2.y + rcSrc2.cy) - rcDst.y;
        }
    }
    else
    {
        const BOOL b1 = IsRectEmpty(rcSrc1), b2 = IsRectEmpty(rcSrc2);
        if (b1)
        {
            if (b2)
                rcDst = {};
            else
                rcDst = rcSrc2;
        }
        else if (b2)
            rcDst = rcSrc1;
        else
        {
            rcDst.left = std::min(rcSrc1.left, rcSrc2.left);
            rcDst.top = std::min(rcSrc1.top, rcSrc2.top);
            rcDst.right = std::max(rcSrc1.right, rcSrc2.right);
            rcDst.bottom = std::max(rcSrc1.bottom, rcSrc2.bottom);
        }
    }
}

template<CcpRect T>
EckInlineNdCe BOOL IsRectInclude(const T& rcIn, const T& rcOut) noexcept
{
    if constexpr (CRectTraits<T>::IsRcwh)
        return
        rcIn.x <= rcOut.x &&
        rcIn.y <= rcOut.y &&
        rcIn.x + rcIn.cx >= rcOut.x + rcOut.cx &&
        rcIn.y + rcIn.cy >= rcOut.y + rcOut.cy;
    else
        return
        rcIn.left <= rcOut.left &&
        rcIn.top <= rcOut.top &&
        rcIn.right >= rcOut.right &&
        rcIn.bottom >= rcOut.bottom;
}

EckInlineNdCe bool operator==(const D2D1_RECT_F& rc1, const D2D1_RECT_F& rc2) noexcept
{
    return rc1.left == rc2.left && rc1.top == rc2.top &&
        rc1.right == rc2.right && rc1.bottom == rc2.bottom;
}
EckInlineNdCe bool operator==(const D2D1_POINT_2F& pt1, const D2D1_POINT_2F& pt2) noexcept
{
    return pt1.x == pt2.x && pt1.y == pt2.y;
}
EckInlineNdCe bool operator==(const D2D1_SIZE_F& sz1, const D2D1_SIZE_F& sz2) noexcept
{
    return sz1.width == sz2.width && sz1.height == sz2.height;
}
EckInlineNdCe bool operator==(const D2D1_SIZE_U& sz1, const D2D1_SIZE_U& sz2) noexcept
{
    return sz1.width == sz2.width && sz1.height == sz2.height;
}
EckInlineNdCe bool operator==(const RCWH& rc1, const RCWH& rc2) noexcept
{
    return rc1.x == rc2.x && rc1.y == rc2.y && rc1.cx == rc2.cx && rc1.cy == rc2.cy;
}

/// <summary>
/// 调整矩形以完全包含。
/// 函数以最小的距离偏移矩形使之完全处于参照矩形当中
/// </summary>
/// <param name="rc">要调整的矩形</param>
/// <param name="rcRef">参照矩形</param>
/// <returns>成功返回TRUE，失败或无需调整返回FALSE</returns>
template<CcpRect T>
EckNfInlineCe BOOL AdjustRectIntoAnother(_Inout_ T& rc, const T& rcRef) noexcept
{
    constexpr auto MaxValue = std::numeric_limits<typename CRectTraits<T>::T>::max();
    typename CRectTraits<T>::T dxLeft, dxRight, dyTop, dyBottom;
    if constexpr (CRectTraits<T>::IsRcwh)
    {
        if (rc.cx > rcRef.cx || rc.cy > rcRef.cy)
            return FALSE;
        dxLeft = rcRef.x - rc.x;
        dxRight = rc.x + rc.cx - rcRef.cx;
        dyTop = rcRef.y - rc.y;
        dyBottom = rc.y + rc.cy - rcRef.cy;
    }
    else
    {
        if (rc.right - rc.left > rcRef.right - rcRef.left ||
            rc.bottom - rc.top > rcRef.bottom - rcRef.top)
            return FALSE;
        dxLeft = rcRef.left - rc.left;
        dxRight = rc.right - rcRef.right;
        dyTop = rcRef.top - rc.top;
        dyBottom = rc.bottom - rcRef.bottom;
    }
    if (dxLeft <= 0 && dxRight <= 0 && dyTop <= 0 && dyBottom <= 0)
        return FALSE;
    if (dxLeft < 0)
        dxLeft = MaxValue;
    if (dxRight < 0)
        dxRight = MaxValue;
    if (dxLeft == MaxValue && dxRight == MaxValue)
        dxLeft = 0;
    else if (dxLeft > dxRight)
        dxLeft = -dxRight;

    if (dyTop < 0)
        dyTop = MaxValue;
    if (dyBottom < 0)
        dyBottom = MaxValue;
    if (dyTop == MaxValue && dyBottom == MaxValue)
        dyTop = 0;
    else if (dyTop > dyBottom)
        dyTop = -dyBottom;
    OffsetRect(rc, dxLeft, dyTop);
    return TRUE;
}

/// <summary>
/// 调整矩形以适应。
/// 函数等宽高比缩放矩形使之成为完全包含于参照矩形之中的最大矩形
/// </summary>
/// <param name="rc">欲调整的矩形</param>
/// <param name="rcRef">参照矩形</param>
/// <returns>成功返回TRUE，失败返回FALSE</returns>
template<CcpRect T>
EckNfInlineCe BOOL AdjustRectToFitAnother(_Inout_ T& rc, const T& rcRef) noexcept
{
    if constexpr (CRectTraits<T>::IsRcwh)
    {
        const auto
            cxMax = rcRef.cx,
            cyMax = rcRef.cy,
            cx0 = rc.cx,
            cy0 = rc.cy;
        if (cxMax <= 0 || cyMax <= 0 || cx0 <= 0 || cy0 <= 0)
            return FALSE;
        typename CRectTraits<T>::T cx, cy;
        if (cxMax * cy0 > cx0 * cyMax)// y对齐
        {
            cy = cyMax;
            cx = cx0 * cy / cy0;
        }
        else// x对齐
        {
            cx = cxMax;
            cy = cx * cy0 / cx0;
        }

        rc.x = rcRef.x + (cxMax - cx) / 2;
        rc.y = rcRef.y + (cyMax - cy) / 2;
        rc.cx = cx;
        rc.cy = cy;
        return TRUE;
    }
    else
    {
        const auto
            cxMax = rcRef.right - rcRef.left,
            cyMax = rcRef.bottom - rcRef.top,
            cx0 = rc.right - rc.left,
            cy0 = rc.bottom - rc.top;
        if (cxMax <= 0 || cyMax <= 0 || cx0 <= 0 || cy0 <= 0)
            return FALSE;
        typename CRectTraits<T>::T cx, cy;
        if (cxMax * cy0 > cx0 * cyMax)// y对齐
        {
            cy = cyMax;
            cx = cx0 * cy / cy0;
        }
        else// x对齐
        {
            cx = cxMax;
            cy = cx * cy0 / cx0;
        }

        rc.left = rcRef.left + (cxMax - cx) / 2;
        rc.top = rcRef.top + (cyMax - cy) / 2;
        rc.right = rc.left + cx;
        rc.bottom = rc.top + cy;
        return TRUE;
    }
}

/// <summary>
/// 调整矩形以充满。
/// 函数等宽高比缩放矩形使之成为完全覆盖参照矩形内容的最小矩形（可能超出参照矩形）
/// </summary>
/// <param name="rc">欲调整的矩形</param>
/// <param name="rcRef">参照矩形</param>
/// <returns>成功返回TRUE，失败返回FALSE</returns>
template<CcpRect T>
EckNfInlineCe BOOL AdjustRectToFillAnother(_Inout_ T& rc, const T& rcRef) noexcept
{
    if constexpr (CRectTraits<T>::IsRcwh)
    {
        const auto
            cxMax = rcRef.cx,
            cyMax = rcRef.cy,
            cx0 = rc.cx,
            cy0 = rc.cy;
        if (cxMax <= 0 || cyMax <= 0 || cx0 <= 0 || cy0 <= 0)
            return FALSE;

        typename CRectTraits<T>::T cxRgn, cyRgn, x, y;
        cxRgn = cyMax * cx0 / cy0;
        if (cxRgn < cxMax)// 先尝试y对齐，看x方向是否充满
        {
            cxRgn = cxMax;
            cyRgn = cxMax * cy0 / cx0;
            x = 0;
            y = (cyMax - cyRgn) / 2;
        }
        else
        {
            cyRgn = cyMax;
            x = (cxMax - cxRgn) / 2;
            y = 0;
        }

        rc.x = rcRef.x + x;
        rc.y = rcRef.y + y;
        rc.cx = cxRgn;
        rc.cy = cyRgn;
        return TRUE;
    }
    else
    {
        const auto
            cxMax = rcRef.right - rcRef.left,
            cyMax = rcRef.bottom - rcRef.top,
            cx0 = rc.right - rc.left,
            cy0 = rc.bottom - rc.top;
        if (cxMax <= 0 || cyMax <= 0 || cx0 <= 0 || cy0 <= 0)
            return FALSE;

        typename CRectTraits<T>::T cxRgn, cyRgn, x, y;
        cxRgn = cyMax * cx0 / cy0;
        if (cxRgn < cxMax)// 先尝试y对齐，看x方向是否充满
        {
            cxRgn = cxMax;
            cyRgn = cxMax * cy0 / cx0;
            x = 0;
            y = (cyMax - cyRgn) / 2;
        }
        else
        {
            cyRgn = cyMax;
            x = (cxMax - cxRgn) / 2;
            y = 0;
        }

        rc.left = rcRef.left + x;
        rc.top = rcRef.top + y;
        rc.right = rcRef.left + x + cxRgn;
        rc.bottom = rcRef.top + y + cyRgn;
        return TRUE;
    }
}

/// <summary>
/// 居中矩形。
/// 函数使矩形居中于参照矩形
/// </summary>
/// <param name="rc">欲调整的矩形</param>
/// <param name="rcRef">参照矩形</param>
template<CcpRect T>
EckNfInlineCe void CenterRect(_Inout_ T& rc, const T& rcRef) noexcept
{
    if constexpr (CRectTraits<T>::IsRcwh)
    {
        rc.x = rcRef.x + (rcRef.cx - rc.cx) / 2;
        rc.y = rcRef.y + (rcRef.cy - rc.cy) / 2;
    }
    else
    {
        const auto
            cx = rc.right - rc.left,
            cy = rc.bottom - rc.top,
            cxRef = rcRef.right - rcRef.left,
            cyRef = rcRef.bottom - rcRef.top;
        rc.left = rcRef.left + (cxRef - cx) / 2;
        rc.top = rcRef.top + (cyRef - cy) / 2;
        rc.right = rc.left + cx;
        rc.bottom = rc.top + cy;
    }
}

EckInlineNdCe MARGINS MakeMargin(int i) noexcept
{
    return { i,i,i,i };
}
EckInlineNdCe MARGINS MakeMarginTopBottom(int i) noexcept
{
    return { 0,0,i,i };
}
EckInlineNdCe MARGINS MakeMarginLeftRight(int i) noexcept
{
    return { i,i,0,0 };
}
EckInlineNdCe MARGINS MakeMarginHV(int h, int v) noexcept
{
    return { h,h,v,v };
}

EckInlineNdCe BOOL PtInCircle(D2D1_POINT_2F pt, D2D1_POINT_2F ptCenter, float fRadius) noexcept
{
    return (pt.x - ptCenter.x) * (pt.x - ptCenter.x) + (pt.y - ptCenter.y) * (pt.y - ptCenter.y) <=
        fRadius * fRadius;
}
EckInlineNdCe BOOL PtInCircle(POINT pt, POINT ptCenter, int iRadius) noexcept
{
    return (pt.x - ptCenter.x) * (pt.x - ptCenter.x) + (pt.y - ptCenter.y) * (pt.y - ptCenter.y) <=
        iRadius * iRadius;
}

#ifdef _D2D1_H_// 与ECK_OPT_NO_DX选项兼容
EckInlineNdCe D2D1_ELLIPSE MakeD2DEllipse(float x, float y, float w, float h) noexcept
{
    return D2D1_ELLIPSE{ { x + w / 2.f,y + h / 2.f },w / 2.f,h / 2.f };
}
#endif // _D2D1_H_

EckInlineNdCe MARGINS D2DRectFToMargins(const D2D1_RECT_F& rc) noexcept
{
    return MARGINS{ (int)rc.left, (int)rc.top, (int)rc.right, (int)rc.bottom };
}
EckInlineNdCe D2D1_RECT_F MarginsToD2DRectF(const MARGINS& m) noexcept
{
    return D2D1_RECT_F{ (float)m.cxLeftWidth,(float)m.cyTopHeight,
        (float)m.cxRightWidth,(float)m.cyBottomHeight };
}

EckInlineCe void RectCornerToPoint(const D2D1_RECT_F& rc,
    _Out_writes_(4) D2D1_POINT_2F* ppt) noexcept
{
    ppt[0] = { rc.left,rc.top };
    ppt[1] = { rc.right,rc.top };
    ppt[2] = { rc.left,rc.bottom };
    ppt[3] = { rc.right,rc.bottom };
}

EckInline void CeilRect(_Inout_ D2D1_RECT_F& rc) noexcept
{
    rc.left = floorf(rc.left);
    rc.top = floorf(rc.top);
    rc.right = ceilf(rc.right);
    rc.bottom = ceilf(rc.bottom);
}
EckInline void CeilRect(const D2D1_RECT_F& rc, _Out_ RECT& rcOut) noexcept
{
    rcOut.left = (LONG)floorf(rc.left);
    rcOut.top = (LONG)floorf(rc.top);
    rcOut.right = (LONG)ceilf(rc.right);
    rcOut.bottom = (LONG)ceilf(rc.bottom);
}

EckInlineNdCe BLENDFUNCTION MakeBlendFunction(BYTE byAlpha)
{
    return { AC_SRC_OVER,0,byAlpha,AC_SRC_ALPHA };
}
#pragma endregion 图形结构

#pragma region 字符串
EckInlineNdCe CHAR ByteToHex(BYTE x) noexcept { return x > 9 ? x + 55 : x + 48; }
EckInlineNdCe CHAR ByteToHexLower(BYTE x) noexcept { return x > 9 ? x + 87 : x + 48; }

EckInlineNdCe BYTE ByteFromHex(CHAR x) noexcept
{
    if (x >= 'A' && x <= 'Z')
        return x - 'A' + 10;
    else if (x >= 'a' && x <= 'z')
        return x - 'a' + 10;
    else if (x >= '0' && x <= '9')
        return x - '0';
    else
        return 0;
}
EckInlineNdCe BYTE ByteFromHex(CHAR x1, CHAR x2) noexcept
{
    return (ByteFromHex(x1) << 4) | ByteFromHex(x2);
}

EckInlineCe void ToStringUpper(_In_reads_bytes_(cb) PCVOID p, size_t cb,
    _Out_writes_(cb * 2) CcpStdCharPtr auto pszResult) noexcept
{
    EckCounter(cb, i)
    {
        const BYTE b = ((PCBYTE)p)[i];
        pszResult[i * 2] = ByteToHex(b >> 4);
        pszResult[i * 2 + 1] = ByteToHex(b & 0b1111);
    }
}
EckInlineCe void ToStringLower(_In_reads_bytes_(cb) PCVOID p, size_t cb,
    _Out_writes_(cb * 2) CcpStdCharPtr auto pszResult) noexcept
{
    EckCounter(cb, i)
    {
        const BYTE b = ((PCBYTE)p)[i];
        pszResult[i * 2] = ByteToHexLower(b >> 4);
        pszResult[i * 2 + 1] = ByteToHexLower(b & 0b1111);
    }
}

EckInlineCe void FromString(_Out_writes_bytes_(cb) PVOID p, size_t cb,
    _In_reads_(cb * 2) PCSTR psz) noexcept
{
    EckCounter(cb, i)
        * ((BYTE*)p + i) = ByteFromHex(psz[i * 2], psz[i * 2 + 1]);
}

EckInlineCe void Md5ToString(_In_reads_bytes_(16) PCVOID pMd5,
    _Out_writes_(32) CcpStdCharPtr auto pszResult, BOOL bUpper = TRUE) noexcept
{
    if (bUpper)
        ToStringUpper(pMd5, (size_t)16, pszResult);
    else
        ToStringLower(pMd5, (size_t)16, pszResult);
}

inline constexpr void GuidToString(const GUID& guid,
    _Out_writes_(32) CcpStdCharPtr auto pszResult, BOOL bUpper = TRUE) noexcept
{
    const BYTE* p = (const BYTE*)&guid;
    BYTE by[16];
    by[0] = p[3];
    by[1] = p[2];
    by[2] = p[1];
    by[3] = p[0];

    by[4] = p[5];
    by[5] = p[4];

    by[6] = p[7];
    by[7] = p[6];
    for (int i = 0; i < 8; ++i)
        by[8 + i] = p[8 + i];

    if (bUpper)
        ToStringUpper(by, 16, pszResult);
    else
        ToStringUpper(by, 16, pszResult);
}
#pragma endregion 字符串

#pragma region 转换
template<class T, class U>
EckInlineNdCe T DwordToPtr(U i) noexcept
{
    return (T)((ULONG_PTR)i);
}
template<class T, class U>
EckInlineNdCe T PtrToDword(U p) noexcept
{
    return (T)((ULONG_PTR)p);
}

EckInlineNdCe SIZE_T Cch2CbW(int cch) noexcept
{
    return (cch + 1) * sizeof(WCHAR);
}
EckInlineNdCe SIZE_T Cch2CbA(int cch) noexcept
{
    return (cch + 1) * sizeof(CHAR);
}

EckInlineNdCe HRESULT HResultFromBool(BOOL b) noexcept
{
    return b ? S_OK : E_FAIL;
}

template<class T1, class T2>
EckInlineNdCe T1 ReinterpretValue(T2 v) noexcept
{
    return std::bit_cast<T1>(v);
}

template<class T, class U>
EckInline T DbgDynamicCast(U p) noexcept
{
#ifdef _DEBUG
    if (!p)
        return nullptr;
    const auto p1 = dynamic_cast<T>(p);
    if (!p1)
    {
        EckDbgPrint(L"Dynamic cast failed!");
        EckDbgBreak();
    }
    return p1;
#else
    return (T)p;
#endif// _DEBUG
}
#pragma endregion 转换

#pragma region 运算
EckInlineNd float RoundToF(float fVal, int cDigits) noexcept
{
    float fTemp = powf(10, (float)cDigits);
    return roundf(fVal * fTemp) / fTemp;
}
EckInlineNd double RoundTo(double fVal, int cDigits) noexcept
{
    double fTemp = pow(10, (double)cDigits);
    return round(fVal * fTemp) / fTemp;
}

EckInlineNdCe BOOL Sign(auto v) noexcept { return v >= 0; }
template<class T>
EckInlineNdCe T SignValue(T v) noexcept { return (v >= 0 ? 1 : -1); }

template<class T, class U>
EckInlineNdCe T SetSign(T x, U iSign) noexcept
{
    if (iSign > 0) return Abs(x);
    if (iSign < 0) return -Abs(x);
    else return x;
}

EckInlineNdCe auto Abs(auto x) noexcept { return (x >= 0) ? x : -x; }

EckInlineNdCe auto DivUpper(std::integral auto x, std::integral auto y) noexcept { return (x - 1) / y + 1; }

#ifdef _WIN64
constexpr inline size_t FnvOffsetBasis = 14695981039346656037ull;
constexpr inline size_t FnvPrime = 1099511628211ull;
#else
constexpr inline size_t FnvOffsetBasis = 2166136261u;
constexpr inline size_t FnvPrime = 16777619u;
#endif// _WIN64

EckInlineNdCe size_t Fnv1aHash(PCBYTE p, size_t cb) noexcept
{
    size_t hash = FnvOffsetBasis;
    EckCounter(cb, i)
    {
        hash ^= p[i];
        hash *= FnvPrime;
    }
    return hash;
}

EckInline BOOL FloatEqual(float f1, float f2, float fEpsilon = FLT_EPSILON) noexcept
{
    return fabs(f1 - f2) < fEpsilon;
}
EckInline BOOL FloatEqual(double f1, double f2, double fEpsilon = DBL_EPSILON) noexcept
{
    return abs(f1 - f2) < fEpsilon;
}

template<class T>
EckInlineNdCe T ValDistance(T x1, T x2) noexcept { return (x1 > x2) ? (x1 - x2) : (x2 - x1); }

template<CcpNumberOrEnum T>
EckInlineNdCe T DpiScale(T i, int iDpiNew, int iDpiOld = 96) noexcept { return T(i * iDpiNew / iDpiOld); }
// deprecated.
template<CcpNumberOrEnum T>
EckInlineNdCe T DpiScaleF(T i, int iDpiNew, int iDpiOld = 96) noexcept { return T(i * iDpiNew / iDpiOld); }
EckInlineCe void DpiScale(_Inout_ CcpRectStruct auto& rc, int iDpiNew, int iDpiOld = 96) noexcept
{
    rc.left = rc.left * iDpiNew / iDpiOld;
    rc.top = rc.top * iDpiNew / iDpiOld;
    rc.right = rc.right * iDpiNew / iDpiOld;
    rc.bottom = rc.bottom * iDpiNew / iDpiOld;
}
EckInlineCe void DpiScale(_Inout_ SIZE& size, int iDpiNew, int iDpiOld = 96) noexcept
{
    size.cx = size.cx * iDpiNew / iDpiOld;
    size.cy = size.cy * iDpiNew / iDpiOld;
}
EckInlineCe void DpiScale(_Inout_ D2D1_SIZE_F& size, int iDpiNew, int iDpiOld = 96) noexcept
{
    size.width = size.width * iDpiNew / iDpiOld;
    size.height = size.height * iDpiNew / iDpiOld;
}
EckInlineCe void DpiScale(_Inout_ CcpPointStruct auto& pt, int iDpiNew, int iDpiOld = 96) noexcept
{
    pt.x = pt.x * iDpiNew / iDpiOld;
    pt.y = pt.y * iDpiNew / iDpiOld;
}

EckInlineCe void UpdateDpiSize(auto& Dpis, int iDpi) noexcept
{
    for (int* p = ((int*)&Dpis) + 1; p < PointerSkipType(&Dpis); p += 2)
        *p = DpiScale(*(p - 1), iDpi);
}
EckInlineCe void UpdateDpiSizeF(auto& Dpis, int iDpi) noexcept
{
    for (float* p = ((float*)&Dpis) + 1; p < PointerSkipType(&Dpis); p += 2)
        *p = DpiScaleF(*(p - 1), iDpi);
}
#pragma endregion 运算

#pragma region WinLargeInt
EckInlineNdCe ULARGE_INTEGER operator+(ULARGE_INTEGER x1, ULARGE_INTEGER x2) noexcept
{
    return ULARGE_INTEGER{ .QuadPart = x1.QuadPart + x2.QuadPart };
}
EckInlineNdCe ULARGE_INTEGER operator+(ULARGE_INTEGER x1, ULONGLONG x2) noexcept
{
    return ULARGE_INTEGER{ .QuadPart = x1.QuadPart + x2 };
}

EckInlineNdCe ULARGE_INTEGER operator-(ULARGE_INTEGER x1, ULARGE_INTEGER x2) noexcept
{
    return ULARGE_INTEGER{ .QuadPart = x1.QuadPart - x2.QuadPart };
}
EckInlineNdCe ULARGE_INTEGER operator-(ULARGE_INTEGER x1, ULONGLONG x2) noexcept
{
    return ULARGE_INTEGER{ .QuadPart = x1.QuadPart - x2 };
}

EckInlineNdCe ULARGE_INTEGER operator*(ULARGE_INTEGER x1, ULARGE_INTEGER x2) noexcept
{
    return ULARGE_INTEGER{ .QuadPart = x1.QuadPart * x2.QuadPart };
}
EckInlineNdCe ULARGE_INTEGER operator*(ULARGE_INTEGER x1, ULONGLONG x2) noexcept
{
    return ULARGE_INTEGER{ .QuadPart = x1.QuadPart * x2 };
}

EckInlineNdCe ULARGE_INTEGER operator/(ULARGE_INTEGER x1, ULARGE_INTEGER x2) noexcept
{
    return ULARGE_INTEGER{ .QuadPart = x1.QuadPart / x2.QuadPart };
}
EckInlineNdCe ULARGE_INTEGER operator/(ULARGE_INTEGER x1, ULONGLONG x2) noexcept
{
    return ULARGE_INTEGER{ .QuadPart = x1.QuadPart / x2 };
}

EckInlineNdCe std::strong_ordering operator<=>(ULARGE_INTEGER x1, ULARGE_INTEGER x2) noexcept
{
    return x1.QuadPart <=> x2.QuadPart;
}
EckInlineNdCe std::strong_ordering operator<=>(ULARGE_INTEGER x1, ULONGLONG x2) noexcept
{
    return x1.QuadPart <=> x2;
}

EckInlineNdCe bool operator==(ULARGE_INTEGER x1, ULARGE_INTEGER x2) noexcept
{
    return x1.QuadPart == x2.QuadPart;
}
EckInlineNdCe bool operator==(ULARGE_INTEGER x1, ULONGLONG x2) noexcept
{
    return x1.QuadPart == x2;
}

EckInlineCe ULARGE_INTEGER operator+=(ULARGE_INTEGER& x1, ULARGE_INTEGER x2) noexcept
{
    x1.QuadPart += x2.QuadPart;
    return x1;
}
EckInlineCe ULARGE_INTEGER operator+=(ULARGE_INTEGER& x1, ULONGLONG x2) noexcept
{
    x1.QuadPart += x2;
    return x1;
}

EckInlineCe ULARGE_INTEGER operator-=(ULARGE_INTEGER& x1, ULARGE_INTEGER x2) noexcept
{
    x1.QuadPart -= x2.QuadPart;
    return x1;
}
EckInlineCe ULARGE_INTEGER operator-=(ULARGE_INTEGER& x1, ULONGLONG x2) noexcept
{
    x1.QuadPart -= x2;
    return x1;
}

EckInlineNdCe LARGE_INTEGER operator+(LARGE_INTEGER x1, LARGE_INTEGER x2) noexcept
{
    return LARGE_INTEGER{ .QuadPart = x1.QuadPart + x2.QuadPart };
}
EckInlineNdCe LARGE_INTEGER operator+(LARGE_INTEGER x1, LONGLONG x2) noexcept
{
    return LARGE_INTEGER{ .QuadPart = x1.QuadPart + x2 };
}

EckInlineNdCe LARGE_INTEGER operator-(LARGE_INTEGER x1, LARGE_INTEGER x2) noexcept
{
    return LARGE_INTEGER{ .QuadPart = x1.QuadPart - x2.QuadPart };
}
EckInlineNdCe LARGE_INTEGER operator-(LARGE_INTEGER x1, LONGLONG x2) noexcept
{
    return LARGE_INTEGER{ .QuadPart = x1.QuadPart - x2 };
}

EckInlineNdCe LARGE_INTEGER operator*(LARGE_INTEGER x1, LARGE_INTEGER x2) noexcept
{
    return LARGE_INTEGER{ .QuadPart = x1.QuadPart * x2.QuadPart };
}
EckInlineNdCe LARGE_INTEGER operator*(LARGE_INTEGER x1, LONGLONG x2) noexcept
{
    return LARGE_INTEGER{ .QuadPart = x1.QuadPart * x2 };
}

EckInlineNdCe LARGE_INTEGER operator/(LARGE_INTEGER x1, LARGE_INTEGER x2) noexcept
{
    return LARGE_INTEGER{ .QuadPart = x1.QuadPart / x2.QuadPart };
}
EckInlineNdCe LARGE_INTEGER operator/(LARGE_INTEGER x1, LONGLONG x2) noexcept
{
    return LARGE_INTEGER{ .QuadPart = x1.QuadPart / x2 };
}

EckInlineNdCe std::strong_ordering operator<=>(LARGE_INTEGER x1, LARGE_INTEGER x2) noexcept
{
    return x1.QuadPart <=> x2.QuadPart;
}
EckInlineNdCe std::strong_ordering operator<=>(LARGE_INTEGER x1, LONGLONG x2) noexcept
{
    return x1.QuadPart <=> x2;
}

EckInlineNdCe bool operator==(LARGE_INTEGER x1, LARGE_INTEGER x2) noexcept
{
    return x1.QuadPart == x2.QuadPart;
}
EckInlineNdCe bool operator==(LARGE_INTEGER x1, LONGLONG x2) noexcept
{
    return x1.QuadPart == x2;
}

EckInlineCe LARGE_INTEGER& operator-=(LARGE_INTEGER& x1, LARGE_INTEGER x2) noexcept
{
    x1.QuadPart -= x2.QuadPart;
    return x1;
}
EckInlineCe LARGE_INTEGER operator-=(LARGE_INTEGER& x1, LONGLONG x2) noexcept
{
    x1.QuadPart -= x2;
    return x1;
}

EckInlineCe LARGE_INTEGER& operator+=(LARGE_INTEGER& x1, LARGE_INTEGER x2) noexcept
{
    x1.QuadPart += x2.QuadPart;
    return x1;
}
EckInlineCe LARGE_INTEGER operator+=(LARGE_INTEGER& x1, LONGLONG x2) noexcept
{
    x1.QuadPart += x2;
    return x1;
}

EckInlineNdCe LARGE_INTEGER ToLi(ULARGE_INTEGER x) noexcept
{
    return LARGE_INTEGER{ .QuadPart = (LONGLONG)x.QuadPart };
}
EckInlineNdCe LARGE_INTEGER ToLi(LONGLONG x) noexcept
{
    return LARGE_INTEGER{ .QuadPart = x };
}

EckInlineNdCe ULARGE_INTEGER ToUli(LARGE_INTEGER x) noexcept
{
    return ULARGE_INTEGER{ .QuadPart = (ULONGLONG)x.QuadPart };
}
EckInlineNdCe ULARGE_INTEGER ToUli(ULONGLONG x) noexcept
{
    return ULARGE_INTEGER{ .QuadPart = x };
}
#pragma endregion WinLargeInt

#pragma region 其他
EckInlineNdCe BOOL IsGuidEqu(REFGUID x1, REFGUID x2) noexcept
{
    return
        x1.Data1 == x2.Data1 &&
        x1.Data2 == x2.Data2 &&
        x1.Data3 == x2.Data3 &&
        x1.Data4[0] == x2.Data4[0] &&
        x1.Data4[1] == x2.Data4[1] &&
        x1.Data4[2] == x2.Data4[2] &&
        x1.Data4[3] == x2.Data4[3] &&
        x1.Data4[4] == x2.Data4[4] &&
        x1.Data4[5] == x2.Data4[5] &&
        x1.Data4[6] == x2.Data4[6] &&
        x1.Data4[7] == x2.Data4[7];
}

template<class T, size_t N>
EckInlineNdCe void AssignArray(T(&x1)[N], const T(&x2)[N]) noexcept
{
    EckCounter(N, i)
        x1[i] = x2[i];
}

EckInlineNdCe LPARAM MakeKeyStrokeFlag(USHORT cRepeat, UINT uScanCode, BOOL bExtended,
    BOOL bAlt, BOOL bPreviousState, BOOL bTransition) noexcept
{
    EckAssert(bExtended == 0 || bExtended == 1);
    EckAssert(bAlt == 0 || bAlt == 1);
    EckAssert(bPreviousState == 0 || bPreviousState == 1);
    EckAssert(bTransition == 0 || bTransition == 1);
    return cRepeat | (uScanCode << 16) | (bExtended << 24) | (bAlt << 29) |
        (bPreviousState << 30) | (bTransition << 31);
}

EckInline void SafeRelease(_Inout_ CcpComInterface auto*& pUnk) noexcept
{
    if (pUnk)
    {
        pUnk->Release();
        pUnk = nullptr;
    }
}
EckInline void SafeReleaseAssert0(_Inout_ CcpComInterface auto*& pUnk) noexcept
{
#ifdef _DEBUG
    if (pUnk)
    {
        EckAssert(!pUnk->Release());
        pUnk = nullptr;
    }
#else
    SafeRelease(pUnk);
#endif
}

template<class TChar>
    requires requires{ sizeof(TChar) == 1 || sizeof(TChar) == 2; }
EckInlineNdCe auto StringViewToNtString(std::basic_string_view<TChar> sv) noexcept
{
    using TNtString = std::conditional_t<sizeof(TChar) == 1, ANSI_STRING, UNICODE_STRING>;
    const auto cb = USHORT(sv.size() * sizeof(TChar));
    return TNtString{
        .Length = cb,
        .MaximumLength = cb,
        .Buffer = (decltype(TNtString::Buffer))sv.data(),
    };
}
#pragma endregion 其他
ECK_NAMESPACE_END