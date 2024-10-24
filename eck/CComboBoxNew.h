/*
* WinEzCtrlKit Library
*
* CComboBoxNew.h ： 组合框
*
* Copyright(C) 2024 QingKong
*/
#pragma once
#include "CListBoxNew.h"
#include "CEditExt.h"

ECK_NAMESPACE_BEGIN
class CComboBoxNew :public CWnd
{
public:
	ECK_RTTI(CComboBoxNew);
	ECK_CWND_SINGLEOWNER(CComboBoxNew);
	ECK_CWND_CREATE_CLS_HINST(WCN_COMBOBOXNEW, g_hInstance);

	enum class View :BYTE
	{
		DropDown,
		DropDownEdit,
	};

private:
	HWND m_hParent{};	// 接收通知的父窗口

	CListBoxNew m_LB{};
	CEditExt* m_pED{};

	HFONT m_hFont{};
	HTHEME m_hTheme{};
	CEzCDC m_DC{};

	int m_cxClient{},
		m_cyClient{};

	BITBOOL m_bHot : 1{};
	BITBOOL m_bDrop : 1{};
	BITBOOL m_bDisabled : 1{};
	BITBOOL m_bHasFocus : 1{};
	BITBOOL m_bAutoDropSize : 1{ TRUE };

	View m_eView{ View::DropDown };
	AnimateStyle m_eAnimate{ AnimateStyle::Blend };

	COLORREF m_crText{ CLR_DEFAULT };

	int m_cxDrop{};
	int m_cyDrop{};

	int m_iDpi{ USER_DEFAULT_SCREEN_DPI };

	int GetDropButtonWidth() const
	{
		return DaGetSystemMetrics(SM_CXVSCROLL, m_iDpi);
	}

	void PositionEdit(BOOL bShow = TRUE)
	{
		EckAssert(m_pED && m_eView == View::DropDownEdit);
		RCWH rc;
		GetEditRect(rc);
		SetWindowPos(m_pED->HWnd, nullptr,
			rc.x, rc.y, rc.cx, rc.cy,
			SWP_NOZORDER | SWP_NOACTIVATE | (bShow ? SWP_SHOWWINDOW : 0));
	}

	void OnPaint(HWND hWnd, WPARAM wParam)
	{
		const auto* const ptc = GetThreadCtx();
		PAINTSTRUCT ps;
		BeginPaint(hWnd, wParam, ps);

		RECT rc{ 0,0,m_cxClient,m_cyClient };
		NMCUSTOMDRAWEXT ne;
		FillNmhdr(ne, NM_CUSTOMDRAW);
		ne.nmcd.hdc = m_DC.GetDC();
		ne.nmcd.rc = rc;
		ne.nmcd.dwItemSpec = 0;
		ne.nmcd.lItemlParam = 0;
		ne.crBk = ne.crText = CLR_DEFAULT;
		ne.iPartId = 0;
		if (m_bDisabled)
		{
			ne.iStateId = CBRO_DISABLED;
			ne.nmcd.uItemState = CDIS_DISABLED;
		}
		else if (m_bDrop)
		{
			ne.iStateId = CBRO_PRESSED;
			ne.nmcd.uItemState = CDIS_SELECTED;
		}
		else if (m_bHot)
		{
			ne.iStateId = CBRO_HOT;
			ne.nmcd.uItemState = CDIS_HOT;
		}
		else
		{
			ne.iStateId = CBRO_NORMAL;
			ne.nmcd.uItemState = 0u;
		}

		ne.nmcd.dwDrawStage = CDDS_PREPAINT;
		const auto lRet = SendNotify(ne, m_hParent);
		if (lRet & CDRF_SKIPDEFAULT)
			goto SkipDef;

		SetDCBrushColor(ne.nmcd.hdc, ptc->crDefBkg);
		FillRect(ne.nmcd.hdc, &ps.rcPaint, GetStockBrush(DC_BRUSH));

		switch (m_eView)
		{
		case View::DropDown:
		{
			DrawThemeBackground(m_hTheme, ne.nmcd.hdc,
				CP_READONLY, ne.iStateId, &rc, nullptr);
			rc.left = rc.right - GetDropButtonWidth();
			DrawThemeBackground(m_hTheme, ne.nmcd.hdc,
				CP_DROPDOWNBUTTONRIGHT, CBXSR_NORMAL, &rc, nullptr);

			NMLBNGETDISPINFO nmdi;
			nmdi.Item.idxItem = m_LB.GetCurrSel();
			if (nmdi.Item.idxItem >= 0)
			{
				m_LB.RequestItem(nmdi);
				if (nmdi.Item.cchText > 0)
				{
					rc.right = rc.left - DaGetSystemMetrics(SM_CXEDGE, m_iDpi);
					rc.left = DaGetSystemMetrics(SM_CXEDGE, m_iDpi);
					if (ne.crText == CLR_DEFAULT)
						SetTextColor(ne.nmcd.hdc, m_crText == CLR_DEFAULT ?
							ptc->crDefText : m_crText);
					else
						SetTextColor(ne.nmcd.hdc, ne.crText);
					DrawTextW(m_DC.GetDC(), nmdi.Item.pszText, nmdi.Item.cchText,
						&rc, DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX);
				}
			}
		}
		break;

		case View::DropDownEdit:
		{
			int iState2;
			if (m_bDisabled)
				iState2 = CBB_DISABLED;
			else if (m_bHasFocus)
				iState2 = CBB_FOCUSED;
			else
				iState2 = CBB_NORMAL;

			DrawThemeBackground(m_hTheme, ne.nmcd.hdc,
				CP_BORDER, iState2, &rc, nullptr);

			rc.left = rc.right - GetDropButtonWidth();
			if (ne.iStateId == CBRO_NORMAL)
				DrawThemeBackground(m_hTheme, ne.nmcd.hdc,
					CP_DROPDOWNBUTTONRIGHT, CBXSR_NORMAL, &rc, nullptr);
			else
				DrawThemeBackground(m_hTheme, ne.nmcd.hdc,
					CP_DROPDOWNBUTTON, ne.iStateId, &rc, nullptr);
		}
		break;

		default: ECK_UNREACHABLE;
		}
	SkipDef:
		if (lRet & CDRF_NOTIFYPOSTPAINT)
		{
			ne.nmcd.dwDrawStage = CDDS_POSTPAINT;
			SendNotify(ne, m_hParent);
		}
		BitBltPs(&ps, m_DC.GetDC());
		EndPaint(hWnd, wParam, ps);
	}

	void UpdateDropSize()
	{
		m_cxDrop = m_cxClient;
		m_cyDrop = std::min(DpiScale(540, m_iDpi),
			m_LB.GetItemCount() * m_LB.GetItemHeight() + DaGetSystemMetrics(SM_CYEDGE, m_iDpi) * 2);
	}

	void OnMouseWheel(int d)
	{

	}
public:
	BOOL PreTranslateMessage(const MSG& Msg) override
	{
		if (Msg.message == WM_MOUSEWHEEL)
		{
			const int idxCurr = m_LB.GetCurrSel();
			if (idxCurr < 0)
				m_LB.SetCurrSel(0);
			else
			{
				int idx = idxCurr + -GET_WHEEL_DELTA_WPARAM(Msg.wParam) / WHEEL_DELTA;
				if (idx < 0)
					idx = 0;
				else if (idx >= m_LB.GetItemCount())
					idx = m_LB.GetItemCount() - 1;
				if (idx != idxCurr)
					m_LB.SetCurrSel(idx);
			}
		}
		return __super::PreTranslateMessage(Msg);
	}

	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		switch (uMsg)
		{
		case WM_PRINTCLIENT:
		case WM_PAINT:
			OnPaint(hWnd, wParam);
			return 0;

		case WM_SIZE:
		{
			ECK_GET_SIZE_LPARAM(m_cxClient, m_cyClient, lParam);
			m_DC.ReSize(hWnd, m_cxClient, m_cyClient);
			SetBkMode(m_DC.GetDC(), TRANSPARENT);
			if (m_eView == View::DropDownEdit)
				PositionEdit();
		}
		break;

		case WM_MOUSEMOVE:
		{
			if (!m_bHot)
			{
				m_bHot = TRUE;
				Redraw();
			}

			TRACKMOUSEEVENT tme;
			tme.cbSize = sizeof(tme);
			tme.dwFlags = TME_LEAVE;
			tme.hwndTrack = hWnd;
			TrackMouseEvent(&tme);
		}
		break;

		case WM_MOUSELEAVE:
		{
			if (m_bHot)
			{
				m_bHot = FALSE;
				Redraw();
			}
		}
		break;

		case WM_LBUTTONDBLCLK:
		case WM_LBUTTONDOWN:
			if (m_bDrop)
				DismissList();
			else
				DropList();
			break;

		case WM_KEYDOWN:
		case WM_KEYUP:
			if (m_bDrop)
			{
				switch (wParam)
				{
				case VK_RETURN:
				case VK_ESCAPE:
					DismissList();
					break;
				}
				m_LB.OnMsg(m_LB.HWnd, uMsg, wParam, lParam);
			}
			break;

		case WM_NOTIFY:
		{
			const auto pnmhdr = (NMHDR*)lParam;
			if (pnmhdr->hwndFrom == m_LB.HWnd)
			{
				switch (pnmhdr->code)
				{
				case NM_LBN_ITEMCHANGED:
				{
					const auto* const p = (NMLBNITEMCHANGED*)lParam;
					if ((p->uFlagsNew & LBN_IF_SEL) && !(p->uFlagsOld & LBN_IF_SEL))
					{
						if (m_eView == View::DropDown)
						{
							Redraw();
							UpdateWindow(hWnd);
						}
						else
						{
							NMLBNGETDISPINFO nmdi;
							nmdi.Item.idxItem = p->idx;
							m_LB.RequestItem(nmdi);
							if (nmdi.Item.cchText > 0)
								m_pED->SetText(nmdi.Item.pszText);
						}
					}
				}
				goto ForwardNotify;

				case NM_LBN_DISMISS:
					DismissList();
					goto ForwardNotify;

				case NM_LBN_ITEMSTANDBY:
					if (m_bAutoDropSize)
						UpdateDropSize();
					goto ForwardNotify;

				default:
				ForwardNotify:
					pnmhdr->hwndFrom = hWnd;
					pnmhdr->idFrom = GetDlgCtrlID(hWnd);
					return SendMessageW(m_hParent, uMsg, pnmhdr->idFrom, lParam);
				}
			}
		}
		break;

		case WM_SETFOCUS:
			if (!m_bHasFocus)
			{
				m_bHasFocus = TRUE;
				if (m_eView == View::DropDownEdit)
					Redraw();
			}
			break;

		case WM_KILLFOCUS:
			if (m_bHasFocus)
			{
				m_bHasFocus = FALSE;
				if (m_eView == View::DropDownEdit)
					Redraw();
			}
			break;

		case WM_SETFONT:
		{
			SendMessageW(m_LB.HWnd, uMsg, wParam, lParam);
			if (m_LB.GetAutoItemHeight() && m_bAutoDropSize)
				UpdateDropSize();
			m_hFont = (HFONT)wParam;
			SelectObject(m_DC.GetDC(), m_hFont);
			if (wParam)
				Redraw();
		}
		return 0;

		case WM_GETFONT:
			return (LRESULT)m_hFont;

		case WM_THEMECHANGED:
		{
			CloseThemeData(m_hTheme);
			m_hTheme = OpenThemeData(hWnd, L"Combobox");
		}
		break;

		case WM_CREATE:
		{
			m_hParent = ((CREATESTRUCTW*)lParam)->hwndParent;
			m_iDpi = GetDpi(hWnd);

			m_LB.Create(nullptr, WS_POPUP | WS_BORDER,
				WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE | WS_EX_TOPMOST,
				0, 0, ((CREATESTRUCTW*)lParam)->cx, 500, hWnd, nullptr);
			SetWindowLongPtrW(m_LB.HWnd, GWLP_HWNDPARENT, (LONG_PTR)hWnd);
			m_LB.SetComboBox(hWnd);

			m_hTheme = OpenThemeData(hWnd, L"Combobox");
			m_DC.Create(hWnd);
			SetBkMode(m_DC.GetDC(), TRANSPARENT);
		}
		break;
		}
		return CWnd::OnMsg(hWnd, uMsg, wParam, lParam);
	}

	void DropList()
	{
		if (m_bDrop)
			return;
		m_bDrop = TRUE;
		SetFocus(HWnd);
		Redraw();
		UpdateWindow(HWnd);

		RECT rc;
		GetWindowRect(HWnd, &rc);

		int cyReal{ m_cyDrop };
		BOOL bDown{ TRUE };
		MONITORINFO mi;
		mi.cbSize = sizeof(mi);
		if (GetMonitorInfoW(MonitorFromWindow(HWnd, MONITOR_DEFAULTTONEAREST), &mi) &&
			rc.bottom + cyReal > mi.rcWork.bottom)// 向下没有足够空间
		{
			if (rc.top - cyReal < mi.rcWork.top)// 向上没有足够空间
			{
				if (m_bAutoDropSize && rc.bottom >= mi.rcWork.top && rc.bottom <= mi.rcWork.bottom)
					if (int cyUpper = rc.top - mi.rcWork.top, cyLower = mi.rcWork.bottom - rc.bottom;
						cyLower > cyUpper)// 下面较大
						cyReal = cyLower;
					else// 上面较大
					{
						cyReal = cyUpper;
						bDown = FALSE;
					}
			}
			else// 向上有足够空间
				bDown = FALSE;
		}
		if (!bDown)
			rc.bottom = rc.top - cyReal;

		SetWindowPos(m_LB.HWnd, HWND_TOPMOST,
			rc.left, rc.bottom, m_cxDrop, cyReal, SWP_NOACTIVATE);

		m_LB.EnsureVisible(m_LB.GetCurrSel());

		UINT uFlags{ UINT(bDown ? AW_VER_POSITIVE : AW_VER_NEGATIVE) };
		switch (m_eAnimate)
		{
		case AnimateStyle::Roll: break;
		case AnimateStyle::Slide: uFlags |= AW_SLIDE; break;
		case AnimateStyle::Center: uFlags |= AW_CENTER; break;
		case AnimateStyle::Blend: uFlags |= AW_BLEND; break;
		}
		AnimateWindow(m_LB.HWnd, 160, uFlags);
		m_LB.CbEnterTrack();
	}

	void DismissList()
	{
		if (!m_bDrop)
			return;
		m_bDrop = FALSE;
		Redraw();
		m_LB.CbLeaveTrack();
		m_LB.Show(SW_HIDE);
	}

	EckInline auto& GetListBox() { return m_LB; }

	EckInline auto& GetEdit() { return *m_pED; }

	void GetEditRect(RCWH& rc)
	{
		const auto cxEdge = DaGetSystemMetrics(SM_CXEDGE, m_iDpi);
		const auto cyEdge = DaGetSystemMetrics(SM_CYEDGE, m_iDpi);
		rc.x = cxEdge;
		rc.y = cyEdge;
		rc.cx = m_cxClient - GetDropButtonWidth() - cxEdge * 2;
		rc.cy = m_cyClient - cyEdge * 2;
	}

	void SetView(View eView)
	{
		m_eView = eView;
		if (m_eView == View::DropDownEdit)
		{
			NMLBNGETDISPINFO nmdi;
			nmdi.Item.idxItem = m_LB.GetCurrSel();
			if (nmdi.Item.idxItem >= 0)
			{
				m_LB.RequestItem(nmdi);
				if (nmdi.Item.cchText <= 0)
					nmdi.Item.pszText = nullptr;
			}
			else
				nmdi.Item.pszText = nullptr;

			if (m_pED)
			{
				m_pED->SetText(nmdi.Item.pszText);
				PositionEdit();
			}
			else
			{
				m_pED = new CEditExt{};
				RCWH rc;
				GetEditRect(rc);
				m_pED->Create(nmdi.Item.pszText, WS_CHILD | WS_VISIBLE | WS_TABSTOP, 0,
					rc.x, rc.y, rc.cx, rc.cy, HWnd, 0);
			}
		}
		else
		{
			if (m_pED)
				m_pED->Show(SW_HIDE);
		}
	}

	EckInline [[nodiscard]] constexpr View GetView() const { return m_eView; }

	EckInline constexpr void SetAnimateStyle(AnimateStyle e) { m_eAnimate = e; }
	EckInline [[nodiscard]] constexpr AnimateStyle GetAnimateStyle() const { return m_eAnimate; }
};
ECK_RTTI_IMPL_BASE_INLINE(CComboBoxNew, CWnd);
ECK_NAMESPACE_END