#pragma once
#include "CWindow.h"

ECK_NAMESPACE_BEGIN
class CListBox : public CWindow
{
public:
    ECK_RTTI(CListBox, CWindow);
    ECK_CWND_NOSINGLEOWNER(CListBox);
    ECK_CWND_CREATE_CLS(WC_LISTBOXW);

    ECK_CWNDPROP_STYLE(ComboList, LBS_COMBOBOX);
    ECK_CWNDPROP_STYLE(DisableNoScroll, LBS_DISABLENOSCROLL);
    ECK_CWNDPROP_STYLE(ExtendedSelect, LBS_EXTENDEDSEL);
    ECK_CWNDPROP_STYLE(HasStrings, LBS_HASSTRINGS);
    ECK_CWNDPROP_STYLE(MultiColumn, LBS_MULTICOLUMN);
    ECK_CWNDPROP_STYLE(MultipleSelect, LBS_MULTIPLESEL);
    ECK_CWNDPROP_STYLE(NoData, LBS_NODATA);
    ECK_CWNDPROP_STYLE(NoIntegralHeight, LBS_NOINTEGRALHEIGHT);
    ECK_CWNDPROP_STYLE(NoRedraw, LBS_NOREDRAW);
    ECK_CWNDPROP_STYLE(NoSelection, LBS_NOSEL);
    ECK_CWNDPROP_STYLE(Notify, LBS_NOTIFY);
    ECK_CWNDPROP_STYLE(OwnerDrawFixed, LBS_OWNERDRAWFIXED);
    ECK_CWNDPROP_STYLE(OwnerDrawVariable, LBS_OWNERDRAWVARIABLE);
    ECK_CWNDPROP_STYLE(Sort, LBS_SORT);
    ECK_CWNDPROP_STYLE(Standard, LBS_STANDARD);
    ECK_CWNDPROP_STYLE(UseTabStops, LBS_USETABSTOPS);
    ECK_CWNDPROP_STYLE(WantKeyboradInput, LBS_WANTKEYBOARDINPUT);

    EckInline int AddString(PCWSTR psz) const noexcept
    {
        return (int)SendMessage(LB_ADDSTRING, 0, (LPARAM)psz);
    }

    EckInline int AddString(LPARAM lParam) const noexcept
    {
        return (int)SendMessage(LB_ADDSTRING, 0, lParam);
    }

    /// <summary>
    /// 删除项目
    /// </summary>
    /// <param name="idx"></param>
    /// <returns>返回剩余项目数</returns>
    EckInline int DeleteString(int idx) const noexcept
    {
        return (int)SendMessage(LB_DELETESTRING, idx, 0);
    }

    /// <summary>
    /// 加入路径
    /// </summary>
    /// <param name="pszPath">路径</param>
    /// <param name="uFlags">DDL_常量</param>
    /// <returns>索引</returns>
    EckInline int Directory(PCWSTR pszPath, UINT uFlags) const noexcept
    {
        return (int)SendMessage(LB_DIR, uFlags, (LPARAM)pszPath);
    }

    /// <summary>
    /// 查找项目。
    /// 不区分大小写
    /// </summary>
    /// <param name="pszText">文本，将匹配以该文本开头的项目</param>
    /// <param name="idxStart">起始索引，-1 = 从头搜索整个列表</param>
    /// <returns>索引</returns>
    EckInline int FindString(PCWSTR pszText, int idxStart = -1) const noexcept
    {
        return (int)SendMessage(LB_FINDSTRING, idxStart, (LPARAM)pszText);
    }

    /// <summary>
    /// 查找完全匹配项目。
    /// 不区分大小写
    /// </summary>
    /// <param name="pszText">文本，将匹配与该文本完全相同的项目</param>
    /// <param name="idxStart">起始索引，-1 = 从头搜索整个列表</param>
    /// <returns>索引</returns>
    EckInline int FindStringExact(PCWSTR pszText, int idxStart = -1) const noexcept
    {
        return (int)SendMessage(LB_FINDSTRINGEXACT, idxStart, (LPARAM)pszText);
    }

    EckInline int GetAnchorIndex() const noexcept
    {
        return (int)SendMessage(LB_GETANCHORINDEX, 0, 0);
    }

    /// <summary>
    /// 取焦点项目。
    /// 对单选列表框调用返回现行选中项，对多选列表框调用返回焦点项目
    /// </summary>
    /// <returns></returns>
    EckInline int GetCaretIndex() const noexcept
    {
        return (int)SendMessage(LB_GETCARETINDEX, 0, 0);
    }

    EckInline int GetItemCount() const noexcept
    {
        return (int)SendMessage(LB_GETCOUNT, 0, 0);
    }

    /// <summary>
    /// 取现行选中项。
    /// 对单选列表框调用返回现行选中项，对多选列表框调用返回焦点项目
    /// </summary>
    /// <returns>索引</returns>
    EckInline int GetCurrentSelection() const noexcept
    {
        return (int)SendMessage(LB_GETCURSEL, 0, 0);
    }

    EckInline int GetHorizontalExtent() const noexcept
    {
        return (int)SendMessage(LB_GETHORIZONTALEXTENT, 0, 0);
    }

    EckInline LPARAM GetItemData(int idx) const noexcept
    {
        return SendMessage(LB_GETITEMDATA, idx, 0);
    }

    EckInline int GetItemHeight(int idx) const noexcept
    {
        return (int)SendMessage(LB_GETITEMHEIGHT, idx, 0);
    }

    EckInline BOOL GetItemRect(int idx, _Out_ RECT* prc) const noexcept
    {
        return (SendMessage(LB_GETITEMRECT, idx, (LPARAM)prc) != LB_ERR);
    }

    EckInline int GetItemCountPreColumn() const noexcept
    {
        return (int)SendMessage(LB_GETLISTBOXINFO, 0, 0);
    }

    EckInline LCID GetLocale() const noexcept
    {
        return (LCID)SendMessage(LB_GETLOCALE, 0, 0);
    }

    EckInline int GetSelection(int idx) const noexcept
    {
        return (SendMessage(LB_GETSEL, idx, 0) > 0);
    }

    /// <summary>
    /// 取被选择项目数
    /// </summary>
    /// <returns>数目，若为单选列表框，则返回-1</returns>
    EckInline int GetSelectedCount() const noexcept
    {
        return (int)SendMessage(LB_GETSELCOUNT, 0, 0);
    }

    /// <summary>
    /// 取被选择项目
    /// </summary>
    /// <param name="piSelItems">数组</param>
    /// <param name="c">数组中的元素数</param>
    /// <returns>项目数，若为单选列表，则返回-1</returns>
    EckInline int GetSelectedItems(_Out_writes_(c) int* piSelItems, int c) const noexcept
    {
        return (int)SendMessage(LB_GETSELITEMS, c, (LPARAM)piSelItems);
    }

    EckInline CStringW GetItemText(int idx) const noexcept
    {
        CStringW rs;
        int cch = GetItemTextLength(idx);
        if (cch <= 0)
            return rs;
        rs.ReSize(cch);
        SendMessage(LB_GETTEXT, idx, (LPARAM)rs.Data());
        return rs;
    }

    /// <summary>
    /// 取项目文本
    /// </summary>
    /// <param name="idx"></param>
    /// <param name="pszBuf"></param>
    /// <returns>返回字符数（不含结尾NULL），失败返回-1</returns>
    EckInline int GetItemText(int idx, PWSTR pszBuf) const noexcept
    {
        return (int)SendMessage(LB_GETTEXT, idx, (LPARAM)pszBuf);
    }

    /// <summary>
    /// 取项目文本长度
    /// </summary>
    /// <param name="idx"></param>
    /// <returns>返回字符数（不含结尾NULL）</returns>
    EckInline int GetItemTextLength(int idx) const noexcept
    {
        return (int)SendMessage(LB_GETTEXTLEN, idx, 0);
    }

    EckInline int GetTopIndex() const noexcept
    {
        return (int)SendMessage(LB_GETTOPINDEX, 0, 0);
    }

    /// <summary>
    /// 保留内存
    /// </summary>
    /// <param name="cItems"></param>
    /// <param name="cbString"></param>
    /// <returns>成功返回已预分配的项目总数，失败返回LB_ERRSPACE</returns>
    EckInline int InitialzeStorage(int cItems, SIZE_T cbString) const noexcept
    {
        return (int)SendMessage(LB_INITSTORAGE, cItems, cbString);
    }

    EckInline int InsertString(PCWSTR psz, int idxPos = -1) const noexcept
    {
        return (int)SendMessage(LB_INSERTSTRING, idxPos, (LPARAM)psz);
    }

    EckInline int InsertString(LPARAM lParam, int idxPos = -1) const noexcept
    {
        return (int)SendMessage(LB_INSERTSTRING, idxPos, lParam);
    }

    EckInline int ItemFromPoint(POINT pt, BOOL bAutoScroll = FALSE) const noexcept
    {
        ClientToScreen(m_hWnd, &pt);
        return LBItemFromPt(m_hWnd, pt, bAutoScroll);
    }

    EckInline void ResetContent() const noexcept
    {
        SendMessage(LB_RESETCONTENT, 0, 0);
    }

    /// <summary>
    /// 查找并选择项目。
    /// 不区分大小写
    /// </summary>
    /// <param name="pszText">文本，将匹配以该文本开头的项目</param>
    /// <param name="idxStart">起始索引，-1 = 从头搜索整个列表</param>
    /// <returns>索引，失败返回LB_ERR</returns>
    EckInline int SelectString(PCWSTR pszText, int idxStart = -1) const noexcept
    {
        return (int)SendMessage(LB_SELECTSTRING, idxStart, (LPARAM)pszText);
    }

    EckInline BOOL SelectItemRange(int idxStart, int idxEnd, BOOL bSel) const noexcept
    {
        if (!bSel)
            std::swap(idxStart, idxEnd);
        return (SendMessage(LB_SELITEMRANGEEX, idxStart, idxEnd) != LB_ERR);
    }

    EckInline BOOL SetAnchorIndex(int idx) const noexcept
    {
        return (SendMessage(LB_SETANCHORINDEX, idx, 0) != LB_ERR);
    }

    EckInline BOOL SetCaretIndex(int idx, BOOL bNoFullVisible = FALSE) const noexcept
    {
        return (SendMessage(LB_SETCARETINDEX, idx, bNoFullVisible) != LB_ERR);
    }

    EckInline void SetColumnWidth(int cxColumn) const noexcept
    {
        SendMessage(LB_SETCOLUMNWIDTH, cxColumn, 0);
    }

    EckInline int SetCount(int cItems) const noexcept
    {
        return (int)SendMessage(LB_SETCOUNT, cItems, 0);
    }

    EckInline BOOL SetCurrentSelection(int idxSel = -1) const noexcept
    {
        int iRet = (int)SendMessage(LB_SETCURSEL, idxSel, 0);
        if (idxSel < 0)
            return TRUE;
        else
            return (iRet != LB_ERR);
    }

    EckInline void SetHorizontalExtent(int iHorizontalExtent) const noexcept
    {
        SendMessage(LB_SETHORIZONTALEXTENT, iHorizontalExtent, 0);
    }

    EckInline BOOL SetItemData(int idx, LPARAM lParam) const noexcept
    {
        return (SendMessage(LB_SETITEMDATA, idx, lParam) != LB_ERR);
    }

    /// <summary>
    /// 置项目高度
    /// </summary>
    /// <param name="idx">若列表框具有LBS_OWNERDRAWVARIABLE，则该参数指示项目索引，否则则必须设置为0</param>
    /// <param name="cy"></param>
    /// <returns>成功返回TRUE，失败返回FALSE</returns>
    EckInline BOOL SetItemHeight(int idx, int cy) const noexcept
    {
        return (SendMessage(LB_SETITEMHEIGHT, idx, cy) != LB_ERR);
    }

    /// <summary>
    /// 
    /// </summary>
    /// <param name="lcid"></param>
    /// <returns>成功返回先前的LCID，失败返回LB_ERR</returns>
    EckInline LCID SetLocale(LCID lcid) const noexcept
    {
        return (LCID)SendMessage(LB_SETLOCALE, lcid, 0);
    }

    /// <summary>
    /// 选择项目。
    /// 仅用于多选列表
    /// </summary>
    /// <param name="idx">索引，若设为-1则操作所有项目</param>
    /// <param name="bSel"></param>
    /// <returns></returns>
    EckInline BOOL SetSelection(int idx, BOOL bSel) const noexcept
    {
        return (SendMessage(LB_SETSEL, bSel, idx) != LB_ERR);
    }

    EckInline BOOL SetTabStop(_In_reads_(c) const int* piTabStop, int c) const noexcept
    {
        return (BOOL)SendMessage(LB_SETTABSTOPS, c, (LPARAM)piTabStop);
    }

    EckInline BOOL SetTopIndex(int idx) const noexcept
    {
        return (SendMessage(LB_SETTOPINDEX, idx, 0) != LB_ERR);
    }

    EckInline void RedrawItem(int idx) const noexcept
    {
        RECT rcItem;
        if (GetItemRect(idx, &rcItem) == LB_ERR)
            return;
        InvalidateRect(HWnd, &rcItem, TRUE);
    }
};
ECK_NAMESPACE_END