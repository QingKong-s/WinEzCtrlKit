#pragma once
#include "CScrollView.h"
#include "ITimeLine.h"
#include "EasingCurve.h"

ECK_NAMESPACE_BEGIN
class CInertialScrollView : public CScrollView, public ITimeLine
{
public:
	using InertialScrollProc = void (*)(int iPos, int iPrevPos, LPARAM lParam);
protected:
	int m_iStart = 0;		// 起始位置
	int m_iDistance = 0;	// 当前动画应滚动的总距离
	int m_iSustain = 0;		// 持续时间，毫秒
	int m_iDuration = 400;	// 动画总时间
	int m_iDelta = 80;		// 每次滚动的距离

	InertialScrollProc m_pfnCallBack = NULL;
	LPARAM m_lParam = 0;

	LONG m_cRef = 1;
	BOOL m_bValid = FALSE;
public:
	// **IUnknown**
	ULONG STDMETHODCALLTYPE AddRef(void) { return ++m_cRef; }

	ULONG STDMETHODCALLTYPE Release(void)
	{
		if (m_cRef == 1)
		{
			delete this;
			return 0;
		}
		return --m_cRef;
	}

	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void** ppvObject)
	{
		const static QITAB qit[]
		{
			QITABENT(CInertialScrollView, ITimeLine),
			{}
		};

		return QISearch(this, qit, iid, ppvObject);
	}
	// **ITimeLine**
	void STDMETHODCALLTYPE Tick(int iMs)
	{
		const int iPrevPos = GetPos();
		m_iSustain += 20;
		int iCurr = (int)Easing::FOutCubic{}((float)m_iSustain, (float)m_iStart,
			(float)m_iDistance, (float)m_iDuration);
		if (iCurr > GetMaxWithPage())
		{
			SetPos(GetMaxWithPage());
			m_pfnCallBack(GetPos(), iPrevPos, m_lParam);
			InterruptAnimation();
			return;
		}
		else
		{
			SetPos(iCurr);
			m_pfnCallBack(GetPos(), iPrevPos, m_lParam);
			if (m_iSustain >= m_iDuration)
				InterruptAnimation();
		}
	}

	EckInline BOOL STDMETHODCALLTYPE IsValid()
	{
		return m_bValid;
	}
	// 
	EckInline void OnMouseWheel2(int iWheelDelta)
	{
		SmoothScrollDelta(m_iDelta * iWheelDelta);
	}

	EckInline void InterruptAnimation()
	{
		m_bValid = FALSE;
		m_iSustain = 0;
		m_iDistance = 0;
	}

	void SmoothScrollDelta(int iDelta)
	{
		m_iStart = GetPos();
		// 计算新的滚动距离；将原先的滚动距离减去已经滑动完的位移再加上滚动事件产生的位移
		m_iDistance = ((m_iDuration - m_iSustain) * m_iDistance / m_iDuration) + iDelta;
		if (m_iDistance + m_iStart > GetMaxWithPage())
			m_iDistance = GetMaxWithPage() - m_iStart;
		if (m_iDistance + m_iStart < GetMin())
			m_iDistance = GetMin() - m_iStart;
		m_iSustain = 0;

		m_bValid = TRUE;
	}

	EckInline void SetCallBack(InertialScrollProc pfnInertialScroll, LPARAM lParam)
	{
		m_pfnCallBack = pfnInertialScroll;
		m_lParam = lParam;
	}

	EckInline void SetDuration(int iDuration) { m_iDuration = iDuration; }

	EckInline void SetDelta(int iDelta) { m_iDelta = iDelta; }
};
ECK_NAMESPACE_END