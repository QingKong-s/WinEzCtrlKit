/*
* WinEzCtrlKit Library
*
* CToolTip.h ： 标准工具提示
*
* Copyright(C) 2024 QingKong
*/
#pragma once
#include "CWnd.h"

ECK_NAMESPACE_BEGIN
class CToolTip :public CWnd
{
public:
	ECK_CWND_NOSINGLEOWNER(CToolTip);
	ECK_CWND_CREATE;
	HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, HMENU hMenu, PCVOID pData = nullptr) override
	{
		return IntCreate(dwExStyle, TOOLTIPS_CLASS, pszText, dwStyle,
			x, y, cx, cy, hParent, hMenu, nullptr, nullptr);
	}

	EckInline void Active(BOOL bActive)
	{
		SendMsg(TTM_ACTIVATE, bActive, 0);
	}

	EckInline BOOL AddTool(TTTOOLINFOW* pti)
	{
		return (BOOL)SendMsg(TTM_ADDTOOLW, 0, (LPARAM)pti);
	}

	EckInline BOOL AdjustRect(BOOL bTextToWnd, RECT* prc)
	{
		return (BOOL)SendMsg(TTM_ADJUSTRECT, bTextToWnd, (LPARAM)prc);
	}

	EckInline void DeleteTool(TTTOOLINFOW* pti)
	{
		SendMsg(TTM_DELTOOLW, 0, (LPARAM)pti);
	}

	EckInline void EnumTools(int idxTool, TTTOOLINFOW* pti)
	{
		SendMsg(TTM_ENUMTOOLSW, idxTool, (LPARAM)pti);
	}

	EckInline BOOL GetBubbleSize(TTTOOLINFOW* pti, int* pcx, int* pcy)
	{
		DWORD dwRet = (DWORD)SendMsg(TTM_GETBUBBLESIZE, 0, (LPARAM)pti);
		if (dwRet == 0)
			return FALSE;
		if (pcx)
			*pcx = LOWORD(dwRet);
		if (pcy)
			*pcy = HIWORD(dwRet);
		return TRUE;
	};

	EckInline BOOL GetCurrentTool(TTTOOLINFOW* pti)
	{
		return (BOOL)SendMsg(TTM_GETCURRENTTOOL, 0, (LPARAM)pti);
	}

	/// <summary>
	/// 取延迟时间
	/// </summary>
	/// <param name="iType">TTDT_常量</param>
	/// <returns>以毫秒为单位的时间</returns>
	EckInline int GetDelayTime(int iType)
	{
		return (int)SendMsg(TTM_GETDELAYTIME, iType, 0);
	}

	EckInline void GetMargin(RECT* prc)
	{
		SendMsg(TTM_GETMARGIN, 0, (LPARAM)prc);
	}

	EckInline int GetMaxTipWidth()
	{
		return (int)SendMsg(TTM_GETMAXTIPWIDTH, 0, 0);
	}

	EckInline void GetText(int cchBuf, TTTOOLINFOW* pti)
	{
		SendMsg(TTM_GETTEXTW, cchBuf, (LPARAM)pti);
	}

	EckInline COLORREF GetTipBkColor()
	{
		return (COLORREF)SendMsg(TTM_GETTIPBKCOLOR, 0, 0);
	}

	EckInline COLORREF GetTipTextColor()
	{
		return (COLORREF)SendMsg(TTM_GETTIPTEXTCOLOR, 0, 0);
	}

	EckInline void GetTitle(TTGETTITLE* pttgt)
	{
		SendMsg(TTM_GETTITLE, 0, (LPARAM)pttgt);
	}

	EckInline int GetToolCount()
	{
		return (int)SendMsg(TTM_GETTOOLCOUNT, 0, 0);
	}

	EckInline BOOL GetToolInfo(TTTOOLINFOW* pti)
	{
		return (BOOL)SendMsg(TTM_GETTOOLINFOW, 0, (LPARAM)pti);
	}

	EckInline BOOL HitTest(TTHITTESTINFO* pti)
	{
		return (BOOL)SendMsg(TTM_HITTEST, 0, (LPARAM)pti);
	}

	/// <summary>
	/// 
	/// </summary>
	/// <param name="pti">hwnd、uId、rect </param>
	EckInline void NewToolRect(TTTOOLINFOW* pti)
	{
		SendMsg(TTM_NEWTOOLRECTW, 0, (LPARAM)pti);
	}

	EckInline void Pop()
	{
		SendMsg(TTM_POP, 0, 0);
	}

	EckInline void PopUp()
	{
		SendMsg(TTM_POPUP, 0, 0);
	}

	EckInline void RelayEvent(MSG* pMsg, LPARAM lExtraInfo = 0)
	{
		SendMsg(TTM_RELAYEVENT, lExtraInfo, (LPARAM)pMsg);
	}

	EckInline void SetDelayTime(int iType, int iDelay)
	{
		SendMsg(TTM_SETDELAYTIME, iType, LOWORD(iDelay));
	}

	EckInline void SetMargin(RECT* prc)
	{
		SendMsg(TTM_SETMARGIN, 0, (LPARAM)prc);
	}

	EckInline int SetMaxTipWidth(int iWidth)
	{
		return (int)SendMsg(TTM_SETMAXTIPWIDTH, 0, iWidth);
	}

	EckInline void SetTipBkColor(COLORREF cr)
	{
		SendMsg(TTM_SETTIPBKCOLOR, cr, 0);
	}

	EckInline void SetTipTextColor(COLORREF cr)
	{
		SendMsg(TTM_SETTIPTEXTCOLOR, cr, 0);
	}

	EckInline BOOL SetTitle(int iIcon, PCWSTR pszTitle)
	{
		return (BOOL)SendMsg(TTM_SETTITLEW, iIcon, (LPARAM)pszTitle);
	}

	EckInline void SetToolInfo(TTTOOLINFOW* pti)
	{
		SendMsg(TTM_SETTOOLINFOW, 0, (LPARAM)pti);
	}

	/// <summary>
	/// 
	/// </summary>
	/// <param name="pti">hwnd、uId</param>
	/// <param name="bActivate"></param>
	/// <returns></returns>
	EckInline void TrackActivate(TTTOOLINFOW* pti, BOOL bActivate)
	{
		SendMsg(TTM_TRACKACTIVATE, bActivate, (LPARAM)pti);
	}

	EckInline void TrackPosition(int x, int y)
	{
		SendMsg(TTM_TRACKPOSITION, 0, MAKELPARAM(x, y));
	}

	EckInline void Update()
	{
		SendMsg(TTM_UPDATE, 0, 0);
	}

	/// <summary>
	/// 
	/// </summary>
	/// <param name="pti">hwnd、uId、hinst、lpszText</param>
	/// <returns></returns>
	EckInline void UpdateTipText(TTTOOLINFOW* pti)
	{
		SendMsg(TTM_UPDATETIPTEXTW, 0, (LPARAM)pti);
	}
};
ECK_NAMESPACE_END