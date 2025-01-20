#pragma once
#define GDIPVER 0x110

#include "PrivateApi.h"
//#include <Windows.h>
//#include <windowsx.h>
//#include <Uxtheme.h>

#include <vsstyle.h>
#include <dwmapi.h>
#include <wincodec.h>
#include <dwrite.h>
#include <d2d1_1.h>
#include <dxgi1_2.h>
#include <d3d11.h>
#include <Shlwapi.h>
#include <ShlObj.h>
#include <commoncontrols.h>
//#include <CommCtrl.h>
#pragma warning(suppress:5260)
#include <gdiplus.h>

#include <assert.h>
#include <crtdbg.h>

#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <type_traits>
#include <execution>
#include <memory>
#include <optional>
#include <functional>
#include <span>
#include <array>
#include <variant>

#include ".\Detours\detours.h"

#if _MSVC_LANG > 201703L
#	define ECKCXX20 1
#elif _MSVC_LANG > 201402L
#	define ECKCXX20 0
#else
#	error "ECK Lib requires C++17 or later"
#endif

#ifndef ECKDPIAPI
#	if _WIN32_WINNT >= 0x0605
#		define ECKDPIAPI 1
#	else
#		define ECKDPIAPI 0
#		ifndef WM_DPICHANGED_BEFOREPARENT// 不会触发这些消息，但为了通过编译在此将其定义
#			define WM_DPICHANGED_BEFOREPARENT      0x02E2
#			define WM_DPICHANGED_AFTERPARENT       0x02E3
#			define WM_GETDPISCALEDSIZE             0x02E4
#		endif// !defined(WM_DPICHANGED_BEFOREPARENT)
#	endif// _WIN32_WINNT >= 0x0605
#else
#	define ECKDPIAPI 0
#	if _WIN32_WINNT < 0x0605
#		error "Dpi api requires _WIN32_WINNT >= 0x0605."
#	endif
#endif

#define ECK_NAMESPACE_BEGIN				namespace eck {
#define ECK_NAMESPACE_END				}
#define ECK_PRIV_NAMESPACE_BEGIN		namespace Priv {
#define ECK_PRIV_NAMESPACE_END			}
#define ECK_DUI_NAMESPACE_BEGIN			namespace Dui {
#define ECK_DUI_NAMESPACE_END			}
#define ECK_MEDIATAG_NAMESPACE_BEGIN	namespace MediaTag {
#define ECK_MEDIATAG_NAMESPACE_END		}

#pragma region Template
ECK_NAMESPACE_BEGIN
#if ECKCXX20
template <class T>
using RemoveCVRef_T = std::remove_cvref_t<T>;

template <class T>
struct RemoveCVRef
{
	using Type = std::remove_cvref_t<T>;
};

template<class T>
concept ccpIsIntOrEnum = std::is_integral_v<T> || std::is_enum_v<T>;

template<class T>
concept ccpIsComInterface = std::is_base_of_v<IUnknown, std::remove_cvref_t<T>>;

template<class T>
concept ccpIsInteger = std::integral<T>;
#else//	ECKCXX20
template <class T>
using RemoveCVRef_T = std::remove_cv_t<std::remove_reference_t<T>>;

template <class T>
struct RemoveCVRef
{
	using Type = RemoveCVRef_T<T>;
};

#pragma push_macro("ccpIsIntOrEnum")
#define ccpIsIntOrEnum class
#endif// ECKCXX20

template<ccpIsIntOrEnum T, bool = std::is_enum_v<T>>
struct UnderlyingType
{
	using Type = std::underlying_type_t<T>;
};

template<ccpIsIntOrEnum T>
struct UnderlyingType<T, false>
{
	using Type = T;
};

template<ccpIsIntOrEnum T>
using UnderlyingType_T = UnderlyingType<T>::Type;

#if !ECKCXX20
#pragma pop_macro("ccpIsIntOrEnum")
#endif// !ECKCXX20
ECK_NAMESPACE_END
#pragma endregion Template

#pragma region MacroTools
#define ECKPRIV_ECKWIDE2___(x)	L##x
// ANSI字符串到宽字符串
#define ECKWIDE(x)				ECKPRIV_ECKWIDE2___(x)

#define ECKPRIV_ECKTOSTR2___(x)	#x
// 到ANSI字符串
#define ECKTOSTR(x)				ECKPRIV_ECKTOSTR2___(x)
// 到宽字符串
#define ECKTOSTRW(x)			ECKWIDE(ECKPRIV_ECKTOSTR2___(x))

// [预定义] 当前函数名W
#define ECK_FUNCTIONW			ECKWIDE(__FUNCTION__)
// [预定义] 行号W
#define ECK_LINEW				ECKTOSTRW(__LINE__)
// [预定义] 当前文件W
#define ECK_FILEW				__FILEW__
#pragma endregion MacroTools

#pragma region Generator
// 强制内联
#define EckInline				__forceinline
// 强制内联，不丢弃返回值
#define EckInlineNd				__forceinline [[nodiscard]]
// 强制内联，不丢弃返回值，且constexpr
#define EckInlineNdCe			__forceinline [[nodiscard]] constexpr
// 强制内联，且constexpr
#define EckInlineCe				__forceinline constexpr

// 定义读写属性字段
#define ECKPROP(Getter, Setter) __declspec(property(get = Getter, put = Setter))
// 定义只读属性字段
#define ECKPROP_R(Getter)		__declspec(property(get = Getter))
// 定义只写属性字段
#define ECKPROP_W(Setter)		__declspec(property(put = Setter))

// 复制字符串字面量
#define EckCopyConstStringA(pszDst, Src) memcpy(pszDst, Src, ARRAYSIZE(Src))
// 复制宽字符串字面量
#define EckCopyConstStringW(pszDst, Src) wmemcpy(pszDst, Src, ARRAYSIZE(Src))

#define EckIsStartWithConstStringA(psz, sz) (strncmp(psz, sz, ARRAYSIZE(sz) - 1) == 0)
#define EckIsStartWithConstStringW(psz, sz) (wcsncmp(psz, sz, ARRAYSIZE(sz) - 1) == 0)
#define EckIsStartWithConstStringIA(psz, sz) (_strnicmp(psz, sz, ARRAYSIZE(sz) - 1) == 0)
#define EckIsStartWithConstStringIW(psz, sz) (_wcsnicmp(psz, sz, ARRAYSIZE(sz) - 1) == 0)

#define EckStrAndLen(Arr)		Arr, (ARRAYSIZE(Arr) - 1)
#define EckLenAndStr(Arr)		(ARRAYSIZE(Arr) - 1), Arr

// 计次循环
#define EckCounter(c, Var) \
	for(::eck::UnderlyingType_T<::eck::RemoveCVRef_T<decltype(c)>> Var = 0; Var < (c); ++Var)

#define ECKPRIV_CounterNVMakeVarName2___(Name)	\
	ECKPRIV_COUNT_##Name##___
#define ECKPRIV_CounterNVMakeVarName___(Name)	\
	ECKPRIV_CounterNVMakeVarName2___(Name)

// 计次循环，无变量名参数
#define EckCounterNV(c)			EckCounter((c), ECKPRIV_CounterNVMakeVarName___(__LINE__))

// 无限循环
#define EckLoop()				while (true)

// 可空
#define EckOpt(Type, Name)		std::optional<Type> Name
// 可空，默认为空
#define EckOptNul(Type, Name)	std::optional<Type> Name = std::nullopt

// 原子到PWSTR
#define ECKMAKEINTATOMW(i)		(PWSTR)((ULONG_PTR)((WORD)(i)))

// 自取反
#define ECKBOOLNOT(x)			((x) = !(x))

// lParam->POINT 用于处理鼠标消息
#define ECK_GET_PT_LPARAM(lParam)			{ GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam) }
#define ECK_GET_PT_LPARAM_F(lParam)			{ (float)GET_X_LPARAM(lParam), \
											(float)GET_Y_LPARAM(lParam) }

// lParam->size 用于处理WM_SIZE
#define ECK_GET_SIZE_LPARAM(cx,cy,lParam)	{ (cx) = LOWORD(lParam); (cy) = HIWORD(lParam); }

// 定义COM接口
#define ECK_COM_INTERFACE(iid)	__interface __declspec(uuid(iid))

// 不可达
#define ECK_UNREACHABLE			__assume(0)

// 定义GUID
#define ECK_GUID(l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
			{ l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }

#define ECK_DISABLE_COPY_MOVE(e)			\
			e(const e&) = delete;			\
			e& operator=(const e&) = delete;\
			e(e&&) = delete;				\
			e& operator=(e&&) = delete;

#define ECK_DISABLE_COPY_MOVE_DEF_CONS(e)	\
			e() = default;					\
			e(const e&) = delete;			\
			e& operator=(const e&) = delete;\
			e(e&&) = delete;				\
			e& operator=(e&&) = delete;

#define ECK_DISABLE_COPY(e)					\
			e(const e&) = delete;			\
			e& operator=(const e&) = delete;

#define ECK_DISABLE_COPY_DEF_CONS(e)		\
			e() = default;					\
			e(const e&) = delete;			\
			e& operator=(const e&) = delete;

#define ECK_ENUM_BIT_FLAGS(Type)								\
			EckInline constexpr Type operator&(Type a, Type b)  \
			{													\
				return Type((std::underlying_type_t<Type>)a &	\
					(std::underlying_type_t<Type>)b);			\
			}													\
			EckInline constexpr Type operator|(Type a, Type b)	\
			{													\
				return Type((std::underlying_type_t<Type>)a |	\
					(std::underlying_type_t<Type>)b);			\
			}													\
			EckInline constexpr Type operator~(Type a)			\
			{													\
				return Type(~(std::underlying_type_t<Type>)a);	\
			}													\
			EckInline constexpr Type operator^(Type a, Type b)	\
			{													\
				return Type((std::underlying_type_t<Type>)a ^	\
					(std::underlying_type_t<Type>)b);			\
			}													\
			EckInline constexpr Type& operator&=(Type& a, Type b)	\
			{													\
				a = a & b;										\
				return a;										\
			}													\
			EckInline constexpr Type& operator|=(Type& a, Type b)	\
			{													\
				a = a | b;										\
				return a;										\
			}													\
			EckInline constexpr Type& operator^=(Type& a, Type b)	\
			{													\
				a = a ^ b;										\
				return a;										\
			}

#define ECK_ENUM_BIT_FLAGS_FRIEND(Type)							\
			friend constexpr Type operator&(Type a, Type b);	\
			friend constexpr Type operator|(Type a, Type b);	\
			friend constexpr Type operator~(Type a);			\
			friend constexpr Type operator^(Type a, Type b);	\
			friend constexpr Type& operator&=(Type& a, Type b);	\
			friend constexpr Type& operator|=(Type& a, Type b);	\
			friend constexpr Type& operator^=(Type& a, Type b);

#if ECKCXX20
#define ECKLIKELY		[[likely]]
#define ECKUNLIKELY		[[unlikely]]
#define ECKNOUNIQUEADDR [[no_unique_address]]
#else//	!ECKCXX20
#define ECKLIKELY
#define ECKUNLIKELY
#define ECKNOUNIQUEADDR
#endif// ECKCXX20

#define ECK_DISABLE_ARITHMETIC_OVERFLOW_WARNING	__pragma(warning(disable:26451))
#define ECK_SUPPRESS_MISSING_ZERO_TERMINATION	__pragma(warning(suppress:6054))
#pragma endregion Generator

#ifdef _DEBUG
#define EckCheckMem(p)	\
			if (!(p))	\
			{			\
				OutputDebugStringW(L"内存分配失败: " ECK_FILEW L"(" ECK_LINEW L")\r\n"); \
				abort();\
			}
#else
#define EckCheckMem(p) ((void)(p))
#endif

ECK_NAMESPACE_BEGIN
inline namespace Literals
{
	EckInline constexpr WORD operator""_us(ULONGLONG x)
	{
		return (WORD)x;
	}

	EckInline constexpr short operator""_ss(ULONGLONG x)
	{
		return (short)x;
	}

	EckInline constexpr BYTE operator""_by(ULONGLONG x)
	{
		return (BYTE)x;
	}
}

#pragma region Type
inline namespace BaseType
{
	using SCHAR = signed char;
	using BITBOOL = BYTE;
	using PCBYTE = const BYTE*;
	using PCVOID = const void*;
	using ECKENUM = BYTE;
	using SSIZE_T = std::make_signed_t<SIZE_T>;
	using UINTBE = UINT;
	using PITEMIDLIST = LPITEMIDLIST;
	using PCITEMIDLIST = LPCITEMIDLIST;
}

union BIT128
{
	UINT64 u64[2];
	UINT32 u32[4];
	UINT16 u16[8];
	BYTE u8[16];
};

struct MD5
{
	BIT128 v;
};

union GPARGB
{
	struct
	{
		BYTE b, g, r, a;
	};
	DWORD dw;
};

union GDIARGB
{
	struct
	{
		BYTE r, g, b, a;
	};
	DWORD dw;
};

union BIT256
{
	BIT128 bit128[2];
	UINT64 u64[4];
	UINT32 u32[8];
	UINT16 u16[16];
	BYTE u8[32];
};

// 左顶宽高矩形
struct RCWH
{
	int x;
	int y;
	int cx;
	int cy;
};

// NMCD扩展
struct NMCUSTOMDRAWEXT :NMCUSTOMDRAW
{
	int iStateId;
	int iPartId;
	COLORREF crText;
	COLORREF crBk;
};

// 鼠标类通知（NM_CLICK等）
struct NMMOUSENOTIFY
{
	NMHDR nmhdr;
	POINT pt;
	UINT uKeyFlags;
};

// 焦点通知
struct NMFOCUS
{
	NMHDR nmhdr;
	HWND hWnd;
};

struct NTVER
{
	ULONG uMajor;
	ULONG uMinor;
	ULONG uBuild;
};

inline namespace GpNameSpace
{
	using namespace Gdiplus::DllExports;
#define ECK_USING_GDIP_TYPE(Type) using Type = ::Gdiplus::Type

	ECK_USING_GDIP_TYPE(GpGraphics);

	ECK_USING_GDIP_TYPE(GpBrush);
	ECK_USING_GDIP_TYPE(GpTexture);
	ECK_USING_GDIP_TYPE(GpSolidFill);
	ECK_USING_GDIP_TYPE(GpLineGradient);
	ECK_USING_GDIP_TYPE(GpPathGradient);
	ECK_USING_GDIP_TYPE(GpHatch);

	ECK_USING_GDIP_TYPE(GpPen);
	ECK_USING_GDIP_TYPE(GpCustomLineCap);
	ECK_USING_GDIP_TYPE(GpAdjustableArrowCap);

	ECK_USING_GDIP_TYPE(GpImage);
	ECK_USING_GDIP_TYPE(GpBitmap);
	ECK_USING_GDIP_TYPE(GpMetafile);
	ECK_USING_GDIP_TYPE(GpImageAttributes);
	using GpEffect = Gdiplus::CGpEffect;

	ECK_USING_GDIP_TYPE(GpPath);
	ECK_USING_GDIP_TYPE(GpRegion);
	ECK_USING_GDIP_TYPE(GpPathIterator);

	ECK_USING_GDIP_TYPE(GpFontFamily);
	ECK_USING_GDIP_TYPE(GpFont);
	ECK_USING_GDIP_TYPE(GpStringFormat);
	ECK_USING_GDIP_TYPE(GpFontCollection);
	ECK_USING_GDIP_TYPE(GpInstalledFontCollection);
	ECK_USING_GDIP_TYPE(GpPrivateFontCollection);

	ECK_USING_GDIP_TYPE(GpCachedBitmap);

	ECK_USING_GDIP_TYPE(ARGB);
	ECK_USING_GDIP_TYPE(REAL);
	ECK_USING_GDIP_TYPE(GpStatus);
	ECK_USING_GDIP_TYPE(GpRectF);
	ECK_USING_GDIP_TYPE(GpRect);
	ECK_USING_GDIP_TYPE(GpPointF);
	ECK_USING_GDIP_TYPE(GpPoint);
	ECK_USING_GDIP_TYPE(GpSizeF);
	using GpSize = Gdiplus::Size;
	ECK_USING_GDIP_TYPE(GpMatrix);
	ECK_USING_GDIP_TYPE(GdiplusStartupInput);
	ECK_USING_GDIP_TYPE(GpFillMode);
	using GpColor = Gdiplus::Color;

	using Gdiplus::GdiplusShutdown;
	using Gdiplus::GdiplusStartup;

	using GpBlurParams = Gdiplus::BlurParams;
	using GpSharpenParams = Gdiplus::SharpenParams;
	using GpTintParams = Gdiplus::TintParams;
	using GpRedEyeCorrectionParams = Gdiplus::RedEyeCorrectionParams;
	using GpColorMatrix = Gdiplus::ColorMatrix;
	using GpColorLUTParams = Gdiplus::ColorLUTParams;
	using GpBrightnessContrastParams = Gdiplus::BrightnessContrastParams;
	using GpHSLParams = Gdiplus::HueSaturationLightnessParams;
	using GpColorBalanceParams = Gdiplus::ColorBalanceParams;
	using GpLevelsParams = Gdiplus::LevelsParams;
	using GpColorCurveParams = Gdiplus::ColorCurveParams;
}
#pragma endregion Type

#pragma region Const
// 控件序列化数据对齐
#ifdef _WIN64
#	define ECK_CTRLDATA_ALIGN	8
#else
#	define ECK_CTRLDATA_ALIGN	4
#endif

constexpr inline auto CchI32ToStrBufNoRadix2 = std::max({
	_MAX_ITOSTR_BASE16_COUNT,_MAX_ITOSTR_BASE10_COUNT,_MAX_ITOSTR_BASE8_COUNT,
	_MAX_LTOSTR_BASE16_COUNT ,_MAX_LTOSTR_BASE10_COUNT ,_MAX_LTOSTR_BASE8_COUNT,
	_MAX_ULTOSTR_BASE16_COUNT,_MAX_ULTOSTR_BASE10_COUNT,_MAX_ULTOSTR_BASE8_COUNT });
constexpr inline auto CchI64ToStrBufNoRadix2 = std::max({
	_MAX_I64TOSTR_BASE16_COUNT,_MAX_I64TOSTR_BASE10_COUNT,_MAX_I64TOSTR_BASE8_COUNT,
	_MAX_U64TOSTR_BASE16_COUNT,_MAX_U64TOSTR_BASE10_COUNT,_MAX_U64TOSTR_BASE8_COUNT });

constexpr inline auto CchI32ToStrBuf = std::max({ CchI32ToStrBufNoRadix2,
	_MAX_ITOSTR_BASE2_COUNT,_MAX_LTOSTR_BASE2_COUNT,_MAX_ULTOSTR_BASE2_COUNT });
constexpr inline auto CchI64ToStrBuf = std::max({ CchI64ToStrBufNoRadix2,
	_MAX_I64TOSTR_BASE2_COUNT,_MAX_U64TOSTR_BASE2_COUNT });

constexpr inline double Pi = 3.141592653589793;
constexpr inline float PiF = static_cast<float>(Pi);

constexpr inline UINT CP_UTF16LE = 0xFFFFFFFF;
constexpr inline UINT CP_UTF16BE = 0xFFFFFFFE;
constexpr inline UINT CP_ASCII = 0xFFFFFFFD;

constexpr inline BYTE BOM_UTF16LE[]{ 0xFF,0xFE };
constexpr inline BYTE BOM_UTF16BE[]{ 0xFE,0xFF };
constexpr inline BYTE BOM_UTF8[]{ 0xEF,0xBB,0xBF };

constexpr inline COLORREF c_crDarkWnd = RGB(32, 32, 32);
constexpr inline COLORREF c_crDarkBtnFace = 0x383838;

constexpr inline UINT Neg1U{ (UINT)-1 };

constexpr inline LARGE_INTEGER LiZero{};
#if ECKCXX20
constexpr inline ULARGE_INTEGER UliMax{ .QuadPart = 0xFFFF'FFFF'FFFF'FFFF };
#else
constexpr inline ULARGE_INTEGER UliMax{ 0xFFFF'FFFF, 0xFFFF'FFFF };
#endif

constexpr inline size_t SizeTMax{ std::numeric_limits<size_t>::max() };
constexpr inline SIZE_T SIZETMax{ (SIZE_T)SizeTMax };

#ifdef _DEBUG
constexpr inline BOOL Dbg{ TRUE };
#else
constexpr inline BOOL Dbg{ FALSE };
#endif

#ifdef _WIN64
constexpr inline BOOL Win64{ TRUE };
#else
constexpr inline BOOL Win64{ FALSE };
#endif

constexpr inline BLENDFUNCTION BlendFuncAlpha{ AC_SRC_OVER,0,255,AC_SRC_ALPHA };

template<BYTE Alpha>
constexpr inline BLENDFUNCTION BlendFuncAlphaN{ AC_SRC_OVER,0,Alpha,AC_SRC_ALPHA };

constexpr inline BYTE ColorFillAlpha{ 80 };

constexpr inline int MetricsExtraV{ 8 };

constexpr inline UINT WM_USER_SAFE{ WM_USER + 3 };

constexpr inline UINT CS_STDWND{ CS_DBLCLKS | CS_VREDRAW | CS_HREDRAW };

constexpr inline HRESULT HrNotFound{ HRESULT_FROM_WIN32(ERROR_NOT_FOUND) };

constexpr inline PCWSTR WCN_DLG = L"Eck.WndClass.CommDlg";

constexpr inline PCWSTR WCN_DUMMY = L"Eck.WndClass.Dummy";
#ifdef ECK_OPT_NO_SIMPLE_WND_CLS
constexpr inline PCWSTR WCN_LABEL = L"Eck.WndClass.Label";
constexpr inline PCWSTR WCN_BK = L"Eck.WndClass.BK";
constexpr inline PCWSTR WCN_LUNARCALENDAR = L"Eck.WndClass.LunarCalendar";
constexpr inline PCWSTR WCN_FORM = L"Eck.WndClass.Form";
constexpr inline PCWSTR WCN_TABHEADER = L"Eck.WndClass.TabHeader";
constexpr inline PCWSTR WCN_SPLITBAR = L"Eck.WndClass.SplitBar";
constexpr inline PCWSTR WCN_DRAWPANEL = L"Eck.WndClass.DrawPanel";
constexpr inline PCWSTR WCN_LISTBOXNEW = L"Eck.WndClass.ListBoxNew";
constexpr inline PCWSTR WCN_TREELIST = L"Eck.WndClass.TreeList";
constexpr inline PCWSTR WCN_COMBOBOXNEW = L"Eck.WndClass.ComboBoxNew";
constexpr inline PCWSTR WCN_PICTUREBOX = L"Eck.WndClass.PictureBox";
constexpr inline PCWSTR WCN_DUIHOST = L"Eck.WndClass.DuiHost";
constexpr inline PCWSTR WCN_VECDRAWPANEL = L"Eck.WndClass.VectorDrawPanel";
constexpr inline PCWSTR WCN_HEXEDIT = L"Eck.WndClass.HexEdit";
constexpr inline PCWSTR WCN_HITTER = L"Eck.WndClass.Hitter";
#else
constexpr inline PCWSTR WCN_LABEL = WCN_DUMMY;
constexpr inline PCWSTR WCN_BK = WCN_DUMMY;
constexpr inline PCWSTR WCN_LUNARCALENDAR = WCN_DUMMY;
constexpr inline PCWSTR WCN_FORM = WCN_DUMMY;
constexpr inline PCWSTR WCN_TABHEADER = WCN_DUMMY;
constexpr inline PCWSTR WCN_SPLITBAR = WCN_DUMMY;
constexpr inline PCWSTR WCN_DRAWPANEL = WCN_DUMMY;
constexpr inline PCWSTR WCN_LISTBOXNEW = WCN_DUMMY;
constexpr inline PCWSTR WCN_TREELIST = WCN_DUMMY;
constexpr inline PCWSTR WCN_COMBOBOXNEW = WCN_DUMMY;
constexpr inline PCWSTR WCN_PICTUREBOX = WCN_DUMMY;
constexpr inline PCWSTR WCN_DUIHOST = WCN_DUMMY;
constexpr inline PCWSTR WCN_VECDRAWPANEL = WCN_DUMMY;
constexpr inline PCWSTR WCN_HEXEDIT = WCN_DUMMY;
constexpr inline PCWSTR WCN_HITTER = WCN_DUMMY;
#endif// defined(ECK_OPT_NO_SIMPLE_WND_CLS)

constexpr inline PCWSTR MSGREG_FORMTRAY = L"Eck.Message.FormTray";

constexpr inline UINT SCID_DESIGN = 20230621'01u;
#pragma endregion Const

#pragma region Enum
enum class InitStatus
{
	Ok,
	RegWndClassError,
	GdiplusInitError,
	WicFactoryError,
	D2dFactoryError,
	DxgiDeviceError,
	DWriteFactoryError,
	D3dDeviceError,
};

enum class Align :BYTE
{
	Near,
	Center,
	Far
};

// For AnimateWindow
enum class AnimateStyle :BYTE
{
	Roll,	// 滚动
	Slide,	// 滑动
	Center,	// 折叠
	Blend	// 淡入淡出
};

enum class ClrPart :BYTE
{
	Text,
	Bk,
	TextBk,
	Border,
};

#pragma warning(suppress:26454)// 算术溢出
constexpr inline UINT NM_FIRST_ECK = (0u - 0x514Bu * 0x514Bu);
enum :UINT// 控件通知代码
{
	ECKPRIV_NM_FIRST_PLACEHOLDER = NM_FIRST_ECK,
	NM_CLP_CLRCHANGED,		// NMCLPCLRCHANGED
	NM_SPB_DRAGGED,			// NMSPBDRAGGED
	NM_TGL_TASKCLICKED,		// NMTGLCLICKED

	NM_TL_FILLCHILDREN,		// NMTLFILLCHILDREN
	NM_TL_GETDISPINFO,		// NMTLGETDISPINFO
	NM_TL_ITEMEXPANDING,	// NMTLCOMMITEM
	NM_TL_ITEMEXPANDED,		// NMTLCOMMITEM
	NM_TL_HD_CLICK,			// NMHEADER
	NM_TL_FILLALLFLATITEM,	// NMTLFILLALLFLATITEM
	NM_TL_TTGETDISPINFO,	// NMTLTTGETDISPINFO
	NM_TL_TTPRESHOW,		// NMTLTTPRESHOW
	NM_TL_PREEDIT,			// NMTLEDIT
	NM_TL_POSTEDIT,			// NMTLEDIT
	NM_TL_MOUSECLICK,		// NMTLMOUSECLICK
	NM_TL_ITEMCHECKING,		// NMTLCOMMITEM
	NM_TL_ITEMCHECKED,		// NMTLCOMMITEM
	NM_TL_BEGINDRAG,		// NMTLDRAG
	NM_TL_ENDDRAG,			// NMTLDRAG

	NM_LBN_GETDISPINFO,		// NMLBNGETDISPINFO
	NM_LBN_BEGINDRAG,		// NMLBNDRAG
	NM_LBN_ENDDRAG,			// NMLBNDRAG
	NM_LBN_DISMISS,			// NMHDR
	NM_LBN_ITEMCHANGED,		// NMLBNITEMCHANGED
	NM_LBN_ITEMSTANDBY,		// NMHDR
	NM_LBN_SEARCH,			// NMLBNSEARCH

	NM_PKB_OWNERDRAW,		// NMPKBOWNERDRAW
	NM_HTT_SEL,				// NMHTTSEL
};
/*
* 对于ECK控件，部分标准通知对应的结构如下（可能有特定控件会扩展这些结构）
* NM_SETFOCUS			NMFOUCS
* NM_KILLFOCUS			NMFOCUS
* NM_RCLICK				NMMOUSENOTIFY
* NM_CUSTOMDRAW			NMCUSTOMDRAWEXT
*/

// 消息钩子ID保留范围[1, 511]，此范围仅供内部使用
constexpr inline UINT_PTR MsgHookIdUserBegin = 512;
enum :UINT_PTR
{
	MHI_SCROLLBAR_HOOK = 1,
	MHI_HEADER_HOOK,
	MHI_LISTVIEW_ROWHEIGHT,
	MHI_LVE_HEADER_HEIGHT,
	MHI_DUI_TITLEBAR,
	MHI_UXF_MENU,
};

// 构建号
enum :ULONG
{
	WINVER_1607 = 14393,
	WINVER_1809 = 17763,
	WINVER_1903 = 18362,
	WINVER_11_21H2 = 22000,
};

enum DispInfoMask :UINT
{
	DIM_TEXT = 1u << 0,
	DIM_IMAGE = 1u << 1,
	DIM_STATE = 1u << 2,
	DIM_LPARAM = 1u << 3,
};
ECK_ENUM_BIT_FLAGS(DispInfoMask);
#pragma endregion Enum

#pragma region Global
extern NTVER g_NtVer;

extern HINSTANCE g_hInstance;
extern IWICImagingFactory* g_pWicFactory;
extern ID2D1Factory1* g_pD2dFactory;
extern IDWriteFactory* g_pDwFactory;
extern ID2D1Device* g_pD2dDevice;
extern IDXGIDevice1* g_pDxgiDevice;
extern IDXGIFactory2* g_pDxgiFactory;

extern HMODULE g_hModComCtl32;
#pragma endregion Global

#pragma region Init
enum :UINT
{
	EIF_DEFAULT = 0,
	// 不调用ThreadInit
	EIF_NOINITTHREAD = 1u << 0,
	// 不初始化GDI+
	EIF_NOINITGDIPLUS = 1u << 1,
	// 不初始化WIC
	EIF_NOINITWIC = 1u << 2,
	// 不初始化D2D
	EIF_NOINITD2D = 1u << 3,
	// 不初始化DWrite
	EIF_NOINITDWRITE = 1u << 4,
	// 不适配暗色模式
	EIF_NODARKMODE = 1u << 5,
};

constexpr inline D3D_FEATURE_LEVEL c_uDefD3dFeatureLevel[]
{
	D3D_FEATURE_LEVEL_11_1,
	D3D_FEATURE_LEVEL_11_0,
	D3D_FEATURE_LEVEL_10_1,
	D3D_FEATURE_LEVEL_10_0,
	D3D_FEATURE_LEVEL_9_3,
	D3D_FEATURE_LEVEL_9_2,
	D3D_FEATURE_LEVEL_9_1
};

struct INITPARAM
{
	UINT uFlags = EIF_DEFAULT;
	D2D1_FACTORY_TYPE uD2dFactoryType = D2D1_FACTORY_TYPE_MULTI_THREADED;
	DWRITE_FACTORY_TYPE uDWriteFactoryType = DWRITE_FACTORY_TYPE_SHARED;
	const D3D_FEATURE_LEVEL* pD3dFeatureLevel = c_uDefD3dFeatureLevel;
	UINT cD3dFeatureLevel = ARRAYSIZE(c_uDefD3dFeatureLevel);
};

/// <summary>
/// 初始化ECK Lib。
/// 使用任何ECK功能之前需调用该函数。仅允许调用一次。
/// 函数将在内部调用eck::ThreadInit，除非设置了EIF_NOINITTHREAD
/// </summary>
/// <param name="hInstance">实例句柄，所有自定义窗口类将在此实例上注册</param>
/// <param name="pInitParam">指向初始化参数的可选指针</param>
/// <param name="pdwErrCode">指向接收错误码变量的可选指针</param>
/// <returns>错误代码</returns>
InitStatus Init(HINSTANCE hInstance, const INITPARAM* pInitParam = nullptr, DWORD* pdwErrCode = nullptr);

void UnInit();

PCWSTR InitStatusToString(InitStatus iStatus);
#pragma endregion Init

ECK_NAMESPACE_END
#include "DbgHelper.h"
ECK_NAMESPACE_BEGIN
#pragma region Thread
class CWnd;
struct THREADCTX;

using FQueueCallback = void(*)(void* pCtx);
namespace Priv
{
	struct QueuedCallback
	{
		UINT nPriority;	// 值越小优先级越高
		std::variant<std::function<void()>, void*> Callback;

		constexpr std::weak_ordering operator<=>(const QueuedCallback& x) const
		{
			return nPriority <=> x.nPriority;
		}
	};

	struct QueuedCallbackQueue
	{
		DWORD dwTid{};
		std::priority_queue<QueuedCallback> q{};
		RTL_SRWLOCK Lk{};

		QueuedCallbackQueue()
		{
			RtlInitializeSRWLock(&Lk);
			dwTid = NtCurrentThreadId32();
		}

		template<class F>
		void EnQueueCallback(F&& fnCallback, UINT nPriority = UINT_MAX, BOOL bWakeUiThread = TRUE)
		{
			RtlAcquireSRWLockExclusive(&Lk);
			q.push({ nPriority,std::function<void()>(std::forward<F>(fnCallback)) });
			RtlReleaseSRWLockExclusive(&Lk);
			if (bWakeUiThread)
				PostThreadMessage(dwTid, WM_NULL, 0, 0);
		}

		void EnQueueCoroutine(void* pCoroutine, UINT nPriority = UINT_MAX, BOOL bWakeUiThread = TRUE)
		{
			EckAssert(pCoroutine);
			RtlAcquireSRWLockExclusive(&Lk);
			q.push({ nPriority,pCoroutine });
			RtlReleaseSRWLockExclusive(&Lk);
			if (bWakeUiThread)
				PostThreadMessage(dwTid, WM_NULL, 0, 0);
		}
	};
}

using FWndCreating = void(*)(HWND hWnd, CBT_CREATEWNDW* pcs, THREADCTX* pThreadCtx);
struct THREADCTX
{
	//-------窗口映射
	std::unordered_map<HWND, CWnd*> hmWnd{};	// HWND->CWnd*
	std::unordered_map<HWND, CWnd*> hmTopWnd{};	// 顶级窗口映射
	HHOOK hhkTempCBT{};					// CBT钩子句柄
	CWnd* pCurrWnd{};					// 当前正在创建窗口所属的CWnd指针
	FWndCreating pfnWndCreatingProc{};	// 当前创建窗口时要调用的过程
	//-------暗色处理
	HHOOK hhkCbtDarkMode{};	// 启用暗色支持CBT钩子句柄
	COLORREF crDefText{};	// 默认前景色
	COLORREF crDefBkg{};	// 默认背景色
	COLORREF crDefBtnFace{};// 默认BtnFace颜色
	COLORREF crBlue1{};		// 蓝色
	COLORREF crGray1{};		// 灰色
	COLORREF crTip1{};		// 提示颜色
	COLORREF crHiLightText{};// 高亮文本颜色，用于适配高对比度主题

	// 是否允许暗色CBT钩子设置窗口，设为FALSE可暂停Hook。
	// 注意：务必在打开文件对话框前暂停Hook
	BITBOOL bEnableDarkModeHook : 1{ TRUE };
	BITBOOL bAutoNcDark : 1{ TRUE };	// 自动调整非客户区暗色
	//-------回调队列
	Priv::QueuedCallbackQueue Callback{};

	EckInline void WmAdd(HWND hWnd, CWnd* pWnd)
	{
		EckAssert(IsWindow(hWnd) && pWnd);
		hmWnd.insert(std::make_pair(hWnd, pWnd));
	}

	EckInline void WmRemove(HWND hWnd)
	{
		const auto it = hmWnd.find(hWnd);
		if (it != hmWnd.end())
			hmWnd.erase(it);
#ifdef _DEBUG
		else
			EckDbgPrintFmt(L"** WARNING ** 从窗口映射中移除%p时失败。", hWnd);
#endif
	}

	EckInline CWnd* WmAt(HWND hWnd) const
	{
		const auto it = hmWnd.find(hWnd);
		if (it != hmWnd.end())
			return it->second;
		else
			return nullptr;
	}

	EckInline void TwmAdd(HWND hWnd, CWnd* pWnd)
	{
		EckAssert(IsWindow(hWnd) && pWnd);
		EckAssert((GetWindowLongPtrW(hWnd, GWL_STYLE) & WS_CHILD) != WS_CHILD);
		hmTopWnd.insert(std::make_pair(hWnd, pWnd));
	}

	EckInline void TwmRemove(HWND hWnd)
	{
		const auto it = hmTopWnd.find(hWnd);
		if (it != hmTopWnd.end())
			hmTopWnd.erase(it);
	}

	EckInline CWnd* TwmAt(HWND hWnd) const
	{
		const auto it = hmTopWnd.find(hWnd);
		if (it != hmTopWnd.end())
			return it->second;
		else
			return nullptr;
	}

	void SetNcDarkModeForAllTopWnd(BOOL bDark);

	void UpdateDefColor();

	void SendThemeChangedToAllTopWindow();
};

// 取线程上下文TLS槽
DWORD GetThreadCtxTlsSlot();

// 初始化线程上下文。
// 在调用线程上初始化线程上下文，在使用任何ECK窗口功能前必须调用此函数
void ThreadInit();

// 反初始化线程上下文。
// 调用此函数后不允许使用任何ECK窗口对象
void ThreadUnInit();

// 取线程上下文
EckInline THREADCTX* GetThreadCtx()
{
	return (THREADCTX*)TlsGetValue(GetThreadCtxTlsSlot());
}

// 窗口句柄到CWnd指针
EckInline CWnd* CWndFromHWND(HWND hWnd)
{
	return GetThreadCtx()->WmAt(hWnd);
}

HHOOK BeginCbtHook(CWnd* pCurrWnd, FWndCreating pfnCreatingProc = nullptr);

void EndCbtHook();

/// <summary>
/// 过滤消息。
/// 若使用了任何ECK窗口对象，则必须在翻译按键和派发消息之前调用此函数
/// </summary>
/// <param name="Msg">即将处理的消息</param>
/// <returns>若返回值为TRUE，则不应继续处理消息；否则应正常进行剩余步骤</returns>
BOOL PreTranslateMessage(const MSG& Msg);
#pragma endregion Thread

void InitPrivateApi();

// For compatibility.
EckInline constexpr void SetMsgFilter(void*) {}

HRESULT UxfMenuInit(CWnd* pWnd);

HRESULT UxfMenuUnInit(CWnd* pWnd);
ECK_NAMESPACE_END

#ifndef ECK_OPT_NO_USING_GDIPLUS
using namespace eck::GpNameSpace;
#endif// !define(ECK_OPT_NO_USING_GDIPLUS)

#ifndef ECK_OPT_NO_USING_BASE_TYPES
using namespace eck::BaseType;
#endif// !define(ECK_OPT_NO_USING_BASE_TYPES)