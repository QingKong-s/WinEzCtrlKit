/*
* WinEzCtrlKit Library
*
* CToolBar.h ： 标准工具条
*
* Copyright(C) 2023-2024 QingKong
*/
#pragma once
#include "CWnd.h"

ECK_NAMESPACE_BEGIN
#define ECK_CWNDPROP_TLBE_STYLE(Name, Style)			\
	ECKPROP(StyleGet##Name, StyleSet##Name) BOOL Name;	\
	BOOL StyleGet##Name() const							\
	{													\
		if constexpr (Style == 0)						\
			return !GetTBExtendStyle();					\
		else											\
			return IsBitSet(GetTBExtendStyle(), Style);	\
	}													\
	void StyleSet##Name(BOOL b) const					\
	{													\
		if (b)											\
			SetTBExtendStyle(GetTBExtendStyle() | Style);	\
		else											\
			SetTBExtendStyle(GetTBExtendStyle() & ~Style);	\
	}

class CToolBar :public CWnd
{
public:
	ECK_RTTI(CToolBar);
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

	EckInline int AddBitmap(int cBitmaps, HINSTANCE hInstance, UINT_PTR nID) const
	{
		TBADDBITMAP tbab{ hInstance,nID };
		return (int)SendMsg(TB_ADDBITMAP, cBitmaps, (LPARAM)&tbab);
	}

	EckInline BOOL AddButtons(int cButtons, TBBUTTON* ptbb) const
	{
		return (int)SendMsg(TB_ADDBUTTONSW, cButtons, (LPARAM)ptbb);
	}

	EckInline int AddString(HINSTANCE hInstance, PCWSTR pszIDOrText) const
	{
		return (int)SendMsg(TB_ADDSTRINGW, (WPARAM)hInstance, (LPARAM)pszIDOrText);
	}

	EckInline void AutoSize() const
	{
		SendMsg(TB_AUTOSIZE, 0, 0);
	}

	EckInline int GetButtonCount() const
	{
		return (int)SendMsg(TB_BUTTONCOUNT, 0, 0);
	}

	EckInline void SetButtonStructSize(int cbStruct = sizeof(TBBUTTON)) const
	{
		SendMsg(TB_BUTTONSTRUCTSIZE, cbStruct, 0);
	}

	EckInline BOOL ChangeBitmap(int idCommand, int idxImg) const
	{
		return (BOOL)SendMsg(TB_CHANGEBITMAP, idCommand, idxImg);
	}

	EckInline BOOL CheckButton(int idCommand, BOOL bChecked) const
	{
		return (BOOL)SendMsg(TB_CHECKBUTTON, idCommand, bChecked);
	}

	EckInline int CommandToIndex(int idCommand) const
	{
		return (int)SendMsg(TB_COMMANDTOINDEX, idCommand, 0);
	}

	EckInline void Customize() const
	{
		SendMsg(TB_CUSTOMIZE, 0, 0);
	}

	EckInline BOOL DeleteButton(int idxButton) const
	{
		return (BOOL)SendMsg(TB_DELETEBUTTON, idxButton, 0);
	}

	EckInline BOOL EnableButton(int idCommand, BOOL bEnabled) const
	{
		return (BOOL)SendMsg(TB_ENABLEBUTTON, idCommand, bEnabled);
	}

	EckInline BOOL GetAnchorHighlight() const
	{
		return (BOOL)SendMsg(TB_GETANCHORHIGHLIGHT, 0, 0);
	}

	EckInline int GetBitmap(int idCommand) const
	{
		return (int)SendMsg(TB_GETBITMAP, idCommand, 0);
	}

	EckInline DWORD GetBitmapFlags() const
	{
		return (DWORD)SendMsg(TB_GETBITMAPFLAGS, 0, 0);
	}

	EckInline BOOL GetButton(int idxButton, TBBUTTON* ptbb) const
	{
		return (BOOL)SendMsg(TB_GETBUTTON, idxButton, (LPARAM)ptbb);
	}

	EckInline int GetButtonInfo(int idCommand, TBBUTTONINFOW* ptbbi) const
	{
		return (int)SendMsg(TB_GETBUTTONINFOW, idCommand, (LPARAM)ptbbi);
	}

	EckInline DWORD GetButtonSize() const
	{
		return (DWORD)SendMsg(TB_GETBUTTONSIZE, 0, 0);
	}

	EckInline void GetButtonSize(int* pcx, int* pcy) const
	{
		DWORD dwRet = (DWORD)SendMsg(TB_GETBUTTONSIZE, 0, 0);
		if (pcx)
			*pcx = LOWORD(dwRet);
		if (pcy)
			*pcy = HIWORD(dwRet);
	}

	EckInline int GetButtonText(int idCommand, PWSTR pszBuf) const
	{
		return (int)SendMsg(TB_GETBUTTONTEXTW, idCommand, (LPARAM)pszBuf);
	}

	EckInline BOOL GetColorScheme(COLORSCHEME* pcs) const
	{
		return (BOOL)SendMsg(TB_GETCOLORSCHEME, 0, (LPARAM)pcs);
	}

	EckInline HIMAGELIST GetDisabledImageList() const
	{
		return (HIMAGELIST)SendMsg(TB_GETDISABLEDIMAGELIST, 0, 0);
	}

	EckInline DWORD GetTBExtendStyle() const
	{
		return (DWORD)SendMsg(TB_GETEXTENDEDSTYLE, 0, 0);
	}

	EckInline HIMAGELIST GetHotImageList() const
	{
		return (HIMAGELIST)SendMsg(TB_GETHOTIMAGELIST, 0, 0);
	}

	EckInline int GetHotItem() const
	{
		return (int)SendMsg(TB_GETHOTITEM, 0, 0);
	}

	EckInline BOOL GetIdealSize(BOOL bHeight, SIZE* psize) const
	{
		return (BOOL)SendMsg(TB_GETIDEALSIZE, bHeight, (LPARAM)psize);
	}

	EckInline BOOL GetIdealSize(SIZE* psize) const
	{
		SIZE size;
		BOOL b = (BOOL)SendMsg(TB_GETIDEALSIZE, TRUE, (LPARAM)&size);
		psize->cy = size.cy;
		b = b && (BOOL)SendMsg(TB_GETIDEALSIZE, FALSE, (LPARAM)&size);
		psize->cx = size.cx;
		return b;
	}

	EckInline HIMAGELIST GetImageList() const
	{
		return (HIMAGELIST)SendMsg(TB_GETIMAGELIST, 0, 0);
	}

	EckInline int GetImageListCount() const
	{
		return (int)SendMsg(TB_GETIMAGELISTCOUNT, 0, 0);
	}

	EckInline void GetInsertMark(TBINSERTMARK* ptbim) const
	{
		SendMsg(TB_GETINSERTMARK, 0, (LPARAM)ptbim);
	}

	EckInline COLORREF GetInsertMarkColor() const
	{
		return (COLORREF)SendMsg(TB_GETINSERTMARKCOLOR, 0, 0);
	}

	EckInline void GetItemDropDownRect(int idxButton, RECT* prc) const
	{
		SendMsg(TB_GETITEMDROPDOWNRECT, idxButton, (LPARAM)prc);
	}

	EckInline BOOL GetItemRect(int idxButton, RECT* prc) const
	{
		return (BOOL)SendMsg(TB_GETITEMRECT, idxButton, (LPARAM)prc);
	}

	EckInline BOOL GetMaxSize(SIZE* psize) const
	{
		return (BOOL)SendMsg(TB_GETMAXSIZE, 0, (LPARAM)psize);
	}

	EckInline void GetMetrics(TBMETRICS* ptbm) const
	{
		SendMsg(TB_GETMETRICS, 0, (LPARAM)ptbm);
	}

	EckInline HRESULT GetDropTargetObject(REFIID riid, IDropTarget** ppDropTarget) const
	{
		return (HRESULT)SendMsg(TB_GETOBJECT, (WPARAM)&riid, (LPARAM)ppDropTarget);
	}

	EckInline DWORD GetPadding() const
	{
		return (DWORD)SendMsg(TB_GETPADDING, 0, 0);
	}

	EckInline void GetPadding(int* pxPadding, int* pcPadding) const
	{
		DWORD dwRet = (DWORD)SendMsg(TB_GETPADDING, 0, 0);
		if (pxPadding)
			*pxPadding = LOWORD(dwRet);
		if (pcPadding)
			*pcPadding = HIWORD(dwRet);
	}

	EckInline HIMAGELIST GetPressedImageList() const
	{
		return (HIMAGELIST)SendMsg(TB_GETPRESSEDIMAGELIST, 0, 0);
	}

	EckInline BOOL GetRect(int idCommand, RECT* prc) const
	{
		return (BOOL)SendMsg(TB_GETRECT, idCommand, (LPARAM)prc);
	}

	EckInline int GetRows() const
	{
		return (int)SendMsg(TB_GETROWS, 0, 0);
	}

	EckInline UINT GetState(int idxButton) const
	{
		return (UINT)SendMsg(TB_GETSTATE, idxButton, 0);
	}

	EckInline int GetString(int idxString, int cbBuf, PWSTR pszBuf) const
	{
		return (int)SendMsg(TB_GETSTRINGW, MAKEWPARAM(cbBuf, idxString), (LPARAM)pszBuf);
	}

	EckInline DWORD GetStyleTB() const
	{
		return (DWORD)SendMsg(TB_GETSTYLE, 0, 0);
	}

	EckInline int GetTextRows() const
	{
		return (int)SendMsg(TB_GETTEXTROWS, 0, 0);
	}

	EckInline HWND GetToolTips() const
	{
		return (HWND)SendMsg(TB_GETTOOLTIPS, 0, 0);
	}

	EckInline BOOL HideButton(int idCommand, BOOL bHide) const
	{
		return (BOOL)SendMsg(TB_HIDEBUTTON, idCommand, MAKELPARAM(bHide, 0));
	}

	EckInline int HitTest(POINT* ppt) const
	{
		return (int)SendMsg(TB_HITTEST, 0, (LPARAM)ppt);
	}

	EckInline BOOL Indeterminate(int idCommand, BOOL bIndeterminate) const
	{
		return (BOOL)SendMsg(TB_INDETERMINATE, idCommand, MAKELPARAM(bIndeterminate, 0));
	}

	EckInline BOOL InsertButton(int pos, TBBUTTON* ptbb) const
	{
		return (BOOL)SendMsg(TB_INSERTBUTTONW, pos, (LPARAM)ptbb);
	}

	EckInline BOOL InsertMarkHitTest(POINT* ppt, TBINSERTMARK* ptbim) const
	{
		return (BOOL)SendMsg(TB_INSERTMARKHITTEST, (WPARAM)ppt, (LPARAM)ptbim);
	}

	EckInline BOOL IsButtonChecked(int idCommand) const
	{
		return (BOOL)SendMsg(TB_ISBUTTONCHECKED, idCommand, 0);
	}

	EckInline BOOL IsButtonEnabled(int idCommand) const
	{
		return (BOOL)SendMsg(TB_ISBUTTONENABLED, idCommand, 0);
	}

	EckInline BOOL IsButtonHidden(int idCommand) const
	{
		return (BOOL)SendMsg(TB_ISBUTTONHIDDEN, idCommand, 0);
	}

	EckInline BOOL IsButtonHighlighted(int idCommand) const
	{
		return (BOOL)SendMsg(TB_ISBUTTONHIGHLIGHTED, idCommand, 0);
	}

	EckInline BOOL IsButtonIndeterminate(int idCommand) const
	{
		return (BOOL)SendMsg(TB_ISBUTTONINDETERMINATE, idCommand, 0);
	}

	EckInline BOOL IsButtonPressed(int idCommand) const
	{
		return (BOOL)SendMsg(TB_ISBUTTONPRESSED, idCommand, 0);
	}

	EckInline int LoadImages(UINT_PTR nID) const
	{
		return (int)SendMsg(TB_LOADIMAGES, nID, (LPARAM)HINST_COMMCTRL);
	}

	EckInline BOOL MapAccelerator(WCHAR chAccel, UINT* pidCommand) const
	{
		return (BOOL)SendMsg(TB_MAPACCELERATORW, chAccel, (LPARAM)pidCommand);
	}

	EckInline BOOL MarkButton(int idCommand, BOOL bHighlighted) const
	{
		return (BOOL)SendMsg(TB_MARKBUTTON, idCommand, MAKELPARAM(bHighlighted, 0));
	}

	EckInline BOOL MoveButton(int idx, int idxDest) const
	{
		return (BOOL)SendMsg(TB_MOVEBUTTON, idx, idxDest);
	}

	EckInline BOOL PressButton(int idCommand, BOOL bPressed) const
	{
		return (BOOL)SendMsg(TB_PRESSBUTTON, idCommand, MAKELPARAM(bPressed, 0));
	}

	EckInline BOOL ReplaceBitmap(TBREPLACEBITMAP* ptbrb) const
	{
		return (BOOL)SendMsg(TB_REPLACEBITMAP, 0, (LPARAM)ptbrb);
	}

	EckInline void SaveRestore(TBSAVEPARAMSW* ptbsp) const
	{
		SendMsg(TB_SAVERESTOREW, 0, (LPARAM)ptbsp);
	}

	EckInline BOOL SetAnchorHighlighted(BOOL bHighlighted) const
	{
		return (BOOL)SendMsg(TB_SETANCHORHIGHLIGHT, bHighlighted, 0);
	}

	EckInline BOOL SetBitmapSize(int cx, int cy) const
	{
		return (BOOL)SendMsg(TB_SETBITMAPSIZE, 0, MAKELPARAM(cx, cy));
	}

	EckInline BOOL SetButtonInfo(int idCommand, TBBUTTONINFOW* ptbbi) const
	{
		return (BOOL)SendMsg(TB_SETBUTTONINFOW, idCommand, (LPARAM)ptbbi);
	}

	EckInline BOOL SetButtonSize(int cx, int cy) const
	{
		return (BOOL)SendMsg(TB_SETBUTTONSIZE, 0, MAKELPARAM(cx, cy));
	}

	EckInline BOOL SetButtonWidth(int cxMin, int cxMax) const
	{
		return (BOOL)SendMsg(TB_SETBUTTONWIDTH, 0, MAKELPARAM(cxMin, cxMax));
	}

	EckInline BOOL SetCommandID(int idxButton, int idNew) const
	{
		return (BOOL)SendMsg(TB_SETCMDID, idxButton, idNew);
	}

	EckInline void SetColorScheme(COLORSCHEME* pcs) const
	{
		SendMsg(TB_SETCOLORSCHEME, 0, (LPARAM)pcs);
	}

	EckInline HIMAGELIST SetDisabledImageList(HIMAGELIST hImageList) const
	{
		return (HIMAGELIST)SendMsg(TB_SETDISABLEDIMAGELIST, 0, (LPARAM)hImageList);
	}

	EckInline UINT SetDrawTextFlags(UINT uMask, UINT uDTFlags) const
	{
		return (UINT)SendMsg(TB_SETDRAWTEXTFLAGS, uMask, uDTFlags);
	}

	EckInline DWORD SetTBExtendStyle(DWORD dwExStyle) const
	{
		return (DWORD)SendMsg(TB_SETEXTENDEDSTYLE, 0, dwExStyle);
	}

	EckInline HIMAGELIST SetHotImageList(HIMAGELIST hImageList) const
	{
		return (HIMAGELIST)SendMsg(TB_SETHOTIMAGELIST, 0, (LPARAM)hImageList);
	}

	EckInline int SetHotItem(int idxHot) const
	{
		return (int)SendMsg(TB_SETHOTITEM, idxHot, 0);
	}

	EckInline int SetHotItem(int idxHot, DWORD dwFlags) const
	{
		return (int)SendMsg(TB_SETHOTITEM2, idxHot, dwFlags);
	}

	EckInline HIMAGELIST SetImageList(HIMAGELIST hImageList, int idxImageList = 0) const
	{
		return (HIMAGELIST)SendMsg(TB_SETIMAGELIST, idxImageList, (LPARAM)hImageList);
	}

	EckInline BOOL SetIndent(int iIndent) const
	{
		return (BOOL)SendMsg(TB_SETINDENT, iIndent, 0);
	}

	EckInline void SetInsertMark(TBINSERTMARK* ptbim) const
	{
		SendMsg(TB_SETINSERTMARK, 0, (LPARAM)ptbim);
	}

	EckInline COLORREF SetInsertMarkColor(COLORREF cr) const
	{
		return (COLORREF)SendMsg(TB_SETINSERTMARKCOLOR, 0, cr);
	}

	EckInline void SetListGap(int iGap) const
	{
		SendMsg(TB_SETLISTGAP, iGap, 0);
	}

	EckInline BOOL SetMaxRows(int cRows) const
	{
		return (BOOL)SendMsg(TB_SETMAXTEXTROWS, cRows, 0);
	}

	EckInline void SetMetrics(TBMETRICS* ptbm) const
	{
		SendMsg(TB_SETMETRICS, 0, (LPARAM)ptbm);
	}

	EckInline DWORD SetPadding(int xPadding, int yPadding) const
	{
		return (DWORD)SendMsg(TB_SETPADDING, 0, MAKELPARAM(xPadding, yPadding));
	}

	EckInline HWND SetParent(HWND hWndNotify) const
	{
		return (HWND)SendMsg(TB_SETPARENT, (WPARAM)hWndNotify, 0);
	}

	EckInline HIMAGELIST SetPressedImageList(HIMAGELIST hImageList, int idxImageList = 0) const
	{
		return (HIMAGELIST)SendMsg(TB_SETPRESSEDIMAGELIST, idxImageList, (LPARAM)hImageList);
	}

	EckInline void SetRows(int cRows, BOOL bAllowMoreRows, RECT* prc) const
	{
		SendMsg(TB_SETROWS, MAKEWPARAM(cRows, bAllowMoreRows), (LPARAM)prc);
	}

	EckInline UINT SetState(int idCommand, UINT uState) const
	{
		return (UINT)SendMsg(TB_SETSTATE, idCommand, uState);
	}

	EckInline void SetStyleTB(DWORD dwStyle) const
	{
		SendMsg(TB_SETSTYLE, 0, dwStyle);
	}

	EckInline void SetToolTips(HWND hToolTip) const
	{
		SendMsg(TB_SETTOOLTIPS, (WPARAM)hToolTip, 0);
	}
};
ECK_RTTI_IMPL_BASE_INLINE(CToolBar, CWnd);
ECK_NAMESPACE_END