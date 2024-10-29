/*
* WinEzCtrlKit Library
*
* PrivateApi.h ： 未公开API声明
*
* Copyright(C) 2024 QingKong
*/
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

using FOpenNcThemeData = HTHEME(WINAPI*)(HWND, PCWSTR);

extern FOpenNcThemeData pfnOpenNcThemeData;

FORCEINLINE HTHEME OpenNcThemeData(HWND hWnd, PCWSTR pszClassList)
{
	return pfnOpenNcThemeData(hWnd, pszClassList);
}
EXTERN_C_END


// {96a23e16-a1bc-11d1-b084-00c04fc33aa5}
constexpr inline IID IID_ILVRange{ 0x96a23e16L, 0xa1bc, 0x11d1, { 0xb0, 0x84, 0x00, 0xc0, 0x4f, 0xc3, 0x3a, 0xa5 } };

// The ILVRange interface manages a range of items
// in a owner-data list view control.
MIDL_INTERFACE("96a23e16-a1bc-11d1-b084-00c04fc33aa5")
ILVRange : public IUnknown
{
	STDMETHOD(IncludeRange)(LONG idxBegin, LONG idxEnd) = 0;
	STDMETHOD(ExcludeRange)(LONG idxBegin, LONG idxEnd) = 0;
	STDMETHOD(InvertRange)(LONG idxBegin, LONG idxEnd) = 0;
	STDMETHOD(InsertItem)(LONG idxItem) = 0;
	STDMETHOD(RemoveItem)(LONG idxItem) = 0;
	STDMETHOD(Clear)() = 0;
	STDMETHOD(IsSelected)(LONG idxItem) = 0;
	STDMETHOD(IsEmpty)() = 0;
	STDMETHOD(NextSelected)(LONG idxItem, LONG* pidxItem) = 0;
	STDMETHOD(NextUnSelected)(LONG idxItem, LONG* pidxItem) = 0;
	STDMETHOD(CountIncluded)(LONG* pcIncluded) = 0;
};


// 惯用叫法，无从得知微软的实际命名
// (IID*, void**)
#define LVM_QUERYINTERFACE (LVM_FIRST + 189)

/*
* ==============================================
* Written by Timo Kunze, under the Public Domain
* ==============================================
*/

constexpr inline IID IID_IListView2{ 0xE5B16AF2, 0x3990, 0x4681, {0xA6, 0x09, 0x1F, 0x06, 0x0C, 0xD1, 0x42, 0x69} };
constexpr inline IID IID_IOwnerDataCallback{ 0x44C09D56, 0x8D3B, 0x419D, {0xA4, 0x62, 0x7B, 0x95, 0x6B, 0x10, 0x5B, 0x47} };
constexpr inline IID IID_IDrawPropertyControl{ 0xE6DFF6FD, 0xBCD5, 0x4162, {0x9C, 0x65, 0xA3, 0xB1, 0x8C, 0x61, 0x6F, 0xDB} };
constexpr inline IID IID_IPropertyControlBase{ 0x6E71A510, 0x732A, 0x4557, {0x95, 0x96, 0xA8, 0x27, 0xE3, 0x6D, 0xAF, 0x8F} };
constexpr inline IID IID_IPropertyControl{ 0x5E82A4DD, 0x9561, 0x476A, {0x86, 0x34, 0x1B, 0xEB, 0xAC, 0xBA, 0x4A, 0x38} };
constexpr inline IID IID_IListViewFooter{ 0xF0034DA8, 0x8A22, 0x4151, {0x8F, 0x16, 0x2E, 0xBA, 0x76, 0x56, 0x5B, 0xCC} };
constexpr inline IID IID_IListViewFooterCallback{ 0x88EB9442, 0x913B, 0x4AB4, {0xA7, 0x41, 0xDD, 0x99, 0xDC, 0xB7, 0x55, 0x8B} };
constexpr inline IID IID_IPropertyValue{ 0x7AF7F355, 0x1066, 0x4E17, {0xB1, 0xF2, 0x19, 0xFE, 0x2F, 0x09, 0x9C, 0xD2} };
constexpr inline IID IID_ISubItemCallback{ 0x11A66240, 0x5489, 0x42C2, {0xAE, 0xBF, 0x28, 0x6F, 0xC8, 0x31, 0x52, 0x4C} };
// {1E8F0D70-7399-41BF-8598-7949A2DEC898}
constexpr inline GUID CLSID_CBooleanControl{ 0x1E8F0D70, 0x7399, 0x41BF, {0x85, 0x98, 0x79, 0x49, 0xA2, 0xDE, 0xC8, 0x98} };
// {e2183960-9d58-4e9c-878a-4acc06ca564a}
constexpr inline GUID CLSID_CCustomDrawMultiValuePropertyControl{ 0xE2183960, 0x9D58, 0x4E9C, {0x87, 0x8A, 0x4A, 0xCC, 0x06, 0xCA, 0x56, 0x4A} };
// {AB517586-73CF-489c-8D8C-5AE0EAD0613A}
constexpr inline GUID CLSID_CCustomDrawPercentFullControl{ 0xAB517586, 0x73CF, 0x489c, {0x8D, 0x8C, 0x5A, 0xE0, 0xEA, 0xD0, 0x61, 0x3A} };
// {0d81ea0d-13bf-44b2-af1c-fcdf6be7927c}
constexpr inline GUID CLSID_CCustomDrawProgressControl{ 0x0d81ea0d, 0x13bf, 0x44B2, {0xAF, 0x1C, 0xFC, 0xDF, 0x6B, 0xE7, 0x92, 0x7C} };
// {15756be1-a4ad-449c-b576-df3df0e068d3}
constexpr inline GUID CLSID_CHyperlinkControl{ 0x15756BE1, 0xA4AD, 0x449C, {0xB5, 0x76, 0xDF, 0x3D, 0xF0, 0xE0, 0x68, 0xD3} };
// {53a01e9d-61cc-4cb0-83b1-31bc8df63156}
constexpr inline GUID CLSID_CIconListControl{ 0x53A01E9D, 0x61CC, 0x4CB0, {0x83, 0xB1, 0x31, 0xBC, 0x8D, 0xF6, 0x31, 0x56} };
// {6A205B57-2567-4a2c-B881-F787FAB579A3}
constexpr inline GUID CLSID_CInPlaceCalendarControl{ 0x6A205B57, 0x2567, 0x4A2C, {0xB8, 0x81, 0xF7, 0x87, 0xFA, 0xB5, 0x79, 0xA3} };
// {0EEA25CC-4362-4a12-850B-86EE61B0D3EB}
constexpr inline GUID CLSID_CInPlaceDropListComboControl{ 0x0EEA25CC, 0x4362, 0x4A12, {0x85, 0x0B, 0x86, 0xEE, 0x61, 0xB0, 0xD3, 0xEB} };
// {A9CF0EAE-901A-4739-A481-E35B73E47F6D}
constexpr inline GUID CLSID_CInPlaceEditBoxControl{ 0xA9CF0EAE, 0x901A, 0x4739, {0xA4, 0x81, 0xE3, 0x5B, 0x73, 0xE4, 0x7F, 0x6D} };
// {8EE97210-FD1F-4b19-91DA-67914005F020}
constexpr inline GUID CLSID_CInPlaceMLEditBoxControl{ 0x8EE97210, 0xFD1F, 0x4B19, {0x91, 0xDA, 0x67, 0x91, 0x40, 0x05, 0xF0, 0x20} };
// {8e85d0ce-deaf-4ea1-9410-fd1a2105ceb5}
constexpr inline GUID CLSID_CInPlaceMultiValuePropertyControl{ 0x8E85D0CE, 0xDEAF, 0x4EA1, {0x94, 0x10, 0xFD, 0x1A, 0x21, 0x05, 0xCE, 0xB5} };
// {85e94d25-0712-47ed-8cde-b0971177c6a1}
constexpr inline GUID CLSID_CRatingControl{ 0x85e94d25, 0x0712, 0x47ed, {0x8C, 0xDE, 0xB0, 0x97, 0x11, 0x77, 0xC6, 0xA1} };
// {527c9a9b-b9a2-44b0-84f9-f0dc11c2bcfb}
constexpr inline GUID CLSID_CStaticPropertyControl{ 0x527C9A9B, 0xB9A2, 0x44B0, {0x84, 0xF9, 0xF0, 0xDC, 0x11, 0xC2, 0xBC, 0xFB} };

MIDL_INTERFACE("7AF7F355-1066-4E17-B1F2-19FE2F099CD2")
IPropertyValue :public IUnknown
{
public:
	/// <summary>
	/// Sets the PROPERTYKEY structure that identifies
	/// the property wrapped by the object
	/// </summary>
	/// <param name="propKey">The property identifier.
	/// It will be stored by the object.</param>
	/// <returns>An HRESULT error code.</returns>
	STDMETHOD(SetPropertyKey)(PROPERTYKEY const& PropKey) = 0;

	/// <summary>
	/// Retrieves the PROPERTYKEY structure that
	/// identifies the property wrapped by the object
	/// </summary>
	/// <param name="pPropKey">Receives the property identifier.</param>
	/// <returns>An HRESULT error code.</returns>
	STDMETHOD(GetPropertyKey)(PROPERTYKEY* pPropKey) = 0;

	/// <summary>
	/// Retrieves the current value of the property
	/// wrapped by the object
	/// </summary>
	/// <param name="pPropValue">Receives the property value.</param>
	/// <returns>An HRESULT error code.</returns>
	STDMETHOD(GetValue)(PROPVARIANT* pPropValue) = 0;

	/// <summary>
	/// Initializes the object with the property's
	/// current value
	/// </summary>
	/// <param name="propValue">The property's current
	/// value. It will be stored by the object.</param>
	/// <returns>An HRESULT error code.</returns>
	STDMETHOD(InitValue)(PROPVARIANT const& PropValue) = 0;
};

MIDL_INTERFACE("88EB9442-913B-4AB4-A741-DD99DCB7558B")
IListViewFooterCallback :public IUnknown
{
public:
	/// <summary>
	/// Notifies the client that a footer item has been clicked.
	/// This method is called by the list view control to notify
	/// the client application that the user has clicked a footer item.
	/// </summary>
	/// <param name="idxItem">The zero-based index of the footer
	/// item that has been clicked.</param>
	/// <param name="lParam">The application-defined integer value
	/// that is associated with the clicked item.</param>
	/// <param name="pbRemoveFooter">If set to TRUE, the list view
	/// control will remove the footer area.</param>
	/// <returns>An HRESULT error code.</returns>
	STDMETHOD(OnButtonClicked)(int idxItem, LPARAM lParam, BOOL * pbRemoveFooter) = 0;

	/// <summary>
	/// Notifies the client that a footer item has been removed.
	/// This method is called by the list view control to notify
	/// the client application that it has removed a footer item.
	/// </summary>
	/// <param name="idxItem">The zero-based index of the
	/// footer item that has been removed.</param>
	/// <param name="lParam">The application-defined integer
	/// value that is associated with the removed item.</param>
	/// <returns>An HRESULT error code.</returns>
	STDMETHOD(OnDestroyButton)(int idxItem, LPARAM lParam) = 0;
};

MIDL_INTERFACE("F0034DA8-8A22-4151-8F16-2EBA76565BCC")
IListViewFooter :public IUnknown
{
public:
	/// <summary>
	/// Retrieves whether the list view control's footer
	/// area is currently displayed.
	/// </summary>
	/// <param name="pbVisible">TRUE if the footer area
	/// is visible; otherwise FALSE.</param>
	/// <returns>An HRESULT error code.</returns>
	STDMETHOD(IsVisible)(BOOL * pbVisible) = 0;

	/// <summary>
	/// Retrieves the list view control's focused footer item.
	/// </summary>
	/// <param name="pidxItem">Receives the zero-based index
	/// of the footer item that has the keyboard focus.</param>
	/// <returns>An HRESULT error code.</returns>
	STDMETHOD(GetFooterFocus)(int* pidxItem) = 0;

	/// <summary>
	/// Sets the list view control's focused footer item.
	/// </summary>
	/// <param name="idxItem">The zero-based index of the
	/// footer item to which to set the keyboard focus.</param>
	/// <returns>An HRESULT error code.</returns>
	STDMETHOD(SetFooterFocus)(int idxItem) = 0;

	/// <summary>
	/// Sets the title text of the list view control's
	/// footer area.
	/// </summary>
	/// <param name="pszText">The text to display in the
	/// footer area's title.</param>
	/// <returns>An HRESULT error code.</returns>
	STDMETHOD(SetIntroText)(PCWSTR pszText) = 0;

	/// <summary>
	/// Makes the list view control's footer area visible
	/// and registers the callback object that is notified
	/// about item clicks and item deletions.
	/// </summary>
	/// <param name="pCallbackObject">The IListViewFooterCallback
	/// implementation of the callback object to register.</param>
	/// <returns>An HRESULT error code.</returns>
	STDMETHOD(Show)(IListViewFooterCallback* pCallbackObject) = 0;

	/// <summary>
	/// Removes all footer items from the list view
	/// control's footer area.
	/// </summary>
	/// <returns>An HRESULT error code.</returns>
	STDMETHOD(RemoveAllButtons)(void) = 0;

	/// <summary>
	/// Inserts a footer item. Inserts a new footer item
	/// with the specified properties at the specified
	/// position into the list view
	/// control.
	/// </summary>
	/// <param name="posInsert">The zero-based index at
	/// which to insert the new footer item.</param>
	/// <param name="pszText">The new footer item's text.</param>
	/// <param name="pszShorterText">If there is not enough
	/// space, display this text.</param>
	/// <param name="idxImage">The zero-based index of the
	/// new footer item's icon.</param>
	/// <param name="lParam">The integer data that will be
	/// associated with the new footer item.</param>
	/// <returns>An HRESULT error code.</returns>
	STDMETHOD(InsertButton)(int posInsert, PCWSTR pszText, PCWSTR pszShorterText, UINT idxImage, LPARAM lParam) = 0;

	/// <summary>
	/// Retrieves the integer data associated with the
	/// specified footer item.
	/// </summary>
	/// <param name="idxItem">The zero-based index of
	/// the footer for which to retrieve the associated
	/// data.</param>
	/// <param name="pLParam">Receives the associated data.</param>
	/// <returns>An HRESULT error code.</returns>
	STDMETHOD(GetButtonLParam)(int idxItem, LPARAM* pLParam) = 0;
};

MIDL_INTERFACE("6E71A510-732A-4557-9596-A827E36DAF8F")
IPropertyControlBase :public IUnknown
{
public:
	// All parameter names have been guessed!

	// 2 -> Calendar control becomes a real calendar
	// control instead of a date picker (but with a
	// height of only 1 line)
	// 3 -> Calendar control becomes a simple text box
	typedef enum tagPROPDESC_CONTROL_TYPE PROPDESC_CONTROL_TYPE;
	typedef enum tagPROPCTL_RECT_TYPE PROPCTL_RECT_TYPE;
	STDMETHOD(Initialize)(LPUNKNOWN, PROPDESC_CONTROL_TYPE) = 0;

	/// <summary>
	/// called when editing group labels
	/// </summary>
	/// <param name="Unknown1">might be a LVGGR_* value</param>
	/// <param name="hDC">seems to be always NULL</param>
	/// <param name="pUnknown2">seems to be always NULL</param>
	/// <param name="pUnknown3">seems to receive the size set by the method</param>
	/// <returns></returns>
	STDMETHOD(GetSize)(PROPCTL_RECT_TYPE Unknown1, HDC hDC, SIZE const* pUnknown2, LPSIZE pUnknown3) = 0;
	STDMETHOD(SetWindowTheme)(PCWSTR pszSubAppName, PCWSTR pszSubIdList) = 0;
	STDMETHOD(SetFont)(HFONT hFont) = 0;
	STDMETHOD(SetTextColor)(COLORREF cr) = 0;
	STDMETHOD(GetFlags)(int* pFlags) = 0;

	// IPropertyControl:
	// called before the edit control is created and before it is dismissed
	// flags is 1 on first call and 0 on second call (mask is always 1)
	// 1 -> Maybe visibility
	STDMETHOD(SetFlags)(int Flags, int Mask) = 0;

	// possible values for Unknown2 (list may be incomplete):
	//   0x02 - mouse has been moved over the sub-item
	//   0x0C - ISubItemCallBack::EditSubItem has been called or a
	//   slow double click has been made
	STDMETHOD(AdjustWindowRectPCB)(HWND hWndListView, RECT* prcItem, RECT const* pUnknown1, int Unknown2) = 0;
	STDMETHOD(SetValue)(IUnknown*) = 0;
	STDMETHOD(InvokeDefaultAction)(void) = 0;
	STDMETHOD(Destroy)(void) = 0;
	STDMETHOD(SetFormatFlags)(int) = 0;
	STDMETHOD(GetFormatFlags)(int*) = 0;
};

MIDL_INTERFACE("5E82A4DD-9561-476A-8634-1BEBACBA4A38")
IPropertyControl :public IPropertyControlBase
{
public:
	STDMETHOD(GetValue)(REFIID riid, void** ppv) = 0;

	// possible values for Unknown2 (list may be incomplete):
	//   0x02 - mouse has been moved over the sub-item
	//   0x0C - ISubItemCallBack::EditSubItem has been called or
	//   a slow double click has been made
	STDMETHOD(Create)(HWND hWndParent, RECT const* prcItem, RECT const* pUnknown1, int Unknown2) = 0;
	STDMETHOD(SetPosition)(RECT const*, RECT const*) = 0;
	STDMETHOD(IsModified)(BOOL* pbModified) = 0;
	STDMETHOD(SetModified)(BOOL bModified) = 0;
	STDMETHOD(ValidationFailed)(PCWSTR) = 0;
	STDMETHOD(GetState)(int* pState) = 0;
};

MIDL_INTERFACE("11A66240-5489-42C2-AEBF-286FC831524C")
ISubItemCallback :public IUnknown
{
public:
	/// \brief <em>Retrieves the title of the specified sub-item</em>
	///
	/// Retrieves the title of the specified sub-item. This title is displayed in extended tile view mode
	/// in front of the sub-item's value.
	///
	/// \param[in] idxSubItem The one-based index of the sub-item for which to retrieve the title.
	/// \param[out] pBuffer Receives the sub-item's title.
	/// \param[in] bufferSize The size of the buffer (in characters) specified by the \c pBuffer parameter.
	///
	/// \return An \c HRESULT error code.
	STDMETHOD(GetSubItemTitle)(int idxSubItem, PWSTR pBuffer, int bufferSize) = 0;
	/// \brief <em>Retrieves the control representing the specified sub-item</em>
	///
	/// Retrieves the control representing the specified sub-item. The control is used for drawing the
	/// sub-item.\n
	/// Starting with comctl32.dll version 6.10, sub-items can be represented by objects that implement
	/// the \c IPropertyControlBase interface. Representation means drawing the sub-item (by implementing the
	/// \c IDrawPropertyControl interface) and/or editing the sub-item in-place (by implementing the
	/// \c IPropertyControl interface, which allows in-place editing with a complex user interface). The
	/// object that represents the sub-item is retrieved dynamically.\n
	/// This method retrieves the sub-item control that is used for drawing the sub-item.
	///
	/// \param[in] idxItem The zero-based index of the item for which to retrieve the sub-item control.
	/// \param[in] idxSubItem The one-based index of the sub-item for which to retrieve the sub-item
	///            control.
	/// \param[in] riid The IID of the interface of which the sub-item control's implementation
	///            will be returned.
	/// \param[out] ppv Receives the sub-item control's implementation of the interface identified by
	///             \c riid.
	///
	/// \return An \c HRESULT error code.
	///
	/// \remarks With current versions of comctl32.dll, providing a sub-item control is the only way to
	///          custom-draw sub-items in Tiles view mode.
	///
	/// \sa BeginSubItemEdit, IPropertyControlBase, IPropertyControl, IDrawPropertyControl
	STDMETHOD(GetSubItemControl)(int idxItem, int idxSubItem, REFIID riid, void** ppv) = 0;
	/// \brief <em>Retrieves the control used to edit the specified sub-item</em>
	///
	/// Retrieves the control that will be used to edit the specified sub-item. The control is used for
	/// editing the sub-item.\n
	/// Starting with comctl32.dll version 6.10, sub-items can be represented by objects that implement
	/// the \c IPropertyControlBase interface. Representation means drawing the sub-item (by implementing the
	/// \c IDrawPropertyControl interface) and/or editing the sub-item in-place (by implementing the
	/// \c IPropertyControl interface, which allows in-place editing with a complex user interface). The
	/// object that represents the sub-item is retrieved dynamically.\n
	/// This method retrieves the sub-item control that is used for editing the sub-item.
	///
	/// \param[in] idxItem The zero-based index of the item for which to retrieve the sub-item control.
	/// \param[in] idxSubItem The one-based index of the sub-item for which to retrieve the sub-item
	///            control.
	/// \param[in] mode If set to 0, the edit mode has been entered by moving the mouse over the sub-item.
	///            If set to 1, the edit mode has been entered by calling \c IListView::EditSubItem or by
	///            clicking on the sub-item.
	/// \param[in] riid The IID of the interface of which the sub-item control's implementation
	///            will be returned.
	/// \param[out] ppv Receives the sub-item control's implementation of the interface identified by
	///             \c riid.
	///
	/// \return An \c HRESULT error code.
	///
	/// \sa EndSubItemEdit, GetSubItemControl, IPropertyControlBase,
	///     IPropertyControl, IDrawPropertyControl
	STDMETHOD(BeginSubItemEdit)(int idxItem, int idxSubItem, int mode, REFIID riid, void** ppv) = 0;
	/// \brief <em>Notifies the control that editing the specified sub-item has ended</em>
	///
	/// Notifies the control that editing the specified sub-item using the specified sub-item control has
	/// been finished.\n
	/// Starting with comctl32.dll version 6.10, sub-items can be represented by objects that implement
	/// the \c IPropertyControlBase interface. Representation means drawing the sub-item (by implementing the
	/// \c IDrawPropertyControl interface) and/or editing the sub-item in-place (by implementing the
	/// \c IPropertyControl interface, which allows in-place editing with a complex user interface). The
	/// object that represents the sub-item is retrieved dynamically.
	///
	/// \param[in] idxItem The zero-based index of the item whose sub-item has been edited.
	/// \param[in] idxSubItem The one-based index of the sub-item that has been edited.
	/// \param[in] mode If set to 0, the edit mode has been entered by moving the mouse over the sub-item.
	///            If set to 1, the edit mode has been entered by calling \c IListView::EditSubItem or by
	///            clicking on the sub-item.
	/// \param[in] pPropertyControl The property control that has been used to edit the sub-item. This
	///            property control has to be destroyed by this method.
	///
	/// \return An \c HRESULT error code.
	///
	/// \remarks Call \c IPropertyControl::IsModified to retrieve whether editing the sub-item has been
	///          completed or canceled.
	///
	/// \sa BeginSubItemEdit, IListView_WINVISTA::EditSubItem, IListView_WIN7::EditSubItem,
	///     IPropertyControlBase, IPropertyControl, IDrawPropertyControl, IPropertyControlBase::Destroy,
	///     IPropertyControl::IsModified
	STDMETHOD(EndSubItemEdit)(int idxItem, int idxSubItem, int mode, IPropertyControl* pPropertyControl) = 0;
	// TBD
	STDMETHOD(BeginGroupEdit)(int idxGroup, REFIID riid, void** ppv) = 0;
	// TBD
	STDMETHOD(EndGroupEdit)(int idxGroup, int mode, IPropertyControl* pPropertyControl) = 0;
	/// \brief <em>Notifies the control that the specified verb has been invoked on the specified item</em>
	///
	/// Notifies the control that an action identified by the specified verb has been invoked on the
	/// specified item. The action usually is generated by the user. The sub-item control translates the
	/// action into a verb which it invokes.\n
	/// For the hyperlink sub-item control the action is clicking the link and the verb is the string that
	/// has been specified as the \c id attribute of the hyperlink markup
	/// (&lt;a id=&quot;<i>verb</i>&quot;&gt;<i>text</i>&lt;/a&gt;).
	///
	/// \param[in] idxItem The zero-based index of the item on which the verb is being invoked.
	/// \param[in] pVerb The verb identifying the action.
	///
	/// \return An \c HRESULT error code.
	///
	/// \sa GetSubItemControl, IPropertyControlBase::InvokeDefaultAction
	STDMETHOD(OnInvokeVerb)(int idxItem, PCWSTR pVerb) = 0;
};

MIDL_INTERFACE("E6DFF6FD-BCD5-4162-9C65-A3B18C616FDB")
IDrawPropertyControl :public IPropertyControlBase
{
public:
	// All parameter names have been guessed!
	STDMETHOD(GetDrawFlags)(int*) = 0;
	STDMETHOD(WindowlessDraw)(HDC hDC, RECT const* pDrawingRectangle, int a) = 0;
	STDMETHOD(HasVisibleContent)(void) = 0;
	STDMETHOD(GetDisplayText)(PWSTR*) = 0;
	STDMETHOD(GetTooltipInfo)(HDC, SIZE const*, int*) = 0;
};

MIDL_INTERFACE("44C09D56-8D3B-419D-A462-7B956B105B47")
IOwnerDataCallback :public IUnknown
{
public:
	/// \brief <em>TODO</em>
	///
	/// TODO
	///
	/// \return An \c HRESULT error code.
	STDMETHOD(GetItemPosition)(int idxItem, LPPOINT pPosition) = 0;
	/// \brief <em>TODO</em>
	///
	/// TODO
	///
	/// \return An \c HRESULT error code.
	STDMETHOD(SetItemPosition)(int idxItem, POINT position) = 0;
	/// \brief <em>Will be called to retrieve an item's zero-based control-wide index</em>
	///
	/// This method is called by the listview control to retrieve an item's zero-based control-wide index.
	/// The item is identified by a zero-based group index, which identifies the listview group in which
	/// the item is displayed, and a zero-based group-wide item index, which identifies the item within its
	/// group.
	///
	/// \param[in] idxGroup The zero-based index of the listview group containing the item.
	/// \param[in] groupWideItemIndex The item's zero-based group-wide index within the listview group
	///            specified by \c idxGroup.
	/// \param[out] pTotalItemIndex Receives the item's zero-based control-wide index.
	///
	/// \return An \c HRESULT error code.
	STDMETHOD(GetItemInGroup)(int idxGroup, int groupWideItemIndex, int* pTotalItemIndex) = 0;
	/// \brief <em>Will be called to retrieve the group containing a specific occurrence of an item</em>
	///
	/// This method is called by the listview control to retrieve the listview group in which the specified
	/// occurrence of the specified item is displayed.
	///
	/// \param[in] idxItem The item's zero-based (control-wide) index.
	/// \param[in] occurrenceIndex The zero-based index of the item's copy for which the group membership is
	///            retrieved.
	/// \param[out] pGroupIndex Receives the zero-based index of the listview group that shall contain the
	///             specified copy of the specified item.
	///
	/// \return An \c HRESULT error code.
	STDMETHOD(GetItemGroup)(int idxItem, int occurenceIndex, int* pGroupIndex) = 0;
	/// \brief <em>Will be called to determine how often an item occurs in the listview control</em>
	///
	/// This method is called by the listview control to determine how often the specified item occurs in the
	/// listview control.
	///
	/// \param[in] idxItem The item's zero-based (control-wide) index.
	/// \param[out] pOccurrencesCount Receives the number of occurrences of the item in the listview control.
	///
	/// \return An \c HRESULT error code.
	STDMETHOD(GetItemGroupCount)(int idxItem, int* pOccurenceCount) = 0;
	/// \brief <em>Will be called to prepare the client app that the data for a certain range of items will be required very soon</em>
	///
	/// This method is similar to the \c LVN_ODCACHEHINT notification. It tells the client application that
	/// it should preload the details for a certain range of items because the listview control is about to
	/// request these details. The difference to \c LVN_ODCACHEHINT is that this method identifies the items
	/// by their zero-based group-wide index and the zero-based index of the listview group containing the
	/// item.
	///
	/// \param[in] firstItem The first item to cache.
	/// \param[in] lastItem The last item to cache.
	///
	/// \return An \c HRESULT error code.
	STDMETHOD(OnCacheHint)(LVITEMINDEX firstItem, LVITEMINDEX lastItem) = 0;
};

MIDL_INTERFACE("E5B16AF2-3990-4681-A609-1F060CD14269")
IListView2 :public IOleWindow
{
public:
	STDMETHOD(GetImageList)(int iType, HIMAGELIST * phIL) = 0;
	STDMETHOD(SetImageList)(int iType, HIMAGELIST hILNew, HIMAGELIST* phILOld) = 0;
	STDMETHOD(GetBackgroundColor)(COLORREF* pcr) = 0;
	STDMETHOD(SetBackgroundColor)(COLORREF cr) = 0;
	STDMETHOD(GetTextColor)(COLORREF* pcr) = 0;
	STDMETHOD(SetTextColor)(COLORREF cr) = 0;
	STDMETHOD(GetTextBackgroundColor)(COLORREF* pcr) = 0;
	STDMETHOD(SetTextBackgroundColor)(COLORREF cr) = 0;
	STDMETHOD(GetHotLightColor)(COLORREF* pcr) = 0;
	STDMETHOD(SetHotLightColor)(COLORREF cr) = 0;
	STDMETHOD(GetItemCount)(int* pItemCount) = 0;
	STDMETHOD(SetItemCount)(int cItem, DWORD dwFlags) = 0;
	STDMETHOD(GetItem)(LVITEMW* pItem) = 0;
	STDMETHOD(SetItem)(LVITEMW* const pItem) = 0;
	STDMETHOD(GetItemState)(int idxItem, int idxSubItem, ULONG mask, ULONG* pState) = 0;
	STDMETHOD(SetItemState)(int idxItem, int idxSubItem, ULONG mask, ULONG state) = 0;
	STDMETHOD(GetItemText)(int idxItem, int idxSubItem, PWSTR pBuffer, int bufferSize) = 0;
	STDMETHOD(SetItemText)(int idxItem, int idxSubItem, PCWSTR pText) = 0;
	STDMETHOD(GetBackgroundImage)(LVBKIMAGEW* pBkImage) = 0;
	STDMETHOD(SetBackgroundImage)(LVBKIMAGEW* const pBkImage) = 0;
	STDMETHOD(GetFocusedColumn)(int* pColumnIndex) = 0;
	STDMETHOD(SetSelectionFlags)(ULONG uMask, ULONG uFlags) = 0;
	STDMETHOD(GetSelectedColumn)(int* pColumnIndex) = 0;
	STDMETHOD(SetSelectedColumn)(int columnIndex) = 0;
	STDMETHOD(GetView)(DWORD* pView) = 0;
	STDMETHOD(SetView)(DWORD view) = 0;
	STDMETHOD(InsertItem)(LVITEMW* const pItem, int* pItemIndex) = 0;
	STDMETHOD(DeleteItem)(int idxItem) = 0;
	STDMETHOD(DeleteAllItems)(void) = 0;
	STDMETHOD(UpdateItem)(int idxItem) = 0;
	STDMETHOD(GetItemRect)(LVITEMINDEX idxItem, int rectangleType, LPRECT pRectangle) = 0;
	STDMETHOD(GetSubItemRect)(LVITEMINDEX idxItem, int idxSubItem, int rectangleType, LPRECT pRectangle) = 0;
	STDMETHOD(HitTestSubItem)(LVHITTESTINFO* pHitTestData) = 0;
	STDMETHOD(GetIncrSearchString)(PWSTR pBuffer, int bufferSize, int* pCopiedChars) = 0;
	// pHorizontalSpacing and pVerticalSpacing may be in wrong order
	STDMETHOD(GetItemSpacing)(BOOL bSmallIconView, int* pHorizontalSpacing, int* pVerticalSpacing) = 0;
	// parameters may be in wrong order
	STDMETHOD(SetIconSpacing)(int horizontalSpacing, int verticalSpacing, int* pHorizontalSpacing, int* pVerticalSpacing) = 0;
	STDMETHOD(GetNextItem)(LVITEMINDEX idxItem, ULONG flags, LVITEMINDEX* pNextItemIndex) = 0;
	STDMETHOD(FindItem)(LVITEMINDEX startItemIndex, LVFINDINFOW const* pFindInfo, LVITEMINDEX* pFoundItemIndex) = 0;
	STDMETHOD(GetSelectionMark)(LVITEMINDEX* pSelectionMark) = 0;
	STDMETHOD(SetSelectionMark)(LVITEMINDEX newSelectionMark, LVITEMINDEX* pOldSelectionMark) = 0;
	STDMETHOD(GetItemPosition)(LVITEMINDEX idxItem, POINT* pPosition) = 0;
	STDMETHOD(SetItemPosition)(int idxItem, POINT const* pPosition) = 0;
	// parameters may be in wrong order
	STDMETHOD(ScrollView)(int horizontalScrollDistance, int verticalScrollDistance) = 0;
	STDMETHOD(EnsureItemVisible)(LVITEMINDEX idxItem, BOOL partialOk) = 0;
	STDMETHOD(EnsureSubItemVisible)(LVITEMINDEX idxItem, int idxSubItem) = 0;
	STDMETHOD(EditSubItem)(LVITEMINDEX idxItem, int idxSubItem) = 0;
	STDMETHOD(RedrawItems)(int firstItemIndex, int lastItemIndex) = 0;
	STDMETHOD(ArrangeItems)(int mode) = 0;
	STDMETHOD(RecomputeItems)(int unknown) = 0;
	STDMETHOD(GetEditControl)(HWND* pHWndEdit) = 0;
	// TODO: verify that 'initialEditText' really is used to specify the initial text
	STDMETHOD(EditLabel)(LVITEMINDEX idxItem, PCWSTR initialEditText, HWND* phWndEdit) = 0;
	STDMETHOD(EditGroupLabel)(int idxGroup) = 0;
	STDMETHOD(CancelEditLabel)(void) = 0;
	STDMETHOD(GetEditItem)(LVITEMINDEX* idxItem, int* idxSubItem) = 0;
	STDMETHOD(HitTest)(LVHITTESTINFO* pHitTestData) = 0;
	STDMETHOD(GetStringWidth)(PCWSTR pString, int* pWidth) = 0;
	STDMETHOD(GetColumn)(int columnIndex, LVCOLUMNW* pColumn) = 0;
	STDMETHOD(SetColumn)(int columnIndex, LVCOLUMNW* const pColumn) = 0;
	STDMETHOD(GetColumnOrderArray)(int numberOfColumns, int* pColumns) = 0;
	STDMETHOD(SetColumnOrderArray)(int numberOfColumns, int const* pColumns) = 0;
	STDMETHOD(GetHeaderControl)(HWND* pHWndHeader) = 0;
	STDMETHOD(InsertColumn)(int insertAt, LVCOLUMNW* const pColumn, int* pColumnIndex) = 0;
	STDMETHOD(DeleteColumn)(int columnIndex) = 0;
	STDMETHOD(CreateDragImage)(int idxItem, POINT const* pUpperLeft, HIMAGELIST* pHImageList) = 0;
	STDMETHOD(GetViewRect)(RECT* pRectangle) = 0;
	STDMETHOD(GetClientRect)(BOOL unknown, RECT* pClientRectangle) = 0;
	STDMETHOD(GetColumnWidth)(int columnIndex, int* pWidth) = 0;
	STDMETHOD(SetColumnWidth)(int columnIndex, int width) = 0;
	STDMETHOD(GetCallbackMask)(ULONG* pMask) = 0;
	STDMETHOD(SetCallbackMask)(ULONG mask) = 0;
	STDMETHOD(GetTopIndex)(int* pTopIndex) = 0;
	STDMETHOD(GetCountPerPage)(int* pCountPerPage) = 0;
	STDMETHOD(GetOrigin)(POINT* pOrigin) = 0;
	STDMETHOD(GetSelectedCount)(int* pSelectedCount) = 0;
	// 'unknown' might specify whether to pass items' data or indexes
	STDMETHOD(SortItems)(BOOL unknown, LPARAM lParam, PFNLVCOMPARE pComparisonFunction) = 0;
	STDMETHOD(GetExtendedStyle)(DWORD* pStyle) = 0;
	// parameters may be in wrong order
	STDMETHOD(SetExtendedStyle)(DWORD mask, DWORD style, DWORD* pOldStyle) = 0;
	STDMETHOD(GetHoverTime)(UINT* pTime) = 0;
	STDMETHOD(SetHoverTime)(UINT time, UINT* pOldSetting) = 0;
	STDMETHOD(GetToolTip)(HWND* pHWndToolTip) = 0;
	STDMETHOD(SetToolTip)(HWND hWndToolTip, HWND* pHWndOldToolTip) = 0;
	STDMETHOD(GetHotItem)(LVITEMINDEX* pHotItem) = 0;
	STDMETHOD(SetHotItem)(LVITEMINDEX newHotItem, LVITEMINDEX* pOldHotItem) = 0;
	STDMETHOD(GetHotCursor)(HCURSOR* pHCursor) = 0;
	STDMETHOD(SetHotCursor)(HCURSOR hCursor, HCURSOR* pHOldCursor) = 0;
	// parameters may be in wrong order
	STDMETHOD(ApproximateViewRect)(int cItem, int* pWidth, int* pHeight) = 0;
	STDMETHOD(SetRangeObject)(int unknown, void*/*ILVRange**/ pObject) = 0;
	STDMETHOD(GetWorkAreas)(int numberOfWorkAreas, RECT* pWorkAreas) = 0;
	STDMETHOD(SetWorkAreas)(int numberOfWorkAreas, RECT const* pWorkAreas) = 0;
	STDMETHOD(GetWorkAreaCount)(int* pNumberOfWorkAreas) = 0;
	STDMETHOD(ResetEmptyText)(void) = 0;
	STDMETHOD(EnableGroupView)(BOOL enable) = 0;
	STDMETHOD(IsGroupViewEnabled)(BOOL* pIsEnabled) = 0;
	STDMETHOD(SortGroups)(PFNLVGROUPCOMPARE pComparisonFunction, PVOID lParam) = 0;
	STDMETHOD(GetGroupInfo)(int unknown1, int unknown2, LVGROUP* pGroup) = 0;
	STDMETHOD(SetGroupInfo)(int unknown, int groupID, LVGROUP* const pGroup) = 0;
	STDMETHOD(GetGroupRect)(BOOL unknown, int groupID, int rectangleType, RECT* pRectangle) = 0;
	STDMETHOD(GetGroupState)(int groupID, ULONG mask, ULONG* pState) = 0;
	STDMETHOD(HasGroup)(int groupID, BOOL* pHasGroup) = 0;
	STDMETHOD(InsertGroup)(int insertAt, LVGROUP* const pGroup, int* pGroupID) = 0;
	STDMETHOD(RemoveGroup)(int groupID) = 0;
	STDMETHOD(InsertGroupSorted)(LVINSERTGROUPSORTED const* pGroup, int* pGroupID) = 0;
	STDMETHOD(GetGroupMetrics)(LVGROUPMETRICS* pMetrics) = 0;
	STDMETHOD(SetGroupMetrics)(LVGROUPMETRICS* const pMetrics) = 0;
	STDMETHOD(RemoveAllGroups)(void) = 0;
	STDMETHOD(GetFocusedGroup)(int* pGroupID) = 0;
	STDMETHOD(GetGroupCount)(int* pCount) = 0;
	STDMETHOD(SetOwnerDataCallback)(IOwnerDataCallback* pCallback) = 0;
	STDMETHOD(GetTileViewInfo)(LVTILEVIEWINFO* pInfo) = 0;
	STDMETHOD(SetTileViewInfo)(LVTILEVIEWINFO* const pInfo) = 0;
	STDMETHOD(GetTileInfo)(LVTILEINFO* pTileInfo) = 0;
	STDMETHOD(SetTileInfo)(LVTILEINFO* const pTileInfo) = 0;
	STDMETHOD(GetInsertMark)(LVINSERTMARK* pInsertMarkDetails) = 0;
	STDMETHOD(SetInsertMark)(LVINSERTMARK const* pInsertMarkDetails) = 0;
	STDMETHOD(GetInsertMarkRect)(LPRECT pInsertMarkRectangle) = 0;
	STDMETHOD(GetInsertMarkColor)(COLORREF* pcr) = 0;
	STDMETHOD(SetInsertMarkColor)(COLORREF color, COLORREF* pOldColor) = 0;
	STDMETHOD(HitTestInsertMark)(POINT const* pPoint, LVINSERTMARK* pInsertMarkDetails) = 0;
	STDMETHOD(SetInfoTip)(LVSETINFOTIP* const pInfoTip) = 0;
	STDMETHOD(GetOutlineColor)(COLORREF* pcr) = 0;
	STDMETHOD(SetOutlineColor)(COLORREF color, COLORREF* pOldColor) = 0;
	STDMETHOD(GetFrozenItem)(int* pItemIndex) = 0;
	// one parameter will be the item index; works in Icons view only
	STDMETHOD(SetFrozenItem)(int unknown1, int unknown2) = 0;
	STDMETHOD(GetFrozenSlot)(RECT* pUnknown) = 0;
	STDMETHOD(SetFrozenSlot)(int unknown1, POINT const* pUnknown2) = 0;
	STDMETHOD(GetViewMargin)(RECT* pMargin) = 0;
	STDMETHOD(SetViewMargin)(RECT const* pMargin) = 0;
	STDMETHOD(SetKeyboardSelected)(LVITEMINDEX idxItem) = 0;
	STDMETHOD(MapIndexToId)(int idxItem, int* pItemID) = 0;
	STDMETHOD(MapIdToIndex)(int itemID, int* pItemIndex) = 0;
	STDMETHOD(IsItemVisible)(LVITEMINDEX idxItem, BOOL* pVisible) = 0;
	STDMETHOD(EnableAlphaShadow)(BOOL enable) = 0;
	STDMETHOD(GetGroupSubsetCount)(int* pNumberOfRowsDisplayed) = 0;
	STDMETHOD(SetGroupSubsetCount)(int numberOfRowsToDisplay) = 0;
	STDMETHOD(GetVisibleSlotCount)(int* pCount) = 0;
	STDMETHOD(GetColumnMargin)(RECT* pMargin) = 0;
	STDMETHOD(SetSubItemCallback)(ISubItemCallback* pCallback) = 0;
	STDMETHOD(GetVisibleItemRange)(LVITEMINDEX* pFirstItem, LVITEMINDEX* pLastItem) = 0;
	STDMETHOD(SetTypeAheadFlags)(UINT mask, UINT flags) = 0;
};