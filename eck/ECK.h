/*
* WinEzCtrlKit Library
*
* ECK.h ： 公共头文件
*
* Copyright(C) 2023-2024 QingKong
*/
#pragma once
#include <Windows.h>
#include <Uxtheme.h>
#include <vsstyle.h>
#include <dwmapi.h>
#include <wincodec.h>
#include <dwrite.h>
#include <d2d1_1.h>
#include <dxgi1_2.h>

#include <assert.h>

#include <unordered_map>
#include <type_traits>

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
#	endif
#else
#	if _WIN32_WINNT < 0x0605
#		error "dpi api requires _WIN32_WINNT >= 0x0605"
#	endif
#endif


#define ECK_NAMESPACE_BEGIN			namespace eck {
#define ECK_NAMESPACE_END			}
#define ECK_PRIV_NAMESPACE_BEGIN	namespace EckPriv___ {
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

#else//	ECKCXX20
template <class T>
using RemoveCVRef_T = std::remove_cv_t<std::remove_reference_t<T>>;

template <class T>
struct RemoveCVRef
{
	using Type = RemoveCVRef_T<T>;
};
#endif// ECKCXX20

template<class T>
concept ccpIsIntOrEnum = std::is_integral_v<T> || std::is_enum_v<T>;

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

template<class T>
using UnderlyingType_T = UnderlyingType<T>::Type;

ECK_NAMESPACE_END

#define EckInline				__forceinline

// 控件序列化数据对齐
#define ECK_CTRLDATA_ALIGN		4

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

// lParam->size 用于处理WM_SIZE   e.g. ECK_GET_SIZE_LPARAM(cxClient, cyClient, lParam);
#define ECK_GET_SIZE_LPARAM(cx,cy,lParam) (cx) = LOWORD(lParam); (cy) = HIWORD(lParam);

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
}

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

constexpr inline BYTE BOM_UTF16LE[]{ 0xFF,0xFE };
constexpr inline BYTE BOM_UTF16BE[]{ 0xFE,0xFF };
constexpr inline BYTE BOM_UTF8[]{ 0xEF,0xBB,0xBF };

constexpr inline COLORREF c_crDarkWnd = RGB(32, 32, 32);
constexpr inline COLORREF c_crDarkBtnFace = 0x383838;

/*-------------------*/
/*控件通知代码*/
#pragma warning(suppress:26454)// 算术溢出
constexpr inline UINT NM_FIRST_ECK = (0u - 0x514B * 0x514B);
enum :UINT
{
	ECKPRIV_NM_FIRST_PLACEHOLDER___ = NM_FIRST_ECK,
	NM_CLP_CLRCHANGED,// NMCLPCLRCHANGED
	NM_SPB_DRAGGED,// NMSPBDRAGGED
	NM_TGL_TASKCLICKED,// NMTGLCLICKED
	NM_LBN_GETDISPINFO,// NMLBNGETDISPINFO
	NM_TL_FILLCHILDREN,// NMTLFILLCHILDREN
	NM_TL_GETDISPINFO,// NMTLGETDISPINFO
	NM_TL_ITEMEXPANDING,// NMTLCOMMITEM
	NM_TL_ITEMEXPANDED,// NMTLCOMMITEM
	NM_TL_HD_CLICK,// NMHEADER
	NM_TL_FILLALLFLATITEM,// NMTLFILLALLFLATITEM
	NM_TL_CUSTOMDRAW,// NMTLCUSTOMDRAW
	NM_TL_TTGETDISPINFO,// NMTLTTGETDISPINFO
	NM_TL_TTPRESHOW,// NMTLTTPRESHOW
	NM_TL_PREEDIT,// NMTLEDIT
	NM_TL_POSTEDIT,// NMTLEDIT
	NM_TL_MOUSECLICK,// NMTLMOUSECLICK
	NM_TL_ITEMCHECKING,// NMTLCOMMITEM
	NM_TL_ITEMCHECKED,// NMTLCOMMITEM
	NM_TL_BEGINDRAG,// NMTLDRAG
	NM_TL_ENDDRAG,// NMTLDRAG
};
/*-------------------*/
/*属性字符串*/

/*-------------------*/
ECK_NAMESPACE_END
#include "DbgHelper.h"
#include "GdiplusFlatDef.h"
ECK_NAMESPACE_BEGIN
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

using FSetWindowCompositionAttribute = BOOL(WINAPI*)(HWND hWnd, WINDOWCOMPOSITIONATTRIBDATA*);
// 1809 17763 暗色功能引入
using FAllowDarkModeForWindow = BOOL(WINAPI*)(HWND hWnd, BOOL bAllow);
using FAllowDarkModeForApp = BOOL(WINAPI*)(BOOL bAllow);
using FIsDarkModeAllowedForWindow = BOOL(WINAPI*)(HWND hWnd);
using FShouldAppsUseDarkMode = BOOL(WINAPI*)();

using FFlushMenuThemes = void(WINAPI*)();

using FRefreshImmersiveColorPolicyState = void(WINAPI*)();
using FGetIsImmersiveColorUsingHighContrast = BOOL(WINAPI*)(IMMERSIVE_HC_CACHE_MODE mode);
// 1903 18362
using FShouldSystemUseDarkMode = BOOL(WINAPI*)();
using FSetPreferredAppMode = PreferredAppMode(WINAPI*)(PreferredAppMode appMode);
using FIsDarkModeAllowedForApp = BOOL(WINAPI*)();


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

constexpr inline PCWSTR MSG_INERTIALSV = L"Eck.Message.InertialScrollView";
constexpr inline PCWSTR MSGREG_FORMTRAY = L"Eck.Message.FormTray";
constexpr inline PCWSTR MSGREG_EASING = L"Eck.Message.Easing";

constexpr inline PCWSTR WPROP_EASING = L"Eck.Prop.Easing";

constexpr inline UINT SCID_DESIGN = 20230621'01u;
constexpr inline UINT SCID_INERTIALSCROLLVIEW = 20231103'01u;
constexpr inline UINT SCID_EASING = 20240208'01u;

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
	D2D1_FACTORY_TYPE uD2dFactoryType = D2D1_FACTORY_TYPE_SINGLE_THREADED;
	DWRITE_FACTORY_TYPE uDWriteFactoryType = DWRITE_FACTORY_TYPE_SHARED;
	const D3D_FEATURE_LEVEL* pD3dFeatureLevel = c_uDefD3dFeatureLevel;
	UINT cD3dFeatureLevel = ARRAYSIZE(c_uDefD3dFeatureLevel);
	UINT uFlags = EIF_DEFAULT;
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
	std::unordered_map<HWND, CWnd*> hmWnd{};	// HWND->CWnd*
	HHOOK hhkTempCBT = NULL;					// CBT钩子句柄
	CWnd* pCurrWnd = NULL;						// 当前正在创建窗口所属的CWnd指针
	FWndCreating pfnWndCreatingProc = NULL;		// 当前创建窗口时要调用的过程
	HHOOK hhkDarkMode = NULL;
	BOOL bDarkMode = FALSE;

	EckInline void WmAdd(HWND hWnd, CWnd* pWnd)
	{
		hmWnd.insert(std::make_pair(hWnd, pWnd));
	}

	EckInline void WmRemove(HWND hWnd)
	{
		const auto it = hmWnd.find(hWnd);
		if (it != hmWnd.end())
			hmWnd.erase(it);
	}

	[[nodiscard]] EckInline CWnd* WmAt(HWND hWnd) const
	{
		const auto it = hmWnd.find(hWnd);
		if (it != hmWnd.end())
			return it->second;
		else
			return NULL;
	}
};

/// <summary>
/// 取线程上下文TLS槽
/// </summary>
[[nodiscard]] DWORD GetThreadCtxTlsSlot();

/// <summary>
/// 初始化线程上下文。
/// 在调用线程上初始化线程上下文，如果线程使用了ECK的窗口对象，则必须调用此函数
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
	EckPriv___::pfnRtlGetNtVersionNumbers(pdwMajor, pdwMinor, pdwBuild);
}
ECK_NAMESPACE_END