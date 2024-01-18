/*
* WinEzCtrlKit Library
*
* Utility.h ： 实用函数
*
* Copyright(C) 2023 QingKong
*/

#pragma once
#include "ECK.h"

#include <math.h>
#include <time.h>
#include <process.h>

#include <string_view>
#if ECKCXX20
#include <concepts>
#endif

#include <windowsx.h>
#include <d2d1_1.h>
#include <Shlwapi.h>

ECK_NAMESPACE_BEGIN
template <class T1, class T2, bool = std::is_empty_v<T1> && !std::is_final_v<T1>>
struct CompressedPair final : private T1
{
	T2 v2;

	constexpr T1& GetFirst() noexcept { return *this; }

	constexpr const T1& GetFirst() const noexcept { return *this; }
};

template <class T1, class T2>
struct CompressedPair<T1, T2, false> final
{
	T1 v1;
	T2 v2;

	EckInline constexpr T1& GetFirst() noexcept { return v1; }

	EckInline constexpr const T1& GetFirst() const noexcept { return v1; }
};


namespace Colorref
{
	inline constexpr COLORREF
		Red               = 0x0000FF,// 红色
		Green             = 0x00FF00,// 绿色
		Blue              = 0xFF0000,// 蓝色
		Yellow            = 0x00FFFF,// 黄色
		Magenta           = 0xFF00FF,// 品红/洋红
		Cyan              = 0xFFFF00,// 艳青/青色

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
	return (TRet)(... | fn(by));
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

EckInline constexpr COLORREF ARGBToColorref(ARGB argb)
{
	return ReverseColorref(argb);
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

EckInline BOOL IsFILETIMEZero(const FILETIME* pft)
{
	return pft->dwLowDateTime == 0 && pft->dwHighDateTime == 0;
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

EckInline constexpr bool operator==(const RECT& rc1, const RECT& rc2)
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

template<class T>
EckInline constexpr BOOL Sign(T v)
{
	return v >= 0;
}

EckInline constexpr SIZE_T Cch2Cb(int cch)
{
	return (cch + 1) * sizeof(WCHAR);
}

EckInline constexpr D2D1_RECT_F MakeD2DRcF(const RECT& rc)
{
	return D2D1_RECT_F{ (float)rc.left, (float)rc.top, (float)rc.right, (float)rc.bottom };
}

EckInline constexpr RECT MakeRect(const D2D1_RECT_F& rc)
{
	return { (LONG)rc.left, (LONG)rc.top, (LONG)rc.right, (LONG)rc.bottom };
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

EckInline constexpr GpRectF ToGpRectF(const RECT& rc)
{
	return { (REAL)rc.left,(REAL)rc.top,(REAL)(rc.right - rc.left),(REAL)(rc.bottom - rc.top) };
}

EckInline constexpr HRESULT HResultFromBool(BOOL b)
{
	return b ? S_OK : S_FALSE;
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

EckInline size_t Fnv1aHash(PCBYTE p, size_t cb)
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
ECK_NAMESPACE_END