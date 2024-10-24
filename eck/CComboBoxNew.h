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
	CEditExt m_ED{};

	HFONT m_hFont{};
	HTHEME m_hTheme{};
	CEzCDC m_DC{};

	int m_cxClient{},
		m_cyClient{};

	BITBOOL m_bHot : 1{};
	BITBOOL m_bDrop : 1{};
	BITBOOL m_bDisabled : 1{};

	View m_eView{ View::DropDown };

	COLORREF m_crText{ CLR_DEFAULT };

	int m_iDpi{ USER_DEFAULT_SCREEN_DPI };
	ECK_DS_BEGIN(DPIS)
		ECK_DS_ENTRY(cyMaxDropDown, 540)
		;
	ECK_DS_END_VAR(m_Ds);

	int GetDropButtonWidth() const
	{
		return DaGetSystemMetrics(SM_CXVSCROLL, m_iDpi);
	}
public:
	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		switch (uMsg)
		{
		case WM_PRINTCLIENT:
		case WM_PAINT:
		{
			const auto* const ptc = GetThreadCtx();
			PAINTSTRUCT ps;
			BeginPaint(hWnd, wParam, ps);

			NMCUSTOMDRAWEXT ne;


			int iState;
			RECT rc{ 0,0,m_cxClient,m_cyClient };
			switch (m_eView)
			{
			case View::DropDown:
			{
				if (m_bDisabled)
					iState = CBRO_DISABLED;
				else if (m_bDrop)
					iState = CBRO_PRESSED;
				else if (m_bHot)
					iState = CBRO_HOT;
				else
					iState = CBRO_NORMAL;
				DrawThemeBackground(m_hTheme, m_DC.GetDC(),
					CP_READONLY, iState, &rc, nullptr);
				rc.left = rc.right - GetDropButtonWidth();
				DrawThemeBackground(m_hTheme, m_DC.GetDC(),
					CP_DROPDOWNBUTTONRIGHT, CBXSR_NORMAL, &rc, nullptr);
			}
			break;

			case View::DropDownEdit:
			{

			}
			break;
			}

			NMLBNGETDISPINFO nm{};
			nm.Item.idxItem = m_LB.GetCurrSel();
			if (nm.Item.idxItem >=0)
			{
				FillNmhdrAndSendNotify(nm, m_hParent, NM_LBN_GETDISPINFO);
				rc.right = rc.left - DaGetSystemMetrics(SM_CXEDGE, m_iDpi);
				rc.left = DaGetSystemMetrics(SM_CXEDGE, m_iDpi);
				DrawTextW(m_DC.GetDC(), nm.Item.pszText, nm.Item.cchText,
					&rc, DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX);
			}

			BitBltPs(&ps, m_DC.GetDC());
			EndPaint(hWnd, wParam, ps);
		}
		return 0;

		case WM_SIZE:
		{
			ECK_GET_SIZE_LPARAM(m_cxClient, m_cyClient, lParam);
			m_DC.ReSize(hWnd, m_cxClient, m_cyClient);
			SetBkMode(m_DC.GetDC(), TRANSPARENT);
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
				case NM_LBN_ITEMSTANDBY:
				{

				}
				return 0;

				case NM_LBN_DISMISS:
					DismissList();
					return 0;

				default:
				{
					pnmhdr->hwndFrom = hWnd;
					pnmhdr->idFrom = GetDlgCtrlID(hWnd);
				}
				return SendMessageW(m_hParent, uMsg, pnmhdr->idFrom, lParam);
				}
			}
		}
		break;

		case WM_SETFONT:
		{
			SendMessageW(m_LB.HWnd, uMsg, wParam, lParam);
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
			UpdateDpiSize(m_Ds, m_iDpi);

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
		InvalidateRect(HWnd, nullptr, FALSE);
		UpdateWindow(HWnd);

		RECT rc;
		GetWindowRect(HWnd, &rc);

		const auto hMon = MonitorFromWindow(HWnd, MONITOR_DEFAULTTONEAREST);
		MONITORINFO mi;
		mi.cbSize = sizeof(mi);
		GetMonitorInfoW(hMon, &mi);
		const auto rcWork = mi.rcWork;

		SetWindowPos(m_LB.HWnd, HWND_TOPMOST,
			rc.left, rc.bottom, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE);
		AnimateWindow(m_LB.HWnd, 200, AW_BLEND);
		//AnimateWindow(m_LB.HWnd, 200, AW_VER_POSITIVE);
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

	EckInline auto& GetEdit() { return m_ED; }
};
ECK_RTTI_IMPL_BASE_INLINE(CComboBoxNew, CWnd);
ECK_NAMESPACE_END