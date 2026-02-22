#pragma once
#include "CWindow.h"

ECK_NAMESPACE_BEGIN
#define ECK_CWNDPROP_TLBE_STYLE(Name, Style)			\
	ECKPROP(StyleGet##Name, StyleSet##Name) BOOL Name;	\
	BOOL StyleGet##Name() const noexcept							\
	{													\
		if constexpr (Style == 0)						\
			return !GetTBExtendStyle();					\
		else											\
			return IsBitSet(GetTBExtendStyle(), Style);	\
	}													\
	void StyleSet##Name(BOOL b) const noexcept					\
	{													\
		if (b)											\
			SetTBExtendStyle(GetTBExtendStyle() | Style);	\
		else											\
			SetTBExtendStyle(GetTBExtendStyle() & ~Style);	\
	}

constexpr inline DWORD ToolBarPrettyStyle = TBSTYLE_LIST | TBSTYLE_TRANSPARENT |
CCS_NOPARENTALIGN | CCS_NORESIZE | CCS_NODIVIDER;

class CToolBar : public CWindow
{
public:
    ECK_RTTI(CToolBar, CWindow);
    ECK_CWND_NOSINGLEOWNER(CToolBar);
    ECK_CWND_CREATE_CLS(TOOLBARCLASSNAMEW);

    ECK_CWNDPROP_STYLE(AltDrag, TBSTYLE_ALTDRAG);
    ECK_CWNDPROP_STYLE(CustomErase, TBSTYLE_CUSTOMERASE);
    ECK_CWNDPROP_STYLE(Flat, TBSTYLE_FLAT);
    ECK_CWNDPROP_STYLE(List, TBSTYLE_LIST);
    ECK_CWNDPROP_STYLE(RegisterDrop, TBSTYLE_REGISTERDROP);
    ECK_CWNDPROP_STYLE(ToolTips, TBSTYLE_TOOLTIPS);
    ECK_CWNDPROP_STYLE(Transparent, TBSTYLE_TRANSPARENT);
    ECK_CWNDPROP_STYLE(Wrapable, TBSTYLE_WRAPABLE);
    ECK_CWNDPROP_TLBE_STYLE(DrawDropDownArrows, TBSTYLE_EX_DRAWDDARROWS);
    ECK_CWNDPROP_TLBE_STYLE(HideClippedButtons, TBSTYLE_EX_HIDECLIPPEDBUTTONS);
    ECK_CWNDPROP_TLBE_STYLE(DoubleBuffer, TBSTYLE_EX_DOUBLEBUFFER);
    ECK_CWNDPROP_TLBE_STYLE(MixedButtons, TBSTYLE_EX_MIXEDBUTTONS);

    EckInline int AddBitmap(int cBitmaps, HINSTANCE hInstance, UINT_PTR nID) const noexcept
    {
        TBADDBITMAP tbab{ hInstance,nID };
        return (int)SendMessage(TB_ADDBITMAP, cBitmaps, (LPARAM)&tbab);
    }

    EckInline BOOL AddButtons(int cButtons, TBBUTTON* ptbb) const noexcept
    {
        return (int)SendMessage(TB_ADDBUTTONSW, cButtons, (LPARAM)ptbb);
    }

    EckInline int AddString(HINSTANCE hInstance, PCWSTR pszIDOrText) const noexcept
    {
        return (int)SendMessage(TB_ADDSTRINGW, (WPARAM)hInstance, (LPARAM)pszIDOrText);
    }

    EckInline void AutoSize() const noexcept
    {
        SendMessage(TB_AUTOSIZE, 0, 0);
    }

    EckInline int GetButtonCount() const noexcept
    {
        return (int)SendMessage(TB_BUTTONCOUNT, 0, 0);
    }

    EckInline void SetButtonStructSize(int cbStruct = sizeof(TBBUTTON)) const noexcept
    {
        SendMessage(TB_BUTTONSTRUCTSIZE, cbStruct, 0);
    }

    EckInline BOOL ChangeBitmap(int idCommand, int idxImg) const noexcept
    {
        return (BOOL)SendMessage(TB_CHANGEBITMAP, idCommand, idxImg);
    }

    EckInline BOOL CheckButton(int idCommand, BOOL bChecked) const noexcept
    {
        return (BOOL)SendMessage(TB_CHECKBUTTON, idCommand, bChecked);
    }

    EckInline int CommandToIndex(int idCommand) const noexcept
    {
        return (int)SendMessage(TB_COMMANDTOINDEX, idCommand, 0);
    }

    EckInline void Customize() const noexcept
    {
        SendMessage(TB_CUSTOMIZE, 0, 0);
    }

    EckInline BOOL DeleteButton(int idxButton) const noexcept
    {
        return (BOOL)SendMessage(TB_DELETEBUTTON, idxButton, 0);
    }

    EckInline BOOL EnableButton(int idCommand, BOOL bEnabled) const noexcept
    {
        return (BOOL)SendMessage(TB_ENABLEBUTTON, idCommand, bEnabled);
    }

    EckInline BOOL GetAnchorHighlight() const noexcept
    {
        return (BOOL)SendMessage(TB_GETANCHORHIGHLIGHT, 0, 0);
    }

    EckInline int GetBitmap(int idCommand) const noexcept
    {
        return (int)SendMessage(TB_GETBITMAP, idCommand, 0);
    }

    EckInline UINT GetBitmapFlags() const noexcept
    {
        return (UINT)SendMessage(TB_GETBITMAPFLAGS, 0, 0);
    }

    EckInline BOOL GetButton(int idxButton, TBBUTTON* ptbb) const noexcept
    {
        return (BOOL)SendMessage(TB_GETBUTTON, idxButton, (LPARAM)ptbb);
    }

    EckInline int GetButtonInfomation(int idCommand, TBBUTTONINFOW* ptbbi) const noexcept
    {
        return (int)SendMessage(TB_GETBUTTONINFOW, idCommand, (LPARAM)ptbbi);
    }

    EckInline UINT GetButtonSize() const noexcept
    {
        return (UINT)SendMessage(TB_GETBUTTONSIZE, 0, 0);
    }

    EckInline void GetButtonSize(int* pcx, int* pcy) const noexcept
    {
        const auto uRet = (UINT)SendMessage(TB_GETBUTTONSIZE, 0, 0);
        if (pcx)
            *pcx = LOWORD(uRet);
        if (pcy)
            *pcy = HIWORD(uRet);
    }

    EckInline int GetButtonText(int idCommand, PWSTR pszBuf) const noexcept
    {
        return (int)SendMessage(TB_GETBUTTONTEXTW, idCommand, (LPARAM)pszBuf);
    }

    EckInline BOOL GetColorScheme(COLORSCHEME* pcs) const noexcept
    {
        return (BOOL)SendMessage(TB_GETCOLORSCHEME, 0, (LPARAM)pcs);
    }

    EckInline HIMAGELIST GetDisabledImageList() const noexcept
    {
        return (HIMAGELIST)SendMessage(TB_GETDISABLEDIMAGELIST, 0, 0);
    }

    EckInline DWORD GetTBExtendStyle() const noexcept
    {
        return (DWORD)SendMessage(TB_GETEXTENDEDSTYLE, 0, 0);
    }

    EckInline HIMAGELIST GetHotImageList() const noexcept
    {
        return (HIMAGELIST)SendMessage(TB_GETHOTIMAGELIST, 0, 0);
    }

    EckInline int GetHotItem() const noexcept
    {
        return (int)SendMessage(TB_GETHOTITEM, 0, 0);
    }

    EckInline BOOL GetIdealSize(BOOL bHeight, SIZE* psize) const noexcept
    {
        return (BOOL)SendMessage(TB_GETIDEALSIZE, bHeight, (LPARAM)psize);
    }

    EckInline BOOL GetIdealSize(SIZE* psize) const noexcept
    {
        SIZE size;
        BOOL b = (BOOL)SendMessage(TB_GETIDEALSIZE, TRUE, (LPARAM)&size);
        psize->cy = size.cy;
        b = b && (BOOL)SendMessage(TB_GETIDEALSIZE, FALSE, (LPARAM)&size);
        psize->cx = size.cx;
        return b;
    }

    EckInline HIMAGELIST GetImageList() const noexcept
    {
        return (HIMAGELIST)SendMessage(TB_GETIMAGELIST, 0, 0);
    }

    EckInline int GetImageListCount() const noexcept
    {
        return (int)SendMessage(TB_GETIMAGELISTCOUNT, 0, 0);
    }

    EckInline void GetInsertMark(TBINSERTMARK* ptbim) const noexcept
    {
        SendMessage(TB_GETINSERTMARK, 0, (LPARAM)ptbim);
    }

    EckInline COLORREF GetInsertMarkColor() const noexcept
    {
        return (COLORREF)SendMessage(TB_GETINSERTMARKCOLOR, 0, 0);
    }

    EckInline void GetItemDropDownRect(int idxButton, RECT* prc) const noexcept
    {
        SendMessage(TB_GETITEMDROPDOWNRECT, idxButton, (LPARAM)prc);
    }

    EckInline BOOL GetItemRect(int idxButton, RECT* prc) const noexcept
    {
        return (BOOL)SendMessage(TB_GETITEMRECT, idxButton, (LPARAM)prc);
    }

    EckInline BOOL GetMaximumSize(SIZE* psize) const noexcept
    {
        return (BOOL)SendMessage(TB_GETMAXSIZE, 0, (LPARAM)psize);
    }

    EckInline void GetMetrics(TBMETRICS* ptbm) const noexcept
    {
        SendMessage(TB_GETMETRICS, 0, (LPARAM)ptbm);
    }

    EckInline HRESULT GetDropTargetObject(REFIID riid, IDropTarget** ppDropTarget) const noexcept
    {
        return (HRESULT)SendMessage(TB_GETOBJECT, (WPARAM)&riid, (LPARAM)ppDropTarget);
    }

    EckInline UINT GetPadding() const noexcept
    {
        return (UINT)SendMessage(TB_GETPADDING, 0, 0);
    }

    EckInline void GetPadding(int* pxPadding, int* pcPadding) const noexcept
    {
        const auto uRet = (UINT)SendMessage(TB_GETPADDING, 0, 0);
        if (pxPadding)
            *pxPadding = LOWORD(uRet);
        if (pcPadding)
            *pcPadding = HIWORD(uRet);
    }

    EckInline HIMAGELIST GetPressedImageList() const noexcept
    {
        return (HIMAGELIST)SendMessage(TB_GETPRESSEDIMAGELIST, 0, 0);
    }

    EckInline BOOL GetRect(int idCommand, RECT* prc) const noexcept
    {
        return (BOOL)SendMessage(TB_GETRECT, idCommand, (LPARAM)prc);
    }

    EckInline int GetRows() const noexcept
    {
        return (int)SendMessage(TB_GETROWS, 0, 0);
    }

    EckInline UINT GetState(int idxButton) const noexcept
    {
        return (UINT)SendMessage(TB_GETSTATE, idxButton, 0);
    }

    EckInline int GetString(int idxString, int cbBuf, PWSTR pszBuf) const noexcept
    {
        return (int)SendMessage(TB_GETSTRINGW, MAKEWPARAM(cbBuf, idxString), (LPARAM)pszBuf);
    }

    EckInline DWORD GetStyleTB() const noexcept
    {
        return (DWORD)SendMessage(TB_GETSTYLE, 0, 0);
    }

    EckInline int GetTextRows() const noexcept
    {
        return (int)SendMessage(TB_GETTEXTROWS, 0, 0);
    }

    EckInline HWND GetToolTips() const noexcept
    {
        return (HWND)SendMessage(TB_GETTOOLTIPS, 0, 0);
    }

    EckInline BOOL HideButton(int idCommand, BOOL bHide) const noexcept
    {
        return (BOOL)SendMessage(TB_HIDEBUTTON, idCommand, MAKELPARAM(bHide, 0));
    }

    EckInline int HitTest(POINT* ppt) const noexcept
    {
        return (int)SendMessage(TB_HITTEST, 0, (LPARAM)ppt);
    }

    EckInline BOOL Indeterminate(int idCommand, BOOL bIndeterminate) const noexcept
    {
        return (BOOL)SendMessage(TB_INDETERMINATE, idCommand, MAKELPARAM(bIndeterminate, 0));
    }

    EckInline BOOL InsertButton(int pos, TBBUTTON* ptbb) const noexcept
    {
        return (BOOL)SendMessage(TB_INSERTBUTTONW, pos, (LPARAM)ptbb);
    }

    EckInline BOOL InsertMarkHitTest(POINT* ppt, TBINSERTMARK* ptbim) const noexcept
    {
        return (BOOL)SendMessage(TB_INSERTMARKHITTEST, (WPARAM)ppt, (LPARAM)ptbim);
    }

    EckInline BOOL IsButtonChecked(int idCommand) const noexcept
    {
        return (BOOL)SendMessage(TB_ISBUTTONCHECKED, idCommand, 0);
    }

    EckInline BOOL IsButtonEnabled(int idCommand) const noexcept
    {
        return (BOOL)SendMessage(TB_ISBUTTONENABLED, idCommand, 0);
    }

    EckInline BOOL IsButtonHidden(int idCommand) const noexcept
    {
        return (BOOL)SendMessage(TB_ISBUTTONHIDDEN, idCommand, 0);
    }

    EckInline BOOL IsButtonHighlighted(int idCommand) const noexcept
    {
        return (BOOL)SendMessage(TB_ISBUTTONHIGHLIGHTED, idCommand, 0);
    }

    EckInline BOOL IsButtonIndeterminate(int idCommand) const noexcept
    {
        return (BOOL)SendMessage(TB_ISBUTTONINDETERMINATE, idCommand, 0);
    }

    EckInline BOOL IsButtonPressed(int idCommand) const noexcept
    {
        return (BOOL)SendMessage(TB_ISBUTTONPRESSED, idCommand, 0);
    }

    EckInline int LoadImages(UINT_PTR nID) const noexcept
    {
        return (int)SendMessage(TB_LOADIMAGES, nID, (LPARAM)HINST_COMMCTRL);
    }

    EckInline BOOL MapAccelerator(WCHAR chAccel, UINT* pidCommand) const noexcept
    {
        return (BOOL)SendMessage(TB_MAPACCELERATORW, chAccel, (LPARAM)pidCommand);
    }

    EckInline BOOL MarkButton(int idCommand, BOOL bHighlighted) const noexcept
    {
        return (BOOL)SendMessage(TB_MARKBUTTON, idCommand, MAKELPARAM(bHighlighted, 0));
    }

    EckInline BOOL MoveButton(int idx, int idxDest) const noexcept
    {
        return (BOOL)SendMessage(TB_MOVEBUTTON, idx, idxDest);
    }

    EckInline BOOL PressButton(int idCommand, BOOL bPressed) const noexcept
    {
        return (BOOL)SendMessage(TB_PRESSBUTTON, idCommand, MAKELPARAM(bPressed, 0));
    }

    EckInline BOOL ReplaceBitmap(TBREPLACEBITMAP* ptbrb) const noexcept
    {
        return (BOOL)SendMessage(TB_REPLACEBITMAP, 0, (LPARAM)ptbrb);
    }

    EckInline void SaveRestore(TBSAVEPARAMSW* ptbsp) const noexcept
    {
        SendMessage(TB_SAVERESTOREW, 0, (LPARAM)ptbsp);
    }

    EckInline BOOL SetAnchorHighlighted(BOOL bHighlighted) const noexcept
    {
        return (BOOL)SendMessage(TB_SETANCHORHIGHLIGHT, bHighlighted, 0);
    }

    EckInline BOOL SetBitmapSize(int cx, int cy) const noexcept
    {
        return (BOOL)SendMessage(TB_SETBITMAPSIZE, 0, MAKELPARAM(cx, cy));
    }

    EckInline BOOL SetButtonInfomation(int idCommand, TBBUTTONINFOW* ptbbi) const noexcept
    {
        return (BOOL)SendMessage(TB_SETBUTTONINFOW, idCommand, (LPARAM)ptbbi);
    }

    EckInline BOOL SetButtonSize(int cx, int cy) const noexcept
    {
        return (BOOL)SendMessage(TB_SETBUTTONSIZE, 0, MAKELPARAM(cx, cy));
    }

    EckInline BOOL SetButtonWidth(int cxMin, int cxMax) const noexcept
    {
        return (BOOL)SendMessage(TB_SETBUTTONWIDTH, 0, MAKELPARAM(cxMin, cxMax));
    }

    EckInline BOOL SetCommandId(int idxButton, int idNew) const noexcept
    {
        return (BOOL)SendMessage(TB_SETCMDID, idxButton, idNew);
    }

    EckInline void SetColorScheme(COLORSCHEME* pcs) const noexcept
    {
        SendMessage(TB_SETCOLORSCHEME, 0, (LPARAM)pcs);
    }

    EckInline HIMAGELIST SetDisabledImageList(HIMAGELIST hImageList) const noexcept
    {
        return (HIMAGELIST)SendMessage(TB_SETDISABLEDIMAGELIST, 0, (LPARAM)hImageList);
    }

    EckInline UINT SetDrawTextFlags(UINT uMask, UINT uDTFlags) const noexcept
    {
        return (UINT)SendMessage(TB_SETDRAWTEXTFLAGS, uMask, uDTFlags);
    }

    EckInline DWORD SetTBExtendStyle(DWORD dwExStyle) const noexcept
    {
        return (DWORD)SendMessage(TB_SETEXTENDEDSTYLE, 0, dwExStyle);
    }

    EckInline HIMAGELIST SetHotImageList(HIMAGELIST hImageList) const noexcept
    {
        return (HIMAGELIST)SendMessage(TB_SETHOTIMAGELIST, 0, (LPARAM)hImageList);
    }

    EckInline int SetHotItem(int idxHot) const noexcept
    {
        return (int)SendMessage(TB_SETHOTITEM, idxHot, 0);
    }

    EckInline int SetHotItem(int idxHot, UINT uFlags) const noexcept
    {
        return (int)SendMessage(TB_SETHOTITEM2, idxHot, uFlags);
    }

    EckInline HIMAGELIST SetImageList(HIMAGELIST hImageList, int idxImageList = 0) const noexcept
    {
        return (HIMAGELIST)SendMessage(TB_SETIMAGELIST, idxImageList, (LPARAM)hImageList);
    }

    EckInline BOOL SetIndent(int iIndent) const noexcept
    {
        return (BOOL)SendMessage(TB_SETINDENT, iIndent, 0);
    }

    EckInline void SetInsertMark(TBINSERTMARK* ptbim) const noexcept
    {
        SendMessage(TB_SETINSERTMARK, 0, (LPARAM)ptbim);
    }

    EckInline COLORREF SetInsertMarkColor(COLORREF cr) const noexcept
    {
        return (COLORREF)SendMessage(TB_SETINSERTMARKCOLOR, 0, cr);
    }

    EckInline void SetListGap(int iGap) const noexcept
    {
        SendMessage(TB_SETLISTGAP, iGap, 0);
    }

    EckInline BOOL SetMaximumRows(int cRows) const noexcept
    {
        return (BOOL)SendMessage(TB_SETMAXTEXTROWS, cRows, 0);
    }

    EckInline void SetMetrics(TBMETRICS* ptbm) const noexcept
    {
        SendMessage(TB_SETMETRICS, 0, (LPARAM)ptbm);
    }

    EckInline UINT SetPadding(int xPadding, int yPadding) const noexcept
    {
        return (UINT)SendMessage(TB_SETPADDING, 0, MAKELPARAM(xPadding, yPadding));
    }

    EckInline HWND SetParent(HWND hWndNotify) const noexcept
    {
        return (HWND)SendMessage(TB_SETPARENT, (WPARAM)hWndNotify, 0);
    }

    EckInline HIMAGELIST SetPressedImageList(HIMAGELIST hImageList, int idxImageList = 0) const noexcept
    {
        return (HIMAGELIST)SendMessage(TB_SETPRESSEDIMAGELIST, idxImageList, (LPARAM)hImageList);
    }

    EckInline void SetRows(int cRows, BOOL bAllowMoreRows, RECT* prc) const noexcept
    {
        SendMessage(TB_SETROWS, MAKEWPARAM(cRows, bAllowMoreRows), (LPARAM)prc);
    }

    EckInline UINT SetState(int idCommand, UINT uState) const noexcept
    {
        return (UINT)SendMessage(TB_SETSTATE, idCommand, uState);
    }

    EckInline void SetStyleTB(DWORD dwStyle) const noexcept
    {
        SendMessage(TB_SETSTYLE, 0, dwStyle);
    }

    EckInline void SetToolTips(HWND hToolTip) const noexcept
    {
        SendMessage(TB_SETTOOLTIPS, (WPARAM)hToolTip, 0);
    }
};
ECK_NAMESPACE_END