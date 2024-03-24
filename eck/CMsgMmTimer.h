#pragma once
#include "ECK.h"

ECK_NAMESPACE_BEGIN
class CMsgMmTimer
{
private:
	MMRESULT m_ID = 0;
	HWND m_hWnd = NULL;
	LPARAM m_lParam = 0;
	UINT m_uNotifyMsg = 0;
	

	static void CALLBACK TimerProc(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2)
	{
		auto p = (CMsgMmTimer*)dwUser;
		p->pfn(0, 0, 0, 0, 0, p->m_lParam);
		//PostMessageW(p->m_hWnd, p->m_uNotifyMsg, uTimerID, p->m_lParam);
	}
public:
	SUBCLASSPROC pfn = 0;
	CMsgMmTimer() = default;
	CMsgMmTimer(HWND hWnd, UINT uMsg) :m_hWnd{ hWnd }, m_uNotifyMsg{ uMsg }
	{}

	~CMsgMmTimer()
	{
		KillTimer();
	}

	EckInline void SetHWND(HWND hWnd) { m_hWnd = hWnd; }

	EckInline HWND GetHWND() const { return m_hWnd; }

	EckInline void SetMsg(UINT uMsg) { m_uNotifyMsg = uMsg; }

	EckInline MMRESULT SetTimer(UINT uElapse, UINT uResolution = 20)
	{
		KillTimer();
		return m_ID = timeSetEvent(uElapse, uResolution, TimerProc, (DWORD_PTR)this, 
			TIME_PERIODIC | TIME_KILL_SYNCHRONOUS);
	}

	EckInline void KillTimer()
	{
		if (m_ID)
		{
			const auto idOld = m_ID;
			timeKillEvent(m_ID);
			m_ID = 0;
			MSG msg;
			while (PeekMessageW(&msg, NULL, 0u, 0u, PM_REMOVE))
			{
				if (msg.message == WM_QUIT)
				{
					PostQuitMessage((int)msg.wParam);
					return;
				}
				else if (msg.message == m_uNotifyMsg && msg.wParam == idOld)
					continue;
				TranslateMessage(&msg);
				DispatchMessageW(&msg);
			}
		}
	}

	EckInline MMRESULT GetID() const { return m_ID; }

	EckInline void SetParam(LPARAM lParam) { m_lParam = lParam; }

	EckInline LPARAM GetParam() const { return m_lParam; }
};
ECK_NAMESPACE_END