#pragma once
#include "CScrollView.h"
#include "ITimeLine.h"
#include "EasingCurve.h"

ECK_NAMESPACE_BEGIN
class CInertialScrollView : public CScrollViewF, public ITimeLine
{
public:
    using FCallback = void(*)(float fPos, float fPrevPos, void* pUser);
protected:
    float m_fStart{};           // 起始位置
    float m_fDistance{};        // 当前动画应滚动的总距离
    float m_fSustain{};         // 持续时间，毫秒
    float m_fDuration{ 400.f }; // 动画总时间
    float m_fDelta{ 80.f };     // 每次滚动的距离

    int m_iCurrInterval{};

    FCallback m_pfnCallback{};
    void* m_pUser{};

    BOOL m_bValid{};
    BOOL m_bStop{ TRUE };
public:
    virtual ~CInertialScrollView() = default;

    void TlTick(int iMs) noexcept override
    {
        m_iCurrInterval = iMs;
        const float fPrevPos = GetPosition();
        m_fSustain += iMs;
        const float f = Easing::OutCubic(m_fSustain, m_fStart, m_fDistance, m_fDuration);
        if (SetPosition(f))
            m_bStop = TRUE;
        else
            m_bStop = (m_fDistance > 0.f ?
                (f >= m_fStart + m_fDistance) : (f <= m_fStart + m_fDistance));
        m_pfnCallback(GetPosition(), fPrevPos, m_pUser);
        if (m_bStop)
            InterruptAnimation();
    }
    EckInline BOOL TlIsValid() noexcept override { return m_bValid; }
    EckInline int TlGetCurrentInterval() noexcept override { return m_iCurrInterval; }
    // 
    EckInline void OnMouseWheel2(int iWheelDelta) noexcept { SmoothScrollDelta(m_fDelta * iWheelDelta); }

    EckInlineCe void InterruptAnimation() noexcept
    {
        m_bValid = FALSE;
        m_fSustain = 0.f;
        m_fDistance = 0.f;
    }

    constexpr void SmoothScrollDelta(float iDelta) noexcept
    {
        m_fStart = GetPosition();
        // 计算新的滚动距离；将原先的滚动距离减去已经滑动完的位移再加上滚动事件产生的位移
        m_fDistance = ((m_fDuration - m_fSustain) * m_fDistance / m_fDuration) + iDelta;
        if (m_fDistance + m_fStart > GetMaxWithPage())
            m_fDistance = GetMaxWithPage() - m_fStart;
        if (m_fDistance + m_fStart < GetMinimum())
            m_fDistance = GetMinimum() - m_fStart;
        m_fSustain = 0.f;

        m_bValid = TRUE;
    }

    EckInlineCe void SetCallback(FCallback pfnCallBack, void* pUser) noexcept
    {
        m_pfnCallback = pfnCallBack;
        m_pUser = pUser;
    }

    EckInlineCe void SetDuration(float iDuration) noexcept { m_fDuration = iDuration; }
    EckInlineNdCe float GetDuration() const noexcept { return m_fDuration; }

    EckInline void SetDelta(float iDelta) noexcept { m_fDelta = iDelta; }
    EckInlineNdCe float GetDelta() const noexcept { return m_fDelta; }

    EckInlineNdCe float GetCurrentTime() const noexcept { return m_fSustain; }

    EckInlineNdCe BOOL IsStop() const noexcept { return m_bStop; }
};
ECK_NAMESPACE_END