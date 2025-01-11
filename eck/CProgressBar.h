#pragma once
#include "CWnd.h"

ECK_NAMESPACE_BEGIN
class CProgressBar :public CWnd
{
public:
	ECK_RTTI(CProgressBar);
	ECK_CWND_NOSINGLEOWNER(CProgressBar);
	ECK_CWND_CREATE_CLS(PROGRESS_CLASSW);

	ECK_CWNDPROP_STYLE(Marquee, PBS_MARQUEE);
	ECK_CWNDPROP_STYLE(Smooth, PBS_SMOOTH);
	ECK_CWNDPROP_STYLE(SmoothReverse, PBS_SMOOTHREVERSE);
	ECK_CWNDPROP_STYLE(Vertical, PBS_VERTICAL);

	EckInline int DeltaPos(int iDelta) const
	{
		return (int)SendMsg(PBM_DELTAPOS, iDelta, 0);
	}

	EckInline COLORREF GetBarClr() const
	{
		return (COLORREF)SendMsg(PBM_GETBARCOLOR, 0, 0);
	}

	EckInline COLORREF GetBkClr() const
	{
		return (COLORREF)SendMsg(PBM_GETBKCOLOR, 0, 0);
	}

	EckInline int GetPos() const
	{
		return (int)SendMsg(PBM_GETPOS, 0, 0);
	}

	EckInline PBRANGE GetRange() const
	{
		PBRANGE r;
		SendMsg(PBM_GETRANGE, 0, (LPARAM)&r);
		return r;
	}

	EckInline int GetMin() const
	{
		return (int)SendMsg(PBM_GETRANGE, TRUE, 0);
	}

	EckInline int GetMax() const
	{
		return (int)SendMsg(PBM_GETRANGE, FALSE, 0);
	}

	/// <summary>
	/// 取状态
	/// </summary>
	/// <returns>PBST_常量</returns>
	EckInline int GetState() const
	{
		return (int)SendMsg(PBM_GETSTATE, 0, 0);
	}

	EckInline int GetStep() const
	{
		return (int)SendMsg(PBM_GETSTEP, 0, 0);
	}

	EckInline COLORREF SetBarClr(COLORREF cr) const
	{
		return (COLORREF)SendMsg(PBM_GETBARCOLOR, 0, cr);
	}

	EckInline COLORREF SetBkClr(COLORREF cr) const
	{
		return (COLORREF)SendMsg(PBM_GETBKCOLOR, 0, cr);
	}

	EckInline void SetMarquee(BOOL bEnable, int msAnimation = 0) const
	{
		SendMsg(PBM_SETMARQUEE, bEnable, msAnimation);
	}

	EckInline int SetPos(int i) const
	{
		return (int)SendMsg(PBM_SETPOS, i, 0);
	}

	EckInline DWORD SetRange(int iMin, int iMax) const
	{
		return (DWORD)SendMsg(PBM_SETRANGE32, iMin, iMax);
	}

	EckInline DWORD SetMin(int i) const
	{
		return (DWORD)SendMsg(PBM_SETRANGE32, i, GetMax());
	}

	EckInline DWORD SetMax(int i) const
	{
		return (DWORD)SendMsg(PBM_SETRANGE32, GetMin(), i);
	}

	/// <summary>
	/// 置状态
	/// </summary>
	/// <param name="iState">PBST_常量</param>
	/// <returns></returns>
	EckInline int SetState(int iState) const
	{
		return (int)SendMsg(PBM_SETSTATE, iState, 0);
	}

	EckInline int SetStep(int i) const
	{
		return (int)SendMsg(PBM_SETSTEP, i, 0);
	}

	EckInline int StepIt() const
	{
		return (int)SendMsg(PBM_STEPIT, 0, 0);
	}
};
ECK_RTTI_IMPL_BASE_INLINE(CProgressBar, CWnd);
ECK_NAMESPACE_END