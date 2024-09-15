/*
* WinEzCtrlKit Library
*
* CUpDown.h ： 标准调节器
*
* Copyright(C) 2023 QingKong
*/
#pragma once
#include "CWnd.h"
#include "Utility.h"

#include <vector>

#include <CommCtrl.h>

ECK_NAMESPACE_BEGIN
struct EUPDOWNDATA
{
	int iDirection;			// 方向
	BOOL bAutoBuddy;		// 自动选择伙伴
	int iBuddyAlign;		// 伙伴窗口定位方式
	BOOL bArrowKeys;		// 是否由上下箭头控制
	BOOL bHotTrack;			// 是否热点跟踪
};
/*
* 调节器
*
* 事件：
* WM_NOTIFY
* ->UDN_DELTAPOS(NMUPDOWN)
*/
class CUpDown :public CWnd
{
private:
	EUPDOWNDATA m_Info{};

public:
	ECK_CWND_CREATE;
	HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, HMENU hMenu, PCVOID pData = nullptr) override
	{
		dwStyle |= (WS_CHILD | UDS_SETBUDDYINT);
		switch (m_Info.iBuddyAlign)
		{
		case 1:dwStyle |= UDS_ALIGNLEFT; break;
		case 2:dwStyle |= UDS_ALIGNRIGHT; break;
		}
		if (m_Info.bAutoBuddy)
			dwStyle |= UDS_AUTOBUDDY;
		if (m_Info.iDirection)
			dwStyle |= UDS_HORZ;
		if (m_Info.bArrowKeys)
			dwStyle |= UDS_ARROWKEYS;
		if (m_Info.bHotTrack)
			dwStyle |= UDS_HOTTRACK;
		return IntCreate(dwExStyle, UPDOWN_CLASSW, pszText, dwStyle,
			x, y, cx, cy, hParent, hMenu, nullptr, nullptr);
	}

	EckInline void SetDirection(int iDirection)
	{
		m_Info.iDirection = iDirection;
	}

	EckInline int GetDirection()
	{
		return (int)IsBitSet(GetStyle(), UDS_HORZ);
	}

	EckInline void SetMin(int iMin)
	{
		int iMax;
		SendMsg(UDM_GETRANGE32, NULL, (LPARAM)&iMax);
		SendMsg(UDM_SETRANGE32, iMin, iMax);
	}

	EckInline void SetMax(int iMax)
	{
		int iMin;
		SendMsg(UDM_GETRANGE32, (WPARAM)&iMin, NULL);
		SendMsg(UDM_SETRANGE32, iMin, iMax);
	}

	EckInline void GetRange(int* piMin = nullptr, int* piMax = nullptr)
	{
		SendMsg(UDM_GETRANGE32, (WPARAM)piMin, (LPARAM)piMax);
	}

	EckInline void SetPos(int iPos)
	{
		SendMsg(UDM_SETPOS, 0, iPos);
	}

	EckInline int GetPos()
	{
		return (int)SendMsg(UDM_GETPOS32, 0, NULL);
	}

	EckInline void SetAutoBuddy(BOOL bAutoBuddy)
	{
		m_Info.bAutoBuddy = bAutoBuddy;
	}

	EckInline BOOL GetAutoBuddy()
	{
		return IsBitSet(GetStyle(), UDS_AUTOBUDDY);
	}

	EckInline void SetBuddy(HWND hBuddy)
	{
		SendMsg(UDM_SETBUDDY, (WPARAM)hBuddy, 0);
	}

	EckInline HWND GetBuddy()
	{
		return (HWND)SendMsg(UDM_GETBUDDY, 0, 0);
	}

	EckInline void SetBuddyAlign(int iAlign)
	{
		DWORD dwStyle = 0;
		switch (iAlign)
		{
		case 0:dwStyle = 0; break;
		case 1:dwStyle = UDS_ALIGNLEFT; break;
		case 2:dwStyle = UDS_ALIGNRIGHT; break;
		default:assert(FALSE); break;
		}
		ModifyStyle(dwStyle, UDS_ALIGNLEFT | UDS_ALIGNRIGHT);
	}

	EckInline int GetBuddyAlign()
	{
		DWORD dwStyle = GetStyle();
		if (IsBitSet(dwStyle, UDS_ALIGNLEFT))
			return 1;
		else if (IsBitSet(dwStyle, UDS_ALIGNRIGHT))
			return 2;
		else
			return 0;
	}

	EckInline void SetBase(int iBase)
	{
		SendMsg(UDM_SETBASE, (iBase ? 16 : 10), 0);
	}

	EckInline int GetBase()
	{
		return (int)(SendMsg(UDM_GETBASE, 0, 0) == 16);
	}

	EckInline void SetHousands(BOOL bHousands)
	{
		ModifyStyle(bHousands ? 0 : UDS_NOTHOUSANDS, UDS_NOTHOUSANDS);
	}

	EckInline BOOL GetHousands()
	{
		return !IsBitSet(GetStyle(), UDS_NOTHOUSANDS);
	}

	EckInline void SetArrowKeys(BOOL bArrowKeys)
	{
		m_Info.bArrowKeys = bArrowKeys;
	}

	EckInline BOOL GetArrowKeys()
	{
		return IsBitSet(GetStyle(), UDS_ARROWKEYS);
	}

	EckInline void SetHotTrack(BOOL bHotTrack)
	{
		m_Info.bHotTrack = bHotTrack;
	}

	EckInline BOOL GetHotTrack()
	{
		return IsBitSet(GetStyle(), UDS_HOTTRACK);
	}

	EckInline BOOL SetAccel(UDACCEL* puda, int c)
	{
		return (BOOL)SendMsg(UDM_SETACCEL, c, (LPARAM)puda);
	}

	EckInline std::vector<UDACCEL> GetAccel()
	{
		std::vector<UDACCEL> aAccel;
		int cAccel = (int)SendMsg(UDM_GETACCEL, 0, NULL);
		if (!cAccel)
			return aAccel;
		aAccel.resize(cAccel);
		SendMsg(UDM_GETACCEL, cAccel, (LPARAM)aAccel.data());
		return aAccel;
	}
};
ECK_NAMESPACE_END