/*
* WinEzCtrlKit Library
*
* CTreeView.h ： 标准树视图
*
* Copyright(C) 2023 QingKong
*/
#pragma once
#include "CWnd.h"

#include <CommCtrl.h>


ECK_NAMESPACE_BEGIN
#define TVS_EX_ALL (/*TVS_EX_NOSINGLECOLLAPSE | */ /*该样式不建议使用*/ /*TVS_EX_MULTISELECT | */ /*该样式不被支持*/ \
	TVS_EX_DOUBLEBUFFER | TVS_EX_NOINDENTSTATE | TVS_EX_RICHTOOLTIP | TVS_EX_AUTOHSCROLL | \
	TVS_EX_FADEINOUTEXPANDOS | TVS_EX_PARTIALCHECKBOXES | TVS_EX_EXCLUSIONCHECKBOXES | \
	TVS_EX_DIMMEDCHECKBOXES | TVS_EX_DRAWIMAGEASYNC)

class CTreeView :public CWnd
{
public:
	ECK_CWND_CREATE;
	HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, HMENU hMenu, PCVOID pData = nullptr) override
	{
		return IntCreate(0, WC_TREEVIEWW, pszText, dwStyle,
			x, y, cx, cy, hParent, hMenu, nullptr, nullptr);
	}

	EckInline void SetCheckBox(BOOL bCheckBox)
	{
		ModifyStyle(bCheckBox ? TVS_CHECKBOXES : 0, TVS_CHECKBOXES);
	}

	EckInline BOOL GetCheckBox()
	{
		return IsBitSet(GetStyle(), TVS_CHECKBOXES);
	}

	EckInline void SetDisableDragDrop(BOOL bDisableDragDrop)
	{
		ModifyStyle(bDisableDragDrop ? TVS_DISABLEDRAGDROP : 0, TVS_DISABLEDRAGDROP);
	}

	EckInline BOOL GetDisableDragDrop()
	{
		return IsBitSet(GetStyle(), TVS_DISABLEDRAGDROP);
	}

	EckInline void SetEditLabel(BOOL bEditLabel)
	{
		ModifyStyle(bEditLabel ? TVS_EDITLABELS : 0, TVS_EDITLABELS);
	}

	EckInline BOOL GetEditLabel()
	{
		return IsBitSet(GetStyle(), TVS_EDITLABELS);
	}

	EckInline void SetFullRowSelect(BOOL bFullRowSelect)
	{
		ModifyStyle(bFullRowSelect ? TVS_FULLROWSELECT : 0, TVS_FULLROWSELECT);
	}

	EckInline BOOL GetFullRowSelect()
	{
		return IsBitSet(GetStyle(), TVS_FULLROWSELECT);
	}

	EckInline void SetHasButton(BOOL bHasButton)
	{
		ModifyStyle(bHasButton ? TVS_HASBUTTONS : 0, TVS_HASBUTTONS);
	}

	EckInline BOOL GetHasButton()
	{
		return IsBitSet(GetStyle(), TVS_HASBUTTONS);
	}

	EckInline void SetHasLine(BOOL bHasLine)
	{
		ModifyStyle(bHasLine ? TVS_HASLINES : 0, TVS_HASLINES);
	}

	EckInline BOOL GetHasLine()
	{
		return IsBitSet(GetStyle(), TVS_HASLINES);
	}

	EckInline void SetInfoTip(BOOL bInfoTip)
	{
		ModifyStyle(bInfoTip ? TVS_INFOTIP : 0, TVS_INFOTIP);
	}

	EckInline BOOL GetInfoTip()
	{
		return IsBitSet(GetStyle(), TVS_INFOTIP);
	}

	EckInline void SetLineStaRoot(BOOL bLineStaRoot)
	{
		ModifyStyle(bLineStaRoot ? TVS_LINESATROOT : 0, TVS_LINESATROOT);
	}

	EckInline BOOL GetLineStaRoot()
	{
		return IsBitSet(GetStyle(), TVS_LINESATROOT);
	}

	EckInline void SetNoHScroll(BOOL bNoHScroll)
	{
		ModifyStyle(bNoHScroll ? TVS_NOHSCROLL : 0, TVS_NOHSCROLL);
	}

	EckInline BOOL GetNoHScroll()
	{
		return IsBitSet(GetStyle(), TVS_NOHSCROLL);
	}

	EckInline void SetNoEvenHeight(BOOL bNoEvenHeight)
	{
		ModifyStyle(bNoEvenHeight ? TVS_NONEVENHEIGHT : 0, TVS_NONEVENHEIGHT);
	}

	EckInline BOOL GetNoEvenHeight()
	{
		return IsBitSet(GetStyle(), TVS_NONEVENHEIGHT);
	}

	EckInline void SetNoScroll(BOOL bNoScroll)
	{
		ModifyStyle(bNoScroll ? TVS_NOSCROLL : 0, TVS_NOSCROLL);
	}

	EckInline BOOL GetNoScroll()
	{
		return IsBitSet(GetStyle(), TVS_NOSCROLL);
	}

	EckInline void SetRtlReading(BOOL bRtlReading)
	{
		ModifyStyle(bRtlReading ? TVS_RTLREADING : 0, TVS_RTLREADING);
	}

	EckInline BOOL GetRtlReading()
	{
		return IsBitSet(GetStyle(), TVS_RTLREADING);
	}

	EckInline void SetShowSelAlways(BOOL bShowSelAlways)
	{
		ModifyStyle(bShowSelAlways ? TVS_SHOWSELALWAYS : 0, TVS_SHOWSELALWAYS);
	}

	EckInline BOOL GetShowSelAlways()
	{
		return IsBitSet(GetStyle(), TVS_SHOWSELALWAYS);
	}

	EckInline void SetSingleExpand(BOOL bSingleExpand)
	{
		ModifyStyle(bSingleExpand ? TVS_SINGLEEXPAND : 0, TVS_SINGLEEXPAND);
	}

	EckInline BOOL GetSingleExpand()
	{
		return IsBitSet(GetStyle(), TVS_SINGLEEXPAND);
	}

	EckInline void SetTrackSelect(BOOL bTrackSelect)
	{
		ModifyStyle(bTrackSelect ? TVS_TRACKSELECT : 0, TVS_TRACKSELECT);
	}

	EckInline BOOL GetTrackSelect()
	{
		return IsBitSet(GetStyle(), TVS_TRACKSELECT);
	}

	EckInline void SetAutoHScroll(BOOL bAutoHScroll)
	{
		SetTVExtStyle(bAutoHScroll ? TVS_EX_AUTOHSCROLL : 0, TVS_EX_AUTOHSCROLL);
	}

	EckInline BOOL GetAutoHScroll()
	{
		return IsBitSet(GetTVExtStyle(), TVS_EX_AUTOHSCROLL);
	}

	EckInline void SetDimmedCheckBox(BOOL bDimmedCheckBox)
	{
		SetTVExtStyle(bDimmedCheckBox ? TVS_EX_DIMMEDCHECKBOXES : 0, TVS_EX_DIMMEDCHECKBOXES);
	}

	EckInline BOOL GetDimmedCheckBox()
	{
		return IsBitSet(GetTVExtStyle(), TVS_EX_DIMMEDCHECKBOXES);
	}

	EckInline void SetDoubleBuffer(BOOL bDoubleBuffer)
	{
		SetTVExtStyle(bDoubleBuffer ? TVS_EX_DOUBLEBUFFER : 0, TVS_EX_DOUBLEBUFFER);
	}

	EckInline BOOL GetDoubleBuffer()
	{
		return IsBitSet(GetTVExtStyle(), TVS_EX_DOUBLEBUFFER);
	}

	EckInline void SetDrawImageAsync(BOOL bDrawImageAsync)
	{
		SetTVExtStyle(bDrawImageAsync ? TVS_EX_DRAWIMAGEASYNC : 0, TVS_EX_DRAWIMAGEASYNC);
	}

	EckInline BOOL GetDrawImageAsync()
	{
		return IsBitSet(GetTVExtStyle(), TVS_EX_DRAWIMAGEASYNC);
	}

	EckInline void SetExclusionCheckBox(BOOL bExclusionCheckBox)
	{
		SetTVExtStyle(bExclusionCheckBox ? TVS_EX_EXCLUSIONCHECKBOXES : 0, TVS_EX_EXCLUSIONCHECKBOXES);
	}

	EckInline BOOL GetExclusionCheckBox()
	{
		return IsBitSet(GetTVExtStyle(), TVS_EX_EXCLUSIONCHECKBOXES);
	}

	EckInline void SetFadeInOutExpandos(BOOL bFadeInOutExpandos)
	{
		SetTVExtStyle(bFadeInOutExpandos ? TVS_EX_FADEINOUTEXPANDOS : 0, TVS_EX_FADEINOUTEXPANDOS);
	}

	EckInline BOOL GetFadeInOutExpandos()
	{
		return IsBitSet(GetTVExtStyle(), TVS_EX_FADEINOUTEXPANDOS);
	}

	EckInline void SetNoIndentState(BOOL bNoIndentState)
	{
		SetTVExtStyle(bNoIndentState ? TVS_EX_NOINDENTSTATE : 0, TVS_EX_NOINDENTSTATE);
	}

	EckInline BOOL GetNoIndentState()
	{
		return IsBitSet(GetTVExtStyle(), TVS_EX_NOINDENTSTATE);
	}

	EckInline void SetPartialCheckBox(BOOL bPartialCheckBox)
	{
		SetTVExtStyle(bPartialCheckBox ? TVS_EX_PARTIALCHECKBOXES : 0, TVS_EX_PARTIALCHECKBOXES);
	}

	EckInline BOOL GetPartialCheckBox()
	{
		return IsBitSet(GetTVExtStyle(), TVS_EX_PARTIALCHECKBOXES);
	}

	EckInline void SetRichToolTip(BOOL bRichToolTip)
	{
		SetTVExtStyle(bRichToolTip ? TVS_EX_RICHTOOLTIP : 0, TVS_EX_RICHTOOLTIP);
	}

	EckInline BOOL GetRichToolTip()
	{
		return IsBitSet(GetTVExtStyle(), TVS_EX_RICHTOOLTIP);
	}

	EckInline HIMAGELIST CreateDragImage(HTREEITEM hItem)
	{
		return (HIMAGELIST)SendMsg(TVM_CREATEDRAGIMAGE, 0, (LPARAM)hItem);
	}

	EckInline BOOL DeleteItem(HTREEITEM hItem)
	{
		return (BOOL)SendMsg(TVM_DELETEITEM, 0, (LPARAM)hItem);
	}

	/// <summary>
	/// 进入编辑。
	/// 控件必须具有焦点
	/// </summary>
	/// <param name="hItem">项目</param>
	/// <returns>成功返回编辑框句柄，失败返回NULL</returns>
	EckInline HWND EditLabel(HTREEITEM hItem)
	{
		return (HWND)SendMsg(TVM_EDITLABELW, 0, (LPARAM)hItem);
	}

	EckInline BOOL EndEditLabel(BOOL bSave)
	{
		return (BOOL)SendMsg(TVM_ENDEDITLABELNOW, bSave, 0);
	}

	/// <summary>
	/// 保证显示
	/// </summary>
	/// <param name="hItem">项目</param>
	/// <param name="bTop">是否尽可能将项目滚动到顶部</param>
	/// <returns>如果滚动视图并且未展开任何项目则返回TRUE，否则返回FALSE</returns>
	EckInline BOOL EnsureVisible(HTREEITEM hItem, BOOL bTop = FALSE)
	{
		if (bTop)
			return (BOOL)SendMsg(TVM_SELECTITEM, TVGN_FIRSTVISIBLE, (LPARAM)hItem);
		else
			return (BOOL)SendMsg(TVM_ENSUREVISIBLE, 0, (LPARAM)hItem);
	}

	/// <summary>
	/// 展开/折叠项目
	/// </summary>
	/// <param name="hItem">项目</param>
	/// <param name="uOp">操作，可选下列值：
	/// TVE_COLLAPSE - 折叠
	/// (TVE_COLLAPSERESET | TVE_COLLAPSE) - 折叠并删除所有子项
	/// TVE_EXPAND - 展开
	/// (TVE_EXPANDPARTIAL | TVE_EXPAND) - 部分展开
	/// TVE_TOGGLE - 反转折叠状态
	/// </param>
	/// <returns>成功返回TRUE</returns>
	EckInline BOOL Expand(HTREEITEM hItem, UINT uOp)
	{
		return (BOOL)SendMsg(TVM_ENSUREVISIBLE, uOp, (LPARAM)hItem);
	}

	EckInline COLORREF GetBKColor()
	{
		return (COLORREF)SendMsg(TVM_GETBKCOLOR, 0, 0);
	}

	EckInline int GetCount()
	{
		return (int)SendMsg(TVM_GETCOUNT, 0, 0);
	}

	EckInline HWND GetEditControl()
	{
		return (HWND)SendMsg(TVM_GETEDITCONTROL, 0, 0);
	}

	EckInline DWORD GetTVExtStyle()
	{
		return (DWORD)SendMsg(TVM_GETEXTENDEDSTYLE, 0, 0);
	}

	/// <summary>
	/// 取图像列表
	/// </summary>
	/// <param name="uType">类型，TVSIL_常量</param>
	/// <returns>图像列表句柄</returns>
	EckInline HIMAGELIST GetImageList(UINT uType = TVSIL_NORMAL)
	{
		return (HIMAGELIST)SendMsg(TVM_GETIMAGELIST, uType, 0);
	}

	/// <summary>
	/// 取缩进宽度。
	/// 取得子项相对其父项的缩进宽度，以像素为单位
	/// </summary>
	/// <returns>缩进宽度</returns>
	EckInline int GetIndent()
	{
		return (int)SendMsg(TVM_GETINDENT, 0, 0);
	}

	EckInline COLORREF GetInsertMarkColor()
	{
		return (COLORREF)SendMsg(TVM_GETINSERTMARKCOLOR, 0, 0);
	}

	EckInline CRefStrW GetISearchString()
	{
		CRefStrW rs;
		int cch = (int)SendMsg(TVM_GETISEARCHSTRINGW, 0, NULL);
		if (cch <= 0)
			return rs;
		rs.ReSize(cch);
		SendMsg(TVM_GETISEARCHSTRINGW, 0, (LPARAM)rs.Data());
		return rs;
	}

	EckInline int GetISearchString(PWSTR pszBuf)
	{
		return (int)SendMsg(TVM_GETISEARCHSTRINGW, 0, (LPARAM)pszBuf);
	}

	EckInline BOOL GetItem(TVITEMEXW* ptvi)
	{
		return (BOOL)SendMsg(TVM_GETITEMW, 0, (LPARAM)ptvi);
	}

	EckInline int GetItemHeight()
	{
		return (int)SendMsg(TVM_GETITEMHEIGHT, 0, 0);
	}

	/// <summary>
	/// 取项目矩形
	/// </summary>
	/// <param name="hItem">项目</param>
	/// <param name="prc">接收矩形</param>
	/// <param name="bOnlyText">是否仅文本尺寸</param>
	/// <returns>如果项目可见且矩形检索成功则返回TRUE，否则返回FALSE</returns>
	EckInline BOOL GetItemRect(HTREEITEM hItem, RECT* prc, BOOL bOnlyText = FALSE)
	{
		*(HTREEITEM*)prc = hItem;
		return (BOOL)SendMsg(TVM_GETITEMRECT, bOnlyText, (LPARAM)prc);
	}

	EckInline UINT GetItemState(HTREEITEM hItem, UINT uMask)
	{
		return (UINT)SendMsg(TVM_GETITEMSTATE, (WPARAM)hItem, uMask);
	}

	EckInline COLORREF GetLineColor()
	{
		return (COLORREF)SendMsg(TVM_GETLINECOLOR, 0, 0);
	}

	EckInline HTREEITEM GetNextItem(HTREEITEM hItem, UINT uFlag)
	{
		return (HTREEITEM)SendMsg(TVM_GETNEXTITEM, uFlag, (LPARAM)hItem);
	}

	EckInline HTREEITEM GetCurrSel()
	{
		return GetNextItem(nullptr, TVGN_CARET);
	}

	EckInline HTREEITEM GetFirstChildItem(HTREEITEM hItem)
	{
		return GetNextItem(hItem, TVGN_CHILD);
	}

	EckInline HTREEITEM GetDropTargetItem()
	{
		return GetNextItem(nullptr, TVGN_DROPHILITE);
	}

	EckInline HTREEITEM GetFirstVisibleItem()
	{
		return GetNextItem(nullptr, TVGN_FIRSTVISIBLE);
	}

	EckInline HTREEITEM GetLastVisibleItem()
	{
		return GetNextItem(nullptr, TVGN_LASTVISIBLE);
	}

	EckInline HTREEITEM GetNextSiblingItem(HTREEITEM hItem)
	{
		return GetNextItem(hItem, TVGN_NEXT);
	}

	EckInline HTREEITEM GetNextSelItem(HTREEITEM hItem)
	{
		return GetNextItem(hItem, TVGN_NEXTSELECTED);
	}

	EckInline HTREEITEM GetNextVisibleItem(HTREEITEM hItem)
	{
		return GetNextItem(hItem, TVGN_NEXTVISIBLE);
	}

	EckInline HTREEITEM GetParentItem(HTREEITEM hItem)
	{
		return GetNextItem(hItem, TVGN_PARENT);
	}

	EckInline HTREEITEM GetPrevSiblingItem(HTREEITEM hItem)
	{
		return GetNextItem(hItem, TVGN_PREVIOUS);
	}

	EckInline HTREEITEM GetPrevVisibleItem(HTREEITEM hItem)
	{
		return GetNextItem(hItem, TVGN_PREVIOUSVISIBLE);
	}

	EckInline HTREEITEM GetRootItem()
	{
		return GetNextItem(nullptr, TVGN_ROOT);
	}

	EckInline int GetScrollTime()
	{
		return (int)SendMsg(TVM_GETSCROLLTIME, 0, 0);
	}

	EckInline COLORREF GetTextColor()
	{
		return (COLORREF)SendMsg(TVM_GETTEXTCOLOR, 0, 0);
	}

	EckInline HWND GetToolTip()
	{
		return (HWND)SendMsg(TVM_GETTOOLTIPS, 0, 0);
	}

	EckInline int GetVisibleCount()
	{
		return (int)SendMsg(TVM_GETVISIBLECOUNT, 0, 0);
	}

	/// <summary>
	/// 命中测试
	/// </summary>
	/// <param name="pt">测试点，相对客户区</param>
	/// <param name="puFlags">接收测试结果标志，TVHT_常量</param>
	/// <returns>项目句柄</returns>
	EckInline HTREEITEM HitTest(POINT pt, UINT* puFlags = nullptr)
	{
		TVHITTESTINFO tvhti{ pt };
		SendMsg(TVM_HITTEST, 0, (LPARAM)&tvhti);
		if (puFlags)
			*puFlags = tvhti.flags;
		return tvhti.hItem;
	}

	/// <summary>
	/// 插入项目
	/// </summary>
	/// <param name="ptvis">TVINSERTSTRUCTW指针，只需要初始化itemex成员</param>
	/// <param name="hParent">父项目，可为TVI_ROOT/NULL或项目句柄</param>
	/// <param name="hInsertAfter">欲插入到其后的项目，可为TVI_常量或项目句柄</param>
	/// <returns>项目句柄，失败返回NULL</returns>
	EckInline HTREEITEM InsertItem(TVINSERTSTRUCTW* ptvis, HTREEITEM hParent = TVI_ROOT,
		HTREEITEM hInsertAfter = TVI_FIRST)
	{
		ptvis->hParent = hParent;
		ptvis->hInsertAfter = hInsertAfter;
		return (HTREEITEM)SendMsg(TVM_INSERTITEMW, 0, (LPARAM)ptvis);
	}

	/// <summary>
	/// 选择项目
	/// </summary>
	/// <param name="hItem">项目</param>
	/// <param name="bNoSingleExpand">选择单个项时不展开子项</param>
	/// <returns>成功返回TRUE，失败返回FALSE</returns>
	EckInline BOOL SelectItem(HTREEITEM hItem, BOOL bNoSingleExpand = FALSE)
	{
		return (BOOL)SendMsg(TVM_SELECTITEM, TVGN_CARET | (bNoSingleExpand ? TVSI_NOSINGLEEXPAND : 0), (LPARAM)hItem);
	}

	EckInline BOOL SelectDropTargetItem(HTREEITEM hItem)
	{
		return (BOOL)SendMsg(TVM_SELECTITEM, TVGN_DROPHILITE, (LPARAM)hItem);
	}

	EckInline void SetAutoScrollInfo(int iPixelPreSecond, int iRedrawGap)
	{
		SendMsg(TVM_SETAUTOSCROLLINFO, iPixelPreSecond, iRedrawGap);
	}

	EckInline COLORREF SetBKColor(COLORREF cr)
	{
		return (COLORREF)SendMsg(TVM_SETBKCOLOR, 0, cr);
	}

	EckInline HRESULT SetTVExtStyle(DWORD dwNew, DWORD dwMask)
	{
		return (HRESULT)SendMsg(TVM_SETEXTENDEDSTYLE, dwMask, dwNew);
	}

	EckInline HRESULT SetTVExtStyle(DWORD dwNew)
	{
		return (HRESULT)SetTVExtStyle(dwNew, TVS_EX_ALL);
	}

	EckInline HIMAGELIST SetImageList(HIMAGELIST hImageList, UINT uType = TVSIL_NORMAL)
	{
		return (HIMAGELIST)SendMsg(TVM_SETIMAGELIST, uType, (LPARAM)hImageList);
	}

	/// <summary>
	/// 置缩进宽度
	/// </summary>
	/// <param name="iIndent">缩进宽度，若小于最小缩进宽度，则设为最小缩进宽度的值</param>
	EckInline void SetIndent(int iIndent)
	{
		SendMsg(TVM_SETINDENT, iIndent, 0);
	}

	EckInline BOOL SetInsertMark(HTREEITEM hItem, BOOL bInsertAfterItem = TRUE)
	{
		return (BOOL)SendMsg(TVM_SETINSERTMARK, bInsertAfterItem, (LPARAM)hItem);
	}

	EckInline COLORREF SetInsertMarkColor(COLORREF cr)
	{
		return (COLORREF)SendMsg(TVM_SETINSERTMARKCOLOR, 0, cr);
	}

	EckInline BOOL SetItem(TVITEMEXW* ptvi)
	{
		return (BOOL)SendMsg(TVM_SETITEM, 0, (LPARAM)ptvi);
	}

	/// <summary>
	/// 置项目高度
	/// </summary>
	/// <param name="cy">高度，若为-1则使用默认高度</param>
	/// <returns></returns>
	EckInline int SetItemHeight(int cy)
	{
		return (int)SendMsg(TVM_SETITEMHEIGHT, cy, 0);
	}

	EckInline COLORREF SetLineColor(COLORREF cr)
	{
		return (COLORREF)SendMsg(TVM_SETLINECOLOR, 0, cr);
	}

	EckInline int SetScrollTime(int iTime)
	{
		return (int)SendMsg(TVM_SETSCROLLTIME, iTime, 0);
	}

	EckInline COLORREF SetTextColor(COLORREF cr)
	{
		return (COLORREF)SendMsg(TVM_SETTEXTCOLOR, 0, cr);
	}

	EckInline HWND SetToolTip(HWND hToolTip)
	{
		return (HWND)SendMsg(TVM_SETTOOLTIPS, (WPARAM)hToolTip, 0);
	}

	EckInline void ShowTip(HTREEITEM hItem)
	{
		SendMsg(TVM_SHOWINFOTIP, 0, (LPARAM)hItem);
	}

	/// <summary>
	/// 排序子项
	/// </summary>
	/// <param name="hItem">项目</param>
	/// <param name="bAllChildren">是否包含所有子项，若为FALSE则指排序hItem的直接子项</param>
	/// <returns>成功返回TRUE，失败返回FALSE</returns>
	EckInline BOOL SortChildren(HTREEITEM hItem, BOOL bAllChildren = TRUE)
	{
		return (BOOL)SendMsg(TVM_SORTCHILDREN, bAllChildren, (LPARAM)hItem);
	}

	/// <summary>
	/// 排序子项。
	/// 使用自定义过程排序项目的子项
	/// </summary>
	/// <param name="hItem">项目</param>
	/// <param name="pfnCompare">比较函数，若第一项应在第二项之前，则返回负值；若在之后，则返回正值；若相等，则返回0</param>
	/// <param name="lParam">自定义数值</param>
	/// <returns>成功返回TRUE，失败返回FALSE</returns>
	EckInline BOOL SortChildren(HTREEITEM hItem, PFNTVCOMPARE pfnCompare, LPARAM lParam)
	{
		TVSORTCB tvscb;
		tvscb.hParent = hItem;
		tvscb.lpfnCompare = pfnCompare;
		tvscb.lParam = lParam;
		return (BOOL)SendMsg(TVM_SORTCHILDRENCB, 0, (LPARAM)&tvscb);
	}
};

ECK_NAMESPACE_END