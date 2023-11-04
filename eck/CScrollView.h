#pragma once
#include "Utility.h"
#include "CMsgMmTimer.h"
#include "EasingCurve.h"

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

	int GetMaxWithPage() const
	{
		return m_iMax - m_iPage - 1;
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
	CMsgMmTimer m_Timer{};

	int m_iStart = 0;		// 起始位置
	int m_iDistance = 0;	// 当前动画应滚动的总距离
	int m_iSustain = 0;		// 持续时间，毫秒
	int m_iDuration = 400;	// 动画总时间
	int m_iDelta = 80;		// 每次滚动的距离

	InertialScrollProc m_pfnInertialScroll = NULL;
	LPARAM m_lParam = 0;

	static UINT s_uTimerNotify;
public:
	CInertialScrollView()
	{
		m_Timer.SetMsg(s_uTimerNotify);
	}

	static LRESULT CALLBACK SubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
	{
		auto p = (CInertialScrollView*)dwRefData;
		if (uMsg == s_uTimerNotify && wParam == p->m_Timer.GetID())
		{
			const int iPrevPos = p->GetPos();
			p->m_iSustain += 20;
			int iCurr = (int)OutCubic((float)p->m_iSustain, (float)p->m_iStart, (float)p->m_iDistance, (float)p->m_iDuration);
			if (iCurr > p->GetMaxWithPage())
			{
				p->SetPos(p->GetMaxWithPage());
				p->m_pfnInertialScroll(p->GetPos(), iPrevPos, p->m_lParam);
				p->m_iDistance = 0;
				p->m_Timer.KillTimer();
				return 0;
			}
			p->SetPos(iCurr);
			p->m_pfnInertialScroll(p->m_iPos, iPrevPos, p->m_lParam);
			if (p->m_iSustain >= p->m_iDuration)
			{
				p->m_iDistance = 0;
				p->m_Timer.KillTimer();
			}
			return 0;
		}

		return DefSubclassProc(hWnd, uMsg, wParam, lParam);
	}

	void SetHWND(HWND hWnd)
	{
		if (m_Timer.GetHWND() == hWnd)
			return;
		m_Timer.SetHWND(hWnd);
		RemoveWindowSubclass(hWnd, SubclassProc, SCID_INERTIALSCROLLVIEW);
		SetWindowSubclass(hWnd, SubclassProc, SCID_INERTIALSCROLLVIEW, (DWORD_PTR)this);
	}

	void OnMouseWheel2(int iDelta, InertialScrollProc pfnInertialScroll, LPARAM lParam)
	{
		m_pfnInertialScroll = pfnInertialScroll;
		m_lParam = lParam;

		m_iStart = GetPos();
		// 计算新的滚动距离；将原先的滚动距离减去已经滑动完的位移再加上滚动事件产生的位移
		m_iDistance = ((m_iDuration - m_iSustain) * m_iDistance / m_iDuration) + (m_iDelta * iDelta);
		if (m_iDistance + m_iStart > GetMaxWithPage())
			m_iDistance = GetMaxWithPage() - m_iStart;
		if (m_iDistance + m_iStart < GetMin())
			m_iDistance = GetMin() - m_iStart;
		m_iSustain = 0;
		if (!m_Timer.GetID())
			m_Timer.SetTimer(20);
	}

	EckInline void SetDuration(int iDuration) { m_iDuration = iDuration; }

	EckInline void SetDelta(int iDelta) { m_iDelta = iDelta; }
};
ECK_COMDAT UINT CInertialScrollView::s_uTimerNotify = RegisterWindowMessageW(MSG_INERTIALSV);
ECK_NAMESPACE_END