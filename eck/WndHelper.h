/*
* WinEzCtrlKit Library
*
* WndHelper.h ： 窗口操作帮助函数
*
* Copyright(C) 2023-2024 QingKong
*/
#pragma once
#include "ECK.h"
#include "Utility.h"

#include <vssym32.h>


#define ECK_DS_BEGIN(StructName)	struct StructName {
#define ECK_DS_END_VAR(VarName)		} VarName{};
#define ECK_DS_END()				};
#define ECK_DS_ENTRY(Name, Size)	const int	o_##Name = Size; int	Name = Size;
#define ECK_DS_ENTRY_F(Name, Size)	const float o_##Name = Size; float	Name = Size;

#define ECK_HANDLE_WM_MOUSELEAVE(hWnd, wParam, lParam, fn) \
	((fn)((hWnd)), 0L)
#define ECK_HANDLE_WM_DPICHANGED(hWnd, wParam, lParam, fn) \
	((fn)((hWnd), (int)(short)LOWORD(wParam), (int)(short)HIWORD(wParam), (RECT*)(lParam)), 0L)

#define ECK_STYLE_GETSET(Name, Style)					\
	BOOL StyleGet##Name()								\
	{													\
		return IsBitSet(GetStyle(), Style);				\
	}													\
	void StyleSet##Name(BOOL b)							\
	{													\
		ModifyStyle((b ? Style : 0), Style, GWL_STYLE); \
	}

#define ECK_CWNDPROP_STYLE(Name, Style)					\
	ECKPROP(StyleGet##Name, StyleSet##Name) BOOL Name;	\
	ECK_STYLE_GETSET(Name, Style)

ECK_NAMESPACE_BEGIN
struct NMFOCUS
{
	NMHDR nmhdr;
	HWND hWnd;
};

constexpr inline UINT WM_USER_SAFE = WM_USER + 3;

constexpr inline UINT CS_STDWND = CS_DBLCLKS | CS_VREDRAW | CS_HREDRAW;

EckInline DWORD ModifyWindowStyle(HWND hWnd, DWORD dwNew, DWORD dwMask, int idx = GWL_STYLE)
{
	DWORD dwStyle = (DWORD)GetWindowLongPtrW(hWnd, idx);
	dwStyle &= ~dwMask;
	dwStyle |= dwNew;
	SetWindowLongPtrW(hWnd, idx, dwStyle);
	return dwStyle;
}

EckInline int GetDpi(HWND hWnd)
{
#if ECKDPIAPI
	if (hWnd)
		return GetDpiForWindow(hWnd);
	else
		return GetDpiForSystem();
#else
	HDC hDC = GetDC(hWnd);
	int i = GetDeviceCaps(hDC, LOGPIXELSX);
	ReleaseDC(hWnd, hDC);
	return i;
#endif
}

EckInline int DpiScale(int i, int iDpiNew, int iDpiOld)
{
	return i * iDpiNew / iDpiOld;
}

EckInline int DpiScale(int i, int iDpi)
{
	return DpiScale(i, iDpi, USER_DEFAULT_SCREEN_DPI);
}

EckInline float DpiScaleF(float i, int iDpiNew, int iDpiOld)
{
	return i * iDpiNew / iDpiOld;
}

EckInline float DpiScaleF(float i, int iDpi)
{
	return DpiScaleF(i, iDpi, USER_DEFAULT_SCREEN_DPI);
}

EckInline void DpiScale(RECT& rc, int iDpiNew, int iDpiOld)
{
	rc.left = rc.left * iDpiNew / iDpiOld;
	rc.top = rc.top * iDpiNew / iDpiOld;
	rc.right = rc.right * iDpiNew / iDpiOld;
	rc.bottom = rc.bottom * iDpiNew / iDpiOld;
}

EckInline void DpiScale(RECT& rc, int iDpi)
{
	DpiScale(rc, iDpi, USER_DEFAULT_SCREEN_DPI);
}

EckInline void DpiScale(SIZE& size, int iDpiNew, int iDpiOld)
{
	size.cx = size.cx * iDpiNew / iDpiOld;
	size.cy = size.cy * iDpiNew / iDpiOld;
}

EckInline void DpiScale(SIZE& size, int iDpi)
{
	DpiScale(size, iDpi, USER_DEFAULT_SCREEN_DPI);
}

/// <summary>
/// 创建字体
/// </summary>
/// <param name="pszFontName">字体名称</param>
/// <param name="iPoint">点数</param>
/// <param name="iWeight">权重</param>
/// <param name="bItalic">是否倾斜</param>
/// <param name="bUnderline">是否下划线</param>
/// <param name="bStrikeOut">是否删除线</param>
/// <param name="hWnd">计算高度时的参照窗口，将使用此窗口的DC来度量，默认使用桌面窗口</param>
/// <returns>字体句柄</returns>
EckInline HFONT EzFont(PCWSTR pszFontName, int iPoint, int iWeight = 400, BOOL bItalic = FALSE,
	BOOL bUnderline = FALSE, BOOL bStrikeOut = FALSE, HWND hWnd = NULL, DWORD dwCharSet = GB2312_CHARSET)
{
	HDC hDC = GetDC(hWnd);
	int iSize;
	iSize = -MulDiv(iPoint, GetDeviceCaps(hDC, LOGPIXELSY), 72);
	ReleaseDC(hWnd, hDC);
	return CreateFontW(iSize, 0, 0, 0, iWeight, bItalic, bUnderline,
		bStrikeOut, dwCharSet, 0, 0, 0, 0, pszFontName);
}

/// <summary>
/// 设置窗口字体。
/// 函数枚举窗口的所有子窗口并悉数设置字体
/// </summary>
/// <param name="hWnd">窗口句柄</param>
/// <param name="hFont">字体句柄</param>
/// <param name="bRedraw">是否重画</param>
EckInline void SetFontForWndAndCtrl(HWND hWnd, HFONT hFont, BOOL bRedraw = FALSE)
{
	EnumChildWindows(hWnd,
		[](HWND hWnd, LPARAM lParam)->BOOL
		{
			SendMessageW(hWnd, WM_SETFONT, lParam, FALSE);
			return TRUE;
		}, (LPARAM)hFont);
	if (bRedraw)
		RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_UPDATENOW);
}

/// <summary>
/// 按新旧DPI重新创建字体
/// </summary>
/// <param name="hFont">字体</param>
/// <param name="iDpiNew">新DPI</param>
/// <param name="iDpiOld">旧DPI</param>
/// <param name="bDeletePrevFont">是否删除hFont指示的字体，默认FALSE</param>
/// <returns></returns>
EckInline HFONT ReCreateFontForDpiChanged(HFONT hFont, int iDpiNew, int iDpiOld, BOOL bDeletePrevFont = FALSE)
{
	LOGFONTW lf;
	GetObjectW(hFont, sizeof(lf), &lf);
	if (bDeletePrevFont)
		DeleteObject(hFont);
	lf.lfHeight = DpiScale(lf.lfHeight, iDpiNew, iDpiOld);
	return CreateFontIndirectW(&lf);
}

/// <summary>
/// 按新旧DPI重新设置窗口字体
/// </summary>
/// <param name="hWnd">窗口句柄</param>
/// <param name="iDpiNew">新DPI</param>
/// <param name="iDpiOld">旧DPI</param>
/// <param name="bRedraw">是否重画，默认TRUE</param>
/// <param name="bDeletePrevFont">是否删除先前的窗口字体，默认FALSE</param>
/// <returns></returns>
EckInline HFONT ResetFontForDpiChanged(HWND hWnd, int iDpiNew, int iDpiOld, BOOL bRedraw = TRUE, BOOL bDeletePrevFont = FALSE)
{
	HFONT hFontNew = ReCreateFontForDpiChanged(
		(HFONT)SendMessageW(hWnd, WM_GETFONT, 0, 0), 
		iDpiNew, iDpiOld, bDeletePrevFont);
	SendMessageW(hWnd, WM_SETFONT, (WPARAM)hFontNew, bRedraw);
	return hFontNew;
}

inline BOOL GetWindowClientRect(HWND hWnd, HWND hParent, RECT& rc)
{
	if (!GetWindowRect(hWnd, &rc))
		return FALSE;
	int cx = rc.right - rc.left,
		cy = rc.bottom - rc.top;
	if (!ScreenToClient(hParent, (POINT*)&rc))
		return FALSE;
	rc.right = rc.left + cx;
	rc.bottom = rc.top + cy;
	return TRUE;
}

/// <summary>
/// 通过设置图像列表来设置ListView行高
/// </summary>
/// <param name="hLV"></param>
/// <param name="cy"></param>
/// <returns></returns>
EckInline HIMAGELIST LVSetItemHeight(HWND hLV, int cy)
{
	return (HIMAGELIST)SendMessageW((hLV), LVM_SETIMAGELIST, LVSIL_SMALL, (LPARAM)ImageList_Create(1, (cy), 0, 1, 0));
}

EckInline BOOL BitBltPs(const PAINTSTRUCT* pps, HDC hdcSrc)
{
	return BitBlt(pps->hdc,
		pps->rcPaint.left,
		pps->rcPaint.top,
		pps->rcPaint.right - pps->rcPaint.left,
		pps->rcPaint.bottom - pps->rcPaint.top,
		hdcSrc,
		pps->rcPaint.left,
		pps->rcPaint.top,
		SRCCOPY);
}

EckInline WNDPROC SetWindowProc(HWND hWnd, WNDPROC pfnWndProc)
{
	return (WNDPROC)SetWindowLongPtrW(hWnd, GWLP_WNDPROC, (LONG_PTR)pfnWndProc);
}

#ifdef ECKMACRO_NO_WIN11_22621
EckInline HRESULT EnableWindowMica(HWND hWnd, DWORD uType = 2)
{
	return E_FAIL;
}
#else
EckInline HRESULT EnableWindowMica(HWND hWnd, DWM_SYSTEMBACKDROP_TYPE uType = DWMSBT_MAINWINDOW)
{
	return DwmSetWindowAttribute(hWnd, DWMWA_SYSTEMBACKDROP_TYPE, &uType, sizeof(uType));
}
#endif

inline LRESULT MsgOnNcCalcSize(WPARAM wParam, LPARAM lParam, const MARGINS& Margins)
{
	if (wParam)
	{
		auto pnccsp = (NCCALCSIZE_PARAMS*)lParam;
		pnccsp->rgrc[0].left += Margins.cxLeftWidth;
		pnccsp->rgrc[0].top += Margins.cyTopHeight;
		pnccsp->rgrc[0].right -= Margins.cxRightWidth;
		pnccsp->rgrc[0].bottom -= Margins.cyBottomHeight;
	}
	else
	{
		auto prc = (RECT*)lParam;
		prc->left += Margins.cxLeftWidth;
		prc->top += Margins.cyTopHeight;
		prc->right -= Margins.cxRightWidth;
		prc->bottom -= Margins.cyBottomHeight;
	}
	return 0;
}

/// <summary>
/// 按边距执行非客户区命中测试
/// </summary>
/// <param name="pt">测试点的坐标，相对客户区</param>
/// <param name="Margins">边距</param>
/// <param name="cxWnd">窗口宽度</param>
/// <param name="cyWnd">窗口高度</param>
/// <returns>若指定点在边框内，返回对应的测试代码，否则返回HTCAPTION</returns>
inline LRESULT MsgOnNcHitTest(POINT pt, const MARGINS& Margins, int cxWnd, int cyWnd)
{
	if (pt.x < Margins.cxLeftWidth)
	{
		if (pt.y < Margins.cyTopHeight)
			return HTTOPLEFT;
		else if (pt.y > cyWnd - Margins.cyBottomHeight)
			return HTBOTTOMLEFT;
		else
			return HTLEFT;
	}
	else if (pt.x > cxWnd - Margins.cxRightWidth)
	{
		if (pt.y < Margins.cyTopHeight)
			return HTTOPRIGHT;
		else if (pt.y > cyWnd - Margins.cyBottomHeight)
			return HTBOTTOMRIGHT;
		else
			return HTRIGHT;
	}
	else
	{
		if (pt.y < Margins.cyTopHeight)
			return HTTOP;
		else if (pt.y > cyWnd - Margins.cyBottomHeight)
			return HTBOTTOM;
		else
			return HTCAPTION;
	}
}

template<class T>
EckInline void UpdateDpiSize(T& Dpis, int iDpi)
{
	for (int* p = ((int*)&Dpis) + 1; p < PtrSkipType(&Dpis); p += 2)
		*p = DpiScale(*(p - 1), iDpi);
}

template<class T>
EckInline void UpdateDpiSizeF(T& Dpis, int iDpi)
{
	for (float* p = ((float*)&Dpis) + 1; p < PtrSkipType(&Dpis); p += 2)
		*p = DpiScaleF(*(p - 1), iDpi);
}

inline HWND GetThreadFirstWindow(DWORD dwTid)
{
	HWND hWnd = GetActiveWindow();
	if (!hWnd)
		return hWnd;
	EnumThreadWindows(GetCurrentThreadId(), [](HWND hWnd, LPARAM lParam)->BOOL
		{
			if (IsWindowVisible(hWnd))
			{
				*(HWND*)lParam = hWnd;
				return TRUE;
			}
			else
				return FALSE;
		}, (LPARAM)&hWnd);
	return hWnd;
}

inline HWND GetSafeOwner(HWND hParent, HWND* phWndTop)
{
	HWND hWnd = hParent;
	if (hWnd == NULL)
		hWnd = GetThreadFirstWindow(GetCurrentThreadId());

	while (hWnd != NULL && (GetWindowLongPtrW(hWnd, GWL_STYLE) & WS_CHILD))
		hWnd = GetParent(hWnd);

	HWND hWndTop = hWnd, hWndTemp = hWnd;
	for (;;)
	{
		if (hWndTemp == NULL)
			break;
		else
			hWndTop = hWndTemp;
		hWndTemp = GetParent(hWndTop);
	}

	if (hParent == NULL && hWnd != NULL)
		hWnd = GetLastActivePopup(hWnd);

	if (phWndTop != NULL)
	{
		if (hWndTop != NULL && IsWindowEnabled(hWndTop) && hWndTop != hWnd)
		{
			*phWndTop = hWndTop;
			EnableWindow(hWndTop, FALSE);
		}
		else
			*phWndTop = NULL;
	}

	return hWnd;
}

EckInline WNDPROC GetClassWndProc(HINSTANCE hInstance, PCWSTR pszClass)
{
	WNDCLASSEXW wcex{ sizeof(wcex) };
	GetClassInfoExW(hInstance, pszClass, &wcex);
	return wcex.lpfnWndProc;
}

inline ATOM EzRegisterWndClass(PCWSTR pszClass, UINT uStyle = CS_STDWND, HBRUSH hbrBK = NULL)
{
	WNDCLASSW wc{};
	wc.cbWndExtra = sizeof(void*);
	wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
	wc.hInstance = g_hInstance;
	wc.lpfnWndProc = DefWindowProcW;
	wc.lpszClassName = pszClass;
	wc.style = uStyle;
	wc.hbrBackground = hbrBK;
	return RegisterClassW(&wc);
}

EckInline HMONITOR GetPrimaryMonitor()
{
	HMONITOR hMonitor = NULL;
	EnumDisplayMonitors(NULL, NULL, [](HMONITOR hMonitor, HDC, RECT*, LPARAM lParam)->BOOL
		{
			MONITORINFO mi;
			mi.cbSize = sizeof(mi);
			GetMonitorInfoW(hMonitor, &mi);
			if (IsBitSet(mi.dwFlags, MONITORINFOF_PRIMARY))
			{
				*(HMONITOR*)lParam = hMonitor;
				return FALSE;
			}
			else
				return TRUE;
		}, (LPARAM)&hMonitor);
	return hMonitor;
}

inline HMONITOR GetOwnerMonitor(HWND hWnd)
{
	if (hWnd)
		return MonitorFromWindow(hWnd, MONITOR_DEFAULTTOPRIMARY);
	else
	{
		const auto hForeGnd = GetForegroundWindow();
		DWORD dwPID;
		GetWindowThreadProcessId(hForeGnd, &dwPID);
		if (dwPID == GetCurrentProcessId())// 如果前台窗口是自进程窗口，返回这个窗口所在的显示器
			return MonitorFromWindow(hForeGnd, MONITOR_DEFAULTTOPRIMARY);
		else// 否则返回主显示器
			return GetPrimaryMonitor();
	}
}

inline POINT CalcCenterWndPos(HWND hParent, int cx, int cy)
{
	if (hParent)
	{
		RECT rc;
		GetWindowRect(hParent, &rc);
		return
		{
			rc.left + (rc.right - rc.left - cx) / 2,
			rc.top + (rc.bottom - rc.top - cy) / 2
		};
	}
	else
	{
		const auto hMonitor = GetOwnerMonitor(NULL);
		MONITORINFO mi;
		mi.cbSize = sizeof(mi);
		GetMonitorInfoW(hMonitor, &mi);
		return
		{
			mi.rcWork.left + (mi.rcWork.right - mi.rcWork.left - cx) / 2,
			mi.rcWork.top + (mi.rcWork.bottom - mi.rcWork.top - cy) / 2
		};
	}
}

/// <summary>
/// 鼠标是否移动出拖放距离阈值。
/// 给定起始点，判断自调用函数的瞬间起鼠标移动距离是否超过(dx, dy)
/// </summary>
/// <param name="hWnd">窗口句柄</param>
/// <param name="x">起始X，相对屏幕</param>
/// <param name="y">起始Y，相对屏幕</param>
/// <param name="dx">x方向的阈值，若为负，则使用GetSystemMetrics(SM_CXDRAG)</param>
/// <param name="dy">y方向的阈值，若为负，则使用GetSystemMetrics(SM_CYDRAG)</param>
/// <returns>移动距离超出阈值返回TRUE，此时调用方可执行拖放操作；否则返回FALSE</returns>
inline BOOL IsMouseMovedBeforeDragging(HWND hWnd, int x, int y, int dx = -1, int dy = -1)
{
	const int dxDrag = (dx < 0 ? GetSystemMetrics(SM_CXDRAG) : dx);
	const int dyDrag = (dy < 0 ? GetSystemMetrics(SM_CYDRAG) : dy);
	SetCapture(hWnd);
	MSG msg;
	while (GetCapture() == hWnd)
	{
		if (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
		{
			switch (msg.message)
			{
			case WM_LBUTTONUP:
			case WM_LBUTTONDOWN:
			case WM_RBUTTONUP:
			case WM_RBUTTONDOWN:
			case WM_MBUTTONUP:
			case WM_MBUTTONDOWN:
				ReleaseCapture();
				TranslateMessage(&msg);
				DispatchMessageW(&msg);
				return FALSE;
			case WM_MOUSEMOVE:
				if (Abs(x - msg.pt.x) >= dxDrag && Abs(y - msg.pt.y) >= dyDrag)
				{
					ReleaseCapture();
					return TRUE;
				}
				[[fallthrough]];
			default:
				TranslateMessage(&msg);
				DispatchMessageW(&msg);
				break;
			}
		}
		else
			WaitMessage();
	}
	ReleaseCapture();
	return FALSE;
}

EckInline void BroadcastChildrenMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	const MSG msg{ NULL,uMsg,wParam,lParam };
	EnumChildWindows(hWnd, [](HWND hWnd, LPARAM lParam)->BOOL
		{
			const auto pMsg = (const MSG*)lParam;
			SendMessageW(hWnd, pMsg->message, pMsg->wParam, pMsg->lParam);
			return TRUE;
		}, (LPARAM)&msg);
}

EckInline BOOL AllowDarkModeForWindow(HWND hWnd, BOOL bAllow)
{
#ifdef ECK_MACRO_DRAKMODE
	return EckPriv___::pfnAllowDarkModeForWindow(hWnd, bAllow);
#else
	if (EckPriv___::pfnAllowDarkModeForWindow)
		return EckPriv___::pfnAllowDarkModeForWindow(hWnd, bAllow);
	return FALSE;
#endif
}

EckInline PreferredAppMode SetPreferredAppMode(PreferredAppMode iMode)
{
	if (EckPriv___::pfnSetPreferredAppMode)
		return EckPriv___::pfnSetPreferredAppMode(iMode);
	else if (EckPriv___::pfnAllowDarkModeForApp)
	{
		EckPriv___::pfnAllowDarkModeForApp(
			iMode == PreferredAppMode::AllowDark || iMode == PreferredAppMode::ForceDark);
		return PreferredAppMode::Default;
	}
	return PreferredAppMode::Default;
}

EckInline BOOL IsDarkModeAllowedForWindow(HWND hWnd)
{
#ifdef ECK_MACRO_DRAKMODE
	return EckPriv___::pfnIsDarkModeAllowedForWindow(hWnd);
#else
	if (EckPriv___::pfnIsDarkModeAllowedForWindow)
		return EckPriv___::pfnIsDarkModeAllowedForWindow(hWnd);
	return FALSE;
#endif
}

EckInline BOOL ShouldAppUseDarkMode()
{
#ifdef ECK_MACRO_DRAKMODE
	return EckPriv___::pfnShouldAppsUseDarkMode();
#else
	if (EckPriv___::pfnShouldAppsUseDarkMode)
		return EckPriv___::pfnShouldAppsUseDarkMode();
	return FALSE;
#endif
}

EckInline void FlushMenuTheme()
{
#ifdef ECK_MACRO_DRAKMODE
	EckPriv___::pfnFlushMenuThemes();
#else
	if (EckPriv___::pfnFlushMenuThemes)
		EckPriv___::pfnFlushMenuThemes();
#endif
}

EckInline void RefreshImmersiveColorPolicyState()
{
#ifdef ECK_MACRO_DRAKMODE
	EckPriv___::pfnRefreshImmersiveColorPolicyState();
#else
	if (EckPriv___::pfnRefreshImmersiveColorPolicyState)
		EckPriv___::pfnRefreshImmersiveColorPolicyState();
#endif
}

EckInline BOOL GetIsImmersiveColorUsingHighContrast(IMMERSIVE_HC_CACHE_MODE iCacheMode)
{
#ifdef ECK_MACRO_DRAKMODE
	return EckPriv___::pfnGetIsImmersiveColorUsingHighContrast(iCacheMode);
#else
	if (EckPriv___::pfnGetIsImmersiveColorUsingHighContrast)
		return EckPriv___::pfnGetIsImmersiveColorUsingHighContrast(iCacheMode);
	return FALSE;
#endif
}

EckInline BOOL ShouldSystemUseDarkMode()
{
#ifdef ECK_MACRO_DRAKMODE
	return EckPriv___::pfnShouldSystemUseDarkMode();
#else
	if (EckPriv___::pfnShouldSystemUseDarkMode)
		return EckPriv___::pfnShouldSystemUseDarkMode();
	return FALSE;
#endif
}

EckInline BOOL IsDarkModeAllowedForApp()
{
#ifdef ECK_MACRO_DRAKMODE
	return EckPriv___::pfnIsDarkModeAllowedForApp();
#else
	if (EckPriv___::pfnIsDarkModeAllowedForApp)
		return EckPriv___::pfnIsDarkModeAllowedForApp();
	return FALSE;
#endif
}

EckInline HRESULT EnableWindowNcDarkMode(HWND hWnd, BOOL bAllow)
{
	if (g_bWin11_B22000)
		return DwmSetWindowAttribute(hWnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &bAllow, sizeof(bAllow));
	else
	{
		WINDOWCOMPOSITIONATTRIBDATA t{ WCA_USEDARKMODECOLORS, &bAllow, sizeof(bAllow) };
		return EckPriv___::pfnSetWindowCompositionAttribute(hWnd, &t) ? S_OK : E_FAIL;
	}
}

EckInline BOOL IsColorSchemeChangeMessage(LPARAM lParam)
{
	return CompareStringOrdinal((PCWCH)lParam, -1, L"ImmersiveColorSet", -1, TRUE) == CSTR_EQUAL;
}

EckInline void RefreshImmersiveColorStuff()
{
	RefreshImmersiveColorPolicyState();
	GetIsImmersiveColorUsingHighContrast(IHCM_REFRESH);
}

/// <summary>
/// 取ItemsView前景背景色
/// </summary>
/// <param name="crText"></param>
/// <param name="crBk"></param>
inline void GetItemsViewForeBackColor(COLORREF& crText, COLORREF& crBk)
{
	crBk = GetSysColor(COLOR_WINDOW);
	crText = GetSysColor(COLOR_WINDOWTEXT);
	if (ShouldAppUseDarkMode())
	{
		const auto hThemeIV = OpenThemeData(NULL, L"ItemsView");
		if (hThemeIV)
		{
			if (FAILED(GetThemeColor(hThemeIV, 0, 0, TMT_FILLCOLOR, &crBk)))
				crBk = GetSysColor(COLOR_WINDOW);

			if (FAILED(GetThemeColor(hThemeIV, 0, 0, TMT_TEXTCOLOR, &crText)))
				crText = GetSysColor(COLOR_WINDOWTEXT);

			CloseThemeData(hThemeIV);
		}
	}
}

EckInline BOOL AdjustWindowRectExDpi(RECT* prc, DWORD dwStyle, BOOL bMenu, DWORD dwExStyle, UINT uDpi)
{
#if ECKDPIAPI
	return AdjustWindowRectExForDpi(prc, dwStyle, bMenu, dwExStyle, uDpi);
#else
	return AdjustWindowRectEx(prc, dwStyle, bMenu, dwExStyle);
#endif
}

/// <summary>
/// 取窗口客户区尺寸。
/// 在最小化时也有效
/// </summary>
/// <param name="hWnd">窗口句柄</param>
/// <param name="rcClient">客户区尺寸，其中left和top总为0</param>
/// <returns>成功返回TRUE，失败返回FALSE</returns>
inline BOOL GetWindowClientRect(HWND hWnd, RECT& rcClient)
{
	const int iDpi = eck::GetDpi(hWnd);
	RECT rcMainClient;
	RECT rcNcOnly{};
	WINDOWPLACEMENT wp;
	wp.length = sizeof(wp);
	if (AdjustWindowRectExDpi(&rcNcOnly, GetWindowStyle(hWnd),
		!!GetMenu(hWnd), GetWindowExStyle(hWnd), iDpi) &&
		GetWindowPlacement(hWnd, &wp))
	{
		if (wp.flags & WPF_RESTORETOMAXIMIZED)
		{
			if (HMONITOR hMonitor;
				hMonitor = MonitorFromRect(&wp.rcNormalPosition, MONITOR_DEFAULTTONULL))
			{
				MONITORINFO mi;
				mi.cbSize = sizeof(mi);
				GetMonitorInfoW(hMonitor, &mi);
				rcMainClient = mi.rcMonitor;
			}
			else
				return FALSE;
		}
		else
			rcMainClient = wp.rcNormalPosition;
		// 对齐到(0, 0)
		OffsetRect(rcMainClient, -rcMainClient.left, -rcMainClient.top);
		// 减掉非客户区
		rcMainClient.right -= (rcNcOnly.right - rcNcOnly.left);
		rcMainClient.bottom -= (rcNcOnly.bottom - rcNcOnly.top);
		rcClient = rcMainClient;
		return TRUE;
	}
	else
		return FALSE;
}
ECK_NAMESPACE_END