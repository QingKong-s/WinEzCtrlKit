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
	int m_iMinThumbSize = 0;

	int m_oxyThumbCursor = 0;
#ifdef _DEBUG
	BOOL m_bLBtnDown = FALSE;
#endif // _DEBUG

	void RangeChanged()
	{
		if (!IsValid())
		{
			SetPosUncheck(GetMin());
			return;
		}
		if (GetPos() < GetMin())
			SetPosUncheck(GetMin());
		else if (GetPos() > GetMaxWithPage())
			SetPosUncheck(GetMaxWithPage());
	}
public:
	EckInline int GetMin() const { return m_iMin; }

	void SetMin(int iMin)
	{
		m_iMin = iMin;
		RangeChanged();
	}

	EckInline int GetMax() const { return m_iMax; }

	EckInline int GetMaxWithPage() const { return m_iMax - m_iPage + 1; }

	void SetMax(int iMax)
	{
		m_iMax = iMax;
		RangeChanged();
	}

	EckInline int GetPage() const { return m_iPage; }

	void SetPage(int iPage)
	{
		m_iPage = iPage;
		RangeChanged();
	}

	EckInline int GetPos() const { return m_iPos; }

	void SetPos(int iPos)
	{
		m_iPos = iPos;
		RangeChanged();
	}

	void SetPosUncheck(int iPos)
	{
		m_iPos = iPos;
	}

	void SetRange(int iMin, int iMax)
	{
		m_iMin = iMin;
		m_iMax = iMax;
		RangeChanged();
	}

	int GetRangeDistance() const
	{
		return GetMaxWithPage() - GetMin();
	}

	BOOL IsValid() const { return GetRangeDistance() > 0; }

	void OnMouseWheel(int iDelta)
	{
		if (!IsValid())
			return;
		int iPos = GetPos() + iDelta;
		if (iPos < GetMin())
			iPos = GetMin();
		else if (iPos > GetMaxWithPage())
			iPos = GetMaxWithPage();

		SetPos(iPos);
	}

	float GetPrecent() const
	{
		const int d = GetMaxWithPage() - GetMin();
		if (d <= 0)
			return 0.f;
		else
			return (float)(GetPos() - GetMin()) / (float)d;
	}

	EckInline void SetViewSize(int iViewSize) { m_iViewSize = iViewSize; }

	EckInline int GetViewSize() const { return m_iViewSize; }

	EckInline void SetMinThumbSize(int iMinThumbSize) { m_iMinThumbSize = iMinThumbSize; }

	EckInline int GetMinThumbSize() const { return m_iMinThumbSize; }

	EckInline int GetThumbSize() const
	{
		const int d = GetMax() - GetMin();
		if (d <= 0)
			return std::numeric_limits<int>::min();
		const int i = GetPage() * GetViewSize() / d;
		return std::max(GetMinThumbSize(), i);
	}

	EckInline int GetThumbPos(int iThumbSize) const
	{
		const int d = GetRangeDistance();
		if (d <= 0)
			return std::numeric_limits<int>::min();
		return (GetPos() - GetMin()) * (GetViewSize() - iThumbSize) / d;
	}

	EckInline int GetThumbPos() const { return GetThumbPos(GetThumbSize()); }

	EckInline void OnLButtonDown(int xy)
	{
#ifdef _DEBUG
		EckAssert(!m_bLBtnDown);
		m_bLBtnDown = TRUE;
#endif // _DEBUG
		if (!IsValid())
			return;
		EckAssert(xy >= GetThumbPos() && xy <= GetThumbPos() + GetThumbSize());
		m_oxyThumbCursor = xy - GetThumbPos();
	}

	EckInline void OnMouseMove(int xy)
	{
		EckAssert(m_bLBtnDown);
		if (!IsValid())
			return;
		SetPos(GetMin() +
			(xy - m_oxyThumbCursor) *
			GetRangeDistance() /
			(GetViewSize() - GetThumbSize()));
	}

	EckInline void OnLButtonUp()
	{
#ifdef _DEBUG
		EckAssert(m_bLBtnDown);
		m_bLBtnDown = FALSE;
#endif // _DEBUG
	}
};

class CInertialScrollView : public CScrollView
{
public:
	using InertialScrollProc = void (*)(int iPos, int iPrevPos, LPARAM lParam);
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

	static LRESULT CALLBACK SubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
	{
		auto p = (CInertialScrollView*)dwRefData;
		if (uMsg == s_uTimerNotify && wParam == p->m_Timer.GetID())
		{
			const int iPrevPos = p->GetPos();
			p->m_iSustain += 20;
			int iCurr = (int)Easing::FOutCubic{}((float)p->m_iSustain, (float)p->m_iStart, (float)p->m_iDistance, (float)p->m_iDuration);
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
		SmoothScrollDelta(m_iDelta * iDelta, pfnInertialScroll, lParam);
	}

	void InterruptAnimation()
	{
		const auto iId = m_Timer.GetID();
		if (!iId)
			return;
		m_Timer.KillTimer();
		m_iSustain = 0;
		m_iDistance = 0;
	}

	void SmoothScrollDelta(int iDelta, InertialScrollProc pfnInertialScroll, LPARAM lParam)
	{
		m_pfnInertialScroll = pfnInertialScroll;
		m_lParam = lParam;

		m_iStart = GetPos();
		// 计算新的滚动距离；将原先的滚动距离减去已经滑动完的位移再加上滚动事件产生的位移
		m_iDistance = ((m_iDuration - m_iSustain) * m_iDistance / m_iDuration) + iDelta;
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
inline UINT CInertialScrollView::s_uTimerNotify = RegisterWindowMessageW(MSG_INERTIALSV);
ECK_NAMESPACE_END