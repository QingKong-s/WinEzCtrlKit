#pragma once
#include "DuiBase.h"
#include "CInertialScrollView.h"

ECK_NAMESPACE_BEGIN
ECK_DUI_NAMESPACE_BEGIN
class CScrollBar : public CElement, public ITimeLine
{
public:
    const static inline UINT IdPtTrackH = TmNextResourceId();
    const static inline UINT IdPtTrackV = TmNextResourceId();
    const static inline UINT IdPtThumbH = TmNextResourceId();
    const static inline UINT IdPtThumbV = TmNextResourceId();
    const static inline UINT IdCrThumb = TmNextResourceId();

    enum class Part
    {
        Button1,
        Button2,
        Track,
        Thumb,
    };

    enum : UINT
    {
        SsTrack,
        SsTrackHot,
        SsTrackDisabled,
        SsThumb,
        SsThumbHot,
        SsThumbDisabled,
        SsMax
    };
private:
    CInertialScrollView m_sv{};
    CEasingCurveLite<Easing::FOutCubic> m_ec{};

    BITBOOL m_bDragThumb : 1{};
    BITBOOL m_bAnActive : 1{};
    BITBOOL m_bThumbHot : 1{};

    BITBOOL m_bHorizontal : 1{};
    BITBOOL m_bTransparentTrack : 1{ TRUE };

    SimpleStyle m_Style[SsMax]
    {
        // Track
        { IdTmInvalid, IdTmInvalid,        IdTmInvalid },
        // Track Hot
        { IdTmInvalid, IdCrBorder,         IdTmInvalid },
        // Track Disabled
        { IdTmInvalid, IdCrBorderDisabled, IdTmInvalid },
        // Thumb
        { IdCrBorder,  IdCrBorder,         IdCrBorder, FLT_MAX, 1.f },
        // Thumb Hot
        { IdCrBorder,  IdCrBorderHot,      IdCrBorder, FLT_MAX, 1.f },
        // Thumb Disabled
        { IdTmInvalid, IdCrBorderDisabled, IdTmInvalid },
    };

    void OnPaint(WPARAM wParam, LPARAM lParam) noexcept
    {
        PAINTINFO ps;
        BeginPaint(ps, wParam, lParam);

        if (m_sv.IsVisible())
        {
            Kw::Rect rcThumb;
            GetPartRect(rcThumb, Part::Thumb);
            float cxyLeave, cxyMin;
            if (m_bHorizontal)
            {
                cxyLeave = (rcThumb.bottom - rcThumb.top) / 3 * 2;
                cxyMin = (rcThumb.bottom - rcThumb.top) - cxyLeave;
            }
            else
            {
                cxyLeave = (rcThumb.right - rcThumb.left) / 3 * 2;
                cxyMin = (rcThumb.right - rcThumb.left) - cxyLeave;
            }

            if (m_bAnActive)
            {
                const auto Style = TmSsLerp(
                    GetTheme().Get(),
                    m_Style[SsTrack],
                    m_Style[SsTrackHot],
                    m_ec.K);

                GetTheme()->Draw(
                    this,
                    &Style,
                    m_bHorizontal ? IdPtTrackH : IdPtTrackV,
                    GetViewRectD2D(),
                    &ps.rcfClip);

                if (m_bHorizontal)
                    rcThumb.top = rcThumb.bottom - cxyMin - cxyLeave * m_ec.K;
                else
                    rcThumb.left = rcThumb.right - cxyMin - cxyLeave * m_ec.K;
            }
            else
            {
                if (TmGetState() & SaHot)
                {
                    GetTheme()->Draw(
                        this,
                        &m_Style[SsTrackHot],
                        m_bHorizontal ? IdPtTrackH : IdPtTrackV,
                        GetViewRectD2D(),
                        &ps.rcfClip);
                }
                else
                {
                    if (m_bHorizontal)
                        rcThumb.top += cxyLeave;
                    else
                        rcThumb.left += cxyLeave;
                }
            }

            GetTheme()->Draw(
                this,
                &m_Style[m_bThumbHot ? SsThumbHot : SsThumb],
                m_bHorizontal ? IdPtThumbH : IdPtThumbV,
                Kw::MakeD2DRectF(rcThumb),
                &ps.rcfClip);
        }

        EndPaint(ps);
    }
public:
    LRESULT OnEvent(UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept override
    {
        switch (uMsg)
        {
        case WM_PAINT:
            OnPaint(wParam, lParam);
            return 0;

        case WM_NCHITTEST:
        {
            if (m_bTransparentTrack && !(TmGetState() & SaHot))
            {
                if (!m_sv.IsVisible())
                    return HTTRANSPARENT;
                auto pt = *(Kw::Vec2*)lParam;
                ClientToElement(pt);
                Kw::Rect rc;
                GetPartRect(rc, Part::Thumb);
                if (!PointInRect(rc, pt))
                    return HTTRANSPARENT;
            }
        }
        break;

        case WM_MOUSEMOVE:
        {
            if (!(TmState() & SaHot))
            {
                TmState() |= SaHot;
                m_ec.Start(0.f, 1.f, m_bAnActive);
                KctWake();
            }
            if (m_bDragThumb)
            {
                const auto pt = *(Kw::Vec2*)lParam;
                m_sv.OnMouseMove(m_bHorizontal ? pt.x - GetHeight() : pt.y - GetWidth());
                EvtScroll();
            }
        }
        return 0;

        case WM_MOUSELEAVE:
        {
            if ((TmState() & SaHot) && !m_bDragThumb)
            {
                TmState() &= ~SaHot;
                m_ec.Start(1.f, 0.f, m_bAnActive);
                KctWake();
            }
        }
        return 0;

        case WM_LBUTTONDOWN:
        case WM_LBUTTONDBLCLK:
        {
            SetCapture();
            TmState() |= SapLButtonDown;
            const auto pt = *(Kw::Vec2*)lParam;
            Kw::Rect rc;
            GetPartRect(rc, Part::Thumb);
            if (PointInRect(rc, pt))
            {
                m_bDragThumb = TRUE;
                m_sv.OnLButtonDown(m_bHorizontal ? pt.x - GetHeight() : pt.y - GetWidth());
            }
            else
            {
                if (m_bHorizontal)
                    m_sv.SetPosition(pt.x < rc.left ?
                        m_sv.GetPosition() - m_sv.GetPage() :
                        m_sv.GetPosition() + m_sv.GetPage());
                else
                    m_sv.SetPosition(pt.y < rc.top ?
                        m_sv.GetPosition() - m_sv.GetPage() :
                        m_sv.GetPosition() + m_sv.GetPage());
                EvtScroll();
            }
        }
        return 0;

        case WM_CAPTURECHANGED:
        case WM_LBUTTONUP:
        {
            if (!(TmState() & SapLButtonDown))
                break;
            TmState() &= ~SapLButtonDown;
            ReleaseCapture();
            if (m_bDragThumb)
            {
                m_bDragThumb = FALSE;
                m_sv.OnLButtonUp();
                EvtScroll();
            }
        }
        return 0;

        case WM_SIZE:
        {
            const auto cx = GetWidth(), cy = GetHeight();
            if (m_bHorizontal)
                m_sv.SetViewSize(cx - 2.f * cy);
            else
                m_sv.SetViewSize(cy - 2.f * cx);
        }
        break;

        case WM_MOUSEWHEEL:
        case WM_MOUSEHWHEEL:
        {
            const auto p = EtParent();
            if (p)
                return p->CallEvent(uMsg, wParam, lParam);
        }
        break;

        case WM_CREATE:
            GetWindow().KctRegisterTimeLine(this);
            GetWindow().KctRegisterTimeLine(&m_sv);
            return 0;

        case WM_DESTROY:
            GetWindow().KctUnregisterTimeLine(this);
            GetWindow().KctUnregisterTimeLine(&m_sv);
            return 0;
        }
        return CElement::OnEvent(uMsg, wParam, lParam);
    }

    EckInlineNdCe auto& GetScrollView() const noexcept { return m_sv; }

    void GetPartRect(_Out_ Kw::Rect& rc, Part eType) const noexcept
    {
        const auto cx = GetWidth(), cy = GetHeight();
        if (m_bHorizontal)
            switch (eType)
            {
            case Part::Button1:
                rc = { 0, 0, cy, cy };
                return;
            case Part::Button2:
                rc = { cx - cy, 0, cx, cy };
                return;
            case Part::Track:
                rc = { cy, 0, cx - cy, cy };
                return;
            case Part::Thumb:
            {
                if (!m_sv.IsVisible())
                {
                    rc = {};
                    return;
                }
                const auto cyThumb = GetTheme()->GetMetric(IdMeScrollThumbWidth);
                const auto cxyThumb = m_sv.GetThumbSize();
                rc.left = cy + m_sv.GetThumbPosition(cxyThumb);
                rc.top = (cy - cyThumb) / 2.f;
                rc.right = rc.left + cxyThumb;
                rc.bottom = rc.top + cyThumb;
            }
            return;
            }
        else
            switch (eType)
            {
            case Part::Button1:
                rc = { 0, 0, cx, cx };
                return;
            case Part::Button2:
                rc = { 0, cy - cx, cx, cy };
                return;
            case Part::Track:
                rc = { 0, cx, cx, cy - cx };
                return;
            case Part::Thumb:
            {
                if (!m_sv.IsVisible())
                {
                    rc = {};
                    return;
                }
                const auto cxThumb = GetTheme()->GetMetric(IdMeScrollThumbWidth);
                const auto cxyThumb = m_sv.GetThumbSize();
                rc.left = (cx - cxThumb) / 2.f;
                rc.top = cx + m_sv.GetThumbPosition(cxyThumb);
                rc.right = rc.left + cxThumb;
                rc.bottom = rc.top + cxyThumb;
            }
            return;
            }
    }

    EckInlineCe void SetHorizontal(BOOL b) noexcept { m_bHorizontal = b; }
    EckInlineNdCe BOOL GetHorizontal() const noexcept { return m_bHorizontal; }

    EckInlineCe void SetTransparentTrack(BOOL b) noexcept { m_bTransparentTrack = b; }
    EckInlineNdCe BOOL GetTransparentTrack() const noexcept { return m_bTransparentTrack; }

    void EvtScroll() noexcept
    {
        ELENMHDR nm{ m_bHorizontal ? ENC_HSCROLL : ENC_VSCROLL };
        SendNotify(&nm);
    }
};
ECK_DUI_NAMESPACE_END
ECK_NAMESPACE_END