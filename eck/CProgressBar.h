#pragma once
#include "CWnd.h"

ECK_NAMESPACE_BEGIN
class CProgressBar : public CWnd
{
public:
    ECK_RTTI(CProgressBar, CWnd);
    ECK_CWND_NOSINGLEOWNER(CProgressBar);
    ECK_CWND_CREATE_CLS(PROGRESS_CLASSW);

    ECK_CWNDPROP_STYLE(Marquee, PBS_MARQUEE);
    ECK_CWNDPROP_STYLE(Smooth, PBS_SMOOTH);
    ECK_CWNDPROP_STYLE(SmoothReverse, PBS_SMOOTHREVERSE);
    ECK_CWNDPROP_STYLE(Vertical, PBS_VERTICAL);

    EckInline int DeltaPosition(int iDelta) const noexcept
    {
        return (int)SendMsg(PBM_DELTAPOS, iDelta, 0);
    }

    EckInline COLORREF GetBarColor() const noexcept
    {
        return (COLORREF)SendMsg(PBM_GETBARCOLOR, 0, 0);
    }

    EckInline COLORREF GetBackgroundColor() const noexcept
    {
        return (COLORREF)SendMsg(PBM_GETBKCOLOR, 0, 0);
    }

    EckInline int GetPosition() const noexcept
    {
        return (int)SendMsg(PBM_GETPOS, 0, 0);
    }

    EckInline PBRANGE GetRange() const noexcept
    {
        PBRANGE r;
        SendMsg(PBM_GETRANGE, 0, (LPARAM)&r);
        return r;
    }

    EckInline int GetMinimum() const noexcept
    {
        return (int)SendMsg(PBM_GETRANGE, TRUE, 0);
    }

    EckInline int GetMaximum() const noexcept
    {
        return (int)SendMsg(PBM_GETRANGE, FALSE, 0);
    }

    /// <summary>
    /// 取状态
    /// </summary>
    /// <returns>PBST_常量</returns>
    EckInline int GetState() const noexcept
    {
        return (int)SendMsg(PBM_GETSTATE, 0, 0);
    }

    EckInline int GetStep() const noexcept
    {
        return (int)SendMsg(PBM_GETSTEP, 0, 0);
    }

    EckInline COLORREF SetBarColor(COLORREF cr) const noexcept
    {
        return (COLORREF)SendMsg(PBM_GETBARCOLOR, 0, cr);
    }

    EckInline COLORREF SetBackgroundColor(COLORREF cr) const noexcept
    {
        return (COLORREF)SendMsg(PBM_GETBKCOLOR, 0, cr);
    }

    EckInline void SetMarquee(BOOL bEnable, int msAnimation = 0) const noexcept
    {
        SendMsg(PBM_SETMARQUEE, bEnable, msAnimation);
    }

    EckInline int SetPosition(int i) const noexcept
    {
        return (int)SendMsg(PBM_SETPOS, i, 0);
    }

    EckInline DWORD SetRange(int iMin, int iMax) const noexcept
    {
        return (DWORD)SendMsg(PBM_SETRANGE32, iMin, iMax);
    }

    EckInline DWORD SetMinimum(int i) const noexcept
    {
        return (DWORD)SendMsg(PBM_SETRANGE32, i, GetMaximum());
    }

    EckInline DWORD SetMaximum(int i) const noexcept
    {
        return (DWORD)SendMsg(PBM_SETRANGE32, GetMinimum(), i);
    }

    /// <summary>
    /// 置状态
    /// </summary>
    /// <param name="iState">PBST_常量</param>
    /// <returns></returns>
    EckInline int SetState(int iState) const noexcept
    {
        return (int)SendMsg(PBM_SETSTATE, iState, 0);
    }

    EckInline int SetStep(int i) const noexcept
    {
        return (int)SendMsg(PBM_SETSTEP, i, 0);
    }

    EckInline int StepIt() const noexcept
    {
        return (int)SendMsg(PBM_STEPIT, 0, 0);
    }
};
ECK_NAMESPACE_END