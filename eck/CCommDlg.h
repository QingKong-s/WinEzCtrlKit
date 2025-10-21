#pragma once
#include "CDialog.h"
#include "Utility2.h"

#include <commdlg.h>

ECK_NAMESPACE_BEGIN
struct TASKDIALOGCTX
{
	TASKDIALOGCONFIG* ptdc;
	int iRadioButton;
	BOOL bChecked;
	HRESULT hr;
};

class CTaskDialog :public CDialog
{
protected:
	TASKDIALOGCTX* m_pParam{};
	PFTASKDIALOGCALLBACK m_pfnRealCallBack{};
	LONG_PTR m_lRealRefData{};
	CSignal<Intercept_T, HRESULT, HWND, UINT, WPARAM, LPARAM > m_CallbackSig{};

	static LRESULT CALLBACK EckTdLinkParentSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
		UINT_PTR uSubclassId, DWORD_PTR lRefData)
	{
		if (ShouldAppsUseDarkMode())
			switch (uMsg)
			{
			case WM_PAINT:
			{
				PAINTSTRUCT ps;
				BeginPaint(hWnd, &ps);
				FillRect(ps.hdc, &ps.rcPaint, GetStockBrush(BLACK_BRUSH));
				EndPaint(hWnd, &ps);
			}
			return 0;
			case WM_CTLCOLORSTATIC:
				SetTextColor((HDC)wParam, GetThreadCtx()->crDefText);
				SetBkColor((HDC)wParam, GetThreadCtx()->crDefBkg);
				return (LRESULT)GetStockBrush(BLACK_BRUSH);// 防止展开时出现闪烁的白色部分
			}
		return DefSubclassProc(hWnd, uMsg, wParam, lParam);
	}

	static HRESULT CALLBACK EckTdCallBack(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LONG_PTR lRefData)
	{
		auto p = (CTaskDialog*)lRefData;
		switch (uMsg)
		{
		case TDN_NAVIGATED:
		case TDN_DIALOG_CONSTRUCTED:
		{
			EnumChildWindows(hWnd, [](HWND hWnd, LPARAM lParam)->BOOL
				{
					if (_wcsicmp(CWnd(hWnd).GetClsName().Data(), WC_LINK) == 0)
						SetWindowSubclass(GetParent(hWnd), EckTdLinkParentSubclassProc, 0, 0);
					return TRUE;
				}, 0);
		}
		break;
		}

		return p->OnTdNotify(hWnd, uMsg, wParam, lParam);
	}
public:
	ECK_RTTI(CTaskDialog);
	// TASKDIALOGCTX*
	INT_PTR DlgBox(HWND hParent,
		_In_reads_bytes_(sizeof(TASKDIALOGCTX)) void* pData) override
	{
		auto pCtx = (TASKDIALOGCTX*)pData;
		const auto ptdc = pCtx->ptdc;

		m_pfnRealCallBack = ptdc->pfCallback;
		m_lRealRefData = ptdc->lpCallbackData;
		ptdc->pfCallback = EckTdCallBack;
		ptdc->lpCallbackData = (LONG_PTR)this;

		int iButton{};
		BeginCbtHook(this);
		pCtx->hr = TaskDialogIndirect(ptdc, &iButton,
			&pCtx->iRadioButton, &pCtx->bChecked);
		return iButton;
	}

	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		switch (uMsg)
		{
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint(hWnd, &ps);
			SetDCBrushColor(ps.hdc, GetThreadCtx()->crDefBkg);
			FillRect(ps.hdc, &ps.rcPaint, GetStockBrush(DC_BRUSH));
			EndPaint(hWnd, &ps);
		}
		return 0;
		}
		return CDialog::OnMsg(hWnd, uMsg, wParam, lParam);
	}

	EckInline BOOL EndDlg(INT_PTR nResult) override { return FALSE; }

	virtual HRESULT OnTdNotify(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if (m_pfnRealCallBack)
			return m_pfnRealCallBack(hWnd, uMsg, wParam, lParam, m_lRealRefData);
		else
			return m_CallbackSig.Emit(hWnd, uMsg, wParam, lParam);
	}

	EckInline void ClickButton(int iID)
	{
		SendMsg(TDM_CLICK_BUTTON, iID, 0);
	}

	EckInline void ClickRadioButton(int iID)
	{
		SendMsg(TDM_CLICK_RADIO_BUTTON, iID, 0);
	}

	EckInline void ClickCheckBox(BOOL bChecked, BOOL bSetFocus = FALSE)
	{
		SendMsg(TDM_CLICK_VERIFICATION, bChecked, bSetFocus);
	}

	EckInline void EnableButton(int iID, BOOL bEnable)
	{
		SendMsg(TDM_ENABLE_BUTTON, iID, bEnable);
	}

	EckInline void EnableRadioButton(int iID, BOOL bEnable)
	{
		SendMsg(TDM_ENABLE_RADIO_BUTTON, iID, bEnable);
	}

	EckInline void NavigatePage(_In_ TASKDIALOGCONFIG* pInfo)
	{
		SendMsg(TDM_NAVIGATE_PAGE, 0, (LPARAM)pInfo);
	}

	EckInline void NavigatePage()
	{
		SendMsg(TDM_NAVIGATE_PAGE, 0, (LPARAM)m_pParam->ptdc);
	}

	EckInline void SetShieldIcon(int iID, BOOL bShieldIcon)
	{
		SendMsg(TDM_SET_BUTTON_ELEVATION_REQUIRED_STATE, iID, bShieldIcon);
	}

	/// <summary>
	/// 置元素文本。
	/// 窗口布局可能会变化以适应新文本
	/// </summary>
	/// <param name="uType">元素类型，TDE_常量</param>
	/// <param name="pszText">文本</param>
	EckInline void SetElementText(UINT uType, _In_z_ PCWSTR pszText)
	{
		SendMsg(TDM_SET_ELEMENT_TEXT, uType, (LPARAM)pszText);
	}

	EckInline void PBSetMarqueeShowing(BOOL bShowing)
	{
		SendMsg(TDM_SET_MARQUEE_PROGRESS_BAR, bShowing, 0);
	}

	EckInline void PBSetMarquee(BOOL bMarquee, UINT uAnimationGap = 0u)
	{
		SendMsg(TDM_SET_PROGRESS_BAR_MARQUEE, bMarquee, uAnimationGap);
	}

	EckInline void PBSetPos(int iPos)
	{
		SendMsg(TDM_SET_PROGRESS_BAR_POS, iPos, 0);
	}

	EckInline void PBSetRange(int iMin, int iMax)
	{
		SendMsg(TDM_SET_PROGRESS_BAR_POS, 0, MAKELPARAM(iMin, iMax));
	}

	/// <summary>
	/// 进度条_置状态
	/// </summary>
	/// <param name="uState">状态，PBST_常量</param>
	EckInline void PBSetState(UINT uState)
	{
		SendMsg(TDM_SET_PROGRESS_BAR_STATE, uState, 0);
	}

	/// <summary>
	/// 更新元素文本。
	/// 窗口布局不会变化，因此新文本必须短于旧文本
	/// </summary>
	/// <param name="uType">元素类型，TDE_常量</param>
	/// <param name="pszText">文本</param>
	EckInline void UpdateElementText(UINT uType, _In_z_ PCWSTR pszText)
	{
		SendMsg(TDM_UPDATE_ELEMENT_TEXT, uType, (LPARAM)pszText);
	}

	/// <summary>
	/// 更新图标
	/// </summary>
	/// <param name="uType">元素类型，TDIE_ICON_常量</param>
	/// <param name="Icon">图标，可为HICON或PCWSTR，取决于创建对话框时的设置</param>
	EckInline void UpdateIcon(UINT uType, HICON hIcon)
	{
		SendMsg(TDM_UPDATE_ICON, uType, (LPARAM)hIcon);
	}

	EckInline void UpdateIcon(UINT uType, PCWSTR pszIcon)
	{
		SendMsg(TDM_UPDATE_ICON, uType, (LPARAM)pszIcon);
	}

	EckInline constexpr auto& GetCallbackSignal() { return m_CallbackSig; }
};
ECK_RTTI_IMPL_BASE_INLINE(CTaskDialog, CDialog);

class CColorDialog :public CDialog
{
public:
	ECK_RTTI(CColorDialog);

	static UINT s_uMsgSetRgb;
protected:
	COLORREF m_crCust[16]{};
public:
	// CHOOSECOLORW*
	INT_PTR DlgBox(HWND hParent,
		_In_reads_bytes_(sizeof(CHOOSECOLORW)) void* pData = nullptr) override
	{
		auto pcc = (CHOOSECOLORW*)pData;
		if (!pcc->lpCustColors)
			pcc->lpCustColors = m_crCust;
		BeginCbtHook(this);
		return ChooseColorW((CHOOSECOLORW*)pData);
	}

	INT_PTR DlgBox(HWND hParent, COLORREF crInit = 0,
		DWORD dwFlags = 0, _Inout_opt_count_(16) COLORREF* pcrCust = nullptr)
	{
		CHOOSECOLORW cc{ sizeof(cc) };
		cc.hwndOwner = hParent;
		cc.rgbResult = crInit;
		cc.Flags = dwFlags;
		cc.lpCustColors = (pcrCust ? pcrCust : m_crCust);
		return DlgBox(hParent, &cc);
	}

	void OnOk(HWND hCtrl) override {}
	void OnCancel(HWND hCtrl) override {}

	EckInline void SetRGB(COLORREF cr) const { SendMsg(s_uMsgSetRgb, 0, cr); }
};
ECK_RTTI_IMPL_BASE_INLINE(CColorDialog, CDialog);
inline UINT CColorDialog::s_uMsgSetRgb = RegisterWindowMessageW(SETRGBSTRINGW);

class CFontDialog :public CDialog
{
public:
	ECK_RTTI(CFontDialog);
public:
	// CHOOSEFONTW*
	INT_PTR DlgBox(HWND hParent,
		_In_reads_bytes_(sizeof(CHOOSEFONTW)) void* pData = nullptr) override
	{
		BeginCbtHook(this);
		return ChooseFontW((CHOOSEFONTW*)pData);
	}

	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		switch (uMsg)
		{
		case WM_DRAWITEM:
		{
			const auto* const pdis = (DRAWITEMSTRUCT*)lParam;
			switch (pdis->CtlID)
			{
			case 0x470:// 字体
			case 0x471:// 字形
			case 0x472:// 大小
			case 0x473:// 颜色
			case 0x474:// 脚本
			{
				if (!ShouldAppsUseDarkMode())
					break;
				if (pdis->itemState & ODS_FOCUS)
					DrawFocusRect(pdis->hDC, &pdis->rcItem);

				const auto cch = (int)SendMessageW(pdis->hwndItem,
					CB_GETLBTEXTLEN, pdis->itemID, 0);
				if (cch == CB_ERR)
					return TRUE;
				const auto pszBuf = (PWSTR)_malloca((cch + 1) * sizeof(WCHAR));
				SendMessageW(pdis->hwndItem, CB_GETLBTEXT,
					pdis->itemID, (LPARAM)pszBuf);
				const auto* const ptc = GetThreadCtx();

				BOOL bDelFont{};
				HFONT hFont{};
				if (pdis->CtlID == 0x470)
				{
					LOGFONTW lf;
					GetObjectW(HFont, sizeof(lf), &lf);
					const auto nDefHeight = lf.lfHeight;
					if (SUCCEEDED(MatchLogFontFromFamilyName(pszBuf, lf)) &&
						lf.lfCharSet != SYMBOL_CHARSET)
					{
						lf.lfHeight = nDefHeight;
						lf.lfWidth = 0;
						hFont = CreateFontIndirectW(&lf);
						if (hFont)
							bDelFont = TRUE;
#ifdef _DEBUG
						else
							EckDbgPrintFmt(L"CFontDialog::OnMsg: %s not found", pszBuf);
#endif// _DEBUG
					}
				}
				if (!hFont)
					hFont = HFont;
				const auto hOldFont = (HFONT)SelectObject(pdis->hDC, hFont);
				if (pdis->itemState & ODS_SELECTED)
				{
					SetTextColor(pdis->hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
					SetDCBrushColor(pdis->hDC, GetSysColor(COLOR_HIGHLIGHT));
				}
				else
				{
					SetTextColor(pdis->hDC, ptc->crDefText);
					SetDCBrushColor(pdis->hDC, ptc->crDefBkg);
				}
				SetBkMode(pdis->hDC, TRANSPARENT);
				const auto iDpi = GetDeviceCaps(pdis->hDC, LOGPIXELSX);
				const auto cxBorder = DaGetSystemMetrics(SM_CXBORDER, iDpi);
				int x = pdis->rcItem.left + cxBorder;
				if (pdis->CtlID == 0x473)// 让出颜色块位置
					x += ((pdis->rcItem.bottom - pdis->rcItem.top) * 2 + cxBorder);
				RECT rc{ pdis->rcItem };
				rc.left += x;
				FillRect(pdis->hDC, &pdis->rcItem, GetStockBrush(DC_BRUSH));
				DrawTextW(pdis->hDC, pszBuf, cch, &rc,
					DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX);
				if (pdis->CtlID == 0x473)// 画颜色块
				{
					const auto cyBorder = DaGetSystemMetrics(SM_CYBORDER, iDpi);
					SetDCPenColor(pdis->hDC, ptc->crDefText);
					SetDCBrushColor(pdis->hDC, (COLORREF)
						SendMessageW(pdis->hwndItem, CB_GETITEMDATA,
							pdis->itemID, 0));
					SelectObject(pdis->hDC, GetStockObject(DC_PEN));
					SelectObject(pdis->hDC, GetStockObject(DC_BRUSH));
					Rectangle(pdis->hDC,
						pdis->rcItem.left + cxBorder,
						pdis->rcItem.top + cyBorder,
						pdis->rcItem.left + (pdis->rcItem.bottom - pdis->rcItem.top) * 2,
						pdis->rcItem.bottom - cyBorder);
				}
				SelectObject(pdis->hDC, hOldFont);
				_freea(pszBuf);
				if (bDelFont)
					DeleteObject(hFont);
			}
			return TRUE;
			}
		}
		break;

		case WM_PAINT:
		{
			if (!ShouldAppsUseDarkMode())
				break;
			PAINTSTRUCT ps;
			BeginPaint(hWnd, &ps);
			const auto hBTSample = GetDlgItem(hWnd, 0x431);
			RECT rc;
			GetWindowRect(hBTSample, &rc);
			ScreenToClient(hWnd, &rc);
			if (IsRectsIntersect(ps.rcPaint, rc))
			{
				const auto* const ptc = GetThreadCtx();
				const auto hCBBColor = GetDlgItem(hWnd, 0x473);
				COLORREF cr{ CLR_INVALID };
				if (GetWindowLongPtrW(hCBBColor, GWL_STYLE) & WS_VISIBLE)
				{
					const auto idx = (int)SendMessageW(hCBBColor, CB_GETCURSEL, 0, 0);
					if (idx != CB_ERR)
						cr = (COLORREF)SendMessageW(hCBBColor, CB_GETITEMDATA,
							SendMessageW(hCBBColor, CB_GETCURSEL, 0, 0), 0);
				}
				if (cr == CLR_INVALID)
					cr = ptc->crDefText;
				SetTextColor(ps.hdc, cr);
				SetDCBrushColor(ps.hdc, ptc->crDefBkg);
				FillRect(ps.hdc, &rc, GetStockBrush(DC_BRUSH));
				SetBkMode(ps.hdc, TRANSPARENT);

				LOGFONTW lf;
				GetLogFont(&lf);
				lf.lfEscapement = 0;
				const auto hFont = CreateFontIndirectW(&lf);
				SelectObject(ps.hdc, hFont);

				const auto svText = GetResourceStringForCurrentLocale(
					0x700 + lf.lfCharSet, GetModuleHandleW(L"comdlg32.dll"));
				DrawTextW(ps.hdc, svText.data(), (int)svText.size(), &rc,
					DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_NOPREFIX);
				DeleteObject(hFont);
			}
			EndPaint(hWnd, &ps);
		}
		return 0;
		}
		return __super::OnMsg(hWnd, uMsg, wParam, lParam);
	}

	void OnOk(HWND hCtrl) override {}
	void OnCancel(HWND hCtrl) override {}

	EckInline void GetLogFont(_Out_ LOGFONTW* plf) const
	{
		SendMsg(WM_CHOOSEFONT_GETLOGFONT, 0, (LPARAM)plf);
	}
	EckInline void SetLogFont(_In_ LOGFONTW* plf)
	{
		SendMsg(WM_CHOOSEFONT_SETLOGFONT, 0, (LPARAM)plf);
	}
	EckInline void SetFlags(_In_ CHOOSEFONTW* pcf)
	{
		SendMsg(WM_CHOOSEFONT_SETFLAGS, 0, (LPARAM)pcf);
	}
	EckInline void SetFlags(DWORD dwFlags)
	{
		CHOOSEFONTW cf{ sizeof(cf) };
		cf.Flags = dwFlags;
		SetFlags(&cf);
	}
};
ECK_RTTI_IMPL_BASE_INLINE(CFontDialog, CDialog);

EckInline int MsgBox(_In_z_ PCWSTR pszText,
	PCWSTR pszCaption = L"", UINT uType = 0, HWND hParent = nullptr)
{
	return MessageBoxW(hParent, pszText, pszCaption, uType);
}

EckInline int MsgBox(const CRefStrW& rs,
	PCWSTR pszCaption = L"", UINT uType = 0, HWND hParent = nullptr)
{
	return MessageBoxW(hParent, rs.Data(), pszCaption, uType);
}
ECK_NAMESPACE_END