#pragma once
#include "CWaitableTimer.h"
#include "CEvent.h"

ECK_NAMESPACE_BEGIN
class CMessageTimer
{
private:
    HWND m_hWnd{};
    UINT m_uMsg{};
    UINT m_msInterval : 31{};
    UINT m_bExit : 1{};
    CWaitableTimer m_Timer{};
    CEvent m_Event{ {}, TRUE/*Notification*/, TRUE/*Signal*/ };
    CWaitableObject m_Thread{};
public:
    EckInlineCe void SetTargetWindow(HWND hWnd) noexcept { m_hWnd = hWnd; }
    EckInlineCe void SetMessageValue(UINT uMsg) noexcept { m_uMsg = uMsg; }

    EckInlineCe void SetInterval(UINT ms) noexcept { m_msInterval = ms; }
    EckInlineNdCe UINT GetInterval() const noexcept { return m_msInterval; }

    void Start() noexcept
    {
        if (m_Thread.Get())
            return;
        m_Thread.Attach(CrtCreateThread([](void* p_) -> UINT
            {
                const auto p = (CMessageTimer*)p_;
                while (!p->m_bExit)
                {
                    const auto ms = (long)p->m_msInterval;
                    p->m_Timer.SetDueTimeAndPeriod(ms, ms);
                    NtWaitForMultipleObjects(2, (HANDLE*)&p->m_Timer,
                        WaitAll, FALSE, nullptr);
                    SendNotifyMessageW(p->m_hWnd, p->m_uMsg, 0, 0);
                }
                return 0u;
            }, this));
    }

    void Pause() noexcept { m_Event.NoSignal(); }
    void Resume() noexcept { m_Event.Signal(); }

    void Stop() noexcept
    {
        m_bExit = TRUE;
        m_Event.Signal();
        WaitObject(m_Thread);
        m_Thread.Clear();
    }

    EckInlineNdCe BOOL IsRunning() const noexcept { return !!m_Thread.Get(); }
};
ECK_NAMESPACE_END