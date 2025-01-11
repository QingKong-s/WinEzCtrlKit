#pragma once
#include "ECK.h"

ECK_NAMESPACE_BEGIN
class CTimerQueue
{
private:
	HANDLE m_hQueue{};
public:
	ECK_DISABLE_COPY_MOVE(CTimerQueue)
public:
	EckInline CTimerQueue()
	{
		RtlCreateTimerQueue(&m_hQueue);
	}

	EckInline ~CTimerQueue()
	{
		RtlDeleteTimerQueue(m_hQueue);
	}

	EckInline NTSTATUS Delete(HANDLE hEvent)
	{
		const auto nts = RtlDeleteTimerQueueEx(m_hQueue, hEvent);
		m_hQueue = nullptr;
		return nts;
	}

	EckInline NTSTATUS Delete()
	{
		const auto nts = RtlDeleteTimerQueue(m_hQueue);
		m_hQueue = nullptr;
		return nts;
	}

	EckInline [[nodiscard]] constexpr HANDLE GetHQueue() const { return m_hQueue; }
};

class CQueueTimer
{
private:
	HANDLE m_hTimer{};
	HANDLE m_hQueue{};
public:
	ECK_DISABLE_COPY_MOVE_DEF_CONS(CQueueTimer)
public:
	EckInline NTSTATUS SetTimer(WAITORTIMERCALLBACK pfnCallBack, void* pParam,
		DWORD dwDueTime, DWORD dwPeriod = std::numeric_limits<DWORD>::max(), UINT uFlags = 0u)
	{
		EckAssert(!m_hTimer);
		if (dwPeriod == std::numeric_limits<DWORD>::max())
			dwPeriod = dwDueTime;
		return RtlCreateTimer(m_hQueue, &m_hTimer, pfnCallBack, pParam,
			dwDueTime, dwPeriod, uFlags);
	}

	EckInline BOOL KillTimer(HANDLE hEvent = INVALID_HANDLE_VALUE)
	{
		HANDLE hTimer{};
		std::swap(m_hTimer, hTimer);
		return RtlDeleteTimer(m_hQueue, hTimer, hEvent);
	}

	EckInline BOOL ChangeTimer(DWORD dwDueTime, DWORD dwPeriod)
	{
		return RtlUpdateTimer(m_hQueue, m_hTimer, dwDueTime, dwPeriod);
	}

	EckInline constexpr void SetHQueue(HANDLE hQueue) { EckAssert(!m_hTimer); m_hQueue = hQueue; }

	EckInline [[nodiscard]] constexpr HANDLE GetHQueue() const { return m_hQueue; }

	EckInline [[nodiscard]] constexpr HANDLE GetHTimer() const { return m_hTimer; }

	EckInline [[nodiscard]] constexpr BOOL IsValid() const { return !!m_hTimer; }
};
ECK_NAMESPACE_END