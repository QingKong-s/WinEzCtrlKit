/*
* WinEzCtrlKit Library
*
* ECK.h ： 公共头文件
*
* Copyright(C) 2023-2024 QingKong
*/
#pragma once
#define GDIPVER 0x110

#include <Windows.h>
#include <Uxtheme.h>
#include <vsstyle.h>
#include <dwmapi.h>
#include <wincodec.h>
#include <dwrite.h>
#include <d2d1_1.h>
#include <dxgi1_2.h>
#include <Shlwapi.h>
#include <d3d11.h>
#include <windowsx.h>

#include <assert.h>

#include <unordered_map>
#include <unordered_set>
#include <type_traits>
#include <execution>

#include ".\Detours\detours.h"

#pragma region 宏
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
#		endif// WM_DPICHANGED_BEFOREPARENT
#	endif// _WIN32_WINNT >= 0x0605
#else
#	if _WIN32_WINNT < 0x0605
#		error "Dpi api requires _WIN32_WINNT >= 0x0605 !!!"
#	endif
#endif


#define ECK_NAMESPACE_BEGIN			namespace eck {
#define ECK_NAMESPACE_END			}
#define ECK_PRIV_NAMESPACE_BEGIN	namespace EckPriv {
#define ECK_PRIV_NAMESPACE_END		}

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

#pragma pop_macro("ccpIsIntOrEnum")

ECK_NAMESPACE_END

#define EckInline				__forceinline

// 控件序列化数据对齐
#ifdef _WIN64
#	define ECK_CTRLDATA_ALIGN	8
#else
#	define ECK_CTRLDATA_ALIGN	4
#endif

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

// 定义读写属性字段
#define ECKPROP(Getter, Setter) __declspec(property(get = Getter, put = Setter))
// 定义只读属性字段
#define ECKPROP_R(Getter)		__declspec(property(get = Getter))
// 定义只写属性字段
#define ECKPROP_W(Setter)		__declspec(property(put = Setter))



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
#define ECKMAKEINTATOMW(i) (PWSTR)((ULONG_PTR)((WORD)(i)))

// 自取反
#define ECKBOOLNOT(x) ((x) = !(x))

// lParam->POINT 用于处理鼠标消息   e.g. POINT pt ECK_GET_PT_LPARAM(lParam);
#define ECK_GET_PT_LPARAM(lParam) { GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam) }
#define ECK_GET_PT_LPARAM_F(lParam) { (float)GET_X_LPARAM(lParam),(float)GET_Y_LPARAM(lParam) }

// lParam->size 用于处理WM_SIZE   e.g. ECK_GET_SIZE_LPARAM(cxClient, cyClient, lParam);
#define ECK_GET_SIZE_LPARAM(cx,cy,lParam) { (cx) = LOWORD(lParam); (cy) = HIWORD(lParam); }

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

#define ECK_COM_INTERFACE(iid)				\
			__interface __declspec(uuid(iid))

#define ECK_UNREACHABLE __assume(0)

#define ECK_GUID(l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
			{ l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }

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
			}

#define ECK_ENUM_BIT_FLAGS_FRIEND(Type)							\
			friend constexpr Type operator&(Type a, Type b);	\
			friend constexpr Type operator|(Type a, Type b);	\
			friend constexpr Type operator~(Type a);			\
			friend constexpr Type operator^(Type a, Type b);

#if ECKCXX20
#define ECKLIKELY [[likely]]
#define ECKUNLIKELY [[unlikely]]
#define ECKNOUNIQUEADDR [[no_unique_address]]
#else//	!ECKCXX20
#define ECKLIKELY
#define ECKUNLIKELY
#define ECKNOUNIQUEADDR
#endif// ECKCXX20
#pragma endregion

#define ECK_DISABLE_ARITHMETIC_OVERFLOW_WARNING __pragma(warning(disable:26451))

ECK_NAMESPACE_BEGIN
#pragma region 类型
using SCHAR = signed char;
using BITBOOL = UINT;
using PCBYTE = const BYTE*;
using PCVOID = const void*;
using ECKENUM = BYTE;
using SSIZE_T = std::make_signed_t<SIZE_T>;
using UINTBE = UINT;
#pragma endregion

namespace Literals
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
using namespace Literals;

enum class Align
{
	Near,
	Center,
	Far
};

constexpr inline auto c_cchI32ToStrBufNoRadix2 = std::max({
	_MAX_ITOSTR_BASE16_COUNT,_MAX_ITOSTR_BASE10_COUNT,_MAX_ITOSTR_BASE8_COUNT,
	_MAX_LTOSTR_BASE16_COUNT ,_MAX_LTOSTR_BASE10_COUNT ,_MAX_LTOSTR_BASE8_COUNT,
	_MAX_ULTOSTR_BASE16_COUNT,_MAX_ULTOSTR_BASE10_COUNT,_MAX_ULTOSTR_BASE8_COUNT });
constexpr inline auto c_cchI64ToStrBufNoRadix2 = std::max({
	_MAX_I64TOSTR_BASE16_COUNT,_MAX_I64TOSTR_BASE10_COUNT,_MAX_I64TOSTR_BASE8_COUNT,
	_MAX_U64TOSTR_BASE16_COUNT,_MAX_U64TOSTR_BASE10_COUNT,_MAX_U64TOSTR_BASE8_COUNT });

constexpr inline auto c_cchI32ToStrBuf = std::max({ c_cchI32ToStrBufNoRadix2,
	_MAX_ITOSTR_BASE2_COUNT,_MAX_LTOSTR_BASE2_COUNT,_MAX_ULTOSTR_BASE2_COUNT });
constexpr inline auto c_cchI64ToStrBuf = std::max({ c_cchI64ToStrBufNoRadix2,
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
constexpr ULARGE_INTEGER UliMax{ 0xFFFF'FFFF, 0xFFFF'FFFF };
#endif

constexpr inline size_t SizeTMax{ std::numeric_limits<size_t>::max() };
constexpr inline SIZE_T SIZETMax{ (SIZE_T)SizeTMax };

#ifdef _DEBUG
constexpr inline BOOL Dbg{ TRUE };
#else
constexpr inline BOOL Dbg{ FALSE };
#endif

/*-------------------*/
/*控件通知代码*/
#pragma warning(suppress:26454)// 算术溢出
constexpr inline UINT NM_FIRST_ECK = (0u - 0x514Bu * 0x514Bu);
enum :UINT
{
	ECKPRIV_NM_FIRST_PLACEHOLDER = NM_FIRST_ECK,
	NM_CLP_CLRCHANGED,		// NMCLPCLRCHANGED
	NM_SPB_DRAGGED,			// NMSPBDRAGGED
	NM_TGL_TASKCLICKED,		// NMTGLCLICKED
	NM_LBN_GETDISPINFO,		// NMLBNGETDISPINFO
	NM_TL_FILLCHILDREN,		// NMTLFILLCHILDREN
	NM_TL_GETDISPINFO,		// NMTLGETDISPINFO
	NM_TL_ITEMEXPANDING,	// NMTLCOMMITEM
	NM_TL_ITEMEXPANDED,		// NMTLCOMMITEM
	NM_TL_HD_CLICK,			// NMHEADER
	NM_TL_FILLALLFLATITEM,	// NMTLFILLALLFLATITEM
	NM_TL_CUSTOMDRAW,		// NMTLCUSTOMDRAW
	NM_TL_TTGETDISPINFO,	// NMTLTTGETDISPINFO
	NM_TL_TTPRESHOW,		// NMTLTTPRESHOW
	NM_TL_PREEDIT,			// NMTLEDIT
	NM_TL_POSTEDIT,			// NMTLEDIT
	NM_TL_MOUSECLICK,		// NMTLMOUSECLICK
	NM_TL_ITEMCHECKING,		// NMTLCOMMITEM
	NM_TL_ITEMCHECKED,		// NMTLCOMMITEM
	NM_TL_BEGINDRAG,		// NMTLDRAG
	NM_TL_ENDDRAG,			// NMTLDRAG
	NM_LBN_BEGINDRAG,		// NMLBNDRAG
	NM_LBN_ENDDRAG,			// NMLBNDRAG
	NM_LBN_DISMISS,			// NMHDR
	NM_LBN_LBTNDOWN,		// NMHDR
	NM_LBN_CUSTOMDRAW,		// NMLBNCUSTOMDRAW
	NM_PKB_OWNERDRAW,		// NMPKBOWNERDRAW
};

enum
{
	NMECDS_PREDRAW,
	NMECDS_POSTDRAWBK,

	NMECDR_DODEF = 0,
	NMECDR_SKIPDEF,
	NMECDR_PRIV_BEGIN,
};

struct NMECKCTRLCUSTOMDRAW
{
	NMHDR nmhdr;
	int idx;
	int iStage;
	HDC hDC;
	RECT rcItem;
};

// 消息钩子保留范围[1, 4095]
constexpr inline UINT_PTR MsgHookIdUserBegin = 4096;
enum :UINT_PTR
{
	ECKPRIV_MHI_FIRST_PLACEHOLDER = 1,
	MHI_SCROLLBAR_HOOK,
	MHI_HEADER_HOOK
};
/*-------------------*/
/*属性字符串*/

/*-------------------*/
ECK_NAMESPACE_END
#include "DbgHelper.h"
#pragma warning(suppress:5260)
#include <gdiplus.h>
ECK_NAMESPACE_BEGIN
namespace GpNameSpace
{
	using namespace Gdiplus::DllExports;
#define ECK_USING_GDIP_TYPE(Type) using Type = ::Gdiplus::Type

	ECK_USING_GDIP_TYPE(GpGraphics);
	ECK_USING_GDIP_TYPE(GpFontCollection);
	ECK_USING_GDIP_TYPE(GpFontFamily);
	ECK_USING_GDIP_TYPE(GpFont);
	ECK_USING_GDIP_TYPE(GpStringFormat);
	ECK_USING_GDIP_TYPE(GpPen);
	ECK_USING_GDIP_TYPE(GpPath);
	ECK_USING_GDIP_TYPE(GpBrush);
	ECK_USING_GDIP_TYPE(GpSolidFill);
	ECK_USING_GDIP_TYPE(GpLineGradient);
	ECK_USING_GDIP_TYPE(GpImage);
	ECK_USING_GDIP_TYPE(GpImageAttributes);
	using GpEffect = Gdiplus::CGpEffect;
	ECK_USING_GDIP_TYPE(GpBitmap);
	ECK_USING_GDIP_TYPE(GpRegion);
	ECK_USING_GDIP_TYPE(GpMatrix);
	ECK_USING_GDIP_TYPE(ARGB);
	ECK_USING_GDIP_TYPE(REAL);
	ECK_USING_GDIP_TYPE(GpStatus);
	ECK_USING_GDIP_TYPE(GpRectF);
	ECK_USING_GDIP_TYPE(GpRect);
	ECK_USING_GDIP_TYPE(GpPointF);
	ECK_USING_GDIP_TYPE(GpPoint);
	ECK_USING_GDIP_TYPE(GpSizeF);
	using GpSize = ::Gdiplus::Size;
	ECK_USING_GDIP_TYPE(GdiplusStartupInput);
	ECK_USING_GDIP_TYPE(GpFillMode);

	using Gdiplus::GdiplusShutdown;
	using Gdiplus::GdiplusStartup;
}
using namespace GpNameSpace;


extern HINSTANCE g_hInstance;
extern IWICImagingFactory* g_pWicFactory;
extern ID2D1Factory1* g_pD2dFactory;
extern IDWriteFactory* g_pDwFactory;
extern ID2D1Device* g_pD2dDevice;
extern IDXGIDevice1* g_pDxgiDevice;
extern IDXGIFactory2* g_pDxgiFactory;

extern BOOL g_bWin10_1607;
extern BOOL g_bWin10_1809;
extern BOOL g_bWin10_1903;
extern BOOL g_bWin11_B22000;
extern BOOL g_bAllowDark;

enum IMMERSIVE_HC_CACHE_MODE
{
	IHCM_USE_CACHED_VALUE,
	IHCM_REFRESH
};

enum class PreferredAppMode
{
	Default,
	AllowDark,
	ForceDark,
	ForceLight,
	Max
};

enum WINDOWCOMPOSITIONATTRIB
{
	WCA_UNDEFINED = 0,
	WCA_NCRENDERING_ENABLED = 1,
	WCA_NCRENDERING_POLICY = 2,
	WCA_TRANSITIONS_FORCEDISABLED = 3,
	WCA_ALLOW_NCPAINT = 4,
	WCA_CAPTION_BUTTON_BOUNDS = 5,
	WCA_NONCLIENT_RTL_LAYOUT = 6,
	WCA_FORCE_ICONIC_REPRESENTATION = 7,
	WCA_EXTENDED_FRAME_BOUNDS = 8,
	WCA_HAS_ICONIC_BITMAP = 9,
	WCA_THEME_ATTRIBUTES = 10,
	WCA_NCRENDERING_EXILED = 11,
	WCA_NCADORNMENTINFO = 12,
	WCA_EXCLUDED_FROM_LIVEPREVIEW = 13,
	WCA_VIDEO_OVERLAY_ACTIVE = 14,
	WCA_FORCE_ACTIVEWINDOW_APPEARANCE = 15,
	WCA_DISALLOW_PEEK = 16,
	WCA_CLOAK = 17,
	WCA_CLOAKED = 18,
	WCA_ACCENT_POLICY = 19,
	WCA_FREEZE_REPRESENTATION = 20,
	WCA_EVER_UNCLOAKED = 21,
	WCA_VISUAL_OWNER = 22,
	WCA_HOLOGRAPHIC = 23,
	WCA_EXCLUDED_FROM_DDA = 24,
	WCA_PASSIVEUPDATEMODE = 25,
	WCA_USEDARKMODECOLORS = 26,
	WCA_LAST = 27
};

struct WINDOWCOMPOSITIONATTRIBDATA
{
	WINDOWCOMPOSITIONATTRIB Attrib;
	PVOID pvData;
	SIZE_T cbData;
};

ECK_PRIV_NAMESPACE_BEGIN
using FRtlGetNtVersionNumbers = void(WINAPI*)(DWORD*, DWORD*, DWORD*);
using FSetWindowCompositionAttribute = BOOL(WINAPI*)(HWND, WINDOWCOMPOSITIONATTRIBDATA*);

// UxTheme
using FOpenNcThemeData = HTHEME(WINAPI*)(HWND, LPCWSTR);
using FOpenThemeData = HTHEME(WINAPI*)(HWND, LPCWSTR);
using FDrawThemeText = HRESULT(WINAPI*)(_In_ HTHEME, HDC, int, int, LPCWSTR, int, DWORD, DWORD, LPCRECT);
using FOpenThemeDataForDpi = HTHEME(WINAPI*)(HWND, LPCWSTR, UINT);
using FDrawThemeBackgroundEx = HRESULT(WINAPI*)(HTHEME, HDC, int, int, LPCRECT, const DTBGOPTS*);
using FDrawThemeBackground = HRESULT(WINAPI*)(HTHEME, HDC, int, int, LPCRECT, LPCRECT);
using FGetThemeColor = HRESULT(WINAPI*)(HTHEME, int, int, int, COLORREF*);
using FCloseThemeData = HRESULT(WINAPI*)(HTHEME);
using FDrawThemeParentBackground = HRESULT(WINAPI*)(HWND, HDC, const RECT*);
// 1809 17763 暗色功能引入
using FAllowDarkModeForWindow = bool(WINAPI*)(HWND, BOOL);
using FAllowDarkModeForApp = bool(WINAPI*)(BOOL);
using FIsDarkModeAllowedForWindow = bool(WINAPI*)(HWND);
using FShouldAppsUseDarkMode = bool(WINAPI*)();

using FFlushMenuThemes = void(WINAPI*)();

using FRefreshImmersiveColorPolicyState = void(WINAPI*)();
using FGetIsImmersiveColorUsingHighContrast = bool(WINAPI*)(IMMERSIVE_HC_CACHE_MODE);
// 1903 18362
using FShouldSystemUseDarkMode = bool(WINAPI*)();
using FSetPreferredAppMode = PreferredAppMode(WINAPI*)(PreferredAppMode);
using FIsDarkModeAllowedForApp = bool(WINAPI*)();


extern FAllowDarkModeForWindow			pfnAllowDarkModeForWindow;
extern FAllowDarkModeForApp				pfnAllowDarkModeForApp;
extern FIsDarkModeAllowedForWindow		pfnIsDarkModeAllowedForWindow;
extern FShouldAppsUseDarkMode			pfnShouldAppsUseDarkMode;
extern FFlushMenuThemes					pfnFlushMenuThemes;
extern FRefreshImmersiveColorPolicyState		pfnRefreshImmersiveColorPolicyState;
extern FGetIsImmersiveColorUsingHighContrast	pfnGetIsImmersiveColorUsingHighContrast;

extern FShouldSystemUseDarkMode			pfnShouldSystemUseDarkMode;
extern FSetPreferredAppMode				pfnSetPreferredAppMode;
extern FIsDarkModeAllowedForApp			pfnIsDarkModeAllowedForApp;

extern FRtlGetNtVersionNumbers			pfnRtlGetNtVersionNumbers;
extern FSetWindowCompositionAttribute	pfnSetWindowCompositionAttribute;
extern FOpenNcThemeData					pfnOpenNcThemeData;
ECK_PRIV_NAMESPACE_END


/*窗口类名*/

constexpr inline PCWSTR WCN_LABEL = L"Eck.WndClass.Label";
constexpr inline PCWSTR WCN_COLORPICKER = L"Eck.WndClass.ColorPicker";
constexpr inline PCWSTR WCN_BK = L"Eck.WndClass.BK";
constexpr inline PCWSTR WCN_LUNARCALENDAR = L"Eck.WndClass.LunarCalendar";
constexpr inline PCWSTR WCN_CHARTPIE = L"Eck.WndClass.ChartPie";
constexpr inline PCWSTR WCN_FORM = L"Eck.WndClass.Form";
constexpr inline PCWSTR WCN_TABHEADER = L"Eck.WndClass.TabHeader";
constexpr inline PCWSTR WCN_DLG = L"Eck.WndClass.CommDlg";
constexpr inline PCWSTR WCN_SPLITBAR = L"Eck.WndClass.SplitBar";
constexpr inline PCWSTR WCN_DRAWPANEL = L"Eck.WndClass.DrawPanel";
constexpr inline PCWSTR WCN_DRAWPANELD2D = L"Eck.WndClass.DrawPanelD2D";
constexpr inline PCWSTR WCN_LISTBOXNEW = L"Eck.WndClass.ListBoxNew";
constexpr inline PCWSTR WCN_ANIMATIONBOX = L"Eck.WndClass.AnimationBox";
constexpr inline PCWSTR WCN_TREELIST = L"Eck.WndClass.TreeList";
constexpr inline PCWSTR WCN_COMBOBOXNEW = L"Eck.WndClass.ComboBoxNew";
constexpr inline PCWSTR WCN_PICTUREBOX = L"Eck.WndClass.PictureBox";
constexpr inline PCWSTR WCN_DUIHOST = L"Eck.WndClass.DuiHost";
constexpr inline PCWSTR WCN_VECDRAWPANEL = L"Eck.WndClass.VectorDrawPanel";

constexpr inline PCWSTR MSGREG_FORMTRAY = L"Eck.Message.FormTray";

constexpr inline UINT SCID_DESIGN = 20230621'01u;

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
	UxThemeError,
};

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
/// 函数将在内部调用eck::ThreadInit，除非设置了ECKINIT_NOINITTHREAD
/// </summary>
/// <param name="hInstance">实例句柄，所有自定义窗口类将在此实例上注册</param>
/// <param name="pInitParam">指向初始化参数的可选指针</param>
/// <param name="pdwErrCode">指向接收错误码变量的可选指针</param>
/// <returns>错误代码</returns>
InitStatus Init(HINSTANCE hInstance, const INITPARAM* pInitParam = NULL, DWORD* pdwErrCode = NULL);

void UnInit();

PCWSTR InitStatusToString(InitStatus iStatus);

class CWnd;
struct ECKTHREADCTX;

using FWndCreating = void(*)(HWND hWnd, CBT_CREATEWNDW* pcs, ECKTHREADCTX* pThreadCtx);
struct ECKTHREADCTX
{
	//-------窗口映射
	std::unordered_map<HWND, CWnd*> hmWnd{};	// HWND->CWnd*
	std::unordered_map<HWND, CWnd*> hmTopWnd{};	// 顶级窗口映射
	HHOOK hhkTempCBT{};					// CBT钩子句柄
	CWnd* pCurrWnd{};					// 当前正在创建窗口所属的CWnd指针
	FWndCreating pfnWndCreatingProc{};	// 当前创建窗口时要调用的过程
	//-------暗色处理
	HHOOK hhkCbtDarkMode{};				// 设置允许暗色CBT钩子句柄
	BOOL bEnableDarkModeHook{ TRUE };	// 是否允许暗色CBT钩子设置窗口，设为FALSE可暂停HOOK
	BOOL bAutoNcDark{ TRUE };			// 自动调整非客户区暗色
	COLORREF crDefText{};		// 默认前景色
	COLORREF crDefBkg{};		// 默认背景色
	COLORREF crDefBtnFace{};	// 默认BtnFace颜色
	std::unordered_map<HTHEME, int> hsButtonTheme{};
	std::unordered_map<HTHEME, int> hsTaskDialogTheme{};
	std::unordered_map<HTHEME, int> hsTabTheme{};
	std::unordered_map<HTHEME, std::pair<int, HTHEME>> hsToolBarTheme{};
	std::unordered_map<HTHEME, int> hsAeroWizardTheme{};
	std::unordered_map<HTHEME, std::pair<int, HTHEME>> hsDateTimePickerTheme{};

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

	[[nodiscard]] EckInline CWnd* WmAt(HWND hWnd) const
	{
		const auto it = hmWnd.find(hWnd);
		if (it != hmWnd.end())
			return it->second;
		else
			return NULL;
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

	[[nodiscard]] EckInline CWnd* TwmAt(HWND hWnd) const
	{
		const auto it = hmTopWnd.find(hWnd);
		if (it != hmTopWnd.end())
			return it->second;
		else
			return NULL;
	}

	void SetNcDarkModeForAllTopWnd(BOOL bDark);

	void UpdateDefColor();

	void OnThemeOpen(HTHEME hTheme, PCWSTR pszClassList);

	void OnThemeClose(HTHEME hTheme);

	EckInline BOOL IsThemeButton(HTHEME hTheme) const
	{
		return hsButtonTheme.find(hTheme) != hsButtonTheme.end();
	}

	EckInline BOOL IsThemeTaskDialog(HTHEME hTheme) const
	{
		return hsTaskDialogTheme.find(hTheme) != hsTaskDialogTheme.end();
	}

	EckInline BOOL IsThemeTab(HTHEME hTheme) const
	{
		return hsTabTheme.find(hTheme) != hsTabTheme.end();
	}

	EckInline BOOL IsThemeToolBar(HTHEME hTheme, HTHEME* phThemeLV = NULL) const
	{
		if (phThemeLV)
		{
			const auto it = hsToolBarTheme.find(hTheme);
			if (it != hsToolBarTheme.end())
			{
				*phThemeLV = it->second.second;
				return TRUE;
			}
			else
				return FALSE;
		}
		else
			return hsToolBarTheme.find(hTheme) != hsToolBarTheme.end();
	}

	EckInline BOOL IsThemeAeroWizard(HTHEME hTheme) const
	{
		return hsAeroWizardTheme.find(hTheme) != hsAeroWizardTheme.end();
	}

	EckInline BOOL IsThemeDateTimePicker(HTHEME hTheme, HTHEME* phThemeCBB = NULL) const
	{
		if (phThemeCBB)
		{
			const auto it = hsDateTimePickerTheme.find(hTheme);
			if (it != hsDateTimePickerTheme.end())
			{
				*phThemeCBB = it->second.second;
				return TRUE;
			}
			else
				return FALSE;
		}
		else
			return hsDateTimePickerTheme.find(hTheme) != hsDateTimePickerTheme.end();
	}

	void SendThemeChangedToAllTopWindow();
};

/// <summary>
/// 取线程上下文TLS槽
/// </summary>
[[nodiscard]] DWORD GetThreadCtxTlsSlot();

/// <summary>
/// 初始化线程上下文。
/// 在调用线程上初始化线程上下文，在使用任何ECK窗口功能前必须调用此函数
/// </summary>
void ThreadInit();

/// <summary>
/// 反初始化线程上下文。
/// 调用此函数后不允许使用任何ECK窗口对象
/// </summary>
void ThreadUnInit();

/// <summary>
/// 取线程上下文
/// </summary>
[[nodiscard]] EckInline ECKTHREADCTX* GetThreadCtx()
{
	return (ECKTHREADCTX*)TlsGetValue(GetThreadCtxTlsSlot());
}

/// <summary>
/// 窗口句柄到CWnd指针
/// </summary>
[[nodiscard]] EckInline CWnd* CWndFromHWND(HWND hWnd)
{
	return GetThreadCtx()->WmAt(hWnd);
}

HHOOK BeginCbtHook(CWnd* pCurrWnd, FWndCreating pfnCreatingProc = NULL);

void EndCbtHook();

// 全局消息过滤器，若要拦截消息则应返回TRUE，否则应返回FALSE
using FMsgFilter = BOOL(*)(const MSG& Msg);

/// <summary>
/// 过滤消息。
/// 若使用了任何ECK窗口对象，则必须在翻译按键和派发消息之前调用此函数
/// </summary>
/// <param name="Msg">即将处理的消息</param>
/// <returns>若返回值为TRUE，则不应继续处理消息；否则应正常进行剩余步骤</returns>
BOOL PreTranslateMessage(const MSG& Msg);

/// <summary>
/// 置消息过滤器
/// </summary>
/// <param name="pfnFilter">应用程序定义的过滤器函数指针</param>
void SetMsgFilter(FMsgFilter pfnFilter);

EckInline void RtlGetNtVersionNumbers(DWORD* pdwMajor, DWORD* pdwMinor, DWORD* pdwBuild)
{
	EckPriv::pfnRtlGetNtVersionNumbers(pdwMajor, pdwMinor, pdwBuild);
}
ECK_NAMESPACE_END

#ifndef ECK_MACRO_NO_USING_GDIPLUS
using namespace eck::GpNameSpace;
#endif// !define(ECK_MACRO_NO_USING_GDIPLUS)