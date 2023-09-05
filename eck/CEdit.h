/*
* WinEzCtrlKit Library
*
* CEdit.h ： 标准编辑框
*
* Copyright(C) 2023 QingKong
*/
#pragma once
#include "CWnd.h"
#include "CSubclassMgr.h"
#include "CRefStr.h"
#include "Utility.h"


ECK_NAMESPACE_BEGIN
inline constexpr int
DATAVER_EDIT_1 = 1;

#pragma pack(push, ECK_CTRLDATA_ALIGN)
struct CREATEDATA_EDIT
{
	int iVer;

};
#pragma pack(pop)

class CEdit :public CWnd
{
protected:
	// 默认提示横幅缓冲区长度，不含结尾NULL
	constexpr static int c_cchMaxCueBanner = 260;
public:
	enum class TransMode
	{
		None,
		ToLowerCase,
		ToUpperCase
	};
	
	
	virtual ~CEdit() {}

	HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, int nID, PCVOID pData = NULL) override;

	CRefBin SerializeData(SIZE_T cbExtra = 0, SIZE_T* pcbSize = NULL) override;

	EckInline BOOL CanUndo()
	{
		return (BOOL)SendMsg(EM_CANUNDO, 0, 0);
	}

	/// <summary>
	/// 取坐标处字符
	/// </summary>
	/// <param name="pt">坐标，相对客户区</param>
	/// <param name="piPosInLine">接收行中位置变量，可为NULL，失败设置为-1</param>
	/// <returns>返回字符位置，失败返回-1</returns>
	int CharFromPos(POINT pt, int* piPosInLine = NULL);

	EckInline void EmptyUndoBuffer()
	{
		SendMsg(EM_EMPTYUNDOBUFFER, 0, 0);
	}

	EckInline void FmtLines(BOOL bSoftLineBreakChar)
	{
		SendMsg(EM_FMTLINES, bSoftLineBreakChar, 0);
	}

	EckInline CRefStrW GetCueBanner(int cchMax = c_cchMaxCueBanner, BOOL bReCalcLen = TRUE)
	{
		CRefStrW rs;
		rs.ReSize(cchMax);
		SendMsg(EM_GETCUEBANNER, (WPARAM)rs.Data(), cchMax + 1);
		if (bReCalcLen)
			rs.ReCalcLen();
		return rs;
	}

	/// <summary>
	/// 取提示横幅
	/// </summary>
	/// <param name="pszBuf">缓冲区指针</param>
	/// <param name="cchBuf">缓冲区大小，不含结尾NULL</param>
	/// <returns>成功返回TRUE，失败返回FALSE</returns>
	EckInline BOOL GetCueBanner(PWSTR pszBuf, int cchBuf)
	{
		return (BOOL)SendMsg(EM_GETCUEBANNER, (WPARAM)pszBuf, cchBuf + 1);
	}

	/// <summary>
	/// 取第一可见行
	/// </summary>
	/// <returns>多行：返回行索引，单行：返回第一个可见字符索引</returns>
	EckInline int GetFirstVisibleLine()
	{
		return (int)SendMsg(EM_GETFIRSTVISIBLELINE, 0, 0);
	}

	EckInline HLOCAL GetHandle()
	{
		return (HLOCAL)SendMsg(EM_GETHANDLE, 0, 0);
	}

	EckInline UINT GetImeStatus(UINT uMask)
	{
		return (UINT)SendMsg(EM_GETIMESTATUS, uMask, 0);
	}

	EckInline int GetLimitText()
	{
		return (int)SendMsg(EM_GETLIMITTEXT, 0, 0);
	}

	/// <summary>
	/// 取某行文本
	/// </summary>
	/// <param name="iPos">行中字符位置</param>
	/// <returns>文本</returns>
	CRefStrW GetLine(int iPos);

	int GetLine(int idxLine, PWSTR pszBuf, int cchMax)
	{
		if (!pszBuf || cchMax < 1)
			return 0;
		*(WORD*)pszBuf = cchMax;
		int cch = (int)SendMsg(EM_GETLINE, idxLine, (LPARAM)pszBuf);
		*(pszBuf + cch) = L'\0';
		return cch;
	}

	EckInline int GetLineCount()
	{
		return (int)SendMsg(EM_GETLINECOUNT, 0, 0);
	}

	EckInline DWORD GetMargins()
	{
		return (DWORD)SendMsg(EM_GETMARGINS, 0, 0);
	}

	EckInline void GetMargins(int* piLeftMargin, int* piRightMargin)
	{
		DWORD dwRet = (DWORD)SendMsg(EM_GETMARGINS, 0, 0);
		if (piLeftMargin)
			*piLeftMargin = LOWORD(dwRet);
		if (piRightMargin)
			*piRightMargin = HIWORD(dwRet);
	}

	EckInline int GetModify()
	{
		return (int)SendMsg(EM_GETMODIFY, 0, 0);
	}

	EckInline WCHAR GetPasswordChar()
	{
		return (WCHAR)SendMsg(EM_GETPASSWORDCHAR, 0, 0);
	}

	EckInline void GetRect(RECT* prc)
	{
		SendMsg(EM_GETRECT, 0, (LPARAM)prc);
	}

	EckInline void GetSel(int* piSelStart, int* piSelEnd)
	{
		SendMsg(EM_GETSEL, (WPARAM)&piSelStart, (LPARAM)piSelEnd);
	}

	EckInline int GetThumb()
	{
		return (int)SendMsg(EM_GETTHUMB, 0, 0);
	}

	EckInline EDITWORDBREAKPROCW GetWordBreakProc()
	{
		return (EDITWORDBREAKPROCW)SendMsg(EM_GETWORDBREAKPROC, 0, 0);
	}

	EckInline void HideBalloonTip()
	{
		SendMsg(EM_HIDEBALLOONTIP, 0, 0);
	}

	/// <summary>
	/// 字符位置到行数
	/// </summary>
	/// <param name="iPos">字符索引，若设为-1则返回当前光标所在行，或者返回选定内容所在行（如果有）</param>
	/// <returns>返回行索引</returns>
	EckInline int LineFromChar(int iPos)
	{
		return (int)SendMsg(EM_LINEFROMCHAR, iPos, 0);
	}

	/// <summary>
	/// 取某行第一字符位置
	/// </summary>
	/// <param name="iLine">行索引，若设为-1则指定当前光标所在行</param>
	/// <returns>返回一行的第一个字符的索引</returns>
	EckInline int LineIndex(int iLine)
	{
		return (int)SendMsg(EM_LINEINDEX, iLine, 0);
	}

	/// <summary>
	/// 取某行长度
	/// </summary>
	/// <param name="iPos">行中字符位置</param>
	/// <returns>返回长度，失败返回0</returns>
	EckInline int LineLength(int iPos)
	{
		return (int)SendMsg(EM_LINELENGTH, iPos, 0);
	}

	EckInline void LineScroll(int cchHScroll, int cchVScroll)
	{
		SendMsg(EM_LINESCROLL, cchHScroll, cchVScroll);
	}

	EckInline POINT PosFromChar(int iPos)
	{
		DWORD dwRet = (DWORD)SendMsg(EM_LINESCROLL, iPos, 0);
		POINT pt;
		pt.x = LOWORD(dwRet);
		pt.y = HIWORD(dwRet);
		return pt;
	}

	EckInline void ReplaceSel(PCWSTR pszText, BOOL bAllowUndo)
	{
		SendMsg(EM_REPLACESEL, bAllowUndo, (LPARAM)pszText);
	}

	/// <summary>
	/// 滚动
	/// </summary>
	/// <param name="iOp">滚动操作，可选常量值：SB_LINEDOWN、SB_LINEUP、SB_PAGEDOWN、SB_PAGEUP</param>
	/// <returns>成功返回TRUE</returns>
	EckInline BOOL Scroll(int iOp)
	{
		return HIWORD(SendMsg(EM_SCROLL, iOp, 0));
	}

	EckInline void ScrollCaret()
	{
		SendMsg(EM_SCROLLCARET, 0, 0);
	}

	EckInline BOOL SetCueBanner(PCWSTR psz, BOOL bShowAlways)
	{
		return (BOOL)SendMsg(EM_SETCUEBANNER, bShowAlways, (LPARAM)psz);
	}

	EckInline void SetHandle(HLOCAL hLocal)
	{
		SendMsg(EM_SETHANDLE, (WPARAM)hLocal, 0);
	}

	EckInline UINT SetImeStatus(UINT uMask)
	{
		return (UINT)SendMsg(EM_SETIMESTATUS, uMask, 0);
	}

	EckInline void SetLimitText(int iMaxLen)
	{
		SendMsg(EM_SETLIMITTEXT, iMaxLen, 0);
	}

	EckInline void SetMargins(int iLeftMargin, int iRightMargin, UINT uMask = EC_LEFTMARGIN)
	{
		SendMsg(EM_SETMARGINS, uMask, MAKELPARAM(iLeftMargin, iRightMargin));
	}

	EckInline void SetModify(BOOL bModify)
	{
		SendMsg(EM_SETMODIFY, bModify, 0);
	}

	EckInline void SetPasswordChar(WCHAR chMask)
	{
		SendMsg(EM_SETPASSWORDCHAR, chMask, 0);
	}

	EckInline BOOL SetReadOnly(BOOL bReadOnly)
	{
		return (BOOL)SendMsg(EM_SETREADONLY, bReadOnly, 0);
	}

	EckInline void SetRect(RECT* prc)
	{
		SendMsg(EM_SETRECT, 0, (LPARAM)prc);
	}

	EckInline void SetRectNp(RECT* prc)
	{
		SendMsg(EM_SETRECTNP, 0, (LPARAM)prc);
	}

	EckInline void SetSel(int iSelStart, int iSelEnd)
	{
		SendMsg(EM_SETSEL, iSelStart, iSelEnd);
	}

	/// <summary>
	/// 置制表位
	/// </summary>
	/// <param name="piTabStop">制表位数组</param>
	/// <param name="c">数组元素数，若为0则设置默认制表位</param>
	EckInline void SetTabStop(int* piTabStop, int c)
	{
		SendMsg(EM_SETTABSTOPS, c, (LPARAM)piTabStop);
	}

	EckInline void SetWordBreakProc(EDITWORDBREAKPROCW pfnProc)
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
	EckInline BOOL ShowBalloonTip(PCWSTR pszCaption, PCWSTR pszContent, int iIcon)
	{
		EDITBALLOONTIP ebt;
		ebt.cbStruct = sizeof(EDITBALLOONTIP);
		ebt.pszTitle = pszCaption;
		ebt.pszText = pszContent;
		ebt.ttiIcon = iIcon;

		return (BOOL)SendMsg(EM_SHOWBALLOONTIP, 0, (LPARAM)&ebt);
	}

	EckInline BOOL ShowBalloonTip(const EDITBALLOONTIP* pebt)
	{
		return (BOOL)SendMsg(EM_SHOWBALLOONTIP, 0, (LPARAM)pebt);
	}

	EckInline BOOL Undo()
	{
		return (BOOL)SendMsg(EM_UNDO, 0, 0);
	}

	EckInline void Paste()
	{
		SendMsg(WM_PASTE, 0, 0);
	}

	EckInline void Copy()
	{
		SendMsg(WM_COPY, 0, 0);
	}

	EckInline void SelAll()
	{
		SendMsg(EM_SETSEL, 0, -1);
	}

	void SetSelPos(int iSelPos);

	EckInline int GetSelPos()
	{
		DWORD dwStart;
		SendMsg(EM_GETSEL, (WPARAM)&dwStart, NULL);
		return dwStart;
	}

	EckInline void SetSelNum(int iSelNum)
	{
		DWORD dwStart;
		SendMsg(EM_GETSEL, (WPARAM)&dwStart, NULL);
		SendMsg(EM_SETSEL, dwStart, dwStart + iSelNum);
	}

	int GetSelNum();

	EckInline void SetSelEnd(int iSelEnd)
	{
		DWORD dwStart;
		SendMsg(EM_GETSEL, (WPARAM)&dwStart, NULL);
		SendMsg(EM_SETSEL, dwStart, iSelEnd);
	}

	EckInline int GetSelEnd()
	{
		DWORD dwEnd;
		SendMsg(EM_GETSEL, NULL, (LPARAM)&dwEnd);
		return dwEnd;
	}

	EckInline void SetSelText(PCWSTR pszText)
	{
		SendMsg(EM_REPLACESEL, TRUE, (LPARAM)pszText);
	}

	int GetSelLen();

	CRefStrW GetSelText();

	int GetSelText(PWSTR pszBuf, int cchMax);

	EckInline void AddText(PCWSTR pszText)
	{
		SendMsg(EM_SETSEL, -2, -1);
		SendMsg(EM_REPLACESEL, FALSE, (LPARAM)pszText);
	}

	void SetTransformMode(TransMode iTransformMode);

	TransMode GetTransformMode();

	/// <summary>
	/// 置失去焦点时隐藏选择。
	/// 需要重新创建控件
	/// </summary>
	/// <param name="bHideSel">是否隐藏</param>
	/// <returns></returns>
	EckInline void SetHideSel(BOOL bHideSel)
	{
		ModifyStyle(bHideSel ? 0 : ES_NOHIDESEL, ES_NOHIDESEL);
	}

	EckInline BOOL GetHideSel()
	{
		return !IsBitSet(GetStyle(), ES_NOHIDESEL);
	}

	/// <summary>
	/// 置对齐。
	/// 需要重新创建控件
	/// </summary>
	/// <param name="iAlign"></param>
	/// <returns></returns>
	EckInline void SetAlign(Align iAlign)
	{
		DWORD dwStyle = 0;
		switch (iAlign)
		{
		case Align::Near:dwStyle = ES_LEFT; break;
		case Align::Center:dwStyle = ES_CENTER; break;
		case Align::Far:dwStyle = ES_RIGHT; break;
		}
		ModifyStyle(dwStyle, ES_LEFT | ES_CENTER | ES_RIGHT);
	}

	EckInline Align GetAlign()
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
ECK_NAMESPACE_END