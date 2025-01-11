#pragma once
#include "ECK.h"

ECK_NAMESPACE_BEGIN
class CCriticalSection
{
private:
	RTL_CRITICAL_SECTION m_cs;
public:
	ECK_DISABLE_COPY_MOVE(CCriticalSection);
	EckInline CCriticalSection(DWORD dwSpinCount)
	{
		RtlInitializeCriticalSectionAndSpinCount(&m_cs, dwSpinCount);
	}

	EckInline CCriticalSection()
	{
		RtlInitializeCriticalSection(&m_cs);
	}

	EckInline ~CCriticalSection()
	{
		RtlDeleteCriticalSection(&m_cs);
	}

	EckInline _Acquires_lock_(m_cs) void Enter()
	{
		RtlEnterCriticalSection(&m_cs);
	}

	EckInline BOOL TryEnter()
	{
		RtlTryEnterCriticalSection(&m_cs);
	}

	EckInline _Releases_lock_(m_cs) void Leave()
	{
		RtlLeaveCriticalSection(&m_cs);
	}

	EckInline auto GetPCs() { return &m_cs; }
};

class CCsGuard
{
private:
	CCriticalSection& x;
public:
	ECK_DISABLE_COPY_MOVE(CCsGuard);
	EckInline CCsGuard(CCriticalSection& x) :x{ x }
	{
		x.Enter();
	}

	EckInline ~CCsGuard()
	{
		x.Leave();
	}
};
ECK_NAMESPACE_END