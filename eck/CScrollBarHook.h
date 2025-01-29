#pragma once
#define ECK_SBHOOK_INCLUDED 1
#include "CWnd.h"
#include "CScrollView.h"

ECK_NAMESPACE_BEGIN
constexpr inline PCWSTR PROP_SBHOOK = L"Eck.Prop.SBHook";

class CScrollBarHook
{
public:
	enum class Part : BYTE
	{
		Invalid,

		ArrowUp,
		ArrowDown,
		ArrowLeft,
		ArrowRight,

		PageUp,
		PageDown,
		PageLeft,
		PageRight,

		ThumbV,
		ThumbH
	};

	enum class HotState : BYTE
	{
		None,
		VSB,
		HSB
	};
private:
	CWnd* m_pWnd{};
	HTHEME m_hTheme{};
	CScrollView m_ViewV{}, m_ViewH{}, m_ViewTracking{};
	int m_nLastTrackPos{ INT_MIN };

	RECT m_rcWnd{};// 窗口矩形（相对自身窗口）

	int m_cxVSB{};
	int m_cyHSB{};
	int m_cxSBArrow{};
	int m_cySBArrow{};

	int m_iDpi{ USER_DEFAULT_SCREEN_DPI };

	MARGINS m_Margins{};

	RECT m_rcSBV{};
	RECT m_rcSBH{};

	int m_cyThumbV{};
	int m_cxThumbH{};

	int m_yThumbV{};
	int m_xThumbH{};

	Part m_eHotPart{ Part::Invalid };
	Part m_eLBtnDownPart{ Part::Invalid };
	HotState m_eHotState{ HotState::None };

	BITBOOL m_bProtectStyle : 1{ FALSE };
	BITBOOL m_bVisibleH : 1{ TRUE };
	BITBOOL m_bVisibleV : 1{ TRUE };
	BITBOOL m_bDisableH : 1{ FALSE };
	BITBOOL m_bDisableV : 1{ FALSE };


	void ReCalcScrollRect()
	{
		m_rcSBV.right = m_rcWnd.right - m_Margins.cxRightWidth;
		m_rcSBV.left = m_rcSBV.right - m_cxVSB;
		m_rcSBV.top = m_rcWnd.top + m_Margins.cyTopHeight;
		m_rcSBV.bottom = m_rcWnd.bottom - m_Margins.cyBottomHeight;
		if (m_bVisibleH)
			m_rcSBV.bottom -= m_cyHSB;

		m_rcSBH.bottom = m_rcWnd.bottom - m_Margins.cyBottomHeight;
		m_rcSBH.top = m_rcSBH.bottom - m_cyHSB;
		m_rcSBH.left = m_rcWnd.left + m_Margins.cxLeftWidth;
		m_rcSBH.right = m_rcWnd.right - m_Margins.cxRightWidth;
		if (m_bVisibleV)
			m_rcSBH.right -= m_cxVSB;

		m_ViewH.SetViewSize(m_rcSBH.right - m_rcSBH.left - m_cxSBArrow * 2);
		m_ViewV.SetViewSize(m_rcSBV.bottom - m_rcSBV.top - m_cySBArrow * 2);
	}

	void ReCalcThumbH()
	{
		m_cxThumbH = m_ViewH.GetThumbSize();
		m_xThumbH = m_ViewH.GetThumbPos(m_cxThumbH) + m_rcSBH.left + m_cxSBArrow;
	}

	void ReCalcThumbV()
	{
		m_cyThumbV = m_ViewV.GetThumbSize();
		m_yThumbV = m_ViewV.GetThumbPos(m_cyThumbV) + m_rcSBV.top + m_cySBArrow;
	}

	void ReCalcThumb()
	{
		ReCalcThumbH();
		ReCalcThumbV();
	}

	void ReCalcTrackingThumbH()
	{
		m_cxThumbH = m_ViewTracking.GetThumbSize();
		m_xThumbH = m_ViewTracking.GetThumbPos(m_cxThumbH) + m_rcSBH.left + m_cxSBArrow;
	}
	void ReCalcTrackingThumbV()
	{
		m_cyThumbV = m_ViewTracking.GetThumbSize();
		m_yThumbV = m_ViewTracking.GetThumbPos(m_cyThumbV) + m_rcSBV.top + m_cySBArrow;
	}

	void UpdateVisibleFromStyle()
	{
		const auto dwStyle = m_pWnd->GetStyle();
		m_bVisibleH = IsBitSet(dwStyle, WS_HSCROLL);
		m_bVisibleV = IsBitSet(dwStyle, WS_VSCROLL);
	}


	void Redraw(HDC hDC)
	{
		HTHEME hTheme = m_hTheme;
		RECT rc;
		int iState;
		if (m_bVisibleV)
		{
			rc = m_rcSBV;
			// 上箭头
			rc.bottom = rc.top + m_cySBArrow;
			if (m_bDisableV)
				iState = ABS_UPDISABLED;
			else if (m_eLBtnDownPart == Part::ArrowUp)
				iState = ABS_UPPRESSED;
			else if (m_eHotPart == Part::ArrowUp)
				iState = ABS_UPHOT;
			else if (m_eHotState == HotState::VSB)
				iState = ABS_UPHOVER;
			else
				iState = ABS_UPNORMAL;
			auto hr = DrawThemeBackground(hTheme, hDC, SBP_ARROWBTN, iState, &rc, NULL);
			// 上空白
			rc.top = rc.bottom;
			rc.bottom = m_yThumbV;
			if (m_bDisableV)
				iState = SCRBS_DISABLED;
			else if (m_eLBtnDownPart == Part::PageUp)
				iState = SCRBS_PRESSED;
			else if (m_eHotPart == Part::PageUp)
				iState = SCRBS_HOT;
			else if (m_eHotState == HotState::VSB)
				iState = SCRBS_HOVER;
			else
				iState = SCRBS_NORMAL;
			DrawThemeBackground(hTheme, hDC, SBP_LOWERTRACKVERT, iState, &rc, NULL);
			// 滑块
			rc.top = rc.bottom;
			rc.bottom = rc.top + m_cyThumbV;
			if (m_bDisableV)
				iState = SCRBS_DISABLED;
			else if (m_eLBtnDownPart == Part::ThumbV)
				iState = SCRBS_PRESSED;
			else if (m_eHotPart == Part::ThumbV)
				iState = SCRBS_HOT;
			else if (m_eHotState == HotState::VSB)
				iState = SCRBS_HOVER;
			else
				iState = SCRBS_NORMAL;
			DrawThemeBackground(hTheme, hDC, SBP_THUMBBTNVERT, iState, &rc, NULL);
			// 下空白
			rc.top = rc.bottom;
			rc.bottom = m_rcSBV.bottom - m_cySBArrow;
			if (m_bDisableV)
				iState = SCRBS_DISABLED;
			else if (m_eLBtnDownPart == Part::PageDown)
				iState = SCRBS_PRESSED;
			else if (m_eHotPart == Part::PageDown)
				iState = SCRBS_HOT;
			else if (m_eHotState == HotState::VSB)
				iState = SCRBS_HOVER;
			else
				iState = SCRBS_NORMAL;
			DrawThemeBackground(hTheme, hDC, SBP_UPPERTRACKVERT, iState, &rc, NULL);
			// 下箭头
			rc.top = rc.bottom;
			rc.bottom = m_rcSBV.bottom;
			if (m_bDisableV)
				iState = ABS_DOWNDISABLED;
			else if (m_eLBtnDownPart == Part::ArrowDown)
				iState = ABS_DOWNPRESSED;
			else if (m_eHotPart == Part::ArrowDown)
				iState = ABS_DOWNHOT;
			else if (m_eHotState == HotState::VSB)
				iState = ABS_DOWNHOVER;
			else
				iState = ABS_DOWNNORMAL;
			DrawThemeBackground(hTheme, hDC, SBP_ARROWBTN, iState, &rc, NULL);
		}

		if (m_bVisibleH)
		{
			rc = m_rcSBH;
			// 左箭头
			rc.right = rc.left + m_cxSBArrow;
			if (m_bDisableH)
				iState = ABS_LEFTDISABLED;
			else if (m_eLBtnDownPart == Part::ArrowLeft)
				iState = ABS_LEFTPRESSED;
			else if (m_eHotPart == Part::ArrowLeft)
				iState = ABS_LEFTHOT;
			else if (m_eHotState == HotState::HSB)
				iState = ABS_LEFTHOVER;
			else
				iState = ABS_LEFTNORMAL;
			DrawThemeBackground(hTheme, hDC, SBP_ARROWBTN, iState, &rc, NULL);
			// 左空白
			rc.left = rc.right;
			rc.right = m_xThumbH;
			if (m_bDisableH)
				iState = SCRBS_DISABLED;
			else if (m_eLBtnDownPart == Part::PageLeft)
				iState = SCRBS_PRESSED;
			else if (m_eHotPart == Part::PageLeft)
				iState = SCRBS_HOT;
			else if (m_eHotState == HotState::HSB)
				iState = SCRBS_HOVER;
			else
				iState = SCRBS_NORMAL;
			DrawThemeBackground(hTheme, hDC, SBP_LOWERTRACKHORZ, iState, &rc, NULL);
			// 滑块
			rc.left = rc.right;
			rc.right = rc.left + m_cxThumbH;
			if (m_bDisableH)
				iState = SCRBS_DISABLED;
			else if (m_eLBtnDownPart == Part::ThumbH)
				iState = SCRBS_PRESSED;
			else if (m_eHotPart == Part::ThumbH)
				iState = SCRBS_HOT;
			else if (m_eHotState == HotState::HSB)
				iState = SCRBS_HOVER;
			else
				iState = SCRBS_NORMAL;
			DrawThemeBackground(hTheme, hDC, SBP_THUMBBTNHORZ, iState, &rc, NULL);
			// 右空白
			rc.left = rc.right;
			rc.right = m_rcSBH.right - m_cxSBArrow;
			if (m_bDisableH)
				iState = SCRBS_DISABLED;
			else if (m_eLBtnDownPart == Part::PageRight)
				iState = SCRBS_PRESSED;
			else if (m_eHotPart == Part::PageRight)
				iState = SCRBS_HOT;
			else if (m_eHotState == HotState::HSB)
				iState = SCRBS_HOVER;
			else
				iState = SCRBS_NORMAL;
			DrawThemeBackground(hTheme, hDC, SBP_UPPERTRACKHORZ, iState, &rc, NULL);
			// 右箭头
			rc.left = rc.right;
			rc.right = m_rcSBH.right;
			if (m_bDisableH)
				iState = ABS_RIGHTDISABLED;
			else if (m_eLBtnDownPart == Part::ArrowRight)
				iState = ABS_RIGHTPRESSED;
			else if (m_eHotPart == Part::ArrowRight)
				iState = ABS_RIGHTHOT;
			else if (m_eHotState == HotState::HSB)
				iState = ABS_RIGHTHOVER;
			else
				iState = ABS_RIGHTNORMAL;
			DrawThemeBackground(hTheme, hDC, SBP_ARROWBTN, iState, &rc, NULL);
		}
	}

	void Redraw()
	{
		const auto hDC = GetWindowDC(m_pWnd->HWnd);
		Redraw(hDC);
		ReleaseDC(m_pWnd->HWnd, hDC);
	}
public:
	LRESULT OnWndMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bProcessed)
	{
		switch (uMsg)
		{
		case WM_NCHITTEST:
		{
			if (!m_bVisibleH && !m_bVisibleV)
				break;
			POINT pt ECK_GET_PT_LPARAM(lParam);
			ScreenToClient(hWnd, &pt);
			if (m_bVisibleV && PtInRect(m_rcSBV, pt))
			{
				bProcessed = TRUE;
				return HTVSCROLL;
			}
			if (m_bVisibleH && PtInRect(m_rcSBH, pt))
			{
				bProcessed = TRUE;
				return HTHSCROLL;
			}
		}
		break;

		case WM_MOUSEMOVE:
		{
			if (m_eLBtnDownPart == Part::ThumbV)
			{
				bProcessed = TRUE;
				m_ViewTracking.OnMouseMove(GET_Y_LPARAM(lParam) + m_Margins.cyTopHeight
					- m_rcSBV.top - m_cySBArrow);
				ReCalcTrackingThumbV();
				Redraw();
				if (m_ViewTracking.GetPos() != m_nLastTrackPos)
				{
					m_pWnd->SendMsg(WM_VSCROLL, MAKEWPARAM(SB_THUMBTRACK, m_ViewTracking.GetPos()), 0);
					m_nLastTrackPos = m_ViewTracking.GetPos();
				}
				return 0;
			}
			else if (m_eLBtnDownPart == Part::ThumbH)
			{
				bProcessed = TRUE;
				m_ViewTracking.OnMouseMove(GET_X_LPARAM(lParam) + m_Margins.cxLeftWidth
					- m_rcSBH.left - m_cxSBArrow);
				ReCalcTrackingThumbH();
				Redraw();
				if (m_ViewTracking.GetPos() != m_nLastTrackPos)
				{
					m_pWnd->SendMsg(WM_HSCROLL, MAKEWPARAM(SB_THUMBTRACK, m_ViewTracking.GetPos()), 0);
					m_nLastTrackPos = m_ViewTracking.GetPos();
				}
				return 0;
			}
		}
		break;

		case WM_NCLBUTTONUP:
		case WM_LBUTTONUP:
		{
			if (m_eLBtnDownPart != Part::Invalid)
			{
				bProcessed = TRUE;// Eat it.
				if (m_eLBtnDownPart == Part::ThumbV)
				{
					m_ViewTracking.OnLButtonUp();
					m_pWnd->SendMsg(WM_VSCROLL, MAKEWPARAM(SB_THUMBPOSITION, m_ViewTracking.GetPos()), 0);
					m_pWnd->SendMsg(WM_VSCROLL, MAKEWPARAM(SB_ENDSCROLL, 0), 0);
				}
				else if (m_eLBtnDownPart == Part::ThumbH)
				{
					m_ViewTracking.OnLButtonUp();
					m_pWnd->SendMsg(WM_HSCROLL, MAKEWPARAM(SB_THUMBPOSITION, m_ViewTracking.GetPos()), 0);
					m_pWnd->SendMsg(WM_HSCROLL, MAKEWPARAM(SB_ENDSCROLL, 0), 0);
				}
				m_eLBtnDownPart = Part::Invalid;
				ReleaseCapture();

				POINT pt ECK_GET_PT_LPARAM(lParam);
				if (uMsg == WM_NCLBUTTONUP)
					ScreenToClient(hWnd, &pt);
				const auto ePart = HitTest(pt);
				const auto eHot = (
					ePart == Part::ThumbV ? HotState::VSB :
					ePart == Part::ThumbH ? HotState::HSB : HotState::None);
				if (eHot != m_eHotState)
				{
					m_eHotState = eHot;
					Redraw();
				}
				return 0;
			}
		}
		break;

		case WM_WINDOWPOSCHANGED:
		{
			const auto* const pwp = (WINDOWPOS*)lParam;
			m_rcWnd.left = 0;
			m_rcWnd.top = 0;
			m_rcWnd.right = pwp->cx;
			m_rcWnd.bottom = pwp->cy;
			ReCalcScrollRect();
			ReCalcThumb();
		}
		break;

		case WM_NCCALCSIZE:
		{
			bProcessed = TRUE;
			const auto dwStyle = m_pWnd->GetStyle();
			if (dwStyle & (WS_HSCROLL | WS_VSCROLL))
			{
				m_bProtectStyle = TRUE;
				m_pWnd->SetStyle(dwStyle & ~(WS_HSCROLL | WS_VSCROLL));
			}
			const RECT rcWnd{ *(RECT*)lParam };
			m_pWnd->OnMsg(hWnd, uMsg, wParam, lParam);
			if (dwStyle & (WS_HSCROLL | WS_VSCROLL))
			{
				m_pWnd->SetStyle(dwStyle);
				m_bProtectStyle = FALSE;
			}
			const auto prcClient = (RECT*)lParam;
			m_Margins.cxLeftWidth = prcClient->left - rcWnd.left;
			m_Margins.cxRightWidth = rcWnd.right - prcClient->right;
			m_Margins.cyTopHeight = prcClient->top - rcWnd.top;
			m_Margins.cyBottomHeight = rcWnd.bottom - prcClient->bottom;

			if (m_bVisibleV && (prcClient->right - prcClient->left) > m_cxVSB)
				prcClient->right -= m_cxVSB;
			if (m_bVisibleH && (prcClient->bottom - prcClient->top) > m_cyHSB)
				prcClient->bottom -= m_cyHSB;
		}
		return 0;

		case WM_NCPAINT:
		{
			bProcessed = TRUE;
			m_pWnd->OnMsg(hWnd, uMsg, wParam, lParam);
			Redraw();
		}
		return 0;

		case WM_NCLBUTTONDOWN:
		{
			if (wParam != HTVSCROLL && wParam != HTHSCROLL)
				break;
			bProcessed = TRUE;
			POINT pt ECK_GET_PT_LPARAM(lParam);
			ScreenToClient(hWnd, &pt);
			const auto ePart = HitTest(pt);
			m_eLBtnDownPart = ePart;
			if (wParam == HTVSCROLL)
			{
				switch (ePart)
				{
				case Part::ThumbV:
					m_ViewTracking = m_ViewV;
					m_nLastTrackPos = m_ViewTracking.GetPos();
					m_ViewTracking.OnLButtonDown(pt.y + m_Margins.cyTopHeight
						- m_rcSBV.top - m_cySBArrow);
					break;
				}
			}
			else
			{
				switch (ePart)
				{
				case Part::ThumbH:
					m_ViewTracking = m_ViewH;
					m_nLastTrackPos = m_ViewTracking.GetPos();
					m_ViewTracking.OnLButtonDown(pt.x + m_Margins.cxLeftWidth
						- m_rcSBH.left - m_cxSBArrow);
					break;
				}
			}
			SetCapture(hWnd);
		}
		return 0;

		case WM_NCMOUSEMOVE:
		{
			bProcessed = TRUE;
			if (m_eLBtnDownPart != Part::Invalid)
				break;
			const auto eOldHot = m_eHotState;
			switch (wParam)
			{
			case HTVSCROLL:
				m_eHotState = HotState::VSB;
				break;
			case HTHSCROLL:
				m_eHotState = HotState::HSB;
				break;
			default:
				m_eHotState = HotState::None;
				break;
			}
			TRACKMOUSEEVENT tme;
			tme.cbSize = sizeof(tme);
			tme.dwFlags = TME_LEAVE | TME_NONCLIENT;
			tme.hwndTrack = hWnd;
			TrackMouseEvent(&tme);
			if (eOldHot != m_eHotState)
				Redraw();
		}
		return 0;

		case WM_NCMOUSELEAVE:
		{
			if (m_eLBtnDownPart != Part::Invalid)
				break;
			if (m_eHotState != HotState::None)
			{
				m_eHotState = HotState::None;
				Redraw();
			}
		}
		return 0;

		case WM_STYLECHANGED:
		{
			bProcessed = m_bProtectStyle;
			const auto* const pss = (STYLESTRUCT*)lParam;
			if (wParam == GWL_STYLE)
			{
				m_bVisibleH = IsBitSet(pss->styleNew, WS_HSCROLL);
				m_bVisibleV = IsBitSet(pss->styleNew, WS_VSCROLL);
			}
		}
		break;
		}

		return 0;
	}

	HRESULT BindWnd(CWnd* pWnd)
	{
		if (pWnd->GetSignal().FindSlot(MHI_SCROLLBAR_HOOK))
			return S_FALSE;
		SCROLLINFO si{ sizeof(si), SIF_ALL };
		pWnd->GetSbInfo(SB_HORZ, &si);
		m_ViewH.SetScrollInfo(si);
		pWnd->GetSbInfo(SB_VERT, &si);
		m_ViewV.SetScrollInfo(si);

		SetPropW(pWnd->HWnd, PROP_SBHOOK, this);
		m_pWnd = pWnd;

		m_iDpi = GetDpi(pWnd->HWnd);
		m_cxVSB = GetSystemMetrics(SM_CXVSCROLL);
		//m_cxVSB = DpiScale(50, m_iDpi);
		m_cyHSB = GetSystemMetrics(SM_CYHSCROLL);
		//m_cyHSB = DpiScale(60, m_iDpi);
		m_cxSBArrow = GetSystemMetrics(SM_CXHSCROLL);
		m_cySBArrow = GetSystemMetrics(SM_CYVSCROLL);

		m_hTheme = OpenNcThemeData(pWnd->HWnd, L"ScrollBar");

		GetWindowRect(pWnd->HWnd, &m_rcWnd);
		m_rcWnd.right -= m_rcWnd.left;
		m_rcWnd.bottom -= m_rcWnd.top;
		m_rcWnd.left = 0;
		m_rcWnd.top = 0;

		pWnd->GetSignal().Connect(this, &CScrollBarHook::OnWndMsg, MHI_SCROLLBAR_HOOK);
		pWnd->FrameChanged();

		UpdateVisibleFromStyle();
		ReCalcScrollRect();
		ReCalcThumb();
		return S_OK;
	}

	Part HitTest(POINT pt)
	{
		if (m_bVisibleV && PtInRect(m_rcSBV, pt))
		{
			/*
			* ---	m_rcSBV.top
			*  ↑
			* ---   ySpace1
			*
			* ---	m_yThumb
			*  T
			*  h
			*  u
			*  m
			*  b
			* ---   ySpace2
			*
			* ---	yArrow2
			*  ↓
			* ---	m_rcSBV.bottom
			*/

			// ySpace1 = m_rcSBV.top + m_cySBArrow
			// ySpace2 = m_yThumb + m_cyVThumb
			// yArrow2 = m_rcSBV.bottom - m_cySBArrow

			if (pt.y < m_rcSBV.top + m_cySBArrow)
				return Part::ArrowUp;
			if (pt.y < m_yThumbV)
				return Part::PageUp;
			else if (pt.y < m_yThumbV + m_cyThumbV)
				return Part::ThumbV;
			else if (pt.y < m_rcSBV.bottom - m_cySBArrow)
				return Part::PageDown;
			else// if (pt.y < m_rcSBV.bottom)
				return Part::ArrowDown;
		}
		else if (m_bVisibleH && PtInRect(&m_rcSBH, pt))
		{
			// m_rcSBH.left    m_xThumb	           m_rcSBH.right
			// |←|            | Thumb |        |→|
			//	  xSpace1			   xSpace2  xArrow2

			// xSpace1 = m_rcSBH.left + m_cxSBArrow
			// xSpace2 = m_xThumb + m_cxHThumb
			// xArrow2 = m_rcSBH.right - m_cxSBArrow

			if (pt.x < m_rcSBH.left + m_cxSBArrow)
				return Part::ArrowLeft;
			else if (pt.x < m_xThumbH)
				return Part::PageLeft;
			else if (pt.x < m_xThumbH + m_cxThumbH)
				return Part::ThumbH;
			else if (pt.x < m_rcSBH.right - m_cxSBArrow)
				return Part::PageRight;
			else// if (pt.x < m_rcSBH.right)
				return Part::ArrowRight;
		}
		return Part::Invalid;
	}

	int HkSetScrollInfo(HWND hWnd, int nBar, const SCROLLINFO* psi, BOOL bRedraw)
	{
		switch (nBar)
		{
		case SB_HORZ:
			m_ViewH.SetScrollInfo(*psi);
			ReCalcThumbH();
			if (bRedraw)
				Redraw();
			return m_ViewH.GetPos();
		case SB_VERT:
			m_ViewV.SetScrollInfo(*psi);
			ReCalcThumbV();
			if (bRedraw)
				Redraw();
			return m_ViewV.GetPos();
		}
	}

	BOOL HkGetScrollInfo(HWND hWnd, int nBar, SCROLLINFO* psi)
	{
		switch (nBar)
		{
		case SB_HORZ:
			m_ViewH.GetScrollInfo(*psi);
			if ((psi->fMask & SIF_TRACKPOS) && m_eLBtnDownPart == Part::ThumbH)
				psi->nTrackPos = m_ViewTracking.GetPos();
			return TRUE;
		case SB_VERT:
			m_ViewV.GetScrollInfo(*psi);
			if ((psi->fMask & SIF_TRACKPOS) && m_eLBtnDownPart == Part::ThumbV)
				psi->nTrackPos = m_ViewTracking.GetPos();
			return TRUE;
		}
		return FALSE;
	}

	BOOL HkShowScrollBar(HWND hWnd, int nBar, BOOL bShow)
	{
		return TRUE;
	}
};
ECK_NAMESPACE_END