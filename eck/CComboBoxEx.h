/*
* WinEzCtrlKit Library
*
* CComboBoxEx.h ： 标准组合框Ex
*
* Copyright(C) 2024 QingKong
*/
#pragma once
#include "CComboBox.h"

ECK_NAMESPACE_BEGIN
class CComboBoxEx :public CComboBox
{
public:
	EckInline 
	ECK_CWND_CREATE
	{
		dwStyle |= WS_CHILD;
		m_hWnd = IntCreate(0, WC_COMBOBOXEXW, NULL, dwStyle,
			x, y, cx, cy, hParent, hMenu, NULL, NULL);
		return m_hWnd;
	}
	/// <summary>
	/// 删除项目
	/// </summary>
	/// <param name="idx">索引</param>
	/// <returns>成功返回剩余项数，失败返回CB_ERR</returns>
	EckInline int DeleteItem(int idx) const
	{
		return (int)SendMsg(CBEM_DELETEITEM, idx, 0);
	}

	EckInline HWND GetComboBoxControl() const
	{
		return (HWND)SendMsg(CBEM_GETCOMBOCONTROL, 0, 0);
	}

	EckInline HWND GetEditControl() const
	{
		return (HWND)SendMsg(CBEM_GETEDITCONTROL, 0, 0);
	}

	EckInline DWORD GetExtendedStyle() const
	{
		return (DWORD)SendMsg(CBEM_GETEXTENDEDSTYLE, 0, 0);
	}

	EckInline HIMAGELIST GetImageList() const
	{
		return (HIMAGELIST)SendMsg(CBEM_GETIMAGELIST, 0, 0);
	}

	EckInline BOOL GetItem(COMBOBOXEXITEMW* pcbei) const
	{
		return (BOOL)SendMsg(CBEM_GETITEMW, 0, (LPARAM)pcbei);
	}

	EckInline CRefStrW GetItemText(int idx) const
	{
		CRefStrW rs(260);
		COMBOBOXEXITEMW cbei;
		cbei.iItem = idx;
		cbei.mask = CBEIF_TEXT;
		cbei.pszText = rs.Data();
		cbei.cchTextMax = rs.Size();

		BOOL bRet = (BOOL)SendMsg(CBEM_GETITEMW, 0, (LPARAM)&cbei);
		if (bRet)
			return rs;
		else
			return {};
	}

	EckInline BOOL HasEditChanged() const
	{
		return (BOOL)SendMsg(CBEM_HASEDITCHANGED, 0, 0);
	}

	/// <summary>
	/// 置扩展样式
	/// </summary>
	/// <param name="dwMask">掩码，若为0则修改所有样式</param>
	/// <param name="dwStyle">样式</param>
	/// <returns>返回先前样式</returns>
	EckInline DWORD SetExtendedStyle(DWORD dwMask, DWORD dwStyle) const
	{
		return (DWORD)SendMsg(CBEM_SETEXTENDEDSTYLE, dwMask, dwStyle);
	}

	EckInline HIMAGELIST SetImageList(HIMAGELIST hImageList) const
	{
		return (HIMAGELIST)SendMsg(CBEM_SETIMAGELIST, 0, (LPARAM)hImageList);
	}

	EckInline BOOL SetItem(COMBOBOXEXITEMW* pcbei) const
	{
		return (BOOL)SendMsg(CBEM_SETITEMW, 0, (LPARAM)pcbei);
	}

	EckInline BOOL SetItemText(int idx, PCWSTR pszText) const
	{
		COMBOBOXEXITEMW cbei;
		cbei.iItem = idx;
		cbei.mask = CBEIF_TEXT;
		cbei.pszText = (PWSTR)pszText;

		return (BOOL)SendMsg(CBEM_SETITEMW, 0, (LPARAM)&cbei);
	}
};
ECK_NAMESPACE_END