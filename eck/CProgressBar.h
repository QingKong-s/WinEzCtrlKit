/*
* WinEzCtrlKit Library
*
* CProgressBar.h ： 标准进度条
*
* Copyright(C) 2024 QingKong
*/
#pragma once
#include "CWnd.h"

ECK_NAMESPACE_BEGIN
class CProgressBar :public CWnd
{
public:
	EckInline HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, HMENU hMenu, PCVOID pData = nullptr) override
	{
		return IntCreate(0, PROGRESS_CLASSW, nullptr, dwStyle,
			x, y, cx, cy, hParent, hMenu, nullptr, nullptr);
	}

	EckInline int DeltaPos(int iDelta)
	{
		return (int)SendMsg(PBM_DELTAPOS, iDelta, 0);
	}

	EckInline COLORREF GetBarClr()
	{
		return (COLORREF)SendMsg(PBM_GETBARCOLOR, 0, 0);
	}

	EckInline COLORREF GetBkClr()
	{
		return (COLORREF)SendMsg(PBM_GETBKCOLOR, 0, 0);
	}

	EckInline int GetPos()
	{
		return (int)SendMsg(PBM_GETPOS, 0, 0);
	}

	EckInline PBRANGE GetRange()
	{
		PBRANGE r;
		SendMsg(PBM_GETRANGE, 0, (LPARAM)&r);
		return r;
	}

	EckInline int GetMin()
	{
		return (int)SendMsg(PBM_GETRANGE, TRUE, nullptr);
	}

	EckInline int GetMax()
	{
		return (int)SendMsg(PBM_GETRANGE, FALSE, nullptr);
	}

	/// <summary>
	/// 
	/// </summary>
	/// <returns>PBST_常量</returns>
	EckInline int GetState()
	{
		return (int)SendMsg(PBM_GETSTATE, 0, 0);
	}

	EckInline int GetStep()
	{
		return (int)SendMsg(PBM_GETSTEP, 0, 0);
	}

	EckInline COLORREF SetBarClr(COLORREF cr)
	{
		return (COLORREF)SendMsg(PBM_GETBARCOLOR, 0, cr);
	}

	EckInline COLORREF SetBkClr(COLORREF cr)
	{
		return (COLORREF)SendMsg(PBM_GETBKCOLOR, 0, cr);
	}

	EckInline void SetMarquee(BOOL bEnable, int msAnimation = 0)
	{
		SendMsg(PBM_SETMARQUEE, bEnable, msAnimation);
	}

	EckInline int SetPos(int i)
	{
		return (int)SendMsg(PBM_SETPOS, i, 0);
	}

	EckInline DWORD SetRange(int iMin, int iMax)
	{
		return (DWORD)SendMsg(PBM_SETRANGE32, iMin, iMax);
	}

	EckInline DWORD SetMin(int i)
	{
		return (DWORD)SendMsg(PBM_SETRANGE32, i, GetMax());
	}

	EckInline DWORD SetMax(int i)
	{
		return (DWORD)SendMsg(PBM_SETRANGE32, GetMin(), i);
	}

	/// <summary>
	/// 
	/// </summary>
	/// <param name="iState">PBST_常量</param>
	/// <returns></returns>
	EckInline int SetState(int iState)
	{
		return (int)SendMsg(PBM_SETSTATE, iState, 0);
	}

	EckInline int SetStep(int i)
	{
		return (int)SendMsg(PBM_SETSTEP, i, 0);
	}

	EckInline int StepIt()
	{
		return (int)SendMsg(PBM_STEPIT, 0, 0);
	}
};
ECK_NAMESPACE_END