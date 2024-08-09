#pragma once
#include "PhNt/phnt_windows.h"
#include "PhNt/phnt.h"

#include <Uxtheme.h>

void EckInitPrivateApi();

EXTERN_C_START

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
	WCA_NCRENDERING_ENABLED = 1,			// DWMWA_NCRENDERING_ENABLED
	WCA_NCRENDERING_POLICY = 2,				// DWMWA_NCRENDERING_POLICY
	WCA_TRANSITIONS_FORCEDISABLED = 3,		// DWMWA_TRANSITIONS_FORCEDISABLED
	WCA_ALLOW_NCPAINT = 4,					// DWMWA_ALLOW_NCPAINT
	WCA_CAPTION_BUTTON_BOUNDS = 5,			// DWMWA_CAPTION_BUTTON_BOUNDS
	WCA_NONCLIENT_RTL_LAYOUT = 6,			// DWMWA_NONCLIENT_RTL_LAYOUT
	WCA_FORCE_ICONIC_REPRESENTATION = 7,	// DWMWA_FORCE_ICONIC_REPRESENTATION
	WCA_EXTENDED_FRAME_BOUNDS = 8,			// DWMWA_EXTENDED_FRAME_BOUNDS
	WCA_HAS_ICONIC_BITMAP = 9,				// DWMWA_HAS_ICONIC_BITMAP
	WCA_THEME_ATTRIBUTES = 10,				// 
	WCA_NCRENDERING_EXILED = 11,			// 
	WCA_NCADORNMENTINFO = 12,				// 
	WCA_EXCLUDED_FROM_LIVEPREVIEW = 13,		// DWMWA_EXCLUDED_FROM_PEEK
	WCA_VIDEO_OVERLAY_ACTIVE = 14,			// 
	WCA_FORCE_ACTIVEWINDOW_APPEARANCE = 15,	// 
	WCA_DISALLOW_PEEK = 16,					// DWMWA_DISALLOW_PEEK
	WCA_CLOAK = 17,							// DWMWA_CLOAK
	WCA_CLOAKED = 18,						// DWMWA_CLOAKED
	WCA_ACCENT_POLICY = 19,					// 修改窗口强调模式，pvData指向ACCENT_POLICY结构
	WCA_FREEZE_REPRESENTATION = 20,			// DWMWA_FREEZE_REPRESENTATION
	WCA_EVER_UNCLOAKED = 21,				// 
	WCA_VISUAL_OWNER = 22,					// 
	WCA_HOLOGRAPHIC = 23,					// 
	WCA_EXCLUDED_FROM_DDA = 24,				// [已文档化]防止桌面复制API捕捉窗口，pvData指向BOOL类型的值
	WCA_PASSIVEUPDATEMODE = 25,				// DWMWA_PASSIVE_UPDATE_MODE
	WCA_USEDARKMODECOLORS = 26,				// DWMWA_USE_IMMERSIVE_DARK_MODE
	WCA_LAST = 27
};
/*
* DWMWA_不对应项：
* DWMWA_FLIP3D_POLICY
* DWMWA_USE_HOSTBACKDROPBRUSH   22000+
* 
* 下列常量可能有对应项，但目前缺少WINDOWCOMPOSITIONATTRIB的最新定义：
* DWMWA_BORDER_COLOR
* DWMWA_CAPTION_COLOR
* DWMWA_TEXT_COLOR
* DWMWA_VISIBLE_FRAME_BORDER_THICKNESS
* DWMWA_SYSTEMBACKDROP_TYPE
*/

struct WINDOWCOMPOSITIONATTRIBDATA
{
	WINDOWCOMPOSITIONATTRIB Attrib;
	PVOID pvData;
	SIZE_T cbData;
};

enum AccentState
{
	ACCENT_DISABLED,
	ACCENT_ENABLE_GRADIENT,
	ACCENT_ENABLE_TRANSPARENTGRADIENT,
	ACCENT_ENABLE_BLURBEHIND,
	ACCENT_ENABLE_ACRYLICBLURBEHIND,// 1803+
	ACCENT_ENABLE_HOSTBACKDROP,// 1809+
	ACCENT_INVALID_STATE,
};

struct ACCENT_POLICY
{
	AccentState AccentState;
	int AccentFlags;
	int GradientColor;
	int AnimationId;
};

using FSetWindowCompositionAttribute = BOOL(WINAPI*)(HWND, WINDOWCOMPOSITIONATTRIBDATA*);
extern FSetWindowCompositionAttribute pfnSetWindowCompositionAttribute;

FORCEINLINE BOOL SetWindowCompositionAttribute(HWND hWnd, WINDOWCOMPOSITIONATTRIBDATA* pAttrData)
{
	return pfnSetWindowCompositionAttribute(hWnd, pAttrData);
}

BOOL WINAPI IsNTAdmin(DWORD dwReserved, DWORD* pdwResevered);

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

FORCEINLINE BOOL AllowDarkModeForWindow(HWND hWnd, BOOL bAllow)
{
#if NTDDI_VERSION >= NTDDI_WIN10_RS5
	return pfnAllowDarkModeForWindow(hWnd, bAllow);
#else
	return FALSE;
#endif
}

FORCEINLINE PreferredAppMode SetPreferredAppMode_Org(PreferredAppMode iMode)
{
#if NTDDI_VERSION >= NTDDI_WIN10_RS5
	return pfnSetPreferredAppMode(iMode);
#else
	return PreferredAppMode::Default;
#endif
}

FORCEINLINE void AllowDarkModeForApp_Org(BOOL bAllow)
{
#if NTDDI_VERSION >= NTDDI_WIN10_RS5
	pfnAllowDarkModeForApp(bAllow);
#endif
}

FORCEINLINE PreferredAppMode SetPreferredAppMode(PreferredAppMode iMode)
{
	if (pfnSetPreferredAppMode)
		return pfnSetPreferredAppMode(iMode);
	else if (pfnAllowDarkModeForApp)
	{
		pfnAllowDarkModeForApp(
			iMode == PreferredAppMode::AllowDark || iMode == PreferredAppMode::ForceDark);
		return PreferredAppMode::Default;
	}
	return PreferredAppMode::Default;
}

FORCEINLINE BOOL IsDarkModeAllowedForWindow(HWND hWnd)
{
#if NTDDI_VERSION >= NTDDI_WIN10_RS5
	return pfnIsDarkModeAllowedForWindow(hWnd);
#else
	return FALSE;
#endif
}

FORCEINLINE BOOL ShouldAppsUseDarkMode()
{
#if NTDDI_VERSION >= NTDDI_WIN10_RS5
	return pfnShouldAppsUseDarkMode();
#else
	return FALSE;
#endif
}

FORCEINLINE void FlushMenuTheme()
{
#if NTDDI_VERSION >= NTDDI_WIN10_RS5
	pfnFlushMenuThemes();
#endif
}

FORCEINLINE void RefreshImmersiveColorPolicyState()
{
#if NTDDI_VERSION >= NTDDI_WIN10_RS5
	pfnRefreshImmersiveColorPolicyState();
#endif
}

FORCEINLINE BOOL GetIsImmersiveColorUsingHighContrast(IMMERSIVE_HC_CACHE_MODE iCacheMode)
{
#if NTDDI_VERSION >= NTDDI_WIN10_RS5
	return pfnGetIsImmersiveColorUsingHighContrast(iCacheMode);
#else
	return FALSE;
#endif
}

FORCEINLINE BOOL ShouldSystemUseDarkMode()
{
#if NTDDI_VERSION >= NTDDI_WIN10_19H1
	return pfnShouldSystemUseDarkMode();
#else
	return FALSE;
#endif
}

FORCEINLINE BOOL IsDarkModeAllowedForApp()
{
#if NTDDI_VERSION >= NTDDI_WIN10_19H1
	return pfnIsDarkModeAllowedForApp();
#else
	return FALSE;
#endif
}

using FOpenNcThemeData = HTHEME(WINAPI*)(HWND, LPCWSTR);

extern FOpenNcThemeData pfnOpenNcThemeData;

FORCEINLINE HTHEME OpenNcThemeData(HWND hWnd, PCWSTR pszClassList)
{
	return pfnOpenNcThemeData(hWnd, pszClassList);
}

EXTERN_C_END