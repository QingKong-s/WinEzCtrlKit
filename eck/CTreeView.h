#pragma once
#include "CWnd.h"

ECK_NAMESPACE_BEGIN
constexpr inline UINT TVS_EX_ALL{/*TVS_EX_NOSINGLECOLLAPSE | TVS_EX_MULTISELECT |*/
    TVS_EX_DOUBLEBUFFER | TVS_EX_NOINDENTSTATE | TVS_EX_RICHTOOLTIP | TVS_EX_AUTOHSCROLL |
    TVS_EX_FADEINOUTEXPANDOS | TVS_EX_PARTIALCHECKBOXES | TVS_EX_EXCLUSIONCHECKBOXES |
    TVS_EX_DIMMEDCHECKBOXES | TVS_EX_DRAWIMAGEASYNC };

#define ECK_CWNDPROP_TVE_STYLE(Name, Style)				\
	ECKPROP(StyleGet##Name, StyleSet##Name) BOOL Name;	\
	BOOL StyleGet##Name() const noexcept							\
	{													\
		if constexpr (Style == 0)						\
			return !GetTVExtendStyle();				\
		else											\
			return IsBitSet(GetTVExtendStyle(), Style);	\
	}													\
	void StyleSet##Name(BOOL b) const noexcept					\
	{													\
		SetTVExtendStyle(b ? Style : 0, Style);			\
	}

class CTreeView : public CWnd
{
public:
    ECK_RTTI(CTreeView, CWnd);
    ECK_CWND_NOSINGLEOWNER(CTreeView);
    ECK_CWND_CREATE_CLS(WC_TREEVIEWW);

    ECK_CWNDPROP_STYLE(CheckBoxes, TVS_CHECKBOXES);
    ECK_CWNDPROP_STYLE(DisableDragDrop, TVS_DISABLEDRAGDROP);
    ECK_CWNDPROP_STYLE(EditLabels, TVS_EDITLABELS);
    ECK_CWNDPROP_STYLE(FullRowSelect, TVS_FULLROWSELECT);
    ECK_CWNDPROP_STYLE(HasButtons, TVS_HASBUTTONS);
    ECK_CWNDPROP_STYLE(HasLines, TVS_HASLINES);
    ECK_CWNDPROP_STYLE(InfoTip, TVS_INFOTIP);
    ECK_CWNDPROP_STYLE(LinesAtRoot, TVS_LINESATROOT);
    ECK_CWNDPROP_STYLE(NoHScroll, TVS_NOHSCROLL);
    ECK_CWNDPROP_STYLE(NonEvenHeight, TVS_NONEVENHEIGHT);
    ECK_CWNDPROP_STYLE(NoScroll, TVS_NOSCROLL);
    ECK_CWNDPROP_STYLE(NoToolTips, TVS_NOTOOLTIPS);
    ECK_CWNDPROP_STYLE(RtlReading, TVS_RTLREADING);
    ECK_CWNDPROP_STYLE(ShowSelAlways, TVS_SHOWSELALWAYS);
    ECK_CWNDPROP_STYLE(SingleExpand, TVS_SINGLEEXPAND);
    ECK_CWNDPROP_STYLE(TrackSelect, TVS_TRACKSELECT);
    ECK_CWNDPROP_TVE_STYLE(AutoHScroll, TVS_EX_AUTOHSCROLL);
    ECK_CWNDPROP_TVE_STYLE(DimmedCheckboxes, TVS_EX_DIMMEDCHECKBOXES);
    ECK_CWNDPROP_TVE_STYLE(DoubleBuffer, TVS_EX_DOUBLEBUFFER);
    ECK_CWNDPROP_TVE_STYLE(DrawImageAsync, TVS_EX_DRAWIMAGEASYNC);
    ECK_CWNDPROP_TVE_STYLE(ExclusionCheckboxes, TVS_EX_EXCLUSIONCHECKBOXES);
    ECK_CWNDPROP_TVE_STYLE(FadeInOutExpandos, TVS_EX_FADEINOUTEXPANDOS);
    ECK_CWNDPROP_TVE_STYLE(NoIndentState, TVS_EX_NOINDENTSTATE);
    ECK_CWNDPROP_TVE_STYLE(NoSingleCollapse, TVS_EX_NOSINGLECOLLAPSE);
    ECK_CWNDPROP_TVE_STYLE(PartialCheckboxes, TVS_EX_PARTIALCHECKBOXES);
    ECK_CWNDPROP_TVE_STYLE(RichTooltip, TVS_EX_RICHTOOLTIP);
protected:
    BOOL m_bAutoDarkMode{ TRUE };
public:
    EckInlineCe void SetAutoDarkMode(BOOL b) { m_bAutoDarkMode = b; }

    LRESULT OnMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept
    {
        switch (uMsg)
        {
        case WM_CREATE:
        case WM_THEMECHANGED:
        {
            if (m_bAutoDarkMode)
            {
                const auto lResult = CWnd::OnMessage(hWnd, uMsg, wParam, lParam);
                const auto* const ptc = PtcCurrent();
                SetTextForegroundColor(ptc->crDefText);
                SetBackgroundColor(ptc->crDefBkg);
                return lResult;
            }
        }
        break;
        }
        return __super::OnMessage(hWnd, uMsg, wParam, lParam);
    }

    EckInline HIMAGELIST CreateDragImage(HTREEITEM hItem) const noexcept
    {
        return (HIMAGELIST)SendMsg(TVM_CREATEDRAGIMAGE, 0, (LPARAM)hItem);
    }

    EckInline BOOL DeleteItem(HTREEITEM hItem) const noexcept
    {
        return (BOOL)SendMsg(TVM_DELETEITEM, 0, (LPARAM)hItem);
    }

    /// <summary>
    /// 进入编辑。
    /// 控件必须具有焦点
    /// </summary>
    /// <param name="hItem">项目</param>
    /// <returns>成功返回编辑框句柄，失败返回NULL</returns>
    EckInline HWND EditLabel(HTREEITEM hItem) const noexcept
    {
        return (HWND)SendMsg(TVM_EDITLABELW, 0, (LPARAM)hItem);
    }

    EckInline BOOL EndEditLabel(BOOL bSave) const noexcept
    {
        return (BOOL)SendMsg(TVM_ENDEDITLABELNOW, bSave, 0);
    }

    /// <summary>
    /// 保证显示
    /// </summary>
    /// <param name="hItem">项目</param>
    /// <param name="bTop">是否尽可能将项目滚动到顶部</param>
    /// <returns>如果滚动视图并且未展开任何项目则返回TRUE，否则返回FALSE</returns>
    EckInline BOOL EnsureVisible(HTREEITEM hItem, BOOL bTop = FALSE) const noexcept
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
    EckInline BOOL Expand(HTREEITEM hItem, UINT uOp) const noexcept
    {
        return (BOOL)SendMsg(TVM_ENSUREVISIBLE, uOp, (LPARAM)hItem);
    }

    EckInline COLORREF GetBackgroundColor() const noexcept
    {
        return (COLORREF)SendMsg(TVM_GETBKCOLOR, 0, 0);
    }

    EckInline int GetItemCount() const noexcept
    {
        return (int)SendMsg(TVM_GETCOUNT, 0, 0);
    }

    EckInline HWND GetEditControl() const noexcept
    {
        return (HWND)SendMsg(TVM_GETEDITCONTROL, 0, 0);
    }

    EckInline DWORD GetTVExtendStyle() const noexcept
    {
        return (DWORD)SendMsg(TVM_GETEXTENDEDSTYLE, 0, 0);
    }

    /// <summary>
    /// 取图像列表
    /// </summary>
    /// <param name="uType">类型，TVSIL_常量</param>
    /// <returns>图像列表句柄</returns>
    EckInline HIMAGELIST GetImageList(UINT uType = TVSIL_NORMAL) const noexcept
    {
        return (HIMAGELIST)SendMsg(TVM_GETIMAGELIST, uType, 0);
    }

    /// <summary>
    /// 取缩进宽度。
    /// 取得子项相对其父项的缩进宽度，以像素为单位
    /// </summary>
    /// <returns>缩进宽度</returns>
    EckInline int GetIndent() const noexcept
    {
        return (int)SendMsg(TVM_GETINDENT, 0, 0);
    }

    EckInline COLORREF GetInsertMarkColor() const noexcept
    {
        return (COLORREF)SendMsg(TVM_GETINSERTMARKCOLOR, 0, 0);
    }

    EckInline CRefStrW GetIncrementalSearchString() const noexcept
    {
        CRefStrW rs;
        int cch = (int)SendMsg(TVM_GETISEARCHSTRINGW, 0, NULL);
        if (cch <= 0)
            return rs;
        rs.ReSize(cch);
        SendMsg(TVM_GETISEARCHSTRINGW, 0, (LPARAM)rs.Data());
        return rs;
    }

    EckInline int GetIncrementalSearchString(_In_opt_ PWSTR pszBuf) const noexcept
    {
        return (int)SendMsg(TVM_GETISEARCHSTRINGW, 0, (LPARAM)pszBuf);
    }

    EckInline BOOL GetItem(_Inout_ TVITEMEXW* ptvi) const noexcept
    {
        return (BOOL)SendMsg(TVM_GETITEMW, 0, (LPARAM)ptvi);
    }

    EckInline int GetItemHeight() const noexcept
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
    EckInline BOOL GetItemRect(HTREEITEM hItem,
        _Out_ RECT* prc, BOOL bOnlyText = FALSE) const noexcept
    {
        *(HTREEITEM*)prc = hItem;
        return (BOOL)SendMsg(TVM_GETITEMRECT, bOnlyText, (LPARAM)prc);
    }

    EckInline UINT GetItemState(HTREEITEM hItem, UINT uMask) const noexcept
    {
        return (UINT)SendMsg(TVM_GETITEMSTATE, (WPARAM)hItem, uMask);
    }

    EckInline COLORREF GetLineColor() const noexcept
    {
        return (COLORREF)SendMsg(TVM_GETLINECOLOR, 0, 0);
    }

    EckInline HTREEITEM GetNextItem(HTREEITEM hItem, UINT uFlag) const noexcept
    {
        return (HTREEITEM)SendMsg(TVM_GETNEXTITEM, uFlag, (LPARAM)hItem);
    }

    EckInline HTREEITEM GetCurrentSelection() const noexcept
    {
        return GetNextItem(nullptr, TVGN_CARET);
    }

    EckInline HTREEITEM GetFirstChildItem(HTREEITEM hItem) const noexcept
    {
        return GetNextItem(hItem, TVGN_CHILD);
    }

    EckInline HTREEITEM GetDropTargetItem() const noexcept
    {
        return GetNextItem(nullptr, TVGN_DROPHILITE);
    }

    EckInline HTREEITEM GetFirstVisibleItem() const noexcept
    {
        return GetNextItem(nullptr, TVGN_FIRSTVISIBLE);
    }

    EckInline HTREEITEM GetLastVisibleItem() const noexcept
    {
        return GetNextItem(nullptr, TVGN_LASTVISIBLE);
    }

    EckInline HTREEITEM GetNextSiblingItem(HTREEITEM hItem) const noexcept
    {
        return GetNextItem(hItem, TVGN_NEXT);
    }

    EckInline HTREEITEM GetNextSelectedItem(HTREEITEM hItem) const noexcept
    {
        return GetNextItem(hItem, TVGN_NEXTSELECTED);
    }

    EckInline HTREEITEM GetNextVisibleItem(HTREEITEM hItem) const noexcept
    {
        return GetNextItem(hItem, TVGN_NEXTVISIBLE);
    }

    EckInline HTREEITEM GetParentItem(HTREEITEM hItem) const noexcept
    {
        return GetNextItem(hItem, TVGN_PARENT);
    }

    EckInline HTREEITEM GetPrevSiblingItem(HTREEITEM hItem) const noexcept
    {
        return GetNextItem(hItem, TVGN_PREVIOUS);
    }

    EckInline HTREEITEM GetPrevVisibleItem(HTREEITEM hItem) const noexcept
    {
        return GetNextItem(hItem, TVGN_PREVIOUSVISIBLE);
    }

    EckInline HTREEITEM GetRootItem() const noexcept
    {
        return GetNextItem(nullptr, TVGN_ROOT);
    }

    EckInline int GetScrollTime() const noexcept
    {
        return (int)SendMsg(TVM_GETSCROLLTIME, 0, 0);
    }

    EckInline COLORREF GetTextForegroundColor() const noexcept
    {
        return (COLORREF)SendMsg(TVM_GETTEXTCOLOR, 0, 0);
    }

    EckInline HWND GetToolTip() const noexcept
    {
        return (HWND)SendMsg(TVM_GETTOOLTIPS, 0, 0);
    }

    EckInline int GetVisibleCount() const noexcept
    {
        return (int)SendMsg(TVM_GETVISIBLECOUNT, 0, 0);
    }

    /// <summary>
    /// 命中测试
    /// </summary>
    /// <param name="pt">测试点，相对客户区</param>
    /// <param name="puFlags">接收测试结果标志，TVHT_常量</param>
    /// <returns>项目句柄</returns>
    EckInline HTREEITEM HitTest(POINT pt, _Out_opt_ UINT* puFlags = nullptr) const noexcept
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
    EckInline HTREEITEM InsertItem(_In_ TVINSERTSTRUCTW* ptvis) const noexcept
    {
        return (HTREEITEM)SendMsg(TVM_INSERTITEMW, 0, (LPARAM)ptvis);
    }

    EckInline HTREEITEM InsertItem(_In_z_ PCWSTR pszText, HTREEITEM hParent = TVI_ROOT,
        HTREEITEM hInsertAfter = TVI_LAST, LPARAM lParam = 0, int iImage = -1, int iSelectedImage = -1)
    {
        TVINSERTSTRUCTW tvis;
        tvis.hParent = hParent;
        tvis.hInsertAfter = hInsertAfter;
        tvis.itemex.pszText = (PWSTR)pszText;
        tvis.itemex.lParam = lParam;
        tvis.itemex.iImage = iImage;
        tvis.itemex.iSelectedImage = iSelectedImage;
        tvis.itemex.mask = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
        return InsertItem(&tvis);
    }

    /// <summary>
    /// 选择项目
    /// </summary>
    /// <param name="hItem">项目</param>
    /// <param name="bNoSingleExpand">选择单个项时不展开子项</param>
    /// <returns>成功返回TRUE，失败返回FALSE</returns>
    EckInline BOOL SelectItem(HTREEITEM hItem, BOOL bNoSingleExpand = FALSE) const noexcept
    {
        return (BOOL)SendMsg(TVM_SELECTITEM, TVGN_CARET |
            (bNoSingleExpand ? TVSI_NOSINGLEEXPAND : 0), (LPARAM)hItem);
    }

    EckInline BOOL SelectDropTargetItem(HTREEITEM hItem) const noexcept
    {
        return (BOOL)SendMsg(TVM_SELECTITEM, TVGN_DROPHILITE, (LPARAM)hItem);
    }

    EckInline void SetAutoScrollInfomation(int iPixelPreSecond, int iRedrawGap) const noexcept
    {
        SendMsg(TVM_SETAUTOSCROLLINFO, iPixelPreSecond, iRedrawGap);
    }

    EckInline COLORREF SetBackgroundColor(COLORREF cr) const noexcept
    {
        return (COLORREF)SendMsg(TVM_SETBKCOLOR, 0, cr);
    }

    EckInline HRESULT SetTVExtendStyle(DWORD dwNew, DWORD dwMask) const noexcept
    {
        return (HRESULT)SendMsg(TVM_SETEXTENDEDSTYLE, dwMask, dwNew);
    }

    EckInline HRESULT SetTVExtendStyle(DWORD dwNew) const noexcept
    {
        return (HRESULT)SetTVExtendStyle(dwNew, TVS_EX_ALL);
    }

    EckInline HIMAGELIST SetImageList(HIMAGELIST hImageList, UINT uType = TVSIL_NORMAL) const noexcept
    {
        return (HIMAGELIST)SendMsg(TVM_SETIMAGELIST, uType, (LPARAM)hImageList);
    }

    /// <summary>
    /// 置缩进宽度
    /// </summary>
    /// <param name="iIndent">缩进宽度，若小于最小缩进宽度，则设为最小缩进宽度的值</param>
    EckInline void SetIndent(int iIndent) const noexcept
    {
        SendMsg(TVM_SETINDENT, iIndent, 0);
    }

    EckInline BOOL SetInsertMark(HTREEITEM hItem, BOOL bInsertAfterItem = TRUE) const noexcept
    {
        return (BOOL)SendMsg(TVM_SETINSERTMARK, bInsertAfterItem, (LPARAM)hItem);
    }

    EckInline COLORREF SetInsertMarkColor(COLORREF cr) const noexcept
    {
        return (COLORREF)SendMsg(TVM_SETINSERTMARKCOLOR, 0, cr);
    }

    EckInline BOOL SetItem(_Inout_ TVITEMEXW* ptvi) const noexcept
    {
        return (BOOL)SendMsg(TVM_SETITEM, 0, (LPARAM)ptvi);
    }

    /// <summary>
    /// 置项目高度
    /// </summary>
    /// <param name="cy">高度，若为-1则使用默认高度</param>
    /// <returns></returns>
    EckInline int SetItemHeight(int cy) const noexcept
    {
        return (int)SendMsg(TVM_SETITEMHEIGHT, cy, 0);
    }

    EckInline COLORREF SetLineColor(COLORREF cr) const noexcept
    {
        return (COLORREF)SendMsg(TVM_SETLINECOLOR, 0, cr);
    }

    EckInline int SetScrollTime(int iTime) const noexcept
    {
        return (int)SendMsg(TVM_SETSCROLLTIME, iTime, 0);
    }

    EckInline COLORREF SetTextForegroundColor(COLORREF cr) const noexcept
    {
        return (COLORREF)SendMsg(TVM_SETTEXTCOLOR, 0, cr);
    }

    EckInline HWND SetToolTip(HWND hToolTip) const noexcept
    {
        return (HWND)SendMsg(TVM_SETTOOLTIPS, (WPARAM)hToolTip, 0);
    }

    EckInline void ShowTip(HTREEITEM hItem) const noexcept
    {
        SendMsg(TVM_SHOWINFOTIP, 0, (LPARAM)hItem);
    }

    /// <summary>
    /// 排序子项
    /// </summary>
    /// <param name="hItem">项目</param>
    /// <param name="bAllChildren">是否包含所有子项，若为FALSE则指排序hItem的直接子项</param>
    /// <returns>成功返回TRUE，失败返回FALSE</returns>
    EckInline BOOL SortChildren(HTREEITEM hItem, BOOL bAllChildren = TRUE) const noexcept
    {
        return (BOOL)SendMsg(TVM_SORTCHILDREN, bAllChildren, (LPARAM)hItem);
    }

    /// <summary>
    /// 排序子项。
    /// 使用自定义过程排序项目的子项
    /// </summary>
    /// <param name="hItem">项目</param>
    /// <param name="pfnCompare">
    /// 比较函数，若第一项应在第二项之前，则返回负值；
    /// 若在之后，则返回正值；若相等，则返回0
    /// </param>
    /// <param name="lParam">自定义数值</param>
    /// <returns>成功返回TRUE，失败返回FALSE</returns>
    EckInline BOOL SortChildren(HTREEITEM hItem,
        _In_ PFNTVCOMPARE pfnCompare, LPARAM lParam) const noexcept
    {
        TVSORTCB tvscb;
        tvscb.hParent = hItem;
        tvscb.lpfnCompare = pfnCompare;
        tvscb.lParam = lParam;
        return (BOOL)SendMsg(TVM_SORTCHILDRENCB, 0, (LPARAM)&tvscb);
    }
};
ECK_NAMESPACE_END