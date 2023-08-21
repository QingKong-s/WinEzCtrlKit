/*
* WinEzCtrlKit Library
*
* CEdit.h �� ��׼�༭��
* ��װ�˱�׼�༭���ԼӸ�װʹ�����Ը��ḻ
*
* Copyright(C) 2023 QingKong
*/
#pragma once
#include "CWnd.h"
#include "CSubclassMgr.h"
#include "CRefStr.h"
#include "Utility.h"

#include <string>

#define ED_CUEBANNER_MAXLEN 260

ECK_NAMESPACE_BEGIN

struct ECKEDITDATA
{
	COLORREF crText;		// �ı���ɫ
	COLORREF crTextBK;		// �ı�����ɫ
	COLORREF crBK;			// �༭�򱳾�ɫ
	int iInputMode;			// ���뷽ʽ
	BITBOOL bMultiLine : 1;	// ����
	BITBOOL bAutoWrap : 1;	// �Զ�����
};

class CEdit :public CWnd
{
public:
	enum class InputMode
	{
		Normal = 0,
		ReadOnly = 1,
		Password = 2,
		IntText = 3,
		RealText = 4,
		Byte = 5,
		Short = 6,
		Int = 7,
		LongLong = 8,
		Float = 9,
		Double = 10,
		DateTime = 11
	};
private:
	SUBCLASS_MGR_DECL(CEdit)
	SUBCLASS_REF_MGR_DECL(CEdit, ObjRecorderRefPlaceholder)
private:
	ECKEDITDATA m_Info{};

	HBRUSH m_hbrEditBK = NULL;
	int m_cyText = 0;// �ı��߶�
	RECT m_rcMargins{};// �ı߿�ߴ�
	HWND m_hParent = NULL;

	void UpdateTextInfo();

	static LRESULT CALLBACK SubclassProc_Parent(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
		UINT_PTR uIdSubclass, DWORD_PTR dwRefData);

	static LRESULT CALLBACK SubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
		UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
public:
	EckInline HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, int nID, PCVOID pData = NULL)
	{
		m_Info.crTextBK = m_Info.crBK = GetSysColor(COLOR_WINDOW);
		m_Info.crText = CLR_DEFAULT;
		dwStyle |= WS_CHILD;
		if (m_Info.bMultiLine)
			dwStyle |= ES_MULTILINE | ES_AUTOVSCROLL | (m_Info.bAutoWrap ? ES_AUTOHSCROLL : 0);
		else
			dwStyle |= ES_AUTOHSCROLL;

		m_hWnd = CreateWindowExW(dwExStyle, WC_EDITW, pszText, dwStyle,
			x, y, cx, cy, hParent, i32ToP<HMENU>(nID), NULL, NULL);
		m_SM.AddSubclass(m_hWnd, this);
		m_hParent = hParent;
		m_SMRef.AddRef(hParent, ObjRecorderRefPlaceholderVal);

		UpdateTextInfo();
		FrameChanged();
		return m_hWnd;
	}

	EckInline void ChangeParent(HWND hNewParent)
	{
		
	}

	~CEdit()
	{
		DeleteObject(m_hbrEditBK);
	}

	void SetClr(int iType, COLORREF cr);

	EckInline COLORREF GetClr(int iType)
	{
		switch (iType)
		{
		case 0:return m_Info.crText;
		case 1:return m_Info.crTextBK;
		case 2:return m_Info.crBK;
		}
		assert(FALSE);
	}

	EckInline void SetHideSel(BOOL bHideSel)
	{
		ModifyStyle(bHideSel ? 0 : ES_NOHIDESEL, ES_NOHIDESEL);
	}

	EckInline BOOL GetHideSel()
	{
		return !IsBitSet(GetWindowLongPtrW(m_hWnd, GWL_STYLE), ES_NOHIDESEL);
	}

	EckInline void SetMaxLen(int iMaxLen)
	{
		SendMsg(EM_SETLIMITTEXT, iMaxLen, 0);
	}

	EckInline int GetMaxLen()
	{
		return (int)SendMsg(EM_GETLIMITTEXT, 0, 0);
	}

	EckInline void SetMultiLine(BOOL bMultiLine)
	{
		m_Info.bMultiLine = bMultiLine;
	}

	EckInline BOOL GetMultiLine()
	{
		return IsBitSet(GetWindowLongPtrW(m_hWnd, GWL_STYLE), ES_MULTILINE);
	}

	EckInline void SetAutoWrap(BOOL bAutoWrap)
	{
		m_Info.bAutoWrap = bAutoWrap;
	}

	EckInline BOOL GetAutoWrap()
	{
		return m_Info.bAutoWrap;
	}

	EckInline void SetAlign(int iAlign)
	{
		DWORD dwStyle = 0;
		switch (iAlign)
		{
		case CALeft:dwStyle = ES_LEFT; break;
		case CACenter:dwStyle = ES_CENTER; break;
		case CARight:dwStyle = ES_RIGHT; break;
		}
		ModifyStyle(dwStyle, ES_LEFT | ES_CENTER | ES_RIGHT);
	}

	EckInline int GetAlign()
	{
		DWORD dwStyle = GetStyle();
		if (IsBitSet(dwStyle, ES_RIGHT))
			return CARight;
		else if (IsBitSet(dwStyle, ES_CENTER))
			return CACenter;
		else
			return CALeft;
	}

	EckInline void SetInputMode(InputMode iInputMode)
	{
		SetInputMode((int)iInputMode);
	}

	EckInline void SetInputMode(int iInputMode)
	{
		m_Info.iInputMode = iInputMode;
		ModifyStyle((iInputMode == 1) ? ES_READONLY : 0, ES_READONLY);

		if (iInputMode == 2)// ��������
			SendMsg(EM_SETPASSWORDCHAR, L'*', 0);
		else
			SendMsg(EM_SETPASSWORDCHAR, 0, 0);
	}

	EckInline int GetInputMode()
	{
		return m_Info.iInputMode;
	}

	EckInline void SetMaskChar(WCHAR chMask)
	{
		if (!chMask)
			chMask = L'*';
		SendMsg(EM_SETPASSWORDCHAR, chMask, 0);
	}

	EckInline WCHAR GetMaskChar()
	{
		return (WCHAR)SendMsg(EM_GETPASSWORDCHAR, 0, 0);
	}

	void SetTransformMode(int iTransformMode);

	int GetTransformMode();

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
		SendMsg(EM_SETSEL, dwStart, (LPARAM)(dwStart + iSelNum));
	}

	int GetSelNum();

	EckInline void SetSelText(PCWSTR pszText)
	{
		SendMsg(EM_REPLACESEL, TRUE, (LPARAM)pszText);
	}

	CRefStrW GetSelText();

	EckInline void SetCueBanner(PCWSTR psz, BOOL bShowAlways)
	{
		SendMsg(EM_SETCUEBANNER, bShowAlways, (LPARAM)psz);
	}

	EckInline CRefStrW GetCueBanner(SIZE_T* pcb = NULL)
	{
		CRefStrW rs;
		rs.ReSize(ED_CUEBANNER_MAXLEN);
		SendMsg(EM_GETCUEBANNER, (WPARAM)rs.m_pszText, ED_CUEBANNER_MAXLEN);
		return rs;
	}

	EckInline void AddText(PCWSTR pszText)
	{
		SendMsg(EM_SETSEL, -2, -1);
		SendMsg(EM_REPLACESEL, FALSE, (LPARAM)pszText);
	}

	/// <summary>
	/// ȡ���괦�ַ�
	/// </summary>
	/// <param name="pt">���꣬��Կͻ���</param>
	/// <param name="piPosInLine">��������λ�ñ�������ΪNULL��ʧ������Ϊ-1</param>
	/// <returns>�����ַ�λ�ã�ʧ�ܷ���-1</returns>
	int CharFromPos(POINT pt, int* piPosInLine = NULL);

	EckInline BOOL CanUndo()
	{
		return (BOOL)SendMsg(EM_CANUNDO, 0, 0);
	}

	EckInline void EmptyUndoBuffer()
	{
		SendMsg(EM_EMPTYUNDOBUFFER, 0, 0);
	}

	EckInline int GetFirstLine()
	{
		return (int)SendMsg(EM_GETFIRSTVISIBLELINE, 0, 0);
	}

	EckInline int GetLineCount()
	{
		return (int)SendMsg(EM_GETLINECOUNT, 0, 0);
	}

	EckInline int GetModify()
	{
		return (int)SendMsg(EM_GETMODIFY, 0, 0);
	}

	EckInline void HideBalloonTip()
	{
		SendMsg(EM_HIDEBALLOONTIP, 0, 0);
	}

	/// <summary>
	/// ȡĳ�г���
	/// </summary>
	/// <param name="iPos">�����ַ�λ��</param>
	/// <returns>���س��ȣ�ʧ�ܷ���0</returns>
	EckInline int LineLength(int iPos)
	{
		return (int)SendMsg(EM_LINELENGTH, iPos, 0);
	}

	/// <summary>
	/// ȡĳ���ı�
	/// </summary>
	/// <param name="iPos">�����ַ�λ��</param>
	/// <returns>�ı�</returns>
	CRefStrW GetLine(int iPos);

	EckInline void GetMargins(int* piLeftMargin, int* piRightMargin)
	{
		DWORD dwRet = (DWORD)SendMsg(EM_GETMARGINS, 0, 0);
		if (piLeftMargin)
			*piLeftMargin = LOWORD(dwRet);
		if (piRightMargin)
			*piRightMargin = HIWORD(dwRet);
	}

	EckInline RECT GetRect()
	{
		RECT rc;
		SendMsg(EM_GETRECT, 0, (LPARAM)&rc);
		return rc;
	}

	/// <summary>
	/// �ַ�λ�õ�����
	/// </summary>
	/// <param name="iPos">�ַ�����������Ϊ-1�򷵻ص�ǰ��������У����߷���ѡ�����������У�����У�</param>
	/// <returns>����������</returns>
	EckInline int LineFromChar(int iPos)
	{
		return (int)SendMsg(EM_LINEFROMCHAR, iPos, 0);
	}

	/// <summary>
	/// ȡĳ�е�һ�ַ�λ��
	/// </summary>
	/// <param name="iLine">������������Ϊ-1��ָ����ǰ���������</param>
	/// <returns>����һ�еĵ�һ���ַ�������</returns>
	EckInline int LineIndex(int iLine)
	{
		return (int)SendMsg(EM_LINEINDEX, iLine, 0);
	}

	/// <summary>
	/// ����
	/// </summary>
	/// <param name="iOp">������������ѡ����ֵ��SB_LINEDOWN��SB_LINEUP��SB_PAGEDOWN��SB_PAGEUP</param>
	/// <returns>�ɹ�����TRUE</returns>
	EckInline BOOL Scroll(int iOp)
	{
		return HIWORD(SendMsg(EM_SCROLL, iOp, 0));
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

	EckInline void SetMargins(int iLeftMargin, int iRightMargin, UINT uMask = EC_LEFTMARGIN)
	{
		SendMsg(EM_SETMARGINS, uMask, MAKELPARAM(iLeftMargin, iRightMargin));
	}

	EckInline void SetModify(BOOL bModify)
	{
		SendMsg(EM_SETMODIFY, bModify, 0);
	}

	EckInline void SetRect(RECT* prc)
	{
		SendMsg(EM_SETRECT, 0, (LPARAM)prc);
	}

	/// <summary>
	/// ���Ʊ�λ
	/// </summary>
	/// <param name="piTabStip">�Ʊ�λ����</param>
	/// <param name="c">����Ԫ��������Ϊ0������Ĭ���Ʊ�λ</param>
	EckInline void SetTabStop(int* piTabStip, int c)
	{
		SendMsg(EM_SETTABSTOPS, c, (LPARAM)piTabStip);
	}

	/// <summary>
	/// ����������ʾ
	/// </summary>
	/// <param name="pszCaption">����</param>
	/// <param name="pszContent">�ı�</param>
	/// <param name="iIcon">ͼ�����ͣ�TTI_����</param>
	/// <returns>�ɹ�����TRUE</returns>
	EckInline BOOL ShowBalloonTip(PCWSTR pszCaption, PCWSTR pszContent, int iIcon)
	{
		EDITBALLOONTIP ebt;
		ebt.cbStruct = sizeof(EDITBALLOONTIP);
		ebt.pszTitle = pszCaption;
		ebt.pszText = pszContent;
		ebt.ttiIcon = iIcon;

		return (BOOL)SendMsg(EM_SHOWBALLOONTIP, 0, (LPARAM)&ebt);
	}

	EckInline BOOL Undo()
	{
		return (BOOL)SendMsg(WM_UNDO, 0, 0);
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

	EckInline void SetSel(int iStart, int iEnd)
	{
		SendMsg(EM_SETSEL, iStart, iEnd);
	}
};
ECK_NAMESPACE_END