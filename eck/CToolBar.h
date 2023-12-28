#pragma once
#include "CWnd.h"

ECK_NAMESPACE_BEGIN
class CToolBar :public CWnd
{
public:
	ECK_CWND_CREATE;
	HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, HMENU hMenu, PCVOID pData = NULL) override
	{
		dwStyle |= WS_CHILD;
		return IntCreate(dwExStyle, TOOLBARCLASSNAMEW, pszText, dwStyle,
			x, y, cx, cy, hParent, hMenu, NULL, NULL);
	}

	EckInline int AddBitmap(int cBitmaps, HINSTANCE hInstance, UINT_PTR nID)
	{
		TBADDBITMAP tbab{ hInstance,nID };
		return (int)SendMsg(TB_ADDBITMAP, cBitmaps, (LPARAM)&tbab);
	}

	EckInline BOOL AddButtons(int cButtons, TBBUTTON* ptbb)
	{
		return (int)SendMsg(TB_ADDBUTTONSW, cButtons, (LPARAM)ptbb);
	}

	EckInline int AddString(HINSTANCE hInstance, PCWSTR pszIDOrText)
	{
		return (int)SendMsg(TB_ADDSTRINGW, (WPARAM)hInstance, (LPARAM)pszIDOrText);
	}

	EckInline void AutoSize()
	{
		SendMsg(TB_AUTOSIZE, 0, 0);
	}

	EckInline int GetButtonCount()
	{
		return (int)SendMsg(TB_BUTTONCOUNT, 0, 0);
	}

	EckInline void SetButtonStructSize(int cbStruct = sizeof(TBBUTTON))
	{
		SendMsg(TB_BUTTONSTRUCTSIZE, cbStruct, 0);
	}

	EckInline BOOL ChangeBitmap(int idCommand, int idxImg)
	{
		return (BOOL)SendMsg(TB_CHANGEBITMAP, idCommand, idxImg);
	}

	EckInline BOOL CheckButton(int idCommand, BOOL bChecked)
	{
		return (BOOL)SendMsg(TB_CHECKBUTTON, idCommand, bChecked);
	}

	EckInline int CommandToIndex(int idCommand)
	{
		return (int)SendMsg(TB_COMMANDTOINDEX, idCommand, 0);
	}

	EckInline void Customize()
	{
		SendMsg(TB_CUSTOMIZE, 0, 0);
	}

	EckInline BOOL DeleteButton(int idxButton)
	{
		return (BOOL)SendMsg(TB_DELETEBUTTON, idxButton, 0);
	}

	EckInline BOOL EnableButton(int idCommand, BOOL bEnabled)
	{
		return (BOOL)SendMsg(TB_ENABLEBUTTON, idCommand, bEnabled);
	}

	EckInline BOOL GetAnchorHighlight()
	{
		return (BOOL)SendMsg(TB_GETANCHORHIGHLIGHT, 0, 0);
	}

	EckInline int GetBitmap(int idCommand)
	{
		return (int)SendMsg(TB_GETBITMAP, idCommand, 0);
	}

	EckInline DWORD GetBitmapFlags()
	{
		return (DWORD)SendMsg(TB_GETBITMAPFLAGS, 0, 0);
	}

	EckInline BOOL GetButton(int idxButton, TBBUTTON* ptbb)
	{
		return (BOOL)SendMsg(TB_GETBUTTON, idxButton, (LPARAM)ptbb);
	}

	EckInline int GetButtonInfo(int idCommand, TBBUTTONINFOW* ptbbi)
	{
		return (int)SendMsg(TB_GETBUTTONINFOW, idCommand, (LPARAM)ptbbi);
	}

	EckInline DWORD GetButtonSize()
	{
		return (DWORD)SendMsg(TB_GETBUTTONSIZE, 0, 0);
	}

	EckInline void GetButtonSize(int* pcx, int* pcy)
	{
		DWORD dwRet = (DWORD)SendMsg(TB_GETBUTTONSIZE, 0, 0);
		if (pcx)
			*pcx = LOWORD(dwRet);
		if (pcy)
			*pcy = HIWORD(dwRet);
	}

	EckInline int GetButtonText(int idCommand, PWSTR pszBuf)
	{
		return (int)SendMsg(TB_GETBUTTONTEXTW, idCommand, (LPARAM)pszBuf);
	}

	EckInline BOOL GetColorScheme(COLORSCHEME* pcs)
	{
		return (BOOL)SendMsg(TB_GETCOLORSCHEME, 0, (LPARAM)pcs);
	}

	EckInline HIMAGELIST GetDisabledImageList()
	{
		return (HIMAGELIST)SendMsg(TB_GETDISABLEDIMAGELIST, 0, 0);
	}

	EckInline DWORD GetExtendedStyle()
	{
		return (DWORD)SendMsg(TB_GETEXTENDEDSTYLE, 0, 0);
	}

	EckInline HIMAGELIST GetHotImageList()
	{
		return (HIMAGELIST)SendMsg(TB_GETHOTIMAGELIST, 0, 0);
	}

	EckInline int GetHotItem()
	{
		return (int)SendMsg(TB_GETHOTITEM, 0, 0);
	}

	EckInline BOOL GetIdealSize(BOOL bHeight, SIZE* psize)
	{
		return (BOOL)SendMsg(TB_GETIDEALSIZE, bHeight, (LPARAM)psize);
	}

	EckInline BOOL GetIdealSize(SIZE* psize)
	{
		SIZE size;
		BOOL b = (BOOL)SendMsg(TB_GETIDEALSIZE, TRUE, (LPARAM)&size);
		psize->cy = size.cy;
		b = b && (BOOL)SendMsg(TB_GETIDEALSIZE, FALSE, (LPARAM)&size);
		psize->cx = size.cx;
		return b;
	}

	EckInline HIMAGELIST GetImageList()
	{
		return (HIMAGELIST)SendMsg(TB_GETIMAGELIST, 0, 0);
	}

	EckInline int GetImageListCount()
	{
		return (int)SendMsg(TB_GETIMAGELISTCOUNT, 0, 0);
	}

	EckInline void GetInsertMark(TBINSERTMARK* ptbim)
	{
		SendMsg(TB_GETINSERTMARK, 0, (LPARAM)ptbim);
	}

	EckInline COLORREF GetInsertMarkColor()
	{
		return (COLORREF)SendMsg(TB_GETINSERTMARKCOLOR, 0, 0);
	}

	EckInline void GetItemDropDownRect(int idxButton, RECT* prc)
	{
		SendMsg(TB_GETITEMDROPDOWNRECT, idxButton, (LPARAM)prc);
	}

	EckInline BOOL GetItemRect(int idxButton, RECT* prc)
	{
		return (BOOL)SendMsg(TB_GETITEMRECT, idxButton, (LPARAM)prc);
	}

	EckInline BOOL GetMaxSize(SIZE* psize)
	{
		return (BOOL)SendMsg(TB_GETMAXSIZE, 0, (LPARAM)psize);
	}

	EckInline void GetMetrics(TBMETRICS* ptbm)
	{
		SendMsg(TB_GETMETRICS, 0, (LPARAM)ptbm);
	}
#pragma push_macro("GetObject")
#undef GetObject
	EckInline HRESULT GetObject(IID* piid, IDropTarget** ppDropTarget)
	{
		return (HRESULT)SendMsg(TB_GETOBJECT, (WPARAM)piid, (LPARAM)ppDropTarget);
	}
#pragma pop_macro("GetObject")
	EckInline DWORD GetPadding()
	{
		return (DWORD)SendMsg(TB_GETPADDING, 0, 0);
	}

	EckInline void GetPadding(int* pxPadding, int* pcPadding)
	{
		DWORD dwRet = (DWORD)SendMsg(TB_GETPADDING, 0, 0);
		if (pxPadding)
			*pxPadding = LOWORD(dwRet);
		if (pcPadding)
			*pcPadding = HIWORD(dwRet);
	}

	EckInline HIMAGELIST GetPressedImageList()
	{
		return (HIMAGELIST)SendMsg(TB_GETPRESSEDIMAGELIST, 0, 0);
	}

	EckInline BOOL GetRect(int idCommand, RECT* prc)
	{
		return (BOOL)SendMsg(TB_GETRECT, idCommand, (LPARAM)prc);
	}

	EckInline int GetRows()
	{
		return (int)SendMsg(TB_GETROWS, 0, 0);
	}

	EckInline UINT GetState(int idxButton)
	{
		return (UINT)SendMsg(TB_GETSTATE, idxButton, 0);
	}

	EckInline int GetString(int idxString, int cbBuf, PWSTR pszBuf)
	{
		return (int)SendMsg(TB_GETSTRINGW, MAKEWPARAM(cbBuf, idxString), (LPARAM)pszBuf);
	}

	EckInline DWORD GetStyleTB()
	{
		return (DWORD)SendMsg(TB_GETSTYLE, 0, 0);
	}

	EckInline int GetTextRows()
	{
		return (int)SendMsg(TB_GETTEXTROWS, 0, 0);
	}

	EckInline HWND GetToolTips()
	{
		return (HWND)SendMsg(TB_GETTOOLTIPS, 0, 0);
	}

	EckInline BOOL HideButton(int idCommand, BOOL bHide)
	{
		return (BOOL)SendMsg(TB_HIDEBUTTON, idCommand, MAKELPARAM(bHide, 0));
	}

	EckInline int HitTest(POINT* ppt)
	{
		return (int)SendMsg(TB_HITTEST, 0, (LPARAM)ppt);
	}

	EckInline BOOL Indeterminate(int idCommand, BOOL bIndeterminate)
	{
		return (BOOL)SendMsg(TB_INDETERMINATE, idCommand, MAKELPARAM(bIndeterminate, 0));
	}

	EckInline BOOL InsertButton(int pos, TBBUTTON* ptbb)
	{
		return (BOOL)SendMsg(TB_INSERTBUTTONW, pos, (LPARAM)ptbb);
	}

	EckInline BOOL InsertMarkHitTest(POINT* ppt, TBINSERTMARK* ptbim)
	{
		return (BOOL)SendMsg(TB_INSERTMARKHITTEST, (WPARAM)ppt, (LPARAM)ptbim);
	}

	EckInline BOOL IsButtonChecked(int idCommand)
	{
		return (BOOL)SendMsg(TB_ISBUTTONCHECKED, idCommand, 0);
	}

	EckInline BOOL IsButtonEnabled(int idCommand)
	{
		return (BOOL)SendMsg(TB_ISBUTTONENABLED, idCommand, 0);
	}

	EckInline BOOL IsButtonHidden(int idCommand)
	{
		return (BOOL)SendMsg(TB_ISBUTTONHIDDEN, idCommand, 0);
	}

	EckInline BOOL IsButtonHighlighted(int idCommand)
	{
		return (BOOL)SendMsg(TB_ISBUTTONHIGHLIGHTED, idCommand, 0);
	}

	EckInline BOOL IsButtonIndeterminate(int idCommand)
	{
		return (BOOL)SendMsg(TB_ISBUTTONINDETERMINATE, idCommand, 0);
	}

	EckInline BOOL IsButtonPressed(int idCommand)
	{
		return (BOOL)SendMsg(TB_ISBUTTONPRESSED, idCommand, 0);
	}

	EckInline int LoadImages(UINT_PTR nID)
	{
		return (int)SendMsg(TB_LOADIMAGES, nID, (LPARAM)HINST_COMMCTRL);
	}

	EckInline BOOL MapAccelerator(WCHAR chAccel, UINT* pidCommand)
	{
		return (BOOL)SendMsg(TB_MAPACCELERATORW, chAccel, (LPARAM)pidCommand);
	}

	EckInline BOOL MarkButton(int idCommand, BOOL bHighlighted)
	{
		return (BOOL)SendMsg(TB_MARKBUTTON, idCommand, MAKELPARAM(bHighlighted, 0));
	}

	EckInline BOOL MoveButton(int idx, int idxDest)
	{
		return (BOOL)SendMsg(TB_MOVEBUTTON, idx, idxDest);
	}

	EckInline BOOL PressButton(int idCommand, BOOL bPressed)
	{
		return (BOOL)SendMsg(TB_PRESSBUTTON, idCommand, MAKELPARAM(bPressed, 0));
	}

	EckInline BOOL ReplaceBitmap(TBREPLACEBITMAP* ptbrb)
	{
		return (BOOL)SendMsg(TB_REPLACEBITMAP, 0, (LPARAM)ptbrb);
	}

	EckInline void SaveRestore(TBSAVEPARAMSW* ptbsp)
	{
		SendMsg(TB_SAVERESTOREW, 0, (LPARAM)ptbsp);
	}

	EckInline BOOL SetAnchorHighlighted(BOOL bHighlighted)
	{
		return (BOOL)SendMsg(TB_SETANCHORHIGHLIGHT, bHighlighted, 0);
	}

	EckInline BOOL SetBitmapSize(int cx, int cy)
	{
		return (BOOL)SendMsg(TB_SETBITMAPSIZE, 0, MAKELPARAM(cx, cy));
	}

	EckInline BOOL SetButtonInfo(int idCommand, TBBUTTONINFOW* ptbbi)
	{
		return (BOOL)SendMsg(TB_SETBUTTONINFOW, idCommand, (LPARAM)ptbbi);
	}

	EckInline BOOL SetButtonSize(int cx, int cy)
	{
		return (BOOL)SendMsg(TB_SETBUTTONSIZE, 0, MAKELPARAM(cx, cy));
	}

	EckInline BOOL SetButtonWidth(int cxMin, int cxMax)
	{
		return (BOOL)SendMsg(TB_SETBUTTONWIDTH, 0, MAKELPARAM(cxMin, cxMax));
	}

	EckInline BOOL SetCommandID(int idxButton, int idNew)
	{
		return (BOOL)SendMsg(TB_SETCMDID, idxButton, idNew);
	}

	EckInline void SetColorScheme(COLORSCHEME* pcs)
	{
		SendMsg(TB_SETCOLORSCHEME, 0, (LPARAM)pcs);
	}

	EckInline HIMAGELIST SetDisabledImageList(HIMAGELIST hImageList)
	{
		return (HIMAGELIST)SendMsg(TB_SETDISABLEDIMAGELIST, 0, (LPARAM)hImageList);
	}

	EckInline UINT SetDrawTextFlags(UINT uMask, UINT uDTFlags)
	{
		return (UINT)SendMsg(TB_SETDRAWTEXTFLAGS, uMask, uDTFlags);
	}

	EckInline DWORD SetExtendedStyle(DWORD dwExStyle)
	{
		return (DWORD)SendMsg(TB_SETEXTENDEDSTYLE, 0, dwExStyle);
	}

	EckInline HIMAGELIST SetHotImageList(HIMAGELIST hImageList)
	{
		return (HIMAGELIST)SendMsg(TB_SETHOTIMAGELIST, 0, (LPARAM)hImageList);
	}

	EckInline int SetHotItem(int idxHot)
	{
		return (int)SendMsg(TB_SETHOTITEM, idxHot, 0);
	}

	EckInline int SetHotItem(int idxHot, DWORD dwFlags)
	{
		return (int)SendMsg(TB_SETHOTITEM2, idxHot, dwFlags);
	}

	EckInline HIMAGELIST SetImageList(HIMAGELIST hImageList, int idxImageList = 0)
	{
		return (HIMAGELIST)SendMsg(TB_SETIMAGELIST, idxImageList, (LPARAM)hImageList);
	}

	EckInline BOOL SetIndent(int iIndent)
	{
		return (BOOL)SendMsg(TB_SETINDENT, iIndent, 0);
	}

	EckInline void SetInsertMark(TBINSERTMARK* ptbim)
	{
		SendMsg(TB_SETINSERTMARK, 0, (LPARAM)ptbim);
	}

	EckInline COLORREF SetInsertMarkColor(COLORREF cr)
	{
		return (COLORREF)SendMsg(TB_SETINSERTMARKCOLOR, 0, cr);
	}

	EckInline void SetListGap(int iGap)
	{
		SendMsg(TB_SETLISTGAP, iGap, 0);
	}

	EckInline BOOL SetMaxRows(int cRows)
	{
		return (BOOL)SendMsg(TB_SETMAXTEXTROWS, cRows, 0);
	}

	EckInline void SetMetrics(TBMETRICS* ptbm)
	{
		SendMsg(TB_SETMETRICS, 0, (LPARAM)ptbm);
	}

	EckInline DWORD SetPadding(int xPadding, int yPadding)
	{
		return (DWORD)SendMsg(TB_SETPADDING, 0, MAKELPARAM(xPadding, yPadding));
	}

	EckInline HWND SetParent(HWND hWndNotify)
	{
		return (HWND)SendMsg(TB_SETPARENT, (WPARAM)hWndNotify, 0);
	}

	EckInline HIMAGELIST SetPressedImageList(HIMAGELIST hImageList, int idxImageList = 0)
	{
		return (HIMAGELIST)SendMsg(TB_SETPRESSEDIMAGELIST, idxImageList, (LPARAM)hImageList);
	}

	EckInline void SetRows(int cRows, BOOL bAllowMoreRows, RECT* prc)
	{
		SendMsg(TB_SETROWS, MAKEWPARAM(cRows, bAllowMoreRows), (LPARAM)prc);
	}

	EckInline UINT SetState(int idCommand, UINT uState)
	{
		return (UINT)SendMsg(TB_SETSTATE, idCommand, uState);
	}

	EckInline void SetStyleTB(DWORD dwStyle)
	{
		SendMsg(TB_SETSTYLE, 0, dwStyle);
	}

	EckInline void SetToolTips(HWND hToolTip)
	{
		SendMsg(TB_SETTOOLTIPS, (WPARAM)hToolTip, 0);
	}
};
ECK_NAMESPACE_END