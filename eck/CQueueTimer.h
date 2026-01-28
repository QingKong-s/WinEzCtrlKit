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
    CTimerQueue() noexcept
    {
        RtlCreateTimerQueue(&m_hQueue);
    }

    ~CTimerQueue()
    {
        RtlDeleteTimerQueue(m_hQueue);
    }

    EckInline NTSTATUS Delete(HANDLE hEvent) noexcept
    {
        const auto nts = RtlDeleteTimerQueueEx(m_hQueue, hEvent);
        m_hQueue = nullptr;
        return nts;
    }

    EckInline NTSTATUS Delete() noexcept
    {
        const auto nts = RtlDeleteTimerQueue(m_hQueue);
        m_hQueue = nullptr;
        return nts;
    }

    EckInlineNdCe HANDLE GetHQueue() const noexcept { return m_hQueue; }
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
        UINT uDueTime, UINT uPeriod = std::numeric_limits<UINT>::max(), UINT uFlags = 0u) noexcept
    {
        EckAssert(!m_hTimer);
        if (uPeriod == std::numeric_limits<DWORD>::max())
            uPeriod = uDueTime;
        return RtlCreateTimer(m_hQueue, &m_hTimer, pfnCallBack, pParam,
            uDueTime, uPeriod, uFlags);
    }

    EckInline BOOL KillTimer(HANDLE hEvent = INVALID_HANDLE_VALUE) noexcept
    {
        HANDLE hTimer{};
        std::swap(m_hTimer, hTimer);
        return RtlDeleteTimer(m_hQueue, hTimer, hEvent);
    }

    EckInline BOOL ChangeTimer(UINT uDueTime, UINT uPeriod) noexcept
    {
        return RtlUpdateTimer(m_hQueue, m_hTimer, uDueTime, uPeriod);
    }

    EckInlineCe void SetHQueue(HANDLE hQueue) noexcept { EckAssert(!m_hTimer); m_hQueue = hQueue; }
    EckInlineNdCe HANDLE GetHQueue() const noexcept { return m_hQueue; }
    EckInlineNdCe HANDLE GetHTimer() const noexcept { return m_hTimer; }
    EckInlineNdCe BOOL IsValid() const noexcept { return !!m_hTimer; }
};
ECK_NAMESPACE_END