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

#include <CommCtrl.h>

ECK_NAMESPACE_BEGIN
struct TASKDIALOGCTX
{
	TASKDIALOGCONFIG* ptdc;
	int* piRadioButton;
	BOOL* pbChecked;
	HRESULT* phr;
};

class CTaskDialog :public CDialog
{
private:
	std::vector<TASKDIALOG_BUTTON> m_aBtn{};// 所有按钮
	std::vector<TASKDIALOG_BUTTON> m_aRadioBtn{};// 所有单选按钮
public:
	CTaskDialog() = default;

	INT_PTR DlgBox(HWND hParent, void* pData = NULL) override
	{
		auto pCtx = (TASKDIALOGCTX*)pData;
		const auto ptdc = pCtx->ptdc;
		ptdc->cButtons = (int)m_aBtn.size();
		if (ptdc->cButtons)
			ptdc->pButtons = m_aBtn.data();
		else
			ptdc->pButtons = NULL;

		ptdc->cRadioButtons = (int)m_aRadioBtn.size();
		if (ptdc->cRadioButtons)
			ptdc->pRadioButtons = m_aRadioBtn.data();
		else
			ptdc->pRadioButtons = NULL;

		ptdc->pfCallback = ptdc->pfCallback;
		ptdc->lpCallbackData = ptdc->lpCallbackData;

		int iButton = 0;
		int iRadioButton = 0;
		BOOL bChecked = FALSE;
		BeginCbtHook(this);
		HRESULT hr = TaskDialogIndirect(ptdc, &iButton,
			pCtx->piRadioButton ? pCtx->piRadioButton : &iRadioButton,
			pCtx->pbChecked ? pCtx->pbChecked : &bChecked);
		if (pCtx->phr)
			*pCtx->phr = hr;
		return iButton;
	}

	EckInline BOOL EndDlg(INT_PTR nResult) override { return FALSE; }

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
		SendMsg(TDM_NAVIGATE_PAGE, 0, (LPARAM)this);
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
};

class CColorDialog :public CDialog
{
protected:
	static UINT s_uMsgSetRgb;
public:
	CColorDialog() = default;

	INT_PTR DlgBox(HWND hParent, void* pData = NULL) override
	{
		auto pcc = (CHOOSECOLORW*)pData;
		pcc->hwndOwner = hParent;
		BeginCbtHook(this);
		return ChooseColorW((CHOOSECOLORW*)pData);
	}

	INT_PTR DlgBox(HWND hParent, COLORREF crInit = 0, DWORD dwFlags = 0, COLORREF* pcrCust = NULL)
	{
		static COLORREF crCust[16]{};
		CHOOSECOLORW cc{ sizeof(cc) };
		cc.hwndOwner = hParent;
		cc.rgbResult = crInit;
		cc.Flags = dwFlags;
		cc.lpCustColors = (pcrCust ? pcrCust : crCust);
		return DlgBox(hParent, &cc);
	}

	EckInline void SetRGB(COLORREF cr) const { SendMsg(s_uMsgSetRgb, 0, cr); }

	EckInline BOOL EndDlg(INT_PTR nResult) override { return FALSE; }
};
inline UINT CColorDialog::s_uMsgSetRgb = RegisterWindowMessageW(SETRGBSTRINGW);

EckInline int MsgBox(PCWSTR pszText, PCWSTR pszCaption = L"", UINT uType = 0, HWND hParent = NULL)
{
	return MessageBoxW(hParent, pszText, pszCaption, uType);
}
ECK_NAMESPACE_END