/*
* WinEzCtrlKit Library
*
* Utility.h ： 实用函数
*
* Copyright(C) 2023-2024 QingKong
*/

#pragma once
#include "ECK.h"

#include <math.h>
#include <time.h>
#include <process.h>

#include <string_view>
#include <memory>
#if ECKCXX20
#include <concepts>
#include <bit>
#endif

#include <windowsx.h>
#include <d2d1_1.h>
#include <Shlwapi.h>

ECK_NAMESPACE_BEGIN

template<class T>
struct VADeleter
{
	void operator()(T* p)
	{
		VirtualFree(p, 0, MEM_RELEASE);
	}
};

template<class T>
using UniquePtrVA = std::unique_ptr<T, VADeleter<T>>;


namespace Colorref
{
	inline constexpr COLORREF
		Red               = 0x0000FF,// 红色
		Green             = 0x00FF00,// 绿色
		Blue              = 0xFF0000,// 蓝色
		Yellow            = 0x00FFFF,// 黄色
		Magenta           = 0xFF00FF,// 品红/洋红
		Cyan              = 0xFFFF00,// 艳青/青色
		Aqua			  = Cyan,

		Maroon            = 0x000080,// 红褐/暗红
		OfficeGreen       = 0x008000,// 墨绿/暗绿
		Olive             = 0x008080,// 褐绿/暗黄
		NavyBlue          = 0x800000,// 藏青/暗蓝
		Patriarch         = 0x800080,// 紫红/暗洋红
		Teal              = 0x808000,// 深青/暗青

		Silver            = 0xC0C0C0,// 浅灰/亮灰
		MoneyGreen        = 0xC0DCC0,// 美元绿
		LightBlue         = 0xF0CAA6,// 浅蓝/天蓝

		Gray              = 0x808080,// 灰色/暗灰
		NeutralGray       = 0xA4A0A0,// 中性灰
		MilkyWhite        = 0xF0FBFF,// 乳白

		Black             = 0x000000,// 黑色
		White             = 0xFFFFFF,// 白色

		BlueGray          = 0xFF8080,// 蓝灰
		PurplishBlue      = 0xE03058,// 藏蓝
		TenderGreen       = 0x00E080,// 嫩绿
		Turquoise         = 0x80E000,// 青绿
		YellowishBrown    = 0x0060C0,// 黄褐
		Pink              = 0xFFA8FF,// 粉红
		BrightYellow      = 0x00D8D8,// 嫩黄
		JadeWhite         = 0xECECEC,// 银白
		Purple            = 0xFF0090,// 紫色
		Azure             = 0xFF8800,// 天蓝
		Celadon           = 0x80A080,// 灰绿
		CyanBlue          = 0xC06000,// 青蓝
		Orange            = 0x0080FF,// 橙黄
		Peachblow         = 0x8050FF,// 桃红
		HibiscusRed       = 0xC080FF,// 芙红
		DeepGray          = 0x606060// 深灰
		;
}

/// <summary>
/// 计算下一对齐边界
/// </summary>
/// <param name="pStart">起始地址</param>
/// <param name="pCurr">当前地址</param>
/// <param name="cbAlign">对齐尺寸</param>
/// <returns>当前地址到下一对齐边界的距离，如果当前地址已经落在对齐边界上，则返回0</returns>
EckInline constexpr SIZE_T CalcNextAlignBoundaryDistance(const void* pStart, const void* pCurr, SIZE_T cbAlign)
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
EckInline constexpr const T* StepToNextAlignBoundary(const T* pStart, const T* pCurr, SIZE_T cbAlign)
{
	return (const T*)((BYTE*)pCurr + CalcNextAlignBoundaryDistance(pStart, pCurr, cbAlign));
}

/// <summary>
/// 步进到下一对齐边界
/// </summary>
/// <param name="pStart">起始指针</param>
/// <param name="pCurr">当前指针</param>
/// <param name="cbAlign">对齐尺寸</param>
/// <returns>步进后的指针，如果当前指针已经落在对齐边界上，则指针不变</returns>
template<class T>
EckInline constexpr T* StepToNextAlignBoundary(T* pStart, T* pCurr, SIZE_T cbAlign)
{
	return (T*)((BYTE*)pCurr + CalcNextAlignBoundaryDistance(pStart, pCurr, cbAlign));
}

template<class T>
EckInline constexpr PCVOID PtrSkipType(const T* p)
{
	return (PCVOID)((PCBYTE)p + sizeof(T));
}

template<class T>
EckInline constexpr void* PtrSkipType(T* p)
{
	return (void*)((PCBYTE)p + sizeof(T));
}

/// <summary>
/// 计算对齐后内存尺寸
/// </summary>
/// <param name="cbSize">尺寸</param>
/// <param name="cbAlign">对齐尺寸</param>
/// <returns>计算结果</returns>
EckInline constexpr SIZE_T AlignMemSize(SIZE_T cbSize, SIZE_T cbAlign)
{
	if (cbSize / cbAlign * cbAlign == cbSize)
		return cbSize;
	else ECKLIKELY
		return (cbSize / cbAlign + 1) * cbAlign;
}

#if ECKCXX20
template<class T>
concept ccpIsInteger = std::integral<T>;
#else
#pragma push_macro("ccpIsInteger")
#define ccpIsInteger class
#endif

template<ccpIsInteger T>
EckInline constexpr BYTE GetIntegerByte(T i, int idxByte)
{
	EckAssert(idxByte >= 0 && idxByte < sizeof(T));
	return (BYTE)((i >> (idxByte * 8)) & 0b11111111);
}

template<ccpIsInteger T>
EckInline constexpr void SetIntegerByte(T& i, int idxByte, BYTE by)
{
	EckAssert(idxByte >= 0 && idxByte < sizeof(T));
	i &= ((~(T)0b11111111) << (idxByte * 8));
}

template<int N, ccpIsInteger T>
EckInline constexpr BYTE GetIntegerByte(T i)
{
	static_assert(N >= 0 && N < sizeof(T));
	return (BYTE)((i >> (N * 8)) & 0b11111111);
}

template<int N, ccpIsInteger T>
EckInline constexpr void SetIntegerByte(T& i, BYTE by)
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
template<ccpIsInteger TRet, class... T>
EckInline constexpr TRet BytesToInteger(T... by)
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

EckInline constexpr UINT ReverseColorref(COLORREF cr)
{
	return BytesToInteger<UINT>(
		GetIntegerByte<2>(cr),
		GetIntegerByte<1>(cr),
		GetIntegerByte<0>(cr),
		0);
}

EckInline constexpr ARGB ColorrefToARGB(COLORREF cr, BYTE byAlpha = 0xFF)
{
	return ReverseColorref(cr) | (byAlpha << 24);
}

EckInline constexpr COLORREF ARGBToColorref(ARGB argb, BYTE* pbyAlpha = NULL)
{
	if (pbyAlpha)
		*pbyAlpha = GetIntegerByte<3>(argb);
	return ReverseColorref(argb);
}

EckInline constexpr D2D1_COLOR_F ARGBToD2dColorF(ARGB argb)
{
	return D2D1_COLOR_F
	{
		GetIntegerByte<2>(argb) / 255.f,
		GetIntegerByte<1>(argb) / 255.f,
		GetIntegerByte<0>(argb) / 255.f,
		GetIntegerByte<3>(argb) / 255.f
	};
}

EckInline constexpr D2D1_COLOR_F RgbToD2dColorF(UINT rgb, float fAlpha = 1.f)
{
	return D2D1_COLOR_F
	{
		GetIntegerByte<2>(rgb) / 255.f,
		GetIntegerByte<1>(rgb) / 255.f,
		GetIntegerByte<0>(rgb) / 255.f,
		fAlpha
	};
}

EckInline constexpr COLORREF ColorrefAlphaBlend(COLORREF cr, COLORREF crBK, BYTE byAlpha)
{
	return BytesToInteger<COLORREF>(
		GetIntegerByte<0>(crBK) * byAlpha / 0xFF + GetIntegerByte<0>(cr) * (0xFF - byAlpha) / 0xFF,
		GetIntegerByte<1>(crBK) * byAlpha / 0xFF + GetIntegerByte<1>(cr) * (0xFF - byAlpha) / 0xFF,
		GetIntegerByte<2>(crBK) * byAlpha / 0xFF + GetIntegerByte<2>(cr) * (0xFF - byAlpha) / 0xFF,
		0);
}

/// <summary>
/// 反转整数字节序
/// </summary>
/// <param name="i">输入</param>
/// <returns>转换结果</returns>
template<ccpIsInteger T>
EckInline constexpr T ReverseInteger(T i)
{
	auto p = (BYTE*)&i;
	std::reverse(p, p + sizeof(T));
	return i;
}

EckInline constexpr void ReverseByteOrder(BYTE* p, size_t cb)
{
	std::reverse(p, p + cb);
}


template<class T, class U>
EckInline constexpr T i32ToP(U i)
{
	return (T)((ULONG_PTR)i);
}

template<class T, class U>
EckInline constexpr T pToI32(U p)
{
	return (T)((ULONG_PTR)p);
}

EckInline BOOL IsFILETIMEZero(const FILETIME& ft)
{
	return ft.dwLowDateTime == 0 && ft.dwHighDateTime == 0;
}

EckInline bool operator==(const FILETIME& ft1, const FILETIME& ft2)
{
	return CompareFileTime(&ft1, &ft2) == 0;
}

EckInline bool operator>(const FILETIME& ft1, const FILETIME& ft2)
{
	return CompareFileTime(&ft1, &ft2) == 1;
}

EckInline bool operator<(const FILETIME& ft1, const FILETIME& ft2)
{
	return CompareFileTime(&ft1, &ft2) == -1;
}

//微软你妈死了
//#if defined(D3D11_NO_HELPERS) || !defined(__d3d11_h__)
//EckInline constexpr bool operator==(const RECT& rc1, const RECT& rc2)
//{
//	return rc1.left == rc2.left && rc1.top == rc2.top &&
//		rc1.right == rc2.right && rc1.bottom == rc2.bottom;
//}
//#endif

EckInline constexpr BOOL EquRect(const RECT& rc1, const RECT& rc2)
{
	return rc1.left == rc2.left && rc1.top == rc2.top &&
		rc1.right == rc2.right && rc1.bottom == rc2.bottom;
}

template<class T1, class T2>
EckInline constexpr BOOL IsBitSet(T1 dw1, T2 dw2)
{
	return (dw1 & dw2) == dw2;
}

EckInline void ScreenToClient(HWND hWnd, RECT* prc)
{
	::ScreenToClient(hWnd, (POINT*)prc);
	::ScreenToClient(hWnd, ((POINT*)prc) + 1);
}

EckInline void ClientToScreen(HWND hWnd, RECT* prc)
{
	::ClientToScreen(hWnd, (POINT*)prc);
	::ClientToScreen(hWnd, ((POINT*)prc) + 1);
}

inline constexpr RECT MakeRect(POINT pt1, POINT pt2)
{
	RECT rc{};
	if (pt1.x >= pt2.x)
	{
		rc.left = pt2.x;
		rc.right = pt1.x;
	}
	else
	{
		rc.left = pt1.x;
		rc.right = pt2.x;
	}

	if (pt1.y >= pt2.y)
	{
		rc.top = pt2.y;
		rc.bottom = pt1.y;
	}
	else
	{
		rc.top = pt1.y;
		rc.bottom = pt2.y;
	}

	return rc;
}

EckInline float RoundToF(float fVal, int cDigits)
{
	float fTemp = powf(10, (float)cDigits);
	return roundf(fVal * fTemp) / fTemp;
}

EckInline double RoundTo(double fVal, int cDigits)
{
	double fTemp = pow(10, (double)cDigits);
	return round(fVal * fTemp) / fTemp;
}

EckInline void RandSeed(UINT uSeed)
{
	srand(uSeed);
}

EckInline void RandSeed()
{
	srand((UINT)time(NULL));
} 

EckInline int Rand(int iMin = INT_MIN, int iMax = INT_MAX)
{
	return rand() % ((LONGLONG)iMax - (LONGLONG)iMin + 1ll) + (LONGLONG)iMin;
}

EckInline constexpr BOOL IsRectsIntersect(const RECT&rc1, const RECT&rc2)
{
	return
		std::max(rc1.left, rc2.left) < std::min(rc1.right, rc2.right) &&
		std::max(rc1.top, rc2.top) < std::min(rc1.bottom, rc2.bottom);
}

EckInline constexpr BOOL IsRectsIntersect(const D2D1_RECT_F& rc1, const D2D1_RECT_F& rc2)
{
	return
		std::max(rc1.left, rc2.left) < std::min(rc1.right, rc2.right) &&
		std::max(rc1.top, rc2.top) < std::min(rc1.bottom, rc2.bottom);
}

template<class T>
EckInline constexpr BOOL Sign(T v)
{
	return v >= 0;
}

template<class T>
EckInline constexpr BOOL SignVal(T v)
{
	return (v >= 0 ? 1 : -1);
}

EckInline constexpr SIZE_T Cch2Cb(int cch)
{
	return (cch + 1) * sizeof(WCHAR);
}

EckInline constexpr D2D1_RECT_F MakeD2DRcF(const RECT& rc)
{
	return { (float)rc.left, (float)rc.top, (float)rc.right, (float)rc.bottom };
}

EckInline constexpr D2D1_RECT_F MakeD2DRcF(float x, float y, float cx, float cy)
{
	return { x, y, x + cx, y + cy };
}

EckInline constexpr RECT MakeRect(const D2D1_RECT_F& rc)
{
	return { (LONG)rc.left, (LONG)rc.top, (LONG)rc.right, (LONG)rc.bottom };
}

EckInline constexpr RECT MakeRect(int x, int y, int cx, int cy)
{
	return { x, y, x + cx, y + cy };
}

EckInline constexpr UINT Gcd(UINT a, UINT b)
{
	UINT c = 0;
	for (;;)
	{
		c = a % b;
		if (c)
		{
			a = b;
			b = c;
		}
		else
			return b;
	}
}

EckInline GpRectF ToGpRectF(const RECT& rc)
{
	return { (REAL)rc.left,(REAL)rc.top,(REAL)(rc.right - rc.left),(REAL)(rc.bottom - rc.top) };
}

EckInline constexpr D2D1_POINT_2F MakeD2dPtF(POINT pt)
{
	return { (float)pt.x,(float)pt.y };
}

EckInline constexpr HRESULT HResultFromBool(BOOL b)
{
	return b ? S_OK : E_FAIL;
}

template<class T>
EckInline constexpr T Abs(T x)
{
	return (x >= 0) ? x : -x;
}

template<class T>
EckInline constexpr T SetSign(T x, T iSign)
{
	if (iSign > 0) return Abs(x);
	if (iSign < 0) return -Abs(x);
	else return x;
}

template<class TInterface>
EckInline void SafeRelease(TInterface*& pUnk)
{
	if (pUnk)
	{
		pUnk->Release();
		pUnk = NULL;
	}
}

#ifdef _WIN64
constexpr inline size_t c_FNVOffsetBasis = 14695981039346656037ull;
constexpr inline size_t c_FNVPrime = 1099511628211ull;
#else
constexpr inline size_t c_FNVOffsetBasis = 2166136261u;
constexpr inline size_t c_FNVPrime = 16777619u;
#endif// _WIN64

EckInline constexpr size_t Fnv1aHash(PCBYTE p, size_t cb)
{
	size_t hash = c_FNVOffsetBasis;
	EckCounter(cb, i)
	{
		hash ^= p[i];
		hash *= c_FNVPrime;
	}
	return hash;
}

#if !ECKCXX20
#undef ccpIsInteger
#pragma pop_macro("ccpIsInteger")
#endif

EckInline constexpr D2D1_COLOR_F ColorrefToD2dColorF(COLORREF cr, float fAlpha = 1.f)
{
	return D2D1_COLOR_F
	{
		GetRValue(cr) / 255.f,
		GetGValue(cr) / 255.f,
		GetBValue(cr) / 255.f,
		fAlpha
	};
}

EckInline constexpr void InflateRect(D2D1_RECT_F& rc, float dx, float dy)
{
	rc.left -= dx;
	rc.top -= dy;
	rc.right += dx;
	rc.bottom += dy;
}

EckInline constexpr void InflateRect(RECT& rc, int dx, int dy)
{
	rc.left -= dx;
	rc.top -= dy;
	rc.right += dx;
	rc.bottom += dy;
}

EckInline constexpr BOOL IntersectRect(D2D1_RECT_F& rcDst, const D2D1_RECT_F& rcSrc1, const D2D1_RECT_F& rcSrc2)
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

EckInline constexpr BOOL IntersectRect(RECT& rcDst, const RECT& rcSrc1, const RECT& rcSrc2)
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

EckInline constexpr void OffsetRect(D2D1_RECT_F& rc, float dx, float dy)
{
	rc.left += dx;
	rc.top += dy;
	rc.right += dx;
	rc.bottom += dy;
}

EckInline constexpr void OffsetRect(RECT& rc, int dx, int dy)
{
	rc.left += dx;
	rc.top += dy;
	rc.right += dx;
	rc.bottom += dy;
}

EckInline constexpr BOOL PtInRect(const RECT& rc, POINT pt)
{
	return ((pt.x >= rc.left) && (pt.x < rc.right) &&
		(pt.y >= rc.top) && (pt.y < rc.bottom));
}

EckInline constexpr BOOL PtInRect(const D2D1_RECT_F& rc, D2D1_POINT_2F pt)
{
	return ((pt.x >= rc.left) && (pt.x < rc.right) &&
		(pt.y >= rc.top) && (pt.y < rc.bottom));
}

EckInline constexpr BOOL PtInRect(const D2D1_RECT_F& rc, POINT pt)
{
	return ((pt.x >= rc.left) && (pt.x < rc.right) &&
		(pt.y >= rc.top) && (pt.y < rc.bottom));
}

EckInline constexpr BOOL IsRectEmpty(const RECT& rc)
{
	return rc.left >= rc.right || rc.top >= rc.bottom;
}

inline constexpr void UnionRect(RECT &rcDst,const RECT& rcSrc1,const RECT& rcSrc2)
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

EckInline constexpr BOOL IsRectEmpty(const D2D1_RECT_F& rc)
{
	return rc.left >= rc.right || rc.top >= rc.bottom;
}

inline constexpr void UnionRect(D2D1_RECT_F& rcDst, const D2D1_RECT_F& rcSrc1, const D2D1_RECT_F& rcSrc2)
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

EckInline constexpr bool operator==(const D2D1_RECT_F& rc1, const D2D1_RECT_F& rc2)
{
	return rc1.left == rc2.left && rc1.top == rc2.top &&
		rc1.right == rc2.right && rc1.bottom == rc2.bottom;
}

EckInline constexpr bool operator==(const D2D1_POINT_2F& pt1, const D2D1_POINT_2F& pt2)
{
	return pt1.x == pt2.x && pt1.y == pt2.y;
}

EckInline constexpr bool operator==(const D2D1_SIZE_F& sz1, const D2D1_SIZE_F& sz2)
{
	return sz1.width == sz2.width && sz1.height == sz2.height;
}

EckInline constexpr bool operator==(const D2D1_SIZE_U& sz1, const D2D1_SIZE_U& sz2)
{
	return sz1.width == sz2.width && sz1.height == sz2.height;
}

template<class T1, class T2>
EckInline constexpr T1 ReinterpretValue(T2 v)
{
	return std::bit_cast<T1>(v);
}

EckInline BOOL FloatEqual(float f1, float f2, float fEpsilon = FLT_EPSILON)
{
	return fabs(f1 - f2) < fEpsilon;
}

EckInline BOOL FloatEqual(double f1, double f2, double fEpsilon = DBL_EPSILON)
{
	return abs(f1 - f2) < fEpsilon;
}

EckInline constexpr BOOL PtInCircle(D2D1_POINT_2F pt, D2D1_POINT_2F ptCenter, float fRadius)
{
	return (pt.x - ptCenter.x) * (pt.x - ptCenter.x) + (pt.y - ptCenter.y) * (pt.y - ptCenter.y) <= 
		fRadius * fRadius;
}

EckInline constexpr BOOL PtInCircle(POINT pt, POINT ptCenter, int iRadius)
{
	return (pt.x - ptCenter.x) * (pt.x - ptCenter.x) + (pt.y - ptCenter.y) * (pt.y - ptCenter.y) <=
		iRadius * iRadius;
}

EckInline constexpr BOOL IsColorLight(BYTE r, BYTE g, BYTE b)
{
	return 5 * g + 2 * r + b > 8 * 128;
}

EckInline constexpr BOOL IsColorLightArgb(ARGB argb)
{
	return IsColorLight(
		GetIntegerByte<2>(argb),
		GetIntegerByte<1>(argb),
		GetIntegerByte<0>(argb));
}

EckInline constexpr BOOL IsColorLightColorref(COLORREF cr)
{
	return IsColorLight(
		GetIntegerByte<0>(cr),
		GetIntegerByte<1>(cr),
		GetIntegerByte<2>(cr));
}

EckInline constexpr BOOL IsGuidEqu(REFGUID x1, REFGUID x2)
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

EckInline constexpr BOOL IsPszId(PCWSTR p)
{
	return (((UINT_PTR)p) & (~((UINT_PTR)0xFFFF))) == 0;
}

/// <summary>
/// 调整矩形以完全包含。
/// 函数以最小的距离偏移矩形使之完全处于参照矩形当中
/// </summary>
/// <param name="rc">要调整的矩形</param>
/// <param name="rcRef">参照矩形</param>
/// <returns>成功返回TRUE，失败返回FALSE</returns>
inline constexpr BOOL AdjustRectIntoAnother(RECT& rc, const RECT& rcRef)
{
	if (rc.right - rc.left > rcRef.right - rcRef.left ||
		rc.bottom - rc.top > rcRef.bottom - rcRef.top)
		return FALSE;
	int dxLeft = rcRef.left - rc.left;
	if (dxLeft < 0)
		dxLeft = INT_MAX;
	int dxRight = rc.right - rcRef.right;
	if (dxRight < 0)
		dxRight = INT_MAX;
	int dyTop = rcRef.top - rc.top;
	if (dyTop < 0)
		dyTop = INT_MAX;
	int dyBottom = rc.bottom - rcRef.bottom;
	if (dyBottom < 0)
		dyBottom = INT_MAX;
	OffsetRect(rc, std::min(dxLeft, dxRight), std::min(dyTop, dyBottom));
	return TRUE;
}

/// <summary>
/// 调整矩形以适应。
/// 函数等比例缩放矩形使之成为包含于参照矩形的最大居中矩形
/// </summary>
/// <param name="rc">要调整的矩形</param>
/// <param name="rcRef">参照矩形</param>
/// <returns>成功返回TRUE，失败返回FALSE</returns>
inline constexpr BOOL AdjustRectToFitAnother(RECT& rc, const RECT& rcRef)
{
	const int
		cxMax = rcRef.right - rcRef.left,
		cyMax = rcRef.bottom - rcRef.top,
		cx0 = rc.right - rc.left,
		cy0 = rc.bottom - rc.top;
	if (cxMax <= 0 || cyMax <= 0 || cx0 <= 0 || cy0 <= 0)
		return FALSE;
	int cx, cy;
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

	rc =
	{
		rcRef.left + (cxMax - cx) / 2,
		rcRef.top + (cyMax - cy) / 2,
		cx,
		cy
	};
	return TRUE;
}

EckInline constexpr MARGINS MakeMargin(int i)
{
	return { i,i,i,i };
}

EckInline constexpr MARGINS MakeMarginTopBottom(int i)
{
	return { 0,0,i,i };
}

EckInline constexpr MARGINS MakeMarginLeftRight(int i)
{
	return { i,i,0,0 };
}

EckInline constexpr MARGINS MakeMarginHV(int h, int v)
{
	return { h,h,v,v };
}

EckInline constexpr COLORREF AdjustColorrefLuma(COLORREF cr, int iPrecent)
{
	return RGB(
		std::min(GetRValue(cr) * iPrecent / 100, 0xFF),
		std::min(GetGValue(cr) * iPrecent / 100, 0xFF),
		std::min(GetBValue(cr) * iPrecent / 100, 0xFF));
}

EckInline constexpr ULARGE_INTEGER operator+(ULARGE_INTEGER x1, ULARGE_INTEGER x2)
{
	return ULARGE_INTEGER{ .QuadPart = x1.QuadPart + x2.QuadPart };
}

EckInline constexpr ULARGE_INTEGER operator+(ULARGE_INTEGER x1, ULONGLONG x2)
{
	return ULARGE_INTEGER{ .QuadPart = x1.QuadPart + x2 };
}

EckInline constexpr ULARGE_INTEGER operator-(ULARGE_INTEGER x1, ULARGE_INTEGER x2)
{
	return ULARGE_INTEGER{ .QuadPart = x1.QuadPart - x2.QuadPart };
}

EckInline constexpr ULARGE_INTEGER operator-(ULARGE_INTEGER x1, ULONGLONG x2)
{
	return ULARGE_INTEGER{ .QuadPart = x1.QuadPart - x2 };
}

EckInline constexpr std::strong_ordering operator<=>(ULARGE_INTEGER x1, ULARGE_INTEGER x2)
{
	return x1.QuadPart <=> x2.QuadPart;
}

EckInline constexpr bool operator==(ULARGE_INTEGER x1, ULARGE_INTEGER x2)
{
	return x1.QuadPart == x2.QuadPart;
}

EckInline constexpr bool operator==(ULARGE_INTEGER x1, ULONGLONG x2)
{
	return x1.QuadPart == x2;
}

EckInline constexpr std::strong_ordering operator<=>(ULARGE_INTEGER x1, ULONGLONG x2)
{
	return x1.QuadPart <=> x2;
}

EckInline constexpr LARGE_INTEGER operator+(LARGE_INTEGER x1, LARGE_INTEGER x2)
{
	return LARGE_INTEGER{ .QuadPart = x1.QuadPart + x2.QuadPart };
}

EckInline constexpr LARGE_INTEGER operator+(LARGE_INTEGER x1, LONGLONG x2)
{
	return LARGE_INTEGER{ .QuadPart = x1.QuadPart + x2 };
}

EckInline constexpr LARGE_INTEGER operator-(LARGE_INTEGER x1, LARGE_INTEGER x2)
{
	return LARGE_INTEGER{ .QuadPart = x1.QuadPart - x2.QuadPart };
}

EckInline constexpr LARGE_INTEGER operator-(LARGE_INTEGER x1, LONGLONG x2)
{
	return LARGE_INTEGER{ .QuadPart = x1.QuadPart - x2 };
}

EckInline constexpr LARGE_INTEGER& operator-=(LARGE_INTEGER& x1, LARGE_INTEGER x2)
{
	x1.QuadPart -= x2.QuadPart;
	return x1;
}

EckInline constexpr LARGE_INTEGER operator-=(LARGE_INTEGER& x1, LONGLONG x2)
{
	x1.QuadPart -= x2;
	return x1;
}

EckInline constexpr LARGE_INTEGER& operator+=(LARGE_INTEGER& x1, LARGE_INTEGER x2)
{
	x1.QuadPart += x2.QuadPart;
	return x1;
}

EckInline constexpr LARGE_INTEGER operator+=(LARGE_INTEGER& x1, LONGLONG x2)
{
	x1.QuadPart += x2;
	return x1;
}

EckInline constexpr std::strong_ordering operator<=>(LARGE_INTEGER x1, LARGE_INTEGER x2)
{
	return x1.QuadPart <=> x2.QuadPart;
}

EckInline constexpr std::strong_ordering operator<=>(LARGE_INTEGER x1, LONGLONG x2)
{
	return x1.QuadPart <=> x2;
}

EckInline constexpr LARGE_INTEGER ToLi(ULARGE_INTEGER x)
{
	return LARGE_INTEGER{ .QuadPart = (LONGLONG)x.QuadPart };
}

EckInline constexpr LARGE_INTEGER ToLi(LONGLONG x)
{
	return LARGE_INTEGER{ .QuadPart = x };
}

EckInline constexpr ULARGE_INTEGER ToUli(LARGE_INTEGER x)
{
	return ULARGE_INTEGER{ .QuadPart = (ULONGLONG)x.QuadPart };
}

EckInline constexpr ULARGE_INTEGER ToUli(ULONGLONG x)
{
	return ULARGE_INTEGER{ .QuadPart = x };
}

template<class T, size_t N>
EckInline constexpr void ArrAssign(T(&x1)[N], const T(&x2)[N])
{
	EckCounter(N, i)
		x1[i] = x2[i];
}
ECK_NAMESPACE_END