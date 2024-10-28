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
#define ECK_CWNDPROP_REE_STYLE(Name, Style)				\
	ECKPROP(StyleGet##Name, StyleSet##Name) BOOL Name;	\
	BOOL StyleGet##Name() const							\
	{													\
		if constexpr (Style == 0)						\
			return !GetEditStyle();						\
		else											\
			return IsBitSet(GetEditStyle(), Style);		\
	}													\
	void StyleSet##Name(BOOL b) const					\
	{													\
		SetEditStyle(b ? Style : 0, Style);				\
	}

#define ECK_CWNDPROP_REEEX_STYLE(Name, Style)			\
	ECKPROP(StyleGet##Name, StyleSet##Name) BOOL Name;	\
	BOOL StyleGet##Name() const							\
	{													\
		if constexpr (Style == 0)						\
			return !GetEditStyleEx();					\
		else											\
			return IsBitSet(GetEditStyleEx(), Style);	\
	}													\
	void StyleSet##Name(BOOL b) const					\
	{													\
		SetEditStyleEx(b ? Style : 0, Style);			\
	}

class CRichEdit : public CWnd
{
public:
	ECK_RTTI(CRichEdit);
	ECK_CWND_NOSINGLEOWNER(CRichEdit);
	ECK_CWND_CREATE_CLS(MSFTEDIT_CLASS);

	ECK_CWNDPROP_STYLE(AutoHScroll, ES_AUTOHSCROLL);
	ECK_CWNDPROP_STYLE(AutoVScroll, ES_AUTOVSCROLL);
	ECK_CWNDPROP_STYLE(Center, ES_CENTER);
	ECK_CWNDPROP_STYLE(Left, ES_LEFT);
	ECK_CWNDPROP_STYLE(Multiline, ES_MULTILINE);
	ECK_CWNDPROP_STYLE(NoHideSel, ES_NOHIDESEL);
	ECK_CWNDPROP_STYLE(Number, ES_NUMBER);
	ECK_CWNDPROP_STYLE(Password, ES_PASSWORD);
	ECK_CWNDPROP_STYLE(ReadOnly, ES_READONLY);
	ECK_CWNDPROP_STYLE(Right, ES_RIGHT);
	ECK_CWNDPROP_STYLE(WantReturn, ES_WANTRETURN);
	ECK_CWNDPROP_STYLE(DisableNoScroll, ES_DISABLENOSCROLL);
	ECK_CWNDPROP_STYLE(NoIme, ES_NOIME);
	ECK_CWNDPROP_STYLE(NoOleDragDrop, ES_NOOLEDRAGDROP);
	ECK_CWNDPROP_STYLE(SaveSel, ES_SAVESEL);
	ECK_CWNDPROP_STYLE(SelectionBar, ES_SELECTIONBAR);
	ECK_CWNDPROP_STYLE(SelfIme, ES_SELFIME);
	ECK_CWNDPROP_STYLE(Sunken, ES_SUNKEN);
	ECK_CWNDPROP_STYLE(Vertical, ES_VERTICAL);
	ECK_CWNDPROP_REE_STYLE(BeepOnMaxText, SES_BEEPONMAXTEXT);
	ECK_CWNDPROP_REE_STYLE(BiDi, SES_BIDI);
	ECK_CWNDPROP_REE_STYLE(CtfAllowEmbed, SES_CTFALLOWEMBED);
	ECK_CWNDPROP_REE_STYLE(CtfAllowProofing, SES_CTFALLOWPROOFING);
	ECK_CWNDPROP_REE_STYLE(CtfAllowSmartTag, SES_CTFALLOWSMARTTAG);
	ECK_CWNDPROP_REE_STYLE(DraftMode, SES_DRAFTMODE);
	ECK_CWNDPROP_REE_STYLE(EmulateSysEdit, SES_EMULATESYSEDIT);
	ECK_CWNDPROP_REE_STYLE(ExtendBackColor, SES_EXTENDBACKCOLOR);
	ECK_CWNDPROP_REE_STYLE(HideGridLines, SES_HIDEGRIDLINES);
	ECK_CWNDPROP_REE_STYLE(LowerCase, SES_LOWERCASE);
	ECK_CWNDPROP_REE_STYLE(NoFocusLinkNotify, SES_NOFOCUSLINKNOTIFY);
	ECK_CWNDPROP_REE_STYLE(NoImeEs, SES_NOIME);
	ECK_CWNDPROP_REE_STYLE(NoInputSequenceCheck, SES_NOINPUTSEQUENCECHK);
	ECK_CWNDPROP_REE_STYLE(ScrollOnKillFocus, SES_SCROLLONKILLFOCUS);
	ECK_CWNDPROP_REE_STYLE(UpperCase, SES_UPPERCASE);
	ECK_CWNDPROP_REE_STYLE(UseActiveImm, SES_USEAIMM);
	ECK_CWNDPROP_REE_STYLE(UseAtFont, SES_USEATFONT);
	ECK_CWNDPROP_REE_STYLE(UseCtf, SES_USECTF);
	ECK_CWNDPROP_REE_STYLE(ConvertCrCrLfToCr, SES_XLTCRCRLFTOCR);
	ECK_CWNDPROP_REEEX_STYLE(HandleFriendlyUrl, SES_EX_HANDLEFRIENDLYURL);
	ECK_CWNDPROP_REEEX_STYLE(MultiTouch, SES_EX_MULTITOUCH);
	ECK_CWNDPROP_REEEX_STYLE(NoAcetateSelection, SES_EX_NOACETATESELECTION);
	ECK_CWNDPROP_REEEX_STYLE(NoMath, SES_EX_NOMATH);
	ECK_CWNDPROP_REEEX_STYLE(NoTable, SES_EX_NOTABLE);
	ECK_CWNDPROP_REEEX_STYLE(UseSingleLine, SES_EX_USESINGLELINE);
	ECK_CWNDPROP_REEEX_STYLE(HideTempFormat, SES_EX_HIDETEMPFORMAT);
	ECK_CWNDPROP_REEEX_STYLE(UseMouseWParam, SES_EX_USEMOUSEWPARAM);
#if NTDDI_VERSION >= NTDDI_WINBLUE
	ECK_CWNDPROP_REE_STYLE(CtfNoLock, SES_CTFNOLOCK);
	ECK_CWNDPROP_REE_STYLE(DefaultLatinLiga, SES_DEFAULTLATINLIGA);
	ECK_CWNDPROP_REE_STYLE(Emulate1_0, SES_EMULATE10);
	ECK_CWNDPROP_REE_STYLE(HyperLinkTooltips, SES_HYPERLINKTOOLTIPS);
	ECK_CWNDPROP_REE_STYLE(LogicalCaret, SES_LOGICALCARET);
	ECK_CWNDPROP_REE_STYLE(MultiSelect, SES_MULTISELECT);
	ECK_CWNDPROP_REE_STYLE(NoEastAsianLineHeightAdjust, SES_NOEALINEHEIGHTADJUST);
	ECK_CWNDPROP_REE_STYLE(SmartDrapDrop, SES_SMARTDRAGDROP);
	ECK_CWNDPROP_REE_STYLE(WordDragDrop, SES_WORDDRAGDROP);
#endif

	/// <summary>
	/// 启用/禁用自动URL检测
	/// </summary>
	/// <param name="iType">AURL_*</param>
	/// <param name="pszUrlPatterns">为NULL使用默认URL方案，否则为自定义URL方案，如（news:http:ftp:telnet:），最多50个</param>
	/// <returns>HRESULT</returns>
	EckInline HRESULT AutoUrlDetect(int iType, PCWSTR pszUrlPatterns = nullptr) const
	{
		return (HRESULT)SendMsg(EM_AUTOURLDETECT, iType, (LPARAM)pszUrlPatterns);
	}

	EckInline HRESULT CallAutoCorrectProc(WCHAR ch) const
	{
		return (HRESULT)SendMsg(EM_CALLAUTOCORRECTPROC, ch, 0);
	}

	EckInline HRESULT CanPaste(UINT uClipboardFormat) const
	{
		return (HRESULT)SendMsg(EM_CANPASTE, uClipboardFormat, 0);
	}

	EckInline HRESULT CanRedo() const
	{
		return (HRESULT)SendMsg(EM_CANREDO, 0, 0);
	}

	EckInline BOOL DisplayBand(RECT* prc) const
	{
		return (BOOL)SendMsg(EM_DISPLAYBAND, 0, (LPARAM)prc);
	}

	EckInline void GetSel(CHARRANGE* pchrg) const
	{
		SendMsg(EM_EXGETSEL, 0, (LPARAM)pchrg);
	}

	EckInline void LimitText(int cch) const
	{
		SendMsg(EM_EXLIMITTEXT, 0, cch);
	}

	EckInline int LineFromChar(int ich) const
	{
		return (int)SendMsg(EM_EXLINEFROMCHAR, 0, ich);
	}

	EckInline LRESULT SetSel(CHARRANGE* pchrg) const
	{
		return SendMsg(EM_EXSETSEL, 0, (LPARAM)pchrg);
	}

	EckInline int FindTextW(UINT uFlags, FINDTEXTW* pFindText) const
	{
		return (int)SendMsg(EM_FINDTEXTW, uFlags, (LPARAM)pFindText);
	}

	EckInline int FindTextEx(UINT uFlags, FINDTEXTEXW* pFindText) const
	{
		return (int)SendMsg(EM_FINDTEXTEXW, uFlags, (LPARAM)pFindText);
	}

	EckInline int FindWordBreak(UINT uFlags, int idxChar) const
	{
		return (int)SendMsg(EM_FINDWORDBREAK, uFlags, idxChar);
	}

	EckInline int FormatRange(BOOL bInPlace, FORMATRANGE* pfr) const
	{
		return (int)SendMsg(EM_FORMATRANGE, bInPlace, (LPARAM)pfr);
	}

	EckInline AutoCorrectProc GetAutoCorrectProc() const
	{
		return (AutoCorrectProc)SendMsg(EM_GETAUTOCORRECTPROC, 0, 0);
	}

	EckInline BOOL GetAutoURLDetect() const
	{
		return (BOOL)SendMsg(EM_GETAUTOURLDETECT, 0, 0);
	}

	EckInline void GetBidiOptions(BIDIOPTIONS* pbidio) const
	{
		SendMsg(EM_GETBIDIOPTIONS, 0, (LPARAM)pbidio);
	}

	EckInline DWORD GetCharFormat(int iRange, CHARFORMAT2W* pcf) const
	{
		return (DWORD)SendMsg(EM_GETCHARFORMAT, iRange, (LPARAM)pcf);
	}

	EckInline int GetCtfModeBias() const
	{
		return (int)SendMsg(EM_GETCTFMODEBIAS, 0, 0);
	}

	EckInline BOOL GetCtfOpenStatus() const
	{
		return (BOOL)SendMsg(EM_GETCTFOPENSTATUS, 0, 0);
	}

	EckInline UINT GetEditStyle() const
	{
		return (UINT)SendMsg(EM_GETEDITSTYLE, 0, 0);
	}

	EckInline UINT GetEditStyleEx() const
	{
		return (UINT)SendMsg(EM_GETEDITSTYLEEX, 0, 0);
	}

	EckInline DWORD GetEllipsisMode() const
	{
		DWORD dw;
		SendMsg(EM_GETELLIPSISMODE, 0, (LPARAM)&dw);
		return dw;
	}

	EckInline BOOL GetEllipsisState() const
	{
		return (BOOL)SendMsg(EM_GETELLIPSISSTATE, 0, 0);
	}

	EckInline UINT GetEventMask() const
	{
		return (UINT)SendMsg(EM_GETEVENTMASK, 0, 0);
	}

	EckInline void GetHyphenateInfo(HYPHRESULT* phr) const
	{
		SendMsg(EM_GETHYPHENATEINFO, (WPARAM)phr, 0);
	}

	EckInline int GetImeCompositionMode() const
	{
		return (int)SendMsg(EM_GETIMECOMPMODE, 0, 0);
	}

	EckInline int GetImeCompositionText(IMECOMPTEXT* pict, PWSTR pszBuf) const
	{
		return (int)SendMsg(EM_GETIMECOMPTEXT, (WPARAM)pict, (LPARAM)pszBuf);
	}

	EckInline int GetImeModeBias() const
	{
		return (int)SendMsg(EM_GETIMEMODEBIAS, 0, 0);
	}

	EckInline UINT GetImeProperty(int iType) const
	{
		return (UINT)SendMsg(EM_GETIMEPROPERTY, iType, 0);
	}

	EckInline UINT GetLangOptions() const
	{
		return (UINT)SendMsg(EM_GETLANGOPTIONS, 0, 0);
	}

	EckInline HRESULT GetOleInterface(IRichEditOle** ppRichEditOle) const
	{
		return (HRESULT)SendMsg(EM_GETOLEINTERFACE, 0, (LPARAM)ppRichEditOle);
	}

	EckInline UINT GetOptions() const
	{
		return (UINT)SendMsg(EM_GETOPTIONS, 0, 0);
	}

	EckInline UINT GetPageRotate() const
	{
		return (UINT)SendMsg(EM_GETPAGEROTATE, 0, 0);
	}

	EckInline UINT GetParaFormat(PARAFORMAT2* ppf) const
	{
		return (UINT)SendMsg(EM_GETPARAFORMAT, 0, (LPARAM)ppf);
	}

	EckInline UNDONAMEID GetRedoName() const
	{
		return (UNDONAMEID)SendMsg(EM_GETREDONAME, 0, 0);
	}

	EckInline void GetScrollPos(POINT* ppt) const
	{
		SendMsg(EM_GETSCROLLPOS, 0, (LPARAM)ppt);
	}

	EckInline int GetSelText(PWSTR pszBuf) const
	{
		return (int)SendMsg(EM_GETSELTEXT, 0, (LPARAM)pszBuf);
	}

	EckInline int GetStoryType(int idxStory) const
	{
		return (int)SendMsg(EM_GETSTORYTYPE, idxStory, 0);
	}

	EckInline HRESULT GetTableParams(TABLEROWPARMS* ptrp, TABLECELLPARMS* ptcp) const
	{
		return (HRESULT)SendMsg(EM_GETTABLEPARMS, (WPARAM)ptrp, (LPARAM)ptcp);
	}

	EckInline int GetTextEx(GETTEXTEX* pgt, PWSTR pszBuf) const
	{
		return (int)SendMsg(EM_GETTEXTEX, (WPARAM)pgt, (LPARAM)pszBuf);
	}

	EckInline int GetTextLengthEx(GETTEXTLENGTHEX* pgtl) const
	{
		return (int)SendMsg(EM_GETTEXTLENGTHEX, (WPARAM)pgtl, 0);
	}

	EckInline TEXTMODE GetTextMode() const
	{
		return (TEXTMODE)SendMsg(EM_GETTEXTMODE, 0, 0);
	}

	EckInline int GetTextRange(TEXTRANGEW* pTextRange) const
	{
		return (int)SendMsg(EM_GETTEXTRANGE, 0, (LPARAM)pTextRange);
	}

	EckInline BOOL GetTouchOptions() const
	{
		return (BOOL)SendMsg(EM_GETTOUCHOPTIONS, RTO_SHOWHANDLES, 0);
	}

	EckInline UINT GetTypographyOptions() const
	{
		return (UINT)SendMsg(EM_GETTYPOGRAPHYOPTIONS, 0, 0);
	}

	EckInline UNDONAMEID GetUndoName() const
	{
		return (UNDONAMEID)SendMsg(EM_GETUNDONAME, 0, 0);
	}

	EckInline EDITWORDBREAKPROCEX GetWordBreakProc() const
	{
		return (EDITWORDBREAKPROCEX)SendMsg(EM_GETWORDBREAKPROCEX, 0, 0);
	}

	EckInline BOOL GetZoom(int* pnZoomNumerator, int* pnZoomDenominator) const
	{
		return (BOOL)SendMsg(EM_GETZOOM, (WPARAM)pnZoomNumerator, (LPARAM)pnZoomDenominator);
	}

	EckInline void HideSelection(BOOL bHide) const
	{
		SendMsg(EM_HIDESELECTION, bHide, 0);
	}

	EckInline HRESULT InsertImage(RICHEDIT_IMAGE_PARAMETERS* pImageParams) const
	{
		return (HRESULT)SendMsg(EM_INSERTIMAGE, 0, (LPARAM)pImageParams);
	}

	EckInline HRESULT InsertTable(TABLEROWPARMS* ptrp, TABLECELLPARMS* ptcp) const
	{
		return (HRESULT)SendMsg(EM_INSERTTABLE, (WPARAM)ptrp, (LPARAM)ptcp);
	}

	EckInline BOOL IsIme() const
	{
		return (BOOL)SendMsg(EM_ISIME, 0, 0);
	}

	EckInline void PasteSpecial(UINT uClipFormat, REPASTESPECIAL* prps) const
	{
		SendMsg(EM_PASTESPECIAL, uClipFormat, (LPARAM)prps);
	}

	EckInline void Reconversion() const
	{
		SendMsg(EM_RECONVERSION, 0, 0);
	}

	EckInline BOOL Redo() const
	{
		return (BOOL)SendMsg(EM_REDO, 0, 0);
	}

	EckInline void RequestResize() const
	{
		SendMsg(EM_REQUESTRESIZE, 0, 0);
	}

	EckInline UINT SelectionType() const
	{
		return (UINT)SendMsg(EM_SELECTIONTYPE, 0, 0);
	}

	EckInline BOOL SetAutoCorrectProc(AutoCorrectProc pfnNewProc) const
	{
		return (BOOL)SendMsg(EM_SETAUTOCORRECTPROC, (WPARAM)pfnNewProc, 0);
	}

	EckInline void SetBidiOptions(BIDIOPTIONS* pbidio) const
	{
		SendMsg(EM_SETBIDIOPTIONS, 0, (LPARAM)pbidio);
	}

	EckInline COLORREF SetBackgroundColor(BOOL bSysColor, COLORREF cr) const
	{
		return (COLORREF)SendMsg(EM_SETBKGNDCOLOR, bSysColor, cr);
	}

	EckInline BOOL SetCharFormat(int iFmt, CHARFORMAT2W* pcf) const
	{
		return (BOOL)SendMsg(EM_SETCHARFORMAT, iFmt, (LPARAM)pcf);
	}

	EckInline int SetCtfModeBias(int iModeBias) const
	{
		return (int)SendMsg(EM_SETCTFMODEBIAS, iModeBias, 0);
	}

	EckInline BOOL SetCtfOpenStatus(BOOL bOpen) const
	{
		return (BOOL)SendMsg(EM_SETCTFOPENSTATUS, bOpen, 0);
	}

	EckInline BOOL SetDisableOleLinkConversion(BOOL bDisable) const
	{
		return (BOOL)SendMsg(EM_SETDISABLEOLELINKCONVERSION, 0, bDisable);
	}

	EckInline UINT SetEditStyle(UINT dwStyle, UINT uMask) const
	{
		return (UINT)SendMsg(EM_SETEDITSTYLE, dwStyle, uMask);
	}

	EckInline UINT SetEditStyleEx(UINT dwStyleEx, UINT dwMask) const
	{
		return (UINT)SendMsg(EM_SETEDITSTYLEEX, dwStyleEx, dwMask);
	}

	EckInline BOOL SetEllipsisMode(DWORD dwElideMode) const
	{
		return (BOOL)SendMsg(EM_SETELLIPSISMODE, 0, dwElideMode);
	}

	EckInline UINT SetEventMask(UINT dwEventMask) const
	{
		return (UINT)SendMsg(EM_SETEVENTMASK, 0, dwEventMask);
	}

	EckInline BOOL SetFontSize(int iSize) const
	{
		return (BOOL)SendMsg(EM_SETFONTSIZE, iSize, 0);
	}

	EckInline void SetHyphenateInfo(HYPHENATEINFO* phr) const
	{
		SendMsg(EM_SETHYPHENATEINFO, (LPARAM)phr, 0);
	}

	EckInline int SetImeModeBias(int iModeBias) const
	{
		return (int)SendMsg(EM_SETIMEMODEBIAS, iModeBias, iModeBias);
	}

	EckInline void SetLangOptions(UINT dwOptions) const
	{
		SendMsg(EM_SETLANGOPTIONS, 0, dwOptions);
	}

	EckInline BOOL SetOleCallback(IRichEditOleCallback* pCallback) const
	{
		return (BOOL)SendMsg(EM_SETOLECALLBACK, 0, (LPARAM)pCallback);
	}

	EckInline UINT SetOptions(int iType, UINT dwOptions) const
	{
		return (UINT)SendMsg(EM_SETOPTIONS, iType, dwOptions);
	}

	EckInline BOOL SetParaFormat(PARAFORMAT2* ppf) const
	{
		return (BOOL)SendMsg(EM_SETPARAFORMAT, 0, (LPARAM)ppf);
	}

	EckInline int SetPageRotate(int iRotate) const
	{
		return (int)SendMsg(EM_SETPAGEROTATE, iRotate, 0);
	}

	EckInline void SetPalette(HPALETTE hPal) const
	{
		SendMsg(EM_SETPALETTE, (WPARAM)hPal, 0);
	}

	EckInline HRESULT SetQueryConvertOleLinkCallback(WPARAM Context,
		OLESTREAMQUERYCONVERTOLELINKCALLBACK pfnCallback) const
	{
		return (HRESULT)SendMsg(EM_SETQUERYCONVERTOLELINKCALLBACK,
			Context, (LPARAM)pfnCallback);
	}

	EckInline void SetScrollPos(POINT* ppt) const
	{
		SendMsg(EM_SETSCROLLPOS, 0, (LPARAM)ppt);
	}

	EckInline int SetStoryType(int idxStory, int iStoryType) const
	{
		return (int)SendMsg(EM_SETSTORYTYPE, idxStory, iStoryType);
	}

	EckInline HRESULT SetTableParams(TABLEROWPARMS* ptrp, TABLECELLPARMS* ptcp) const
	{
		return (HRESULT)SendMsg(EM_SETTABLEPARMS, (WPARAM)ptrp, (LPARAM)ptcp);
	}

	EckInline HRESULT SetTargetDevice(HDC hDC, int cxLine) const
	{
		return (HRESULT)SendMsg(EM_SETTARGETDEVICE, (WPARAM)hDC, cxLine);
	}

	EckInline int SetTextEx(SETTEXTEX* pst, PCWSTR pszText) const
	{
		EckAssert(pst->codepage == 1200);
		return (int)SendMsg(EM_SETTEXTEX, (WPARAM)pst, (LPARAM)pszText);
	}

	EckInline int SetTextEx(SETTEXTEX* pst, PCSTR pszText) const
	{
		EckAssert(pst->codepage != 1200);
		return (int)SendMsg(EM_SETTEXTEX, (WPARAM)pst, (LPARAM)pszText);
	}

	EckInline BOOL SetTextMode(TEXTMODE tm) const
	{
		return (BOOL)SendMsg(EM_SETTEXTMODE, tm, 0);
	}

	EckInline void SetTouchOptions(BOOL bShowHandles) const
	{
		SendMsg(EM_SETTOUCHOPTIONS, RTO_SHOWHANDLES, bShowHandles);
	}

	EckInline BOOL SetTypographyOptions(UINT dwOptions, UINT dwMask) const
	{
		return (BOOL)SendMsg(EM_SETTYPOGRAPHYOPTIONS, dwOptions, dwMask);
	}

	EckInline BOOL SetUiAutomationName(PCWSTR pszText) const
	{
		return (BOOL)SendMsg(EM_SETUIANAME, 0, (LPARAM)pszText);
	}

	EckInline int SetUndoLimit(int cLimit) const
	{
		return (int)SendMsg(EM_SETUNDOLIMIT, cLimit, 0);
	}

	EckInline EDITWORDBREAKPROCEX SetWordBreakProc(EDITWORDBREAKPROCEX pfnNewProc) const
	{
		return (EDITWORDBREAKPROCEX)SendMsg(EM_SETWORDBREAKPROCEX, 0, (LPARAM)pfnNewProc);
	}

	EckInline BOOL SetZoom(int iZoomNumerator, int iZoomDenominator) const
	{
		return (BOOL)SendMsg(EM_SETZOOM, iZoomNumerator, iZoomDenominator);
	}

	EckInline void ShowScrollBar(int iBar, BOOL bShow) const
	{
		SendMsg(EM_SHOWSCROLLBAR, iBar, bShow);
	}

	EckInline void StopGroupTyping() const
	{
		SendMsg(EM_STOPGROUPTYPING, 0, 0);
	}

	EckInline int StreamIn(UINT uFlags, EDITSTREAM* es) const
	{
		return (int)SendMsg(EM_STREAMIN, uFlags, (LPARAM)es);
	}

	EckInline int StreamOut(UINT uFlags, EDITSTREAM* es) const
	{
		return (int)SendMsg(EM_STREAMOUT, uFlags, (LPARAM)es);
	}
};
ECK_RTTI_IMPL_BASE_INLINE(CRichEdit, CWnd);
ECK_NAMESPACE_END