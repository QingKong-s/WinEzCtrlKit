/*
* WinEzCtrlKit Library
*
* CWaitableTimer.h : 可等待定时器
*
* Copyright(C) 2024 QingKong
*/
#pragma once
#include "CWaitableObject.h"

ECK_NAMESPACE_BEGIN
class CWaitableTimer : public CWaitableObject
{
public:
	CWaitableTimer()
	{
		OBJECT_ATTRIBUTES oa;
		InitOA(oa);
		NtCreateTimer(&m_hObj, TIMER_ALL_ACCESS, &oa, SynchronizationTimer);
	}

	CWaitableTimer(PCWSTR pszName, BOOL bManualReset = FALSE, BOOL bInheritHandle = FALSE)
	{
		UNICODE_STRING us;
		if (pszName)
			RtlInitUnicodeString(&us, pszName);
		OBJECT_ATTRIBUTES oa;
		InitOA(oa, (pszName ? &us : nullptr), (bInheritHandle ? OBJ_INHERIT : 0));
		NtCreateTimer(&m_hObj, TIMER_ALL_ACCESS, &oa,
			(bManualReset ? NotificationTimer : SynchronizationTimer));
	}

	EckInline NTSTATUS SetDueTime100ns(LONGLONG ll)
	{
		ll *= -1;
		return NtSetTimer(m_hObj, (LARGE_INTEGER*)&ll,
			nullptr, nullptr, FALSE, 0, nullptr);
	}

	EckInline NTSTATUS SetDueTimeAndPeriod100ns(LONGLONG ll, LONG lPeriod)
	{
		ll *= -1;
		return NtSetTimer(m_hObj, (LARGE_INTEGER*)&ll,
			nullptr, nullptr, FALSE, lPeriod, nullptr);
	}

	EckInline NTSTATUS SetDueTime(LONG ll)
	{
		return SetDueTime100ns(ll * 10000ll);
	}

	EckInline NTSTATUS SetDueTimeAndPeriod(LONG ll, LONG lPeriod)
	{
		return SetDueTimeAndPeriod100ns(ll * 10000ll, lPeriod);
	}

	EckInline NTSTATUS Cancel()
	{
		return NtCancelTimer(m_hObj, nullptr);
	}

	EckInline NTSTATUS Cancel(BOOLEAN& bPrevState)
	{
		return NtCancelTimer(m_hObj, &bPrevState);
	}

	EckInline NTSTATUS QueryState(BOOLEAN& bState, LONGLONG& llRemainingTime)
	{
		TIMER_BASIC_INFORMATION	tbi;
		ULONG cbRet;
		const auto nts = NtQueryTimer(m_hObj, TimerBasicInformation, &tbi, sizeof(tbi), &cbRet);
		if (NT_SUCCESS(nts))
		{
			bState = tbi.TimerState;
			llRemainingTime = tbi.RemainingTime.QuadPart;
		}
		else
		{
			bState = FALSE;
			llRemainingTime = 0;
		}
		return nts;
	}
};
ECK_NAMESPACE_END