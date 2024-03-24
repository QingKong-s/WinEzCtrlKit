#pragma once
#include "ECK.h"

ECK_NAMESPACE_BEGIN
class CTimerQueue
{
private:
	HANDLE m_hQueue = NULL;
public:
	ECK_DISABLE_COPY_MOVE(CTimerQueue)
public:
	EckInline CTimerQueue()
	{
		m_hQueue = CreateTimerQueue();
	}

	EckInline ~CTimerQueue()
	{
		(void)DeleteTimerQueue(m_hQueue);
	}

	EckInline BOOL Delete(HANDLE hEvent)
	{
		const auto b = DeleteTimerQueueEx(m_hQueue, hEvent);
		m_hQueue = NULL;
		return b;
	}
};

class CQueueTimer
{
private:
	HANDLE m_hTimer = NULL;
	HANDLE m_hQueue = NULL;
public:
	ECK_DISABLE_COPY_MOVE_DEF_CONS(CQueueTimer)
public:
	EckInline BOOL SetTimer(WAITORTIMERCALLBACK pfnCallBack, void* pParam,
		DWORD dwDueTime, DWORD dwPeriod = std::numeric_limits<DWORD>::max(), UINT uFlags = 0u)
	{
		EckAssert(!m_hTimer);
		if (dwPeriod == std::numeric_limits<DWORD>::max())
			dwPeriod = dwDueTime;
		return CreateTimerQueueTimer(&m_hTimer, m_hQueue, pfnCallBack, pParam, 
			dwDueTime, dwPeriod, uFlags);
	}

	EckInline BOOL KillTimer(HANDLE hEvent = INVALID_HANDLE_VALUE)
	{
		HANDLE hTimer{};
		std::swap(m_hTimer, hTimer);
		return DeleteTimerQueueTimer(m_hQueue, hTimer, hEvent);
	}

	EckInline BOOL ChangeTimer(DWORD dwDueTime, DWORD dwPeriod)
	{
		return ChangeTimerQueueTimer(m_hQueue, m_hTimer, dwDueTime, dwPeriod);
	}

	EckInline void SetHQueue(HANDLE hQueue) { EckAssert(!m_hTimer); m_hQueue = hQueue; }

	EckInline HANDLE GetHQueue() const { return m_hQueue; }

	EckInline HANDLE GetHTimer() const { return m_hTimer; }

	[[nodiscard]] EckInline BOOL IsValid() const { return !!m_hTimer; }
};
ECK_NAMESPACE_END