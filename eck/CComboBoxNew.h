#pragma once
#include "CListBoxNew.h"
#include "CEditExt.h"

ECK_NAMESPACE_BEGIN
class CComboBoxNew :public CWnd
{
public:
	enum class View
	{
		DropDown,
		DropDownEdit,
	};
private:
	CListBoxNew m_LB{};
	CEditExt m_ED{};

	HTHEME m_hTheme{};

	int m_cxClient{},
		m_cyClient{};

	CEzCDC m_DC{};

	BITBOOL m_bHot : 1{};
	BITBOOL m_bDrop : 1{};
	BITBOOL m_bDisabled : 1{};

	View m_eView{ View::DropDown };

	COLORREF m_crText{ CLR_DEFAULT };
	COLORREF m_crDefText{ CLR_INVALID };

	int m_cxLB{ -1 },
		m_cyLB{ -1 };

	int m_iDpi{ USER_DEFAULT_SCREEN_DPI };
	ECK_DS_BEGIN(DPIS)
		ECK_DS_ENTRY(cxDropBtn, 24)
		ECK_DS_ENTRY(cxDefLB, 300)
		ECK_DS_ENTRY(cyDefLB, 400)
		;
	ECK_DS_END_VAR(m_Ds);

	void UpdateThemeInfo()
	{
		COLORREF dummy;
		GetItemsViewForeBackColor(m_crDefText, dummy);
	}
public:
	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		switch (uMsg)
		{
		case WM_CREATE:
		{
			m_iDpi = GetDpi(hWnd);
			UpdateDpiSize(m_Ds, m_iDpi);

			SetExplorerTheme();

			m_LB.Create(NULL, WS_POPUP | WS_BORDER, 
				WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE|WS_EX_TOPMOST,
				0, 0, 0, 0, hWnd, NULL);
			SetWindowLongPtrW(m_LB.HWnd, GWLP_HWNDPARENT, (LONG_PTR)hWnd);
			m_LB.SetComboBox(hWnd);

			m_hTheme = OpenThemeData(hWnd, L"ComboBox");
			UpdateThemeInfo();
			m_DC.Create(hWnd);
		}
		break;

		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint(hWnd, &ps);

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
					CP_READONLY, iState, &rc, NULL);
				rc.left = rc.right - m_Ds.cxDropBtn;
				DrawThemeBackground(m_hTheme, m_DC.GetDC(),
					CP_DROPDOWNBUTTONRIGHT, CBXSR_NORMAL, &rc, NULL);
			}
			break;

			case View::DropDownEdit:
			{

			}
			break;
			}
			
			BitBltPs(&ps, m_DC.GetDC());
			EndPaint(hWnd, &ps);
		}
		return 0;

		case WM_SIZE:
		{
			ECK_GET_SIZE_LPARAM(m_cxClient, m_cyClient, lParam);
			m_DC.ReSize(hWnd, m_cxClient, m_cyClient);
		}
		return 0;

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

		case WM_NOTIFY:
		{
			const auto pnmhdr = (NMHDR*)lParam;
			if (pnmhdr->hwndFrom == m_LB.HWnd)
			{
				switch (pnmhdr->code)
				{
				case NM_LBN_LBTNDOWN:
					SetFocus(hWnd);
					return 0;
				case NM_LBN_DISMISS:
					DismissList();
					return 0;
				default:
				{
					pnmhdr->hwndFrom = hWnd;
					return SendMessageW(GetParent(hWnd), uMsg, wParam, lParam);
				}
				}
			}
		}
		break;

		case WM_THEMECHANGED:
		{
			CloseThemeData(m_hTheme);
			m_hTheme = OpenThemeData(hWnd, L"ComboBox");
			UpdateThemeInfo();
		}
		break;
		}
		return CWnd::OnMsg(hWnd, uMsg, wParam, lParam);
	}

	ECK_CWND_CREATE;
	HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, HMENU hMenu, PCVOID pData = NULL) override
	{
		return IntCreate(dwExStyle, WCN_COMBOBOXNEW, pszText, dwStyle,
			x, y, cx, cy, hParent, hMenu, g_hInstance, this);
	}

	void DropList()
	{
		if (m_bDrop)
			return;
		m_bDrop = TRUE;
		SetFocus(HWnd);
		Redraw();
		RECT rc;
		GetWindowRect(HWnd, &rc);
		SetWindowPos(m_LB.HWnd, NULL,
			rc.left, rc.bottom,
			m_cxLB < 0 ? m_Ds.cxDefLB : m_cxLB,
			m_cyLB < 0 ? m_Ds.cxDefLB : m_cyLB,
			SWP_NOZORDER | SWP_SHOWWINDOW | SWP_NOACTIVATE);
		m_LB.EnterTrack();
	}

	void DismissList()
	{
		if (!m_bDrop)
			return;
		m_bDrop = FALSE;
		Redraw();
		m_LB.LeaveTrack();
		m_LB.Show(SW_HIDE);
	}

	EckInline void SetItemCount(int c)
	{
		m_LB.SetItemCount(c);
	}

	EckInline auto& GetListBox() { return m_LB; }
};
ECK_NAMESPACE_END