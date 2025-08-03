#pragma once
#include "CScrollView.h"
#include "ITimeLine.h"
#include "EasingCurve.h"

ECK_NAMESPACE_BEGIN
class CInertialScrollView :
	public CScrollView,
	public CUnknown<CInertialScrollView, ITimeLine>
{
public:
	using FInertialScrollProc = void(*)(int iPos, int iPrevPos, LPARAM lParam);
protected:
	float m_iStart = 0;		// 起始位置
	float m_iDistance = 0;	// 当前动画应滚动的总距离
	float m_iSustain = 0;	// 持续时间，毫秒
	float m_iDuration = 400;// 动画总时间
	int m_iDelta = 80;	// 每次滚动的距离

	int m_iCurrInterval = 0;

	FInertialScrollProc m_pfnCallBack = nullptr;
	LPARAM m_lParam = 0;

	BOOLEAN m_bValid = FALSE;
	BOOLEAN m_bStop = TRUE;
public:
	virtual ~CInertialScrollView() = default;

	void Tick(int iMs) override
	{
		m_iCurrInterval = iMs;
		const int iPrevPos = GetPos();
		m_iSustain += iMs;
		const int iCurr = (int)Easing::OutCubic(
			(float)m_iSustain, (float)m_iStart,
			(float)m_iDistance, (float)m_iDuration);
		if (iCurr > GetMaxWithPage())
		{
			SetPos(GetMaxWithPage());
			m_bStop = TRUE;
			m_pfnCallBack(GetPos(), iPrevPos, m_lParam);
			InterruptAnimation();
			return;
		}
		else
		{
			SetPos(iCurr);
			m_bStop = (m_iSustain >= m_iDuration);
			m_pfnCallBack(GetPos(), iPrevPos, m_lParam);
			if (m_bStop)
				InterruptAnimation();
		}
	}
	EckInline BOOL IsValid() override { return m_bValid; }
	EckInline int GetCurrTickInterval() override { return m_iCurrInterval; }
	// 
	EckInline void OnMouseWheel2(int iWheelDelta) { SmoothScrollDelta(m_iDelta * iWheelDelta); }

	EckInlineCe void InterruptAnimation()
	{
		m_bValid = FALSE;
		m_iSustain = 0.f;
		m_iDistance = 0.f;
	}

	constexpr void SmoothScrollDelta(int iDelta)
	{
		m_iStart = (float)GetPos();
		// 计算新的滚动距离；将原先的滚动距离减去已经滑动完的位移再加上滚动事件产生的位移
		m_iDistance = ((m_iDuration - m_iSustain) * m_iDistance / m_iDuration) + iDelta;
		if (m_iDistance + m_iStart > GetMaxWithPage())
			m_iDistance = GetMaxWithPage() - m_iStart;
		if (m_iDistance + m_iStart < GetMin())
			m_iDistance = GetMin() - m_iStart;
		m_iSustain = 0.f;

		m_bValid = TRUE;
	}

	EckInlineCe void SetCallBack(FInertialScrollProc pfnInertialScroll, LPARAM lParam)
	{
		m_pfnCallBack = pfnInertialScroll;
		m_lParam = lParam;
	}

	EckInlineCe void SetDuration(float iDuration) { m_iDuration = iDuration; }
	EckInlineNdCe float GetDuration() const { return m_iDuration; }

	EckInline void SetDelta(int iDelta) { m_iDelta = iDelta; }
	EckInlineNdCe int GetDelta() const { return m_iDelta; }

	EckInlineNdCe float GetCurrTime() const { return m_iSustain; }

	EckInlineNdCe BOOL IsStop() const { return m_bStop; }
};
ECK_NAMESPACE_END