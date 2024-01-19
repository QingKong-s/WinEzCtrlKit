/*
* WinEzCtrlKit Library
*
* CHeader.h ： 标准表头
*
* Copyright(C) 2023 QingKong
*/
#pragma once
#include "CWnd.h"

ECK_NAMESPACE_BEGIN
class CHeader :public CWnd
{
public:
	ECK_CWND_NOSINGLEOWNER(CHeader)
public:
	ECK_CWND_CREATE;
	HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, HMENU hMenu, PCVOID pData = NULL) override
	{
		return IntCreate(dwExStyle, WC_HEADERW, pszText, dwStyle,
			x, y, cx, cy, hParent, hMenu, NULL, NULL);
	}

	/// <summary>
	/// 清除筛选器
	/// </summary>
	/// <param name="idx">列索引，若为-1则清除所有筛选器</param>
	/// <returns></returns>
	EckInline BOOL ClearFilter(int idx)
	{
		return (BOOL)SendMsg(HDM_CLEARFILTER, idx, 0);
	}

	EckInline HIMAGELIST CreateDragImage(int idx)
	{
		return (HIMAGELIST)SendMsg(HDM_CREATEDRAGIMAGE, idx, 0);
	}

	EckInline BOOL DeleteItem(int idx)
	{
		return (BOOL)SendMsg(HDM_DELETEITEM, idx, 0);
	}

	EckInline BOOL EditFilter(int idx, BOOL bDiscardUserInput)
	{
		return (BOOL)SendMsg(HDM_CLEARFILTER, idx, bDiscardUserInput);
	}

	EckInline int GetBitmapMargin()
	{
		return (int)SendMsg(HDM_GETBITMAPMARGIN, 0, 0);
	}

	EckInline int GetFocusItem()
	{
		return (int)SendMsg(HDM_GETFOCUSEDITEM, 0, 0);
	}

	EckInline HIMAGELIST GetImageList(UINT uType = HDSIL_NORMAL)
	{
		return (HIMAGELIST)SendMsg(HDM_GETIMAGELIST, uType, 0);
	}

	EckInline BOOL GetItem(int idx, HDITEMW* phdi)
	{
		return (BOOL)SendMsg(HDM_GETITEMW, idx, (LPARAM)phdi);
	}

	EckInline int GetItemCount()
	{
		return (int)SendMsg(HDM_GETITEMCOUNT, 0, 0);
	}

	/// <summary>
	/// 取项目拆分按钮矩形
	/// </summary>
	/// <param name="idx">项目索引</param>
	/// <param name="prc">矩形指针，相对控件父窗口</param>
	/// <returns>成功返回TRUE，失败返回FALSE</returns>
	EckInline BOOL GetItemDropDownRect(int idx, RECT* prc)
	{
		return (BOOL)SendMsg(HDM_GETITEMDROPDOWNRECT, idx, (LPARAM)prc);
	}

	/// <summary>
	/// 取项目矩形
	/// </summary>
	/// <param name="idx">项目索引</param>
	/// <param name="prc">矩形指针，相对控件父窗口</param>
	/// <returns>成功返回TRUE，失败返回FALSE</returns>
	EckInline BOOL GetItemRect(int idx, RECT* prc)
	{
		return (BOOL)SendMsg(HDM_GETITEMRECT, idx, (LPARAM)prc);
	}

	EckInline std::vector<int> GetOrderArray()
	{
		std::vector<int> aOrder(GetItemCount());
		SendMsg(HDM_GETORDERARRAY, aOrder.size(), (LPARAM)aOrder.data());
		return aOrder;
	}

	EckInline BOOL GetOrderArray(int* piOrder, int cBuf)
	{
		return (BOOL)SendMsg(HDM_GETORDERARRAY, cBuf, (LPARAM)piOrder);
	}

	/// <summary>
	/// 取溢出按钮矩形
	/// </summary>
	/// <param name="prc">矩形指针，相对屏幕</param>
	/// <returns>成功返回TRUE，失败返回FALSE</returns>
	EckInline BOOL GetOverFlowRect(RECT* prc)
	{
		return (BOOL)SendMsg(HDM_GETOVERFLOWRECT, 0, (LPARAM)prc);
	}

	EckInline int HitTest(HDHITTESTINFO* phdhti)
	{
		return (int)SendMsg(HDM_HITTEST, 0, (LPARAM)phdhti);
	}

	EckInline int InsertItem(int idx, HDITEMW* phdi)
	{
		return (int)SendMsg(HDM_INSERTITEMW, idx, (LPARAM)phdi);
	}

	EckInline int InsertItem(PCWSTR pszText, int idx = -1, int cxItem = -1, 
		int idxImage = -1, int iFmt = HDF_LEFT, LPARAM lParam = 0)
	{
		if (idx < 0)
			idx = INT_MAX;
		HDITEMW hdi;
		hdi.mask = HDI_TEXT | HDI_FORMAT | HDI_LPARAM;
		hdi.fmt = iFmt;
		hdi.lParam = lParam;
		hdi.pszText = (PWSTR)pszText;
		if (cxItem >= 0)
		{
			hdi.mask |= HDI_WIDTH;
			hdi.cxy = cxItem;
		}

		if (idxImage >= 0)
		{
			hdi.mask |= HDI_IMAGE;
			hdi.iImage = idxImage;
		}

		return InsertItem(idx, &hdi);
	}

	EckInline BOOL Layout(HDLAYOUT* phdl)
	{
		return (BOOL)SendMsg(HDM_LAYOUT, 0, (LPARAM)phdl);
	}

	EckInline int OrderToIndex(int iOrder)
	{
		return (int)SendMsg(HDM_ORDERTOINDEX, iOrder, 0);
	}

	EckInline int SetBitmapMargin(int iMargin)
	{
		return (int)SendMsg(HDM_SETBITMAPMARGIN, iMargin, 0);
	}

	EckInline int SetFilterChangeTimeout(int iTimeout)
	{
		return (int)SendMsg(HDM_SETFILTERCHANGETIMEOUT, 0, iTimeout);
	}

	EckInline BOOL SetFocusedItem(int idx)
	{
		return (BOOL)SendMsg(HDM_SETFOCUSEDITEM, 0, idx);
	}

	EckInline int SetHotDivider(int idxDivider)
	{
		return (int)SendMsg(HDM_SETHOTDIVIDER, FALSE, idxDivider);
	}

	EckInline int SetHotDivider(POINT ptCursor)
	{
		return (int)SendMsg(HDM_SETHOTDIVIDER, TRUE, MAKELPARAM(ptCursor.x, ptCursor.y));
	}

	EckInline HIMAGELIST SetImageList(HIMAGELIST hImageList, UINT uType = HDSIL_NORMAL)
	{
		return (HIMAGELIST)SendMsg(HDM_SETIMAGELIST, uType, (LPARAM)hImageList);
	}

	EckInline BOOL SetItem(int idx, HDITEMW* phdi)
	{
		return (BOOL)SendMsg(HDM_SETITEMW, idx, (LPARAM)phdi);
	}

	EckInline BOOL SetOrderArray(int* piOrder)
	{
		return (BOOL)SendMsg(HDM_SETORDERARRAY, GetItemCount(), (LPARAM)piOrder);
	}
};

ECK_NAMESPACE_END