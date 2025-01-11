#pragma once
#include "CWnd.h"

ECK_NAMESPACE_BEGIN
class CToolTip :public CWnd
{
public:
	ECK_RTTI(CToolTip);
	ECK_CWND_NOSINGLEOWNER(CToolTip);
	ECK_CWND_CREATE_CLS(TOOLTIPS_CLASS);

	ECK_CWNDPROP_STYLE(AlwaysTip, TTS_ALWAYSTIP);
	ECK_CWNDPROP_STYLE(Balloon, TTS_BALLOON);
	ECK_CWNDPROP_STYLE(CloseButton, TTS_CLOSE);
	ECK_CWNDPROP_STYLE(NoAnimate, TTS_NOANIMATE);
	ECK_CWNDPROP_STYLE(NoFade, TTS_NOFADE);
	ECK_CWNDPROP_STYLE(NoPrefix, TTS_NOPREFIX);
	ECK_CWNDPROP_STYLE(UseVisualStyle, TTS_USEVISUALSTYLE);

	BOOL IsNeedTheme() const override { return TRUE; }

	EckInline void Active(BOOL bActive) const
	{
		SendMsg(TTM_ACTIVATE, bActive, 0);
	}

	EckInline BOOL AddTool(TTTOOLINFOW* pti) const
	{
		return (BOOL)SendMsg(TTM_ADDTOOLW, 0, (LPARAM)pti);
	}

	EckInline BOOL AdjustRect(BOOL bTextToWnd, RECT* prc) const
	{
		return (BOOL)SendMsg(TTM_ADJUSTRECT, bTextToWnd, (LPARAM)prc);
	}

	EckInline void DeleteTool(TTTOOLINFOW* pti) const
	{
		SendMsg(TTM_DELTOOLW, 0, (LPARAM)pti);
	}

	EckInline void EnumTools(int idxTool, TTTOOLINFOW* pti) const
	{
		SendMsg(TTM_ENUMTOOLSW, idxTool, (LPARAM)pti);
	}

	EckInline BOOL GetBubbleSize(TTTOOLINFOW* pti, int* pcx, int* pcy) const
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

	EckInline BOOL GetCurrentTool(TTTOOLINFOW* pti) const
	{
		return (BOOL)SendMsg(TTM_GETCURRENTTOOL, 0, (LPARAM)pti);
	}

	/// <summary>
	/// 取延迟时间
	/// </summary>
	/// <param name="iType">TTDT_常量</param>
	/// <returns>以毫秒为单位的时间</returns>
	EckInline int GetDelayTime(int iType) const
	{
		return (int)SendMsg(TTM_GETDELAYTIME, iType, 0);
	}

	EckInline void GetMargin(RECT* prc) const
	{
		SendMsg(TTM_GETMARGIN, 0, (LPARAM)prc);
	}

	EckInline int GetMaxTipWidth() const
	{
		return (int)SendMsg(TTM_GETMAXTIPWIDTH, 0, 0);
	}

	EckInline void GetText(int cchBuf, TTTOOLINFOW* pti) const
	{
		SendMsg(TTM_GETTEXTW, cchBuf, (LPARAM)pti);
	}

	EckInline COLORREF GetTipBkColor() const
	{
		return (COLORREF)SendMsg(TTM_GETTIPBKCOLOR, 0, 0);
	}

	EckInline COLORREF GetTipTextColor() const
	{
		return (COLORREF)SendMsg(TTM_GETTIPTEXTCOLOR, 0, 0);
	}

	EckInline void GetTitle(TTGETTITLE* pttgt) const
	{
		SendMsg(TTM_GETTITLE, 0, (LPARAM)pttgt);
	}

	EckInline int GetToolCount() const
	{
		return (int)SendMsg(TTM_GETTOOLCOUNT, 0, 0);
	}

	EckInline BOOL GetToolInfo(TTTOOLINFOW* pti) const
	{
		return (BOOL)SendMsg(TTM_GETTOOLINFOW, 0, (LPARAM)pti);
	}

	EckInline BOOL HitTest(TTHITTESTINFOW* pti) const
	{
		return (BOOL)SendMsg(TTM_HITTEST, 0, (LPARAM)pti);
	}

	/// <summary>
	/// 
	/// </summary>
	/// <param name="pti">hwnd、uId、rect </param>
	EckInline void NewToolRect(TTTOOLINFOW* pti) const
	{
		SendMsg(TTM_NEWTOOLRECTW, 0, (LPARAM)pti);
	}

	EckInline void Pop() const
	{
		SendMsg(TTM_POP, 0, 0);
	}

	EckInline void PopUp() const
	{
		SendMsg(TTM_POPUP, 0, 0);
	}

	EckInline void RelayEvent(MSG* pMsg, LPARAM lExtraInfo = 0) const
	{
		SendMsg(TTM_RELAYEVENT, lExtraInfo, (LPARAM)pMsg);
	}

	EckInline void SetDelayTime(int iType, int iDelay) const
	{
		SendMsg(TTM_SETDELAYTIME, iType, LOWORD(iDelay));
	}

	EckInline void SetMargin(RECT* prc) const
	{
		SendMsg(TTM_SETMARGIN, 0, (LPARAM)prc);
	}

	EckInline int SetMaxTipWidth(int iWidth) const
	{
		return (int)SendMsg(TTM_SETMAXTIPWIDTH, 0, iWidth);
	}

	EckInline void SetTipBkColor(COLORREF cr) const
	{
		SendMsg(TTM_SETTIPBKCOLOR, cr, 0);
	}

	EckInline void SetTipTextColor(COLORREF cr) const
	{
		SendMsg(TTM_SETTIPTEXTCOLOR, cr, 0);
	}

	EckInline BOOL SetTitle(int iIcon, PCWSTR pszTitle) const
	{
		return (BOOL)SendMsg(TTM_SETTITLEW, iIcon, (LPARAM)pszTitle);
	}

	EckInline void SetToolInfo(TTTOOLINFOW* pti) const
	{
		SendMsg(TTM_SETTOOLINFOW, 0, (LPARAM)pti);
	}

	/// <summary>
	/// 
	/// </summary>
	/// <param name="pti">hwnd、uId</param>
	/// <param name="bActivate"></param>
	/// <returns></returns>
	EckInline void TrackActivate(TTTOOLINFOW* pti, BOOL bActivate) const
	{
		SendMsg(TTM_TRACKACTIVATE, bActivate, (LPARAM)pti);
	}

	EckInline void TrackPosition(int x, int y) const
	{
		SendMsg(TTM_TRACKPOSITION, 0, MAKELPARAM(x, y));
	}

	EckInline void Update() const
	{
		SendMsg(TTM_UPDATE, 0, 0);
	}

	/// <summary>
	/// 
	/// </summary>
	/// <param name="pti">hwnd、uId、hinst、lpszText</param>
	/// <returns></returns>
	EckInline void UpdateTipText(TTTOOLINFOW* pti) const
	{
		SendMsg(TTM_UPDATETIPTEXTW, 0, (LPARAM)pti);
	}
};
ECK_RTTI_IMPL_BASE_INLINE(CToolTip, CWnd);
ECK_NAMESPACE_END