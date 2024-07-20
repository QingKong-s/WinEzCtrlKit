/*
* WinEzCtrlKit Library
*
* CCommDlg.h ： 通用对话框
*
* Copyright(C) 2023 QingKong
*/
#pragma once
#include "CDialog.h"

#include <vector>

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
	std::vector<TASKDIALOG_BUTTON> m_vBtn{};// 所有按钮
	std::vector<TASKDIALOG_BUTTON> m_vRadioBtn{};// 所有单选按钮

	TASKDIALOGCTX* m_pParam{};
	PFTASKDIALOGCALLBACK m_pfnRealCallBack{};
	LONG_PTR m_lRealRefData{};

	static LRESULT CALLBACK EckTdLinkParentSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
		UINT_PTR uSubclassId, DWORD_PTR lRefData)
	{
		if (ShouldAppUseDarkMode())
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
					SetWindowTheme(hWnd, L"Explorer", NULL);
					return TRUE;
				}, 0);
		}
		break;
		}

		return p->OnTdNotify(hWnd, uMsg, wParam, lParam);
	}
public:
	INT_PTR DlgBox(HWND hParent, void* pData = NULL) override
	{
		auto pCtx = (TASKDIALOGCTX*)pData;
		const auto ptdc = pCtx->ptdc;
		if (!m_vBtn.empty())
		{
			ptdc->cButtons = (int)m_vBtn.size();
			if (ptdc->cButtons)
				ptdc->pButtons = m_vBtn.data();
			else
				ptdc->pButtons = NULL;
		}

		if (!m_vRadioBtn.empty())
		{
			ptdc->cRadioButtons = (int)m_vRadioBtn.size();
			if (ptdc->cRadioButtons)
				ptdc->pRadioButtons = m_vRadioBtn.data();
			else
				ptdc->pRadioButtons = NULL;
		}

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
			return S_OK;
	}

	EckInline void ClickButton(int iID, BOOL bRadioButton = FALSE)
	{
		if (bRadioButton)
			SendMsg(TDM_CLICK_RADIO_BUTTON, iID, 0);
		else
			SendMsg(TDM_CLICK_BUTTON, iID, 0);
	}

	EckInline void ClickCheckBox(BOOL bChecked, BOOL bSetFocus = FALSE)
	{
		SendMsg(TDM_CLICK_VERIFICATION, bChecked, bSetFocus);
	}

	EckInline void ClickButton(BOOL bEnable, int iID, BOOL bRadioButton = FALSE)
	{
		if (bRadioButton)
			SendMsg(TDM_ENABLE_RADIO_BUTTON, iID, bEnable);
		else
			SendMsg(TDM_ENABLE_BUTTON, iID, bEnable);
	}

	EckInline void NavigatePage(TASKDIALOGCONFIG* pInfo)
	{
		SendMsg(TDM_NAVIGATE_PAGE, 0, (LPARAM)pInfo);
	}

	EckInline void NavigatePage()
	{
		SendMsg(TDM_NAVIGATE_PAGE, 0, (LPARAM)m_pParam->ptdc);
	}

	EckInline void SetShieldIcon(BOOL bShieldIcon, int iID)
	{
		SendMsg(TDM_SET_BUTTON_ELEVATION_REQUIRED_STATE, iID, bShieldIcon);
	}

	/// <summary>
	/// 置元素文本。
	/// 窗口布局可能会变化以适应新文本
	/// </summary>
	/// <param name="uType">元素类型，TDE_常量</param>
	/// <param name="pszText">文本</param>
	EckInline void SetElementText(UINT uType, PCWSTR pszText)
	{
		SendMsg(TDM_SET_ELEMENT_TEXT, uType, (LPARAM)pszText);
	}

	EckInline void SetMarqueePBShowing(BOOL bShowing)
	{
		SendMsg(TDM_SET_MARQUEE_PROGRESS_BAR, bShowing, 0);
	}

	EckInline void SetPBMarquee(BOOL bMarquee, UINT uAnimationGap = 0u)
	{
		SendMsg(TDM_SET_PROGRESS_BAR_MARQUEE, bMarquee, uAnimationGap);
	}

	EckInline void SetPBPos(int iPos)
	{
		SendMsg(TDM_SET_PROGRESS_BAR_POS, iPos, 0);
	}

	EckInline void SetPBRange(int iMax, int iMin)
	{
		SendMsg(TDM_SET_PROGRESS_BAR_POS, 0, MAKELPARAM(iMin, iMax));
	}

	/// <summary>
	/// 进度条_置状态
	/// </summary>
	/// <param name="uState">状态，PBST_常量</param>
	EckInline void SetPBState(UINT uState)
	{
		SendMsg(TDM_SET_PROGRESS_BAR_STATE, uState, 0);
	}

	/// <summary>
	/// 更新元素文本。
	/// 窗口布局不会变化，因此新文本必须短于旧文本
	/// </summary>
	/// <param name="uType">元素类型，TDE_常量</param>
	/// <param name="pszText">文本</param>
	EckInline void UpdateElementText(UINT uType, PCWSTR pszText)
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

	EckInline void ClearButtonVector() { m_vBtn.clear(); }

	EckInline void ClearRadioButtonVector() { m_vRadioBtn.clear(); }
};

class CColorDialog :public CDialog
{
public:
	static UINT s_uMsgSetRgb;
protected:
	COLORREF m_crCust[16]{};
public:
	INT_PTR DlgBox(HWND hParent, void* pData = NULL) override
	{
		auto pcc = (CHOOSECOLORW*)pData;
		if (pcc->lpCustColors)
			pcc->lpCustColors = m_crCust;
		BeginCbtHook(this);
		return ChooseColorW((CHOOSECOLORW*)pData);
	}

	INT_PTR DlgBox(HWND hParent, COLORREF crInit = 0, DWORD dwFlags = 0, COLORREF* pcrCust = NULL)
	{
		CHOOSECOLORW cc{ sizeof(cc) };
		cc.hwndOwner = hParent;
		cc.rgbResult = crInit;
		cc.Flags = dwFlags;
		cc.lpCustColors = (pcrCust ? pcrCust : m_crCust);
		return DlgBox(hParent, &cc);
	}

	EckInline void SetRGB(COLORREF cr) const { SendMsg(s_uMsgSetRgb, 0, cr); }

	EckInline BOOL EndDlg(INT_PTR nResult) override { return FALSE; }
};
inline UINT CColorDialog::s_uMsgSetRgb = RegisterWindowMessageW(SETRGBSTRINGW);

class CFontDialog :public CDialog
{
public:
	static UINT s_uMsgSetRgb;
public:
	INT_PTR DlgBox(HWND hParent, void* pData = NULL) override
	{
		auto pcf = (CHOOSEFONTW*)pData;
		BeginCbtHook(this);
		return ChooseFontW((CHOOSEFONTW*)pData);
	}

	EckInline BOOL EndDlg(INT_PTR nResult) override { return FALSE; }
};

EckInline int MsgBox(PCWSTR pszText, PCWSTR pszCaption = L"", UINT uType = 0, HWND hParent = NULL)
{
	return MessageBoxW(hParent, pszText, pszCaption, uType);
}

EckInline int MsgBox(const CRefStrW& rs, PCWSTR pszCaption = L"", UINT uType = 0, HWND hParent = NULL)
{
	return MessageBoxW(hParent, rs.Data(), pszCaption, uType);
}
ECK_NAMESPACE_END