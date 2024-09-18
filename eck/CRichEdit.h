/*
* WinEzCtrlKit Library
*
* CRichEdit.h ： 标准丰富文本框
*
* Copyright(C) 2024 QingKong
*/
#pragma once
#include "CWnd.h"

#include <Richedit.h>
#include <Richole.h>

ECK_NAMESPACE_BEGIN
class CRichEdit : public CWnd
{
public:
	ECK_RTTI(CRichEdit);

	ECK_CWND_CREATE;
	HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, HMENU hMenu, PCVOID pData = nullptr) override
	{
		return IntCreate(dwExStyle, MSFTEDIT_CLASS, pszText, dwStyle,
			x, y, cx, cy, hParent, hMenu, nullptr, nullptr);
	}

	/// <summary>
	/// 启用/禁用自动URL检测
	/// </summary>
	/// <param name="iType">AURL_*</param>
	/// <param name="pszUrlPatterns">为NULL使用默认URL方案，否则为自定义URL方案，如（news:http:ftp:telnet:），最多50个</param>
	/// <returns>HRESULT</returns>
	EckInline HRESULT AutoUrlDetect(int iType, PCWSTR pszUrlPatterns = nullptr)
	{
		return (HRESULT)SendMsg(EM_AUTOURLDETECT, iType, (LPARAM)pszUrlPatterns);
	}

	EckInline HRESULT CallAutoCorrectProc(WCHAR ch)
	{
		return (HRESULT)SendMsg(EM_CALLAUTOCORRECTPROC, ch, 0);
	}

	EckInline HRESULT CanPaste(UINT uClipboardFormat)
	{
		return (HRESULT)SendMsg(EM_CANPASTE, uClipboardFormat, 0);
	}

	EckInline HRESULT CanRedo()
	{
		return (HRESULT)SendMsg(EM_CANREDO, 0, 0);
	}

	EckInline BOOL DisplayBand(RECT* prc)
	{
		return (BOOL)SendMsg(EM_DISPLAYBAND, 0, (LPARAM)prc);
	}

	EckInline void GetSel(CHARRANGE* pchrg)
	{
		SendMsg(EM_EXGETSEL, 0, (LPARAM)pchrg);
	}

	EckInline void LimitText(int cch)
	{
		SendMsg(EM_EXLIMITTEXT, 0, cch);
	}

	EckInline int LineFromChar(int ich)
	{
		return (int)SendMsg(EM_EXLINEFROMCHAR, 0, ich);
	}

	EckInline LRESULT SetSel(CHARRANGE* pchrg)
	{
		return SendMsg(EM_EXSETSEL, 0, (LPARAM)pchrg);
	}

	EckInline int FindTextW(UINT uFlags, FINDTEXTW* pFindText)
	{
		return (int)SendMsg(EM_FINDTEXTW, uFlags, (LPARAM)pFindText);
	}

	EckInline int FindTextEx(UINT uFlags, FINDTEXTEXW* pFindText)
	{
		return (int)SendMsg(EM_FINDTEXTEXW, uFlags, (LPARAM)pFindText);
	}

	EckInline int FindWordBreak(UINT uFlags, int idxChar)
	{
		return (int)SendMsg(EM_FINDWORDBREAK, uFlags, idxChar);
	}

	EckInline int FormatRange(BOOL bInPlace, FORMATRANGE* pfr)
	{
		return (int)SendMsg(EM_FORMATRANGE, bInPlace, (LPARAM)pfr);
	}

	EckInline AutoCorrectProc GetAutoCorrectProc()
	{
		return (AutoCorrectProc)SendMsg(EM_GETAUTOCORRECTPROC, 0, 0);
	}

	EckInline BOOL GetAutoURLDetect()
	{
		return (BOOL)SendMsg(EM_GETAUTOURLDETECT, 0, 0);
	}

	EckInline void GetBidiOptions(BIDIOPTIONS* pbidio)
	{
		SendMsg(EM_GETBIDIOPTIONS, 0, (LPARAM)pbidio);
	}

	EckInline DWORD GetCharFormat(int iRange, CHARFORMAT2W* pcf)
	{
		return (DWORD)SendMsg(EM_GETCHARFORMAT, iRange, (LPARAM)pcf);
	}

	EckInline int GetCtfModeBias()
	{
		return (int)SendMsg(EM_GETCTFMODEBIAS, 0, 0);
	}

	EckInline BOOL GetCtfOpenStatus()
	{
		return (BOOL)SendMsg(EM_GETCTFOPENSTATUS, 0, 0);
	}

	EckInline UINT GetEditStyle()
	{
		return (UINT)SendMsg(EM_GETEDITSTYLE, 0, 0);
	}

	EckInline UINT GetEditStyleEx()
	{
		return (UINT)SendMsg(EM_GETEDITSTYLEEX, 0, 0);
	}

	EckInline DWORD GetEllipsisMode()
	{
		DWORD dw;
		SendMsg(EM_GETELLIPSISMODE, 0, (LPARAM)&dw);
		return dw;
	}

	EckInline BOOL GetEllipsisState()
	{
		return (BOOL)SendMsg(EM_GETELLIPSISSTATE, 0, 0);
	}

	EckInline UINT GetEventMask()
	{
		return (UINT)SendMsg(EM_GETEVENTMASK, 0, 0);
	}

	EckInline void GetHyphenateInfo(HYPHRESULT* phr)
	{
		SendMsg(EM_GETHYPHENATEINFO, (WPARAM)phr, 0);
	}

	EckInline int GetImeCompositionMode()
	{
		return (int)SendMsg(EM_GETIMECOMPMODE, 0, 0);
	}

	EckInline int GetImeCompositionText(IMECOMPTEXT* pict, PWSTR pszBuf)
	{
		return (int)SendMsg(EM_GETIMECOMPTEXT, (WPARAM)pict, (LPARAM)pszBuf);
	}

	EckInline int GetImeModeBias()
	{
		return (int)SendMsg(EM_GETIMEMODEBIAS, 0, 0);
	}

	EckInline UINT GetImeProperty(int iType)
	{
		return (UINT)SendMsg(EM_GETIMEPROPERTY, iType, 0);
	}

	EckInline UINT GetLangOptions()
	{
		return (UINT)SendMsg(EM_GETLANGOPTIONS, 0, 0);
	}

	EckInline HRESULT GetOleInterface(IRichEditOle** ppRichEditOle)
	{
		return (HRESULT)SendMsg(EM_GETOLEINTERFACE, 0, (LPARAM)ppRichEditOle);
	}

	EckInline UINT GetOptions()
	{
		return (UINT)SendMsg(EM_GETOPTIONS, 0, 0);
	}

	EckInline UINT GetPageRotate()
	{
		return (UINT)SendMsg(EM_GETPAGEROTATE, 0, 0);
	}

	EckInline UINT GetParaFormat(PARAFORMAT2* ppf)
	{
		return (UINT)SendMsg(EM_GETPARAFORMAT, 0, (LPARAM)ppf);
	}

	EckInline UNDONAMEID GetRedoName()
	{
		return (UNDONAMEID)SendMsg(EM_GETREDONAME, 0, 0);
	}

	EckInline void GetScrollPos(POINT* ppt)
	{
		SendMsg(EM_GETSCROLLPOS, 0, (LPARAM)ppt);
	}

	EckInline int GetSelText(PWSTR pszBuf)
	{
		return (int)SendMsg(EM_GETSELTEXT, 0, (LPARAM)pszBuf);
	}

	EckInline int GetStoryType(int idxStory)
	{
		return (int)SendMsg(EM_GETSTORYTYPE, idxStory, 0);
	}

	EckInline HRESULT GetTableParams(TABLEROWPARMS* ptrp, TABLECELLPARMS* ptcp)
	{
		return (HRESULT)SendMsg(EM_GETTABLEPARMS, (WPARAM)ptrp, (LPARAM)ptcp);
	}

	EckInline int GetTextEx(GETTEXTEX* pgt, PWSTR pszBuf)
	{
		return (int)SendMsg(EM_GETTEXTEX, (WPARAM)pgt, (LPARAM)pszBuf);
	}

	EckInline int GetTextLengthEx(GETTEXTLENGTHEX* pgtl)
	{
		return (int)SendMsg(EM_GETTEXTLENGTHEX, (WPARAM)pgtl, 0);
	}

	EckInline TEXTMODE GetTextMode()
	{
		return (TEXTMODE)SendMsg(EM_GETTEXTMODE, 0, 0);
	}

	EckInline int GetTextRange(TEXTRANGEW* pTextRange)
	{
		return (int)SendMsg(EM_GETTEXTRANGE, 0, (LPARAM)pTextRange);
	}

	EckInline BOOL GetTouchOptions()
	{
		return (BOOL)SendMsg(EM_GETTOUCHOPTIONS, RTO_SHOWHANDLES, 0);
	}

	EckInline UINT GetTypographyOptions()
	{
		return (UINT)SendMsg(EM_GETTYPOGRAPHYOPTIONS, 0, 0);
	}

	EckInline UNDONAMEID GetUndoName()
	{
		return (UNDONAMEID)SendMsg(EM_GETUNDONAME, 0, 0);
	}

	EckInline EDITWORDBREAKPROCEX GetWordBreakProc()
	{
		return (EDITWORDBREAKPROCEX)SendMsg(EM_GETWORDBREAKPROCEX, 0, 0);
	}

	EckInline BOOL GetZoom(int* pnZoomNumerator, int* pnZoomDenominator)
	{
		return (BOOL)SendMsg(EM_GETZOOM, (WPARAM)pnZoomNumerator, (LPARAM)pnZoomDenominator);
	}

	EckInline void HideSelection(BOOL bHide)
	{
		SendMsg(EM_HIDESELECTION, bHide, 0);
	}

	EckInline HRESULT InsertImage(RICHEDIT_IMAGE_PARAMETERS* pImageParams)
	{
		return (HRESULT)SendMsg(EM_INSERTIMAGE, 0, (LPARAM)pImageParams);
	}

	EckInline HRESULT InsertTable(TABLEROWPARMS* ptrp, TABLECELLPARMS* ptcp)
	{
		return (HRESULT)SendMsg(EM_INSERTTABLE, (WPARAM)ptrp, (LPARAM)ptcp);
	}

	EckInline BOOL IsIme()
	{
		return (BOOL)SendMsg(EM_ISIME, 0, 0);
	}

	EckInline void PasteSpecial(UINT uClipFormat, REPASTESPECIAL* prps)
	{
		SendMsg(EM_PASTESPECIAL, uClipFormat, (LPARAM)prps);
	}

	EckInline void Reconversion()
	{
		SendMsg(EM_RECONVERSION, 0, 0);
	}

	EckInline BOOL Redo()
	{
		return (BOOL)SendMsg(EM_REDO, 0, 0);
	}

	EckInline void RequestResize()
	{
		SendMsg(EM_REQUESTRESIZE, 0, 0);
	}

	EckInline UINT SelectionType()
	{
		return (UINT)SendMsg(EM_SELECTIONTYPE, 0, 0);
	}

	EckInline BOOL SetAutoCorrectProc(AutoCorrectProc pfnNewProc)
	{
		return (BOOL)SendMsg(EM_SETAUTOCORRECTPROC, (WPARAM)pfnNewProc, 0);
	}

	EckInline void SetBidiOptions(BIDIOPTIONS* pbidio)
	{
		SendMsg(EM_SETBIDIOPTIONS, 0, (LPARAM)pbidio);
	}

	EckInline COLORREF SetBackgroundColor(BOOL bSysColor, COLORREF cr)
	{
		return (COLORREF)SendMsg(EM_SETBKGNDCOLOR, bSysColor, cr);
	}

	EckInline BOOL SetCharFormat(int iFmt, CHARFORMAT2W* pcf)
	{
		return (BOOL)SendMsg(EM_SETCHARFORMAT, iFmt, (LPARAM)pcf);
	}

	EckInline int SetCtfModeBias(int iModeBias)
	{
		return (int)SendMsg(EM_SETCTFMODEBIAS, iModeBias, 0);
	}

	EckInline BOOL SetCtfOpenStatus(BOOL bOpen)
	{
		return (BOOL)SendMsg(EM_SETCTFOPENSTATUS, bOpen, 0);
	}

	EckInline BOOL SetDisableOleLinkConversion(BOOL bDisable)
	{
		return (BOOL)SendMsg(EM_SETDISABLEOLELINKCONVERSION, 0, bDisable);
	}

	EckInline UINT SetEditStyle(UINT dwStyle, UINT uMask)
	{
		return (UINT)SendMsg(EM_SETEDITSTYLE, dwStyle, uMask);
	}

	EckInline UINT SetEditStyleEx(UINT dwStyleEx, UINT dwMask)
	{
		return (UINT)SendMsg(EM_SETEDITSTYLEEX, dwStyleEx, dwMask);
	}

	EckInline BOOL SetEllipsisMode(DWORD dwElideMode)
	{
		return (BOOL)SendMsg(EM_SETELLIPSISMODE, 0, dwElideMode);
	}

	EckInline UINT SetEventMask(UINT dwEventMask)
	{
		return (UINT)SendMsg(EM_SETEVENTMASK, 0, dwEventMask);
	}

	EckInline BOOL SetFontSize(int iSize)
	{
		return (BOOL)SendMsg(EM_SETFONTSIZE, iSize, 0);
	}

	EckInline void SetHyphenateInfo(HYPHENATEINFO* phr)
	{
		SendMsg(EM_SETHYPHENATEINFO, (LPARAM)phr, 0);
	}

	EckInline int SetImeModeBias(int iModeBias)
	{
		return (int)SendMsg(EM_SETIMEMODEBIAS, iModeBias, iModeBias);
	}

	EckInline void SetLangOptions(UINT dwOptions)
	{
		SendMsg(EM_SETLANGOPTIONS, 0, dwOptions);
	}

	EckInline BOOL SetOleCallback(IRichEditOleCallback* pCallback)
	{
		return (BOOL)SendMsg(EM_SETOLECALLBACK, 0, (LPARAM)pCallback);
	}

	EckInline UINT SetOptions(int iType, UINT dwOptions)
	{
		return (UINT)SendMsg(EM_SETOPTIONS, iType, dwOptions);
	}

	EckInline BOOL SetParaFormat(PARAFORMAT2* ppf)
	{
		return (BOOL)SendMsg(EM_SETPARAFORMAT, 0, (LPARAM)ppf);
	}

	EckInline int SetPageRotate(int iRotate)
	{
		return (int)SendMsg(EM_SETPAGEROTATE, iRotate, 0);
	}

	EckInline void SetPalette(HPALETTE hPal)
	{
		SendMsg(EM_SETPALETTE, (WPARAM)hPal, 0);
	}

	EckInline HRESULT SetQueryConvertOleLinkCallback(WPARAM Context, OLESTREAMQUERYCONVERTOLELINKCALLBACK pfnCallback)
	{
		return (HRESULT)SendMsg(EM_SETQUERYCONVERTOLELINKCALLBACK, Context, (LPARAM)pfnCallback);
	}

	EckInline void SetScrollPos(POINT* ppt)
	{
		SendMsg(EM_SETSCROLLPOS, 0, (LPARAM)ppt);
	}

	EckInline int SetStoryType(int idxStory, int iStoryType)
	{
		return (int)SendMsg(EM_SETSTORYTYPE, idxStory, iStoryType);
	}

	EckInline HRESULT SetTableParams(TABLEROWPARMS* ptrp, TABLECELLPARMS* ptcp)
	{
		return (HRESULT)SendMsg(EM_SETTABLEPARMS, (WPARAM)ptrp, (LPARAM)ptcp);
	}

	EckInline HRESULT SetTargetDevice(HDC hDC, int cxLine)
	{
		return (HRESULT)SendMsg(EM_SETTARGETDEVICE, (WPARAM)hDC, cxLine);
	}

	EckInline int SetTextEx(SETTEXTEX* pst, PCWSTR pszText)
	{
		EckAssert(pst->codepage == 1200);
		return (int)SendMsg(EM_SETTEXTEX, (WPARAM)pst, (LPARAM)pszText);
	}

	EckInline int SetTextEx(SETTEXTEX* pst, PCSTR pszText)
	{
		EckAssert(pst->codepage != 1200);
		return (int)SendMsg(EM_SETTEXTEX, (WPARAM)pst, (LPARAM)pszText);
	}

	EckInline BOOL SetTextMode(TEXTMODE tm)
	{
		return (BOOL)SendMsg(EM_SETTEXTMODE, tm, 0);
	}

	EckInline void SetTouchOptions(BOOL bShowHandles)
	{
		SendMsg(EM_SETTOUCHOPTIONS, RTO_SHOWHANDLES, bShowHandles);
	}

	EckInline BOOL SetTypographyOptions(UINT dwOptions, UINT dwMask)
	{
		return (BOOL)SendMsg(EM_SETTYPOGRAPHYOPTIONS, dwOptions, dwMask);
	}

	EckInline BOOL SetUiAutomationName(PCWSTR pszText)
	{
		return (BOOL)SendMsg(EM_SETUIANAME, 0, (LPARAM)pszText);
	}

	EckInline int SetUndoLimit(int cLimit)
	{
		return (int)SendMsg(EM_SETUNDOLIMIT, cLimit, 0);
	}

	EckInline EDITWORDBREAKPROCEX SetWordBreakProc(EDITWORDBREAKPROCEX pfnNewProc)
	{
		return (EDITWORDBREAKPROCEX)SendMsg(EM_SETWORDBREAKPROCEX, 0, (LPARAM)pfnNewProc);
	}

	EckInline BOOL SetZoom(int iZoomNumerator, int iZoomDenominator)
	{
		return (BOOL)SendMsg(EM_SETZOOM, iZoomNumerator, iZoomDenominator);
	}

	EckInline void ShowScrollBar(int iBar, BOOL bShow)
	{
		SendMsg(EM_SHOWSCROLLBAR, iBar, bShow);
	}

	EckInline void StopGroupTyping()
	{
		SendMsg(EM_STOPGROUPTYPING, 0, 0);
	}

	EckInline int StreamIn(UINT uFlags, EDITSTREAM* es)
	{
		return (int)SendMsg(EM_STREAMIN, uFlags, (LPARAM)es);
	}

	EckInline int StreamOut(UINT uFlags, EDITSTREAM* es)
	{
		return (int)SendMsg(EM_STREAMOUT, uFlags, (LPARAM)es);
	}
};
ECK_RTTI_IMPL_BASE_INLINE(CRichEdit, CWnd);
ECK_NAMESPACE_END