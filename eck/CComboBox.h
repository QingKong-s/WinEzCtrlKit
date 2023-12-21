/*
* WinEzCtrlKit Library
*
* CComboBox.h ： 标准组合框
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
	ECK_CWNDPROP_STYLE(AutoHScroll, CBS_AUTOHSCROLL);
	ECK_CWNDPROP_STYLE(DisableNoScroll, CBS_DISABLENOSCROLL);
	ECK_CWNDPROP_STYLE(DropDown, CBS_DROPDOWN);
	ECK_CWNDPROP_STYLE(DropDownList, CBS_DROPDOWNLIST);
	ECK_CWNDPROP_STYLE(HasString, CBS_HASSTRINGS);
	ECK_CWNDPROP_STYLE(LowerCase, CBS_LOWERCASE);
	ECK_CWNDPROP_STYLE(NoIntegralHeight, CBS_NOINTEGRALHEIGHT);
	ECK_CWNDPROP_STYLE(OemConvert, CBS_OEMCONVERT);
	ECK_CWNDPROP_STYLE(OwnerDrawFixed, CBS_OWNERDRAWFIXED);
	ECK_CWNDPROP_STYLE(OwnerDrawVariable, CBS_OWNERDRAWVARIABLE);
	ECK_CWNDPROP_STYLE(Simple, CBS_SIMPLE);
	ECK_CWNDPROP_STYLE(Sort, CBS_SORT);
	ECK_CWNDPROP_STYLE(UpperCase, CBS_UPPERCASE);
public:
	ECK_CWND_CREATE;
	HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, HMENU hMenu, PCVOID pData = NULL) override
	{
		dwStyle |= WS_CHILD;
		m_hWnd = IntCreate(0, WC_COMBOBOXW, NULL, dwStyle,
			x, y, cx, cy, hParent, hMenu, NULL, NULL);
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
	/// 删除项目
	/// </summary>
	/// <param name="idx"></param>
	/// <returns>返回剩余项目数</returns>
	EckInline int DeleteString(int idx)
	{
		return (int)SendMsg(CB_DELETESTRING, idx, 0);
	}

	/// <summary>
	/// 加入路径
	/// </summary>
	/// <param name="pszPath">路径</param>
	/// <param name="uFlags">DDL_常量</param>
	/// <returns>索引</returns>
	EckInline int Dir(PCWSTR pszPath, UINT uFlags)
	{
		return (int)SendMsg(CB_DIR, uFlags, (LPARAM)pszPath);
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
		return (int)SendMsg(CB_FINDSTRING, idxStart, (LPARAM)pszText);
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
	/// 取提示横幅文本
	/// </summary>
	/// <param name="pszBuf">缓冲区</param>
	/// <param name="cchBuf">pszBuf指示的缓冲区大小，以WCHAR计，包含结尾NULL</param>
	/// <returns>成功返回1，失败返回错误代码</returns>
	EckInline int GetCueBanner(PWSTR pszBuf, int cchBuf)
	{
		return (int)SendMsg(CB_GETCUEBANNER, (WPARAM)pszBuf, cchBuf);
	}

	/// <summary>
	/// 取现行选中项。
	/// 对单选列表框调用返回现行选中项，对多选列表框调用返回焦点项目
	/// </summary>
	/// <returns>索引</returns>
	EckInline int GetCurSel()
	{
		return (int)SendMsg(CB_GETCURSEL, 0, 0);
	}

	/// <summary>
	/// 取下拉列表框矩形
	/// </summary>
	/// <param name="prc">接收矩形，相对屏幕</param>
	/// <returns>成功返回TRUE，失败返回FALSE</returns>
	EckInline BOOL GetDroppedCtrlRect(RECT* prc)
	{
		return (BOOL)SendMsg(CB_GETDROPPEDCONTROLRECT, 0, (LPARAM)prc);
	}

	EckInline BOOL GetDroppedState()
	{
		return (BOOL)SendMsg(CB_GETDROPPEDSTATE, 0, 0);
	}

	/// <summary>
	/// 取下拉列表框最小宽度。
	/// 默认最小宽度为0，列表框宽度为max(最小宽度, 组合框主控件宽度)
	/// </summary>
	/// <returns>成功返回正的最小宽度，失败返回CB_ERR</returns>
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
	/// 取项目文本
	/// </summary>
	/// <param name="idx"></param>
	/// <param name="pszBuf"></param>
	/// <returns>返回字符数（不含结尾NULL），失败返回-1</returns>
	EckInline int GetItemText(int idx, PWSTR pszBuf)
	{
		return (int)SendMsg(CB_GETLBTEXT, idx, (LPARAM)pszBuf);
	}

	/// <summary>
	/// 取项目文本长度
	/// </summary>
	/// <param name="idx"></param>
	/// <returns>返回字符数（不含结尾NULL）</returns>
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
	/// <returns>成功返回已预分配的项目总数，失败返回CB_ERRSPACE</returns>
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
	/// 置文本输入限制
	/// </summary>
	/// <param name="cch">字符串最长长度，若为0则限制为0x7FFFFFFE</param>
	EckInline void LimitText(int cch = 0)
	{
		SendMsg(CB_LIMITTEXT, cch, 0);
	}

	EckInline void ResetContent()
	{
		SendMsg(CB_RESETCONTENT, 0, 0);
	}

	/// <summary>
	/// 查找并选择项目。
	/// 不区分大小写
	/// </summary>
	/// <param name="pszText">文本，将匹配以该文本开头的项目</param>
	/// <param name="idxStart">起始索引，-1 = 从头搜索整个列表</param>
	/// <returns>索引，失败返回CB_ERR</returns>
	EckInline int SelectString(PCWSTR pszText, int idxStart = -1)
	{
		return (int)SendMsg(CB_SELECTSTRING, idxStart, (LPARAM)pszText);
	}

	/// <summary>
	/// 置提示横幅文本
	/// </summary>
	/// <param name="pszText">文本</param>
	/// <returns>成功返回1，失败返回错误码</returns>
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
	/// 置项目高度
	/// </summary>
	/// <param name="idx">为0时设置列表项目高度，为-1时设置主控件高度。
	/// 若组合框具有CBS_OWNERDRAWVARIABLE，则该参数指示项目索引</param>
	/// <param name="cy">高度</param>
	/// <returns>成功返回TRUE，失败返回FALSE</returns>
	EckInline BOOL SetItemHeight(int idx, int cy)
	{
		return (SendMsg(CB_SETITEMHEIGHT, idx, cy) != CB_ERR);
	}

	/// <summary>
	/// 置项目高度扩展。
	/// 修复设置高度时有偏差的问题，仅用于设置主控件高度
	/// </summary>
	/// <param name="cy">高度</param>
	/// <returns>成功返回TRUE，失败返回FALSE</returns>
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
	/// <returns>成功返回先前的LCID，失败返回CB_ERR</returns>
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