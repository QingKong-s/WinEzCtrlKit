/*
* WinEzCtrlKit Library
*
* CListBox.h ： 标准列表框
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
	BITBOOL bAutoSort : 1;			// 自动排序
	BITBOOL bMultiSel : 1;			// 鼠标多选
	BITBOOL bExtSel : 1;			// 按键多选
	BITBOOL bDragList : 1;			// 表项可拖动
	BITBOOL bIntegralHeight : 1;	// 取整控件高度
	BITBOOL bDisableNoScroll : 1;	// 显示禁止的滚动条
};

class CListBox :public CWnd
{
public:
	ECK_RTTI(CListBox);
protected:
	ECKLISTBOXINFO m_Info{};

	static UINT m_uMsgDragList;		// 拖动列表框消息
public:
	ECK_CWND_CREATE;
	HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, HMENU hMenu, PCVOID pData = nullptr) override
	{
		dwStyle |= WS_CHILD;
		m_hWnd = CreateWindowExW(0, WC_LISTBOXW, nullptr, dwStyle,
			x, y, cx, cy, hParent, hMenu, nullptr, nullptr);
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
	/// 删除项目
	/// </summary>
	/// <param name="idx"></param>
	/// <returns>返回剩余项目数</returns>
	EckInline int DeleteString(int idx)
	{
		return (int)SendMsg(LB_DELETESTRING, idx, 0);
	}

	/// <summary>
	/// 加入路径
	/// </summary>
	/// <param name="pszPath">路径</param>
	/// <param name="uFlags">DDL_常量</param>
	/// <returns>索引</returns>
	EckInline int Dir(PCWSTR pszPath, UINT uFlags)
	{
		return (int)SendMsg(LB_DIR, uFlags, (LPARAM)pszPath);
	}

	/// <summary>
	/// 查找项目。
	/// 不区分大小写
	/// </summary>
	/// <param name="pszText">文本，将匹配以该文本开头的项目</param>
	/// <param name="idxStart">起始索引，-1 = 从头搜索整个列表</param>
	/// <returns>索引</returns>
	EckInline int FindString(PCWSTR pszText, int idxStart = -1)
	{
		return (int)SendMsg(LB_FINDSTRING, idxStart, (LPARAM)pszText);
	}

	/// <summary>
	/// 查找完全匹配项目。
	/// 不区分大小写
	/// </summary>
	/// <param name="pszText">文本，将匹配与该文本完全相同的项目</param>
	/// <param name="idxStart">起始索引，-1 = 从头搜索整个列表</param>
	/// <returns>索引</returns>
	EckInline int FindStringExact(PCWSTR pszText, int idxStart = -1)
	{
		return (int)SendMsg(LB_FINDSTRINGEXACT, idxStart, (LPARAM)pszText);
	}

	EckInline int GetAnchorIndex()
	{
		return (int)SendMsg(LB_GETANCHORINDEX, 0, 0);
	}

	/// <summary>
	/// 取焦点项目。
	/// 对单选列表框调用返回现行选中项，对多选列表框调用返回焦点项目
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
	/// 取现行选中项。
	/// 对单选列表框调用返回现行选中项，对多选列表框调用返回焦点项目
	/// </summary>
	/// <returns>索引</returns>
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
	/// 取被选择项目数
	/// </summary>
	/// <returns>数目，若为单选列表框，则返回-1</returns>
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
	/// 取被选择项目
	/// </summary>
	/// <param name="piSelItems">数组</param>
	/// <param name="c">数组中的元素数</param>
	/// <returns>项目数，若为单选列表，则返回-1</returns>
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
	/// <returns>返回字符数（不含结尾NULL），失败返回-1</returns>
	EckInline int GetItemText(int idx, PWSTR pszBuf)
	{
		return (int)SendMsg(LB_GETTEXT, idx, (LPARAM)pszBuf);
	}

	/// <summary>
	/// 
	/// </summary>
	/// <param name="idx"></param>
	/// <returns>返回字符数（不含结尾NULL）</returns>
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
	/// <returns>成功返回已预分配的项目总数，失败返回LB_ERRSPACE</returns>
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
	/// 查找并选择项目。
	/// 不区分大小写
	/// </summary>
	/// <param name="pszText">文本，将匹配以该文本开头的项目</param>
	/// <param name="idxStart">起始索引，-1 = 从头搜索整个列表</param>
	/// <returns>索引，失败返回LB_ERR</returns>
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
	/// <param name="idx">若列表框具有LBS_OWNERDRAWVARIABLE，则该参数指示项目索引，否则则必须设置为0</param>
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
	/// <returns>成功返回先前的LCID，失败返回LB_ERR</returns>
	EckInline LCID SetLocale(LCID lcid)
	{
		return (LCID)SendMsg(LB_SETLOCALE, lcid, 0);
	}

	/// <summary>
	/// 选择项目。
	/// 仅用于多选列表
	/// </summary>
	/// <param name="idx">索引，若设为-1则操作所有项目</param>
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
ECK_RTTI_IMPL_BASE_INLINE(CListBox, CWnd);
inline UINT CListBox::m_uMsgDragList = RegisterWindowMessageW(DRAGLISTMSGSTRING);
ECK_NAMESPACE_END