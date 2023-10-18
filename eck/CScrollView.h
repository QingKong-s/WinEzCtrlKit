#pragma once
#include "ECK.h"

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
	HANDLE m_hTimer = NULL;
	int m_iCurrSpeed = 0;
	int m_iUnitSpeed = 10;
	int m_iAccel = 1;
	int m_iPrevPos = 0;
	HWND m_hWnd = NULL;
	UINT m_uTimerID = 0;

	int m_iRealAccel = 0;
	int m_cFrame = 0;
	int m_iDestPos = 0;
	int m_iSrcPos = 0;
	int m_v0 = 0;

	InertialScrollProc m_pfnInertialScroll = NULL;
	LPARAM m_lParam = 0;

	BOOL m_bStop = TRUE;
public:
	void SetHWND(HWND hWnd)
	{
		m_hWnd = hWnd;
	}

	void SetTimerID(UINT uTimerID)
	{
		m_uTimerID = uTimerID;
	}

	static void CALLBACK TimerProc(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
	{
		auto p = (CInertialScrollView*)GetPropW(hWnd, L"Eck.Prop.SV");
		if (!p)
			return;
		if (idEvent != p->m_uTimerID)
			return;
		int iMidPos = p->m_iSrcPos + p->m_v0 * p->m_cFrame + p->m_iRealAccel * p->m_cFrame * p->m_cFrame / 2;
		p->m_iCurrSpeed += p->m_iRealAccel;
		
		BOOL bStop = FALSE;
		if (iMidPos < p->m_iMin)
		{
			bStop = TRUE;
			iMidPos = p->m_iMin;
		}
		else if (iMidPos > p->m_iMax - p->m_iPage)
		{
			bStop = TRUE;
			iMidPos = p->m_iMax - p->m_iPage;
		}
		if ((p->m_iCurrSpeed <= 0 && p->m_iRealAccel < 0) ||
			(p->m_iCurrSpeed >= 0 && p->m_iRealAccel > 0))
			bStop = TRUE;


		p->m_iPos = iMidPos;
		p->m_pfnInertialScroll(iMidPos, p->m_iPrevPos, p->m_lParam);
		p->m_iPrevPos = p->m_iPos;
		++p->m_cFrame;
		if (bStop)
		{
			p->m_bStop = TRUE;
			p->m_iCurrSpeed = 0;
			KillTimer(hWnd, p->m_uTimerID);
			RemovePropW(hWnd, L"Eck.Prop.SV");
		}
	}

	void OnMouseWheel2(int iDelta, InertialScrollProc pfnInertialScroll, LPARAM lParam)
	{
		int iOrgPos = m_iPos;
		m_iPrevPos = iOrgPos;
		m_iSrcPos= iOrgPos;
		int iPos = m_iPos - iDelta*60;
		if (iPos < m_iMin)
			iPos = m_iMin;
		else if (iPos > m_iMax - m_iPage)
			iPos = m_iMax - m_iPage;
		m_iDestPos = iPos;
		//int iMidPos;

		if (eck::Sign(m_iRealAccel) != eck::Sign(iDelta))
			m_iCurrSpeed = m_iUnitSpeed * -iDelta;
		else
			m_iCurrSpeed += (m_iUnitSpeed * -iDelta);

		if (m_iCurrSpeed > 200)
			m_iCurrSpeed = 200;
		
		m_v0 = m_iCurrSpeed;
		int iAccel = (iDelta > 0 ? m_iAccel : -m_iAccel);
		m_iRealAccel = iAccel;

		m_pfnInertialScroll = pfnInertialScroll;
		m_lParam = lParam;
		m_cFrame = 0;

		if(m_bStop)
		{
			SetPropW(m_hWnd, L"Eck.Prop.SV", this);
			SetTimer(m_hWnd, m_uTimerID, 40, TimerProc);
		}

		SetPos(iPos);
	}
};
ECK_NAMESPACE_END