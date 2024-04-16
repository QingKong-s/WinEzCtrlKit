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
	HANDLE m_h = NULL;
public:
	ECK_DISABLE_COPY_MOVE(CEvent)
public:
	EckInline CEvent()
	{
		m_h = CreateEventW(NULL, FALSE, FALSE, NULL);
	}

	EckInline CEvent(SECURITY_ATTRIBUTES* psa, BOOL bManualReset, BOOL bInitSignal, PCWSTR pszName)
	{
		m_h = CreateEventW(psa, bManualReset, bInitSignal, pszName);
	}

	EckInline ~CEvent()
	{
		CloseHandle(m_h);
	}

	EckInline BOOL Signal()
	{
		return SetEvent(m_h);
	}

	EckInline BOOL NoSignal()
	{
		return ResetEvent(m_h);
	}

	EckInline HANDLE GetHEvent() const { return m_h; }
};
ECK_NAMESPACE_END