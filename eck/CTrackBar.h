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
	ECK_CWND_NOSINGLEOWNER(CTrackBar);
	ECK_CWND_CREATE_CLS(TRACKBAR_CLASSW);

	ECK_CWNDPROP_STYLE(AutoTicks, TBS_AUTOTICKS);
	ECK_CWNDPROP_STYLE(Vertical, TBS_VERT);
	ECK_CWNDPROP_STYLE(Horizontal, TBS_HORZ);
	ECK_CWNDPROP_STYLE(TicksTop, TBS_TOP);
	ECK_CWNDPROP_STYLE(TicksBottom, TBS_BOTTOM);
	ECK_CWNDPROP_STYLE(TicksLeft, TBS_LEFT);
	ECK_CWNDPROP_STYLE(TicksRight, TBS_RIGHT);
	ECK_CWNDPROP_STYLE(TicksBoth, TBS_BOTH);
	ECK_CWNDPROP_STYLE(Noticks, TBS_NOTICKS);
	ECK_CWNDPROP_STYLE(EnableSelRange, TBS_ENABLESELRANGE);
	ECK_CWNDPROP_STYLE(FixedLength, TBS_FIXEDLENGTH);
	ECK_CWNDPROP_STYLE(NoThumb, TBS_NOTHUMB);
	ECK_CWNDPROP_STYLE(ToolTips, TBS_TOOLTIPS);
	ECK_CWNDPROP_STYLE(Reversed, TBS_REVERSED);
	ECK_CWNDPROP_STYLE(DownIsLeft, TBS_DOWNISLEFT);
	ECK_CWNDPROP_STYLE(NotifyBeforeMove, TBS_NOTIFYBEFOREMOVE);
	ECK_CWNDPROP_STYLE(TransparentBk, TBS_TRANSPARENTBKGND);

	EckInline void ClearSel(BOOL bRedraw) const
	{
		SendMsg(TBM_CLEARSEL, bRedraw, 0);
	}

	EckInline void ClearTics(BOOL bRedraw) const
	{
		SendMsg(TBM_CLEARTICS, bRedraw, 0);
	}

	EckInline HWND GetBuddy(BOOL bLocation) const
	{
		return (HWND)SendMsg(TBM_GETBUDDY, bLocation, 0);
	}

	EckInline void GetChannelRect(RECT* prc) const
	{
		SendMsg(TBM_GETCHANNELRECT, 0, (LPARAM)prc);
	}

	EckInline int GetLineSize() const
	{
		return (int)SendMsg(TBM_GETLINESIZE, 0, 0);
	}

	EckInline int GetNumTics() const
	{
		return (int)SendMsg(TBM_GETNUMTICS, 0, 0);
	}

	EckInline int GetPageSize() const
	{
		return (int)SendMsg(TBM_GETPAGESIZE, 0, 0);
	}

	EckInline int GetPos() const
	{
		return (int)SendMsg(TBM_GETPOS, 0, 0);
	}

	EckInline DWORD* GetPTics() const
	{
		return (DWORD*)SendMsg(TBM_GETPTICS, 0, 0);
	}

	EckInline int GetRangeMax() const
	{
		return (int)SendMsg(TBM_GETRANGEMAX, 0, 0);
	}

	EckInline int GetRangeMin() const
	{
		return (int)SendMsg(TBM_GETRANGEMIN, 0, 0);
	}

	EckInline int GetSelEnd() const
	{
		return (int)SendMsg(TBM_GETSELEND, 0, 0);
	}

	EckInline int GetSelStart() const
	{
		return (int)SendMsg(TBM_GETSELSTART, 0, 0);
	}

	EckInline int GetThumbLength() const
	{
		return (int)SendMsg(TBM_GETTHUMBLENGTH, 0, 0);
	}

	EckInline int GetThumbRect(RECT* prc) const
	{
		return (int)SendMsg(TBM_GETTHUMBRECT, 0, (LPARAM)prc);
	}

	EckInline int GetTic(int idxTic) const
	{
		return (int)SendMsg(TBM_GETTIC, idxTic, 0);
	}

	EckInline int GetTicPos(int idxTic) const
	{
		return (int)SendMsg(TBM_GETTICPOS, idxTic, 0);
	}

	EckInline HWND GetToolTips() const
	{
		return (HWND)SendMsg(TBM_GETTOOLTIPS, 0, 0);
	}

	EckInline HWND SetBuddy(HWND hBuddy, BOOL bLocation) const
	{
		return (HWND)SendMsg(TBM_SETBUDDY, bLocation, (LPARAM)hBuddy);
	}

	EckInline int SetLineSize(int nSize) const
	{
		return (int)SendMsg(TBM_SETLINESIZE, 0, nSize);
	}

	EckInline int SetPageSize(int nSize) const
	{
		return (int)SendMsg(TBM_SETPAGESIZE, 0, nSize);
	}

	EckInline void SetPos(int nPos, BOOL bRedraw = TRUE) const
	{
		SendMsg(TBM_SETPOS, bRedraw, nPos);
	}

	EckInline void SetPosNotify(int nPos) const
	{
		SendMsg(TBM_SETPOSNOTIFY, 0, nPos);
	}

	EckInline void SetRange(int nMin, int nMax, BOOL bRedraw = TRUE) const
	{
		SendMsg(TBM_SETRANGE, bRedraw, MAKELPARAM(nMin, nMax));
	}

	EckInline void SetRangeMax(int nMax, BOOL bRedraw = TRUE) const
	{
		SendMsg(TBM_SETRANGEMAX, bRedraw, nMax);
	}

	EckInline void SetRangeMin(int nMin, BOOL bRedraw = TRUE) const
	{
		SendMsg(TBM_SETRANGEMIN, bRedraw, nMin);
	}

	EckInline void SetRange32(int nMin, int nMax, BOOL bRedraw = TRUE) const
	{
		SetRangeMin(nMin, FALSE);
		SetRangeMax(nMax, bRedraw);
	}

	EckInline void SetSel(int nStart, int nEnd, BOOL bRedraw = TRUE) const
	{
		SendMsg(TBM_SETSEL, bRedraw, MAKELPARAM(nStart, nEnd));
	}

	EckInline void SetSelEnd(int nEnd, BOOL bRedraw = TRUE) const
	{
		SendMsg(TBM_SETSELEND, bRedraw, nEnd);
	}

	EckInline void SetSelStart(int nStart, BOOL bRedraw = TRUE) const
	{
		SendMsg(TBM_SETSELSTART, bRedraw, nStart);
	}

	EckInline void SetThumbLength(int nLength) const
	{
		SendMsg(TBM_SETTHUMBLENGTH, nLength, 0);
	}

	EckInline BOOL SetTic(int nPos) const
	{
		return (BOOL)SendMsg(TBM_SETTIC, 0, nPos);
	}

	EckInline void SetTicFreq(int nFreq) const
	{
		SendMsg(TBM_SETTICFREQ, nFreq, 0);
	}

	EckInline int SetTipSide(int nLocation) const
	{
		return (int)SendMsg(TBM_SETTIPSIDE, nLocation, 0);
	}

	EckInline HWND SetToolTips(HWND hToolTips) const
	{
		return (HWND)SendMsg(TBM_SETTOOLTIPS, (WPARAM)hToolTips, 0);
	}
};
ECK_RTTI_IMPL_BASE_INLINE(CTrackBar, CWnd);
ECK_NAMESPACE_END