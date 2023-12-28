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

struct NMSPBDRAGGED
{
	NMHDR nmhdr;
	int xyPos;
};

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

#if ECKCXX20
	BITBOOL m_bLBtnDown : 1 = FALSE;
	BITBOOL m_bHorizontal : 1 = FALSE;
#else
	union
	{
		struct
		{
			BITBOOL m_bLBtnDown : 1 ;
			BITBOOL m_bHorizontal : 1 ;
		};
		DWORD ECKPRIV_BITFIELD___ = 0;
	};
#endif

	static LRESULT CALLBACK WndProc_Mark(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		auto p = (CSplitBar*)GetWindowLongPtrW(hWnd, 0);
		switch (uMsg)
		{
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint(hWnd, &ps);
			FillRect(ps.hdc, &ps.rcPaint, p->m_hbrMark);
			EndPaint(hWnd, &ps);
		}
		return 0;
		}
		return DefWindowProcW(hWnd, uMsg, wParam, lParam);
	}

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

	int CursorPtToPos(POINT ptClient)
	{
		int xyPos;
		if (m_bHorizontal)
		{
			MapWindowPoints(m_hWnd, GetParent(m_hWnd), &ptClient, 1);// 取相对父窗口的光标位置
			xyPos = ptClient.y - m_cxyCursorOffset;
			// 限制位置
			if (m_xyMin == 0 && m_xyMax == 0)// 默认范围为整个父窗口
			{
				RECT rc;
				GetClientRect(GetParent(m_hWnd), &rc);
				if (xyPos < 0)
					xyPos = 0;
				else if (xyPos > rc.bottom - m_cyClient)
					xyPos = rc.bottom - m_cyClient;
			}
			else
			{
				if (xyPos < m_xyMin)
					xyPos = m_xyMin;
				else if (xyPos > m_xyMax)
					xyPos = m_xyMax;
			}
		}
		else
		{
			MapWindowPoints(m_hWnd, GetParent(m_hWnd), &ptClient, 1);// 取相对父窗口的光标位置
			xyPos = ptClient.x - m_cxyCursorOffset;
			// 限制位置
			if (m_xyMin == 0 && m_xyMax == 0)// 默认范围为整个父窗口
			{
				RECT rc;
				GetClientRect(GetParent(m_hWnd), &rc);
				if (xyPos < 0)
					xyPos = 0;
				else if (xyPos > rc.right - m_cxClient)
					xyPos = rc.right - m_cxClient;
			}
			else
			{
				if (xyPos < m_xyMin)
					xyPos = m_xyMin;
				else if (xyPos > m_xyMax)
					xyPos = m_xyMax;
			}
		}
		return xyPos;
	}
public:
	~CSplitBar()
	{
		DeleteObject(m_hbrBK);
		DeleteObject(m_hbrMark);
	}

	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		switch (uMsg)
		{
		case WM_SETCURSOR:
		{
			if (m_bHorizontal)
				SetCursor(LoadCursorW(NULL, IDC_SIZENS));
			else
				SetCursor(LoadCursorW(NULL, IDC_SIZEWE));
		}
		return 0;
		case WM_MOUSEMOVE:
		{
			if (m_bLBtnDown)
			{
				POINT ptClient ECK_GET_PT_LPARAM(lParam);
				const int xyPos = CursorPtToPos(ptClient);
				POINT ptNew{};
				if (m_bHorizontal)
				{
					ptNew.y = xyPos;
					ClientToScreen(GetParent(hWnd), &ptNew);
					ptNew.x = m_xyFixed;
				}
				else
				{
					ptNew.x = xyPos;
					ClientToScreen(GetParent(hWnd), &ptNew);
					ptNew.y = m_xyFixed;
				}

				MoveMark(ptNew.x, ptNew.y);
			}
		}
		return 0;
		case WM_SIZE:
		{
			ECK_GET_SIZE_LPARAM(m_cxClient, m_cyClient, lParam);
			m_DC.ReSize(hWnd, m_cxClient, m_cyClient);
			SetWindowPos(m_BKMark.GetHWND(), NULL, 0, 0, m_cxClient, m_cyClient,
				SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
		}
		return 0;
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint(hWnd, &ps);
			FillRect(ps.hdc, &ps.rcPaint, m_hbrBK);
			EndPaint(hWnd, &ps);
		}
		return 0;
		case WM_LBUTTONDOWN:
		{
			SetFocus(hWnd);
			SetCapture(hWnd);
			m_bLBtnDown = TRUE;
			RECT rc;
			GetWindowRect(hWnd, &rc);

			if (m_bHorizontal)
			{
				m_xyFixed = rc.left;
				MoveMark(rc.left, rc.top);
				m_cxyCursorOffset = GET_Y_LPARAM(lParam);
			}
			else
			{
				m_xyFixed = rc.top;
				MoveMark(rc.left, rc.top);
				m_cxyCursorOffset = GET_X_LPARAM(lParam);
			}
		}
		return 0;
		case WM_LBUTTONUP:
		{
			if (m_bLBtnDown)
			{
				ReleaseCapture();
				m_bLBtnDown = FALSE;
				HideMark();
				const int xyPos = CursorPtToPos(ECK_GET_PT_LPARAM(lParam));
				NMSPBDRAGGED nm;
				nm.xyPos = xyPos;
				FillNmhdrAndSend(nm, NM_SPB_DRAGGED);
			}
		}
		return 0;
		case WM_KEYDOWN:
		{
			if (m_bLBtnDown && wParam == VK_ESCAPE)
			{
				ReleaseCapture();
				m_bLBtnDown = FALSE;
				HideMark();
			}
		}
		return 0;
		case WM_NCCREATE:
		{
			m_BKMark.Create(NULL, WS_POPUP | WS_DISABLED,
				WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_LAYERED | WS_EX_NOACTIVATE,
				-10000, -10000, 0, 0, hWnd, 0);
			UpdateMarkWndAlpha();
			SetWindowLongPtrW(m_BKMark.GetHWND(), 0, (LONG_PTR)this);
			m_BKMark.SetWindowProc(WndProc_Mark);
			m_DC.Create(hWnd);
		}
		break;
		case WM_DESTROY:
			m_BKMark.Destroy();
			return 0;
		}

		return DefWindowProcW(hWnd, uMsg, wParam, lParam);
	}

	ECK_CWND_CREATE;
	HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, HMENU hMenu, PCVOID pData = NULL) override
	{
		m_hWnd = IntCreate(dwExStyle, WCN_SPLITBAR, pszText, dwStyle,
			x, y, cx, cy, hParent, hMenu, g_hInstance, this);
		return m_hWnd;
	}

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