﻿#pragma once
#include "CBk.h"

ECK_NAMESPACE_BEGIN
struct NMSPBDRAGGED
{
	NMHDR nmhdr;
	int xyPos;
};

class CSplitBar :public CWnd
{
public:
	ECK_RTTI(CSplitBar);
	ECK_CWND_SINGLEOWNER_NO_DEF_CONS(CSplitBar);
	ECK_CWND_CREATE_CLS_HINST(WCN_SPLITBAR, g_hInstance);
private:
	CBk m_BkMark{};

	int m_cxClient{},
		m_cyClient{};

	int m_xyFixed{};
	int m_cxyCursorOffset{};

	int m_xyMin{},
		m_xyMax{};

	COLORREF m_crBK = CLR_DEFAULT;
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
			BITBOOL m_bLBtnDown : 1;
			BITBOOL m_bHorizontal : 1;
		};
		DWORD ECKPRIV_BITFIELD___ = 0;
	};
#endif

	LRESULT OnMarkMsg(HWND hWnd, UINT uMsg,
		WPARAM wParam, LPARAM lParam, BOOL& bProcessed)
	{
		switch (uMsg)
		{
		case WM_PRINTCLIENT:
		case WM_PAINT:
		{
			bProcessed = TRUE;
			PAINTSTRUCT ps;
			BeginPaint(hWnd, wParam, ps);
			SetDCBrushColor(ps.hdc, m_crMark);
			FillRect(ps.hdc, &ps.rcPaint, GetStockBrush(DC_BRUSH));
			EndPaint(hWnd, wParam, ps);
		}
		return 0;
		}
		return 0;
	}

	EckInline void UpdateMarkWndAlpha()
	{
		SetLayeredWindowAttributes(m_BkMark.GetHWND(), 0, m_byMarkAlpha, LWA_ALPHA);
	}

	EckInline void MoveMark(int x, int y)
	{
		SetWindowPos(m_BkMark.GetHWND(), nullptr, x, y, 0, 0,
			SWP_NOZORDER | SWP_NOSIZE | SWP_SHOWWINDOW | SWP_NOACTIVATE);
	}

	EckInline void HideMark()
	{
		ShowWindow(m_BkMark.GetHWND(), SW_HIDE);
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
	CSplitBar()
	{
		m_BkMark.GetSignal().Connect(this, &CSplitBar::OnMarkMsg);
	}

	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		switch (uMsg)
		{
		case WM_SETCURSOR:
		{
			if (m_bHorizontal)
				SetCursor(LoadCursorW(nullptr, IDC_SIZENS));
			else
				SetCursor(LoadCursorW(nullptr, IDC_SIZEWE));
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
			SetWindowPos(m_BkMark.GetHWND(), nullptr, 0, 0, m_cxClient, m_cyClient,
				SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
		}
		return 0;

		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint(hWnd, &ps);
			SetDCBrushColor(ps.hdc, m_crBK == CLR_DEFAULT ? GetThreadCtx()->crDefBkg : m_crBK);
			FillRect(ps.hdc, &ps.rcPaint, GetStockBrush(DC_BRUSH));
			EndPaint(hWnd, &ps);
		}
		return 0;

		case WM_LBUTTONDBLCLK:// 连击修正
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
				FillNmhdrAndSendNotify(nm, NM_SPB_DRAGGED);
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
			m_BkMark.Create(nullptr, WS_POPUP | WS_DISABLED,
				WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_LAYERED | WS_EX_NOACTIVATE,
				-32000, -32000, 0, 0, hWnd, 0);
			UpdateMarkWndAlpha();
		}
		break;
		}

		return __super::OnMsg(hWnd, uMsg, wParam, lParam);
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
			Redraw();
		}
		else if (i == 1)
		{
			m_crMark = cr;
			if (m_BkMark.IsVisible())
				m_BkMark.Redraw();
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
	EckInline constexpr BYTE GetMarkAlpha() const { return m_byMarkAlpha; }

	EckInline constexpr void SetRange(int xyMin, int xyMax)
	{
		m_xyMin = xyMin;
		m_xyMax = xyMax;
	}
	EckInline constexpr void GetRange(int& xyMin, int& xyMax) const
	{
		xyMin = m_xyMin;
		xyMax = m_xyMax;
	}

	EckInline constexpr void SetDirection(BOOL bHorizontal) { m_bHorizontal = bHorizontal; }
	// TRUE - 水平方向，FALSE - 垂直方向
	EckInline constexpr BOOL GetDirection() const { return m_bHorizontal; }
};
ECK_RTTI_IMPL_BASE_INLINE(CSplitBar, CWnd);
ECK_NAMESPACE_END