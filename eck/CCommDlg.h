/*
* WinEzCtrlKit Library
*
* CCommDlg.h ： 通用对话框
*
* Copyright(C) 2023 QingKong
*/
#pragma once
#include "ECK.h"

#include <vector>

#include <CommCtrl.h>

ECK_NAMESPACE_BEGIN
template<class T>
concept IsHICONOrPCWSTR = requires{ std::is_same<T, HICON>::value || std::is_same<T, PCWSTR>::value; };

class CTaskDialog
{
private:
	struct ECKTDCTX
	{
		PFTASKDIALOGCALLBACK pProc;
		LONG_PTR lRefData;
		CTaskDialog* pThis;
		TASKDIALOGCONFIG* ptdc;
	};

	static HRESULT CALLBACK TDCallBack(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LONG_PTR lRefData)
	{
		auto p = (ECKTDCTX*)lRefData;
		switch (uMsg)
		{
		case TDN_DIALOG_CONSTRUCTED:
			p->ptdc->pfCallback = p->pProc;
			p->ptdc->lpCallbackData = p->lRefData;
			p->pThis->m_hDlg = hWnd;
			break;
		case TDN_DESTROYED:
			p->pThis->m_hDlg = NULL;
			break;
		}

		if (p->pProc)
			return p->pProc(hWnd, uMsg, wParam, lParam, p->lRefData);
		else
			return S_OK;
	}

	HWND m_hDlg = NULL;
public:
	std::vector<TASKDIALOG_BUTTON> m_aBtn{};// 所有按钮
	std::vector<TASKDIALOG_BUTTON> m_aRadioBtn{};// 所有单选按钮

	CTaskDialog() = default;

	/// <summary>
	/// 显示。
	/// 此方法更新cButtons、pButtons、cRadioButtons、pRadioButtons和hwndParent字段，然后使用this数据调用TaskDialogIndirect
	/// </summary>
	/// <param name="hParent">父窗口句柄</param>
	/// <param name="piRadioButton">接收单选框状态</param>
	/// <param name="pbChecked">接收选择框状态</param>
	/// <param name="phr">接收错误代码</param>
	/// <returns>返回按钮ID</returns>
	int Show(TASKDIALOGCONFIG* ptdc, int* piRadioButton = NULL, BOOL* pbChecked = NULL, HRESULT* phr = NULL)
	{
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

		ECKTDCTX Ctx{ ptdc->pfCallback,ptdc->lpCallbackData,this };
		ptdc->pfCallback = TDCallBack;
		ptdc->lpCallbackData = (LONG_PTR)&Ctx;

		int iButton = 0;
		int iRadioButton = 0;
		BOOL bChecked = FALSE;
		HRESULT hr = TaskDialogIndirect(
			ptdc,
			&iButton,
			piRadioButton ? piRadioButton : &iRadioButton,
			pbChecked ? pbChecked : &bChecked);
		if (phr)
			*phr = hr;
		return iButton;
	}

	EckInline HWND GetHWND()
	{
		return m_hDlg;
	}

	EckInline operator HWND()
	{
		return m_hDlg;
	}

	EckInline void ClickButton(int iID, BOOL bRadioButton = FALSE)
	{
		if (bRadioButton)
			SendMessageW(m_hDlg, TDM_CLICK_RADIO_BUTTON, iID, 0);
		else
			SendMessageW(m_hDlg, TDM_CLICK_BUTTON, iID, 0);
	}

	EckInline void ClickCheckBox(BOOL bChecked, BOOL bSetFocus = FALSE)
	{
		SendMessageW(m_hDlg, TDM_CLICK_VERIFICATION, bChecked, bSetFocus);
	}

	EckInline void ClickButton(BOOL bEnable, int iID, BOOL bRadioButton = FALSE)
	{
		if (bRadioButton)
			SendMessageW(m_hDlg, TDM_ENABLE_RADIO_BUTTON, iID, bEnable);
		else
			SendMessageW(m_hDlg, TDM_ENABLE_BUTTON, iID, bEnable);
	}

	EckInline void NavigatePage(TASKDIALOGCONFIG* pInfo)
	{
		SendMessageW(m_hDlg, TDM_NAVIGATE_PAGE, 0, (LPARAM)pInfo);
	}

	EckInline void NavigatePage()
	{
		SendMessageW(m_hDlg, TDM_NAVIGATE_PAGE, 0, (LPARAM)this);
	}

	EckInline void SetShieldIcon(BOOL bShieldIcon, int iID)
	{
		SendMessageW(m_hDlg, TDM_SET_BUTTON_ELEVATION_REQUIRED_STATE, iID, bShieldIcon);
	}

	/// <summary>
	/// 置元素文本。
	/// 窗口布局可能会变化以适应新文本
	/// </summary>
	/// <param name="uType">元素类型，TDE_常量</param>
	/// <param name="pszText">文本</param>
	EckInline void SetElementText(UINT uType, PCWSTR pszText)
	{
		SendMessageW(m_hDlg, TDM_SET_ELEMENT_TEXT, uType, (LPARAM)pszText);
	}

	EckInline void SetMarqueePBShowing(BOOL bShowing)
	{
		SendMessageW(m_hDlg, TDM_SET_MARQUEE_PROGRESS_BAR, bShowing, 0);
	}

	EckInline void SetPBMarquee(BOOL bMarquee, UINT uAnimationGap = 0u)
	{
		SendMessageW(m_hDlg, TDM_SET_PROGRESS_BAR_MARQUEE, bMarquee, uAnimationGap);
	}

	EckInline void SetPBPos(int iPos)
	{
		SendMessageW(m_hDlg, TDM_SET_PROGRESS_BAR_POS, iPos, 0);
	}

	EckInline void SetPBRange(int iMax, int iMin)
	{
		SendMessageW(m_hDlg, TDM_SET_PROGRESS_BAR_POS, 0, MAKELPARAM(iMin, iMax));
	}

	/// <summary>
	/// 进度条_置状态
	/// </summary>
	/// <param name="uState">状态，PBST_常量</param>
	EckInline void SetPBState(UINT uState)
	{
		SendMessageW(m_hDlg, TDM_SET_PROGRESS_BAR_STATE, uState, 0);
	}

	/// <summary>
	/// 更新元素文本。
	/// 窗口布局不会变化，因此新文本必须短于旧文本
	/// </summary>
	/// <param name="uType">元素类型，TDE_常量</param>
	/// <param name="pszText">文本</param>
	EckInline void UpdateElementText(UINT uType, PCWSTR pszText)
	{
		SendMessageW(m_hDlg, TDM_UPDATE_ELEMENT_TEXT, uType, (LPARAM)pszText);
	}

	/// <summary>
	/// 更新图标
	/// </summary>
	/// <param name="uType">元素类型，TDIE_ICON_常量</param>
	/// <param name="Icon">图标，可为HICON或PCWSTR，取决于创建对话框时的设置</param>
	template<IsHICONOrPCWSTR T>
	EckInline void UpdateIcon(UINT uType, T Icon)
	{
		SendMessageW(m_hDlg, TDM_UPDATE_ICON, uType, (LPARAM)Icon);
	}
};



EckInline int MsgBox(PCWSTR pszText, PCWSTR pszCaption = L"", UINT uType = 0, HWND hParent = NULL)
{
	return MessageBoxW(hParent, pszText, pszCaption, uType);
}
ECK_NAMESPACE_END