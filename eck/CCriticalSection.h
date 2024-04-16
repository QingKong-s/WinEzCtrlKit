/*
* WinEzCtrlKit Library
*
* CCriticalSection.h ： 临界区
*
* Copyright(C) 2024 QingKong
*/
#pragma once
#include "ECK.h"

ECK_NAMESPACE_BEGIN
class CCriticalSection
{
private:
	CRITICAL_SECTION* m_pcs;
public:
	EckInline CCriticalSection(DWORD dwSpinCount)
	{
		m_pcs = new CRITICAL_SECTION;
		InitializeCriticalSectionEx(m_pcs, dwSpinCount, 0);
	}

	EckInline CCriticalSection()
	{
		m_pcs = new CRITICAL_SECTION;
		InitializeCriticalSection(m_pcs);
	}

	EckInline ~CCriticalSection()
	{
		DeleteCriticalSection(m_pcs);
		delete m_pcs;
	}

	EckInline _Acquires_lock_(*m_pcs) void Enter()
	{
		EnterCriticalSection(m_pcs);
	}

	EckInline BOOL TryEnter()
	{
		TryEnterCriticalSection(m_pcs);
	}

	EckInline _Releases_lock_(*m_pcs) void Leave()
	{
		LeaveCriticalSection(m_pcs);
	}

	EckInline auto GetPCs() { return m_pcs; }
};

class CCsGuard
{
private:
	CCriticalSection& m_cs;
public:
	EckInline CCsGuard(CCriticalSection& cs) :m_cs{ cs }
	{
		m_cs.Enter();
	}

	EckInline ~CCsGuard()
	{
		m_cs.Leave();
	}
};
ECK_NAMESPACE_END