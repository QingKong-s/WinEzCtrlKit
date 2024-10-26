/*
* WinEzCtrlKit Library
*
* CEdit.h ： 标准编辑框
*
* Copyright(C) 2023-2024 QingKong
*/
#pragma once
#include "CWnd.h"

ECK_NAMESPACE_BEGIN
inline constexpr int CDV_EDIT_1 = 0;

#pragma pack(push, ECK_CTRLDATA_ALIGN)
struct CTRLDATA_EDIT
{
	int iVer;
	WCHAR chPassword;
	ECKENUM eTransMode;
	int iSelStart;
	int iSelEnd;
	int cchCueBanner;
	int cchLimit;
	// WCHAR szCueBanner[];

	EckInline PCWSTR CueBanner() const
	{
		if (cchCueBanner)
			return PCWSTR(this + 1);
		else
			return nullptr;
	}
};
#pragma pack(pop)

#if NTDDI_VERSION >= NTDDI_WIN10_RS5// 1809+
#define ECK_CWNDPROP_EDE_STYLE(Name, Style)				\
	ECKPROP(StyleGet##Name, StyleSet##Name) BOOL Name;	\
	BOOL StyleGet##Name() const							\
	{													\
		if constexpr (Style == 0)						\
			return !!GetEDExtendStyle();				\
		else											\
			return IsBitSet(GetEDExtendStyle(), Style);	\
	}													\
	void StyleSet##Name(BOOL b) const					\
	{													\
		SetEDExtendStyle(b ? Style : 0, Style);			\
	}
#endif// NTDDI_VERSION >= NTDDI_WIN10_RS5

class CEdit :public CWnd
{
public:
	ECK_RTTI(CEdit);
	ECK_CWND_NOSINGLEOWNER(CEdit);
	ECK_CWND_CREATE_CLS(WC_EDITW);

	enum class TransMode
	{
		None,
		ToLowerCase,
		ToUpperCase
	};

	ECK_CWNDPROP_STYLE(AutoHScroll, ES_AUTOHSCROLL);
	ECK_CWNDPROP_STYLE(AutoVScroll, ES_AUTOVSCROLL);
	ECK_CWNDPROP_STYLE(Center, ES_CENTER);
	ECK_CWNDPROP_STYLE(Left, ES_LEFT);
	ECK_CWNDPROP_STYLE(Lowercase, ES_LOWERCASE);
	ECK_CWNDPROP_STYLE(Multiline, ES_MULTILINE);
	ECK_CWNDPROP_STYLE(NoHideSel, ES_NOHIDESEL);
	ECK_CWNDPROP_STYLE(Number, ES_NUMBER);
	ECK_CWNDPROP_STYLE(OemConvert, ES_OEMCONVERT);
	ECK_CWNDPROP_STYLE(Password, ES_PASSWORD);
	ECK_CWNDPROP_STYLE(ReadOnly, ES_READONLY);
	ECK_CWNDPROP_STYLE(Right, ES_RIGHT);
	ECK_CWNDPROP_STYLE(Uppercase, ES_UPPERCASE);
	ECK_CWNDPROP_STYLE(WantReturn, ES_WANTRETURN);
#if NTDDI_VERSION >= NTDDI_WIN10_RS5// 1809+
	ECK_CWNDPROP_EDE_STYLE(AllowEolCR, ES_EX_ALLOWEOL_CR);
	ECK_CWNDPROP_EDE_STYLE(AllowEolLF, ES_EX_ALLOWEOL_LF);
	ECK_CWNDPROP_EDE_STYLE(AllowEolAll, ES_EX_ALLOWEOL_ALL);
	ECK_CWNDPROP_EDE_STYLE(ConvertEolOnPaste, ES_EX_CONVERT_EOL_ON_PASTE);
	ECK_CWNDPROP_EDE_STYLE(Zoomable, ES_EX_ZOOMABLE);
#endif// NTDDI_VERSION >= NTDDI_WIN10_RS5

	EckInline constexpr static PCVOID SkipBaseData(PCVOID p)
	{
		return (PCBYTE)p + sizeof(CTRLDATA_EDIT) +
			(((const CTRLDATA_EDIT*)p)->cchCueBanner + 1) * sizeof(WCHAR);
	}

	void SerializeData(CRefBin& rb, const SERIALIZE_OPT* pOpt = nullptr) override
	{
		const auto rsCueBanner = GetCueBanner((pOpt && pOpt->cchTextBuf ?
			pOpt->cchTextBuf : MAX_PATH));
		const SIZE_T cbSize = sizeof(CTRLDATA_EDIT) + rsCueBanner.ByteSize();
		CWnd::SerializeData(rb, pOpt);
		const auto p = (CTRLDATA_EDIT*)rb.PushBack(cbSize);
		p->iVer = CDV_EDIT_1;
		p->chPassword = GetPasswordChar();
		p->eTransMode = (ECKENUM)GetTransformMode();
		GetSel(&p->iSelStart, &p->iSelEnd);
		p->cchCueBanner = rsCueBanner.Size();
		p->cchLimit = GetLimitText();
		if (p->cchCueBanner)
			wmemcpy(PWSTR(p + 1), rsCueBanner.Data(), rsCueBanner.Size() + 1);
		else
			*PWSTR(p + 1) = L'\0';
	}

	void PostDeserialize(PCVOID pData) override
	{
		__super::PostDeserialize(pData);
		const auto* const p = (CTRLDATA_EDIT*)__super::SkipBaseData(pData);
		if (p->iVer != CDV_EDIT_1)
			return;
		SetPasswordChar(p->chPassword);
		SetTransformMode((TransMode)p->eTransMode);
		SetSel(p->iSelStart, p->iSelEnd);
		SetCueBanner(p->CueBanner(), TRUE);
		SetLimitText(p->cchLimit);
	}

	EckInline BOOL CanUndo() const
	{
		return (BOOL)SendMsg(EM_CANUNDO, 0, 0);
	}

	/// <summary>
	/// 取坐标处字符
	/// </summary>
	/// <param name="pt">坐标，相对客户区</param>
	/// <param name="piPosInLine">接收行中位置变量，可为NULL，失败设置为-1</param>
	/// <returns>返回字符位置，失败返回-1</returns>
	int CharFromPos(POINT pt, int* piPosInLine = nullptr) const
	{
		int posChar;
		DWORD dwRet = (DWORD)SendMsg(EM_CHARFROMPOS, 0, MAKELPARAM(pt.x, pt.y));
		USHORT usPos = LOWORD(dwRet);
		if (usPos == 65535)
			posChar = -1;
		else
			posChar = usPos;
		if (piPosInLine)
		{
			usPos = HIWORD(dwRet);
			if (usPos == 65535)
				*piPosInLine = -1;
			else
				*piPosInLine = usPos;
		}
		return posChar;
	}

	EckInline void EmptyUndoBuffer() const
	{
		SendMsg(EM_EMPTYUNDOBUFFER, 0, 0);
	}

	EckInline void FmtLines(BOOL bSoftLineBreakChar) const
	{
		SendMsg(EM_FMTLINES, bSoftLineBreakChar, 0);
	}

	/// <summary>
	/// 取提示横幅
	/// </summary>
	/// <param name="pszBuf">缓冲区指针</param>
	/// <param name="cchBuf">缓冲区大小</param>
	/// <returns>成功返回TRUE，失败返回FALSE</returns>
	EckInline BOOL GetCueBanner(PWSTR pszBuf, int cchBuf) const
	{
		return (BOOL)SendMsg(EM_GETCUEBANNER, (WPARAM)pszBuf, cchBuf);
	}

	EckInline BOOL GetCueBanner(CRefStrW& rs, int cchMax = MAX_PATH,
		BOOL bReCalcLen = TRUE) const
	{
		rs.ReSize(cchMax);
		const auto bRet = GetCueBanner(rs.Data(), rs.Size() + 1);
		if (!bRet)
		{
			rs.Clear();
			return FALSE;
		}
		if (bReCalcLen)
			rs.ReCalcLen();
		return bRet;
	}

	// For compatibility
	EckInline CRefStrW GetCueBanner(int cchMax = MAX_PATH, BOOL bReCalcLen = TRUE) const
	{
		CRefStrW rs;
		GetCueBanner(rs, cchMax, bReCalcLen);
		return rs;
	}

	/// <summary>
	/// 取第一可见行
	/// </summary>
	/// <returns>多行：返回行索引，单行：返回第一个可见字符索引</returns>
	EckInline int GetFirstVisibleLine() const
	{
		return (int)SendMsg(EM_GETFIRSTVISIBLELINE, 0, 0);
	}

	EckInline HLOCAL GetHandle() const
	{
		return (HLOCAL)SendMsg(EM_GETHANDLE, 0, 0);
	}

	EckInline UINT GetImeStatus(UINT uMask) const
	{
		return (UINT)SendMsg(EM_GETIMESTATUS, uMask, 0);
	}

	EckInline int GetLimitText() const
	{
		return (int)SendMsg(EM_GETLIMITTEXT, 0, 0);
	}

	int GetLine(int idxLine, PWSTR pszBuf, int cchMax) const
	{
		if (!pszBuf || cchMax < 1)
			return 0;
		*(WORD*)pszBuf = cchMax - 1;
		int cch = (int)SendMsg(EM_GETLINE, idxLine, (LPARAM)pszBuf);
		*(pszBuf + cch) = L'\0';
		return cch;
	}

	void GetLine(CRefStrW& rs, int posChar) const
	{
		rs.ReSize(LineLength(posChar));
		if (rs.IsEmpty())
			return;
		GetLine(LineFromChar(posChar), rs.Data(), rs.Size() + 1);
	}

	// For compatibility
	CRefStrW GetLine(int posChar) const
	{
		CRefStrW rs;
		GetLine(rs, posChar);
		return rs;
	}

	EckInline int GetLineCount() const
	{
		return (int)SendMsg(EM_GETLINECOUNT, 0, 0);
	}

	EckInline DWORD GetMargins() const
	{
		return (DWORD)SendMsg(EM_GETMARGINS, 0, 0);
	}

	EckInline void GetMargins(int* piLeftMargin, int* piRightMargin) const
	{
		DWORD dwRet = (DWORD)SendMsg(EM_GETMARGINS, 0, 0);
		if (piLeftMargin)
			*piLeftMargin = LOWORD(dwRet);
		if (piRightMargin)
			*piRightMargin = HIWORD(dwRet);
	}

	EckInline int GetModify() const
	{
		return (int)SendMsg(EM_GETMODIFY, 0, 0);
	}

	EckInline WCHAR GetPasswordChar() const
	{
		return (WCHAR)SendMsg(EM_GETPASSWORDCHAR, 0, 0);
	}

	EckInline void GetRect(RECT* prc) const
	{
		SendMsg(EM_GETRECT, 0, (LPARAM)prc);
	}

	EckInline void GetSel(int* piSelStart, int* piSelEnd) const
	{
		SendMsg(EM_GETSEL, (WPARAM)&piSelStart, (LPARAM)piSelEnd);
	}

	/// <summary>
	/// 取垂直滚动条位置
	/// </summary>
	/// <returns>位置</returns>
	EckInline int GetThumb() const
	{
		return (int)SendMsg(EM_GETTHUMB, 0, 0);
	}

	EckInline EDITWORDBREAKPROCW GetWordBreakProc() const
	{
		return (EDITWORDBREAKPROCW)SendMsg(EM_GETWORDBREAKPROC, 0, 0);
	}

	EckInline void HideBalloonTip() const
	{
		SendMsg(EM_HIDEBALLOONTIP, 0, 0);
	}

	/// <summary>
	/// 字符位置到行数
	/// </summary>
	/// <param name="posChar">字符索引，若设为-1则返回当前光标所在行，或者返回选定内容所在行（如果有）</param>
	/// <returns>返回行索引</returns>
	EckInline int LineFromChar(int posChar) const
	{
		return (int)SendMsg(EM_LINEFROMCHAR, posChar, 0);
	}

	/// <summary>
	/// 取某行第一字符位置
	/// </summary>
	/// <param name="iLine">行索引，若设为-1则指定当前光标所在行</param>
	/// <returns>返回一行的第一个字符的索引</returns>
	EckInline int LineIndex(int iLine) const
	{
		return (int)SendMsg(EM_LINEINDEX, iLine, 0);
	}

	/// <summary>
	/// 取某行长度
	/// </summary>
	/// <param name="posChar">行中字符位置</param>
	/// <returns>返回长度，失败返回0</returns>
	EckInline int LineLength(int posChar) const
	{
		return (int)SendMsg(EM_LINELENGTH, posChar, 0);
	}

	EckInline void LineScroll(int cchHScroll, int cchVScroll) const
	{
		SendMsg(EM_LINESCROLL, cchHScroll, cchVScroll);
	}

	EckInline POINT PosFromChar(int posChar) const
	{
		DWORD dwRet = (DWORD)SendMsg(EM_LINESCROLL, posChar, 0);
		POINT pt;
		pt.x = LOWORD(dwRet);
		pt.y = HIWORD(dwRet);
		return pt;
	}

	EckInline void ReplaceSel(PCWSTR pszText, BOOL bAllowUndo) const
	{
		SendMsg(EM_REPLACESEL, bAllowUndo, (LPARAM)pszText);
	}

	/// <summary>
	/// 滚动
	/// </summary>
	/// <param name="iOp">滚动操作，可选常量值：SB_LINEDOWN、SB_LINEUP、SB_PAGEDOWN、SB_PAGEUP</param>
	/// <returns>成功返回TRUE</returns>
	EckInline BOOL Scroll(int iOp) const
	{
		return HIWORD(SendMsg(EM_SCROLL, iOp, 0));
	}

	EckInline void ScrollCaret() const
	{
		SendMsg(EM_SCROLLCARET, 0, 0);
	}

	EckInline BOOL SetCueBanner(PCWSTR psz, BOOL bShowAlways) const
	{
		return (BOOL)SendMsg(EM_SETCUEBANNER, bShowAlways, (LPARAM)psz);
	}

	EckInline void SetHandle(HLOCAL hLocal) const
	{
		SendMsg(EM_SETHANDLE, (WPARAM)hLocal, 0);
	}

	EckInline UINT SetImeStatus(UINT uMask) const
	{
		return (UINT)SendMsg(EM_SETIMESTATUS, uMask, 0);
	}

	EckInline void SetLimitText(int iMaxLen) const
	{
		SendMsg(EM_SETLIMITTEXT, iMaxLen, 0);
	}

	/// <summary>
	/// 置边距
	/// </summary>
	/// <param name="iLeftMargin">左边距，可以为EC_USEFONTINFO</param>
	/// <param name="iRightMargin">右边距，可以为EC_USEFONTINFO</param>
	/// <param name="uMask">EC_LEFTMARGIN、EC_RIGHTMARGIN</param>
	EckInline void SetMargins(int iLeftMargin, int iRightMargin, UINT uMask) const
	{
		SendMsg(EM_SETMARGINS, uMask, MAKELPARAM(iLeftMargin, iRightMargin));
	}

	EckInline void SetModify(BOOL bModify) const
	{
		SendMsg(EM_SETMODIFY, bModify, 0);
	}

	EckInline void SetPasswordChar(WCHAR chMask) const
	{
		SendMsg(EM_SETPASSWORDCHAR, chMask, 0);
	}

	EckInline BOOL SetReadOnly(BOOL bReadOnly) const
	{
		return (BOOL)SendMsg(EM_SETREADONLY, bReadOnly, 0);
	}

	EckInline void SetRect(RECT* prc) const
	{
		SendMsg(EM_SETRECT, 0, (LPARAM)prc);
	}

	EckInline void SetRectNp(RECT* prc) const
	{
		SendMsg(EM_SETRECTNP, 0, (LPARAM)prc);
	}

	EckInline void SetSel(int iSelStart, int iSelEnd) const
	{
		SendMsg(EM_SETSEL, iSelStart, iSelEnd);
	}

	/// <summary>
	/// 置制表位
	/// </summary>
	/// <param name="piTabStop">制表位数组</param>
	/// <param name="c">数组元素数，若为0则设置默认制表位</param>
	EckInline void SetTabStop(int* piTabStop, int c) const
	{
		SendMsg(EM_SETTABSTOPS, c, (LPARAM)piTabStop);
	}

	EckInline void SetWordBreakProc(EDITWORDBREAKPROCW pfnProc) const
	{
		SendMsg(EM_SETWORDBREAKPROC, 0, (LPARAM)pfnProc);
	}

	/// <summary>
	/// 弹出气球提示
	/// </summary>
	/// <param name="pszCaption">标题</param>
	/// <param name="pszContent">文本</param>
	/// <param name="iIcon">图标类型，TTI_常量</param>
	/// <returns>成功返回TRUE</returns>
	EckInline BOOL ShowBalloonTip(PCWSTR pszCaption, PCWSTR pszContent, int iIcon) const
	{
		EDITBALLOONTIP ebt;
		ebt.cbStruct = sizeof(EDITBALLOONTIP);
		ebt.pszTitle = pszCaption;
		ebt.pszText = pszContent;
		ebt.ttiIcon = iIcon;

		return (BOOL)SendMsg(EM_SHOWBALLOONTIP, 0, (LPARAM)&ebt);
	}

	EckInline BOOL ShowBalloonTip(const EDITBALLOONTIP* pebt) const
	{
		return (BOOL)SendMsg(EM_SHOWBALLOONTIP, 0, (LPARAM)pebt);
	}

#if NTDDI_VERSION >= NTDDI_WIN10_RS5// 1809+
	EckInline HRESULT SetEDExtendStyle(DWORD dwStyle, DWORD dwMask) const
	{
		return (HRESULT)SendMsg(EM_SETEXTENDEDSTYLE, dwMask, dwStyle);
	}

	EckInline DWORD GetEDExtendStyle() const
	{
		return (DWORD)SendMsg(EM_GETEXTENDEDSTYLE, 0, 0);
	}

	EckInline BOOL SetEndOfLine(EC_ENDOFLINE Eol) const
	{
		return (BOOL)SendMsg(EM_SETENDOFLINE, (WPARAM)Eol, 0);
	}

	EckInline EC_ENDOFLINE GetEndOfLine() const
	{
		return (EC_ENDOFLINE)SendMsg(EM_GETENDOFLINE, 0, 0);
	}

	EckInline void EnableSearchWeb(BOOL bEnable) const
	{
		SendMsg(EM_ENABLESEARCHWEB, bEnable, 0);
	}

	EckInline void SearchWeb() const
	{
		SendMsg(EM_SEARCHWEB, 0, 0);
	}

	EckInline void SetCaretIndex(int idx) const
	{
		SendMsg(EM_SETCARETINDEX, idx, 0);
	}

	EckInline int GetCaretIndex() const
	{
		return (int)SendMsg(EM_GETCARETINDEX, 0, 0);
	}

	EckInline BOOL SetZoom(int nZoomNumerator, int nZoomDenominator) const
	{
		return (BOOL)SendMsg(EM_SETZOOM, nZoomNumerator, nZoomDenominator);
	}

	EckInline BOOL GetZoom(int* pnZoomNumerator, int* pnZoomDenominator) const
	{
		return (BOOL)SendMsg(EM_GETZOOM, (WPARAM)pnZoomNumerator, (LPARAM)pnZoomDenominator);
	}

	EckInline int FileLineFromChar(int posChar) const
	{
		return (int)SendMsg(EM_FILELINEFROMCHAR, posChar, 0);
	}

	EckInline int FileLineIndex(int iLine) const
	{
		return (int)SendMsg(EM_FILELINEINDEX, iLine, 0);
	}

	EckInline int FileLineLength(int posChar) const
	{
		return (int)SendMsg(EM_FILELINELENGTH, posChar, 0);
	}

	EckInline int GetFileLine(int idxLine, PWSTR pszBuf, int cchMax)
	{
		if (!pszBuf || cchMax < 1)
			return 0;
		*(WORD*)pszBuf = cchMax - 1;
		int cch = (int)SendMsg(EM_GETFILELINE, idxLine, (LPARAM)pszBuf);
		*(pszBuf + cch) = L'\0';
		return cch;
	}

	EckInline int GetFileLineCount() const
	{
		return (int)SendMsg(EM_GETFILELINECOUNT, 0, 0);
	}
#endif // NTDDI_VERSION >= NTDDI_WIN10_RS5

	EckInline BOOL Undo() const
	{
		return (BOOL)SendMsg(EM_UNDO, 0, 0);
	}

	EckInline void Paste() const
	{
		SendMsg(WM_PASTE, 0, 0);
	}

	EckInline void Copy() const
	{
		SendMsg(WM_COPY, 0, 0);
	}

	EckInline void Cut() const
	{
		SendMsg(WM_CUT, 0, 0);
	}

	EckInline void SelAll() const
	{
		SetSel(0, -1);
	}

	// 置选择起始位置，保留选择长度
	void SetSelPos(int iSelPos) const
	{
		int iSelStart, iSelEnd;
		GetSel(&iSelStart, &iSelEnd);
		SetSel(iSelPos, iSelPos + ValDistance(iSelStart, iSelEnd));
	}

	// 取选择起始位置
	int GetSelPos() const
	{
		int iSelStart;
		GetSel(&iSelStart, nullptr);
		return iSelStart;
	}

	// 置选择长度
	void SetSelNum(int iSelNum) const
	{
		int iSelStart;
		GetSel(&iSelStart, nullptr);
		SetSel(iSelStart, iSelStart + iSelNum);
	}

	// 取选择长度
	int GetSelNum() const
	{
		int iStart, iEnd;
		GetSel(&iStart, &iEnd);
		return ValDistance(iStart, iEnd);
	}

	// 置选择结束位置，保留选择起始位置
	void SetSelEnd(int iSelEnd) const
	{
		int iSelStart;
		GetSel(&iSelStart, nullptr);
		SetSel(iSelStart, iSelEnd);
	}

	// 取选择结束位置
	int GetSelEnd() const
	{
		int iEnd;
		GetSel(nullptr, &iEnd);
		return iEnd;
	}

	void GetSelText(CRefStrW& rs) const
	{
		int iStart, iEnd;
		GetSel(&iStart, &iEnd);
		rs.ReSize(iEnd);
		GetText(rs.Data(), iEnd + 1);
		if (iStart)
		{
			const int cch = ValDistance(iStart, iEnd);
			wmemmove(rs.Data(), rs.Data() + iStart, cch + 1/*结尾NULL*/);
			rs.ReSize(cch);
		}
	}

	// For compatibility
	CRefStrW GetSelText() const
	{
		CRefStrW rs;
		GetSelText(rs);
		return rs;
	}

	int GetSelText(PWSTR pszBuf, int cchMax) const
	{
		if (!pszBuf || cchMax < 1)
			return 0;
		int iStart, iEnd;
		GetSel(&iStart, &iEnd);
		if (iEnd < cchMax)
		{
			GetText(pszBuf, iEnd + 1);
			const int cch = ValDistance(iStart, iEnd);
			if (iStart)
				wmemmove(pszBuf, pszBuf + iStart, cch + 1/*结尾NULL*/);
			return cch;
		}
		else
		{
			const auto pszTemp = (PWSTR)_malloca((iEnd + 1) * sizeof(WCHAR));
			EckCheckMem(pszTemp);
			GetText(pszTemp, iEnd + 1);
			const int cch = std::min(cchMax - 1, ValDistance(iStart, iEnd));
			wmemcpy(pszBuf, pszTemp + iStart, cch + 1);
			_freea(pszTemp);
			return cch;
		}
	}

	EckInline void AddText(PCWSTR pszText) const
	{
		SetSel(-2, -1);
		ReplaceSel(pszText, FALSE);
	}

	void SetTransformMode(TransMode iTransformMode) const
	{
		DWORD dwStyle;
		switch (iTransformMode)
		{
		case TransMode::None: dwStyle = 0; break;
		case TransMode::ToLowerCase: dwStyle = ES_LOWERCASE; break;
		case TransMode::ToUpperCase: dwStyle = ES_UPPERCASE; break;
		default: ECK_UNREACHABLE;
		}
		ModifyStyle(dwStyle, ES_LOWERCASE | ES_UPPERCASE);
	}

	TransMode GetTransformMode() const
	{
		DWORD dwStyle = GetStyle();
		if (IsBitSet(dwStyle, ES_LOWERCASE))
			return TransMode::ToLowerCase;
		else if (IsBitSet(dwStyle, ES_UPPERCASE))
			return TransMode::ToUpperCase;
		else
			return TransMode::None;
	}

	/// <summary>
	/// 置对齐。
	/// 需要重新创建控件
	/// </summary>
	/// <param name="iAlign"></param>
	/// <returns></returns>
	EckInline void SetAlign(Align iAlign) const
	{
		DWORD dwStyle;
		switch (iAlign)
		{
		case Align::Near: dwStyle = ES_LEFT; break;
		case Align::Center: dwStyle = ES_CENTER; break;
		case Align::Far: dwStyle = ES_RIGHT; break;
		default: ECK_UNREACHABLE;
		}
		ModifyStyle(dwStyle, ES_LEFT | ES_CENTER | ES_RIGHT);
	}

	EckInline Align GetAlign() const
	{
		DWORD dwStyle = GetStyle();
		if (IsBitSet(dwStyle, ES_RIGHT))
			return Align::Far;
		else if (IsBitSet(dwStyle, ES_CENTER))
			return Align::Center;
		else
			return Align::Near;
	}
};
ECK_RTTI_IMPL_BASE_INLINE(CEdit, CWnd);
ECK_NAMESPACE_END