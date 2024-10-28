/*
* WinEzCtrlKit Library
*
* CUpDown.h ： 标准调节器
*
* Copyright(C) 2023-2024 QingKong
*/
#pragma once
#include "CWnd.h"

ECK_NAMESPACE_BEGIN
class CUpDown :public CWnd
{
public:
	ECK_RTTI(CUpDown);
	ECK_CWND_NOSINGLEOWNER(CUpDown);
	ECK_CWND_CREATE_CLS(UPDOWN_CLASSW);

	ECK_CWNDPROP_STYLE(AlignLeft, UDS_ALIGNLEFT);
	ECK_CWNDPROP_STYLE(AlignRight, UDS_ALIGNRIGHT);
	ECK_CWNDPROP_STYLE(ArrowKeys, UDS_ARROWKEYS);
	ECK_CWNDPROP_STYLE(AutoBuddy, UDS_AUTOBUDDY);
	ECK_CWNDPROP_STYLE(Horizontal, UDS_HORZ);
	ECK_CWNDPROP_STYLE(NoThousands, UDS_NOTHOUSANDS);
	ECK_CWNDPROP_STYLE(SetBuddyInt, UDS_SETBUDDYINT);
	ECK_CWNDPROP_STYLE(Wrap, UDS_WRAP);

	EckInline int GetAccel(_Out_writes_opt_(cBuf) UDACCEL* pBuf, int cBuf) const
	{
		return (int)SendMsg(UDM_GETACCEL, cBuf, (LPARAM)pBuf);
	}

	EckInline void GetAccel(std::vector<UDACCEL>& vAccel) const
	{
		int cAccel = (int)GetAccel(nullptr, 0);
		if (!cAccel)
			return;
		vAccel.resize(cAccel);
		GetAccel(vAccel.data(), cAccel);
	}

	// For compatibility.
	EckInline std::vector<UDACCEL> GetAccel() const
	{
		std::vector<UDACCEL> vAccel;
		GetAccel(vAccel);
		return vAccel;
	}

	EckInline int GetBase() const
	{
		return (int)SendMsg(UDM_GETBASE, 0, 0);
	}

	EckInline HWND GetBuddy() const
	{
		return (HWND)SendMsg(UDM_GETBUDDY, 0, 0);
	}

	EckInline int GetPos(BOOL* pbSuccess = nullptr) const
	{
		return (int)SendMsg(UDM_GETPOS32, 0, (LPARAM)pbSuccess);
	}

	EckInline void GetRange(int* piMin = nullptr, int* piMax = nullptr) const
	{
		SendMsg(UDM_GETRANGE32, (WPARAM)piMin, (LPARAM)piMax);
	}

	EckInline BOOL SetAccel(_In_reads_(c) const UDACCEL* puda, int c) const
	{
		return (BOOL)SendMsg(UDM_SETACCEL, c, (LPARAM)puda);
	}

	EckInline BOOL SetBase(int iBase) const
	{
		return (BOOL)SendMsg(UDM_SETBASE, iBase, 0);
	}

	EckInline HWND SetBuddy(HWND hBuddy) const
	{
		return (HWND)SendMsg(UDM_SETBUDDY, (WPARAM)hBuddy, 0);
	}

	EckInline int SetPos(int iPos) const
	{
		return (int)SendMsg(UDM_SETPOS, 0, iPos);
	}

	EckInline void SetRange(int iMin, int iMax) const
	{
		SendMsg(UDM_SETRANGE32, iMin, iMax);
	}

	EckInline void SetMin(int iMin) const
	{
		int iMax;
		GetRange(nullptr, &iMax);
		SetRange(iMin, iMax);
	}

	EckInline void SetMax(int iMax) const
	{
		int iMin;
		GetRange(&iMin, nullptr);
		SetRange(iMin, iMax);
	}
};
ECK_RTTI_IMPL_BASE_INLINE(CUpDown, CWnd);
ECK_NAMESPACE_END