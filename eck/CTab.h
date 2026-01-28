#pragma once
#include "CWnd.h"

ECK_NAMESPACE_BEGIN
#define ECK_CWNDPROP_TABE_STYLE(Name, Style)			\
	ECKPROP(StyleGet##Name, StyleSet##Name) BOOL Name;	\
	BOOL StyleGet##Name() const noexcept							\
	{													\
		if constexpr (Style == 0)						\
			return !GetTABExtendStyle();				\
		else											\
			return IsBitSet(GetTABExtendStyle(), Style);\
	}													\
	void StyleSet##Name(BOOL b) const noexcept					\
	{													\
		SetTABExtendStyle(b ? Style : 0, Style);		\
	}

class CTab : public CWnd
{
public:
    ECK_RTTI(CTab, CWnd);
    ECK_CWND_NOSINGLEOWNER(CTab);
    ECK_CWND_CREATE_CLS(WC_TABCONTROLW);

    ECK_CWNDPROP_STYLE(ItemBottom, TCS_BOTTOM);
    ECK_CWNDPROP_STYLE(Buttons, TCS_BUTTONS);
    ECK_CWNDPROP_STYLE(FixedWidth, TCS_FIXEDWIDTH);
    ECK_CWNDPROP_STYLE(FlatButtons, TCS_FLATBUTTONS);
    ECK_CWNDPROP_STYLE(FocusNever, TCS_FOCUSNEVER);
    ECK_CWNDPROP_STYLE(FocusOnButtonDown, TCS_FOCUSONBUTTONDOWN);
    ECK_CWNDPROP_STYLE(ForceIconLeft, TCS_FORCEICONLEFT);
    ECK_CWNDPROP_STYLE(ForceLabelLeft, TCS_FORCELABELLEFT);
    ECK_CWNDPROP_STYLE(HotTrack, TCS_HOTTRACK);
    ECK_CWNDPROP_STYLE(MultiLine, TCS_MULTILINE);
    ECK_CWNDPROP_STYLE(MultiSelect, TCS_MULTISELECT);
    ECK_CWNDPROP_STYLE(OwnerDrawFixed, TCS_OWNERDRAWFIXED);
    ECK_CWNDPROP_STYLE(RaggedRight, TCS_RAGGEDRIGHT);
    ECK_CWNDPROP_STYLE(ItemRight, TCS_RIGHT);
    ECK_CWNDPROP_STYLE(RightJustify, TCS_RIGHTJUSTIFY);
    ECK_CWNDPROP_STYLE(ScrollOpposite, TCS_SCROLLOPPOSITE);
    ECK_CWNDPROP_STYLE(SingleLine, TCS_SINGLELINE);
    ECK_CWNDPROP_STYLE(Tabs, TCS_TABS);
    ECK_CWNDPROP_STYLE(ToolTips, TCS_TOOLTIPS);
    ECK_CWNDPROP_STYLE(Vertical, TCS_VERTICAL);
    ECK_CWNDPROP_TABE_STYLE(FlatSeparators, TCS_EX_FLATSEPARATORS);
    ECK_CWNDPROP_TABE_STYLE(RegisterDrop, TCS_EX_REGISTERDROP);

    LRESULT OnMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept override
    {
        switch (uMsg)
        {
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            BeginPaint(hWnd, wParam, ps);
            CEzCDC DC{};
            DC.Create(hWnd, ps.rcPaint.right - ps.rcPaint.left,
                ps.rcPaint.bottom - ps.rcPaint.top);
            SetWindowOrgEx(DC.GetDC(), ps.rcPaint.left, ps.rcPaint.top, nullptr);
            __super::OnMessage(hWnd, WM_PAINT, (WPARAM)DC.GetDC(), lParam);
            BitBltPs(&ps, DC.GetDC());
            EndPaint(hWnd, wParam, ps);
        }
        return 0;
        }
        return __super::OnMessage(hWnd, uMsg, wParam, lParam);
    }

    /// <summary>
    /// 区域矩形转换
    /// </summary>
    /// <param name="prc">RECT指针</param>
    /// <param name="bValidAeraToWndAera">为TRUE则从显示区域转换到窗口区域，为FALSE则相反</param>
    void AdjustRect(RECT* prc, BOOL bValidAeraToWndAera) const noexcept
    {
        SendMsg(TCM_ADJUSTRECT, bValidAeraToWndAera, (LPARAM)prc);
    }

    BOOL DeleteAllItems() const noexcept
    {
        return (BOOL)SendMsg(TCM_DELETEALLITEMS, 0, 0);
    }

    BOOL DeleteItem(int idx) const noexcept
    {
        return (BOOL)SendMsg(TCM_DELETEITEM, idx, 0);
    }

    /// <summary>
    /// 重置所有项目。
    /// 仅设置TCS_BUTTONS时有效
    /// </summary>
    /// <param name="bAllTab">若为TRUE，则重置所有项，若为FALSE，则重置除当前页面之外的所有项</param>
    void DeselectAll(BOOL bAllTab) const noexcept
    {
        SendMsg(TCM_DESELECTALL, bAllTab, 0);
    }

    int GetCurrentFocus() const noexcept
    {
        return (int)SendMsg(TCM_GETCURFOCUS, 0, 0);
    }

    int GetCurrentSelection() const noexcept
    {
        return (int)SendMsg(TCM_GETCURSEL, 0, 0);
    }

    DWORD GetTABExtendStyle() const noexcept
    {
        return (DWORD)SendMsg(TCM_GETEXTENDEDSTYLE, 0, 0);
    }

    HIMAGELIST GetImageList() const noexcept
    {
        return (HIMAGELIST)SendMsg(TCM_GETIMAGELIST, 0, 0);
    }

    BOOL GetItem(int idx, TCITEMW* ptci) const noexcept
    {
        return (BOOL)SendMsg(TCM_GETITEMW, idx, (LPARAM)ptci);
    }

    int GetItemCount() const noexcept
    {
        return (int)SendMsg(TCM_GETITEMCOUNT, 0, 0);
    }

    BOOL GetItemRect(int idx, RECT* prc) const noexcept
    {
        return (BOOL)SendMsg(TCM_GETITEMRECT, idx, (LPARAM)prc);
    }

    int GetRowCount() const noexcept
    {
        return (int)SendMsg(TCM_GETROWCOUNT, 0, 0);
    }

    HWND GetToolTips() const noexcept
    {
        return (HWND)SendMsg(TCM_GETTOOLTIPS, 0, 0);
    }

    BOOL HighlightItem(int idx, BOOL bHighlight) const noexcept
    {
        return (BOOL)SendMsg(TCM_HIGHLIGHTITEM, idx, MAKELPARAM(bHighlight, 0));
    }

    int HitTest(TCHITTESTINFO* ptchti) const noexcept
    {
        return (int)SendMsg(TCM_HITTEST, 0, (LPARAM)ptchti);
    }

    int InsertItem(int idx, TCITEMW* ptci) const noexcept
    {
        return (int)SendMsg(TCM_INSERTITEMW, idx, (LPARAM)ptci);
    }

    int InsertItem(PCWSTR pszText, int idx = -1, int idxImage = -1, LPARAM lParam = 0) const noexcept
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

    void RemoveImage(int idx) const noexcept
    {
        SendMsg(TCM_REMOVEIMAGE, idx, 0);
    }

    void SetCurrentFocus(int idx) const noexcept
    {
        SendMsg(TCM_SETCURFOCUS, idx, 0);
    }

    int SetCurrentSelection(int idx) const noexcept
    {
        return (int)SendMsg(TCM_SETCURSEL, idx, 0);
    }

    DWORD SetTABExtendStyle(DWORD dwNew, DWORD dwMask) const noexcept
    {
        return (DWORD)SendMsg(TCM_SETEXTENDEDSTYLE, dwMask, dwNew);
    }

    HIMAGELIST SetImageList(HIMAGELIST hImageList) const noexcept
    {
        return (HIMAGELIST)SendMsg(TCM_SETIMAGELIST, 0, (LPARAM)hImageList);
    }

    BOOL SetItem(int idx, TCITEMW* ptci) const noexcept
    {
        return (BOOL)SendMsg(TCM_SETITEMW, idx, (LPARAM)ptci);
    }

    BOOL SetItemExtra(SIZE_T cbExtra) const noexcept
    {
        return (BOOL)SendMsg(TCM_SETITEMEXTRA, cbExtra, 0);
    }

    void SetItemSize(int cx, int cy, int* piOldWidth = nullptr, int* piOldHeight = nullptr) const noexcept
    {
        const auto uRet = (UINT)SendMsg(TCM_SETITEMSIZE, 0, MAKELPARAM(cx, cy));
        if (piOldWidth)
            *piOldWidth = LOWORD(uRet);
        if (piOldHeight)
            *piOldHeight = HIWORD(uRet);
    }

    int SetMinimumTabWidth(int cxMin = -1) const noexcept
    {
        return (int)SendMsg(TCM_SETMINTABWIDTH, 0, cxMin);
    }

    void SetPadding(int cx, int cy) const noexcept
    {
        SendMsg(TCM_SETITEMSIZE, 0, MAKELPARAM(cx, cy));
    }

    void SetToolTips(HWND hToolTip) const noexcept
    {
        SendMsg(TCM_SETTOOLTIPS, (WPARAM)hToolTip, 0);
    }
};
ECK_NAMESPACE_END