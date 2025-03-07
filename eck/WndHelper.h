#pragma once
#include "Utility.h"
#include "DpiApi.h"

#include <vssym32.h>
#include <ShellScalingApi.h>

#define ECK_DS_BEGIN(StructName)	struct StructName {
#define ECK_DS_END_VAR(VarName)		} VarName{};
#define ECK_DS_END()				};
#define ECK_DS_ENTRY(Name, Size)		const int o_##Name = Size; int Name = Size;
#define ECK_DS_ENTRY_DYN(Name, Size)	int	o_##Name = Size; int Name = Size;
#define ECK_DS_ENTRY_F(Name, Size)		const float o_##Name = Size; float Name = Size;
#define ECK_DS_ENTRY_F_DYN(Name, Size)	float o_##Name = Size; float Name = Size;

#define ECK_HANDLE_WM_MOUSELEAVE(hWnd, wParam, lParam, fn) \
	((fn)((hWnd)), 0L)
#define ECK_HANDLE_WM_DPICHANGED(hWnd, wParam, lParam, fn) \
	((fn)((hWnd), (int)(short)LOWORD(wParam), (int)(short)HIWORD(wParam), (RECT*)(lParam)), 0L)

#define ECK_STYLE_GETSET(Name, Style)					\
	BOOL StyleGet##Name() const							\
	{													\
		if constexpr (Style == 0)						\
			return !GetStyle();							\
		else											\
			return IsBitSet(GetStyle(), Style);			\
	}													\
	void StyleSet##Name(BOOL b)	const					\
	{													\
		ModifyStyle((b ? Style : 0), Style, GWL_STYLE); \
	}

#define ECK_STYLE_GETSET_MASK(Name, Style, Mask)		\
	BOOL StyleGet##Name() const							\
	{													\
		if constexpr (Style == 0)						\
			return !(GetStyle() & Mask);				\
		else											\
			return IsBitSet(GetStyle(), Style);			\
	}													\
	void StyleSet##Name(BOOL b)	const					\
	{													\
		SetStyle((GetStyle() & ~Mask) | (b ? Style : 0));\
	}

#define ECK_CWNDPROP_STYLE(Name, Style)					\
	ECKPROP(StyleGet##Name, StyleSet##Name) BOOL Name;	\
	ECK_STYLE_GETSET(Name, Style)

#define ECK_CWNDPROP_STYLE_MASK(Name, Style, Mask)		\
	ECKPROP(StyleGet##Name, StyleSet##Name) BOOL Name;	\
	ECK_STYLE_GETSET_MASK(Name, Style, Mask)

ECK_NAMESPACE_BEGIN
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

EckInline int GetMonitorDpi(HMONITOR hMonitor)
{
#if NTDDI_VERSION >= NTDDI_WINBLUE
	UINT xDpi, yDpi;
	if (SUCCEEDED(GetDpiForMonitor(hMonitor, MDT_EFFECTIVE_DPI, &xDpi, &yDpi)))
		return (int)xDpi;
	else
		return USER_DEFAULT_SCREEN_DPI;
#else
	const auto hIC = CreateICW(L"DISPLAY", nullptr, nullptr, nullptr);
	const int i = GetDeviceCaps(hIC, LOGPIXELSX);
	DeleteDC(hIC);
	return i;
#endif
}

EckInline constexpr int DpiScale(int i, int iDpiNew, int iDpiOld)
{
	return i * iDpiNew / iDpiOld;
}

EckInline constexpr int DpiScale(int i, int iDpi)
{
	return DpiScale(i, iDpi, USER_DEFAULT_SCREEN_DPI);
}

EckInline constexpr float DpiScaleF(float i, int iDpiNew, int iDpiOld)
{
	return i * iDpiNew / iDpiOld;
}

EckInline constexpr float DpiScaleF(float i, int iDpi)
{
	return DpiScaleF(i, iDpi, USER_DEFAULT_SCREEN_DPI);
}

EckInline constexpr void DpiScale(_Inout_ RECT& rc, int iDpiNew, int iDpiOld)
{
	rc.left = rc.left * iDpiNew / iDpiOld;
	rc.top = rc.top * iDpiNew / iDpiOld;
	rc.right = rc.right * iDpiNew / iDpiOld;
	rc.bottom = rc.bottom * iDpiNew / iDpiOld;
}

EckInline constexpr void DpiScale(_Inout_ RECT& rc, int iDpi)
{
	DpiScale(rc, iDpi, USER_DEFAULT_SCREEN_DPI);
}

EckInline constexpr void DpiScale(_Inout_ SIZE& size, int iDpiNew, int iDpiOld)
{
	size.cx = size.cx * iDpiNew / iDpiOld;
	size.cy = size.cy * iDpiNew / iDpiOld;
}

EckInline constexpr void DpiScale(_Inout_ SIZE& size, int iDpi)
{
	DpiScale(size, iDpi, USER_DEFAULT_SCREEN_DPI);
}

EckInline constexpr void DpiScale(_Inout_ POINT& pt, int iDpiNew, int iDpiOld)
{
	pt.x = pt.x * iDpiNew / iDpiOld;
	pt.y = pt.y * iDpiNew / iDpiOld;
}

EckInline constexpr void DpiScale(_Inout_ POINT& pt, int iDpi)
{
	DpiScale(pt, iDpi, USER_DEFAULT_SCREEN_DPI);
}

EckInline constexpr void DpiScale(_Inout_ D2D1_RECT_F& rc, int iDpiNew, int iDpiOld)
{
	rc.left = DpiScaleF(rc.left, iDpiNew, iDpiOld);
	rc.top = DpiScaleF(rc.top, iDpiNew, iDpiOld);
	rc.right = DpiScaleF(rc.right, iDpiNew, iDpiOld);
	rc.bottom = DpiScaleF(rc.bottom, iDpiNew, iDpiOld);
}

EckInline constexpr void DpiScale(_Inout_ D2D1_RECT_F& rc, int iDpi)
{
	DpiScale(rc, iDpi, USER_DEFAULT_SCREEN_DPI);
}

EckInline constexpr void DpiScale(_Inout_ D2D1_RECT_U& rc, int iDpiNew, int iDpiOld)
{
	rc.left = (UINT)DpiScale(rc.left, iDpiNew, iDpiOld);
	rc.top = (UINT)DpiScale(rc.top, iDpiNew, iDpiOld);
	rc.right = (UINT)DpiScale(rc.right, iDpiNew, iDpiOld);
	rc.bottom = (UINT)DpiScale(rc.bottom, iDpiNew, iDpiOld);
}

EckInline constexpr void DpiScale(_Inout_ D2D1_RECT_U& rc, int iDpi)
{
	DpiScale(rc, iDpi, USER_DEFAULT_SCREEN_DPI);
}

EckInline constexpr void DpiScale(_Inout_ D2D1_SIZE_F& size, int iDpiNew, int iDpiOld)
{
	size.width = DpiScaleF(size.width, iDpiNew, iDpiOld);
	size.height = DpiScaleF(size.height, iDpiNew, iDpiOld);
}

EckInline constexpr void DpiScale(_Inout_ D2D1_SIZE_F& size, int iDpi)
{
	DpiScale(size, iDpi, USER_DEFAULT_SCREEN_DPI);
}

EckInline constexpr void DpiScale(_Inout_ D2D1_POINT_2F& pt, int iDpiNew, int iDpiOld)
{
	pt.x = DpiScaleF(pt.x, iDpiNew, iDpiOld);
	pt.y = DpiScaleF(pt.y, iDpiNew, iDpiOld);
}

EckInline constexpr void DpiScale(_Inout_ D2D1_POINT_2F& pt, int iDpi)
{
	DpiScale(pt, iDpi, USER_DEFAULT_SCREEN_DPI);
}

EckInline constexpr void DpiScale(_Inout_ D2D1_POINT_2U& pt, int iDpiNew, int iDpiOld)
{
	pt.x = (UINT)DpiScale(pt.x, iDpiNew, iDpiOld);
	pt.y = (UINT)DpiScale(pt.y, iDpiNew, iDpiOld);
}

EckInline constexpr void DpiScale(_Inout_ D2D1_POINT_2U& pt, int iDpi)
{
	DpiScale(pt, iDpi, USER_DEFAULT_SCREEN_DPI);
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
	BOOL bUnderline = FALSE, BOOL bStrikeOut = FALSE, HWND hWnd = nullptr, DWORD dwCharSet = DEFAULT_CHARSET)
{
	const HDC hDC = GetDC(hWnd);
	const int iSize = -MulDiv(iPoint, GetDeviceCaps(hDC, LOGPIXELSY), 72);
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
		RedrawWindow(hWnd, nullptr, nullptr, RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_UPDATENOW);
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
	const HFONT hFontNew = ReCreateFontForDpiChanged(
		(HFONT)SendMessageW(hWnd, WM_GETFONT, 0, 0),
		iDpiNew, iDpiOld, bDeletePrevFont);
	SendMessageW(hWnd, WM_SETFONT, (WPARAM)hFontNew, bRedraw);
	return hFontNew;
}

inline BOOL GetWindowClientRect(HWND hWnd, HWND hParent, RECT& rc)
{
	if (!GetWindowRect(hWnd, &rc))
		return FALSE;
	const int cx = rc.right - rc.left,
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
	return (HIMAGELIST)SendMessageW(hLV, LVM_SETIMAGELIST, LVSIL_SMALL, (LPARAM)ImageList_Create(1, (cy), 0, 1, 0));
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

EckInline BOOL BitBltPs(const PAINTSTRUCT& pps, HDC hdcSrc)
{
	return BitBltPs(&pps, hdcSrc);
}

EckInline WNDPROC SetWindowProc(HWND hWnd, WNDPROC pfnWndProc)
{
	return (WNDPROC)SetWindowLongPtrW(hWnd, GWLP_WNDPROC, (LONG_PTR)pfnWndProc);
}

#if NTDDI_VERSION < NTDDI_WIN10_NI// 22621 SDK将NTDDI_VERSION设置为NTDDI_WIN10_NI
EckInline HRESULT EnableWindowMica(HWND hWnd, DWORD uType = 2)
{
	return E_FAIL;
}
#else
EckInline HRESULT EnableWindowMica(HWND hWnd, DWM_SYSTEMBACKDROP_TYPE uType = DWMSBT_MAINWINDOW)
{
	return DwmSetWindowAttribute(hWnd, DWMWA_SYSTEMBACKDROP_TYPE, &uType, sizeof(uType));
}
#endif// NTDDI_VERSION < NTDDI_WIN10_NI

inline constexpr LRESULT MsgOnNcCalcSize(WPARAM wParam, LPARAM lParam, const MARGINS& Margins)
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
inline constexpr LRESULT MsgOnNcHitTest(POINT pt, const MARGINS& Margins, int cxWnd, int cyWnd)
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
EckInline constexpr void UpdateDpiSize(T& Dpis, int iDpi)
{
	for (int* p = ((int*)&Dpis) + 1; p < PtrSkipType(&Dpis); p += 2)
		*p = DpiScale(*(p - 1), iDpi);
}

template<class T>
EckInline constexpr void UpdateDpiSizeF(T& Dpis, int iDpi)
{
	for (float* p = ((float*)&Dpis) + 1; p < PtrSkipType(&Dpis); p += 2)
		*p = DpiScaleF(*(p - 1), iDpi);
}

inline HWND GetThreadFirstWindow(DWORD dwTid)
{
	HWND hWnd = GetActiveWindow();
	if (!hWnd)
		return hWnd;
	EnumThreadWindows(dwTid, [](HWND hWnd, LPARAM lParam)->BOOL
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
	if (hWnd == nullptr)
		hWnd = GetThreadFirstWindow(NtCurrentThreadId32());

	while (hWnd != nullptr && (GetWindowLongPtrW(hWnd, GWL_STYLE) & WS_CHILD))
		hWnd = GetParent(hWnd);

	HWND hWndTop = hWnd, hWndTemp = hWnd;
	EckLoop()
	{
		if (hWndTemp == nullptr)
			break;
		else
			hWndTop = hWndTemp;
		hWndTemp = GetParent(hWndTop);
	}

	if (hParent == nullptr && hWnd != nullptr)
		hWnd = GetLastActivePopup(hWnd);

	if (phWndTop != nullptr)
	{
		if (hWndTop != nullptr && IsWindowEnabled(hWndTop) && hWndTop != hWnd)
		{
			*phWndTop = hWndTop;
			EnableWindow(hWndTop, FALSE);
		}
		else
			*phWndTop = nullptr;
	}

	return hWnd;
}

EckInline WNDPROC GetClassWndProc(HINSTANCE hInstance, PCWSTR pszClass)
{
	WNDCLASSEXW wcex{ sizeof(wcex) };
	GetClassInfoExW(hInstance, pszClass, &wcex);
	return wcex.lpfnWndProc;
}

inline ATOM EzRegisterWndClass(PCWSTR pszClass, UINT uStyle = CS_STDWND, HBRUSH hbrBK = nullptr)
{
	WNDCLASSW wc{};
	wc.cbWndExtra = sizeof(void*);
	wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
	wc.hInstance = g_hInstance;
	wc.lpfnWndProc = DefWindowProcW;
	wc.lpszClassName = pszClass;
	wc.style = uStyle;
	wc.hbrBackground = hbrBK;
	return RegisterClassW(&wc);
}

[[nodiscard]] EckInline HMONITOR MonitorFromRectByWorkArea(const RECT& rc,
	HMONITOR* phMonMain = nullptr, HMONITOR* phMonNearest = nullptr)
{
	struct CTX
	{
		const RECT& rc;
		int iMinDistance;
		int iMaxArea;
		HMONITOR hMon;
		HMONITOR hMonMain;
		HMONITOR hMonNearest;
	}
	Ctx{ rc,INT_MAX };

	EnumDisplayMonitors(nullptr, nullptr, [](HMONITOR hMonitor, HDC, RECT*, LPARAM lParam)->BOOL
		{
			const auto pCtx = (CTX*)lParam;

			MONITORINFO mi;
			mi.cbSize = sizeof(mi);
			GetMonitorInfoW(hMonitor, &mi);
			if (IsBitSet(mi.dwFlags, MONITORINFOF_PRIMARY))
				pCtx->hMonMain = hMonitor;

			RECT rc;
			if (IntersectRect(rc, mi.rcWork, pCtx->rc))
			{
				const int iArea = (rc.right - rc.left) * (rc.bottom - rc.top);
				if (iArea > pCtx->iMaxArea)
				{
					pCtx->iMaxArea = iArea;
					pCtx->hMon = hMonitor;
				}
			}

			const int dx = (pCtx->rc.left + pCtx->rc.right) / 2 -
				(mi.rcWork.left + mi.rcWork.right) / 2;
			const int dy = (pCtx->rc.top + pCtx->rc.bottom) / 2 -
				(mi.rcWork.top + mi.rcWork.bottom) / 2;
			const int d = dx * dx + dy * dy;
			if (d < pCtx->iMinDistance)
			{
				pCtx->iMinDistance = d;
				pCtx->hMonNearest = hMonitor;
			}
			return TRUE;
		}, (LPARAM)&Ctx);

	if (phMonMain)
		*phMonMain = Ctx.hMonMain;
	if (phMonNearest)
		*phMonNearest = Ctx.hMonNearest;
	return Ctx.hMon;
}

EckInline HMONITOR GetPrimaryMonitor()
{
	HMONITOR hMonitor{};
	EnumDisplayMonitors(nullptr, nullptr, [](HMONITOR hMonitor, HDC, RECT*, LPARAM lParam)->BOOL
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
		const auto hForegnd = GetForegroundWindow();
		DWORD dwPID;
		GetWindowThreadProcessId(hForegnd, &dwPID);
		if (dwPID == NtCurrentProcessId32())// 如果前台窗口是自进程窗口，返回这个窗口所在的显示器
			return MonitorFromWindow(hForegnd, MONITOR_DEFAULTTOPRIMARY);
		else// 否则返回主显示器
			return GetPrimaryMonitor();
	}
}

/// <summary>
/// 计算窗口居中坐标
/// </summary>
/// <param name="hParent">父窗口，若为空，则使用最佳显示器</param>
/// <param name="cx">子窗口宽度</param>
/// <param name="cy">子窗口高度</param>
/// <param name="bEnsureInMonitor">是否确保在显示器内</param>
/// <returns>子窗口左上角坐标，相对屏幕</returns>
inline POINT CalcCenterWndPos(HWND hParent, int cx, int cy,
	BOOL bEnsureInMonitor = TRUE)
{
	if (hParent)
	{
		RECT rc;
		GetWindowRect(hParent, &rc);
		rc.left += (rc.right - rc.left - cx) / 2;
		rc.top += (rc.bottom - rc.top - cy) / 2;
		if (bEnsureInMonitor)
		{
			rc.right = rc.left + cx;
			rc.bottom = rc.top + cy;
			const auto hMonitor = GetOwnerMonitor(hParent);
			MONITORINFO mi;
			mi.cbSize = sizeof(mi);
			GetMonitorInfoW(hMonitor, &mi);
			AdjustRectIntoAnother(rc, mi.rcWork);
			return { rc.left, rc.top };
		}
		else
			return { rc.left, rc.top };
	}
	else
	{
		const auto hMonitor = GetOwnerMonitor(nullptr);
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
/// 给定起始点，判断自调用函数的瞬间起鼠标移动距离是否超过(dx, dy)。
/// 警告：1. 此函数启动拖放消息循环，返回后窗口可能已被销毁；
///      2. 函数使用SetCapture和ReleaseCapture。
/// </summary>
/// <param name="hWnd">窗口句柄</param>
/// <param name="x">起始X，相对屏幕</param>
/// <param name="y">起始Y，相对屏幕</param>
/// <param name="dx">x方向的阈值，若为负，则使用GetSystemMetrics(SM_CXDRAG)</param>
/// <param name="dy">y方向的阈值，若为负，则使用GetSystemMetrics(SM_CYDRAG)</param>
/// <param name="iDpi">DPI，若为负，则自动获取</param>
/// <returns>移动距离超出阈值返回TRUE，此时调用方可执行拖放操作；否则返回FALSE</returns>
inline BOOL IsMouseMovedBeforeDragging(HWND hWnd, int x, int y,
	int dx = -1, int dy = -1, int iDpi = -1)
{
	if ((dx < 0 || dy < 0) && iDpi < 0)
		iDpi = GetDpi(hWnd);
	if (dx < 0)
		dx = DaGetSystemMetrics(SM_CXDRAG, iDpi);
	if (dy < 0)
		dy = DaGetSystemMetrics(SM_CYDRAG, iDpi);
	SetCapture(hWnd);
	MSG msg;
	while (GetCapture() == hWnd)
	{
		if (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
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
				if (Abs(x - msg.pt.x) >= dx && Abs(y - msg.pt.y) >= dx)
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
	const MSG msg{ nullptr,uMsg,wParam,lParam };
	EnumChildWindows(hWnd, [](HWND hWnd, LPARAM lParam)->BOOL
		{
			const auto pMsg = (const MSG*)lParam;
			SendMessageW(hWnd, pMsg->message, pMsg->wParam, pMsg->lParam);
			return TRUE;
		}, (LPARAM)&msg);
}

EckInline HRESULT EnableWindowNcDarkMode(HWND hWnd, BOOL bAllow)
{
	if (g_NtVer.uBuild > WINVER_11_21H2)
		return DwmSetWindowAttribute(hWnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &bAllow, sizeof(bAllow));
	else
	{
		WINDOWCOMPOSITIONATTRIBDATA t{ WCA_USEDARKMODECOLORS, &bAllow, sizeof(bAllow) };
		return SetWindowCompositionAttribute(hWnd, &t) ? S_OK : E_FAIL;
	}
}

EckInline BOOL IsColorSchemeChangeMessage(LPARAM lParam)
{
	return CompareStringOrdinal((PCWCH)lParam, -1,
		EckStrAndLen(L"ImmersiveColorSet"), TRUE) == CSTR_EQUAL;
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
/// <returns>当前是否为暗色</returns>
inline BOOL GetItemsViewForeBackColor(COLORREF& crText, COLORREF& crBk)
{
	crBk = GetSysColor(COLOR_WINDOW);
	crText = GetSysColor(COLOR_WINDOWTEXT);
#if !ECK_OPT_NO_DARKMODE
	if (ShouldAppsUseDarkMode())
	{
		const auto hThemeIV = OpenThemeData(nullptr, L"ItemsView");
		if (hThemeIV)
		{
			if (FAILED(GetThemeColor(hThemeIV, 0, 0, TMT_FILLCOLOR, &crBk)))
				crBk = GetSysColor(COLOR_WINDOW);

			if (FAILED(GetThemeColor(hThemeIV, 0, 0, TMT_TEXTCOLOR, &crText)))
				crText = GetSysColor(COLOR_WINDOWTEXT);

			CloseThemeData(hThemeIV);
		}
		return TRUE;
	}
#endif// !ECK_OPT_NO_DARKMODE
	return FALSE;
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
	const int iDpi = GetDpi(hWnd);
	RECT rcMainClient;
	RECT rcNcOnly{};
	WINDOWPLACEMENT wp;
	wp.length = sizeof(wp);
	if (DaAdjustWindowRectEx(&rcNcOnly, GetWindowStyle(hWnd),
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

EckInline void DoEvents()
{
	MSG msg;
	while (PeekMessageW(&msg, nullptr, 0u, 0u, PM_REMOVE))
	{
		if (msg.message == WM_QUIT)
		{
			PostQuitMessage((int)msg.wParam);
			return;
		}
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}
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

EckInline void MsgOnDpiChanged(HWND hWnd, LPARAM lParam)
{
	const auto* const prc = (RECT*)lParam;
	SetWindowPos(hWnd, nullptr,
		prc->left,
		prc->top,
		prc->right - prc->left,
		prc->bottom - prc->top,
		SWP_NOZORDER | SWP_NOACTIVATE);
}

inline LRESULT GenerateCharMsg(HWND hWnd, WCHAR ch, BOOL bExtended = FALSE)
{
	switch (ch)
	{
	case 58:
	case 59:
		ch = VK_OEM_1;
		break;
	case 43:
		ch = VK_OEM_PLUS;
		break;
	case 44:
		ch = VK_OEM_COMMA;
		break;
	case 45:
		ch = VK_OEM_MINUS;
		break;
	case 46:
		ch = VK_OEM_PERIOD;
		break;
	case 47:
	case 63:
		ch = VK_OEM_2;
		break;
	case 96:
	case 126:
		ch = VK_OEM_3;
		break;
	default:
		if (ch >= 97 && ch <= 122)
			ch -= 32;
		break;
	}
	const auto uScanCode = MapVirtualKeyW(ch, MAPVK_VK_TO_VSC);
	return PostMessageW(hWnd, WM_CHAR, ch, MakeKeyStrokeFlag(1, uScanCode, bExtended, FALSE, FALSE, FALSE));
}

inline void GenerateCharMsg(HWND hWnd, PCWSTR pszText, int cchText = -1,
	BOOL bExtended = FALSE, BOOL bReplaceEndOfLine = TRUE)
{
	if (cchText < 0)
		cchText = (int)wcslen(pszText);
	if (bReplaceEndOfLine)
		for (int i{}; i < cchText;)
		{
			auto ch = pszText[i++];
			if (ch == L'\r' && i < cchText && pszText[i] == L'\n')
			{
				ch = L'\n';
				++i;
			}
			GenerateCharMsg(hWnd, ch, bExtended);
		}
	else
	{
		EckCounter(cchText, i)
			GenerateCharMsg(hWnd, pszText[i], bExtended);
	}
}

inline HWND WndFromPoint(POINT pt, UINT uFlags = CWP_SKIPINVISIBLE)
{
	const HWND hParent = WindowFromPoint(pt);
	if (!hParent)
		return nullptr;
	POINT pt0{ pt };
	ScreenToClient(hParent, &pt0);

	HWND hChild0{ hParent }, hChild1;
	EckLoop()
	{
		pt0 = pt;
		ScreenToClient(hParent, &pt0);
		hChild1 = ChildWindowFromPointEx(hChild0, pt0, uFlags);
		if (!hChild1 || hChild1 == hChild0)
			return hChild0;
		hChild0 = hChild1;
	}
}

enum class KeyType
{
	Down,
	Up,
	Press,
};

inline void GenerateKeyMsg(HWND hWnd, UINT Vk, KeyType eType, BOOL bExtended = FALSE)
{
	switch (eType)
	{
	case KeyType::Down:
		PostMessageW(hWnd, bExtended ? WM_SYSKEYDOWN : WM_KEYDOWN, Vk,
			MakeKeyStrokeFlag(1, MapVirtualKeyW(Vk, MAPVK_VK_TO_VSC), bExtended, FALSE, FALSE, FALSE));
		break;
	case KeyType::Up:
		PostMessageW(hWnd, bExtended ? WM_SYSKEYUP : WM_KEYUP, Vk,
			MakeKeyStrokeFlag(1, MapVirtualKeyW(Vk, MAPVK_VK_TO_VSC), bExtended, FALSE, TRUE, TRUE));
		break;
	case KeyType::Press:
		GenerateKeyMsg(hWnd, Vk, KeyType::Down, bExtended);
		GenerateKeyMsg(hWnd, Vk, KeyType::Up, bExtended);
		break;
	default:
		ECK_UNREACHABLE;
	}
}

// 提供对亮暗切换的默认处理。
// 一般仅用于**主**顶级窗口，除了产生相关更新消息外还更新当前线程上下文的默认颜色
EckInline BOOL MsgOnSettingChangeMainWnd(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	if (IsColorSchemeChangeMessage(lParam))
	{
		BroadcastChildrenMessage(hWnd, WM_SETTINGCHANGE, wParam, lParam);
		const auto ptc = GetThreadCtx();
		ptc->UpdateDefColor();
		ptc->SetNcDarkModeForAllTopWnd(ShouldAppsUseDarkMode());
		ptc->SendThemeChangedToAllTopWindow();
		return TRUE;
	}
	else
		return FALSE;
}

// 提供对亮暗切换的默认处理。
// 一般仅用于顶级窗口
EckInline BOOL MsgOnSettingChange(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	if (IsColorSchemeChangeMessage(lParam))
	{
		BroadcastChildrenMessage(hWnd, WM_SETTINGCHANGE, wParam, lParam);
		BroadcastChildrenMessage(hWnd, WM_THEMECHANGED, 0, 0);
		EnableWindowNcDarkMode(hWnd, ShouldAppsUseDarkMode());
		return TRUE;
	}
	else
		return FALSE;
}

// 提供对WM_SYSCOLORCHANGE的默认处理。
// 一般仅用于**主**顶级窗口，除了向子窗口广播消息外还更新当前线程上下文的默认颜色
EckInline void MsgOnSysColorChangeMainWnd(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	GetThreadCtx()->UpdateDefColor();
	BroadcastChildrenMessage(hWnd, WM_SYSCOLORCHANGE, wParam, lParam);
}

// 提供对WM_SYSCOLORCHANGE的默认处理。一般仅用于顶级窗口
EckInline void MsgOnSysColorChange(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	BroadcastChildrenMessage(hWnd, WM_SYSCOLORCHANGE, wParam, lParam);
}

// WM_PAINT和WM_PRINTCLIENT的合并处理
EckInline HDC BeginPaint(HWND hWnd, WPARAM wParam, PAINTSTRUCT& ps)
{
	if (wParam)
	{
		ps = {};
		ps.hdc = (HDC)wParam;
		GetClipBox(ps.hdc, &ps.rcPaint);
		return ps.hdc;
	}
	else
		return BeginPaint(hWnd, &ps);
}

EckInline void EndPaint(HWND hWnd, WPARAM wParam, const PAINTSTRUCT& ps)
{
	if (!wParam)
		EndPaint(hWnd, &ps);
}

/// <summary>
/// 处理控件着色
/// </summary>
/// <param name="wParam"></param>
/// <param name="lParam"></param>
/// <param name="bHandled">是否已处理</param>
/// <param name="bColorDisableEdit">着色禁用的编辑框</param>
/// <param name="crBk">背景色</param>
/// <param name="crText">文本色</param>
/// <returns>LRESULT</returns>
inline LRESULT MsgOnCtrlColorXxx(WPARAM wParam, LPARAM lParam, BOOL& bHandled,
	BOOL bColorDisableEdit = TRUE, COLORREF crBk = CLR_DEFAULT, COLORREF crText = CLR_DEFAULT)
{
	switch (wParam)
	{
	case WM_CTLCOLORSTATIC:
		if (!bColorDisableEdit)
		{
			WCHAR szCls[ARRAYSIZE(WC_EDITW) + 1];
			if (GetClassNameW((HWND)lParam, szCls, ARRAYSIZE(szCls)) &&
				_wcsicmp(szCls, WC_EDITW) == 0)
				break;
		}
		[[fallthrough]];
	case WM_CTLCOLORBTN:
	case WM_CTLCOLORDLG:
	case WM_CTLCOLOREDIT:
	case WM_CTLCOLORLISTBOX:
	{
		bHandled = TRUE;
		const auto* const ptc = GetThreadCtx();
		SetTextColor((HDC)wParam, ptc->crDefText);
		SetBkColor((HDC)wParam, crBk != CLR_DEFAULT ? crBk : ptc->crDefBkg);
		SetDCBrushColor((HDC)wParam, crBk != CLR_DEFAULT ? crBk : ptc->crDefBkg);
		return (LRESULT)GetStockObject(DC_BRUSH);
	}
	break;
	}
	bHandled = FALSE;
	return 0;
}

inline SIZE GetCharDimension(HWND hWnd, HFONT hFont)
{
	TEXTMETRICW tm;
	HDC hDC = GetDC(hWnd);
	HFONT hFontOld = (HFONT)SelectObject(hDC, hFont);
	GetTextMetricsW(hDC, &tm);
	if (tm.tmPitchAndFamily & TMPF_FIXED_PITCH)
	{
		SIZE size;
		GetTextExtentPoint32W(hDC,
			L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz", 52, &size);
		size.cx = ((size.cx / 26) + 1) / 2;
		size.cy = tm.tmHeight;
		return size;
	}
	else
		return { tm.tmAveCharWidth, tm.tmHeight };
}

// 根据位置判断窗口是否应显示大小调整控件，该函数无视被判断窗口的样式
inline BOOL ShouldWindowDisplaySizeGrip(HWND hWnd, int iDpi = -1)
{
	const auto hDesktop = GetDesktopWindow();
	if (iDpi < 0)
		iDpi = GetDpi(hWnd);
	RECT rc, rcTmp;
	GetWindowRect(hWnd, &rc);
	rc.right += DaGetSystemMetrics(SM_CXFRAME, iDpi);
	rc.bottom += DaGetSystemMetrics(SM_CYFRAME, iDpi);
	while (hWnd && hWnd != hDesktop)
	{
		const auto dwStyle = GetWindowLongPtrW(hWnd, GWL_STYLE);
		if (IsBitSet(dwStyle, WS_SIZEBOX))// 向上找到第一个可调窗口
		{
			if (IsBitSet(dwStyle, WS_MAXIMIZE))
				return FALSE;// 最大化时不应显示
			GetWindowRect(hWnd, &rcTmp);
			if (rc.right < rcTmp.right || rc.bottom < rcTmp.bottom)
				return FALSE;// 没有贴近右下角时不应显示
			return TRUE;
		}
		else
			hWnd = GetParent(hWnd);
	}
	return FALSE;
}

[[nodiscard]] EckInline HFONT CreateDefFont(int iDpi = USER_DEFAULT_SCREEN_DPI)
{
	LOGFONTW lf;
	DaSystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(lf), &lf, 0, iDpi);
	return CreateFontIndirectW(&lf);
}

EckInline BOOL GetDefFontInfo(LOGFONTW& lf, int iDpi = USER_DEFAULT_SCREEN_DPI)
{
	return DaSystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(lf), &lf, 0, iDpi);
}

#if !ECK_OPT_NO_DX
[[nodiscard]] EckInline IDWriteTextFormat* CreateDefTextFormat(
	int iDpi = USER_DEFAULT_SCREEN_DPI, HRESULT* phr = nullptr)
{
	LOGFONTW lf;
	if (!GetDefFontInfo(lf, iDpi))
		return nullptr;
	IDWriteTextFormat* pTextFormat;
	const auto hr = g_pDwFactory->CreateTextFormat(
		lf.lfFaceName,
		nullptr,
		(DWRITE_FONT_WEIGHT)lf.lfWeight,
		lf.lfItalic ? DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		fabsf((float)lf.lfHeight),
		L"zh-cn",
		&pTextFormat);
	if (phr)
		*phr = hr;
	return pTextFormat;
}

[[nodiscard]] EckInline IDWriteTextFormat* CreateDefTextFormatWithSize(
	float cy, int iDpi = USER_DEFAULT_SCREEN_DPI, HRESULT* phr = nullptr)
{
	LOGFONTW lf;
	if (!GetDefFontInfo(lf, iDpi))
		return nullptr;
	IDWriteTextFormat* pTextFormat;
	const auto hr = g_pDwFactory->CreateTextFormat(
		lf.lfFaceName,
		nullptr,
		(DWRITE_FONT_WEIGHT)lf.lfWeight,
		lf.lfItalic ? DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		cy,
		L"zh-cn",
		&pTextFormat);
	if (phr)
		*phr = hr;
	return pTextFormat;
}
#endif// !ECK_OPT_NO_DX
ECK_NAMESPACE_END