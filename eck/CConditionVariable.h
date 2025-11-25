#pragma once
#include "CCriticalSection.h"
#include "CSrwLock.h"

ECK_NAMESPACE_BEGIN
class CConditionVariable
{
private:
	RTL_CONDITION_VARIABLE m_cv;
public:
	ECK_DISABLE_COPY_MOVE(CConditionVariable)
public:
	CConditionVariable()
	{
		RtlInitializeConditionVariable(&m_cv);
	}

	EckInline NTSTATUS SleepLi(RTL_CRITICAL_SECTION* pcs, LARGE_INTEGER* pli = nullptr)
	{
		return RtlSleepConditionVariableCS(&m_cv, pcs, pli);
	}
	EckInline NTSTATUS SleepLi(RTL_SRWLOCK* psrw, BOOL bReadLock, LARGE_INTEGER* pli = nullptr)
	{
		return RtlSleepConditionVariableSRW(&m_cv, psrw, pli,
			bReadLock ? CONDITION_VARIABLE_LOCKMODE_SHARED : 0);
	}

	EckInline NTSTATUS Sleep100ns(RTL_CRITICAL_SECTION* pcs, LONGLONG ll)
	{
		ll *= -1;
		return SleepLi(pcs, (LARGE_INTEGER*)&ll);
	}
	EckInline NTSTATUS Sleep100ns(RTL_SRWLOCK* psrw, BOOL bReadLock, LONGLONG ll)
	{
		ll *= -1;
		return SleepLi(psrw, bReadLock, (LARGE_INTEGER*)&ll);
	}

	EckInline NTSTATUS Sleep(RTL_CRITICAL_SECTION* pcs, LONGLONG ll)
	{
		return Sleep100ns(pcs, ll * 10000ll);
	}
	EckInline NTSTATUS Sleep(RTL_SRWLOCK* psrw, BOOL bReadLock, LONGLONG ll)
	{
		return Sleep100ns(psrw, bReadLock, ll * 10000ll);
	}

	EckInline void Wake()
	{
		RtlWakeConditionVariable(&m_cv);
	}

	EckInline void WakeAll()
	{
		RtlWakeAllConditionVariable(&m_cv);
	}

	EckInline auto GetPtr() { return &m_cv; }
};
ECK_NAMESPACE_END