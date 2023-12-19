/*
* WinEzCtrlKit Library
*
* CSplitBar.h ： 分隔条
*
* Copyright(C) 2023 QingKong
*/
#pragma once
#include "CBk.h"
#include "Utility.h"
#include "GraphicsHelper.h"


ECK_NAMESPACE_BEGIN
class CSplitBar :public CWnd
{
private:
	CBk m_BKMark{};
	CEzCDC m_DC{};

	int m_cxClient = 0,
		m_cyClient = 0;

	int m_xyFixed = 0;
	int m_cxyCursorOffset = 0;

	int m_xyMin = 0,
		m_xyMax = 0;

	HBRUSH m_hbrBK = GetSysColorBrush(COLOR_WINDOW);
	HBRUSH m_hbrMark = CreateSolidBrush(0xFFCC66);

	COLORREF m_crBK = GetSysColor(COLOR_WINDOW);
	COLORREF m_crMark = 0xFFCC66;
	BYTE m_byMarkAlpha = 0xA0;

	BITBOOL m_bLBtnDown : 1 = FALSE;
	BITBOOL m_bHorizontal : 1 = FALSE;

	UINT m_uNotifyMsg = 0;

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	static LRESULT CALLBACK WndProc_Mark(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	EckInline void UpdateMarkWndAlpha()
	{
		SetLayeredWindowAttributes(m_BKMark.GetHWND(), 0, m_byMarkAlpha, LWA_ALPHA);
	}

	EckInline void MoveMark(int x, int y)
	{
		SetWindowPos(m_BKMark.GetHWND(), NULL, x, y, 0, 0,
			SWP_NOZORDER | SWP_NOSIZE | SWP_SHOWWINDOW | SWP_NOACTIVATE);
	}

	EckInline void HideMark()
	{
		ShowWindow(m_BKMark.GetHWND(), SW_HIDE);
	}

	int CursorPtToPos(POINT ptClient);
public:
	~CSplitBar();

	static ATOM RegisterWndClass(HINSTANCE hInstance);

	EckInline 
	ECK_CWND_CREATE
	{
		m_hWnd = IntCreate(dwExStyle, WCN_SPLITBAR, pszText, dwStyle,
			x, y, cx, cy, hParent, hMenu, g_hInstance, this);
		return m_hWnd;
	}

	EckInline void SetNotifyMsg(UINT uMsg) { m_uNotifyMsg = uMsg; }

	/// <summary>
	/// 置颜色
	/// </summary>
	/// <param name="i">0 - 背景颜色  1 - 拖动标记颜色</param>
	/// <param name="cr">颜色</param>
	EckInline void SetClr(int i, COLORREF cr)
	{
		if (i == 0)
		{
			m_crBK = cr;
			DeleteObject(m_hbrBK);
			m_hbrBK = CreateSolidBrush(cr);
			Redraw();
		}
		else if (i == 1)
		{
			m_crMark = cr;
			DeleteObject(m_hbrMark);
			m_hbrMark = CreateSolidBrush(cr);
			if (m_BKMark.IsVisible())
				m_BKMark.Redraw();
		}
		else
			EckDbgBreak();
	}

	/// <summary>
	/// 取颜色
	/// </summary>
	/// <param name="i">0 - 背景颜色  1 - 拖动标记颜色</param>
	/// <return>颜色</return>
	EckInline COLORREF GetClr(int i) const
	{
		if (i == 0)
			return m_crBK;
		else if (i == 1)
			return m_crMark;
		else
			EckDbgBreak();
	}

	EckInline void SetMarkAlpha(BYTE byAlpha)
	{
		m_byMarkAlpha = byAlpha;
		UpdateMarkWndAlpha();
	}

	EckInline void SetRange(int xyMin, int xyMax)
	{
		m_xyMin = xyMin;
		m_xyMax = xyMax;
	}

	EckInline void SetDirection(BOOL bHorizontal)
	{
		m_bHorizontal = bHorizontal;
	}
};
ECK_NAMESPACE_END