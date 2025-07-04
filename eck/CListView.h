﻿#pragma once
#include "CHeader.h"

ECK_NAMESPACE_BEGIN
using FLvItemCompare = int(CALLBACK*)(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
using FLvItemCompareEx = int(CALLBACK*)(int idx1, int idx2, LPARAM lParamSort);

#define ECK_CWNDPROP_LVE_STYLE(Name, Style)				\
	ECKPROP(StyleGet##Name, StyleSet##Name) BOOL Name;	\
	BOOL StyleGet##Name() const							\
	{													\
		if constexpr (Style == 0)						\
			return !GetLVExtendStyle();					\
		else											\
			return IsBitSet(GetLVExtendStyle(), Style);	\
	}													\
	void StyleSet##Name(BOOL b) const					\
	{													\
		SetLVExtendStyle(b ? Style : 0, Style);			\
	}

class CListView :public CWnd
{
public:
	ECK_RTTI(CListView);
	ECK_CWND_NOSINGLEOWNER(CListView);
	ECK_CWND_CREATE_CLS(WC_LISTVIEWW);

	ECK_CWNDPROP_STYLE(AlignLeft, LVS_ALIGNLEFT);
	ECK_CWNDPROP_STYLE(AlignTop, LVS_ALIGNTOP);
	ECK_CWNDPROP_STYLE(AutoArrange, LVS_AUTOARRANGE);
	ECK_CWNDPROP_STYLE(EditLabels, LVS_EDITLABELS);
	ECK_CWNDPROP_STYLE_MASK(ViewIcon, LVS_ICON, LVS_TYPEMASK);
	ECK_CWNDPROP_STYLE_MASK(ViewList, LVS_LIST, LVS_TYPEMASK);
	ECK_CWNDPROP_STYLE(NoColumnHeader, LVS_NOCOLUMNHEADER);
	ECK_CWNDPROP_STYLE(NoLabelWrap, LVS_NOLABELWRAP);
	ECK_CWNDPROP_STYLE(NoScroll, LVS_NOSCROLL);
	ECK_CWNDPROP_STYLE(NoSortHeader, LVS_NOSORTHEADER);
	ECK_CWNDPROP_STYLE(OwnerData, LVS_OWNERDATA);
	ECK_CWNDPROP_STYLE_MASK(ViewReport, LVS_REPORT, LVS_TYPEMASK);
	ECK_CWNDPROP_STYLE(ShareImageLists, LVS_SHAREIMAGELISTS);
	ECK_CWNDPROP_STYLE(ShowSelAlways, LVS_SHOWSELALWAYS);
	ECK_CWNDPROP_STYLE(SingleSel, LVS_SINGLESEL);
	ECK_CWNDPROP_STYLE_MASK(ViewSmallIcon, LVS_SMALLICON, LVS_TYPEMASK);
	ECK_CWNDPROP_STYLE(SortAscending, LVS_SORTASCENDING);
	ECK_CWNDPROP_STYLE(SortDescending, LVS_SORTDESCENDING);
	ECK_CWNDPROP_LVE_STYLE(AutoAutoArrange, LVS_EX_AUTOAUTOARRANGE);
	ECK_CWNDPROP_LVE_STYLE(AutoCheckSelect, LVS_EX_AUTOCHECKSELECT);
	ECK_CWNDPROP_LVE_STYLE(AutoSizeColumns, LVS_EX_AUTOSIZECOLUMNS);
	ECK_CWNDPROP_LVE_STYLE(BorderSelect, LVS_EX_BORDERSELECT);
	ECK_CWNDPROP_LVE_STYLE(Checkboxes, LVS_EX_CHECKBOXES);
	ECK_CWNDPROP_LVE_STYLE(ColumnOverflow, LVS_EX_COLUMNOVERFLOW);
	ECK_CWNDPROP_LVE_STYLE(ColumnSnapPoints, LVS_EX_COLUMNSNAPPOINTS);
	ECK_CWNDPROP_LVE_STYLE(DoubleBuffer, LVS_EX_DOUBLEBUFFER);
	ECK_CWNDPROP_LVE_STYLE(FlatSB, LVS_EX_FLATSB);
	ECK_CWNDPROP_LVE_STYLE(FullRowSelect, LVS_EX_FULLROWSELECT);
	ECK_CWNDPROP_LVE_STYLE(GridLines, LVS_EX_GRIDLINES);
	ECK_CWNDPROP_LVE_STYLE(HeaderDragDrop, LVS_EX_HEADERDRAGDROP);
	ECK_CWNDPROP_LVE_STYLE(HeaderInAllViews, LVS_EX_HEADERINALLVIEWS);
	ECK_CWNDPROP_LVE_STYLE(HideLabels, LVS_EX_HIDELABELS);
	ECK_CWNDPROP_LVE_STYLE(InfoTip, LVS_EX_INFOTIP);
	ECK_CWNDPROP_LVE_STYLE(JustifyColumns, LVS_EX_JUSTIFYCOLUMNS);
	ECK_CWNDPROP_LVE_STYLE(LabelTip, LVS_EX_LABELTIP);
	ECK_CWNDPROP_LVE_STYLE(MultiWorkAreas, LVS_EX_MULTIWORKAREAS);
	ECK_CWNDPROP_LVE_STYLE(OneClickActivate, LVS_EX_ONECLICKACTIVATE);
	ECK_CWNDPROP_LVE_STYLE(Regional, LVS_EX_REGIONAL);
	ECK_CWNDPROP_LVE_STYLE(SimpleSelect, LVS_EX_SIMPLESELECT);
	ECK_CWNDPROP_LVE_STYLE(SnapToGrid, LVS_EX_SNAPTOGRID);
	ECK_CWNDPROP_LVE_STYLE(SubItemImages, LVS_EX_SUBITEMIMAGES);
	ECK_CWNDPROP_LVE_STYLE(TrackSelect, LVS_EX_TRACKSELECT);
	ECK_CWNDPROP_LVE_STYLE(TransparentBk, LVS_EX_TRANSPARENTBKGND);
	ECK_CWNDPROP_LVE_STYLE(TransparentShadowText, LVS_EX_TRANSPARENTSHADOWTEXT);
	ECK_CWNDPROP_LVE_STYLE(TwoClickActivate, LVS_EX_TWOCLICKACTIVATE);
	ECK_CWNDPROP_LVE_STYLE(UnderlineCold, LVS_EX_UNDERLINECOLD);
	ECK_CWNDPROP_LVE_STYLE(UnderlineHot, LVS_EX_UNDERLINEHOT);
protected:
	BOOL m_bAutoDarkMode{ TRUE };
public:
	EckInline constexpr void SetAutoDarkMode(BOOL b) { m_bAutoDarkMode = b; }

	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg)
		{
		case WM_NOTIFY:
		{
			if (m_bAutoDarkMode && ShouldAppsUseDarkMode())
				switch (((NMHDR*)lParam)->code)
				{
				case NM_CUSTOMDRAW:
				{
					const auto pnmcd = (NMCUSTOMDRAW*)lParam;
					switch (pnmcd->dwDrawStage)
					{
					case CDDS_PREPAINT:
						return CDRF_NOTIFYITEMDRAW;
					case CDDS_ITEMPREPAINT:
						SetTextColor(pnmcd->hdc, GetThreadCtx()->crDefText);
						return CDRF_DODEFAULT;
					}
				}
				return CDRF_DODEFAULT;
				}
		}
		break;
		case WM_CREATE:
		case WM_THEMECHANGED:
		{
			if (m_bAutoDarkMode)
			{
				const auto lResult = CWnd::OnMsg(hWnd, uMsg, wParam, lParam);
				const auto* const ptc = GetThreadCtx();
				SetTextClr(ptc->crDefText);
				SetBkClr(ptc->crDefBkg);
				SetTextBKClr(ptc->crDefBkg);
				return lResult;
			}
		}
		break;
		}
		return CWnd::OnMsg(hWnd, uMsg, wParam, lParam);
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
		int* piApprWidth = nullptr, int* piApprHeight = nullptr) const
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
	EckInline BOOL Arrange(UINT uFlag) const
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

	EckInline void CancelEditLabel() const
	{
		SendMsg(LVM_CANCELEDITLABEL, 0, 0);
	}

	/// <summary>
	/// 创建拖放图像
	/// </summary>
	/// <param name="idx">项目</param>
	/// <param name="ptOrg">图像左上角的初始位置</param>
	/// <returns>图像列表句柄</returns>
	EckInline HIMAGELIST CreateDragImage(int idx, POINT ptOrg = {}) const
	{
		return (HIMAGELIST)SendMsg(LVM_CREATEDRAGIMAGE, idx, (LPARAM)&ptOrg);
	}

	EckInline BOOL DeleteAllItems() const
	{
		return (BOOL)SendMsg(LVM_DELETEALLITEMS, 0, 0);
	}

	EckInline BOOL DeleteColumn(int idx) const
	{
		return (BOOL)SendMsg(LVM_DELETECOLUMN, idx, 0);
	}

	EckInline BOOL DeleteItem(int idx) const
	{
		return (BOOL)SendMsg(LVM_DELETEITEM, idx, 0);
	}

	/// <summary>
	/// 进入编辑
	/// </summary>
	/// <param name="idx">项目，若为-1则取消编辑</param>
	/// <returns>成功返回编辑框句柄，失败返回NULL。编辑框将在编辑结束后失效</returns>
	EckInline HWND EditLabel(int idx) const
	{
		return (HWND)SendMsg(LVM_EDITLABELW, idx, 0);
	}

	/// <summary>
	/// 启用/禁用分组视图
	/// </summary>
	/// <param name="bEnable">是否启用</param>
	/// <returns>0 - 成功  1 - 控件状态已更改  -1 - 失败</returns>
	EckInline int EnableGroupView(BOOL bEnable) const
	{
		return (int)SendMsg(LVM_ENABLEGROUPVIEW, bEnable, 0);
	}

	/// <summary>
	/// 保证显示
	/// </summary>
	/// <param name="idx">项目</param>
	/// <param name="bFullVisible">是否保证完全可见</param>
	/// <returns>成功返回TRUE，失败返回FALSE</returns>
	EckInline BOOL EnsureVisible(int idx, BOOL bFullVisible = TRUE) const
	{
		return (BOOL)SendMsg(LVM_ENSUREVISIBLE, idx, bFullVisible);
	}

	/// <summary>
	/// 寻找项目
	/// </summary>
	/// <param name="idxStart">起始项目，若为-1则从头开始寻找</param>
	/// <param name="plvfi">LVFINDINFOW指针</param>
	/// <returns>成功返回项目索引，失败返回-1</returns>
	EckInline int FindItem(int idxStart, LVFINDINFOW* plvfi) const
	{
		return (int)SendMsg(LVM_FINDITEMW, idxStart, (LPARAM)plvfi);
	}

	EckInline COLORREF GetBKColor() const
	{
		return (COLORREF)SendMsg(LVM_GETBKCOLOR, 0, 0);
	}

	EckInline BOOL GetBKImage(LVBKIMAGEW* plvbki) const
	{
		return (BOOL)SendMsg(LVM_GETBKIMAGEW, 0, (LPARAM)plvbki);
	}

	EckInline BOOL GetCallbackMask() const
	{
		return (BOOL)SendMsg(LVM_GETCALLBACKMASK, 0, 0);
	}

	EckInline BOOL GetColumn(int idx, LVCOLUMNW* plvc) const
	{
		return (BOOL)SendMsg(LVM_GETCALLBACKMASK, idx, (LPARAM)plvc);
	}

	EckInline BOOL GetColumnOrderArray(int cColumn, int* piOrder) const
	{
		return (BOOL)SendMsg(LVM_GETCOLUMNORDERARRAY, cColumn, (LPARAM)piOrder);
	}

	/// <summary>
	/// 取列顺序
	/// </summary>
	/// <param name="vOrder">将列顺序尾插到此参数</param>
	/// <returns>尾插的数量</returns>
	EckInline int GetColumnOrderArray(std::vector<int>& vOrder) const
	{
		const int cColumn = (int)GetHeaderCtrl().GetItemCount();
		if (!cColumn)
			return 0;
		const auto cOld = vOrder.size();
		vOrder.resize(cOld + cColumn);
		GetColumnOrderArray(cColumn, vOrder.data() + cOld);
		return cColumn;
	}

	EckInline int GetColumnWidth(int idx) const
	{
		return (int)SendMsg(LVM_GETCOLUMNWIDTH, idx, 0);
	}

	EckInline int GetCountPerPage() const
	{
		return (int)SendMsg(LVM_GETCOUNTPERPAGE, 0, 0);
	}

	EckInline HWND GetEditCtrl() const
	{
		return (HWND)SendMsg(LVM_GETEDITCONTROL, 0, 0);
	}

	EckInline BOOL GetEmptyText(PWSTR pszBuf, int cchBuf) const
	{
		return (BOOL)SendMsg(LVM_GETEMPTYTEXT, cchBuf, (LPARAM)pszBuf);
	}

	EckInline DWORD GetLVExtendStyle() const
	{
		return (DWORD)SendMsg(LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0);
	}

	EckInline int GetFocusedGroup() const
	{
		return (int)SendMsg(LVM_GETFOCUSEDGROUP, 0, 0);
	}

	EckInline int GetFooterItemCount() const
	{
		LVFOOTERINFO lvfi{ LVFF_ITEMCOUNT };
		SendMsg(LVM_GETFOOTERINFO, 0, (LPARAM)&lvfi);
		return lvfi.cItems;
	}

	EckInline BOOL GetFooterItem(int idx, LVFOOTERITEM* plvfi) const
	{
		return (BOOL)SendMsg(LVM_GETFOOTERITEM, idx, (LPARAM)plvfi);
	}

	EckInline BOOL GetFooterItemRect(int idx, RECT* prc) const
	{
		return (BOOL)SendMsg(LVM_GETFOOTERITEMRECT, idx, (LPARAM)prc);
	}

	EckInline BOOL GetFooterRect(RECT* prc) const
	{
		return (BOOL)SendMsg(LVM_GETFOOTERRECT, 0, (LPARAM)prc);
	}

	EckInline int GetGroupCount() const
	{
		return (int)SendMsg(LVM_GETGROUPCOUNT, 0, 0);
	}

	EckInline int GetGroup(int iGroupID, LVGROUP* plvg) const
	{
		return (int)SendMsg(LVM_GETGROUPINFO, iGroupID, (LPARAM)plvg);
	}

	EckInline int GetGroupByIndex(int idx, LVGROUP* plvg) const
	{
		return (int)SendMsg(LVM_GETGROUPINFOBYINDEX, idx, (LPARAM)plvg);
	}

	EckInline void GetGroupMetrics(LVGROUPMETRICS* plvgm) const
	{
		SendMsg(LVM_GETGROUPMETRICS, 0, (LPARAM)plvgm);
	}

	/// <summary>
	/// 取组矩形
	/// </summary>
	/// <param name="iGroupID">组ID</param>
	/// <param name="prc">接收矩形</param>
	/// <param name="uType">类型，LVGGR_常量，默认LVGGR_GROUP</param>
	/// <returns></returns>
	EckInline BOOL GetGroupRect(int iGroupID, RECT* prc, UINT uType = LVGGR_GROUP) const
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
	EckInline UINT GetGroupState(int iGroupID, UINT uState) const
	{
		return (UINT)SendMsg(LVM_GETGROUPSTATE, iGroupID, uState);
	}

	EckInline CHeader GetHeaderCtrl() const
	{
		return CHeader((HWND)SendMsg(LVM_GETHEADER, 0, 0));
	}

	EckInline HWND GetHeaderCtrlHWnd() const
	{
		return (HWND)SendMsg(LVM_GETHEADER, 0, 0);
	}

	EckInline HCURSOR GetHotCursor() const
	{
		return (HCURSOR)SendMsg(LVM_GETHOTCURSOR, 0, 0);
	}

	EckInline int GetHotItem() const
	{
		return (int)SendMsg(LVM_GETHOTITEM, 0, 0);
	}

	EckInline DWORD GetHoverTime() const
	{
		return (DWORD)SendMsg(LVM_GETHOVERTIME, 0, 0);
	}

	/// <summary>
	/// 取图像列表
	/// </summary>
	/// <param name="uType">类型，LVSIL_常量，默认LVSIL_NORMAL</param>
	/// <returns>图像列表句柄</returns>
	EckInline HIMAGELIST GetImageList(UINT uType = LVSIL_NORMAL) const
	{
		return (HIMAGELIST)SendMsg(LVM_GETIMAGELIST, uType, 0);
	}

	EckInline int GetInsertMark(BOOL* pbAfterItem) const
	{
		LVINSERTMARK lvim{ sizeof(LVINSERTMARK) };
		if (!SendMsg(LVM_GETINSERTMARK, 0, (LPARAM)&lvim))
			return -1;
		if (pbAfterItem)
			*pbAfterItem = IsBitSet(lvim.dwFlags, LVIM_AFTER);
		return lvim.iItem;
	}

	EckInline COLORREF GetInsertMarkColor() const
	{
		return (COLORREF)SendMsg(LVM_GETINSERTMARKCOLOR, 0, 0);
	}

	EckInline BOOL GetInsertMarkRect(RECT* prc) const
	{
		return (BOOL)SendMsg(LVM_GETINSERTMARKRECT, 0, (LPARAM)prc);
	}

	EckInline int GetISearchString(PWSTR pszBuf) const
	{
		return (int)SendMsg(LVM_GETISEARCHSTRINGW, 0, (LPARAM)pszBuf);
	}

	EckInline BOOL GetItem(LVITEMW* pli) const
	{
		return (BOOL)SendMsg(LVM_GETITEMW, 0, (LPARAM)pli);
	}

	EckInline int GetItemCount() const
	{
		return (int)SendMsg(LVM_GETITEMCOUNT, 0, 0);
	}

	EckInline BOOL GetItemRect(int idx, int idxGroup, RECT* prc, UINT uType = LVIR_BOUNDS) const
	{
		LVITEMINDEX lvii;
		lvii.iItem = idx;
		lvii.iGroup = idxGroup;
		prc->left = uType;
		return (BOOL)SendMsg(LVM_GETITEMINDEXRECT, (WPARAM)&lvii, (LPARAM)prc);
	}

	EckInline BOOL GetItemRect(int idx, RECT* prc, UINT uType = LVIR_BOUNDS) const
	{
		prc->left = uType;
		return (BOOL)SendMsg(LVM_GETITEMRECT, idx, (LPARAM)prc);
	}

	EckInline BOOL GetItemPosition(int idx, POINT* ppt) const
	{
		return (BOOL)SendMsg(LVM_GETITEMPOSITION, idx, (LPARAM)ppt);
	}

	EckInline void GetItemSpacing(BOOL bSmallIconView, int* pxSpacing = nullptr, int* pySpacing = nullptr) const
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
	EckInline UINT GetItemState(int idx, UINT uState) const
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
	EckInline int GetItemText(int idx, int idxSubItem, PWSTR pszBuf, int cchBuf) const
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
	EckInline CRefStrW GetItemText(int idx, int idxSubItem) const
	{
		CRefStrW rs;
		rs.ReSize(MAX_PATH - 1);
		LVITEMW li;
		li.iSubItem = idxSubItem;
		li.pszText = rs.Data();
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
	EckInline int GetNextItem(int idx, UINT uFlags) const
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
	EckInline int GetNextItem(int idx, int idxGroup, UINT uFlags) const
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

	EckInline int GetNumberOfWorkAreas() const
	{
		UINT u = 0;
		SendMsg(LVM_GETNUMBEROFWORKAREAS, 0, (LPARAM)&u);
		return u;
	}

	EckInline BOOL GetOrigin(POINT* ppt) const
	{
		return (BOOL)SendMsg(LVM_GETORIGIN, 0, (LPARAM)ppt);
	}

	EckInline COLORREF GetOutLineColor() const
	{
		return (COLORREF)SendMsg(LVM_GETOUTLINECOLOR, 0, 0);
	}

	EckInline int GetSelectedColumn() const
	{
		return (int)SendMsg(LVM_GETSELECTEDCOLUMN, 0, 0);
	}

	EckInline int GetSelectedCount() const
	{
		return (int)SendMsg(LVM_GETSELECTEDCOUNT, 0, 0);
	}

	EckInline int GetSelectionMark() const
	{
		return (int)SendMsg(LVM_GETSELECTIONMARK, 0, 0);
	}

	EckInline int GetStringWidth(PCWSTR pszText) const
	{
		return (int)SendMsg(LVM_GETSTRINGWIDTHW, 0, (LPARAM)pszText);
	}

	EckInline int GetSubItemRect(int idx, int idxSubItem, RECT* prc, UINT uType = LVIR_BOUNDS) const
	{
		prc->top = idxSubItem;
		prc->left = uType;
		return (int)SendMsg(LVM_GETSUBITEMRECT, idx, (LPARAM)prc);
	}

	EckInline COLORREF GetTextBKColor() const
	{
		return (COLORREF)SendMsg(LVM_GETTEXTBKCOLOR, 0, 0);
	}

	EckInline COLORREF GetTextColor() const
	{
		return (COLORREF)SendMsg(LVM_GETTEXTCOLOR, 0, 0);
	}

	EckInline void GetTileInfo(LVTILEINFO* plvti) const
	{
		plvti->cbSize = sizeof(LVTILEINFO);
		SendMsg(LVM_GETTILEINFO, 0, (LPARAM)plvti);
	}

	EckInline void GetTileViewInfo(LVTILEVIEWINFO* plvtvi) const
	{
		plvtvi->cbSize = sizeof(LVTILEVIEWINFO);
		SendMsg(LVM_GETTILEVIEWINFO, 0, (LPARAM)plvtvi);
	}

	EckInline HWND GetToolTips() const
	{
		return (HWND)SendMsg(LVM_GETTOOLTIPS, 0, 0);
	}

	EckInline int GetTopIndex() const
	{
		return (int)SendMsg(LVM_GETTOOLTIPS, 0, 0);
	}

	EckInline BOOL GetUnicodeFormat() const
	{
		return (BOOL)SendMsg(LVM_GETUNICODEFORMAT, 0, 0);
	}

	/// <summary>
	/// 取当前视图
	/// </summary>
	/// <returns>LV_VIEW_常量</returns>
	EckInline UINT GetView() const
	{
		return (UINT)SendMsg(LVM_GETVIEW, 0, 0);
	}

	EckInline BOOL GetViewRect(RECT* prc) const
	{
		return (BOOL)SendMsg(LVM_GETVIEWRECT, 0, (LPARAM)prc);
	}

	EckInline std::vector<RECT> GetWorkAreas() const
	{
		std::vector<RECT> aRect{};
		int c = GetNumberOfWorkAreas();
		if (!c)
			return aRect;
		aRect.resize(c);
		SendMsg(LVM_GETWORKAREAS, c, (LPARAM)aRect.data());
		return aRect;
	}

	EckInline void GetWorkAreas(int c, RECT* prc) const
	{
		SendMsg(LVM_GETWORKAREAS, c, (LPARAM)prc);
	}

	EckInline BOOL HasGroup(int iGroupID) const
	{
		return (BOOL)SendMsg(LVM_HASGROUP, iGroupID, 0);
	}

	EckInline int HitTest(LVHITTESTINFO* plvhti, BOOL bTestGroup = FALSE) const
	{
		return (int)SendMsg(LVM_HITTEST, bTestGroup ? -1 : 0, (LPARAM)plvhti);
	}

	EckInline int InsertColumn(int idx, LVCOLUMNW* plvc) const
	{
		return (int)SendMsg(LVM_INSERTCOLUMNW, idx, (LPARAM)plvc);
	}

	EckInline int InsertColumn(PCWSTR pszText, int idx = -1,
		int cxColumn = 160, int iFmt = LVCFMT_LEFT, int idxImage = -1) const
	{
		if (idx < 0)
			idx = INT_MAX;
		LVCOLUMNW lvc;
		lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT | LVCF_IMAGE;
		lvc.pszText = (PWSTR)pszText;
		lvc.cx = cxColumn;
		lvc.fmt = iFmt;
		lvc.iImage = idxImage;
		return InsertColumn(idx, &lvc);
	}

	EckInline int InsertGroup(int idx, LVGROUP* plvg) const
	{
		plvg->cbSize = sizeof(LVGROUP);
		return (int)SendMsg(LVM_INSERTGROUP, idx, (LPARAM)plvg);
	}

	EckInline int InsertGroupSorted(PFNLVGROUPCOMPARE pfnCmp, void* pData, LVINSERTGROUPSORTED* plvigs) const
	{
		plvigs->pfnGroupCompare = pfnCmp;
		plvigs->pvData = pData;
		plvigs->lvGroup.cbSize = sizeof(LVGROUP);
		return (int)SendMsg(LVM_INSERTGROUPSORTED, 0, (LPARAM)plvigs);
	}

	EckInline int InsertItem(int idx, LVITEMW* plvi) const
	{
		plvi->iItem = idx;
		return (int)SendMsg(LVM_INSERTITEM, 0, (LPARAM)plvi);
	}

	EckInline int InsertItem(LVITEMW* plvi) const
	{
		return (int)SendMsg(LVM_INSERTITEM, 0, (LPARAM)plvi);
	}

	EckInline int InsertItem(PCWSTR pszText, int idx = -1, LPARAM lParam = 0, int idxImage = -1) const
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

	EckInline int InsertMarkHitTest(POINT pt, BOOL bAfterItem = FALSE) const
	{
		LVINSERTMARK lvim{ sizeof(LVINSERTMARK) };
		lvim.iItem = -1;
		if (bAfterItem)
			lvim.dwFlags = LVIM_AFTER;
		SendMsg(LVM_INSERTMARKHITTEST, (WPARAM)&pt, (LPARAM)&lvim);
		return lvim.iItem;
	}

	EckInline BOOL IsGroupViewEnabled() const
	{
		return (BOOL)SendMsg(LVM_ISGROUPVIEWENABLED, 0, 0);
	}

	EckInline BOOL IsItemVisible(int idx) const
	{
		return (BOOL)SendMsg(LVM_ISITEMVISIBLE, idx, 0);
	}

	EckInline UINT MapIDToIndex(int idx) const
	{
		return (UINT)SendMsg(LVM_MAPIDTOINDEX, idx, 0);
	}

	EckInline int MapIndexToID(UINT uID) const
	{
		return (int)SendMsg(LVM_MAPINDEXTOID, uID, 0);
	}

	EckInline BOOL RedrawItems(int idxStart, int idxEnd) const
	{
		return (BOOL)SendMsg(LVM_REDRAWITEMS, idxStart, idxEnd);
	}

	EckInline BOOL RedrawItem(int idx) const
	{
		return RedrawItems(idx, idx);
	}

	EckInline void RemoveGroup() const
	{
		SendMsg(LVM_REMOVEALLGROUPS, 0, 0);
	}

	/// <summary>
	/// 删除组
	/// </summary>
	/// <param name="iGroupID">组ID</param>
	/// <returns>成功返回组索引，失败返回-1</returns>
	EckInline int RemoveGroup(int iGroupID) const
	{
		return (int)SendMsg(LVM_REMOVEGROUP, iGroupID, 0);
	}

	EckInline BOOL Scroll(int deltaH = 0, int deltaV = 0) const
	{
		return (BOOL)SendMsg(LVM_SCROLL, deltaH, deltaV);
	}

	/// <summary>
	/// 置背景颜色
	/// </summary>
	/// <param name="cr">颜色，或CLR_NONE指定无背景色</param>
	/// <returns>成功返回TRUE，失败返回FALSE</returns>
	EckInline BOOL SetBkClr(COLORREF cr) const
	{
		return (BOOL)SendMsg(LVM_SETBKCOLOR, 0, cr);
	}

	EckInline BOOL SetBkImage(LVBKIMAGEW* plvbki) const
	{
		return (BOOL)SendMsg(LVM_SETBKIMAGEW, 0, (LPARAM)plvbki);
	}

	/// <summary>
	/// 置回调掩码
	/// </summary>
	/// <param name="dwMask">掩码，LVIS_常量</param>
	/// <returns>成功返回TRUE，失败返回FALSE</returns>
	EckInline BOOL SetCallbackMask(DWORD dwMask) const
	{
		return (BOOL)SendMsg(LVM_SETCALLBACKMASK, dwMask, 0);
	}

	EckInline BOOL SetColumn(int idx, LVCOLUMNW* plvc) const
	{
		return (BOOL)SendMsg(LVM_SETCOLUMNW, idx, (LPARAM)plvc);
	}

	EckInline BOOL SetColumnOrderArray(int* piOrder, int c) const
	{
		return (BOOL)SendMsg(LVM_SETCOLUMNORDERARRAY, c, (LPARAM)piOrder);
	}

	/// <summary>
	/// 置列宽
	/// </summary>
	/// <param name="idx">索引，对于列表模式此参数必须设置为0</param>
	/// <param name="cx">宽度，或LVSCW_AUTOSIZE指定自动调整大小，或LVSCW_AUTOSIZE_USEHEADER指定适应标题文本</param>
	/// <returns>成功返回TRUE，失败返回FALSE</returns>
	EckInline BOOL SetColumnWidth(int idx, int cx) const
	{
		return (BOOL)SendMsg(LVM_SETCOLUMNWIDTH, idx, cx);
	}

	/// <summary>
	/// 置列表视图扩展样式
	/// </summary>
	/// <param name="dwNew">新样式</param>
	/// <param name="dwMask">掩码，若为0则修改所有样式</param>
	/// <returns>返回旧样式</returns>
	EckInline DWORD SetLVExtendStyle(DWORD dwNew, DWORD dwMask = 0) const
	{
		return (DWORD)SendMsg(LVM_SETEXTENDEDLISTVIEWSTYLE, dwMask, dwNew);
	}

	/// <summary>
	/// 置组信息
	/// </summary>
	/// <param name="iGroupID">组ID</param>
	/// <param name="plvg">LVGROUP指针</param>
	/// <returns>成功返回组ID，失败返回-1</returns>
	EckInline int SetGroup(int iGroupID, LVGROUP* plvg) const
	{
		plvg->cbSize = sizeof(LVGROUP);
		return (int)SendMsg(LVM_SETGROUPINFO, iGroupID, (LPARAM)plvg);
	}

	EckInline void SetGroupMetrics(LVGROUPMETRICS* plvgm) const
	{
		plvgm->cbSize = sizeof(LVGROUPMETRICS);
		SendMsg(LVM_SETGROUPMETRICS, 0, (LPARAM)plvgm);
	}

	EckInline HCURSOR SetHotCursor(HCURSOR hCursor) const
	{
		return (HCURSOR)SendMsg(LVM_SETHOTCURSOR, 0, (LPARAM)hCursor);
	}

	EckInline int SetHotItem(int idx) const
	{
		return (int)SendMsg(LVM_SETHOTITEM, idx, 0);
	}

	EckInline DWORD SetHoverTime(DWORD dwTime = (DWORD)-1) const
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
	EckInline DWORD SetIconSpacing(int xSpacing = -1, int ySpacing = -1) const
	{
		return (DWORD)SendMsg(LVM_SETICONSPACING, 0, MAKELPARAM(xSpacing, ySpacing));
	}

	EckInline HIMAGELIST SetImageList(HIMAGELIST hImageList, UINT uType = LVSIL_NORMAL) const
	{
		return (HIMAGELIST)SendMsg(LVM_SETIMAGELIST, uType, (LPARAM)hImageList);
	}

	EckInline BOOL SetInfoTip(LVSETINFOTIP* plvsit) const
	{
		plvsit->cbSize = sizeof(LVSETINFOTIP);
		return (BOOL)SendMsg(LVM_SETINFOTIP, 0, (LPARAM)plvsit);
	}

	EckInline BOOL SetInsertMark(int idx, BOOL bAfterItem = FALSE) const
	{
		LVINSERTMARK lvim{ sizeof(LVINSERTMARK),DWORD(bAfterItem ? LVIM_AFTER : 0),idx };
		return (BOOL)SendMsg(LVM_SETINSERTMARK, 0, (LPARAM)&lvim);
	}

	EckInline BOOL SetInsertMark(LVINSERTMARK* plvim) const
	{
		return (BOOL)SendMsg(LVM_SETINSERTMARK, 0, (LPARAM)plvim);
	}

	EckInline COLORREF SetInsertMarkColor(COLORREF cr) const
	{
		return (COLORREF)SendMsg(LVM_SETINSERTMARKCOLOR, 0, cr);
	}

	EckInline BOOL SetItem(LVITEMW* plvi) const
	{
		return (BOOL)SendMsg(LVM_SETITEMW, 0, (LPARAM)plvi);
	}

	/// <summary>
	/// 置项目数。
	/// 若控件具有所有者数据样式则此方法设置项目数，否则指示控件为指定的项数分配内存
	/// </summary>
	/// <param name="iCount">项目数</param>
	/// <param name="uFlags">标志，LVSICF_常量</param>
	/// <returns>成功返回TRUE，失败返回FALSE</returns>
	EckInline BOOL SetItemCount(int iCount, UINT uFlags = 0) const
	{
		return (BOOL)SendMsg(LVM_SETITEMCOUNT, iCount, uFlags);
	}

	EckInline BOOL SetItemState(int idx, int idxGroup, UINT uState, UINT uMask) const
	{
		LVITEMW li;
		li.state = uState;
		li.stateMask = uMask;
		LVITEMINDEX lvii;
		lvii.iItem = idx;
		lvii.iGroup = idxGroup;
		return (BOOL)SendMsg(LVM_SETITEMINDEXSTATE, (WPARAM)&lvii, (LPARAM)&li);
	}

	EckInline BOOL SetItemState(LVITEMINDEX* plvii, LVITEMW* plvi) const
	{
		return (BOOL)SendMsg(LVM_SETITEMINDEXSTATE, (WPARAM)plvii, (LPARAM)plvi);
	}

	EckInline BOOL SetItemState(int idx, UINT uState, UINT uMask) const
	{
		LVITEMW lvi;
		lvi.state = uState;
		lvi.stateMask = uMask;
		return (BOOL)SendMsg(LVM_SETITEMSTATE, idx, (LPARAM)&lvi);
	}

	EckInline BOOL SetItemState(int idx, LVITEMW* plvi) const
	{
		return (BOOL)SendMsg(LVM_SETITEMSTATE, idx, (LPARAM)plvi);
	}

	EckInline void SetItemPosition(int idx, POINT pt) const
	{
		SendMsg(LVM_SETITEMPOSITION32, idx, (LPARAM)&pt);
	}

	EckInline BOOL SetItemText(int idx, int idxSubItem, PCWSTR pszText) const
	{
		LVITEMW li;
		li.iSubItem = idxSubItem;
		li.pszText = (PWSTR)pszText;
		return SetItemText(idx, &li);
	}

	EckInline BOOL SetItemText(int idx, PCWSTR pszText) const
	{
		LVITEMW li;
		li.iSubItem = 0;
		li.pszText = (PWSTR)pszText;
		return SetItemText(idx, &li);
	}

	EckInline BOOL SetItemText(int idx, LVITEMW* pli) const
	{
		return (BOOL)SendMsg(LVM_SETITEMTEXTW, idx, (LPARAM)pli);
	}


	EckInline COLORREF SetOutLineColor(COLORREF cr) const
	{
		return (COLORREF)SendMsg(LVM_SETOUTLINECOLOR, 0, cr);
	}

	EckInline void SetSelectedColumn(int idx) const
	{
		SendMsg(LVM_SETSELECTEDCOLUMN, idx, 0);
	}

	EckInline int SetSelectionMark(int idx) const
	{
		return (int)SendMsg(LVM_SETSELECTIONMARK, 0, idx);
	}

	EckInline COLORREF SetTextBKClr(COLORREF cr) const
	{
		return (COLORREF)SendMsg(LVM_SETTEXTBKCOLOR, 0, cr);
	}

	EckInline COLORREF SetTextClr(COLORREF cr) const
	{
		return (COLORREF)SendMsg(LVM_SETTEXTCOLOR, 0, cr);
	}

	EckInline BOOL SetTileInfo(int idx, UINT cColumn, UINT* piColumn, int* piFmt) const
	{
		LVTILEINFO lvti{ sizeof(LVTILEINFO),idx,cColumn,piColumn,piFmt };
		return (BOOL)SendMsg(LVM_SETTILEINFO, 0, (LPARAM)&lvti);
	}

	EckInline BOOL SetTileInfo(LVTILEINFO* plvti) const
	{
		return (BOOL)SendMsg(LVM_SETTILEINFO, 0, (LPARAM)plvti);
	}

	EckInline BOOL SetTileViewInfo(LVTILEVIEWINFO* plvtvi) const
	{
		return (BOOL)SendMsg(LVM_SETTILEVIEWINFO, 0, (LPARAM)plvtvi);
	}

	EckInline HWND SetToolTips(HWND hToolTip) const
	{
		return (HWND)SendMsg(LVM_SETTOOLTIPS, (WPARAM)hToolTip, 0);
	}

	EckInline BOOL SetUnicodeFormat(BOOL bUnicode) const
	{
		return (BOOL)SendMsg(LVM_SETUNICODEFORMAT, bUnicode, 0);
	}

	EckInline BOOL SetView(DWORD dwView) const
	{
		return (SendMsg(LVM_SETVIEW, dwView, 0) > 0);
	}

	EckInline void SetWorkArea(RECT* prc, int c) const
	{
		SendMsg(LVM_SETWORKAREAS, c, (LPARAM)prc);
	}

	/// <summary>
	/// 排序组
	/// </summary>
	/// <param name="pfnCmp">排序函数。第一、二个参数为两组的ID，若组1大于组2则返回正值，若小于则返回负值，若等于则返回0</param>
	/// <param name="pData">自定义数据</param>
	/// <returns>成功返回TRUE，失败返回FALSE</returns>
	EckInline BOOL SortGroups(PFNLVGROUPCOMPARE pfnCmp, void* pData) const
	{
		return (BOOL)SendMsg(LVM_SORTGROUPS, (WPARAM)pfnCmp, (LPARAM)pData);
	}

	/// <summary>
	/// 排序项目
	/// </summary>
	/// <param name="pfnCmp">排序函数。第一、二个参数为两项目的lParam，若项目1在项目2之后则返回正值，若在之前则返回负值，若相等则返回0</param>
	/// <param name="pData"></param>
	/// <returns></returns>
	EckInline BOOL SortItemslParam(FLvItemCompare pfnCmp, LPARAM lParam) const
	{
		return (BOOL)SendMsg(LVM_SORTITEMS, lParam, (LPARAM)pfnCmp);
	}

	/// <summary>
	/// 排序项目
	/// </summary>
	/// <param name="pfnCmp">排序函数。第一、二个参数为两项目的索引，若项目1在项目2之后则返回正值，若在之前则返回负值，若相等则返回0</param>
	/// <param name="pData"></param>
	/// <returns></returns>
	EckInline BOOL SortItemsIndex(FLvItemCompareEx pfnCmp, LPARAM lParam) const
	{
		return (BOOL)SendMsg(LVM_SORTITEMSEX, lParam, (LPARAM)pfnCmp);
	}

	EckInline int SubItemHitTest(LVHITTESTINFO* plvhti, BOOL bTestGroup = FALSE) const
	{
		return (int)SendMsg(LVM_SUBITEMHITTEST, bTestGroup ? -1 : 0, (LPARAM)plvhti);
	}

	EckInline BOOL Update() const
	{
		return (BOOL)SendMsg(LVM_UPDATE, 0, 0);
	}

	EckInline int GetCurrSel() const
	{
		EckCounter(GetItemCount(), i)
		{
			if (GetItemState(i, LVIS_SELECTED) == LVIS_SELECTED)
				return i;
		}
		return -1;
	}

	BOOL SetRowHeight(int cy, BOOL bSetOrAdd = TRUE) const
	{
		const auto pParent = CWndFromHWND(GetParent(HWnd));
		if (!pParent)
			return FALSE;
		const auto hSlot = pParent->GetSignal().Connect(
			[cy, bSetOrAdd](HWND, UINT uMsg, WPARAM, LPARAM lParam, SlotCtx& Ctx)->LRESULT
			{
				if (uMsg == WM_MEASUREITEM)
				{
					Ctx.Processed();
					const auto pmis = (MEASUREITEMSTRUCT*)lParam;
					if (bSetOrAdd)
						pmis->itemHeight = cy;
					else
						pmis->itemHeight += cy;
					return TRUE;
				}
				return 0;
			}, MHI_LISTVIEW_ROWHEIGHT);

		Style |= LVS_OWNERDRAWFIXED;
		WINDOWPOS wp
		{
			.hwnd = HWnd,
			.flags = SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW
		};
		SendMsg(WM_WINDOWPOSCHANGED, 0, (LPARAM)&wp);
		Style &= ~LVS_OWNERDRAWFIXED;
		pParent->GetSignal().Disconnect(hSlot);
		return TRUE;
	}

	BOOL MakePretty_RowHeight() const
	{
		return SetRowHeight(DpiScale(6, GetDpi(HWnd)), FALSE);
	}

	BOOL MakePretty() const
	{
		constexpr DWORD dwStyle = LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER;
		SetLVExtendStyle(dwStyle, dwStyle);
		return MakePretty_RowHeight();
	}

	int GetItemCheckState(int idx) const
	{
		return (GetItemState(idx, LVIS_STATEIMAGEMASK) >> 12) - 1;
	}

	BOOL SetItemCheckState(int idx, int iCheck) const
	{
		return SetItemState(idx, INDEXTOSTATEIMAGEMASK(iCheck + 1), LVIS_STATEIMAGEMASK);
	}

	EckInline [[nodiscard]] IListView2* GetLvObject() const
	{
		IListView2* pLv{};
		SendMsg(LVM_QUERYINTERFACE, (WPARAM)&IID_IListView2, (LPARAM)&pLv);
		return pLv;
	}

	EckInline LRESULT GetLvObject(REFIID riid, void** ppv) const
	{
		return SendMsg(LVM_QUERYINTERFACE, (WPARAM)&riid, (LPARAM)ppv);
	}

	EckInline void EnableSpacePartSelect(BOOL bEnable) const
	{
		const auto pLv = GetLvObject();
		pLv->SetSelectionFlags(1, bEnable ? 1 : 0);
		pLv->Release();
	}
};
ECK_RTTI_IMPL_BASE_INLINE(CListView, CWnd);
ECK_NAMESPACE_END