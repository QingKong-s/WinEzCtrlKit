#pragma once
#include "CWnd.h"

ECK_NAMESPACE_BEGIN
struct HEADER_FILTER
{
    int iType;
    union
    {
        int i;
        SYSTEMTIME st;
        HD_TEXTFILTERW str;
    };
};

class CHeader : public CWnd
{
public:
    ECK_RTTI(CHeader, CWnd);
    ECK_CWND_NOSINGLEOWNER(CHeader);
    ECK_CWND_CREATE_CLS(WC_HEADERW);

    ECK_CWNDPROP_STYLE(Buttons, HDS_BUTTONS);
    ECK_CWNDPROP_STYLE(DragDrop, HDS_DRAGDROP);
    ECK_CWNDPROP_STYLE(FilterBar, HDS_FILTERBAR);
    ECK_CWNDPROP_STYLE(Flat, HDS_FLAT);
    ECK_CWNDPROP_STYLE(FullDrag, HDS_FULLDRAG);
    ECK_CWNDPROP_STYLE(Hidden, HDS_HIDDEN);
    ECK_CWNDPROP_STYLE(Horz, HDS_HORZ);
    ECK_CWNDPROP_STYLE(HotTrack, HDS_HOTTRACK);
    ECK_CWNDPROP_STYLE(CheckBoxes, HDS_CHECKBOXES);
    ECK_CWNDPROP_STYLE(NoSizing, HDS_NOSIZING);
    ECK_CWNDPROP_STYLE(OverFlow, HDS_OVERFLOW);
public:
    /// <summary>
    /// 清除筛选器
    /// </summary>
    /// <param name="idx">列索引，若为-1则清除所有筛选器</param>
    /// <returns></returns>
    EckInline BOOL ClearFilter(int idx) const noexcept
    {
        return (BOOL)SendMsg(HDM_CLEARFILTER, idx, 0);
    }

    EckInline HIMAGELIST CreateDragImage(int idx) const noexcept
    {
        return (HIMAGELIST)SendMsg(HDM_CREATEDRAGIMAGE, idx, 0);
    }

    EckInline BOOL DeleteItem(int idx) const noexcept
    {
        return (BOOL)SendMsg(HDM_DELETEITEM, idx, 0);
    }

    EckInline BOOL EditFilter(int idx, BOOL bDiscardUserInput) const noexcept
    {
        return (BOOL)SendMsg(HDM_CLEARFILTER, idx, bDiscardUserInput);
    }

    EckInline int GetBitmapMargin() const noexcept
    {
        return (int)SendMsg(HDM_GETBITMAPMARGIN, 0, 0);
    }

    EckInline int GetFocusItem() const noexcept
    {
        return (int)SendMsg(HDM_GETFOCUSEDITEM, 0, 0);
    }

    EckInline HIMAGELIST GetImageList(UINT uType) const noexcept
    {
        return (HIMAGELIST)SendMsg(HDM_GETIMAGELIST, uType, 0);
    }

    EckInline HIMAGELIST GetImageList() const noexcept { return GetImageList(HDSIL_NORMAL); }

    EckInline BOOL GetItem(int idx, _Inout_ HDITEMW* phdi) const noexcept
    {
        return (BOOL)SendMsg(HDM_GETITEMW, idx, (LPARAM)phdi);
    }

    EckInline int GetItemCount() const noexcept
    {
        return (int)SendMsg(HDM_GETITEMCOUNT, 0, 0);
    }

    /// <summary>
    /// 取项目拆分按钮矩形
    /// </summary>
    /// <param name="idx">项目索引</param>
    /// <param name="prc">矩形指针，相对控件父窗口</param>
    /// <returns>成功返回TRUE，失败返回FALSE</returns>
    EckInline BOOL GetItemDropDownRect(int idx, _Out_ RECT* prc) const noexcept
    {
        return (BOOL)SendMsg(HDM_GETITEMDROPDOWNRECT, idx, (LPARAM)prc);
    }

    /// <summary>
    /// 取项目矩形
    /// </summary>
    /// <param name="idx">项目索引</param>
    /// <param name="prc">矩形指针，相对控件父窗口</param>
    /// <returns>成功返回TRUE，失败返回FALSE</returns>
    EckInline BOOL GetItemRect(int idx, _Out_ RECT* prc) const noexcept
    {
        return (BOOL)SendMsg(HDM_GETITEMRECT, idx, (LPARAM)prc);
    }

    EckInline BOOL GetOrderArray(_Out_writes_(cBuf) int* piOrder, size_t cBuf) const noexcept
    {
        return (BOOL)SendMsg(HDM_GETORDERARRAY, cBuf, (LPARAM)piOrder);
    }

    EckInline BOOL GetOrderArray(std::vector<int>& vOrder) const noexcept
    {
        vOrder.resize(GetItemCount());
        return GetOrderArray(vOrder.data(), vOrder.size());
    }

    /// <summary>
    /// 取溢出按钮矩形
    /// </summary>
    /// <param name="prc">矩形指针，相对屏幕</param>
    /// <returns>成功返回TRUE，失败返回FALSE</returns>
    EckInline BOOL GetOverFlowRect(_Out_ RECT* prc) const noexcept
    {
        return (BOOL)SendMsg(HDM_GETOVERFLOWRECT, 0, (LPARAM)prc);
    }

    EckInline int HitTest(_Inout_ HDHITTESTINFO* phdhti) const noexcept
    {
        return (int)SendMsg(HDM_HITTEST, 0, (LPARAM)phdhti);
    }

    EckInline int InsertItem(int idx, _In_ const HDITEMW* phdi) const noexcept
    {
        return (int)SendMsg(HDM_INSERTITEMW, idx, (LPARAM)phdi);
    }

    EckInline int InsertItem(PCWSTR pszText, int idx = -1, int cxItem = -1,
        int idxImage = -1, int iFmt = HDF_LEFT, LPARAM lParam = 0) const noexcept
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

    EckInline BOOL Layout(_Inout_ HDLAYOUT* phdl) const noexcept
    {
        return (BOOL)SendMsg(HDM_LAYOUT, 0, (LPARAM)phdl);
    }

    EckInline int OrderToIndex(int iOrder) const noexcept
    {
        return (int)SendMsg(HDM_ORDERTOINDEX, iOrder, 0);
    }

    EckInline int SetBitmapMargin(int iMargin) const noexcept
    {
        return (int)SendMsg(HDM_SETBITMAPMARGIN, iMargin, 0);
    }

    EckInline int SetFilterChangeTimeout(int iTimeout) const noexcept
    {
        return (int)SendMsg(HDM_SETFILTERCHANGETIMEOUT, 0, iTimeout);
    }

    EckInline BOOL SetFocusedItem(int idx) const noexcept
    {
        return (BOOL)SendMsg(HDM_SETFOCUSEDITEM, 0, idx);
    }

    EckInline int SetHotDivider(int idxDivider) const noexcept
    {
        return (int)SendMsg(HDM_SETHOTDIVIDER, FALSE, idxDivider);
    }

    EckInline int SetHotDivider(POINT ptCursor) const noexcept
    {
        return (int)SendMsg(HDM_SETHOTDIVIDER, TRUE, MAKELPARAM(ptCursor.x, ptCursor.y));
    }

    EckInline HIMAGELIST SetImageList(HIMAGELIST hImageList, UINT uType) const noexcept
    {
        return (HIMAGELIST)SendMsg(HDM_SETIMAGELIST, uType, (LPARAM)hImageList);
    }

    EckInline HIMAGELIST SetImageList(HIMAGELIST hImageList) const noexcept
    {
        return SetImageList(hImageList, HDSIL_NORMAL);
    }

    EckInline BOOL SetItem(int idx, _In_ const HDITEMW* phdi) const noexcept
    {
        return (BOOL)SendMsg(HDM_SETITEMW, idx, (LPARAM)phdi);
    }

    EckInline BOOL SetOrderArray(const int* piOrder) const noexcept
    {
        return (BOOL)SendMsg(HDM_SETORDERARRAY, GetItemCount(), (LPARAM)piOrder);
    }

    void RadioSetSortMark(int idx, int iFmt) const noexcept
    {
        HDITEMW hdi;
        hdi.mask = HDI_FORMAT;
        int cItems = GetItemCount();
        EckCounter(cItems, i)
        {
            GetItem(i, &hdi);
            hdi.fmt &= ~(HDF_SORTDOWN | HDF_SORTUP);
            if (i == idx)
                hdi.fmt |= iFmt;
            SetItem(i, &hdi);
        }
    }

    BOOL SetItemFilter(int idx, HEADER_FILTER& hf) const noexcept
    {
        HDITEMW hdi;
        hdi.mask = HDI_FILTER;
        hdi.type = hf.iType;
        switch (hf.iType)
        {
        case HDFT_ISSTRING:
            hdi.pvFilter = &hf.str;
            break;
        case HDFT_ISNUMBER:
            hdi.pvFilter = &hf.i;
            break;
        case HDFT_ISDATE:
            hdi.pvFilter = &hf.st;
            break;
        }
        return SetItem(idx, &hdi);
    }

    BOOL GetItemFilter(int idx, HEADER_FILTER& hf) const noexcept
    {
        hf.iType = HDFT_HASNOVALUE;
        HDITEMW hdi;
        hdi.mask = HDI_FILTER;
        hdi.type = HDFT_HASNOVALUE;
        hdi.pvFilter = nullptr;
        if (!GetItem(idx, &hdi))
            return FALSE;
        if (hdi.type == HDFT_HASNOVALUE)
            return TRUE;
        hf.iType = hdi.type;
        switch (hdi.type)
        {
        case HDFT_ISSTRING:
            hdi.pvFilter = &hf.str;
            break;
        case HDFT_ISNUMBER:
            hdi.pvFilter = &hf.i;
            break;
        case HDFT_ISDATE:
            hdi.pvFilter = &hf.st;
            break;
        }
        return GetItem(idx, &hdi);
    }

    void DeleteAllItems() const noexcept
    {
        const int cItem = GetItemCount();
        for (int i = cItem - 1; i >= 0; --i)
            DeleteItem(i);
    }
};
ECK_NAMESPACE_END