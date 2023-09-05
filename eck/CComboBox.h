/*
* WinEzCtrlKit Library
*
* CComboBox.h �� ��׼��Ͽ�
*
* Copyright(C) 2023 QingKong
*/
#pragma once
#include "CWnd.h"

#include <CommCtrl.h>

ECK_NAMESPACE_BEGIN
class CComboBox :public CWnd
{
public:
	CComboBox()
	{

	}

	HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, int nID, PCVOID pData = NULL) override
	{
		dwStyle |= WS_CHILD;
		m_hWnd = CreateWindowExW(0, WC_COMBOBOXW, NULL, dwStyle,
			x, y, cx, cy, hParent, i32ToP<HMENU>(nID), NULL, NULL);
		return m_hWnd;
	}

	EckInline int AddString(PCWSTR psz)
	{
		return (int)SendMsg(CB_ADDSTRING, 0, (LPARAM)psz);
	}

	EckInline int AddString(LPARAM lParam)
	{
		return (int)SendMsg(CB_ADDSTRING, 0, lParam);
	}

	/// <summary>
	/// ɾ����Ŀ
	/// </summary>
	/// <param name="idx"></param>
	/// <returns>����ʣ����Ŀ��</returns>
	EckInline int DeleteString(int idx)
	{
		return (int)SendMsg(CB_DELETESTRING, idx, 0);
	}

	/// <summary>
	/// ����·��
	/// </summary>
	/// <param name="pszPath">·��</param>
	/// <param name="uFlags">DDL_����</param>
	/// <returns>����</returns>
	EckInline int Dir(PCWSTR pszPath, UINT uFlags)
	{
		return (int)SendMsg(CB_DIR, uFlags, (LPARAM)pszPath);
	}

	/// <summary>
	/// ������Ŀ��
	/// �����ִ�Сд
	/// </summary>
	/// <param name="pszText">�ı�����ƥ���Ը��ı���ͷ����Ŀ</param>
	/// <param name="idxStart">��ʼ������-1 = ��ͷ���������б�</param>
	/// <returns>����</returns>
	EckInline int FindString(PCWSTR pszText, int idxStart = -1)
	{
		return (int)SendMsg(CB_FINDSTRING, idxStart, (LPARAM)pszText);
	}

	/// <summary>
	/// ������ȫƥ����Ŀ��
	/// �����ִ�Сд
	/// </summary>
	/// <param name="pszText">�ı�����ƥ������ı���ȫ��ͬ����Ŀ</param>
	/// <param name="idxStart">��ʼ������-1 = ��ͷ���������б�</param>
	/// <returns>����</returns>
	EckInline int FindStringExact(PCWSTR pszText, int idxStart = -1)
	{
		return (int)SendMsg(CB_FINDSTRINGEXACT, idxStart, (LPARAM)pszText);
	}

	EckInline BOOL GetComboBoxInfo(COMBOBOXINFO* pcbi)
	{
		return (BOOL)SendMsg(CB_GETCOMBOBOXINFO, 0, (LPARAM)pcbi);
	}

	EckInline int GetCount()
	{
		return (int)SendMsg(CB_GETCOUNT, 0, 0);
	}

	/// <summary>
	/// ȡ��ʾ����ı�
	/// </summary>
	/// <param name="pszBuf">������</param>
	/// <param name="cchBuf">pszBufָʾ�Ļ�������С����WCHAR�ƣ�������βNULL</param>
	/// <returns>�ɹ�����1��ʧ�ܷ��ش������</returns>
	EckInline int GetCueBanner(PWSTR pszBuf, int cchBuf)
	{
		return (int)SendMsg(CB_GETCUEBANNER, (WPARAM)pszBuf, cchBuf);
	}

	/// <summary>
	/// ȡ����ѡ���
	/// �Ե�ѡ�б����÷�������ѡ����Զ�ѡ�б����÷��ؽ�����Ŀ
	/// </summary>
	/// <returns>����</returns>
	EckInline int GetCurrSel()
	{
		return (int)SendMsg(CB_GETCURSEL, 0, 0);
	}

	/// <summary>
	/// ȡ�����б�����
	/// </summary>
	/// <param name="prc">���վ��Σ������Ļ</param>
	/// <returns>�ɹ�����TRUE��ʧ�ܷ���FALSE</returns>
	EckInline BOOL GetDroppedCtrlRect(RECT* prc)
	{
		return (BOOL)SendMsg(CB_GETDROPPEDCONTROLRECT, 0, (LPARAM)prc);
	}

	EckInline BOOL GetDroppedState()
	{
		return (BOOL)SendMsg(CB_GETDROPPEDSTATE, 0, 0);
	}

	/// <summary>
	/// ȡ�����б����С��ȡ�
	/// Ĭ����С���Ϊ0���б����Ϊmax(��С���, ��Ͽ����ؼ����)
	/// </summary>
	/// <returns>�ɹ�����������С��ȣ�ʧ�ܷ���CB_ERR</returns>
	EckInline BOOL GetDroppedWidth()
	{
		return (BOOL)SendMsg(CB_GETDROPPEDWIDTH, 0, 0);
	}

	EckInline void GetEditSel(DWORD* pdwStart = NULL, DWORD* pdwEnd = NULL)
	{
		SendMsg(CB_GETEDITSEL, (WPARAM)pdwStart, (LPARAM)pdwEnd);
	}

	EckInline BOOL GetExtendUI()
	{
		return (BOOL)SendMsg(CB_GETEXTENDEDUI, 0, 0);
	}

	EckInline int GetHorizontalExtent()
	{
		return (int)SendMsg(CB_GETHORIZONTALEXTENT, 0, 0);
	}

	EckInline LPARAM GetItemData(int idx)
	{
		return SendMsg(CB_GETITEMDATA, idx, 0);
	}

	EckInline int GetItemHeight(int idx)
	{
		return (int)SendMsg(CB_GETITEMHEIGHT, idx, 0);
	}

	EckInline CRefStrW GetItemText(int idx)
	{
		CRefStrW rs;
		int cch = GetItemTextLength(idx);
		if (cch <= 0)
			return rs;
		rs.ReSize(cch);
		SendMsg(CB_GETLBTEXT, idx, (LPARAM)rs.Data());
		return rs;
	}

	/// <summary>
	/// ȡ��Ŀ�ı�
	/// </summary>
	/// <param name="idx"></param>
	/// <param name="pszBuf"></param>
	/// <returns>�����ַ�����������βNULL����ʧ�ܷ���-1</returns>
	EckInline int GetItemText(int idx, PWSTR pszBuf)
	{
		return (int)SendMsg(CB_GETLBTEXT, idx, (LPARAM)pszBuf);
	}

	/// <summary>
	/// ȡ��Ŀ�ı�����
	/// </summary>
	/// <param name="idx"></param>
	/// <returns>�����ַ�����������βNULL��</returns>
	EckInline int GetItemTextLength(int idx)
	{
		return (int)SendMsg(CB_GETLBTEXTLEN, idx, 0);
	}

	EckInline LCID GetLocale()
	{
		return (LCID)SendMsg(CB_GETLOCALE, 0, 0);
	}

	EckInline int GetMinVisible()
	{
		return (int)SendMsg(CB_GETMINVISIBLE, 0, 0);
	}

	EckInline int GetTopIndex()
	{
		return (int)SendMsg(CB_GETTOPINDEX, 0, 0);
	}

	/// <summary>
	/// 
	/// </summary>
	/// <param name="cItems"></param>
	/// <param name="cbString"></param>
	/// <returns>�ɹ�������Ԥ�������Ŀ������ʧ�ܷ���CB_ERRSPACE</returns>
	EckInline int InitStorage(int cItems, SIZE_T cbString)
	{
		return (int)SendMsg(CB_INITSTORAGE, cItems, cbString);
	}

	EckInline int InsertString(PCWSTR psz, int idxPos = -1)
	{
		return (int)SendMsg(CB_INSERTSTRING, idxPos, (LPARAM)psz);
	}

	EckInline int InsertString(LPARAM lParam, int idxPos = -1)
	{
		return (int)SendMsg(CB_INSERTSTRING, idxPos, lParam);
	}

	/// <summary>
	/// ���ı���������
	/// </summary>
	/// <param name="cch">�ַ�������ȣ���Ϊ0������Ϊ0x7FFFFFFE</param>
	EckInline void LimitText(int cch = 0)
	{
		SendMsg(CB_LIMITTEXT, cch, 0);
	}

	EckInline void ResetContent()
	{
		SendMsg(CB_RESETCONTENT, 0, 0);
	}

	/// <summary>
	/// ���Ҳ�ѡ����Ŀ��
	/// �����ִ�Сд
	/// </summary>
	/// <param name="pszText">�ı�����ƥ���Ը��ı���ͷ����Ŀ</param>
	/// <param name="idxStart">��ʼ������-1 = ��ͷ���������б�</param>
	/// <returns>������ʧ�ܷ���CB_ERR</returns>
	EckInline int SelectString(PCWSTR pszText, int idxStart = -1)
	{
		return (int)SendMsg(CB_SELECTSTRING, idxStart, (LPARAM)pszText);
	}

	/// <summary>
	/// ����ʾ����ı�
	/// </summary>
	/// <param name="pszText">�ı�</param>
	/// <returns>�ɹ�����1��ʧ�ܷ��ش�����</returns>
	EckInline int SetCueBanner(PWSTR pszText)
	{
		return (int)SendMsg(CB_SETCUEBANNER, 0, (LPARAM)pszText);
	}

	EckInline BOOL SetCurSel(int idxSel = -1)
	{
		int iRet = (int)SendMsg(CB_SETCURSEL, idxSel, 0);
		if (idxSel < 0)
			return TRUE;
		else
			return (iRet != CB_ERR);
	}

	EckInline BOOL SetDroppedWidth(int cx = 0)
	{
		return (SendMsg(CB_SETDROPPEDWIDTH, cx, 0) != CB_ERR);
	}

	EckInline void SetEditSel(WORD wStart, WORD wEnd)
	{
		SendMsg(CB_SETEDITSEL, 0, MAKELPARAM(wStart, wEnd));
	}

	EckInline BOOL SetExtendUI(BOOL bExtUI)
	{
		return (SendMsg(CB_SETEXTENDEDUI, bExtUI, 0) != CB_ERR);
	}

	EckInline void SetHorizontalExtent(int iHorizontalExtent)
	{
		SendMsg(CB_SETHORIZONTALEXTENT, iHorizontalExtent, 0);
	}

	EckInline BOOL SetItemData(int idx, LPARAM lParam)
	{
		return (SendMsg(CB_SETITEMDATA, idx, lParam) != CB_ERR);
	}

	/// <summary>
	/// ����Ŀ�߶�
	/// </summary>
	/// <param name="idx">Ϊ0ʱ�����б���Ŀ�߶ȣ�Ϊ-1ʱ�������ؼ��߶ȡ�
	/// ����Ͽ����CBS_OWNERDRAWVARIABLE����ò���ָʾ��Ŀ����</param>
	/// <param name="cy">�߶�</param>
	/// <returns>�ɹ�����TRUE��ʧ�ܷ���FALSE</returns>
	EckInline BOOL SetItemHeight(int idx, int cy)
	{
		return (SendMsg(CB_SETITEMHEIGHT, idx, cy) != CB_ERR);
	}

	/// <summary>
	/// ����Ŀ�߶���չ��
	/// �޸����ø߶�ʱ��ƫ������⣬�������������ؼ��߶�
	/// </summary>
	/// <param name="cy">�߶�</param>
	/// <returns>�ɹ�����TRUE��ʧ�ܷ���FALSE</returns>
	EckInline BOOL SetItemHeight(int cy)
	{
		RECT rc;
		GetClientRect(m_hWnd, &rc);
		int iOffset = rc.bottom - (int)SendMsg(CB_GETITEMHEIGHT, -1, 0);
		return (SendMsg(CB_SETITEMHEIGHT, -1, cy - iOffset) != CB_ERR);
	}

	void SetItemString(int idx, PCWSTR pszText)
	{
		LPARAM lParam = GetItemData(idx);
		int idxNew = InsertString(pszText, idx);
		SetItemData(idxNew, lParam);
		if (idxNew <= idx)
			DeleteString(idx + 1);
		else
			DeleteString(idx);
		SetCurSel(idxNew);
	}

	/// <summary>
	/// 
	/// </summary>
	/// <param name="lcid"></param>
	/// <returns>�ɹ�������ǰ��LCID��ʧ�ܷ���CB_ERR</returns>
	EckInline LCID SetLocale(LCID lcid)
	{
		return (LCID)SendMsg(CB_SETLOCALE, lcid, 0);
	}

	EckInline BOOL SetMinVisible(int cItems)
	{
		return (BOOL)SendMsg(CB_SETMINVISIBLE, cItems, 0);
	}

	EckInline BOOL SetTopIndex(int idx)
	{
		return (SendMsg(CB_SETTOPINDEX, idx, 0) != CB_ERR);
	}

	EckInline void ShowDropDown(BOOL bShow)
	{
		SendMsg(CB_SHOWDROPDOWN, bShow, 0);
	}
};
ECK_NAMESPACE_END