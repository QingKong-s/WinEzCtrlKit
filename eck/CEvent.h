/*
* WinEzCtrlKit Library
*
* CEvent.h ： 事件对象
*
* Copyright(C) 2024 QingKong
*/
#pragma once
#include "ECK.h"

ECK_NAMESPACE_BEGIN
class CEvent
{
private:
	HANDLE m_h = nullptr;
public:
	ECK_DISABLE_COPY_MOVE(CEvent)
public:
	EckInline CEvent()
	{
		OBJECT_ATTRIBUTES oa;
		InitOA(oa);
		NtCreateEvent(&m_h, EVENT_ALL_ACCESS, &oa, SynchronizationEvent, FALSE);
	}

	EckInline CEvent(PCWSTR pszName, BOOL bManualReset, BOOL bInitSignal, BOOL bInheritHandle = FALSE)
	{
		UNICODE_STRING us;
		if (pszName)
			RtlInitUnicodeString(&us, pszName);
		OBJECT_ATTRIBUTES oa;
		InitOA(oa, (pszName ? &us : nullptr), (bInheritHandle ? OBJ_INHERIT : 0));
		NtCreateEvent(&m_h, EVENT_ALL_ACCESS, &oa,
			bManualReset ? NotificationEvent : SynchronizationEvent, FALSE);
	}

	EckInline ~CEvent()
	{
		NtClose(m_h);
	}

	EckInline NTSTATUS Signal(LONG* lPrevState = nullptr)
	{
		return NtSetEvent(m_h, lPrevState);
	}

	EckInline NTSTATUS NoSignal(LONG* lPrevState = nullptr)
	{
		return NtResetEvent(m_h, lPrevState);
	}

	EckInline HANDLE GetHEvent() const { return m_h; }
};
ECK_NAMESPACE_END