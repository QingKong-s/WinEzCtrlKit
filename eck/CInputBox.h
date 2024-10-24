/*
* WinEzCtrlKit Library
*
* CInputBox.h ： 输入框
*
* Copyright(C) 2023-2024 QingKong
*/
#pragma once
#include "CDialog.h"
#include "CEditExt.h"
#include "CButton.h"
#include "SystemHelper.h"

ECK_NAMESPACE_BEGIN
enum :UINT
{
	// 相对屏幕居中
	IPBF_CENTERSCREEN = (1u << 0),
	// 相对父窗口居中
	IPBF_CENTERPARENT = (1u << 1),
	// 仅供内部使用
	IPBF_DLGFLAGSMASK = IPBF_CENTERSCREEN | IPBF_CENTERPARENT,
	// 多行输入
	IPBF_MULTILINE = (1u << 2),
	// 窗口大小可调
	IPBF_RESIZEABLE = (1u << 3),
	// 调用者给定宽度，由输入框计算高度
	IPBF_FIXWIDTH = (1u << 4),
};

struct INPUTBOXOPT
{
	PCWSTR pszTitle;
	PCWSTR pszInitContent;
	PCWSTR pszMainTip;
	PCWSTR pszTip;
	CRefStrW rsInput;
	UINT uFlags;
	int x;
	int y;
	int cx;
	int cy;
	CEditExt::InputMode uInputMode;
};

class CInputBox final :public CDialogNew
{
public:
	ECK_RTTI(CInputBox);
private:
	CEditExt m_ED{};
	CPushButton m_BTOk{};
	CPushButton m_BTCancel{};

	HTHEME m_hTheme = nullptr;

	int m_cyMainTip = 0;
	int m_cyTip = 0;
	int m_cxClient = 0,
		m_cyClient = 0;
	int m_cySingleLineText = 0;

	INPUTBOXOPT* m_pOpt = nullptr;

	HFONT m_hFont = nullptr;

	int m_iDpi = USER_DEFAULT_SCREEN_DPI;
	ECK_DS_BEGIN(DPIS)
		ECK_DS_ENTRY(TextPadding, 6)
		ECK_DS_ENTRY(Margin, 10)
		//ECK_DS_ENTRY(cyED, 26)
		ECK_DS_ENTRY(cyMultiLineED, 100)
		ECK_DS_ENTRY(cxBT, 80)
		ECK_DS_ENTRY(cyBT, 32)
		;
	ECK_DS_END_VAR(m_Ds);

	enum
	{
		IDC_ED = 101,
	};

	void UpdateTextMetrics()
	{
		HDC hCDC = CreateCompatibleDC(nullptr);
		SelectObject(hCDC, m_hFont);
		TEXTMETRICW tm;
		GetTextMetricsW(hCDC, &tm);
		m_cySingleLineText = tm.tmHeight + m_Ds.TextPadding * 2;
		DeleteDC(hCDC);
	}

	void UpdateDpi(int iDpi)
	{
		m_iDpi = iDpi;
		UpdateDpiSize(m_Ds, iDpi);
	}

	void UpdateDpiCtrlSize()
	{
		HDWP hDwp = BeginDeferWindowPos(3);
		int y = m_Ds.Margin + m_Ds.TextPadding * 2 + m_cyMainTip + m_cyTip;
		const int cyED = (
			IsBitSet(m_pOpt->uFlags, IPBF_MULTILINE) ?
			m_cyClient - y - m_Ds.TextPadding - m_Ds.cyBT - m_Ds.Margin :
			m_cySingleLineText);
		hDwp = DeferWindowPos(hDwp, m_ED.HWnd, nullptr,
			m_Ds.Margin,
			y,
			m_cxClient - m_Ds.Margin * 2,
			cyED,
			SWP_NOZORDER | SWP_NOACTIVATE);
		y = m_cyClient - m_Ds.Margin - m_Ds.cyBT;
		hDwp = DeferWindowPos(hDwp, m_BTCancel.HWnd, nullptr,
			m_cxClient - m_Ds.Margin - m_Ds.cxBT,
			y,
			m_Ds.cxBT,
			m_Ds.cyBT,
			SWP_NOZORDER | SWP_NOACTIVATE);
		hDwp = DeferWindowPos(hDwp, m_BTOk.HWnd, nullptr,
			m_cxClient - m_Ds.Margin - m_Ds.cxBT * 2 - m_Ds.TextPadding,
			y,
			m_Ds.cxBT,
			m_Ds.cyBT,
			SWP_NOZORDER | SWP_NOACTIVATE);

		EndDeferWindowPos(hDwp);
	}

	void UpdateThemeSize()
	{
		HDC hCDC = CreateCompatibleDC(nullptr);
		RECT rc{ 0,0,m_cxClient - m_Ds.Margin * 2,m_cyClient };
		DTTOPTS dttops{ sizeof(DTTOPTS),DTT_CALCRECT };
		auto hr = DrawThemeTextEx(m_hTheme, hCDC, TEXT_MAININSTRUCTION, 0, m_pOpt->pszMainTip, -1,
			DT_NOPREFIX | DT_EDITCONTROL | DT_WORDBREAK | DT_CALCRECT, &rc, &dttops);
		m_cyMainTip = rc.bottom;

		rc = { 0,0,m_cxClient - m_Ds.Margin * 2,m_cyClient };
		DrawThemeTextEx(m_hTheme, hCDC, TEXT_BODYTEXT, 0, m_pOpt->pszTip, -1,
			DT_NOPREFIX | DT_EDITCONTROL | DT_WORDBREAK | DT_CALCRECT, &rc, &dttops);
		m_cyTip = rc.bottom;
		DeleteDC(hCDC);
	}
public:
	ECK_CWND_SINGLEOWNER(CInputBox);

	BOOL OnInitDialog(HWND hDlg, HWND hFocus, LPARAM lParam) override
	{
		UpdateDpi(GetDpi(hDlg));
		m_hFont = CreateDefFont(m_iDpi);
		UpdateTextMetrics();

		SetExplorerTheme();
		m_hTheme = OpenThemeData(hDlg, L"TextStyle");

		DWORD dwEDStyle;
		if (IsBitSet(m_pOpt->uFlags, IPBF_MULTILINE))
			dwEDStyle = ES_MULTILINE | WS_VSCROLL | ES_AUTOVSCROLL | ES_WANTRETURN;
		else
			dwEDStyle = ES_AUTOHSCROLL;

		m_ED.Create(m_pOpt->pszInitContent,
			WS_TABSTOP | WS_GROUP | WS_VISIBLE | WS_CHILD | dwEDStyle, WS_EX_CLIENTEDGE,
			0, 0, 0, 0, hDlg, IDC_ED);
		if (m_pOpt->uInputMode != CEditExt::InputMode::ReadOnly)
			m_ED.SetInputMode(m_pOpt->uInputMode);
		m_ED.SetExplorerTheme();
		m_BTOk.Create(L"确认输入(&O)", WS_TABSTOP | WS_GROUP | WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 0,
			0, 0, 0, 0, hDlg, IDOK);
		m_BTCancel.Create(L"取消(&C)", WS_TABSTOP | WS_CHILD | WS_VISIBLE, 0,
			0, 0, 0, 0, hDlg, IDCANCEL);

		RECT rcClient{};
		GetClientRect(hDlg, &rcClient);
		m_cxClient = rcClient.right;
		m_cyClient = rcClient.bottom;

		UpdateThemeSize();
		if (IsBitSet(m_pOpt->uFlags, IPBF_FIXWIDTH))
		{
			RECT rc{ 0,0, rcClient.right,
				m_cyMainTip + m_cyTip + m_Ds.TextPadding * 3 + m_Ds.cyBT + m_Ds.Margin * 2 +
				(IsBitSet(m_pOpt->uFlags,IPBF_MULTILINE) ? m_Ds.cyMultiLineED : m_cySingleLineText)
			};
#if _WIN32_WINNT >= _WIN32_WINNT_WIN10
			AdjustWindowRectExForDpi(&rc, Style, FALSE, ExStyle, m_iDpi);
#else
			AdjustWindowRectEx(&rc, Style, FALSE, ExStyle);
#endif
			POINT pt;
			if (IsBitSet(m_pOpt->uFlags, IPBF_CENTERPARENT))
				pt = CalcCenterWndPos(GetParent(hDlg), rc.right - rc.left, rc.bottom - rc.top);
			else if (IsBitSet(m_pOpt->uFlags, IPBF_CENTERSCREEN))
				pt = CalcCenterWndPos(nullptr, rc.right - rc.left, rc.bottom - rc.top);
			else
				pt = { m_pOpt->x,m_pOpt->y };
			SetWindowPos(hDlg, nullptr, pt.x, pt.y, rc.right - rc.left, rc.bottom - rc.top, 
				SWP_NOZORDER | SWP_NOACTIVATE);
		}

		UpdateDpiCtrlSize();
		SetFontForWndAndCtrl(hDlg, m_hFont);
		Show(SW_SHOW);
		SetFocus(m_ED.HWnd);
		return FALSE;
	}

	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		switch (uMsg)
		{
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint(hWnd, &ps);
			FillRect(ps.hdc, &ps.rcPaint, GetSysColorBrush(COLOR_WINDOW));
			RECT rc
			{
				m_Ds.Margin,
				m_Ds.Margin,
				m_cxClient - m_Ds.Margin,
				m_cyClient
			};
			DTTOPTS dttops{ sizeof(DTTOPTS) };
			DrawThemeTextEx(m_hTheme, ps.hdc, TEXT_MAININSTRUCTION, 0, m_pOpt->pszMainTip, -1,
				DT_NOPREFIX | DT_EDITCONTROL | DT_WORDBREAK | DT_NOCLIP, &rc, &dttops);

			rc.top += (m_cyMainTip + m_Ds.TextPadding);
			DrawThemeTextEx(m_hTheme, ps.hdc, TEXT_BODYTEXT, 0, m_pOpt->pszTip, -1,
				DT_NOPREFIX | DT_EDITCONTROL | DT_WORDBREAK | DT_NOCLIP, &rc, &dttops);
			EndPaint(hWnd, &ps);
		}
		return 0;

		case WM_SIZE:
		{
			ECK_GET_SIZE_LPARAM(m_cxClient, m_cyClient, lParam);
			UpdateThemeSize();
			UpdateDpiCtrlSize();
		}
		return 0;

		case WM_DPICHANGED:
		{
			const auto prc = (RECT*)lParam;
			SetWindowPos(hWnd, nullptr, 
				prc->left, prc->top, prc->right - prc->left, prc->bottom - prc->top, 
				SWP_NOZORDER | SWP_NOACTIVATE);

			CloseThemeData(m_hTheme);
			m_hTheme = OpenThemeData(hWnd, L"TextStyle");

			const int iOldDpi = m_iDpi;
			UpdateDpi(LOWORD(wParam));
			const auto hOldFont = m_hFont;
			m_hFont = ReCreateFontForDpiChanged(m_hFont, m_iDpi, iOldDpi);
			SetFontForWndAndCtrl(hWnd, m_hFont);
			DeleteObject(hOldFont);

			UpdateTextMetrics();
			UpdateThemeSize();
			UpdateDpiCtrlSize();
		}
		return 0;

		case WM_THEMECHANGED:
			CloseThemeData(m_hTheme);
			m_hTheme = OpenThemeData(hWnd, L"TextStyle");
			Redraw();
			break;

		case WM_NCDESTROY:
		{
			CloseThemeData(m_hTheme);
			m_hTheme = nullptr;
			DeleteObject(m_hFont);
			m_hFont = nullptr;
		}
		break;
		}

		return CDialogNew::OnMsg(hWnd, uMsg, wParam, lParam);
	}

	void OnOk(HWND hCtrl) override
	{
		m_pOpt->rsInput = m_ED.GetText();
		EndDlg(TRUE);
	}

	void OnCancel(HWND hCtrl) override
	{
		m_pOpt->rsInput = CRefStrW{};
		EndDlg(FALSE);
	}

	INT_PTR DlgBox(HWND hParent, void* pData = nullptr) override
	{
		if (!pData)
		{
			EckDbgBreak();
			return FALSE;
		}

		m_pOpt = (INPUTBOXOPT*)pData;
		IntCreateModalDlg(0, WCN_DLG, m_pOpt->pszTitle, 
			WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX |
			(IsBitSet(m_pOpt->uFlags, IPBF_RESIZEABLE) ? (WS_SIZEBOX | WS_MAXIMIZEBOX) : 0),
			m_pOpt->x, m_pOpt->y, m_pOpt->cx ? m_pOpt->cx : DpiScale(400, GetDpi(hParent)), m_pOpt->cy,
			nullptr, nullptr, g_hInstance, pData, m_pOpt->uFlags & IPBF_DLGFLAGSMASK);
		return m_iResult;
	}
};
ECK_RTTI_IMPL_BASE_INLINE(CInputBox, CDialogNew);
ECK_NAMESPACE_END