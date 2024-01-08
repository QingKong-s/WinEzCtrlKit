/*
* WinEzCtrlKit Library
*
* WndHelper.h ： 窗口操作帮助函数
*
* Copyright(C) 2023 QingKong
*/
#pragma once
#include "ECK.h"
#include "Utility.h"


#define ECK_DS_BEGIN(StructName) struct StructName {
#define ECK_DS_END_VAR(VarName) } VarName{};
#define ECK_DS_END() };
#define ECK_DS_ENTRY(Name, Size) const int o_##Name = Size; int Name = Size;
#define ECK_DS_ENTRY_F(Name, Size) const float o_##Name = Size; float Name = Size;

#define ECK_HANDLE_WM_MOUSELEAVE(hWnd, wParam, lParam, fn) \
	((fn)((hWnd)), 0L)
#define ECK_HANDLE_WM_DPICHANGED(hWnd, wParam, lParam, fn) \
	((fn)((hWnd), (int)(short)LOWORD(wParam), (int)(short)HIWORD(wParam), (RECT*)(lParam)), 0L)

#define ECK_STYLE_GETSET(Name, Style) \
	BOOL StyleGet##Name() \
	{ \
		return IsBitSet(GetStyle(), Style); \
	} \
	void StyleSet##Name(BOOL b) \
	{ \
		ModifyStyle((b ? Style : 0), Style, GWL_STYLE); \
	}
#define ECK_CWNDPROP_STYLE(Name, Style) \
	ECKPROP(StyleGet##Name, StyleSet##Name) BOOL Name; \
	ECK_STYLE_GETSET(Name, Style)

ECK_NAMESPACE_BEGIN
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
#if _WIN32_WINNT >= _WIN32_WINNT_WIN10
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

inline LRESULT OnNcCalcSize(WPARAM wParam, LPARAM lParam, const MARGINS& Margins)
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

EckInline HMONITOR GetMainMonitor()
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
			return GetMainMonitor();
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
ECK_NAMESPACE_END