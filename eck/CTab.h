/*
* WinEzCtrlKit Library
*
* CTab.h ： 标准选择夹
*
* Copyright(C) 2024 QingKong
*/
#pragma once
#include "CWnd.h"

ECK_NAMESPACE_BEGIN
class CTab :public CWnd
{
public:
	ECK_RTTI(CTab);

	ECK_CWND_CREATE;
	HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, HMENU hMenu, PCVOID pData = nullptr) override
	{
		return IntCreate(0, WC_TABCONTROLW, nullptr, dwStyle,
			x, y, cx, cy, hParent, hMenu, nullptr, nullptr);
	}

	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		switch (uMsg)
		{
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint(hWnd, &ps);
			CEzCDC DC{};
			DC.Create(hWnd, ps.rcPaint.right, ps.rcPaint.bottom);
			CWnd::OnMsg(hWnd, WM_PAINT, (WPARAM)DC.GetDC(), lParam);
			BitBltPs(&ps, DC.GetDC());
			EndPaint(hWnd, &ps);
		}
		return 0;
		}
		return CWnd::OnMsg(hWnd, uMsg, wParam, lParam);
	}

	/// <summary>
	/// 区域矩形转换
	/// </summary>
	/// <param name="prc">RECT指针</param>
	/// <param name="bValidAeraToWndAera">为TRUE则从显示区域转换到窗口区域，为FALSE则相反</param>
	void AdjustRect(RECT* prc, BOOL bValidAeraToWndAera)
	{
		SendMsg(TCM_ADJUSTRECT, bValidAeraToWndAera, (LPARAM)prc);
	}

	BOOL DeleteAllItems()
	{
		return (BOOL)SendMsg(TCM_DELETEALLITEMS, 0, 0);
	}

	BOOL DeleteItem(int idx)
	{
		return (BOOL)SendMsg(TCM_DELETEITEM, idx, 0);
	}

	/// <summary>
	/// 重置所有项目。
	/// 仅设置TCS_BUTTONS时有效
	/// </summary>
	/// <param name="bAllTab">若为TRUE，则重置所有项，若为FALSE，则重置除当前页面之外的所有项</param>
	void DeselectAll(BOOL bAllTab)
	{
		SendMsg(TCM_DESELECTALL, bAllTab, 0);
	}

	int GetCurFocus()
	{
		return (int)SendMsg(TCM_GETCURFOCUS, 0, 0);
	}

	int GetCurSel()
	{
		return (int)SendMsg(TCM_GETCURSEL, 0, 0);
	}

	DWORD GetExtendedStyle()
	{
		return (DWORD)SendMsg(TCM_GETEXTENDEDSTYLE, 0, 0);
	}

	HIMAGELIST GetImageList()
	{
		return (HIMAGELIST)SendMsg(TCM_GETIMAGELIST, 0, 0);
	}

	BOOL GetItem(int idx, TCITEMW* ptci)
	{
		return (BOOL)SendMsg(TCM_GETITEMW, idx, (LPARAM)ptci);
	}

	int GetItemCount()
	{
		return (int)SendMsg(TCM_GETITEMCOUNT, 0, 0);
	}

	BOOL GetItemRect(int idx, RECT* prc)
	{
		return (BOOL)SendMsg(TCM_GETITEMRECT, idx, (LPARAM)prc);
	}

	int GetRowCount()
	{
		return (int)SendMsg(TCM_GETROWCOUNT, 0, 0);
	}

	HWND GetToolTips()
	{
		return (HWND)SendMsg(TCM_GETTOOLTIPS, 0, 0);
	}

	BOOL HighlightItem(int idx, BOOL bHighlight)
	{
		return (BOOL)SendMsg(TCM_HIGHLIGHTITEM, idx, MAKELPARAM(bHighlight, 0));
	}

	int HitTest(TCHITTESTINFO* ptchti)
	{
		return (int)SendMsg(TCM_HITTEST, 0, (LPARAM)ptchti);
	}

	int InsertItem(int idx, TCITEMW* ptci)
	{
		return (int)SendMsg(TCM_INSERTITEMW, idx, (LPARAM)ptci);
	}

	int InsertItem(PCWSTR pszText, int idx = -1, int idxImage = -1, LPARAM lParam = 0)
	{
		if (idx < 0)
			idx = GetItemCount();
		TCITEMW tci;
		tci.mask = TCIF_TEXT | TCIF_PARAM;
		tci.pszText = (PWSTR)pszText;
		tci.lParam = lParam;
		if (idxImage >= 0)
		{
			tci.mask |= TCIF_IMAGE;
			tci.iImage = idxImage;
		}
		return InsertItem(idx, &tci);
	}

	void RemoveImage(int idx)
	{
		SendMsg(TCM_REMOVEIMAGE, idx, 0);
	}

	void SetCurFocus(int idx)
	{
		SendMsg(TCM_SETCURFOCUS, idx, 0);
	}

	int SetCurSel(int idx)
	{
		return (int)SendMsg(TCM_SETCURSEL, idx, 0);
	}

	DWORD SetExtendedStyle(DWORD dwNew, DWORD dwMask)
	{
		return (DWORD)SendMsg(TCM_SETEXTENDEDSTYLE, dwMask, dwNew);
	}

	HIMAGELIST SetImageList(HIMAGELIST hImageList)
	{
		return (HIMAGELIST)SendMsg(TCM_SETIMAGELIST, 0, (LPARAM)hImageList);
	}

	BOOL SetItem(int idx, TCITEMW* ptci)
	{
		return (BOOL)SendMsg(TCM_SETITEMW, idx, (LPARAM)ptci);
	}

	BOOL SetItemExtra(SIZE_T cbExtra)
	{
		return (BOOL)SendMsg(TCM_SETITEMEXTRA, cbExtra, 0);
	}

	void SetItemSize(int cx, int cy, int* piOldWidth = nullptr, int* piOldHeight = nullptr)
	{
		DWORD dwRet = (DWORD)SendMsg(TCM_SETITEMSIZE, 0, MAKELPARAM(cx, cy));
		if (piOldWidth)
			*piOldWidth = LOWORD(dwRet);
		if (piOldHeight)
			*piOldHeight = HIWORD(dwRet);
	}

	int SetMinTabWidth(int cxMin = -1)
	{
		return (int)SendMsg(TCM_SETMINTABWIDTH, 0, cxMin);
	}

	void SetPadding(int cx, int cy)
	{
		SendMsg(TCM_SETITEMSIZE, 0, MAKELPARAM(cx, cy));
	}

	void SetToolTips(HWND hToolTip)
	{
		SendMsg(TCM_SETTOOLTIPS, (WPARAM)hToolTip, 0);
	}
};
ECK_RTTI_IMPL_BASE_INLINE(CTab, CWnd);
ECK_NAMESPACE_END