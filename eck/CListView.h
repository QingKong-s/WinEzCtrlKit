/*
* WinEzCtrlKit Library
*
* CListView.h ： 标准列表视图
*
* Copyright(C) 2023 QingKong
*/
#pragma once
#include "CWnd.h"
#include "CHeader.h"

#include <vector>

#include <CommCtrl.h>

ECK_NAMESPACE_BEGIN
typedef int (CALLBACK* PFNLVITEMCOMPARE)(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
typedef int (CALLBACK* PFNLVITEMCOMPAREEX)(int idx1, int idx2, LPARAM lParamSort);
class CListView :public CWnd
{
public:
	EckInline HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, int nID, PCVOID pData = NULL) override
	{
		dwStyle |= WS_CHILD;
		m_hWnd = CreateWindowExW(dwExStyle, WC_LISTVIEWW, pszText, dwStyle,
			x, y, cx, cy, hParent, i32ToP<HMENU>(nID), NULL, NULL);
		return m_hWnd;
	}

	/// <summary>
	/// 取理想尺寸。
	/// 根据参照尺寸计算显示指定项目数的最佳控件尺寸
	/// </summary>
	/// <param name="cItems">项目数，若为-1则使用当前项目数</param>
	/// <param name="cxRef">参照宽度，若为-1则使用控件宽度</param>
	/// <param name="cyRef">参照高度，若为-1则使用控件高度</param>
	/// <param name="piApprWidth">接收理想宽度</param>
	/// <param name="piApprHeight">接收理想高度</param>
	EckInline void ApproximateViewRect(int cItems = -1, int cxRef = -1, int cyRef = -1,
		int* piApprWidth = NULL, int* piApprHeight = NULL)
	{
		DWORD dwRet = (DWORD)SendMsg(LVM_APPROXIMATEVIEWRECT, cItems, MAKEWPARAM(cxRef, cyRef));
		if (piApprWidth)
			*piApprWidth = LOWORD(dwRet);
		if (piApprHeight)
			*piApprHeight = HIWORD(dwRet);
	}

	/// <summary>
	/// 排列项目
	/// </summary>
	/// <param name="uFlag">排列选项，LVA_常量。
	/// 注：当指定LVA_ALIGNLEFT或LVA_ALIGNTOP时会分别添加LVS_ALIGNLEFT和LVS_ALIGNTOP样式</param>
	/// <returns>成功返回TRUE，失败返回FALSE</returns>
	EckInline BOOL Arrange(UINT uFlag)
	{
		switch (uFlag)
		{
		case LVA_ALIGNLEFT:
			ModifyStyle(LVS_ALIGNLEFT, LVS_ALIGNLEFT);
			return TRUE;
		case LVA_ALIGNTOP:
			ModifyStyle(LVS_ALIGNTOP, LVS_ALIGNTOP);
			return TRUE;
		default:
			return (BOOL)SendMsg(LVM_ARRANGE, uFlag, 0);
		}
	}

	EckInline void CancelEditLabel()
	{
		SendMsg(LVM_CANCELEDITLABEL, 0, 0);
	}

	/// <summary>
	/// 创建拖放图像
	/// </summary>
	/// <param name="idx">项目</param>
	/// <param name="ptOrg">图像左上角的初始位置</param>
	/// <returns>图像列表句柄</returns>
	EckInline HIMAGELIST CreateDragImage(int idx, POINT ptOrg = {})
	{
		return (HIMAGELIST)SendMsg(LVM_CREATEDRAGIMAGE, idx, (LPARAM)&ptOrg);
	}

	EckInline BOOL DeleteAllItems()
	{
		return (BOOL)SendMsg(LVM_DELETEALLITEMS, 0, 0);
	}

	EckInline BOOL DeleteColumn(int idx)
	{
		return (BOOL)SendMsg(LVM_DELETECOLUMN, idx, 0);
	}

	EckInline BOOL DeleteItem(int idx)
	{
		return (BOOL)SendMsg(LVM_DELETEITEM, idx, 0);
	}

	/// <summary>
	/// 进入编辑
	/// </summary>
	/// <param name="idx">项目，若为-1则取消编辑</param>
	/// <returns>成功返回编辑框句柄，失败返回NULL。编辑框将在编辑结束后失效</returns>
	EckInline HWND EditLabel(int idx)
	{
		return (HWND)SendMsg(LVM_EDITLABELW, idx, 0);
	}

	/// <summary>
	/// 启用/禁用分组视图
	/// </summary>
	/// <param name="bEnable">是否启用</param>
	/// <returns>0 - 成功  1 - 控件状态已更改  -1 - 失败</returns>
	EckInline int EnableGroupView(BOOL bEnable)
	{
		return (int)SendMsg(LVM_ENABLEGROUPVIEW, bEnable, 0);
	}

	/// <summary>
	/// 保证显示
	/// </summary>
	/// <param name="idx">项目</param>
	/// <param name="bFullVisible">是否保证完全可见</param>
	/// <returns>成功返回TRUE，失败返回FALSE</returns>
	EckInline BOOL EnsureVisible(int idx, BOOL bFullVisible = TRUE)
	{
		return (BOOL)SendMsg(LVM_ENSUREVISIBLE, idx, bFullVisible);
	}

	/// <summary>
	/// 寻找项目
	/// </summary>
	/// <param name="idxStart">起始项目，若为-1则从头开始寻找</param>
	/// <param name="plvfi">LVFINDINFOW指针</param>
	/// <returns>成功返回项目索引，失败返回-1</returns>
	EckInline int FindItem(int idxStart, LVFINDINFOW* plvfi)
	{
		return (int)SendMsg(LVM_FINDITEMW, idxStart, (LPARAM)plvfi);
	}

	EckInline COLORREF GetBKColor()
	{
		return (COLORREF)SendMsg(LVM_GETBKCOLOR, 0, 0);
	}

	EckInline BOOL GetBKImage(LVBKIMAGEW* plvbki)
	{
		return (BOOL)SendMsg(LVM_GETBKIMAGEW, 0, (LPARAM)plvbki);
	}

	EckInline BOOL GetCallbackMask()
	{
		return (BOOL)SendMsg(LVM_GETCALLBACKMASK, 0, 0);
	}

	EckInline BOOL GetColumn(int idx, LVCOLUMNW* plvc)
	{
		return (BOOL)SendMsg(LVM_GETCALLBACKMASK, idx, (LPARAM)plvc);
	}

	EckInline std::vector<int> GetColumnOrderArray()
	{
		std::vector<int> aOrder{};
		int cColumn = (int)SendMessageW(GetHeaderCtrl(), HDM_GETITEMCOUNT, 0, 0);
		if (!cColumn)
			return aOrder;
		aOrder.resize(cColumn);
		SendMsg(LVM_GETCOLUMNORDERARRAY, cColumn, (LPARAM)aOrder.data());
		return aOrder;
	}

	EckInline BOOL GetColumnOrderArray(int cColumn, int* piOrder)
	{
		return (BOOL)SendMsg(LVM_GETCOLUMNORDERARRAY, cColumn, (LPARAM)piOrder);
	}

	EckInline int GetColumnWidth(int idx)
	{
		return (int)SendMsg(LVM_GETCOLUMNWIDTH, idx, 0);
	}

	EckInline int GetCountPerPage()
	{
		return (int)SendMsg(LVM_GETCOUNTPERPAGE, 0, 0);
	}

	EckInline HWND GetEditCtrl()
	{
		return (HWND)SendMsg(LVM_GETEDITCONTROL, 0, 0);
	}

	EckInline BOOL GetEmptyText(PWSTR pszBuf, int cchBuf)
	{
		return (BOOL)SendMsg(LVM_GETEMPTYTEXT, cchBuf, (LPARAM)pszBuf);
	}

	EckInline DWORD GetExtendLVStyle()
	{
		return (DWORD)SendMsg(LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0);
	}

	EckInline int GetFocusedGroup()
	{
		return (int)SendMsg(LVM_GETFOCUSEDGROUP, 0, 0);
	}

	EckInline int GetFooterItemCount()
	{
		LVFOOTERINFO lvfi{ LVFF_ITEMCOUNT };
		SendMsg(LVM_GETFOOTERINFO, 0, (LPARAM)&lvfi);
		return lvfi.cItems;
	}

	EckInline BOOL GetFooterItem(int idx, LVFOOTERITEM* plvfi)
	{
		return (BOOL)SendMsg(LVM_GETFOOTERITEM, idx, (LPARAM)plvfi);
	}

	EckInline BOOL GetFooterItemRect(int idx, RECT* prc)
	{
		return (BOOL)SendMsg(LVM_GETFOOTERITEMRECT, idx, (LPARAM)prc);
	}

	EckInline BOOL GetFooterRect(RECT* prc)
	{
		return (BOOL)SendMsg(LVM_GETFOOTERRECT, 0, (LPARAM)prc);
	}

	EckInline int GetGroupCount()
	{
		return (int)SendMsg(LVM_GETGROUPCOUNT, 0, 0);
	}

	EckInline int GetGroup(int iGroupID, LVGROUP* plvg)
	{
		plvg->cbSize = sizeof(LVGROUP);
		return (int)SendMsg(LVM_GETGROUPINFO, iGroupID, (LPARAM)plvg);
	}

	EckInline int GetGroupByIndex(int idx, LVGROUP* plvg)
	{
		plvg->cbSize = sizeof(LVGROUP);
		return (int)SendMsg(LVM_GETGROUPINFOBYINDEX, idx, (LPARAM)plvg);
	}

	EckInline void GetGroupMetrics(LVGROUPMETRICS* plvgm)
	{
		plvgm->cbSize = sizeof(LVGROUPMETRICS);
		SendMsg(LVM_GETGROUPMETRICS, NULL, (LPARAM)plvgm);
	}

	/// <summary>
	/// 取组矩形
	/// </summary>
	/// <param name="iGroupID">组ID</param>
	/// <param name="prc">接收矩形</param>
	/// <param name="uType">类型，LVGGR_常量，默认LVGGR_GROUP</param>
	/// <returns></returns>
	EckInline BOOL GetGroupRect(int iGroupID, RECT* prc, UINT uType = LVGGR_GROUP)
	{
		prc->top = uType;
		return (BOOL)SendMsg(LVM_GETGROUPRECT, iGroupID, (LPARAM)prc);
	}

	/// <summary>
	/// 取组状态
	/// </summary>
	/// <param name="iGroupID">组ID</param>
	/// <param name="uState">状态掩码，LVGS_常量</param>
	/// <returns>状态，只有掩码指定的位有效</returns>
	EckInline UINT GetGroupState(int iGroupID, UINT uState)
	{
		return (UINT)SendMsg(LVM_GETGROUPSTATE, iGroupID, uState);
	}

	EckInline CHeader GetHeaderCtrl()
	{
		return CHeader((HWND)SendMsg(LVM_GETHEADER, 0, 0));
	}

	EckInline HCURSOR GetHotCursor()
	{
		return (HCURSOR)SendMsg(LVM_GETHOTCURSOR, 0, 0);
	}

	EckInline int GetHotItem()
	{
		return (int)SendMsg(LVM_GETHOTITEM, 0, 0);
	}

	EckInline DWORD GetHoverTime()
	{
		return (DWORD)SendMsg(LVM_GETHOVERTIME, 0, 0);
	}

	/// <summary>
	/// 取图像列表
	/// </summary>
	/// <param name="uType">类型，LVSIL_常量，默认LVSIL_NORMAL</param>
	/// <returns>图像列表句柄</returns>
	EckInline HIMAGELIST GetImageList(UINT uType = LVSIL_NORMAL)
	{
		return (HIMAGELIST)SendMsg(LVM_GETHOVERTIME, uType, 0);
	}

	EckInline int GetInsertMark(BOOL* pbAfterItem)
	{
		LVINSERTMARK lvim{ sizeof(LVINSERTMARK) };
		if (!SendMsg(LVM_GETINSERTMARK, 0, (LPARAM)&lvim))
			return -1;
		if (pbAfterItem)
			*pbAfterItem = IsBitSet(lvim.dwFlags, LVIM_AFTER);
		return lvim.iItem;
	}

	EckInline COLORREF GetInsertMarkColor()
	{
		return (COLORREF)SendMsg(LVM_GETINSERTMARKCOLOR, 0, 0);
	}

	EckInline BOOL GetInsertMarkRect(RECT* prc)
	{
		return (BOOL)SendMsg(LVM_GETINSERTMARKRECT, 0, (LPARAM)prc);
	}

	EckInline int GetISearchString(PWSTR pszBuf)
	{
		return (int)SendMsg(LVM_GETISEARCHSTRINGW, 0, (LPARAM)pszBuf);
	}

	EckInline BOOL GetItem(int idx, int idxSubItem, LVITEMW* pli)
	{
		pli->iItem = idx;
		pli->iSubItem = idxSubItem;
		return (BOOL)SendMsg(LVM_GETITEMW, 0, (LPARAM)pli);
	}

	EckInline int GetItemCount()
	{
		return (int)SendMsg(LVM_GETITEMCOUNT, 0, 0);
	}

	EckInline BOOL GetItemRect(int idx, int idxGroup, RECT* prc, UINT uType = LVIR_BOUNDS)
	{
		LVITEMINDEX lvii;
		lvii.iItem = idx;
		lvii.iGroup = idxGroup;
		prc->left = uType;
		return (BOOL)SendMsg(LVM_GETITEMINDEXRECT, (WPARAM)&lvii, (LPARAM)prc);
	}

	EckInline BOOL GetItemRect(int idx, RECT* prc, UINT uType = LVIR_BOUNDS)
	{
		prc->left = uType;
		return (BOOL)SendMsg(LVM_GETITEMRECT, idx, (LPARAM)prc);
	}

	EckInline BOOL GetItemPosition(int idx, POINT* ppt)
	{
		return (BOOL)SendMsg(LVM_GETITEMPOSITION, idx, (LPARAM)ppt);
	}

	EckInline void GetItemSpacing(BOOL bSmallIconView, int* pxSpacing = NULL, int* pySpacing = NULL)
	{
		DWORD dwRet = (DWORD)SendMsg(LVM_GETITEMSPACING, bSmallIconView, 0);
		if (pxSpacing)
			*pxSpacing = LOWORD(dwRet);
		if (pySpacing)
			*pySpacing = HIWORD(dwRet);
	}

	/// <summary>
	/// 取项目状态
	/// </summary>
	/// <param name="idx">项目</param>
	/// <param name="uState">状态掩码</param>
	/// <returns>状态</returns>
	EckInline UINT GetItemState(int idx, UINT uState)
	{
		return (UINT)SendMsg(LVM_GETITEMSTATE, idx, uState);
	}

	/// <summary>
	/// 取项目文本
	/// </summary>
	/// <param name="idx">项目</param>
	/// <param name="idxSubItem">列</param>
	/// <param name="pszBuf">缓冲区，不可为NULL</param>
	/// <param name="cchBuf">缓冲区字符数</param>
	/// <returns>复制到缓冲区的字符数</returns>
	EckInline int GetItemText(int idx, int idxSubItem, PWSTR pszBuf, int cchBuf)
	{
		LVITEMW li;
		li.iSubItem = idxSubItem;
		li.pszText = pszBuf;
		li.cchTextMax = cchBuf;
		return (int)SendMsg(LVM_GETITEMTEXTW, idx, (LPARAM)&li);
	}

	/// <summary>
	/// 取项目句柄。
	/// 最多获取前259个字符
	/// </summary>
	/// <param name="idx">项目</param>
	/// <param name="idxSubItem">列</param>
	/// <returns>文本</returns>
	EckInline CRefStrW GetItemText(int idx, int idxSubItem)
	{
		CRefStrW rs;
		rs.ReSize(MAX_PATH - 1);
		LVITEMW li;
		li.iSubItem = idxSubItem;
		li.pszText = rs;
		li.cchTextMax = MAX_PATH;
		int cch = (int)SendMsg(LVM_GETITEMTEXTW, idx, (LPARAM)&li);
		rs.ReSize(cch);
		return rs;
	}

	/// <summary>
	/// 取下一项目
	/// </summary>
	/// <param name="idx">参照项目，若为-1则查找符合条件的第一个项目</param>
	/// <param name="uFlags">LVNI_常量</param>
	/// <returns>成功返回查找到的项目索引，失败返回-1</returns>
	EckInline int GetNextItem(int idx, UINT uFlags)
	{
		return (int)SendMsg(LVM_GETNEXTITEM, idx, uFlags);
	}

	/// <summary>
	/// 取下一项目
	/// </summary>
	/// <param name="idx">参照项目，若为-1则查找符合条件的第一个项目</param>
	/// <param name="idxGroup">组索引</param>
	/// <param name="uFlags">LVNI_常量</param>
	/// <returns>成功返回查找到的项目索引，失败返回-1</returns>
	EckInline int GetNextItem(int idx, int idxGroup, UINT uFlags)
	{
		if (idx < 0)
			return (int)SendMsg(LVM_GETNEXTITEMINDEX, -1, uFlags);
		else
		{
			LVITEMINDEX lvii;
			lvii.iItem = idx;
			lvii.iGroup = idxGroup;
			return (int)SendMsg(LVM_GETNEXTITEMINDEX, (WPARAM)&lvii, uFlags);
		}
	}

	EckInline int GetNumberOfWorkAreas()
	{
		UINT u = 0;
		SendMsg(LVM_GETNUMBEROFWORKAREAS, 0, (LPARAM)&u);
		return u;
	}

	EckInline BOOL GetOrigin(POINT* ppt)
	{
		return (BOOL)SendMsg(LVM_GETORIGIN, 0, (LPARAM)ppt);
	}

	EckInline COLORREF GetOutLineColor()
	{
		return (COLORREF)SendMsg(LVM_GETOUTLINECOLOR, 0, 0);
	}

	EckInline int GetSelectedColumn()
	{
		return (int)SendMsg(LVM_GETSELECTEDCOLUMN, 0, 0);
	}

	EckInline int GetSelectedCount()
	{
		return (int)SendMsg(LVM_GETSELECTEDCOUNT, 0, 0);
	}

	EckInline int GetSelectionMark()
	{
		return (int)SendMsg(LVM_GETSELECTIONMARK, 0, 0);
	}

	EckInline int GetStringWidth(PCWSTR pszText)
	{
		return (int)SendMsg(LVM_GETSTRINGWIDTHW, 0, (LPARAM)pszText);
	}

	EckInline int GetSubItemRect(int idx, int idxSubItem, RECT* prc, UINT uType = LVIR_BOUNDS)
	{
		prc->top = idxSubItem;
		prc->left = uType;
		return (int)SendMsg(LVM_GETSUBITEMRECT, idx, (LPARAM)prc);
	}

	EckInline COLORREF GetTextBKColor()
	{
		return (COLORREF)SendMsg(LVM_GETTEXTBKCOLOR, 0, 0);
	}

	EckInline COLORREF GetTextColor()
	{
		return (COLORREF)SendMsg(LVM_GETTEXTCOLOR, 0, 0);
	}

	EckInline void GetTileInfo(LVTILEINFO* plvti)
	{
		plvti->cbSize = sizeof(LVTILEINFO);
		SendMsg(LVM_GETTILEINFO, 0, (LPARAM)plvti);
	}

	EckInline void GetTileViewInfo(LVTILEVIEWINFO* plvtvi)
	{
		plvtvi->cbSize = sizeof(LVTILEVIEWINFO);
		SendMsg(LVM_GETTILEVIEWINFO, 0, (LPARAM)plvtvi);
	}

	EckInline HWND GetToolTips()
	{
		return (HWND)SendMsg(LVM_GETTOOLTIPS, 0, 0);
	}

	EckInline int GetTopIndex()
	{
		return (int)SendMsg(LVM_GETTOOLTIPS, 0, 0);
	}

	EckInline BOOL GetUnicodeFormat()
	{
		return (BOOL)SendMsg(LVM_GETUNICODEFORMAT, 0, 0);
	}

	/// <summary>
	/// 取当前视图
	/// </summary>
	/// <returns>LV_VIEW_常量</returns>
	EckInline UINT GetView()
	{
		return (UINT)SendMsg(LVM_GETVIEW, 0, 0);
	}

	EckInline BOOL GetViewRect(RECT* prc)
	{
		return (BOOL)SendMsg(LVM_GETVIEWRECT, 0, (LPARAM)prc);
	}

	EckInline std::vector<RECT> GetWorkAreas()
	{
		std::vector<RECT> aRect{};
		int c = GetNumberOfWorkAreas();
		if (!c)
			return aRect;
		aRect.resize(c);
		SendMsg(LVM_GETWORKAREAS, c, (LPARAM)aRect.data());
		return aRect;
	}

	EckInline void GetWorkAreas(int c, RECT* prc)
	{
		SendMsg(LVM_GETWORKAREAS, c, (LPARAM)prc);
	}

	EckInline BOOL HasGroup(int iGroupID)
	{
		return (BOOL)SendMsg(LVM_HASGROUP, iGroupID, 0);
	}

	EckInline int HitTest(LVHITTESTINFO* plvhti, BOOL bTestGroup = FALSE)
	{
		return (int)SendMsg(LVM_HITTEST, bTestGroup ? -1 : 0, (LPARAM)plvhti);
	}

	EckInline int InsertColumn(int idx, LVCOLUMNW* plvc)
	{
		return (int)SendMsg(LVM_INSERTCOLUMNW, idx, (LPARAM)plvc);
	}

	EckInline int InsertColumn(PCWSTR pszText, int idx = -1, int cxColumn = 160, int iFmt = LVCFMT_LEFT)
	{
		if (idx < 0)
			idx = GetHeaderCtrl().GetItemCount();
		LVCOLUMNW lvc;
		lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT;
		lvc.pszText = (PWSTR)pszText;
		lvc.cx = cxColumn;
		lvc.fmt = iFmt;
		return InsertColumn(idx, &lvc);
	}

	EckInline int InsertGroup(int idx, LVGROUP* plvg)
	{
		plvg->cbSize = sizeof(LVGROUP);
		return (int)SendMsg(LVM_INSERTGROUP, idx, (LPARAM)plvg);
	}

	EckInline int InsertGroupSorted(PFNLVGROUPCOMPARE pfnCmp, void* pData, LVINSERTGROUPSORTED* plvigs)
	{
		plvigs->pfnGroupCompare = pfnCmp;
		plvigs->pvData = pData;
		plvigs->lvGroup.cbSize = sizeof(LVGROUP);
		return (int)SendMsg(LVM_INSERTGROUPSORTED, 0, (LPARAM)plvigs);
	}

	EckInline int InsertItem(int idx, LVITEMW* plvi)
	{
		plvi->iItem = idx;
		return (int)SendMsg(LVM_INSERTITEM, 0, (LPARAM)plvi);
	}

	EckInline int InsertItem(PCWSTR pszText, int idx = -1, LPARAM lParam = 0, int idxImage = -1)
	{
		++idx;
		if (idx < 0)
			idx = GetItemCount();
		LVITEMW lvi;
		lvi.mask = LVIF_TEXT | LVIF_PARAM;
		lvi.lParam = lParam;
		lvi.pszText = (PWSTR)pszText;
		lvi.iSubItem = 0;
		if (idxImage >= 0)
		{
			lvi.mask |= LVIF_IMAGE;
			lvi.iImage = idxImage;
		}

		return InsertItem(idx, &lvi);
	}

	EckInline int InsertMarkHitTest(POINT pt, BOOL bAfterItem = FALSE)
	{
		LVINSERTMARK lvim{ sizeof(LVINSERTMARK) };
		lvim.iItem = -1;
		if (bAfterItem)
			lvim.dwFlags = LVIM_AFTER;
		SendMsg(LVM_INSERTMARKHITTEST, (WPARAM)&pt, (LPARAM)&lvim);
		return lvim.iItem;
	}

	EckInline BOOL IsGroupViewEnabled()
	{
		return (BOOL)SendMsg(LVM_ISGROUPVIEWENABLED, 0, 0);
	}

	EckInline BOOL IsItemVisible(int idx)
	{
		return (BOOL)SendMsg(LVM_ISITEMVISIBLE, idx, 0);
	}

	EckInline UINT MapIDToIndex(int idx)
	{
		return (UINT)SendMsg(LVM_MAPIDTOINDEX, idx, 0);
	}

	EckInline int MapIndexToID(UINT uID)
	{
		return (int)SendMsg(LVM_MAPINDEXTOID, uID, 0);
	}

	EckInline BOOL RedrawItems(int idxStart, int idxEnd)
	{
		return (BOOL)SendMsg(LVM_REDRAWITEMS, idxStart, idxEnd);
	}

	EckInline void RemoveGroup()
	{
		SendMsg(LVM_REMOVEALLGROUPS, NULL, NULL);
	}

	/// <summary>
	/// 删除组
	/// </summary>
	/// <param name="iGroupID">组ID</param>
	/// <returns>成功返回组索引，失败返回-1</returns>
	EckInline int RemoveGroup(int iGroupID)
	{
		return (int)SendMsg(LVM_REMOVEGROUP, iGroupID, NULL);
	}

	EckInline BOOL Scroll(int deltaH = 0, int deltaV = 0)
	{
		return (BOOL)SendMsg(LVM_SCROLL, deltaH, deltaV);
	}

	/// <summary>
	/// 置背景颜色
	/// </summary>
	/// <param name="cr">颜色，或CLR_NONE指定无背景色</param>
	/// <returns>成功返回TRUE，失败返回FALSE</returns>
	EckInline BOOL SetBKColor(COLORREF cr)
	{
		return (BOOL)SendMsg(LVM_SETBKCOLOR, 0, cr);
	}

	EckInline BOOL SetBKImage(LVBKIMAGEW* plvbki)
	{
		return (BOOL)SendMsg(LVM_SETBKIMAGEW, 0, (LPARAM)plvbki);
	}

	/// <summary>
	/// 置回调掩码
	/// </summary>
	/// <param name="dwMask">掩码，LVIS_常量</param>
	/// <returns>成功返回TRUE，失败返回FALSE</returns>
	EckInline BOOL SetCallbackMask(DWORD dwMask)
	{
		return (BOOL)SendMsg(LVM_SETCALLBACKMASK, dwMask, 0);
	}

	EckInline BOOL SetColumn(int idx, LVCOLUMNW* plvc)
	{
		return (BOOL)SendMsg(LVM_SETCOLUMNW, idx, (LPARAM)plvc);
	}

	EckInline BOOL SetColumnOrderArray(int* piOrder, int c)
	{
		return (BOOL)SendMsg(LVM_SETCOLUMNORDERARRAY, c, (LPARAM)piOrder);
	}

	/// <summary>
	/// 置列宽
	/// </summary>
	/// <param name="idx">索引，对于列表模式此参数必须设置为0</param>
	/// <param name="cx">宽度，或LVSCW_AUTOSIZE指定自动调整大小，或LVSCW_AUTOSIZE_USEHEADER指定适应标题文本</param>
	/// <returns>成功返回TRUE，失败返回FALSE</returns>
	EckInline BOOL SetColumnWidth(int idx, int cx)
	{
		return (BOOL)SendMsg(LVM_SETCOLUMNWIDTH, idx, cx);
	}

	/// <summary>
	/// 置列表视图扩展样式
	/// </summary>
	/// <param name="dwNew">新样式</param>
	/// <param name="dwMask">掩码，若为0则修改所有样式</param>
	/// <returns>返回旧样式</returns>
	EckInline DWORD SetLVExtendStyle(DWORD dwNew, DWORD dwMask = 0)
	{
		return (DWORD)SendMsg(LVM_SETEXTENDEDLISTVIEWSTYLE, dwMask, dwNew);
	}

	/// <summary>
	/// 置组信息
	/// </summary>
	/// <param name="iGroupID">组ID</param>
	/// <param name="plvg">LVGROUP指针</param>
	/// <returns>成功返回组ID，失败返回-1</returns>
	EckInline int SetGroup(int iGroupID, LVGROUP* plvg)
	{
		plvg->cbSize = sizeof(LVGROUP);
		return (int)SendMsg(LVM_SETGROUPINFO, iGroupID, (LPARAM)plvg);
	}

	EckInline void SetGroupMetrics(LVGROUPMETRICS* plvgm)
	{
		plvgm->cbSize = sizeof(LVGROUPMETRICS);
		SendMsg(LVM_SETGROUPMETRICS, 0, (LPARAM)plvgm);
	}

	EckInline HCURSOR SetHotCursor(HCURSOR hCursor)
	{
		return (HCURSOR)SendMsg(LVM_SETHOTCURSOR, 0, (LPARAM)hCursor);
	}

	EckInline int SetHotItem(int idx)
	{
		return (int)SendMsg(LVM_SETHOTITEM, idx, 0);
	}

	EckInline DWORD SetHoverTime(DWORD dwTime = (DWORD)-1)
	{
		return (DWORD)SendMsg(LVM_SETHOVERTIME, 0, dwTime);
	}

	/// <summary>
	/// 置大图标模式图标间距。
	/// 若xSpacing和ySpacing均设为-1，则使用默认间距
	/// </summary>
	/// <param name="xSpacing">水平间隔</param>
	/// <param name="ySpacing">垂直间隔</param>
	/// <returns>返回值的低字为先前的水平距离，高字为先前的垂直距离</returns>
	EckInline DWORD SetIconSpacing(int xSpacing = -1, int ySpacing = -1)
	{
		return (DWORD)SendMsg(LVM_SETICONSPACING, 0, MAKELPARAM(xSpacing, ySpacing));
	}

	EckInline HIMAGELIST SetImageList(HIMAGELIST hImageList, UINT uType = LVSIL_NORMAL)
	{
		return (HIMAGELIST)SendMsg(LVM_SETIMAGELIST, uType, (LPARAM)hImageList);
	}

	EckInline BOOL SetInfoTip(LVSETINFOTIP* plvsit)
	{
		plvsit->cbSize = sizeof(LVSETINFOTIP);
		return (BOOL)SendMsg(LVM_SETINFOTIP, 0, (LPARAM)plvsit);
	}

	EckInline BOOL SetInsertMark(int idx, BOOL bAfterItem = FALSE)
	{
		LVINSERTMARK lvim{ sizeof(LVINSERTMARK),DWORD(bAfterItem ? LVIM_AFTER : 0),idx };
		return (BOOL)SendMsg(LVM_SETINSERTMARK, 0, (LPARAM)&lvim);
	}

	EckInline COLORREF SetInsertMarkColor(COLORREF cr)
	{
		return (COLORREF)SendMsg(LVM_SETINSERTMARKCOLOR, 0, cr);
	}

	EckInline BOOL SetItem(int idx, int idxSubItem, LVITEMW* plvi)
	{
		plvi->iItem = idx;
		plvi->iSubItem = idxSubItem;
		return (BOOL)SendMsg(LVM_SETITEMW, 0, (LPARAM)plvi);
	}

	/// <summary>
	/// 置项目数。
	/// 若控件具有所有者数据样式则此方法设置项目数，否则指示控件为指定的项数分配内存
	/// </summary>
	/// <param name="iCount">项目数</param>
	/// <param name="uFlags">标志，LVSICF_常量</param>
	/// <returns>成功返回TRUE，失败返回FALSE</returns>
	EckInline BOOL SetItemCount(int iCount, UINT uFlags = 0)
	{
		return (BOOL)SendMsg(LVM_SETITEMCOUNT, iCount, uFlags);
	}

	EckInline BOOL SetItemState(int idx, int idxGroup, UINT uState, UINT uMask)
	{
		LVITEMW li;
		li.state = uState;
		li.stateMask = uMask;
		LVITEMINDEX lvii;
		lvii.iItem = idx;
		lvii.iGroup = idxGroup;
		return (BOOL)SendMsg(LVM_SETITEMINDEXSTATE, (WPARAM)&lvii, (LPARAM)&li);
	}

	EckInline BOOL SetItemState(int idx, UINT uState, UINT uMask)
	{
		LVITEMW li;
		li.state = uState;
		li.stateMask = uMask;
		return (BOOL)SendMsg(LVM_SETITEMSTATE, idx, (LPARAM)&li);
	}

	EckInline void SetItemPosition(int idx, POINT pt)
	{
		SendMsg(LVM_SETITEMPOSITION32, idx, (LPARAM)&pt);
	}

	EckInline BOOL SetItemText(int idx, int idxSubItem, PCWSTR pszText)
	{
		LVITEMW li;
		li.iSubItem = idxSubItem;
		li.pszText = (PWSTR)pszText;
		return (BOOL)SendMsg(LVM_SETITEMTEXTW, idx, (LPARAM)&li);
	}

	EckInline COLORREF SetOutLineColor(COLORREF cr)
	{
		return (COLORREF)SendMsg(LVM_SETOUTLINECOLOR, 0, cr);
	}

	EckInline void SetSelectedColumn(int idx)
	{
		SendMsg(LVM_SETSELECTEDCOLUMN, idx, 0);
	}

	EckInline int SetSelectionMark(int idx)
	{
		return (int)SendMsg(LVM_SETSELECTIONMARK, 0, idx);
	}

	EckInline COLORREF SetTextBKColor(COLORREF cr)
	{
		return (COLORREF)SendMsg(LVM_SETTEXTBKCOLOR, 0, cr);
	}

	EckInline COLORREF SetTextColor(COLORREF cr)
	{
		return (COLORREF)SendMsg(LVM_SETTEXTCOLOR, 0, cr);
	}

	EckInline BOOL SetTileInfo(int idx, UINT cColumn, UINT* piColumn, int* piFmt)
	{
		LVTILEINFO lvti{ sizeof(LVTILEINFO),idx,cColumn,piColumn,piFmt };
		return (BOOL)SendMsg(LVM_SETTILEINFO, 0, (LPARAM)&lvti);
	}

	EckInline BOOL SetTileInfo(int idx, LVTILEINFO* plvti)
	{
		plvti->iItem = idx;
		return (BOOL)SendMsg(LVM_SETTILEINFO, 0, (LPARAM)plvti);
	}

	EckInline BOOL SetTileViewInfo(LVTILEVIEWINFO* plvtvi)
	{
		return (BOOL)SendMsg(LVM_SETTILEVIEWINFO, 0, (LPARAM)plvtvi);
	}

	EckInline HWND SetToolTips(HWND hToolTip)
	{
		return (HWND)SendMsg(LVM_SETTOOLTIPS, (WPARAM)hToolTip, 0);
	}

	EckInline BOOL SetUnicodeFormat(BOOL bUnicode)
	{
		return (BOOL)SendMsg(LVM_SETUNICODEFORMAT, bUnicode, 0);
	}

	EckInline BOOL SetView(DWORD dwView)
	{
		return (SendMsg(LVM_SETVIEW, dwView, 0) > 0);
	}

	EckInline void SetWorkArea(RECT* prc, int c)
	{
		SendMsg(LVM_SETWORKAREAS, c, (LPARAM)prc);
	}

	/// <summary>
	/// 排序组
	/// </summary>
	/// <param name="pfnCmp">排序函数。第一、二个参数为两组的ID，若组1大于组2则返回正值，若小于则返回负值，若等于则返回0</param>
	/// <param name="pData">自定义数据</param>
	/// <returns>成功返回TRUE，失败返回FALSE</returns>
	EckInline BOOL SortGroups(PFNLVGROUPCOMPARE pfnCmp, void* pData)
	{
		return (BOOL)SendMsg(LVM_SORTGROUPS, (WPARAM)pfnCmp, (LPARAM)pData);
	}

	/// <summary>
	/// 排序项目
	/// </summary>
	/// <param name="pfnCmp">排序函数。第一、二个参数为两项目的lParam，若项目1在项目2之后则返回正值，若在之前则返回负值，若相等则返回0</param>
	/// <param name="pData"></param>
	/// <returns></returns>
	EckInline BOOL SortItemslParam(PFNLVITEMCOMPARE pfnCmp, LPARAM lParam)
	{
		return (BOOL)SendMsg(LVM_SORTITEMS, lParam, (LPARAM)pfnCmp);
	}

	/// <summary>
	/// 排序项目
	/// </summary>
	/// <param name="pfnCmp">排序函数。第一、二个参数为两项目的索引，若项目1在项目2之后则返回正值，若在之前则返回负值，若相等则返回0</param>
	/// <param name="pData"></param>
	/// <returns></returns>
	EckInline BOOL SortItemsIndex(PFNLVITEMCOMPAREEX pfnCmp, LPARAM lParam)
	{
		return (BOOL)SendMsg(LVM_SORTITEMSEX, lParam, (LPARAM)pfnCmp);
	}

	EckInline int SubItemHitTest(POINT pt, LVHITTESTINFO* plvhti, BOOL bTestGroup = FALSE)
	{
		plvhti->pt = pt;
		return (int)SendMsg(LVM_SUBITEMHITTEST, bTestGroup ? -1 : 0, (LPARAM)plvhti);
	}

	EckInline BOOL Update()
	{
		return (BOOL)SendMsg(LVM_UPDATE, 0, 0);
	}
};
ECK_NAMESPACE_END