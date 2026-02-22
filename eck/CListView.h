#pragma once
#include "CWindow.h"

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

class CListView : public CWindow
{
public:
    ECK_RTTI(CListView, CWindow);
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
    EckInlineCe void SetAutoDarkMode(BOOL b) { m_bAutoDarkMode = b; }

    LRESULT OnMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept override
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
                        SetTextColor(pnmcd->hdc, PtcCurrent()->crDefText);
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
                const auto lResult = CWindow::OnMessage(hWnd, uMsg, wParam, lParam);
                const auto* const ptc = PtcCurrent();
                SetTextForegroundColor(ptc->crDefText);
                SetBackgroundColor(ptc->crDefBkg);
                SetTextBackgroundColor(ptc->crDefBkg);
                return lResult;
            }
        }
        break;
        }
        return CWindow::OnMessage(hWnd, uMsg, wParam, lParam);
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
        int* piApprWidth = nullptr, int* piApprHeight = nullptr) const noexcept
    {
        const auto uRet = (UINT)SendMessage(LVM_APPROXIMATEVIEWRECT, cItems, MAKEWPARAM(cxRef, cyRef));
        if (piApprWidth)
            *piApprWidth = LOWORD(uRet);
        if (piApprHeight)
            *piApprHeight = HIWORD(uRet);
    }

    /// <summary>
    /// 排列项目
    /// </summary>
    /// <param name="uFlag">排列选项，LVA_常量。
    /// 注：当指定LVA_ALIGNLEFT或LVA_ALIGNTOP时会分别添加LVS_ALIGNLEFT和LVS_ALIGNTOP样式</param>
    /// <returns>成功返回TRUE，失败返回FALSE</returns>
    EckInline BOOL Arrange(UINT uFlag) const noexcept
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
            return (BOOL)SendMessage(LVM_ARRANGE, uFlag, 0);
        }
    }

    EckInline void CancelEditLabel() const noexcept
    {
        SendMessage(LVM_CANCELEDITLABEL, 0, 0);
    }

    /// <summary>
    /// 创建拖放图像
    /// </summary>
    /// <param name="idx">项目</param>
    /// <param name="ptOrg">图像左上角的初始位置</param>
    /// <returns>图像列表句柄</returns>
    EckInline HIMAGELIST CreateDragImage(int idx, POINT ptOrg = {}) const noexcept
    {
        return (HIMAGELIST)SendMessage(LVM_CREATEDRAGIMAGE, idx, (LPARAM)&ptOrg);
    }

    EckInline BOOL DeleteAllItems() const noexcept
    {
        return (BOOL)SendMessage(LVM_DELETEALLITEMS, 0, 0);
    }

    EckInline BOOL DeleteColumn(int idx) const noexcept
    {
        return (BOOL)SendMessage(LVM_DELETECOLUMN, idx, 0);
    }

    EckInline BOOL DeleteItem(int idx) const noexcept
    {
        return (BOOL)SendMessage(LVM_DELETEITEM, idx, 0);
    }

    /// <summary>
    /// 进入编辑
    /// </summary>
    /// <param name="idx">项目，若为-1则取消编辑</param>
    /// <returns>成功返回编辑框句柄，失败返回NULL。编辑框将在编辑结束后失效</returns>
    EckInline HWND EditLabel(int idx) const noexcept
    {
        return (HWND)SendMessage(LVM_EDITLABELW, idx, 0);
    }

    /// <summary>
    /// 启用/禁用分组视图
    /// </summary>
    /// <param name="bEnable">是否启用</param>
    /// <returns>0 - 成功  1 - 控件状态已更改  -1 - 失败</returns>
    EckInline int EnableGroupView(BOOL bEnable) const noexcept
    {
        return (int)SendMessage(LVM_ENABLEGROUPVIEW, bEnable, 0);
    }

    /// <summary>
    /// 保证显示
    /// </summary>
    /// <param name="idx">项目</param>
    /// <param name="bFullVisible">是否保证完全可见</param>
    /// <returns>成功返回TRUE，失败返回FALSE</returns>
    EckInline BOOL EnsureVisible(int idx, BOOL bFullVisible = TRUE) const noexcept
    {
        return (BOOL)SendMessage(LVM_ENSUREVISIBLE, idx, bFullVisible);
    }

    /// <summary>
    /// 寻找项目
    /// </summary>
    /// <param name="idxStart">起始项目，若为-1则从头开始寻找</param>
    /// <param name="plvfi">LVFINDINFOW指针</param>
    /// <returns>成功返回项目索引，失败返回-1</returns>
    EckInline int FindItem(int idxStart, LVFINDINFOW* plvfi) const noexcept
    {
        return (int)SendMessage(LVM_FINDITEMW, idxStart, (LPARAM)plvfi);
    }

    EckInline COLORREF GetBackgroundColor() const noexcept
    {
        return (COLORREF)SendMessage(LVM_GETBKCOLOR, 0, 0);
    }

    EckInline BOOL GetBackgroundImage(LVBKIMAGEW* plvbki) const
    {
        return (BOOL)SendMessage(LVM_GETBKIMAGEW, 0, (LPARAM)plvbki);
    }

    EckInline BOOL GetCallbackMask() const noexcept
    {
        return (BOOL)SendMessage(LVM_GETCALLBACKMASK, 0, 0);
    }

    EckInline BOOL GetColumn(int idx, _Inout_ LVCOLUMNW* plvc) const noexcept
    {
        return (BOOL)SendMessage(LVM_GETCALLBACKMASK, idx, (LPARAM)plvc);
    }

    EckInline BOOL GetColumnOrderArray(_Out_writes_(cColumn) int* piOrder, int cColumn) const noexcept
    {
        return (BOOL)SendMessage(LVM_GETCOLUMNORDERARRAY, cColumn, (LPARAM)piOrder);
    }

    /// <summary>
    /// 取列顺序
    /// </summary>
    /// <param name="vOrder">将列顺序尾插到此参数</param>
    /// <returns>尾插的数量</returns>
    EckInline int GetColumnOrderArray(std::vector<int>& vOrder) const noexcept
    {
        const int cColumn = (int)SendMessageW(GetHeaderControl(), HDM_GETITEMCOUNT, 0, 0);
        if (cColumn <= 0)
            return 0;
        const auto cOld = vOrder.size();
        vOrder.resize(cOld + cColumn);
        GetColumnOrderArray(vOrder.data() + cOld, cColumn);
        return cColumn;
    }

    EckInline int GetColumnWidth(int idx) const noexcept
    {
        return (int)SendMessage(LVM_GETCOLUMNWIDTH, idx, 0);
    }

    EckInline int GetCountPerPage() const noexcept
    {
        return (int)SendMessage(LVM_GETCOUNTPERPAGE, 0, 0);
    }

    EckInline HWND GetEditControl() const noexcept
    {
        return (HWND)SendMessage(LVM_GETEDITCONTROL, 0, 0);
    }

    EckInline BOOL GetEmptyText(PWSTR pszBuf, int cchBuf) const noexcept
    {
        return (BOOL)SendMessage(LVM_GETEMPTYTEXT, cchBuf, (LPARAM)pszBuf);
    }

    EckInline DWORD GetLVExtendStyle() const noexcept
    {
        return (DWORD)SendMessage(LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0);
    }

    EckInline int GetFocusedGroup() const noexcept
    {
        return (int)SendMessage(LVM_GETFOCUSEDGROUP, 0, 0);
    }

    EckInline int GetFooterItemCount() const noexcept
    {
        LVFOOTERINFO lvfi{ LVFF_ITEMCOUNT };
        SendMessage(LVM_GETFOOTERINFO, 0, (LPARAM)&lvfi);
        return lvfi.cItems;
    }

    EckInline BOOL GetFooterItem(int idx, LVFOOTERITEM* plvfi) const noexcept
    {
        return (BOOL)SendMessage(LVM_GETFOOTERITEM, idx, (LPARAM)plvfi);
    }

    EckInline BOOL GetFooterItemRect(int idx, RECT* prc) const noexcept
    {
        return (BOOL)SendMessage(LVM_GETFOOTERITEMRECT, idx, (LPARAM)prc);
    }

    EckInline BOOL GetFooterRect(RECT* prc) const noexcept
    {
        return (BOOL)SendMessage(LVM_GETFOOTERRECT, 0, (LPARAM)prc);
    }

    EckInline int GetGroupCount() const noexcept
    {
        return (int)SendMessage(LVM_GETGROUPCOUNT, 0, 0);
    }

    EckInline int GetGroup(int iGroupID, LVGROUP* plvg) const noexcept
    {
        return (int)SendMessage(LVM_GETGROUPINFO, iGroupID, (LPARAM)plvg);
    }

    EckInline int GetGroupByIndex(int idx, LVGROUP* plvg) const noexcept
    {
        return (int)SendMessage(LVM_GETGROUPINFOBYINDEX, idx, (LPARAM)plvg);
    }

    EckInline void GetGroupMetrics(LVGROUPMETRICS* plvgm) const noexcept
    {
        SendMessage(LVM_GETGROUPMETRICS, 0, (LPARAM)plvgm);
    }

    /// <summary>
    /// 取组矩形
    /// </summary>
    /// <param name="iGroupID">组ID</param>
    /// <param name="prc">接收矩形</param>
    /// <param name="uType">类型，LVGGR_常量，默认LVGGR_GROUP</param>
    /// <returns></returns>
    EckInline BOOL GetGroupRect(int iGroupID, RECT* prc, UINT uType = LVGGR_GROUP) const noexcept
    {
        prc->top = uType;
        return (BOOL)SendMessage(LVM_GETGROUPRECT, iGroupID, (LPARAM)prc);
    }

    /// <summary>
    /// 取组状态
    /// </summary>
    /// <param name="iGroupID">组ID</param>
    /// <param name="uState">状态掩码，LVGS_常量</param>
    /// <returns>状态，只有掩码指定的位有效</returns>
    EckInline UINT GetGroupState(int iGroupID, UINT uState) const noexcept
    {
        return (UINT)SendMessage(LVM_GETGROUPSTATE, iGroupID, uState);
    }

    EckInline HWND GetHeaderControl() const noexcept
    {
        return (HWND)SendMessage(LVM_GETHEADER, 0, 0);
    }

    EckInline HCURSOR GetHotCursor() const noexcept
    {
        return (HCURSOR)SendMessage(LVM_GETHOTCURSOR, 0, 0);
    }

    EckInline int GetHotItem() const noexcept
    {
        return (int)SendMessage(LVM_GETHOTITEM, 0, 0);
    }

    EckInline UINT GetHoverTime() const noexcept
    {
        return (UINT)SendMessage(LVM_GETHOVERTIME, 0, 0);
    }

    /// <summary>
    /// 取图像列表
    /// </summary>
    /// <param name="uType">类型，LVSIL_常量，默认LVSIL_NORMAL</param>
    /// <returns>图像列表句柄</returns>
    EckInline HIMAGELIST GetImageList(UINT uType = LVSIL_NORMAL) const noexcept
    {
        return (HIMAGELIST)SendMessage(LVM_GETIMAGELIST, uType, 0);
    }

    EckInline int GetInsertMark(_Out_opt_ BOOL* pbAfterItem) const noexcept
    {
        LVINSERTMARK lvim{ sizeof(LVINSERTMARK) };
        if (!SendMessage(LVM_GETINSERTMARK, 0, (LPARAM)&lvim))
        {
            if (pbAfterItem)
                *pbAfterItem = FALSE;
            return -1;
        }
        if (pbAfterItem)
            *pbAfterItem = IsBitSet(lvim.dwFlags, LVIM_AFTER);
        return lvim.iItem;
    }

    EckInline COLORREF GetInsertMarkColor() const noexcept
    {
        return (COLORREF)SendMessage(LVM_GETINSERTMARKCOLOR, 0, 0);
    }

    EckInline BOOL GetInsertMarkRect(RECT* prc) const noexcept
    {
        return (BOOL)SendMessage(LVM_GETINSERTMARKRECT, 0, (LPARAM)prc);
    }

    EckInline int GetIncrementalSearchString(PWSTR pszBuf) const noexcept
    {
        return (int)SendMessage(LVM_GETISEARCHSTRINGW, 0, (LPARAM)pszBuf);
    }

    EckInline BOOL GetItem(LVITEMW* pli) const noexcept
    {
        return (BOOL)SendMessage(LVM_GETITEMW, 0, (LPARAM)pli);
    }

    EckInline int GetItemCount() const noexcept
    {
        return (int)SendMessage(LVM_GETITEMCOUNT, 0, 0);
    }

    EckInline BOOL GetItemRect(int idx, int idxGroup,
        _Out_ RECT* prc, UINT uType = LVIR_BOUNDS) const noexcept
    {
        LVITEMINDEX lvii;
        lvii.iItem = idx;
        lvii.iGroup = idxGroup;
        prc->left = uType;
        return (BOOL)SendMessage(LVM_GETITEMINDEXRECT, (WPARAM)&lvii, (LPARAM)prc);
    }

    EckInline BOOL GetItemRect(int idx, RECT* prc, UINT uType = LVIR_BOUNDS) const noexcept
    {
        prc->left = uType;
        return (BOOL)SendMessage(LVM_GETITEMRECT, idx, (LPARAM)prc);
    }

    EckInline BOOL GetItemPosition(int idx, POINT* ppt) const noexcept
    {
        return (BOOL)SendMessage(LVM_GETITEMPOSITION, idx, (LPARAM)ppt);
    }

    EckInline void GetItemSpacing(BOOL bSmallIconView,
        _Out_opt_ int* pxSpacing = nullptr,
        _Out_opt_ int* pySpacing = nullptr) const noexcept
    {
        const auto uRet = (UINT)SendMessage(LVM_GETITEMSPACING, bSmallIconView, 0);
        if (pxSpacing)
            *pxSpacing = LOWORD(uRet);
        if (pySpacing)
            *pySpacing = HIWORD(uRet);
    }

    /// <summary>
    /// 取项目状态
    /// </summary>
    /// <param name="idx">项目</param>
    /// <param name="uState">状态掩码</param>
    /// <returns>状态</returns>
    EckInline UINT GetItemState(int idx, UINT uState) const noexcept
    {
        return (UINT)SendMessage(LVM_GETITEMSTATE, idx, uState);
    }

    /// <summary>
    /// 取项目文本
    /// </summary>
    /// <param name="idx">项目</param>
    /// <param name="idxSubItem">列</param>
    /// <param name="pszBuf">缓冲区，不可为NULL</param>
    /// <param name="cchBuf">缓冲区字符数</param>
    /// <returns>复制到缓冲区的字符数</returns>
    EckInline int GetItemText(int idx, int idxSubItem, PWSTR pszBuf, int cchBuf) const noexcept
    {
        LVITEMW li;
        li.iSubItem = idxSubItem;
        li.pszText = pszBuf;
        li.cchTextMax = cchBuf;
        return (int)SendMessage(LVM_GETITEMTEXTW, idx, (LPARAM)&li);
    }

    /// <summary>
    /// 取项目句柄。
    /// 最多获取前259个字符
    /// </summary>
    /// <param name="idx">项目</param>
    /// <param name="idxSubItem">列</param>
    /// <returns>文本</returns>
    EckInline CStringW GetItemText(int idx, int idxSubItem) const noexcept
    {
        CStringW rs;
        rs.ReSize(MAX_PATH - 1);
        LVITEMW li;
        li.iSubItem = idxSubItem;
        li.pszText = rs.Data();
        li.cchTextMax = MAX_PATH;
        int cch = (int)SendMessage(LVM_GETITEMTEXTW, idx, (LPARAM)&li);
        rs.ReSize(cch);
        return rs;
    }

    /// <summary>
    /// 取下一项目
    /// </summary>
    /// <param name="idx">参照项目，若为-1则查找符合条件的第一个项目</param>
    /// <param name="uFlags">LVNI_常量</param>
    /// <returns>成功返回查找到的项目索引，失败返回-1</returns>
    EckInline int GetNextItem(int idx, UINT uFlags) const noexcept
    {
        return (int)SendMessage(LVM_GETNEXTITEM, idx, uFlags);
    }

    /// <summary>
    /// 取下一项目
    /// </summary>
    /// <param name="idx">参照项目，若为-1则查找符合条件的第一个项目</param>
    /// <param name="idxGroup">组索引</param>
    /// <param name="uFlags">LVNI_常量</param>
    /// <returns>成功返回查找到的项目索引，失败返回-1</returns>
    EckInline int GetNextItem(int idx, int idxGroup, UINT uFlags) const noexcept
    {
        if (idx < 0)
            return (int)SendMessage(LVM_GETNEXTITEMINDEX, -1, uFlags);
        else
        {
            LVITEMINDEX lvii;
            lvii.iItem = idx;
            lvii.iGroup = idxGroup;
            return (int)SendMessage(LVM_GETNEXTITEMINDEX, (WPARAM)&lvii, uFlags);
        }
    }

    EckInline int GetNumberOfWorkAreas() const noexcept
    {
        UINT u = 0;
        SendMessage(LVM_GETNUMBEROFWORKAREAS, 0, (LPARAM)&u);
        return u;
    }

    EckInline BOOL GetOrigin(_Out_ POINT* ppt) const noexcept
    {
        return (BOOL)SendMessage(LVM_GETORIGIN, 0, (LPARAM)ppt);
    }

    EckInline COLORREF GetOutLineColor() const noexcept
    {
        return (COLORREF)SendMessage(LVM_GETOUTLINECOLOR, 0, 0);
    }

    EckInline int GetSelectedColumn() const noexcept
    {
        return (int)SendMessage(LVM_GETSELECTEDCOLUMN, 0, 0);
    }

    EckInline int GetSelectedCount() const noexcept
    {
        return (int)SendMessage(LVM_GETSELECTEDCOUNT, 0, 0);
    }

    EckInline int GetSelectionMark() const noexcept
    {
        return (int)SendMessage(LVM_GETSELECTIONMARK, 0, 0);
    }

    EckInline int GetStringWidth(PCWSTR pszText) const noexcept
    {
        return (int)SendMessage(LVM_GETSTRINGWIDTHW, 0, (LPARAM)pszText);
    }

    EckInline int GetSubItemRect(int idx, int idxSubItem,
        _Out_ RECT* prc, UINT uType = LVIR_BOUNDS) const noexcept
    {
        prc->top = idxSubItem;
        prc->left = uType;
        return (int)SendMessage(LVM_GETSUBITEMRECT, idx, (LPARAM)prc);
    }

    EckInline COLORREF GetTextBackgroundColor() const noexcept
    {
        return (COLORREF)SendMessage(LVM_GETTEXTBKCOLOR, 0, 0);
    }

    EckInline COLORREF GetTextColor() const noexcept
    {
        return (COLORREF)SendMessage(LVM_GETTEXTCOLOR, 0, 0);
    }

    EckInline void GetTileInfomation(_Inout_ LVTILEINFO* plvti) const noexcept
    {
        SendMessage(LVM_GETTILEINFO, 0, (LPARAM)plvti);
    }

    EckInline void GetTileViewInfomation(_Inout_ LVTILEVIEWINFO* plvtvi) const noexcept
    {
        SendMessage(LVM_GETTILEVIEWINFO, 0, (LPARAM)plvtvi);
    }

    EckInline HWND GetToolTips() const noexcept
    {
        return (HWND)SendMessage(LVM_GETTOOLTIPS, 0, 0);
    }

    EckInline int GetTopIndex() const noexcept
    {
        return (int)SendMessage(LVM_GETTOOLTIPS, 0, 0);
    }

    EckInline BOOL GetUnicodeFormat() const noexcept
    {
        return (BOOL)SendMessage(LVM_GETUNICODEFORMAT, 0, 0);
    }

    /// <summary>
    /// 取当前视图
    /// </summary>
    /// <returns>LV_VIEW_常量</returns>
    EckInline UINT GetView() const noexcept
    {
        return (UINT)SendMessage(LVM_GETVIEW, 0, 0);
    }

    EckInline BOOL GetViewRect(_Out_ RECT* prc) const noexcept
    {
        return (BOOL)SendMessage(LVM_GETVIEWRECT, 0, (LPARAM)prc);
    }

    EckInline std::vector<RECT> GetWorkAreas() const noexcept
    {
        std::vector<RECT> aRect{};
        int c = GetNumberOfWorkAreas();
        if (!c)
            return aRect;
        aRect.resize(c);
        SendMessage(LVM_GETWORKAREAS, c, (LPARAM)aRect.data());
        return aRect;
    }

    EckInline void GetWorkAreas(_Out_writes_(c) RECT* prc, int c) const noexcept
    {
        SendMessage(LVM_GETWORKAREAS, c, (LPARAM)prc);
    }

    EckInline BOOL HasGroup(int iGroupID) const noexcept
    {
        return (BOOL)SendMessage(LVM_HASGROUP, iGroupID, 0);
    }

    EckInline int HitTest(_Inout_ LVHITTESTINFO* plvhti,
        BOOL bTestGroup = FALSE) const noexcept
    {
        return (int)SendMessage(LVM_HITTEST, bTestGroup ? -1 : 0, (LPARAM)plvhti);
    }

    EckInline int InsertColumn(int idx, _In_ const LVCOLUMNW* plvc) const noexcept
    {
        return (int)SendMessage(LVM_INSERTCOLUMNW, idx, (LPARAM)plvc);
    }

    EckInline int InsertColumn(PCWSTR pszText, int idx = -1,
        int cxColumn = 160, int iFmt = LVCFMT_LEFT, int idxImage = -1) const noexcept
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

    EckInline int InsertGroup(int idx, _In_ const LVGROUP* plvg) const noexcept
    {
        return (int)SendMessage(LVM_INSERTGROUP, idx, (LPARAM)plvg);
    }

    EckInline int InsertGroupSorted(PFNLVGROUPCOMPARE pfnCmp,
        void* pData, LVINSERTGROUPSORTED* plvigs) const noexcept
    {
        plvigs->pfnGroupCompare = pfnCmp;
        plvigs->pvData = pData;
        plvigs->lvGroup.cbSize = sizeof(LVGROUP);
        return (int)SendMessage(LVM_INSERTGROUPSORTED, 0, (LPARAM)plvigs);
    }

    EckInline int InsertItem(const LVITEMW* plvi) const noexcept
    {
        return (int)SendMessage(LVM_INSERTITEM, 0, (LPARAM)plvi);
    }

    EckInline int InsertItem(PCWSTR pszText, int idx = -1,
        LPARAM lParam = 0, int idxImage = -1) const noexcept
    {
        if (idx < 0)
            idx = GetItemCount();
        LVITEMW lvi;
        lvi.mask = LVIF_TEXT | LVIF_PARAM;
        lvi.iItem = idx;
        lvi.lParam = lParam;
        lvi.pszText = (PWSTR)pszText;
        lvi.iSubItem = 0;
        if (idxImage >= 0)
        {
            lvi.mask |= LVIF_IMAGE;
            lvi.iImage = idxImage;
        }
        return InsertItem(&lvi);
    }

    EckInline int InsertMarkHitTest(POINT pt, BOOL bAfterItem = FALSE) const noexcept
    {
        LVINSERTMARK lvim{ sizeof(LVINSERTMARK) };
        lvim.iItem = -1;
        if (bAfterItem)
            lvim.dwFlags = LVIM_AFTER;
        SendMessage(LVM_INSERTMARKHITTEST, (WPARAM)&pt, (LPARAM)&lvim);
        return lvim.iItem;
    }

    EckInline BOOL IsGroupViewEnabled() const noexcept
    {
        return (BOOL)SendMessage(LVM_ISGROUPVIEWENABLED, 0, 0);
    }

    EckInline BOOL IsItemVisible(int idx) const noexcept
    {
        return (BOOL)SendMessage(LVM_ISITEMVISIBLE, idx, 0);
    }

    EckInline UINT MapIdToIndex(int idx) const noexcept
    {
        return (UINT)SendMessage(LVM_MAPIDTOINDEX, idx, 0);
    }

    EckInline int MapIndexToId(UINT uId) const noexcept
    {
        return (int)SendMessage(LVM_MAPINDEXTOID, uId, 0);
    }

    EckInline BOOL RedrawItems(int idxStart, int idxEnd) const noexcept
    {
        return (BOOL)SendMessage(LVM_REDRAWITEMS, idxStart, idxEnd);
    }

    EckInline BOOL RedrawItem(int idx) const noexcept
    {
        return RedrawItems(idx, idx);
    }

    EckInline void RemoveGroup() const noexcept
    {
        SendMessage(LVM_REMOVEALLGROUPS, 0, 0);
    }

    /// <summary>
    /// 删除组
    /// </summary>
    /// <param name="iGroupID">组ID</param>
    /// <returns>成功返回组索引，失败返回-1</returns>
    EckInline int RemoveGroup(int iGroupID) const noexcept
    {
        return (int)SendMessage(LVM_REMOVEGROUP, iGroupID, 0);
    }

    EckInline BOOL Scroll(int deltaH = 0, int deltaV = 0) const noexcept
    {
        return (BOOL)SendMessage(LVM_SCROLL, deltaH, deltaV);
    }

    /// <summary>
    /// 置背景颜色
    /// </summary>
    /// <param name="cr">颜色，或CLR_NONE指定无背景色</param>
    /// <returns>成功返回TRUE，失败返回FALSE</returns>
    EckInline BOOL SetBackgroundColor(COLORREF cr) const noexcept
    {
        return (BOOL)SendMessage(LVM_SETBKCOLOR, 0, cr);
    }

    EckInline BOOL SetBackgroundImage(LVBKIMAGEW* plvbki) const noexcept
    {
        return (BOOL)SendMessage(LVM_SETBKIMAGEW, 0, (LPARAM)plvbki);
    }

    /// <summary>
    /// 置回调掩码
    /// </summary>
    /// <param name="uMask">掩码，LVIS_常量</param>
    /// <returns>成功返回TRUE，失败返回FALSE</returns>
    EckInline BOOL SetCallbackMask(UINT uMask) const noexcept
    {
        return (BOOL)SendMessage(LVM_SETCALLBACKMASK, uMask, 0);
    }

    EckInline BOOL SetColumn(int idx, LVCOLUMNW* plvc) const noexcept
    {
        return (BOOL)SendMessage(LVM_SETCOLUMNW, idx, (LPARAM)plvc);
    }

    EckInline BOOL SetColumnOrderArray(_In_reads_(c) const int* piOrder, int c) const noexcept
    {
        return (BOOL)SendMessage(LVM_SETCOLUMNORDERARRAY, c, (LPARAM)piOrder);
    }

    /// <summary>
    /// 置列宽
    /// </summary>
    /// <param name="idx">索引，对于列表模式此参数必须设置为0</param>
    /// <param name="cx">宽度，或LVSCW_AUTOSIZE指定自动调整大小，或LVSCW_AUTOSIZE_USEHEADER指定适应标题文本</param>
    /// <returns>成功返回TRUE，失败返回FALSE</returns>
    EckInline BOOL SetColumnWidth(int idx, int cx) const noexcept
    {
        return (BOOL)SendMessage(LVM_SETCOLUMNWIDTH, idx, cx);
    }

    /// <summary>
    /// 置列表视图扩展样式
    /// </summary>
    /// <param name="dwNew">新样式</param>
    /// <param name="dwMask">掩码，若为0则修改所有样式</param>
    /// <returns>返回旧样式</returns>
    EckInline DWORD SetLVExtendStyle(DWORD dwNew, DWORD dwMask = 0) const noexcept
    {
        return (DWORD)SendMessage(LVM_SETEXTENDEDLISTVIEWSTYLE, dwMask, dwNew);
    }

    /// <summary>
    /// 置组信息
    /// </summary>
    /// <param name="iGroupID">组ID</param>
    /// <param name="plvg">LVGROUP指针</param>
    /// <returns>成功返回组ID，失败返回-1</returns>
    EckInline int SetGroup(int iGroupID, _In_ const LVGROUP* plvg) const noexcept
    {
        return (int)SendMessage(LVM_SETGROUPINFO, iGroupID, (LPARAM)plvg);
    }

    EckInline void SetGroupMetrics(_In_ const LVGROUPMETRICS* plvgm) const noexcept
    {
        SendMessage(LVM_SETGROUPMETRICS, 0, (LPARAM)plvgm);
    }

    EckInline HCURSOR SetHotCursor(HCURSOR hCursor) const noexcept
    {
        return (HCURSOR)SendMessage(LVM_SETHOTCURSOR, 0, (LPARAM)hCursor);
    }

    EckInline int SetHotItem(int idx) const noexcept
    {
        return (int)SendMessage(LVM_SETHOTITEM, idx, 0);
    }

    EckInline UINT SetHoverTime(UINT uTime = (UINT)-1) const noexcept
    {
        return (UINT)SendMessage(LVM_SETHOVERTIME, 0, uTime);
    }

    /// <summary>
    /// 置大图标模式图标间距。
    /// 若xSpacing和ySpacing均设为-1，则使用默认间距
    /// </summary>
    /// <param name="xSpacing">水平间隔</param>
    /// <param name="ySpacing">垂直间隔</param>
    /// <returns>返回值的低字为先前的水平距离，高字为先前的垂直距离</returns>
    EckInline UINT SetIconSpacing(int xSpacing = -1, int ySpacing = -1) const noexcept
    {
        return (UINT)SendMessage(LVM_SETICONSPACING, 0, MAKELPARAM(xSpacing, ySpacing));
    }

    EckInline HIMAGELIST SetImageList(HIMAGELIST hImageList, UINT uType = LVSIL_NORMAL) const noexcept
    {
        return (HIMAGELIST)SendMessage(LVM_SETIMAGELIST, uType, (LPARAM)hImageList);
    }

    EckInline BOOL SetInfomationTip(LVSETINFOTIP* plvsit) const noexcept
    {
        plvsit->cbSize = sizeof(LVSETINFOTIP);
        return (BOOL)SendMessage(LVM_SETINFOTIP, 0, (LPARAM)plvsit);
    }

    EckInline BOOL SetInsertMark(int idx, BOOL bAfterItem = FALSE) const noexcept
    {
        LVINSERTMARK lvim{ sizeof(LVINSERTMARK), DWORD(bAfterItem ? LVIM_AFTER : 0), idx };
        return (BOOL)SendMessage(LVM_SETINSERTMARK, 0, (LPARAM)&lvim);
    }

    EckInline BOOL SetInsertMark(_In_ const LVINSERTMARK* plvim) const noexcept
    {
        return (BOOL)SendMessage(LVM_SETINSERTMARK, 0, (LPARAM)plvim);
    }

    EckInline COLORREF SetInsertMarkColor(COLORREF cr) const noexcept
    {
        return (COLORREF)SendMessage(LVM_SETINSERTMARKCOLOR, 0, cr);
    }

    EckInline BOOL SetItem(_In_ const LVITEMW* plvi) const noexcept
    {
        return (BOOL)SendMessage(LVM_SETITEMW, 0, (LPARAM)plvi);
    }

    /// <summary>
    /// 置项目数。
    /// 若控件具有所有者数据样式则此方法设置项目数，否则指示控件为指定的项数分配内存
    /// </summary>
    /// <param name="iCount">项目数</param>
    /// <param name="uFlags">标志，LVSICF_常量</param>
    /// <returns>成功返回TRUE，失败返回FALSE</returns>
    EckInline BOOL SetItemCount(int iCount, UINT uFlags = 0) const noexcept
    {
        return (BOOL)SendMessage(LVM_SETITEMCOUNT, iCount, uFlags);
    }

    EckInline BOOL SetItemState(int idx, int idxGroup, UINT uState, UINT uMask) const noexcept
    {
        LVITEMW li;
        li.state = uState;
        li.stateMask = uMask;
        LVITEMINDEX lvii;
        lvii.iItem = idx;
        lvii.iGroup = idxGroup;
        return (BOOL)SendMessage(LVM_SETITEMINDEXSTATE, (WPARAM)&lvii, (LPARAM)&li);
    }

    EckInline BOOL SetItemState(_In_ const LVITEMINDEX* plvii, _In_ const LVITEMW* plvi) const noexcept
    {
        return (BOOL)SendMessage(LVM_SETITEMINDEXSTATE, (WPARAM)plvii, (LPARAM)plvi);
    }

    EckInline BOOL SetItemState(int idx, UINT uState, UINT uMask) const noexcept
    {
        LVITEMW lvi;
        lvi.state = uState;
        lvi.stateMask = uMask;
        return (BOOL)SendMessage(LVM_SETITEMSTATE, idx, (LPARAM)&lvi);
    }

    EckInline BOOL SetItemState(int idx, _In_ const LVITEMW* plvi) const noexcept
    {
        return (BOOL)SendMessage(LVM_SETITEMSTATE, idx, (LPARAM)plvi);
    }

    EckInline void SetItemPosition(int idx, POINT pt) const noexcept
    {
        SendMessage(LVM_SETITEMPOSITION32, idx, (LPARAM)&pt);
    }

    EckInline BOOL SetItemText(int idx, int idxSubItem, PCWSTR pszText) const noexcept
    {
        LVITEMW li;
        li.iSubItem = idxSubItem;
        li.pszText = (PWSTR)pszText;
        return SetItemText(idx, &li);
    }

    EckInline BOOL SetItemText(int idx, PCWSTR pszText) const noexcept
    {
        LVITEMW li;
        li.iSubItem = 0;
        li.pszText = (PWSTR)pszText;
        return SetItemText(idx, &li);
    }

    EckInline BOOL SetItemText(int idx, _In_ const LVITEMW* pli) const noexcept
    {
        return (BOOL)SendMessage(LVM_SETITEMTEXTW, idx, (LPARAM)pli);
    }

    EckInline COLORREF SetOutLineColor(COLORREF cr) const noexcept
    {
        return (COLORREF)SendMessage(LVM_SETOUTLINECOLOR, 0, cr);
    }

    EckInline void SetSelectedColumn(int idx) const noexcept
    {
        SendMessage(LVM_SETSELECTEDCOLUMN, idx, 0);
    }

    EckInline int SetSelectionMark(int idx) const noexcept
    {
        return (int)SendMessage(LVM_SETSELECTIONMARK, 0, idx);
    }

    EckInline COLORREF SetTextBackgroundColor(COLORREF cr) const noexcept
    {
        return (COLORREF)SendMessage(LVM_SETTEXTBKCOLOR, 0, cr);
    }

    EckInline COLORREF SetTextForegroundColor(COLORREF cr) const noexcept
    {
        return (COLORREF)SendMessage(LVM_SETTEXTCOLOR, 0, cr);
    }

    EckInline BOOL SetTileInfomation(int idx, UINT cColumn,
        _In_reads_(cColumn) const UINT* piColumn,
        _In_reads_(cColumn) const int* piFmt) const noexcept
    {
        LVTILEINFO lvti{ sizeof(LVTILEINFO), idx, cColumn, (UINT*)piColumn, (int*)piFmt };
        return (BOOL)SendMessage(LVM_SETTILEINFO, 0, (LPARAM)&lvti);
    }

    EckInline BOOL SetTileInfomation(_In_ const LVTILEINFO* plvti) const noexcept
    {
        return (BOOL)SendMessage(LVM_SETTILEINFO, 0, (LPARAM)plvti);
    }

    EckInline BOOL SetTileViewInfo(_In_ const LVTILEVIEWINFO* plvtvi) const noexcept
    {
        return (BOOL)SendMessage(LVM_SETTILEVIEWINFO, 0, (LPARAM)plvtvi);
    }

    EckInline HWND SetToolTips(HWND hToolTip) const noexcept
    {
        return (HWND)SendMessage(LVM_SETTOOLTIPS, (WPARAM)hToolTip, 0);
    }

    EckInline BOOL SetUnicodeFormat(BOOL bUnicode) const noexcept
    {
        return (BOOL)SendMessage(LVM_SETUNICODEFORMAT, bUnicode, 0);
    }

    EckInline BOOL SetView(UINT uView) const noexcept
    {
        return (SendMessage(LVM_SETVIEW, uView, 0) > 0);
    }

    EckInline void SetWorkArea(_In_reads_opt_(c) const RECT* prc, int c) const noexcept
    {
        SendMessage(LVM_SETWORKAREAS, c, (LPARAM)prc);
    }

    /// <summary>
    /// 排序组
    /// </summary>
    /// <param name="pfnCmp">排序函数。第一、二个参数为两组的ID，若组1大于组2则返回正值，若小于则返回负值，若等于则返回0</param>
    /// <param name="pData">自定义数据</param>
    /// <returns>成功返回TRUE，失败返回FALSE</returns>
    EckInline BOOL SortGroups(PFNLVGROUPCOMPARE pfnCmp, void* pData) const noexcept
    {
        return (BOOL)SendMessage(LVM_SORTGROUPS, (WPARAM)pfnCmp, (LPARAM)pData);
    }

    /// <summary>
    /// 排序项目
    /// </summary>
    /// <param name="pfnCmp">排序函数。第一、二个参数为两项目的lParam，若项目1在项目2之后则返回正值，若在之前则返回负值，若相等则返回0</param>
    /// <param name="pData"></param>
    /// <returns></returns>
    EckInline BOOL SortItemslParam(FLvItemCompare pfnCmp, LPARAM lParam) const noexcept
    {
        return (BOOL)SendMessage(LVM_SORTITEMS, lParam, (LPARAM)pfnCmp);
    }

    /// <summary>
    /// 排序项目
    /// </summary>
    /// <param name="pfnCmp">排序函数。第一、二个参数为两项目的索引，若项目1在项目2之后则返回正值，若在之前则返回负值，若相等则返回0</param>
    /// <param name="pData"></param>
    /// <returns></returns>
    EckInline BOOL SortItemsIndex(FLvItemCompareEx pfnCmp, LPARAM lParam) const noexcept
    {
        return (BOOL)SendMessage(LVM_SORTITEMSEX, lParam, (LPARAM)pfnCmp);
    }

    EckInline int SubItemHitTest(_Inout_ LVHITTESTINFO* plvhti, BOOL bTestGroup = FALSE) const noexcept
    {
        return (int)SendMessage(LVM_SUBITEMHITTEST, bTestGroup ? -1 : 0, (LPARAM)plvhti);
    }

    EckInline BOOL Update() const noexcept
    {
        return (BOOL)SendMessage(LVM_UPDATE, 0, 0);
    }

    EckInline int GetCurrentSelection() const noexcept
    {
        EckCounter(GetItemCount(), i)
        {
            if (GetItemState(i, LVIS_SELECTED) == LVIS_SELECTED)
                return i;
        }
        return -1;
    }

    BOOL SetRowHeight(int cy, BOOL bSetOrAdd = TRUE) const noexcept
    {
        const auto pParent = CWindowFromHWND(GetParent(HWnd));
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
        SendMessage(WM_WINDOWPOSCHANGED, 0, (LPARAM)&wp);
        Style &= ~LVS_OWNERDRAWFIXED;
        pParent->GetSignal().Disconnect(hSlot);
        return TRUE;
    }

    BOOL MakePretty_RowHeight() const noexcept
    {
        return SetRowHeight(DpiScale(6, GetDpi(HWnd)), FALSE);
    }

    BOOL MakePretty() const noexcept
    {
        constexpr DWORD dwStyle = LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER;
        SetLVExtendStyle(dwStyle, dwStyle);
        return MakePretty_RowHeight();
    }

    int GetItemCheckState(int idx) const noexcept
    {
        return (GetItemState(idx, LVIS_STATEIMAGEMASK) >> 12) - 1;
    }

    BOOL SetItemCheckState(int idx, int iCheck) const noexcept
    {
        return SetItemState(idx, INDEXTOSTATEIMAGEMASK(iCheck + 1), LVIS_STATEIMAGEMASK);
    }

    EckInlineNd IListView2* GetLvObject() const noexcept
    {
        IListView2* pLv{};
        SendMessage(LVM_QUERYINTERFACE, (WPARAM)&IID_IListView2, (LPARAM)&pLv);
        return pLv;
    }

    EckInline LRESULT GetLvObject(REFIID riid, void** ppv) const noexcept
    {
        return SendMessage(LVM_QUERYINTERFACE, (WPARAM)&riid, (LPARAM)ppv);
    }

    EckInline void EnableSpacePartSelect(BOOL bEnable) const noexcept
    {
        const auto pLv = GetLvObject();
        pLv->SetSelectionFlags(1, bEnable ? 1 : 0);
        pLv->Release();
    }
};
ECK_NAMESPACE_END