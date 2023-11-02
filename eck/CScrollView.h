#pragma once
#include "Utility.h"

#include <functional>
#include <numbers>

ECK_NAMESPACE_BEGIN
class CScrollView
{
protected:
	int m_iMin = 0;
	int m_iMax = 0;
	int m_iPage = 0;
	int m_iPos = 0;

	int m_iViewSize = 0;
public:
	int GetMin() const
	{
		return m_iMin;
	}

	void SetMin(int iMin)
	{
		m_iMin = iMin;
	}

	int GetMax() const
	{
		return m_iMax;
	}

	void SetMax(int iMax)
	{
		m_iMax = iMax;
	}

	int GetPage() const
	{
		return m_iPage;
	}

	void SetPage(int iPage)
	{
		m_iPage = iPage;
	}

	int GetPos() const
	{
		return m_iPos;
	}

	void SetPos(int iPos)
	{
		m_iPos = iPos;
	}

	void SetRange(int iMin, int iMax)
	{
		m_iMin = iMin;
		m_iMax = iMax;
	}

	void OnMouseWheel(int iDelta)
	{
		int iPos = m_iPos - iDelta * 60;
		if (iPos < m_iMin)
			iPos = m_iMin;
		else if (iPos > m_iMax - m_iPage)
			iPos = m_iMax - m_iPage;

		SetPos(iPos);
	}

	float GetPrecent() const
	{
		int x = m_iMax - m_iMin - m_iPage;
		if (x <= 0)
			return 0.f;
		else
			return (float)m_iPos / (float)x;
	}
};

class CInertialScrollView : public CScrollView
{
public:
	using InertialScrollProc = void (*)(int, int, LPARAM);
protected:
	int m_iTarget = 0;
	int m_iCurrPos = 0;
	int m_iStart = 0;

	int m_iDistance = 0;

	int m_iDuration = 400;

	HWND m_hWnd = NULL;
	UINT m_uTimerID = 0;

	int m_iSustain = 0;

	InertialScrollProc m_pfnInertialScroll = NULL;
	LPARAM m_lParam = 0;

	static UINT s_uTimerNotify;
public:
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
	{
		if (uMsg == s_uTimerNotify)
		{
			TimerProc(hWnd, 0, 0, 0);
			return 0;
		}

		return DefSubclassProc(hWnd, uMsg, wParam, lParam);
	}
	void SetHWND(HWND hWnd)
	{
		s_uTimerNotify = RegisterWindowMessageW(L"wedfgrshgehrwgeurwghuhgvuer");

		m_hWnd = hWnd;
		SetPropW(m_hWnd, L"Eck.Prop.SV", this);

		SetWindowSubclass(m_hWnd, WndProc, 455646, 0);

		CRTCreateThread([](void* pParam)->UINT
			{
				while (TRUE)
				{
					PostMessageW((HWND)pParam, s_uTimerNotify, 0, 0);
					Sleep(20);
				}
				return 0;
			}, m_hWnd);
	}

	void SetTimerID(UINT uTimerID)
	{
		m_uTimerID = uTimerID;
	}

	static float OutCircle(float fCurrTime, float fStart, float fDistance, float fDuration)
	{
		fCurrTime = fCurrTime / fDuration - 1.f;
		return fDistance * sqrt(1 - fCurrTime * fCurrTime) + fStart;
	}

	static float OutSine(float fCurrTime, float fStart, float fDistance, float fDuration)
	{
		return fDistance * sinf(fCurrTime / fDuration * std::numbers::pi_v<float> / 2.f) + fStart;
	}

	static float OutCubic(float fCurrTime, float fStart, float fDistance, float fDuration)
	{
		fCurrTime = fCurrTime / fDuration - 1.f;
		return fDistance * (fCurrTime * fCurrTime * fCurrTime + 1.f) + fStart;
	}

	static float OutExpo(float fCurrTime, float fStart, float fDistance, float fDuration)
	{
		if (fCurrTime == fDuration)
			return fStart + fDistance;
		return fDistance * (-powf(-10.f * fCurrTime / fDuration, 2.f) + 1.f) + fStart;
	}

	static void CALLBACK TimerProc(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
	{
		auto p = (CInertialScrollView*)GetPropW(hWnd, L"Eck.Prop.SV");
		if (!p)
			return;
		//if (idEvent != p->m_uTimerID)
		//	return;
		

		const int iPrevPos = p->m_iPos;
		if (p->m_iDistance == 0)
			return;
		//if (iDistance == 0)
		//	KillTimer(hWnd, p->m_uTimerID);
		p->m_iSustain += 20;
		int iCurr = (int)OutCubic((float)p->m_iSustain, (float)p->m_iStart, (float)p->m_iDistance, 400);
		p->SetPos(iCurr);
		p->m_pfnInertialScroll(p->m_iPos, iPrevPos, p->m_lParam);
		if (p->m_iSustain >= 400)
			p->m_iDistance = 0;
	}

	void OnMouseWheel2(int iDelta, InertialScrollProc pfnInertialScroll, LPARAM lParam)
	{
		m_pfnInertialScroll = pfnInertialScroll;
		m_lParam = lParam;

		m_iStart = GetPos();
		m_iDistance += 50 * iDelta;
		if (m_iDistance + m_iStart > m_iMax)
			m_iDistance = m_iMax - m_iStart;
		if (m_iDistance + m_iStart < m_iMin)
			m_iDistance = m_iMin - m_iStart;
		m_iSustain = 0;
		m_iDuration = m_iDistance * 4;

		//SetTimer(m_hWnd, m_uTimerID, 40, TimerProc);
	}
};

ECK_NAMESPACE_END