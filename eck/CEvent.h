/*
* WinEzCtrlKit Library
*
* CEvent.h ： 事件对象
*
* Copyright(C) 2024 QingKong
*/
#pragma once
#include "CWaitableObject.h"

ECK_NAMESPACE_BEGIN
class CEvent : public CWaitableObject
{
public:
	CEvent()
	{
		OBJECT_ATTRIBUTES oa;
		InitOA(oa);
		NtCreateEvent(&m_hObj, EVENT_ALL_ACCESS, &oa, SynchronizationEvent, FALSE);
	}

	CEvent(PCWSTR pszName, BOOL bManualReset, BOOL bInitSignal, BOOL bInheritHandle = FALSE)
	{
		UNICODE_STRING us;
		if (pszName)
			RtlInitUnicodeString(&us, pszName);
		OBJECT_ATTRIBUTES oa;
		InitOA(oa, (pszName ? &us : nullptr), (bInheritHandle ? OBJ_INHERIT : 0));
		NtCreateEvent(&m_hObj, EVENT_ALL_ACCESS, &oa,
			bManualReset ? NotificationEvent : SynchronizationEvent, FALSE);
	}

	EckInline NTSTATUS Signal()
	{
		return NtSetEvent(m_hObj, nullptr);
	}

	EckInline NTSTATUS Signal(LONG& lPrevState)
	{
		return NtSetEvent(m_hObj, &lPrevState);
	}

	EckInline NTSTATUS NoSignal()
	{
		return NtClearEvent(m_hObj);
	}

	EckInline NTSTATUS NoSignal(LONG& lPrevState)
	{
		return NtResetEvent(m_hObj, &lPrevState);
	}
};
ECK_NAMESPACE_END