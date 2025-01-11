#pragma once
#include "CDialog.h"
#include "CEditExt.h"
#include "CButton.h"
#include "SystemHelper.h"
#include "ShellHelper.h"

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
	CEditExt::InputMode eInputMode;
};

class CInputBox :public CDialogNew
{
public:
	ECK_RTTI(CInputBox);
	ECK_CWND_SINGLEOWNER(CInputBox);
private:
	INPUTBOXOPT* m_pOpt{};

	HFONT m_hFont{};

	CEditExt m_ED{};
	CButton m_BTOk{};
	CButton m_BTCancel{};

	HTHEME m_hTheme{};

	int m_cyMainTip{};
	int m_cyTip{};
	int m_cxClient{},
		m_cyClient{};
	int m_cySingleLineText{};

	int m_iDpi{ USER_DEFAULT_SCREEN_DPI };
	ECK_DS_BEGIN(DPIS)
		ECK_DS_ENTRY(TextPadding, 6)
		ECK_DS_ENTRY(Margin, 10)
		ECK_DS_ENTRY(cxBT, 80)
		ECK_DS_ENTRY(cyBT, 32)
		;
	ECK_DS_END_VAR(m_Ds);

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
			(m_pOpt->uFlags & IPBF_MULTILINE) ?
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
		DrawThemeTextEx(m_hTheme, hCDC, TEXT_MAININSTRUCTION, 0, m_pOpt->pszMainTip, -1,
			DT_NOPREFIX | DT_EDITCONTROL | DT_WORDBREAK | DT_CALCRECT, &rc, &dttops);
		m_cyMainTip = rc.bottom;

		rc = { 0,0,m_cxClient - m_Ds.Margin * 2,m_cyClient };
		DrawThemeTextEx(m_hTheme, hCDC, TEXT_BODYTEXT, 0, m_pOpt->pszTip, -1,
			DT_NOPREFIX | DT_EDITCONTROL | DT_WORDBREAK | DT_CALCRECT, &rc, &dttops);
		m_cyTip = rc.bottom;
		DeleteDC(hCDC);
	}

	void OnCopy()
	{
		const BOOL bChs = (LANGIDFROMLCID(GetThreadLocale()) ==
			MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED));

		constexpr PCWSTR pszDiv = L"---------------------------\r\n";
		constexpr PCWSTR pszDiv2 = L"\r\n---------------------------\r\n";
		constexpr PCWSTR pszDiv3 = L"\r\n---------------------------";

		CRefStrW rs{};
		rs.PushBack(pszDiv);
		rs.PushBack(m_pOpt->pszTitle);
		rs.PushBack(pszDiv2);
		rs.PushBack(m_pOpt->pszMainTip);
		rs.PushBack(pszDiv2);
		rs.PushBack(m_pOpt->pszTip);
		rs.PushBack(pszDiv2);
		rs.PushBack(m_ED.GetText());
		rs.PushBack(pszDiv2);
		if (bChs)
			rs.PushBack(L"确认输入    取消");
		else
			rs.PushBack(L"OK    Cancel");
		rs.PushBack(pszDiv3);
		SetClipboardString(rs.Data(), rs.Size(), HWnd);
	}
public:
	BOOL PreTranslateMessage(const MSG& Msg) override
	{
		if (Msg.message == WM_KEYDOWN)
		{
			if (Msg.wParam == 'C' && (GetAsyncKeyState(VK_CONTROL) & 0x8000))
			{
				if (GetFocus() == m_ED.HWnd)
				{
					int iSelBegin, iSelEnd;
					m_ED.GetSel(&iSelBegin, &iSelEnd);
					if (iSelBegin != iSelEnd)
						goto NoCopy;
				}
				OnCopy();
				MessageBeep(MB_OK);
			NoCopy:;
			}
		}
		return __super::PreTranslateMessage(Msg);
	}

	BOOL OnInitDialog(HWND hDlg, HWND hFocus, LPARAM lParam) override
	{
		UpdateDpi(GetDpi(hDlg));
		m_hFont = CreateDefFont(m_iDpi);
		UpdateTextMetrics();

		SetExplorerTheme();
		m_hTheme = OpenThemeData(hDlg, L"TextStyle");

		DWORD dwEDStyle = WS_TABSTOP | WS_GROUP | WS_VISIBLE |
			WS_CHILD | ES_WANTRETURN;
		if (m_pOpt->uFlags & IPBF_MULTILINE)
		{
			m_ED.SetMultiLine(TRUE);
			dwEDStyle |= WS_VSCROLL;
		}
		m_ED.Create(m_pOpt->pszInitContent, dwEDStyle, WS_EX_CLIENTEDGE,
			0, 0, 0, 0, hDlg, 0);
		if (m_pOpt->eInputMode != CEditExt::InputMode::ReadOnly)
			m_ED.SetInputMode(m_pOpt->eInputMode);

		const BOOL bChs = (LANGIDFROMLCID(GetThreadLocale()) ==
			MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED));
		m_BTOk.Create(bChs ? L"确认输入(&O)" : L"&OK",
			WS_TABSTOP | WS_GROUP | WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 0,
			0, 0, 0, 0, hDlg, IDOK);
		m_BTCancel.Create(bChs ? L"取消(&C)" : L"&Cancel",
			WS_TABSTOP | WS_CHILD | WS_VISIBLE, 0,
			0, 0, 0, 0, hDlg, IDCANCEL);

		RECT rcClient{};
		GetClientRect(hDlg, &rcClient);
		m_cxClient = rcClient.right;
		m_cyClient = rcClient.bottom;

		UpdateThemeSize();
		if (m_pOpt->uFlags & IPBF_FIXWIDTH)
		{
			RECT rc{ 0,0, rcClient.right,
				m_cyMainTip + m_cyTip + m_Ds.TextPadding * 3 + m_Ds.cyBT + m_Ds.Margin * 2 +
				(IsBitSet(m_pOpt->uFlags,IPBF_MULTILINE) ?
					DpiScale(100, m_iDpi) : m_cySingleLineText)
			};
			DaAdjustWindowRectEx(&rc, Style, FALSE, ExStyle, m_iDpi);

			POINT pt;
			if (IsBitSet(m_pOpt->uFlags, IPBF_CENTERPARENT))
				pt = CalcCenterWndPos(HWND(lParam), rc.right - rc.left, rc.bottom - rc.top);
			else if (IsBitSet(m_pOpt->uFlags, IPBF_CENTERSCREEN))
				pt = CalcCenterWndPos(nullptr, rc.right - rc.left, rc.bottom - rc.top);
			else
				pt = { m_pOpt->x,m_pOpt->y };
			SetWindowPos(hDlg, nullptr, pt.x, pt.y, rc.right - rc.left, rc.bottom - rc.top,
				SWP_NOZORDER | SWP_NOACTIVATE);
		}
		m_ED.SelAll();

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
		case WM_PRINTCLIENT:
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint(hWnd, wParam, ps);
			const auto* const ptc = GetThreadCtx();
			SetDCBrushColor(ps.hdc, ptc->crDefBkg);
			FillRect(ps.hdc, &ps.rcPaint, GetStockBrush(DC_BRUSH));
			RECT rc
			{
				m_Ds.Margin,
				m_Ds.Margin,
				m_cxClient - m_Ds.Margin,
				m_cyClient
			};
			DrawThemeTextEx(m_hTheme, ps.hdc, TEXT_MAININSTRUCTION, 0, m_pOpt->pszMainTip, -1,
				DT_NOPREFIX | DT_EDITCONTROL | DT_WORDBREAK | DT_NOCLIP, &rc, nullptr);

			rc.top += (m_cyMainTip + m_Ds.TextPadding);
			DrawThemeTextEx(m_hTheme, ps.hdc, TEXT_BODYTEXT, 0, m_pOpt->pszTip, -1,
				DT_NOPREFIX | DT_EDITCONTROL | DT_WORDBREAK | DT_NOCLIP, &rc, nullptr);
			EndPaint(hWnd, wParam, ps);
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

		case WM_COPY:
			OnCopy();
			return TRUE;

		case WM_DESTROY:
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
			EckDbgPrint(L"** WARNING ** CInputBox::DlgBox: pData is nullptr.");
			EckDbgBreak();
			return FALSE;
		}
		m_pOpt = (INPUTBOXOPT*)pData;
		const UINT uDlgFlags = (m_pOpt->uFlags & IPBF_FIXWIDTH) ?
			0 : (m_pOpt->uFlags & IPBF_DLGFLAGSMASK);

		IntCreateModalDlg(0, WCN_DLG, m_pOpt->pszTitle,
			WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_CLIPCHILDREN |
			((m_pOpt->uFlags & IPBF_RESIZEABLE) ? (WS_SIZEBOX | WS_MAXIMIZEBOX) : 0),
			m_pOpt->x,
			m_pOpt->y,
			m_pOpt->cx ? m_pOpt->cx : DpiScale(480, GetMonitorDpi(GetOwnerMonitor(hParent))),
			m_pOpt->cy,
			hParent, nullptr, g_hInstance, hParent, uDlgFlags);
		return m_iResult;
	}
};
ECK_RTTI_IMPL_BASE_INLINE(CInputBox, CDialogNew);

inline BOOL InputBox(CRefStrW& rs, HWND hParent, PCWSTR pszMainTip, PCWSTR pszTip = nullptr,
	PCWSTR pszInitContent = nullptr, PCWSTR pszTitle = nullptr, BOOL bMultiLine = FALSE,
	CEditExt::InputMode eInputMode = CEditExt::InputMode::Normal)
{
	INPUTBOXOPT Opt{};
	Opt.pszMainTip = pszMainTip;
	Opt.pszTip = pszTip;
	Opt.pszInitContent = pszInitContent;
	Opt.pszTitle = pszTitle;
	Opt.uFlags = IPBF_CENTERPARENT | IPBF_FIXWIDTH | (bMultiLine ? IPBF_MULTILINE : 0);
	Opt.eInputMode = eInputMode;
	CInputBox ib{};
	if (ib.DlgBox(hParent, &Opt))
	{
		rs = std::move(Opt.rsInput);
		return TRUE;
	}
	return FALSE;
}
ECK_NAMESPACE_END