#pragma once
#include "DuiBase.h"

ECK_NAMESPACE_BEGIN
ECK_DUI_NAMESPACE_BEGIN
class CTrackBar : public CElem
{
private:
    CEasingCurve m_ec{};

    float m_fPos{};
    float m_fMin{};
    float m_fMax{ 100.0f };
    float m_fDragPos{};
    float m_cxyTrack{};

    BITBOOL m_bLBtnDown : 1{};
    BITBOOL m_bHover : 1{};

    BITBOOL m_bVertical : 1{};
    BITBOOL m_bGenEventWhenDragging : 1{};
    BITBOOL m_bTransparentSpace : 1{};  // 空白部分穿透鼠标
    BITBOOL m_bThinTrack : 1{};         // 轨道正常情况下显示为尺寸的一半，点燃时显示全尺寸
    BITBOOL m_bAutoTrackSize : 1{};     // 根据控件尺寸自动调整轨道尺寸

    // 取轨道尺寸
    // 返回轨道在当前状态下的实际尺寸，考虑动画
    float GetCxyTrack()
    {
        if (m_bThinTrack)
            if (m_ec.IsActive())
                return  m_cxyTrack / 2.f + m_cxyTrack / 2.f * m_ec.GetCurrentValue();
            else
                return (m_bHover ? m_cxyTrack : m_cxyTrack / 2.f);
        else
            return m_cxyTrack;
    }

    float GetTrackRect(_Out_ D2D1_RECT_F& rc)
    {
        rc = GetViewRectF();
        const float cxyTrack = GetCxyTrack();
        const float fRadius = GetTrackCapSpacing();
        if (m_bVertical)
        {
            rc.left += (rc.right - rc.left - cxyTrack) / 2.f;
            rc.right = rc.left + cxyTrack;
            rc.top += fRadius;
            rc.bottom -= fRadius;
        }
        else
        {
            rc.top += (rc.bottom - rc.top - cxyTrack) / 2.f;
            rc.bottom = rc.top + cxyTrack;
            rc.left += fRadius;
            rc.right -= fRadius;
        }
        return cxyTrack;
    }

    void GetThumbRect(float cxyTrack, const D2D1_RECT_F& rcTrack, _Out_ D2D1_RECT_F& rc)
    {
        const float cxy = m_bThinTrack ?
            (GetTrackCapSpacing() * m_ec.GetCurrentValue()) :
            GetTrackCapSpacing();
        if (m_bVertical)
        {
            rc.left = (rcTrack.left + rcTrack.right) / 2.f - cxy;
            rc.top = rcTrack.bottom - cxy;
        }
        else
        {
            rc.left = rcTrack.right - cxy;
            rc.top = (rcTrack.top + rcTrack.bottom) / 2.f - cxy;
        }
        rc.right = rc.left + cxy * 2;
        rc.bottom = rc.top + cxy * 2;
    }
    void GetThumbRect(_Out_ D2D1_RECT_F& rc)
    {
        const auto cxy = GetTrackRect(rc);
        const float fScale = (GetTrackPos() - m_fMin) / (m_fMax - m_fMin);
        if (m_bVertical)
            rc.bottom = rc.top + (rc.bottom - rc.top) * fScale;
        else
            rc.right = rc.left + (rc.right - rc.left) * fScale;
        GetThumbRect(cxy, rc, rc);
    }

    void SetDragPos(float fPos)
    {
        m_fDragPos = fPos;
        if (m_fDragPos < m_fMin)
            m_fDragPos = m_fMin;
        else if (m_fDragPos > m_fMax)
            m_fDragPos = m_fMax;
    }
public:
    LRESULT OnEvent(UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept override
    {
        switch (uMsg)
        {
        case WM_PAINT:
        {
            ELEMPAINTSTRU ps;
            BeginPaint(ps, wParam, lParam);

            DTB_OPT Opt;
            Opt.uFlags = DTBO_NEW_RADX | DTBO_NEW_RADY;
            const float cxyTrack = GetTrackRect(Opt.rcClip);
            Opt.fRadX = Opt.fRadY = cxyTrack / 2.f;

            GetTheme()->DrawBackground(Part::TrackBar, State::Normal,
                Opt.rcClip, &Opt);

            const float fScale = (GetTrackPos() - m_fMin) / (m_fMax - m_fMin);
            if (m_bVertical)
                Opt.rcClip.bottom = Opt.rcClip.top + (Opt.rcClip.bottom - Opt.rcClip.top) * fScale;
            else
                Opt.rcClip.right = Opt.rcClip.left + (Opt.rcClip.right - Opt.rcClip.left) * fScale;
            GetTheme()->DrawBackground(Part::TrackBar, State::Selected,
                Opt.rcClip, &Opt);

            if (!m_bThinTrack || (m_bHover || m_ec.IsActive()))
            {
                GetThumbRect(cxyTrack, Opt.rcClip, Opt.rcClip);
                GetTheme()->DrawBackground(Part::TrackBarThumb, State::Hot,
                    Opt.rcClip, nullptr);
            }
            ECK_DUI_DBG_DRAW_FRAME;
            EndPaint(ps);
        }
        return 0;

        case WM_NCHITTEST:
        {
            if (m_bTransparentSpace)
            {
                POINT pt ECK_GET_PT_LPARAM(lParam);
                ClientToElem(pt);

                D2D1_RECT_F rcTrack;
                GetTrackRect(rcTrack);
                if (!PtInRect(rcTrack, pt))
                    return HTTRANSPARENT;
            }
        }
        break;

        case WM_MOUSEMOVE:
        {
            if (m_bLBtnDown)
            {
                const POINT pt ECK_GET_PT_LPARAM(lParam);
                SetDragPos(HitTest(pt));
                if (m_bGenEventWhenDragging)
                {
                    DUINMHDR nm{ TBE_POSCHANGED };
                    GenElemNotify(&nm);
                }
                InvalidateRect();
            }
            else if (!m_bHover)
            {
                m_bHover = TRUE;
                if (m_bThinTrack)
                {
                    m_ec.Begin(0.f, 1.f);
                    GetWnd()->WakeRenderThread();
                }
            }
        }
        return 0;

        case WM_MOUSELEAVE:
        {
            if (!m_bLBtnDown && m_bHover)
            {
                m_bHover = FALSE;
                if (m_bThinTrack)
                {
                    m_ec.Begin(1.f, 0.f);
                    GetWnd()->WakeRenderThread();
                }
            }
        }
        return 0;

        case WM_LBUTTONDOWN:
        {
            const POINT pt ECK_GET_PT_LPARAM(lParam);
            D2D1_RECT_F rcThumb;
            GetThumbRect(rcThumb);
            if (PtInRect(rcThumb, MakeD2DPointF(pt)))
            {
                m_bLBtnDown = TRUE;
                SetCapture();
                m_fDragPos = HitTest(pt);
            }
            else
            {
                SetTrackPos(HitTest(pt));
                InvalidateRect();
                DUINMHDR nm{ TBE_POSCHANGED };
                GenElemNotify(&nm);
            }
        }
        return 0;

        case WM_LBUTTONUP:
        {
            if (m_bLBtnDown)
            {
                const POINT pt ECK_GET_PT_LPARAM(lParam);
                m_bLBtnDown = FALSE;
                ReleaseCapture();
                SetTrackPos(HitTest(pt));
                InvalidateRect();
                DUINMHDR nm{ TBE_POSCHANGED };
                GenElemNotify(&nm);

                if (m_bHover)
                {
                    m_bHover = FALSE;
                    if (m_bThinTrack)
                        m_ec.Begin(1.f, 0.f);
                }
            }
        }
        return 0;

        case WM_CREATE:
        {
            InitEasingCurve(&m_ec);
            m_ec.SetDuration(200);
            m_ec.SetProcedure(Easing::OutSine);
            m_ec.SetCallback([](float fCurrValue, float fOldValue, LPARAM lParam)
                {
                    ((CElem*)lParam)->InvalidateRect();
                });
        }
        [[fallthrough]];
        case WM_SIZE:
        {
            if (m_bAutoTrackSize)
                m_cxyTrack = (m_bVertical ? GetWidthF() : GetHeightF()) * 0.6f;
        }
        break;
        }
        return CElem::OnEvent(uMsg, wParam, lParam);
    }

    void SetRange(float fMin, float fMax)
    {
        m_fMin = fMin;
        m_fMax = fMax;
        if (m_fPos < m_fMin)
            m_fPos = m_fMin;
        else if (m_fPos > m_fMax)
            m_fPos = m_fMax;
    }

    constexpr void SetTrackPos(float fPos)
    {
        m_fPos = fPos;
        if (m_fPos < m_fMin)
            m_fPos = m_fMin;
        else if (m_fPos > m_fMax)
            m_fPos = m_fMax;
    }
    EckInlineNdCe float GetTrackPos() const { return m_bLBtnDown ? m_fDragPos : m_fPos; }

    EckInlineCe void SetVertical(BOOL bVertical) { m_bVertical = bVertical; }
    EckInlineNdCe BOOL GetVertical() const { return m_bVertical; }

    EckInlineCe void SetTrackSize(float f) { m_cxyTrack = f; }
    EckInlineNdCe float GetTrackSize() const { return m_cxyTrack; }

    EckInlineCe void SetGenEventWhenDragging(BOOL b) { m_bGenEventWhenDragging = b; }
    EckInlineNdCe BOOL GetGenEventWhenDragging() const { return m_bGenEventWhenDragging; }

    EckInlineCe void SetTransparentSpace(BOOL b) { m_bTransparentSpace = b; }
    EckInlineNdCe BOOL GetTransparentSpace() const { return m_bTransparentSpace; }

    EckInlineCe void SetThinTrack(BOOL b) { m_bThinTrack = b; }
    EckInlineNdCe BOOL GetThinTrack() const { return m_bThinTrack; }

    EckInlineCe void SetAutoTrackSize(BOOL b) { m_bAutoTrackSize = b; }
    EckInlineNdCe BOOL GetAutoTrackSize() const { return m_bAutoTrackSize; }

    float HitTest(POINT pt)
    {
        D2D1_RECT_F rcTrack;
        GetTrackRect(rcTrack);

        if (m_bVertical)
        {
            const float fScale = (pt.y - rcTrack.top) / (rcTrack.bottom - rcTrack.top);
            return m_fMin + (m_fMax - m_fMin) * fScale;
        }
        else
        {
            const float fScale = (pt.x - rcTrack.left) / (rcTrack.right - rcTrack.left);
            return m_fMin + (m_fMax - m_fMin) * fScale;
        }
    }

    // 取端点处空白
    // 因滑块具有大小，故端点处留有一定空白以免滑块显示不全，此函数返回空白大小
    EckInlineNdCe float GetTrackCapSpacing() const noexcept { return m_cxyTrack * 3.f / 4.f; }
};
ECK_DUI_NAMESPACE_END
ECK_NAMESPACE_END