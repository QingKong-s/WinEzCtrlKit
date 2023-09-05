/*
* WinEzCtrlKit Library
*
* CListBox.h �� ��׼�б��
*
* Copyright(C) 2023 QingKong
*/
#pragma once
#include "CWnd.h"

#include <vector>

#include <CommCtrl.h>

ECK_NAMESPACE_BEGIN
struct ECKLISTBOXINFO
{
	BITBOOL bAutoSort : 1;			// �Զ�����
	BITBOOL bMultiSel : 1;			// ����ѡ
	BITBOOL bExtSel : 1;			// ������ѡ
	BITBOOL bDragList : 1;			// ������϶�
	BITBOOL bIntegralHeight : 1;	// ȡ���ؼ��߶�
	BITBOOL bDisableNoScroll : 1;	// ��ʾ��ֹ�Ĺ�����
};

class CListBox :public CWnd
{
protected:
	ECKLISTBOXINFO m_Info{};

	static UINT m_uMsgDragList;		// �϶��б����Ϣ
public:
	CListBox()
	{
		if (!m_uMsgDragList)
			m_uMsgDragList = RegisterWindowMessageW(DRAGLISTMSGSTRING);
	}

	HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, int nID, PCVOID pData = NULL) override
	{
		dwStyle |= WS_CHILD;
		m_hWnd = CreateWindowExW(0, WC_LISTBOXW, NULL, dwStyle,
			x, y, cx, cy, hParent, i32ToP<HMENU>(nID), NULL, NULL);
		return m_hWnd;
	}

	EckInline void SetDragList(BOOL bDragList)
	{
		m_Info.bDragList = bDragList;
		MakeDragList(m_hWnd);
	}

	EckInline BOOL GetDragList()
	{
		return m_Info.bDragList;
	}

	EckInline void SetIntegralHeight(BOOL bIntegralHeight)
	{
		m_Info.bIntegralHeight = bIntegralHeight;
	}

	EckInline BOOL GetIntegralHeight()
	{
		return !IsBitSet(GetStyle(), LBS_NOINTEGRALHEIGHT);
	}

	EckInline void SetDisableNoScroll(BOOL bDisableNoScroll)
	{
		m_Info.bDisableNoScroll = bDisableNoScroll;
	}

	EckInline BOOL GetDisableNoScroll()
	{
		return !IsBitSet(GetStyle(), LBS_DISABLENOSCROLL);
	}

	EckInline void SetAutoSort(BOOL bAutoSort)
	{
		m_Info.bAutoSort = bAutoSort;
	}

	EckInline BOOL GetAutoSort()
	{
		return IsBitSet(GetStyle(), LBS_SORT);
	}

	EckInline void SetMultiSel(BOOL bMultiSel)
	{
		m_Info.bMultiSel = bMultiSel;
	}

	EckInline BOOL GetMultiSel()
	{
		return IsBitSet(GetStyle(), LBS_MULTIPLESEL);
	}

	EckInline void SetExtSel(BOOL bExtSel)
	{
		m_Info.bExtSel = bExtSel;
	}

	EckInline BOOL GetExtSel()
	{
		return IsBitSet(GetStyle(), LBS_EXTENDEDSEL);
	}

	EckInline BOOL GetHasString()
	{
		return IsBitSet(GetStyle(), LBS_HASSTRINGS);
	}

	EckInline void RedrawItem(int idx)
	{
		RECT rcItem;
		if (SendMessageW(m_hWnd, LB_GETITEMRECT, idx, (LPARAM)&rcItem) == LB_ERR)
			return;

		InvalidateRect(m_hWnd, &rcItem, TRUE);
	}

	EckInline int AddString(PCWSTR psz)
	{
		return (int)SendMsg(LB_ADDSTRING, 0, (LPARAM)psz);
	}

	EckInline int AddString(LPARAM lParam)
	{
		return (int)SendMsg(LB_ADDSTRING, 0, lParam);
	}

	/// <summary>
	/// ɾ����Ŀ
	/// </summary>
	/// <param name="idx"></param>
	/// <returns>����ʣ����Ŀ��</returns>
	EckInline int DeleteString(int idx)
	{
		return (int)SendMsg(LB_DELETESTRING, idx, 0);
	}

	/// <summary>
	/// ����·��
	/// </summary>
	/// <param name="pszPath">·��</param>
	/// <param name="uFlags">DDL_����</param>
	/// <returns>����</returns>
	EckInline int Dir(PCWSTR pszPath, UINT uFlags)
	{
		return (int)SendMsg(LB_DIR, uFlags, (LPARAM)pszPath);
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
		return (int)SendMsg(LB_FINDSTRING, idxStart, (LPARAM)pszText);
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
		return (int)SendMsg(LB_FINDSTRINGEXACT, idxStart, (LPARAM)pszText);
	}

	EckInline int GetAnchorIndex()
	{
		return (int)SendMsg(LB_GETANCHORINDEX, 0, 0);
	}

	/// <summary>
	/// ȡ������Ŀ��
	/// �Ե�ѡ�б����÷�������ѡ����Զ�ѡ�б����÷��ؽ�����Ŀ
	/// </summary>
	/// <returns></returns>
	EckInline int GetCaretIndex()
	{
		return (int)SendMsg(LB_GETCARETINDEX, 0, 0);
	}

	EckInline int GetCount()
	{
		return (int)SendMsg(LB_GETCOUNT, 0, 0);
	}

	/// <summary>
	/// ȡ����ѡ���
	/// �Ե�ѡ�б����÷�������ѡ����Զ�ѡ�б����÷��ؽ�����Ŀ
	/// </summary>
	/// <returns>����</returns>
	EckInline int GetCurrSel()
	{
		return (int)SendMsg(LB_GETCURSEL, 0, 0);
	}

	EckInline int GetHorizontalExtent()
	{
		return (int)SendMsg(LB_GETHORIZONTALEXTENT, 0, 0);
	}

	EckInline LPARAM GetItemData(int idx)
	{
		return SendMsg(LB_GETITEMDATA, idx, 0);
	}

	EckInline int GetItemHeight(int idx)
	{
		return (int)SendMsg(LB_GETITEMHEIGHT, idx, 0);
	}

	EckInline BOOL GetItemRect(int idx, RECT* prc)
	{
		return (SendMsg(LB_GETITEMRECT, idx, (LPARAM)prc) != LB_ERR);
	}

	EckInline int GetItemCountPreColumn()
	{
		return (int)SendMsg(LB_GETLISTBOXINFO, 0, 0);
	}

	EckInline LCID GetLocale()
	{
		return (LCID)SendMsg(LB_GETLOCALE, 0, 0);
	}

	EckInline int GetSel(int idx)
	{
		return (SendMsg(LB_GETSEL, idx, 0) > 0);
	}

	/// <summary>
	/// ȡ��ѡ����Ŀ��
	/// </summary>
	/// <returns>��Ŀ����Ϊ��ѡ�б���򷵻�-1</returns>
	EckInline int GetSelCount()
	{
		return (int)SendMsg(LB_GETSELCOUNT, 0, 0);
	}

	EckInline std::vector<int> GetSelItems()
	{
		std::vector<int> aItems;
		int cSelItems = GetSelCount();
		if (cSelItems <= 0)
			return aItems;

		aItems.resize(cSelItems);
		auto lRet = SendMsg(LB_GETSELITEMS, cSelItems, (LPARAM)aItems.data());
		return aItems;
	}

	/// <summary>
	/// ȡ��ѡ����Ŀ
	/// </summary>
	/// <param name="piSelItems">����</param>
	/// <param name="c">�����е�Ԫ����</param>
	/// <returns>��Ŀ������Ϊ��ѡ�б��򷵻�-1</returns>
	EckInline int GetSelItems(int* piSelItems, int c)
	{
		return (int)SendMsg(LB_GETSELITEMS, c, (LPARAM)piSelItems);
	}

	EckInline CRefStrW GetItemText(int idx)
	{
		CRefStrW rs;
		int cch = GetItemTextLength(idx);
		if (cch <= 0)
			return rs;
		rs.ReSize(cch);
		SendMsg(LB_GETTEXT, idx, (LPARAM)rs.Data());
		return rs;
	}

	/// <summary>
	/// 
	/// </summary>
	/// <param name="idx"></param>
	/// <param name="pszBuf"></param>
	/// <returns>�����ַ�����������βNULL����ʧ�ܷ���-1</returns>
	EckInline int GetItemText(int idx, PWSTR pszBuf)
	{
		return (int)SendMsg(LB_GETTEXT, idx, (LPARAM)pszBuf);
	}

	/// <summary>
	/// 
	/// </summary>
	/// <param name="idx"></param>
	/// <returns>�����ַ�����������βNULL��</returns>
	EckInline int GetItemTextLength(int idx)
	{
		return (int)SendMsg(LB_GETTEXTLEN, idx, 0);
	}

	EckInline int GetTopIndex()
	{
		return (int)SendMsg(LB_GETTOPINDEX, 0, 0);
	}

	/// <summary>
	/// 
	/// </summary>
	/// <param name="cItems"></param>
	/// <param name="cbString"></param>
	/// <returns>�ɹ�������Ԥ�������Ŀ������ʧ�ܷ���LB_ERRSPACE</returns>
	EckInline int InitStorage(int cItems, SIZE_T cbString)
	{
		return (int)SendMsg(LB_INITSTORAGE, cItems, cbString);
	}

	EckInline int InsertString(PCWSTR psz, int idxPos = -1)
	{
		return (int)SendMsg(LB_INSERTSTRING, idxPos, (LPARAM)psz);
	}

	EckInline int InsertString(LPARAM lParam, int idxPos = -1)
	{
		return (int)SendMsg(LB_INSERTSTRING, idxPos, lParam);
	}

	EckInline int ItemFromPt(POINT pt, BOOL bAutoScroll = FALSE)
	{
		ClientToScreen(m_hWnd, &pt);
		return LBItemFromPt(m_hWnd, pt, bAutoScroll);
	}

	EckInline void ResetContent()
	{
		SendMsg(LB_RESETCONTENT, 0, 0);
	}

	/// <summary>
	/// ���Ҳ�ѡ����Ŀ��
	/// �����ִ�Сд
	/// </summary>
	/// <param name="pszText">�ı�����ƥ���Ը��ı���ͷ����Ŀ</param>
	/// <param name="idxStart">��ʼ������-1 = ��ͷ���������б�</param>
	/// <returns>������ʧ�ܷ���LB_ERR</returns>
	EckInline int SelectString(PCWSTR pszText, int idxStart = -1)
	{
		return (int)SendMsg(LB_SELECTSTRING, idxStart, (LPARAM)pszText);
	}

	EckInline BOOL SelectItemRange(int idxStart, int idxEnd, BOOL bSel)
	{
		if (!bSel)
			std::swap(idxStart, idxEnd);
		return (SendMsg(LB_SELITEMRANGEEX, idxStart, idxEnd) != LB_ERR);
	}

	EckInline BOOL SetAnchorIndex(int idx)
	{
		return (SendMsg(LB_SETANCHORINDEX, idx, 0) != LB_ERR);
	}

	EckInline BOOL SetCaretIndex(int idx, BOOL bNoFullVisible = FALSE)
	{
		return (SendMsg(LB_SETCARETINDEX, idx, bNoFullVisible) != LB_ERR);
	}

	EckInline void SetColumnWidth(int cxColumn)
	{
		SendMsg(LB_SETCOLUMNWIDTH, cxColumn, 0);
	}

	EckInline int SetCount(int cItems)
	{
		return (int)SendMsg(LB_SETCOUNT, cItems, 0);
	}

	EckInline BOOL SetCurrSel(int idxSel = -1)
	{
		int iRet = (int)SendMsg(LB_SETCURSEL, idxSel, 0);
		if (idxSel < 0)
			return TRUE;
		else
			return (iRet != LB_ERR);
	}

	EckInline void SetHorizontalExtent(int iHorizontalExtent)
	{
		SendMsg(LB_SETHORIZONTALEXTENT, iHorizontalExtent, 0);
	}

	EckInline BOOL SetItemData(int idx, LPARAM lParam)
	{
		return (SendMsg(LB_SETITEMDATA, idx, lParam) != LB_ERR);
	}

	/// <summary>
	/// 
	/// </summary>
	/// <param name="idx">���б�����LBS_OWNERDRAWVARIABLE����ò���ָʾ��Ŀ�������������������Ϊ0</param>
	/// <param name="cy"></param>
	/// <returns></returns>
	EckInline BOOL SetItemHeight(int idx, int cy)
	{
		return (SendMsg(LB_SETITEMHEIGHT, idx, cy) != LB_ERR);
	}

	/// <summary>
	/// 
	/// </summary>
	/// <param name="lcid"></param>
	/// <returns>�ɹ�������ǰ��LCID��ʧ�ܷ���LB_ERR</returns>
	EckInline LCID SetLocale(LCID lcid)
	{
		return (LCID)SendMsg(LB_SETLOCALE, lcid, 0);
	}

	/// <summary>
	/// ѡ����Ŀ��
	/// �����ڶ�ѡ�б�
	/// </summary>
	/// <param name="idx">����������Ϊ-1�����������Ŀ</param>
	/// <param name="bSel"></param>
	/// <returns></returns>
	EckInline BOOL SetSel(int idx, BOOL bSel)
	{
		return (SendMsg(LB_SETSEL, bSel, idx) != LB_ERR);
	}

	EckInline BOOL SetTabStop(int* piTabStop, int c)
	{
		return (BOOL)SendMsg(LB_SETTABSTOPS, c, (LPARAM)piTabStop);
	}

	EckInline BOOL SetTopIndex(int idx)
	{
		return (SendMsg(LB_SETTOPINDEX, idx, 0) != LB_ERR);
	}
};

ECK_NAMESPACE_END