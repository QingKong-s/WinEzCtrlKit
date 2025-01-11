#pragma once
#include "CRefStr.h"
#include "SystemHelper.h"

ECK_NAMESPACE_BEGIN
enum
{
	WPSEV_GOOD_RECT,// 窗口至少部分可见
	WPSEV_ADJUSTED,	// 窗口不可见，但已成功调整为全部可见
	WPSEV_ERROR		// 窗口不可见，且处理过程中发生错误，当前坐标应被丢弃，转而使用程序定义的默认坐标
};

struct CWindowPosSetting
{
	constexpr static WCHAR c_szVer_0[8]{ L"eckwps0" };
	constexpr static int c_cchVer_0 = ARRAYSIZE(c_szVer_0) - 1;

	const int iVer = 0;
	int x{}, y{}, cx{}, cy{};
	BOOL bMaximized{};

	BOOL FromHWND(HWND hWnd)
	{
		WINDOWPLACEMENT wp;
		wp.length = sizeof(wp);
		if (!GetWindowPlacement(hWnd, &wp))
			return FALSE;
		x = wp.rcNormalPosition.left;
		y = wp.rcNormalPosition.top;
		cx = wp.rcNormalPosition.right - wp.rcNormalPosition.left;
		cy = wp.rcNormalPosition.bottom - wp.rcNormalPosition.top;
		bMaximized = !!(wp.flags & WPF_RESTORETOMAXIMIZED);
		return TRUE;
	}

	BOOL FromString(PCWSTR psz)
	{
		if (wcsncmp(psz, c_szVer_0, c_cchVer_0) != 0)
			return FALSE;
		return swscanf(psz + c_cchVer_0, L",%d,%d,%d,%d,%d",
			&x, &y, &cx, &cy, &bMaximized) == 5;
	}

	BOOL ToHWND(HWND hWnd)
	{
		if (bMaximized)
			return ShowWindow(hWnd, SW_SHOWMAXIMIZED);
		else
			return SetWindowPos(hWnd, nullptr, x, y, cx, cy, SWP_NOZORDER);
	}

	CRefStrW ToString()
	{
		return Format(L"%s,%d,%d,%d,%d,%d", c_szVer_0, x, y, cx, cy, bMaximized);
	}

	int EnsureVisible()
	{
		RECT rcWnd{ x,y,x + cx,y + cy };
		if (IsRectEmpty(rcWnd))
			return WPSEV_ERROR;
		HMONITOR hMonNearest;
		if (MonitorFromRectByWorkArea(rcWnd, nullptr, &hMonNearest))
			return WPSEV_GOOD_RECT;
		if (!hMonNearest)
			return WPSEV_ERROR;
		MONITORINFO mi;
		mi.cbSize = sizeof(mi);
		GetMonitorInfoW(hMonNearest, &mi);
		if (!AdjustRectIntoAnother(rcWnd, mi.rcWork))
			return WPSEV_ERROR;
		x = rcWnd.left;
		y = rcWnd.top;
		return WPSEV_ADJUSTED;
	}
};
ECK_NAMESPACE_END