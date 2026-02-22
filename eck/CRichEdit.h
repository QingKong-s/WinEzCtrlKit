#pragma once
#include "CWindow.h"

#include <Richedit.h>
#include <Richole.h>

ECK_NAMESPACE_BEGIN
#define ECK_CWNDPROP_REE_STYLE(Name, Style)				\
	ECKPROP(StyleGet##Name, StyleSet##Name) BOOL Name;	\
	BOOL StyleGet##Name() const noexcept							\
	{													\
		if constexpr (Style == 0)						\
			return !GetEditStyle();						\
		else											\
			return IsBitSet(GetEditStyle(), Style);		\
	}													\
	void StyleSet##Name(BOOL b) const noexcept					\
	{													\
		SetEditStyle(b ? Style : 0, Style);				\
	}

#define ECK_CWNDPROP_REEEX_STYLE(Name, Style)			\
	ECKPROP(StyleGet##Name, StyleSet##Name) BOOL Name;	\
	BOOL StyleGet##Name() const noexcept							\
	{													\
		if constexpr (Style == 0)						\
			return !GetEditStyleEx();					\
		else											\
			return IsBitSet(GetEditStyleEx(), Style);	\
	}													\
	void StyleSet##Name(BOOL b) const noexcept					\
	{													\
		SetEditStyleEx(b ? Style : 0, Style);			\
	}

class CRichEdit : public CWindow
{
public:
    ECK_RTTI(CRichEdit, CWindow);
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
    EckInline HRESULT AutoUrlDetect(int iType, PCWSTR pszUrlPatterns = nullptr) const noexcept
    {
        return (HRESULT)SendMessage(EM_AUTOURLDETECT, iType, (LPARAM)pszUrlPatterns);
    }

    EckInline HRESULT CallAutoCorrectProcedure(WCHAR ch) const noexcept
    {
        return (HRESULT)SendMessage(EM_CALLAUTOCORRECTPROC, ch, 0);
    }

    EckInline HRESULT CanPaste(UINT uClipboardFormat) const noexcept
    {
        return (HRESULT)SendMessage(EM_CANPASTE, uClipboardFormat, 0);
    }

    EckInline HRESULT CanRedo() const noexcept
    {
        return (HRESULT)SendMessage(EM_CANREDO, 0, 0);
    }

    EckInline BOOL DisplayBand(RECT* prc) const noexcept
    {
        return (BOOL)SendMessage(EM_DISPLAYBAND, 0, (LPARAM)prc);
    }

    EckInline void GetSelection(CHARRANGE* pchrg) const noexcept
    {
        SendMessage(EM_EXGETSEL, 0, (LPARAM)pchrg);
    }

    EckInline void LimitText(int cch) const noexcept
    {
        SendMessage(EM_EXLIMITTEXT, 0, cch);
    }

    EckInline int LineFromChar(int ich) const noexcept
    {
        return (int)SendMessage(EM_EXLINEFROMCHAR, 0, ich);
    }

    EckInline LRESULT SetSelection(CHARRANGE* pchrg) const noexcept
    {
        return SendMessage(EM_EXSETSEL, 0, (LPARAM)pchrg);
    }

    EckInline int FindTextW(UINT uFlags, FINDTEXTW* pFindText) const noexcept
    {
        return (int)SendMessage(EM_FINDTEXTW, uFlags, (LPARAM)pFindText);
    }

    EckInline int FindTextEx(UINT uFlags, FINDTEXTEXW* pFindText) const noexcept
    {
        return (int)SendMessage(EM_FINDTEXTEXW, uFlags, (LPARAM)pFindText);
    }

    EckInline int FindWordBreak(UINT uFlags, int idxChar) const noexcept
    {
        return (int)SendMessage(EM_FINDWORDBREAK, uFlags, idxChar);
    }

    EckInline int FormatRange(BOOL bInPlace, FORMATRANGE* pfr) const noexcept
    {
        return (int)SendMessage(EM_FORMATRANGE, bInPlace, (LPARAM)pfr);
    }

    EckInline AutoCorrectProc GetAutoCorrectProcedure() const noexcept
    {
        return (AutoCorrectProc)SendMessage(EM_GETAUTOCORRECTPROC, 0, 0);
    }

    EckInline BOOL GetAutoUrlDetect() const noexcept
    {
        return (BOOL)SendMessage(EM_GETAUTOURLDETECT, 0, 0);
    }

    EckInline void GetBidiOptions(BIDIOPTIONS* pbidio) const noexcept
    {
        SendMessage(EM_GETBIDIOPTIONS, 0, (LPARAM)pbidio);
    }

    EckInline UINT GetCharFormat(int iRange, CHARFORMAT2W* pcf) const noexcept
    {
        return (UINT)SendMessage(EM_GETCHARFORMAT, iRange, (LPARAM)pcf);
    }

    EckInline int GetCtfModeBias() const noexcept
    {
        return (int)SendMessage(EM_GETCTFMODEBIAS, 0, 0);
    }

    EckInline BOOL GetCtfOpenStatus() const noexcept
    {
        return (BOOL)SendMessage(EM_GETCTFOPENSTATUS, 0, 0);
    }

    EckInline UINT GetEditStyle() const noexcept
    {
        return (UINT)SendMessage(EM_GETEDITSTYLE, 0, 0);
    }

    EckInline UINT GetEditStyleEx() const noexcept
    {
        return (UINT)SendMessage(EM_GETEDITSTYLEEX, 0, 0);
    }

    EckInline UINT GetEllipsisMode() const noexcept
    {
        UINT dw;
        SendMessage(EM_GETELLIPSISMODE, 0, (LPARAM)&dw);
        return dw;
    }

    EckInline BOOL GetEllipsisState() const noexcept
    {
        return (BOOL)SendMessage(EM_GETELLIPSISSTATE, 0, 0);
    }

    EckInline UINT GetEventMask() const noexcept
    {
        return (UINT)SendMessage(EM_GETEVENTMASK, 0, 0);
    }

    EckInline void GetHyphenateInfomation(HYPHRESULT* phr) const noexcept
    {
        SendMessage(EM_GETHYPHENATEINFO, (WPARAM)phr, 0);
    }

    EckInline int GetImeCompositionMode() const noexcept
    {
        return (int)SendMessage(EM_GETIMECOMPMODE, 0, 0);
    }

    EckInline int GetImeCompositionText(IMECOMPTEXT* pict, PWSTR pszBuf) const noexcept
    {
        return (int)SendMessage(EM_GETIMECOMPTEXT, (WPARAM)pict, (LPARAM)pszBuf);
    }

    EckInline int GetImeModeBias() const noexcept
    {
        return (int)SendMessage(EM_GETIMEMODEBIAS, 0, 0);
    }

    EckInline UINT GetImeProperty(int iType) const noexcept
    {
        return (UINT)SendMessage(EM_GETIMEPROPERTY, iType, 0);
    }

    EckInline UINT GetLangOptions() const noexcept
    {
        return (UINT)SendMessage(EM_GETLANGOPTIONS, 0, 0);
    }

    EckInline HRESULT GetOleInterface(IRichEditOle** ppRichEditOle) const noexcept
    {
        return (HRESULT)SendMessage(EM_GETOLEINTERFACE, 0, (LPARAM)ppRichEditOle);
    }

    EckInline UINT GetOptions() const noexcept
    {
        return (UINT)SendMessage(EM_GETOPTIONS, 0, 0);
    }

    EckInline UINT GetPageRotate() const noexcept
    {
        return (UINT)SendMessage(EM_GETPAGEROTATE, 0, 0);
    }

    EckInline UINT GetParagraphFormat(PARAFORMAT2* ppf) const noexcept
    {
        return (UINT)SendMessage(EM_GETPARAFORMAT, 0, (LPARAM)ppf);
    }

    EckInline UNDONAMEID GetRedoName() const noexcept
    {
        return (UNDONAMEID)SendMessage(EM_GETREDONAME, 0, 0);
    }

    EckInline void GetScrollPosition(POINT* ppt) const noexcept
    {
        SendMessage(EM_GETSCROLLPOS, 0, (LPARAM)ppt);
    }

    EckInline int GetSelectedText(PWSTR pszBuf) const noexcept
    {
        return (int)SendMessage(EM_GETSELTEXT, 0, (LPARAM)pszBuf);
    }

    EckInline int GetStoryType(int idxStory) const noexcept
    {
        return (int)SendMessage(EM_GETSTORYTYPE, idxStory, 0);
    }

    EckInline HRESULT GetTableParameters(TABLEROWPARMS* ptrp, TABLECELLPARMS* ptcp) const noexcept
    {
        return (HRESULT)SendMessage(EM_GETTABLEPARMS, (WPARAM)ptrp, (LPARAM)ptcp);
    }

    EckInline int GetTextEx(GETTEXTEX* pgt, PWSTR pszBuf) const noexcept
    {
        return (int)SendMessage(EM_GETTEXTEX, (WPARAM)pgt, (LPARAM)pszBuf);
    }

    EckInline int GetTextLengthEx(GETTEXTLENGTHEX* pgtl) const noexcept
    {
        return (int)SendMessage(EM_GETTEXTLENGTHEX, (WPARAM)pgtl, 0);
    }

    EckInline TEXTMODE GetTextMode() const noexcept
    {
        return (TEXTMODE)SendMessage(EM_GETTEXTMODE, 0, 0);
    }

    EckInline int GetTextRange(TEXTRANGEW* pTextRange) const noexcept
    {
        return (int)SendMessage(EM_GETTEXTRANGE, 0, (LPARAM)pTextRange);
    }

    EckInline BOOL GetTouchOptions() const noexcept
    {
        return (BOOL)SendMessage(EM_GETTOUCHOPTIONS, RTO_SHOWHANDLES, 0);
    }

    EckInline UINT GetTypographyOptions() const noexcept
    {
        return (UINT)SendMessage(EM_GETTYPOGRAPHYOPTIONS, 0, 0);
    }

    EckInline UNDONAMEID GetUndoName() const noexcept
    {
        return (UNDONAMEID)SendMessage(EM_GETUNDONAME, 0, 0);
    }

    EckInline EDITWORDBREAKPROCEX GetWordBreakProcedure() const noexcept
    {
        return (EDITWORDBREAKPROCEX)SendMessage(EM_GETWORDBREAKPROCEX, 0, 0);
    }

    EckInline BOOL GetZoom(int* pnZoomNumerator, int* pnZoomDenominator) const noexcept
    {
        return (BOOL)SendMessage(EM_GETZOOM, (WPARAM)pnZoomNumerator, (LPARAM)pnZoomDenominator);
    }

    EckInline void HideSelection(BOOL bHide) const noexcept
    {
        SendMessage(EM_HIDESELECTION, bHide, 0);
    }

    EckInline HRESULT InsertImage(RICHEDIT_IMAGE_PARAMETERS* pImageParams) const noexcept
    {
        return (HRESULT)SendMessage(EM_INSERTIMAGE, 0, (LPARAM)pImageParams);
    }

    EckInline HRESULT InsertTable(TABLEROWPARMS* ptrp, TABLECELLPARMS* ptcp) const noexcept
    {
        return (HRESULT)SendMessage(EM_INSERTTABLE, (WPARAM)ptrp, (LPARAM)ptcp);
    }

    EckInline BOOL IsIme() const noexcept
    {
        return (BOOL)SendMessage(EM_ISIME, 0, 0);
    }

    EckInline void PasteSpecial(UINT uClipFormat, REPASTESPECIAL* prps) const noexcept
    {
        SendMessage(EM_PASTESPECIAL, uClipFormat, (LPARAM)prps);
    }

    EckInline void Reconversion() const noexcept
    {
        SendMessage(EM_RECONVERSION, 0, 0);
    }

    EckInline BOOL Redo() const noexcept
    {
        return (BOOL)SendMessage(EM_REDO, 0, 0);
    }

    EckInline void RequestResize() const noexcept
    {
        SendMessage(EM_REQUESTRESIZE, 0, 0);
    }

    EckInline UINT SelectionType() const noexcept
    {
        return (UINT)SendMessage(EM_SELECTIONTYPE, 0, 0);
    }

    EckInline BOOL SetAutoCorrectProcedure(AutoCorrectProc pfnNewProc) const noexcept
    {
        return (BOOL)SendMessage(EM_SETAUTOCORRECTPROC, (WPARAM)pfnNewProc, 0);
    }

    EckInline void SetBidiOptions(BIDIOPTIONS* pbidio) const noexcept
    {
        SendMessage(EM_SETBIDIOPTIONS, 0, (LPARAM)pbidio);
    }

    EckInline COLORREF SetBackgroundColor(BOOL bSysColor, COLORREF cr) const noexcept
    {
        return (COLORREF)SendMessage(EM_SETBKGNDCOLOR, bSysColor, cr);
    }

    EckInline BOOL SetCharFormat(int iFmt, CHARFORMAT2W* pcf) const noexcept
    {
        return (BOOL)SendMessage(EM_SETCHARFORMAT, iFmt, (LPARAM)pcf);
    }

    EckInline int SetCtfModeBias(int iModeBias) const noexcept
    {
        return (int)SendMessage(EM_SETCTFMODEBIAS, iModeBias, 0);
    }

    EckInline BOOL SetCtfOpenStatus(BOOL bOpen) const noexcept
    {
        return (BOOL)SendMessage(EM_SETCTFOPENSTATUS, bOpen, 0);
    }

    EckInline BOOL SetDisableOleLinkConversion(BOOL bDisable) const noexcept
    {
        return (BOOL)SendMessage(EM_SETDISABLEOLELINKCONVERSION, 0, bDisable);
    }

    EckInline UINT SetEditStyle(UINT dwStyle, UINT uMask) const noexcept
    {
        return (UINT)SendMessage(EM_SETEDITSTYLE, dwStyle, uMask);
    }

    EckInline UINT SetEditStyleEx(UINT dwStyleEx, UINT dwMask) const noexcept
    {
        return (UINT)SendMessage(EM_SETEDITSTYLEEX, dwStyleEx, dwMask);
    }

    EckInline BOOL SetEllipsisMode(UINT uEllipsisMode) const noexcept
    {
        return (BOOL)SendMessage(EM_SETELLIPSISMODE, 0, uEllipsisMode);
    }

    EckInline UINT SetEventMask(UINT dwEventMask) const noexcept
    {
        return (UINT)SendMessage(EM_SETEVENTMASK, 0, dwEventMask);
    }

    EckInline BOOL SetFontSize(int iSize) const noexcept
    {
        return (BOOL)SendMessage(EM_SETFONTSIZE, iSize, 0);
    }

    EckInline void SetHyphenateInfomation(HYPHENATEINFO* phr) const noexcept
    {
        SendMessage(EM_SETHYPHENATEINFO, (LPARAM)phr, 0);
    }

    EckInline int SetImeModeBias(int iModeBias) const noexcept
    {
        return (int)SendMessage(EM_SETIMEMODEBIAS, iModeBias, iModeBias);
    }

    EckInline void SetLangOptions(UINT dwOptions) const noexcept
    {
        SendMessage(EM_SETLANGOPTIONS, 0, dwOptions);
    }

    EckInline BOOL SetOleCallback(IRichEditOleCallback* pCallback) const noexcept
    {
        return (BOOL)SendMessage(EM_SETOLECALLBACK, 0, (LPARAM)pCallback);
    }

    EckInline UINT SetOptions(int iType, UINT dwOptions) const noexcept
    {
        return (UINT)SendMessage(EM_SETOPTIONS, iType, dwOptions);
    }

    EckInline BOOL SetParagraphFormat(PARAFORMAT2* ppf) const noexcept
    {
        return (BOOL)SendMessage(EM_SETPARAFORMAT, 0, (LPARAM)ppf);
    }

    EckInline int SetPageRotate(int iRotate) const noexcept
    {
        return (int)SendMessage(EM_SETPAGEROTATE, iRotate, 0);
    }

    EckInline void SetPalette(HPALETTE hPal) const noexcept
    {
        SendMessage(EM_SETPALETTE, (WPARAM)hPal, 0);
    }

    EckInline HRESULT SetQueryConvertOleLinkCallback(WPARAM Context,
        OLESTREAMQUERYCONVERTOLELINKCALLBACK pfnCallback) const noexcept
    {
        return (HRESULT)SendMessage(EM_SETQUERYCONVERTOLELINKCALLBACK,
            Context, (LPARAM)pfnCallback);
    }

    EckInline void SetScrollPosition(POINT* ppt) const noexcept
    {
        SendMessage(EM_SETSCROLLPOS, 0, (LPARAM)ppt);
    }

    EckInline int SetStoryType(int idxStory, int iStoryType) const noexcept
    {
        return (int)SendMessage(EM_SETSTORYTYPE, idxStory, iStoryType);
    }

    EckInline HRESULT SetTableParameters(TABLEROWPARMS* ptrp, TABLECELLPARMS* ptcp) const noexcept
    {
        return (HRESULT)SendMessage(EM_SETTABLEPARMS, (WPARAM)ptrp, (LPARAM)ptcp);
    }

    EckInline HRESULT SetTargetDevice(HDC hDC, int cxLine) const noexcept
    {
        return (HRESULT)SendMessage(EM_SETTARGETDEVICE, (WPARAM)hDC, cxLine);
    }

    EckInline int SetTextEx(SETTEXTEX* pst, PCWSTR pszText) const noexcept
    {
        EckAssert(pst->codepage == 1200);
        return (int)SendMessage(EM_SETTEXTEX, (WPARAM)pst, (LPARAM)pszText);
    }

    EckInline int SetTextEx(SETTEXTEX* pst, PCSTR pszText) const noexcept
    {
        EckAssert(pst->codepage != 1200);
        return (int)SendMessage(EM_SETTEXTEX, (WPARAM)pst, (LPARAM)pszText);
    }

    EckInline BOOL SetTextMode(TEXTMODE tm) const noexcept
    {
        return (BOOL)SendMessage(EM_SETTEXTMODE, tm, 0);
    }

    EckInline void SetTouchOptions(BOOL bShowHandles) const noexcept
    {
        SendMessage(EM_SETTOUCHOPTIONS, RTO_SHOWHANDLES, bShowHandles);
    }

    EckInline BOOL SetTypographyOptions(UINT dwOptions, UINT dwMask) const noexcept
    {
        return (BOOL)SendMessage(EM_SETTYPOGRAPHYOPTIONS, dwOptions, dwMask);
    }

    EckInline BOOL SetUiAutomationName(PCWSTR pszText) const noexcept
    {
        return (BOOL)SendMessage(EM_SETUIANAME, 0, (LPARAM)pszText);
    }

    EckInline int SetUndoLimit(int cLimit) const noexcept
    {
        return (int)SendMessage(EM_SETUNDOLIMIT, cLimit, 0);
    }

    EckInline EDITWORDBREAKPROCEX SetWordBreakProcedure(EDITWORDBREAKPROCEX pfnNewProc) const noexcept
    {
        return (EDITWORDBREAKPROCEX)SendMessage(EM_SETWORDBREAKPROCEX, 0, (LPARAM)pfnNewProc);
    }

    EckInline BOOL SetZoom(int iZoomNumerator, int iZoomDenominator) const noexcept
    {
        return (BOOL)SendMessage(EM_SETZOOM, iZoomNumerator, iZoomDenominator);
    }

    EckInline void ShowScrollBar(int iBar, BOOL bShow) const noexcept
    {
        SendMessage(EM_SHOWSCROLLBAR, iBar, bShow);
    }

    EckInline void StopGroupTyping() const noexcept
    {
        SendMessage(EM_STOPGROUPTYPING, 0, 0);
    }

    EckInline int StreamIn(UINT uFlags, EDITSTREAM* es) const noexcept
    {
        return (int)SendMessage(EM_STREAMIN, uFlags, (LPARAM)es);
    }

    EckInline int StreamOut(UINT uFlags, EDITSTREAM* es) const noexcept
    {
        return (int)SendMessage(EM_STREAMOUT, uFlags, (LPARAM)es);
    }
};
ECK_NAMESPACE_END