#pragma once
#include "CWindow.h"
#include "DDX.h"

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

    EckInline PCWSTR CueBanner() const noexcept
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
			return !GetEDExtendStyle();					\
		else											\
			return IsBitSet(GetEDExtendStyle(), Style);	\
	}													\
	void StyleSet##Name(BOOL b) const					\
	{													\
		SetEDExtendStyle(b ? Style : 0, Style);			\
	}
#endif// NTDDI_VERSION >= NTDDI_WIN10_RS5

class CEdit : public CWindow
{
public:
    ECK_RTTI(CEdit, CWindow);
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

    EckInline constexpr static PCVOID SkipBaseData(PCVOID p) noexcept
    {
        return (PCBYTE)p + sizeof(CTRLDATA_EDIT) +
            (((const CTRLDATA_EDIT*)p)->cchCueBanner + 1) * sizeof(WCHAR);
    }

    void SerializeData(CByteBuffer& rb, const SERIALIZE_OPT* pOpt = nullptr) noexcept override
    {
        const auto rsCueBanner = GetCueBanner((pOpt && pOpt->cchTextBuf ?
            pOpt->cchTextBuf : MAX_PATH));
        const SIZE_T cbSize = sizeof(CTRLDATA_EDIT) + rsCueBanner.ByteSize();
        CWindow::SerializeData(rb, pOpt);
        const auto p = (CTRLDATA_EDIT*)rb.PushBack(cbSize);
        p->iVer = CDV_EDIT_1;
        p->chPassword = GetPasswordChar();
        p->eTransMode = (ECKENUM)GetTransformMode();
        GetSelection(&p->iSelStart, &p->iSelEnd);
        p->cchCueBanner = rsCueBanner.Size();
        p->cchLimit = GetLimitText();
        if (p->cchCueBanner)
            wmemcpy(PWSTR(p + 1), rsCueBanner.Data(), rsCueBanner.Size() + 1);
        else
            *PWSTR(p + 1) = L'\0';
    }

    void PostDeserialize(PCVOID pData) noexcept override
    {
        __super::PostDeserialize(pData);
        const auto* const p = (CTRLDATA_EDIT*)__super::SkipBaseData(pData);
        if (p->iVer != CDV_EDIT_1)
            return;
        SetPasswordChar(p->chPassword);
        SetTransformMode((TransMode)p->eTransMode);
        SetSelection(p->iSelStart, p->iSelEnd);
        SetCueBanner(p->CueBanner(), TRUE);
        SetLimitText(p->cchLimit);
    }

    EckInline BOOL CanUndo() const noexcept
    {
        return (BOOL)SendMessage(EM_CANUNDO, 0, 0);
    }

    /// <summary>
    /// 取坐标处字符
    /// </summary>
    /// <param name="pt">坐标，相对客户区</param>
    /// <param name="piPosInLine">接收行中位置变量，可为NULL，失败设置为-1</param>
    /// <returns>返回字符位置，失败返回-1</returns>
    int CharFromPosition(POINT pt, _Out_opt_ int* piPosInLine = nullptr) const noexcept
    {
        int posChar;
        UINT uRet = (UINT)SendMessage(EM_CHARFROMPOS, 0, MAKELPARAM(pt.x, pt.y));
        USHORT usPos = LOWORD(uRet);
        if (usPos == 65535)
            posChar = -1;
        else
            posChar = usPos;
        if (piPosInLine)
        {
            usPos = HIWORD(uRet);
            if (usPos == 65535)
                *piPosInLine = -1;
            else
                *piPosInLine = usPos;
        }
        return posChar;
    }

    EckInline void EmptyUndoBuffer() const noexcept
    {
        SendMessage(EM_EMPTYUNDOBUFFER, 0, 0);
    }

    EckInline void FormatLines(BOOL bSoftLineBreakChar) const noexcept
    {
        SendMessage(EM_FMTLINES, bSoftLineBreakChar, 0);
    }

    ECK_SUPPRESS_MISSING_ZERO_TERMINATION
        /// <summary>
        /// 取提示横幅
        /// </summary>
        /// <param name="pszBuf">缓冲区指针</param>
        /// <param name="cchBuf">缓冲区大小</param>
        /// <returns>成功返回TRUE，失败返回FALSE</returns>
        EckInline BOOL GetCueBanner(_Out_writes_(cchBuf) PWSTR pszBuf, int cchBuf) const noexcept
    {
        return (BOOL)SendMessage(EM_GETCUEBANNER, (WPARAM)pszBuf, cchBuf);
    }

    EckInline BOOL GetCueBanner(CStringW& rs, int cchMax = MAX_PATH,
        BOOL bReCalcLen = TRUE) const noexcept
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
    EckInline CStringW GetCueBanner(int cchMax = MAX_PATH,
        BOOL bReCalcLen = TRUE) const noexcept
    {
        CStringW rs;
        GetCueBanner(rs, cchMax, bReCalcLen);
        return rs;
    }

    /// <summary>
    /// 取第一可见行
    /// </summary>
    /// <returns>多行：返回行索引，单行：返回第一个可见字符索引</returns>
    EckInline int GetFirstVisibleLine() const noexcept
    {
        return (int)SendMessage(EM_GETFIRSTVISIBLELINE, 0, 0);
    }

    EckInline HLOCAL GetHandle() const noexcept
    {
        return (HLOCAL)SendMessage(EM_GETHANDLE, 0, 0);
    }

    EckInline UINT GetImeState(UINT uMask) const noexcept
    {
        return (UINT)SendMessage(EM_GETIMESTATUS, uMask, 0);
    }

    EckInline int GetLimitText() const noexcept
    {
        return (int)SendMessage(EM_GETLIMITTEXT, 0, 0);
    }

    int GetLine(int idxLine, _Out_writes_(cchMax) PWSTR pszBuf, int cchMax) const noexcept
    {
        if (!pszBuf || cchMax < 1)
            return 0;
        *(WORD*)pszBuf = cchMax - 1;
        int cch = (int)SendMessage(EM_GETLINE, idxLine, (LPARAM)pszBuf);
        *(pszBuf + cch) = L'\0';
        return cch;
    }

    void GetLine(CStringW& rs, int posChar) const noexcept
    {
        rs.ReSize(LineLength(posChar));
        if (rs.IsEmpty())
            return;
        GetLine(LineFromChar(posChar), rs.Data(), rs.Size() + 1);
    }

    // For compatibility
    CStringW GetLine(int posChar) const noexcept
    {
        CStringW rs;
        GetLine(rs, posChar);
        return rs;
    }

    EckInline int GetLineCount() const noexcept
    {
        return (int)SendMessage(EM_GETLINECOUNT, 0, 0);
    }

    EckInline UINT GetMargins() const noexcept
    {
        return (UINT)SendMessage(EM_GETMARGINS, 0, 0);
    }

    EckInline void GetMargins(_Out_opt_ int* piLeftMargin,
        _Out_opt_ int* piRightMargin) const noexcept
    {
        UINT uRet = (UINT)SendMessage(EM_GETMARGINS, 0, 0);
        if (piLeftMargin)
            *piLeftMargin = LOWORD(uRet);
        if (piRightMargin)
            *piRightMargin = HIWORD(uRet);
    }

    EckInline int GetModify() const noexcept
    {
        return (int)SendMessage(EM_GETMODIFY, 0, 0);
    }

    EckInline WCHAR GetPasswordChar() const noexcept
    {
        return (WCHAR)SendMessage(EM_GETPASSWORDCHAR, 0, 0);
    }

    EckInline void GetRect(_Out_ RECT* prc) const noexcept
    {
        SendMessage(EM_GETRECT, 0, (LPARAM)prc);
    }

    EckInline void GetSelection(_Out_opt_ int* piSelStart,
        _Out_opt_ int* piSelEnd) const noexcept
    {
        SendMessage(EM_GETSEL, (WPARAM)piSelStart, (LPARAM)piSelEnd);
    }

    /// <summary>
    /// 取垂直滚动条位置
    /// </summary>
    /// <returns>位置</returns>
    EckInline int GetThumbPosition() const noexcept
    {
        return (int)SendMessage(EM_GETTHUMB, 0, 0);
    }

    EckInline EDITWORDBREAKPROCW GetWordBreakProcedure() const noexcept
    {
        return (EDITWORDBREAKPROCW)SendMessage(EM_GETWORDBREAKPROC, 0, 0);
    }

    EckInline void HideBalloonTip() const noexcept
    {
        SendMessage(EM_HIDEBALLOONTIP, 0, 0);
    }

    /// <summary>
    /// 字符位置到行数
    /// </summary>
    /// <param name="posChar">字符索引，若设为-1则返回当前光标所在行，或者返回选定内容所在行（如果有）</param>
    /// <returns>返回行索引</returns>
    EckInline int LineFromChar(int posChar) const noexcept
    {
        return (int)SendMessage(EM_LINEFROMCHAR, posChar, 0);
    }

    /// <summary>
    /// 取某行第一字符位置
    /// </summary>
    /// <param name="iLine">行索引，若设为-1则指定当前光标所在行</param>
    /// <returns>返回一行的第一个字符的索引</returns>
    EckInline int LineIndex(int iLine) const noexcept
    {
        return (int)SendMessage(EM_LINEINDEX, iLine, 0);
    }

    /// <summary>
    /// 取某行长度
    /// </summary>
    /// <param name="posChar">行中字符位置</param>
    /// <returns>返回长度，失败返回0</returns>
    EckInline int LineLength(int posChar) const noexcept
    {
        return (int)SendMessage(EM_LINELENGTH, posChar, 0);
    }

    EckInline void LineScroll(int cchHScroll, int cchVScroll) const noexcept
    {
        SendMessage(EM_LINESCROLL, cchHScroll, cchVScroll);
    }

    EckInline POINT PositionFromChar(int posChar) const noexcept
    {
        UINT uRet = (UINT)SendMessage(EM_LINESCROLL, posChar, 0);
        POINT pt;
        pt.x = LOWORD(uRet);
        pt.y = HIWORD(uRet);
        return pt;
    }

    EckInline void ReplaceSelection(PCWSTR pszText, BOOL bAllowUndo) const noexcept
    {
        SendMessage(EM_REPLACESEL, bAllowUndo, (LPARAM)pszText);
    }

    /// <summary>
    /// 滚动
    /// </summary>
    /// <param name="iOp">滚动操作，可选常量值：SB_LINEDOWN、SB_LINEUP、SB_PAGEDOWN、SB_PAGEUP</param>
    /// <returns>成功返回TRUE</returns>
    EckInline BOOL Scroll(int iOp) const noexcept
    {
        return HIWORD(SendMessage(EM_SCROLL, iOp, 0));
    }

    EckInline void ScrollCaret() const noexcept
    {
        SendMessage(EM_SCROLLCARET, 0, 0);
    }

    EckInline BOOL SetCueBanner(PCWSTR psz, BOOL bShowAlways = TRUE) const noexcept
    {
        return (BOOL)SendMessage(EM_SETCUEBANNER, bShowAlways, (LPARAM)psz);
    }

    EckInline void SetHandle(HLOCAL hLocal) const noexcept
    {
        SendMessage(EM_SETHANDLE, (WPARAM)hLocal, 0);
    }

    EckInline UINT SetImeState(UINT uMask) const noexcept
    {
        return (UINT)SendMessage(EM_SETIMESTATUS, uMask, 0);
    }

    EckInline void SetLimitText(int iMaxLen) const noexcept
    {
        SendMessage(EM_SETLIMITTEXT, iMaxLen, 0);
    }

    /// <summary>
    /// 置边距
    /// </summary>
    /// <param name="iLeftMargin">左边距，可以为EC_USEFONTINFO</param>
    /// <param name="iRightMargin">右边距，可以为EC_USEFONTINFO</param>
    /// <param name="uMask">EC_LEFTMARGIN、EC_RIGHTMARGIN</param>
    EckInline void SetMargins(int iLeftMargin, int iRightMargin, UINT uMask) const noexcept
    {
        SendMessage(EM_SETMARGINS, uMask, MAKELPARAM(iLeftMargin, iRightMargin));
    }

    EckInline void SetModify(BOOL bModify) const noexcept
    {
        SendMessage(EM_SETMODIFY, bModify, 0);
    }

    EckInline void SetPasswordChar(WCHAR chMask) const noexcept
    {
        SendMessage(EM_SETPASSWORDCHAR, chMask, 0);
    }

    EckInline BOOL SetReadOnly(BOOL bReadOnly) const noexcept
    {
        return (BOOL)SendMessage(EM_SETREADONLY, bReadOnly, 0);
    }

    EckInline void SetRect(_In_opt_ const RECT* prc) const noexcept
    {
        SendMessage(EM_SETRECT, 0, (LPARAM)prc);
    }

    EckInline void SetRectNoPaint(_In_opt_ const RECT* prc) const noexcept
    {
        SendMessage(EM_SETRECTNP, 0, (LPARAM)prc);
    }

    EckInline void SetSelection(int iSelStart, int iSelEnd) const noexcept
    {
        SendMessage(EM_SETSEL, iSelStart, iSelEnd);
    }

    /// <summary>
    /// 置制表位
    /// </summary>
    /// <param name="piTabStop">制表位数组</param>
    /// <param name="c">数组元素数，若为0则设置默认制表位</param>
    EckInline void SetTabStop(_In_reads_opt_(c) const int* piTabStop, int c) const noexcept
    {
        SendMessage(EM_SETTABSTOPS, c, (LPARAM)piTabStop);
    }

    EckInline void SetWordBreakProcedure(EDITWORDBREAKPROCW pfnProc) const noexcept
    {
        SendMessage(EM_SETWORDBREAKPROC, 0, (LPARAM)pfnProc);
    }

    /// <summary>
    /// 弹出气球提示
    /// </summary>
    /// <param name="pszCaption">标题</param>
    /// <param name="pszContent">文本</param>
    /// <param name="iIcon">图标类型，TTI_常量</param>
    /// <returns>成功返回TRUE</returns>
    EckInline BOOL ShowBalloonTip(PCWSTR pszCaption, PCWSTR pszContent, int iIcon) const noexcept
    {
        EDITBALLOONTIP ebt;
        ebt.cbStruct = sizeof(EDITBALLOONTIP);
        ebt.pszTitle = pszCaption;
        ebt.pszText = pszContent;
        ebt.ttiIcon = iIcon;

        return (BOOL)SendMessage(EM_SHOWBALLOONTIP, 0, (LPARAM)&ebt);
    }

    EckInline BOOL ShowBalloonTip(const EDITBALLOONTIP* pebt) const noexcept
    {
        return (BOOL)SendMessage(EM_SHOWBALLOONTIP, 0, (LPARAM)pebt);
    }

#if NTDDI_VERSION >= NTDDI_WIN10_RS5// 1809+
    EckInline HRESULT SetEDExtendStyle(DWORD dwStyle, DWORD dwMask) const noexcept
    {
        return (HRESULT)SendMessage(EM_SETEXTENDEDSTYLE, dwMask, dwStyle);
    }

    EckInline DWORD GetEDExtendStyle() const noexcept
    {
        return (DWORD)SendMessage(EM_GETEXTENDEDSTYLE, 0, 0);
    }

    EckInline BOOL SetEndOfLine(EC_ENDOFLINE eEol) const noexcept
    {
        return (BOOL)SendMessage(EM_SETENDOFLINE, (WPARAM)eEol, 0);
    }

    EckInline EC_ENDOFLINE GetEndOfLine() const noexcept
    {
        return (EC_ENDOFLINE)SendMessage(EM_GETENDOFLINE, 0, 0);
    }

    EckInline void EnableSearchWeb(BOOL bEnable) const noexcept
    {
        SendMessage(EM_ENABLESEARCHWEB, bEnable, 0);
    }

    EckInline void SearchWeb() const noexcept
    {
        SendMessage(EM_SEARCHWEB, 0, 0);
    }

    EckInline void SetCaretIndex(int idx) const noexcept
    {
        SendMessage(EM_SETCARETINDEX, idx, 0);
    }

    EckInline int GetCaretIndex() const noexcept
    {
        return (int)SendMessage(EM_GETCARETINDEX, 0, 0);
    }

    EckInline BOOL SetZoom(int nZoomNumerator, int nZoomDenominator) const noexcept
    {
        return (BOOL)SendMessage(EM_SETZOOM, nZoomNumerator, nZoomDenominator);
    }

    EckInline BOOL GetZoom(int* pnZoomNumerator, int* pnZoomDenominator) const noexcept
    {
        return (BOOL)SendMessage(EM_GETZOOM, (WPARAM)pnZoomNumerator, (LPARAM)pnZoomDenominator);
    }

    EckInline int FileLineFromChar(int posChar) const noexcept
    {
        return (int)SendMessage(EM_FILELINEFROMCHAR, posChar, 0);
    }

    EckInline int FileLineIndex(int iLine) const noexcept
    {
        return (int)SendMessage(EM_FILELINEINDEX, iLine, 0);
    }

    EckInline int FileLineLength(int posChar) const noexcept
    {
        return (int)SendMessage(EM_FILELINELENGTH, posChar, 0);
    }

    EckInline int GetFileLine(int idxLine,
        _Out_writes_(cchMax) PWSTR pszBuf, int cchMax) const noexcept
    {
        if (!pszBuf || cchMax < 1)
            return 0;
        *(WORD*)pszBuf = cchMax - 1;
        int cch = (int)SendMessage(EM_GETFILELINE, idxLine, (LPARAM)pszBuf);
        *(pszBuf + cch) = L'\0';
        return cch;
    }

    EckInline int GetFileLineCount() const noexcept
    {
        return (int)SendMessage(EM_GETFILELINECOUNT, 0, 0);
    }
#endif // NTDDI_VERSION >= NTDDI_WIN10_RS5

    EckInline BOOL Undo() const noexcept
    {
        return (BOOL)SendMessage(EM_UNDO, 0, 0);
    }

    EckInline void Paste() const noexcept
    {
        SendMessage(WM_PASTE, 0, 0);
    }

    EckInline void Copy() const noexcept
    {
        SendMessage(WM_COPY, 0, 0);
    }

    EckInline void Cut() const noexcept
    {
        SendMessage(WM_CUT, 0, 0);
    }

    EckInline void SelectAll() const noexcept
    {
        SetSelection(0, -1);
    }

    // 置选择起始位置，保留选择长度
    void SetSelectionBegin(int iSelPos) const noexcept
    {
        int iSelStart, iSelEnd;
        GetSelection(&iSelStart, &iSelEnd);
        SetSelection(iSelPos, iSelPos + ValDistance(iSelStart, iSelEnd));
    }

    // 取选择起始位置
    int GetSelectionBegin() const noexcept
    {
        int iSelStart;
        GetSelection(&iSelStart, nullptr);
        return iSelStart;
    }

    // 置选择长度
    void SetSelectionLength(int iSelNum) const noexcept
    {
        int iSelStart;
        GetSelection(&iSelStart, nullptr);
        SetSelection(iSelStart, iSelStart + iSelNum);
    }

    // 取选择长度
    int GetSelectionLength() const noexcept
    {
        int iStart, iEnd;
        GetSelection(&iStart, &iEnd);
        return ValDistance(iStart, iEnd);
    }

    // 置选择结束位置，保留选择起始位置
    void SetSelectionEnd(int iSelEnd) const noexcept
    {
        int iSelStart;
        GetSelection(&iSelStart, nullptr);
        SetSelection(iSelStart, iSelEnd);
    }

    // 取选择结束位置
    int GetSelectionEnd() const noexcept
    {
        int iEnd;
        GetSelection(nullptr, &iEnd);
        return iEnd;
    }

    void GetSelectedText(CStringW& rs) const noexcept
    {
        int iStart, iEnd;
        GetSelection(&iStart, &iEnd);
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
    CStringW GetSelectedText() const noexcept
    {
        CStringW rs;
        GetSelectedText(rs);
        return rs;
    }

    int GetSelectedText(PWSTR pszBuf, int cchMax) const noexcept
    {
        if (!pszBuf || cchMax < 1)
            return 0;
        int iStart, iEnd;
        GetSelection(&iStart, &iEnd);
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

    EckInline void AddText(PCWSTR pszText) const noexcept
    {
        SetSelection(-2, -1);
        ReplaceSelection(pszText, FALSE);
    }

    void SetTransformMode(TransMode iTransformMode) const noexcept
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

    TransMode GetTransformMode() const noexcept
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
    EckInline void SetAlignment(Align iAlign) const noexcept
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

    EckInline Align GetAlignment() const noexcept
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

inline CWindow::HSlot DdxBindEdit(CEdit& Ctrl, CWindow& Parent, Observable<CStringW>& o) noexcept
{
    o.SetCallback([](const CStringW& v, void* p)
        {
            ((CEdit*)p)->SetText(v.Data());
        }, &Ctrl);

    struct Fn : public CDdxControlCollection
    {
        LRESULT operator()(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, SlotCtx& Ctx)
        {
            if (uMsg == WM_COMMAND && HIWORD(wParam) == EN_UPDATE)
            {
                const auto pObservable = (Observable<CStringW>*)At((HWND)lParam);
                if (!pObservable)
                    return 0;
                auto& rs = pObservable->Get();
                auto cch = GetWindowTextLengthW((HWND)lParam);
                rs.ReSize(cch);
                if (cch)
                {
                    cch = GetWindowTextW((HWND)lParam, rs.Data(), cch + 1);
                    if (cch != rs.Size())
                        rs.ReSize(cch);
                }
            }
            return 0;
        }
    };
    return DdxpConnectSlot<Fn, MHI_DDX_EDIT>(Parent, Ctrl, o);
}
ECK_NAMESPACE_END