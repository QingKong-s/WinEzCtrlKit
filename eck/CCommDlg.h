/*
* WinEzCtrlKit Library
*
* CCommDlg.h �� ͨ�öԻ���
*
* Copyright(C) 2023 QingKong
*/
#pragma once
#include "ECK.h"

#include <vector>

#include <CommCtrl.h>

ECK_NAMESPACE_BEGIN
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

	static HRESULT CALLBACK TDCallBack(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LONG_PTR lRefData);

	HWND m_hDlg = NULL;
public:
	std::vector<TASKDIALOG_BUTTON> m_aBtn{};// ���а�ť
	std::vector<TASKDIALOG_BUTTON> m_aRadioBtn{};// ���е�ѡ��ť

	CTaskDialog() = default;

	/// <summary>
	/// ��ʾ��
	/// �˷�������cButtons��pButtons��cRadioButtons��pRadioButtons��hwndParent�ֶΣ�Ȼ��ʹ��this���ݵ���TaskDialogIndirect
	/// </summary>
	/// <param name="hParent">�����ھ��</param>
	/// <param name="piRadioButton">���յ�ѡ��״̬</param>
	/// <param name="pbChecked">����ѡ���״̬</param>
	/// <param name="phr">���մ������</param>
	/// <returns>���ذ�ťID</returns>
	int Show(TASKDIALOGCONFIG* ptdc, int* piRadioButton = NULL, BOOL* pbChecked = NULL, HRESULT* phr = NULL);

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
	/// ��Ԫ���ı���
	/// ���ڲ��ֿ��ܻ�仯����Ӧ���ı�
	/// </summary>
	/// <param name="uType">Ԫ�����ͣ�TDE_����</param>
	/// <param name="pszText">�ı�</param>
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
	/// ������_��״̬
	/// </summary>
	/// <param name="uState">״̬��PBST_����</param>
	EckInline void SetPBState(UINT uState)
	{
		SendMessageW(m_hDlg, TDM_SET_PROGRESS_BAR_STATE, uState, 0);
	}

	/// <summary>
	/// ����Ԫ���ı���
	/// ���ڲ��ֲ���仯��������ı�������ھ��ı�
	/// </summary>
	/// <param name="uType">Ԫ�����ͣ�TDE_����</param>
	/// <param name="pszText">�ı�</param>
	EckInline void UpdateElementText(UINT uType, PCWSTR pszText)
	{
		SendMessageW(m_hDlg, TDM_UPDATE_ELEMENT_TEXT, uType, (LPARAM)pszText);
	}

	/// <summary>
	/// ����ͼ��
	/// </summary>
	/// <param name="uType">Ԫ�����ͣ�TDIE_ICON_����</param>
	/// <param name="Icon">ͼ�꣬��ΪHICON��PCWSTR��ȡ���ڴ����Ի���ʱ������</param>
	template<class T>
	EckInline void UpdateIcon(UINT uType, T Icon)
	{
		static_assert(std::is_same<T, HICON>::value || std::is_same<T, PCWSTR>::value,
			"T is 'NOT' HICON or PCWSTR");
		SendMessageW(m_hDlg, TDM_UPDATE_ICON, uType, (LPARAM)Icon);
	}
};



EckInline UINT MsgBox(PCWSTR pszText, PCWSTR pszCaption = L"", UINT uType = 0, HWND hParent = NULL)
{
	MessageBoxW(hParent, pszText, pszCaption, uType);
}
ECK_NAMESPACE_END