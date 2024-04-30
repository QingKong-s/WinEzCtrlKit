/*
* WinEzCtrlKit Library
*
* CEditNcComp.h ： EditExt非客户区组件基类
*
* Copyright(C) 2024 QingKong
*/
#pragma once
#include "CEditExt.h"

ECK_NAMESPACE_BEGIN
class CEditNcComp :public CEditExt
{
protected:
	RECT m_rcBtn{};
	int m_cxBtn{};

	BITBOOL m_bBtnHot : 1 = FALSE;
	BITBOOL m_bLBtnDown : 1 = FALSE;

	EckInline void CursorPtCorrect(POINT& pt)
	{
		pt.x += m_rcMargins.left;
		pt.y += m_rcMargins.top;
	}

	EckInline void RedrawBtn()
	{
		SendMsg(WM_NCPAINT, 0, 0);
	}
public:
	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		switch (uMsg)
		{
		case WM_NCHITTEST:
		{
			POINT pt ECK_GET_PT_LPARAM(lParam);
			ScreenToClient(hWnd, &pt);
			CursorPtCorrect(pt);
			if (PtInRect(m_rcBtn, pt))
				return HTCLIENT;
		}
		break;

		case WM_NCCALCSIZE:
		{
			const auto lResult = CEditExt::OnMsg(hWnd, uMsg, wParam, lParam);
			auto prcClient = (RECT*)lParam;
			const int cxClient = prcClient->right - prcClient->left;
			RECT rc;
			GetWindowRect(hWnd, &rc);

			const int cxBtn = (m_cxBtn ? m_cxBtn : DpiScale(20, GetDpi(hWnd)));
			m_rcBtn.right = m_rcMargins.left + cxClient;
			m_rcBtn.left = m_rcBtn.right - cxBtn;
			m_rcBtn.top = m_rcMargins.top;
			m_rcBtn.bottom = rc.bottom - rc.top - m_rcMargins.bottom;

			prcClient->right -= cxBtn;
			return lResult;
		}
		break;

		case WM_NCPAINT:
		{
			const auto lResult = CEditExt::OnMsg(hWnd, uMsg, wParam, lParam);
			const HDC hDC = GetWindowDC(hWnd);
			OnPaintBtn(hDC);
			ReleaseDC(hWnd, hDC);
			return lResult;
		}
		break;

		case WM_MOUSEMOVE:
		{
			POINT pt ECK_GET_PT_LPARAM(lParam);
			CursorPtCorrect(pt);
			if (PtInRect(m_rcBtn, pt))
			{
				m_bBtnHot = TRUE;
				RedrawBtn();

				TRACKMOUSEEVENT tme;
				tme.cbSize = sizeof(tme);
				tme.dwFlags = TME_LEAVE;
				tme.hwndTrack = hWnd;
				TrackMouseEvent(&tme);
				return 0;
			}
			else if (m_bBtnHot)
			{
				m_bBtnHot = FALSE;
				RedrawBtn();
			}
		}
		break;

		case WM_MOUSELEAVE:
		{
			if (m_bBtnHot)
			{
				m_bBtnHot = FALSE;
				RedrawBtn();
				return 0;
			}
		}
		break;

		case WM_LBUTTONDBLCLK:
		case WM_LBUTTONDOWN:
		{
			POINT pt ECK_GET_PT_LPARAM(lParam);
			CursorPtCorrect(pt);
			if (PtInRect(m_rcBtn, pt))
			{
				SetCapture(hWnd);
				m_bLBtnDown = TRUE;
				RedrawBtn();
				return 0;
			}
		}
		break;

		case WM_LBUTTONUP:
		{
			if(m_bLBtnDown)
			{
				m_bLBtnDown = FALSE;
				ReleaseCapture();
				RedrawBtn();

				POINT pt ECK_GET_PT_LPARAM(lParam);
				CursorPtCorrect(pt);
				if (PtInRect(m_rcBtn, pt))
				{
					OnBtnClick();
					return 0;
				}
			}
		}
		break;

		case WM_CAPTURECHANGED:
		{
			if (m_bLBtnDown)
			{
				m_bLBtnDown = FALSE;
				RedrawBtn();
			}
		}
		break;
		}
		return CEditExt::OnMsg(hWnd, uMsg, wParam, lParam);
	}

	virtual void OnPaintBtn(HDC hDC)
	{
		COLORREF crOld;
		if (m_bLBtnDown)
		{
			FillRect(hDC, &m_rcBtn, GetSysColorBrush(COLOR_HIGHLIGHT));
			crOld = SetTextColor(hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
		}
		else if (m_bBtnHot)
		{
			FillRect(hDC, &m_rcBtn, GetSysColorBrush(COLOR_HOTLIGHT));
			crOld = SetTextColor(hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
		}
		else
		{
			SetDCBrushColor(hDC, m_crBK != CLR_DEFAULT ? m_crBK : m_crDefBK);
			FillRect(hDC, &m_rcBtn, GetStockBrush(DC_BRUSH));
			crOld = SetTextColor(hDC, m_crText != CLR_DEFAULT ? m_crText : m_crDefText);
		}

		const int iOldMode = SetBkMode(hDC, TRANSPARENT);
		DrawTextW(hDC, L"...", -1, &m_rcBtn,
			DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP | DT_NOPREFIX);
		SetBkMode(hDC, iOldMode);

		SetTextColor(hDC, crOld);
	}

	virtual void OnBtnClick()
	{

	}

	EckInline void SetBtnSize(int i) 
	{ 
		m_cxBtn = i; 
		FrameChanged();
	}

	EckInline int GetBtnSize() const { return m_cxBtn; }
};
ECK_NAMESPACE_END