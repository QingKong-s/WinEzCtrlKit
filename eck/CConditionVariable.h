#pragma once
#include "CCriticalSection.h"
#include "CSrwLock.h"

ECK_NAMESPACE_BEGIN
class CConditionVariable
{
private:
	CONDITION_VARIABLE m_cv;
public:
	ECK_DISABLE_COPY_MOVE(CConditionVariable)
public:
	EckInline CConditionVariable()
	{
		InitializeConditionVariable(&m_cv);
	}

	EckInline BOOL Sleep(CCriticalSection& cs, DWORD dwTimeout)
	{
		return SleepConditionVariableCS(&m_cv, cs.GetPCs(), dwTimeout);
	}

	EckInline BOOL Sleep(CSrwLock& srw, DWORD dwTimeout, BOOL bReadLock)
	{
		return SleepConditionVariableSRW(&m_cv, srw.GetPSrw(),
			dwTimeout, bReadLock ? CONDITION_VARIABLE_LOCKMODE_SHARED : 0);
	}

	EckInline void Wake()
	{
		WakeConditionVariable(&m_cv);
	}

	EckInline void WakeAll()
	{
		WakeAllConditionVariable(&m_cv);
	}

	EckInline auto GetPCv() { return &m_cv; }
};
ECK_NAMESPACE_END