#pragma once
#include "CWnd.h"

ECK_NAMESPACE_BEGIN
class CTrackBar : public CWnd
{
public:
    ECK_RTTI(CTrackBar, CWnd);
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

    EckInline void ClearSelection(BOOL bRedraw) const noexcept
    {
        SendMsg(TBM_CLEARSEL, bRedraw, 0);
    }

    EckInline void ClearTickMarks(BOOL bRedraw) const noexcept
    {
        SendMsg(TBM_CLEARTICS, bRedraw, 0);
    }

    EckInline HWND GetBuddy(BOOL bLeftTopOrRightBottom) const noexcept
    {
        return (HWND)SendMsg(TBM_GETBUDDY, bLeftTopOrRightBottom, 0);
    }

    EckInline void GetChannelRect(_Out_ RECT* prc) const noexcept
    {
        SendMsg(TBM_GETCHANNELRECT, 0, (LPARAM)prc);
    }

    EckInline int GetLineSize() const noexcept
    {
        return (int)SendMsg(TBM_GETLINESIZE, 0, 0);
    }

    EckInline int GetTickMarkCount() const noexcept
    {
        return (int)SendMsg(TBM_GETNUMTICS, 0, 0);
    }

    EckInline int GetPageSize() const noexcept
    {
        return (int)SendMsg(TBM_GETPAGESIZE, 0, 0);
    }

    EckInline int GetPosition() const noexcept
    {
        return (int)SendMsg(TBM_GETPOS, 0, 0);
    }

    /// <summary>
    /// 取刻度线位置数组
    /// </summary>
    /// <returns>返回数组在修改刻度线前有效，数组总数为(GetTickMarkCount() - 2)</returns>
    EckInline UINT* GetTickMarkLogicalPositionArray() const noexcept
    {
        return (UINT*)SendMsg(TBM_GETPTICS, 0, 0);
    }

    EckInline int GetRangeMaximum() const noexcept
    {
        return (int)SendMsg(TBM_GETRANGEMAX, 0, 0);
    }

    EckInline int GetRangeMinimum() const noexcept
    {
        return (int)SendMsg(TBM_GETRANGEMIN, 0, 0);
    }

    EckInline int GetSelectionEnd() const noexcept
    {
        return (int)SendMsg(TBM_GETSELEND, 0, 0);
    }

    EckInline int GetSelectionBegin() const noexcept
    {
        return (int)SendMsg(TBM_GETSELSTART, 0, 0);
    }

    EckInline int GetThumbLength() const noexcept
    {
        return (int)SendMsg(TBM_GETTHUMBLENGTH, 0, 0);
    }

    EckInline int GetThumbRect(_Out_ RECT* prc) const noexcept
    {
        return (int)SendMsg(TBM_GETTHUMBRECT, 0, (LPARAM)prc);
    }

    EckInline int GetTickMarkLogicalPosition(int idxTic) const noexcept
    {
        return (int)SendMsg(TBM_GETTIC, idxTic, 0);
    }

    EckInline int GetTickMarkPosition(int idxTic) const noexcept
    {
        return (int)SendMsg(TBM_GETTICPOS, idxTic, 0);
    }

    EckInline HWND GetToolTips() const noexcept
    {
        return (HWND)SendMsg(TBM_GETTOOLTIPS, 0, 0);
    }

    EckInline HWND SetBuddy(HWND hBuddy, BOOL bLeftTopOrRightBottom) const noexcept
    {
        return (HWND)SendMsg(TBM_SETBUDDY, bLeftTopOrRightBottom, (LPARAM)hBuddy);
    }

    EckInline int SetLineSize(int nSize) const noexcept
    {
        return (int)SendMsg(TBM_SETLINESIZE, 0, nSize);
    }

    EckInline int SetPageSize(int nSize) const noexcept
    {
        return (int)SendMsg(TBM_SETPAGESIZE, 0, nSize);
    }

    EckInline void SetPosition(int nPos, BOOL bRedraw = TRUE) const noexcept
    {
        SendMsg(TBM_SETPOS, bRedraw, nPos);
    }

    // 与SetPosition相同，但会产生滚动通知
    EckInline void SetPositionNotify(int nPos) const noexcept
    {
        SendMsg(TBM_SETPOSNOTIFY, 0, nPos);
    }

    EckInline void SetRange16(int nMin, int nMax, BOOL bRedraw = TRUE) const noexcept
    {
        SendMsg(TBM_SETRANGE, bRedraw, MAKELPARAM(nMin, nMax));
    }

    EckInline void SetRangeMaximum(int nMax, BOOL bRedraw = TRUE) const noexcept
    {
        SendMsg(TBM_SETRANGEMAX, bRedraw, nMax);
    }

    EckInline void SetRangeMinimum(int nMin, BOOL bRedraw = TRUE) const noexcept
    {
        SendMsg(TBM_SETRANGEMIN, bRedraw, nMin);
    }

    EckInline void SetRange(int nMin, int nMax, BOOL bRedraw = TRUE) const noexcept
    {
        SetRangeMinimum(nMin, FALSE);
        SetRangeMaximum(nMax, bRedraw);
    }

    EckInline void SetSelection(int nStart, int nEnd, BOOL bRedraw = TRUE) const noexcept
    {
        SendMsg(TBM_SETSEL, bRedraw, MAKELPARAM(nStart, nEnd));
    }

    EckInline void SetSelectionEnd(int nEnd, BOOL bRedraw = TRUE) const noexcept
    {
        SendMsg(TBM_SETSELEND, bRedraw, nEnd);
    }

    EckInline void SetSelectionBegin(int nStart, BOOL bRedraw = TRUE) const noexcept
    {
        SendMsg(TBM_SETSELSTART, bRedraw, nStart);
    }

    EckInline void SetThumbLength(int nLength) const noexcept
    {
        SendMsg(TBM_SETTHUMBLENGTH, nLength, 0);
    }

    EckInline BOOL SetTickMark(int nLogPos) const noexcept
    {
        return (BOOL)SendMsg(TBM_SETTIC, 0, nLogPos);
    }

    EckInline void SetTickMarkFrequency(int nFreq) const noexcept
    {
        SendMsg(TBM_SETTICFREQ, nFreq, 0);
    }

    /// <summary>
    /// 设置工具提示的位置
    /// </summary>
    /// <param name="nLocation">TBTS_*</param>
    /// <returns>返回先前位置，TBTS_*</returns>
    EckInline int SetTipSide(int nLocation) const noexcept
    {
        return (int)SendMsg(TBM_SETTIPSIDE, nLocation, 0);
    }

    EckInline HWND SetToolTips(HWND hToolTips) const noexcept
    {
        return (HWND)SendMsg(TBM_SETTOOLTIPS, (WPARAM)hToolTips, 0);
    }
};
ECK_NAMESPACE_END