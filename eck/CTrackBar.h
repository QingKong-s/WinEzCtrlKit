/*
* WinEzCtrlKit Library
*
* CTrackBar.h ： 标准滑块条
*
* Copyright(C) 2024 QingKong
*/
#pragma once
#include "CWnd.h"

ECK_NAMESPACE_BEGIN
class CTrackBar : public CWnd
{
public:
	ECK_RTTI(CTrackBar);

	ECK_CWND_CREATE;
	HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, HMENU hMenu, PCVOID pData = nullptr) override
	{
		return IntCreate(0, TRACKBAR_CLASSW, nullptr, dwStyle,
			x, y, cx, cy, hParent, hMenu, nullptr, nullptr);
	}

	EckInline void ClearSel(BOOL bRedraw)
	{
		SendMsg(TBM_CLEARSEL, bRedraw, 0);
	}

	EckInline void ClearTics(BOOL bRedraw)
	{
		SendMsg(TBM_CLEARTICS, bRedraw, 0);
	}

	EckInline HWND GetBuddy(BOOL bLocation)
	{
		return (HWND)SendMsg(TBM_GETBUDDY, bLocation, 0);
	}

	EckInline void GetChannelRect(RECT* prc)
	{
		SendMsg(TBM_GETCHANNELRECT, 0, (LPARAM)prc);
	}

	EckInline int GetLineSize()
	{
		return (int)SendMsg(TBM_GETLINESIZE, 0, 0);
	}

	EckInline int GetNumTics()
	{
		return (int)SendMsg(TBM_GETNUMTICS, 0, 0);
	}

	EckInline int GetPageSize()
	{
		return (int)SendMsg(TBM_GETPAGESIZE, 0, 0);
	}

	EckInline int GetPos()
	{
		return (int)SendMsg(TBM_GETPOS, 0, 0);
	}

	EckInline DWORD* GetPTics()
	{
		return (DWORD*)SendMsg(TBM_GETPTICS, 0, 0);
	}

	EckInline int GetRangeMax()
	{
		return (int)SendMsg(TBM_GETRANGEMAX, 0, 0);
	}

	EckInline int GetRangeMin()
	{
		return (int)SendMsg(TBM_GETRANGEMIN, 0, 0);
	}

	EckInline int GetSelEnd()
	{
		return (int)SendMsg(TBM_GETSELEND, 0, 0);
	}

	EckInline int GetSelStart()
	{
		return (int)SendMsg(TBM_GETSELSTART, 0, 0);
	}

	EckInline int GetThumbLength()
	{
		return (int)SendMsg(TBM_GETTHUMBLENGTH, 0, 0);
	}

	EckInline int GetThumbRect(RECT* prc)
	{
		return (int)SendMsg(TBM_GETTHUMBRECT, 0, (LPARAM)prc);
	}

	EckInline int GetTic(int idxTic)
	{
		return (int)SendMsg(TBM_GETTIC, idxTic, 0);
	}

	EckInline int GetTicPos(int idxTic)
	{
		return (int)SendMsg(TBM_GETTICPOS, idxTic, 0);
	}

	EckInline HWND GetToolTips()
	{
		return (HWND)SendMsg(TBM_GETTOOLTIPS, 0, 0);
	}

	EckInline HWND SetBuddy(HWND hBuddy, BOOL bLocation)
	{
		return (HWND)SendMsg(TBM_SETBUDDY, bLocation, (LPARAM)hBuddy);
	}

	EckInline int SetLineSize(int nSize)
	{
		return (int)SendMsg(TBM_SETLINESIZE, 0, nSize);
	}

	EckInline int SetPageSize(int nSize)
	{
		return (int)SendMsg(TBM_SETPAGESIZE, 0, nSize);
	}

	EckInline void SetPos(int nPos, BOOL bRedraw = TRUE)
	{
		SendMsg(TBM_SETPOS, bRedraw, nPos);
	}

	EckInline void SetPosNotify(int nPos)
	{
		SendMsg(TBM_SETPOSNOTIFY, 0, nPos);
	}

	EckInline void SetRange(int nMin, int nMax, BOOL bRedraw = TRUE)
	{
		SendMsg(TBM_SETRANGE, bRedraw, MAKELPARAM(nMin, nMax));
	}

	EckInline void SetRangeMax(int nMax, BOOL bRedraw = TRUE)
	{
		SendMsg(TBM_SETRANGEMAX, bRedraw, nMax);
	}

	EckInline void SetRangeMin(int nMin, BOOL bRedraw = TRUE)
	{
		SendMsg(TBM_SETRANGEMIN, bRedraw, nMin);
	}

	EckInline void SetRange32(int nMin, int nMax, BOOL bRedraw = TRUE)
	{
		SetRangeMin(nMin, FALSE);
		SetRangeMax(nMax, bRedraw);
	}

	EckInline void SetSel(int nStart, int nEnd, BOOL bRedraw = TRUE)
	{
		SendMsg(TBM_SETSEL, bRedraw, MAKELPARAM(nStart, nEnd));
	}

	EckInline void SetSelEnd(int nEnd, BOOL bRedraw = TRUE)
	{
		SendMsg(TBM_SETSELEND, bRedraw, nEnd);
	}

	EckInline void SetSelStart(int nStart, BOOL bRedraw = TRUE)
	{
		SendMsg(TBM_SETSELSTART, bRedraw, nStart);
	}

	EckInline void SetThumbLength(int nLength)
	{
		SendMsg(TBM_SETTHUMBLENGTH, nLength, 0);
	}

	EckInline BOOL SetTic(int nPos)
	{
		return (BOOL)SendMsg(TBM_SETTIC, 0, nPos);
	}

	EckInline void SetTicFreq(int nFreq)
	{
		SendMsg(TBM_SETTICFREQ, nFreq, 0);
	}

	EckInline int SetTipSide(int nLocation)
	{
		return (int)SendMsg(TBM_SETTIPSIDE, nLocation, 0);
	}

	EckInline HWND SetToolTips(HWND hToolTips)
	{
		return (HWND)SendMsg(TBM_SETTOOLTIPS, (WPARAM)hToolTips, 0);
	}
};
ECK_RTTI_IMPL_BASE_INLINE(CTrackBar, CWnd);
ECK_NAMESPACE_END