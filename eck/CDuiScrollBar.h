#pragma once
#include "DuiBase.h"
#include "CInertialScrollView.h"

ECK_NAMESPACE_BEGIN
ECK_DUI_NAMESPACE_BEGIN
class CScrollBar : public CElement, public ITimeLine, public IScrollController
{
public:
    struct EVT_SCROLL : ELENMHDR
    {
        float fPos;
        float fPrevPos;
        BOOLEAN bAnimating;
        BOOLEAN bVertical;
    };

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
    EasingCurve<Easing::FOutCubic> m_ec{};
    FSccCallback m_pfnSccCallback{};
    void* m_pSccCallbackUser{};

    float m_fSmallDelta{ 1.f };

    BITBOOL m_bDragThumb : 1{};
    BITBOOL m_bAnActive : 1{};
    BITBOOL m_bThumbHot : 1{};

    BITBOOL m_bVertical : 1{};
    BITBOOL m_bTransparentSpace : 1{ TRUE };

    USHORT m_msLastDuration{};

    SimpleStyle m_Style[SsMax]
    {
        // Track
        { IdTmInvalid, IdTmInvalid,        IdTmInvalid },
        // Track Hot
        { IdTmInvalid, IdCrBack,           IdTmInvalid },
        // Track Disabled
        { IdTmInvalid, IdCrBackDisabled,   IdTmInvalid },
        // Thumb
        { IdTmInvalid, IdCrBorder,         IdTmInvalid },
        // Thumb Hot
        { IdTmInvalid, IdCrBorderHot,      IdTmInvalid },
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
            if (m_bVertical)
            {
                cxyLeave = (rcThumb.right - rcThumb.left) / 3 * 2;
                cxyMin = (rcThumb.right - rcThumb.left) - cxyLeave;
            }
            else
            {
                cxyLeave = (rcThumb.bottom - rcThumb.top) / 3 * 2;
                cxyMin = (rcThumb.bottom - rcThumb.top) - cxyLeave;
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
                    m_bVertical ? IdPtTrackV : IdPtTrackH,
                    GetRectInClientD2D(),
                    &ps.rcfClip);

                if (m_bVertical)
                    rcThumb.left = rcThumb.right - cxyMin - cxyLeave * m_ec.K;
                else
                    rcThumb.top = rcThumb.bottom - cxyMin - cxyLeave * m_ec.K;
            }
            else
            {
                if (TmGetState() & SaHot)
                {
                    GetTheme()->Draw(
                        this,
                        &m_Style[SsTrackHot],
                        m_bVertical ? IdPtTrackV : IdPtTrackH,
                        GetRectInClientD2D(),
                        &ps.rcfClip);
                }
                else
                {
                    if (m_bVertical)
                        rcThumb.left += cxyLeave;
                    else
                        rcThumb.top += cxyLeave;
                }
            }

            ElementToClient(rcThumb);
            GetTheme()->Draw(
                this,
                &m_Style[m_bThumbHot ? SsThumbHot : SsThumb],
                m_bVertical ? IdPtThumbV : IdPtThumbH,
                Kw::MakeD2DRectF(rcThumb),
                &ps.rcfClip);
        }

        DbgDrawFrame();
        EndPaint(ps);
    }
public:
    static RcPtr<CThemeBase> TmMakeDefaultTheme(BOOL bDark) noexcept;
    static RcPtr<CThemeBase> TmDefaultTheme(BOOL bDark) noexcept
    {
        static auto p1{ TmMakeDefaultTheme(TRUE) };
        static auto p2{ TmMakeDefaultTheme(FALSE) };
        return bDark ? p1 : p2;
    }

    HRESULT EhUiaMakeInterface() noexcept override;

    LRESULT OnEvent(UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept override
    {
        switch (uMsg)
        {
        case WM_PAINT:
            OnPaint(wParam, lParam);
            return 0;

        case WM_NCHITTEST:
        {
            if (m_bTransparentSpace && !(TmGetState() & SaHot))
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
                m_bAnActive = TRUE;
                KctWake();
            }
            if (m_bDragThumb)
            {
                const auto pt = *(Kw::Vec2*)lParam;
                Kw::Rect rcThumbOld, rcThumb;
                GetPartRect(rcThumbOld, Part::Thumb);
                // 减去Cap
                m_sv.OnMouseMove(m_bVertical ?
                    pt.y - GetWidth() : pt.x - GetHeight());

                GetWindow().RdLockUpdate();// 合批滚动条与被滚动内容
                EvtScroll();
                GetPartRect(rcThumb, Part::Thumb);
                UnionRect(rcThumb, rcThumb, rcThumbOld);
                Invalidate(rcThumb);
                GetWindow().RdUnlockUpdate();
            }
        }
        return 0;

        case WM_MOUSELEAVE:
        {
            if ((TmState() & SaHot) && !m_bDragThumb)
            {
                TmState() &= ~SaHot;
                m_ec.Start(1.f, 0.f, m_bAnActive);
                m_bAnActive = TRUE;
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
                // 减去Cap
                m_sv.OnLButtonDown(m_bVertical ?
                    pt.y - GetWidth() : pt.x - GetHeight());
            }
            else
            {
                float f{};
                if (m_bVertical)
                    if (pt.y < rc.top)
                        f = m_sv.GetPosition() - m_sv.GetPage();
                    else if (pt.y > rc.bottom)
                        f = m_sv.GetPosition() + m_sv.GetPage();
                    else
                        goto Skip;
                else
                    if (pt.x < rc.left)
                        f = m_sv.GetPosition() - m_sv.GetPage();
                    else if (pt.x > rc.right)
                        f = m_sv.GetPosition() + m_sv.GetPage();
                    else
                        goto Skip;
                Kw::Rect rcThumbOld, rcThumb;
                GetPartRect(rcThumbOld, Part::Thumb);
                m_sv.SetPosition(f);
                EvtScroll();
                GetPartRect(rcThumb, Part::Thumb);
                UnionRect(rcThumb, rcThumb, rcThumbOld);
                Invalidate(rcThumb);
            Skip:;
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
            if (m_bVertical)
                m_sv.SetViewSize(cy - 2.f * cx);
            else
                m_sv.SetViewSize(cx - 2.f * cy);
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
            SetTheme(TmDefaultTheme(TmIsDarkMode()).Get());
            m_sv.SetCallback([](float fPos, float fPrevPos, void* pUser)
                {
                    EVT_SCROLL nm{};
                    nm.fPos = fPos;
                    nm.fPrevPos = fPrevPos;
                    nm.bAnimating = TRUE;
                    ((CScrollBar*)pUser)->EvtScroll(nm);
                }, this);
            break;

        case WM_DESTROY:
            GetWindow().KctUnregisterTimeLine(&m_sv);
            GetWindow().KctUnregisterTimeLine(this);
            break;
        }
        return __super::OnEvent(uMsg, wParam, lParam);
    }

    void TlTick(int ms) noexcept override
    {
        m_msLastDuration = (USHORT)ms;
        m_bAnActive = m_ec.Tick((float)ms, 200);
        Invalidate(FALSE);
    }
    BOOL TlIsValid() noexcept override { return m_bAnActive; }
    int TlGetCurrentInterval() noexcept override { return (int)m_msLastDuration; }

    // 外部不得对视图调用SetCallback，滚动条统一管理动画和非动画滚动事件，
    // 若已设置Scc回调，则事件仅发送到回调，否则发送通知
    EckInlineNdCe auto& GetScrollView() noexcept { return m_sv; }

    void GetPartRect(_Out_ Kw::Rect& rc, Part eType) const noexcept
    {
        const auto cx = GetWidth(), cy = GetHeight();
        if (m_bVertical)
            switch (eType)
            {
            case Part::Button1: rc = { 0,       0, cx,      cx };       return;
            case Part::Button2: rc = { 0, cy - cx, cx,      cy }; return;
            case Part::Track:   rc = { 0,      cx, cx, cy - cx }; return;
            case Part::Thumb:
            {
                if (!m_sv.IsVisible())
                    break;
                const auto cxThumb = cx - GetTheme()->GetMetric(IdMeScrollThumbPadding) * 2.f;
                const auto cxyThumb = m_sv.GetThumbSize();
                rc.left = (cx - cxThumb) / 2.f;
                rc.top = cx /*Cap*/ + m_sv.GetThumbPosition(cxyThumb);
                rc.right = rc.left + cxThumb;
                rc.bottom = rc.top + cxyThumb;
            }
            return;
            }
        else
            switch (eType)
            {
            case Part::Button1: rc = { 0,       0,      cy, cy }; return;
            case Part::Button2: rc = { cx - cy, 0,      cx, cy }; return;
            case Part::Track:   rc = { cy,      0, cx - cy, cy }; return;
            case Part::Thumb:
            {
                if (!m_sv.IsVisible())
                    break;
                const auto cyThumb = cy - GetTheme()->GetMetric(IdMeScrollThumbPadding) * 2.f;
                const auto cxThumb = m_sv.GetThumbSize();
                rc.left = cy /*Cap*/ + m_sv.GetThumbPosition(cxThumb);
                rc.top = (cy - cyThumb) / 2.f;
                rc.right = rc.left + cxThumb;
                rc.bottom = rc.top + cyThumb;
            }
            return;
            }
        rc = {};
    }

    EckInlineCe void SetRange(float fMin, float fMax) noexcept { m_sv.SetRange(fMin, fMax); }
    EckInlineNdCe float GetMinimum() const noexcept { return m_sv.GetMinimum(); }
    EckInlineNdCe float GetMaximum() const noexcept { return m_sv.GetMaximum(); }

    EckInlineCe void SetTrackPosition(float fPos) noexcept { m_sv.SetPosition(fPos); }
    EckInlineNdCe float GetTrackPosition() const noexcept { return m_sv.GetPosition(); }

    EckInlineCe void SetVertical(BOOL b) noexcept { m_bVertical = b; }
    EckInlineNdCe BOOL GetVertical() const noexcept { return m_bVertical; }

    EckInlineCe void SetTransparentSpace(BOOL b) noexcept { m_bTransparentSpace = b; }
    EckInlineNdCe BOOL GetTransparentSpace() const noexcept { return m_bTransparentSpace; }

    EckInlineCe void SetSmallDelta(float f) noexcept { m_fSmallDelta = f; }
    EckInlineNdCe float GetSmallDelta() const noexcept { return m_fSmallDelta; }

    EckInlineCe void SetLargeDelta(float f) noexcept { m_sv.SetPage(f); }
    EckInlineNdCe float GetLargeDelta() const noexcept { return m_sv.GetPage(); }
    EckInlineCe void SetPage(float f) noexcept { SetLargeDelta(f); }
    EckInlineNdCe float GetPage() const noexcept { return GetLargeDelta(); }

    void EvtScroll() noexcept
    {
        EVT_SCROLL nm{ ENC_SCROLL };
        nm.fPos = m_sv.GetPosition();
        EvtScroll(nm);
    }
    void EvtScroll(EVT_SCROLL& nm) noexcept
    {
        if (m_pfnSccCallback)
        {
            const SCC_CALLBACK_DATA Data
            {
                .fPos = nm.fPos,
                .fPrevPos = nm.fPrevPos,
                .bAnimating = nm.bAnimating,
                .pUser = m_pSccCallbackUser
            };
            m_pfnSccCallback(Data);
        }
        else
        {
            nm.bVertical = m_bVertical;
            SendNotify(&nm);
        }
    }

    float SccGetPosition() const noexcept override { return m_sv.GetPosition(); }
    void SccSetPosition(float x) noexcept override { m_sv.SetPosition(x); }
    float SccGetPage() const noexcept override { return m_sv.GetPage(); }
    void SccSetPage(float x) noexcept override { m_sv.SetPage(x); }
    float SccGetMinimum() const noexcept override { return m_sv.GetMinimum(); }
    void SccSetMinimum(float x) noexcept override { m_sv.SetMinimum(x); }
    float SccGetMaximum() const noexcept override { return m_sv.GetMaximum(); }
    void SccSetMaximum(float x) noexcept override { m_sv.SetMaximum(x); }
    void SccSetRange(float Min, float Max) noexcept override { m_sv.SetRange(Min, Max); }
    void SccSetVisible(BOOL b) noexcept override { SetVisible(b); }
    void SccRedraw() noexcept override { Invalidate(); }

    void SccMouseWheel(float d) noexcept override { m_sv.OnMouseWheel2(d); }
    void SccScrollDelta(float d, BOOL bSmooth) noexcept override
    {
        if (bSmooth)
            m_sv.SmoothScrollDelta(d);
        else
            m_sv.SetPosition(m_sv.GetPosition() + d);
    }

    void SccSetCallback(FSccCallback pfnCallback, void* pUser) noexcept override
    {
        m_pfnSccCallback = pfnCallback;
        m_pSccCallbackUser = pUser;
    }
};

class CTmScrollBar : public CThemeBase
{
public:
    TmResult Draw(
        CElement* pEle,
        const SimpleStyle* pStyle,
        UINT idPart,
        const D2D1_RECT_F& rc,
        _In_opt_ const D2D1_RECT_F* prcClip) noexcept override
    {
        if (idPart != CScrollBar::IdPtTrackH &&
            idPart != CScrollBar::IdPtTrackV &&
            idPart != CScrollBar::IdPtThumbH &&
            idPart != CScrollBar::IdPtThumbV)
            return TmResult::NotSupport;
        return pEle->TmGenericDrawBackground(pStyle, rc);
    }
};
inline RcPtr<CThemeBase> CScrollBar::TmMakeDefaultTheme(BOOL bDark) noexcept
{
    return TmMakeTheme<CTmScrollBar>(bDark);
}

class CUiaScrollBar : public CUnknownAppend<CUiaBase, IRangeValueProvider>
{
    STDMETHODIMP GetPatternProvider(PATTERNID idPattern, IUnknown** pRetVal) override
    {
        if (idPattern == UIA_RangeValuePatternId)
        {
            *pRetVal = static_cast<IRangeValueProvider*>(this);
            AddRef();
            return S_OK;
        }
        return CUiaBase::GetPatternProvider(idPattern, pRetVal);
    }
    STDMETHODIMP GetPropertyValue(PROPERTYID idProp, VARIANT* pRetVal) override
    {
        if (idProp == UIA_ControlTypePropertyId)
        {
            pRetVal->vt = VT_I4;
            pRetVal->intVal = UIA_ScrollBarControlTypeId;
            return S_OK;
        }
        return CUiaBase::GetPropertyValue(idProp, pRetVal);
    }

    STDMETHODIMP SetValue(double Val) override
    {
        if (!GetElement())
            return UIA_E_ELEMENTNOTAVAILABLE;
        if (GetElement()->GetStyle() & DES_DISABLE)
            return UIA_E_ELEMENTNOTENABLED;
        const auto pEle = DbgDynamicCast<CScrollBar*>(GetElement());
        pEle->SetTrackPosition((float)Val);
        pEle->Invalidate();
        pEle->EvtScroll();
        return S_OK;
    }
    STDMETHODIMP get_Value(double* pRetVal) override
    {
        if (!GetElement())
            return UIA_E_ELEMENTNOTAVAILABLE;
        *pRetVal = DbgDynamicCast<CScrollBar*>(GetElement())->GetTrackPosition();
        return S_OK;
    }
    STDMETHODIMP get_IsReadOnly(BOOL* pRetVal) override
    {
        if (!GetElement())
            return UIA_E_ELEMENTNOTAVAILABLE;
        *pRetVal = FALSE;
        return S_OK;
    }
    STDMETHODIMP get_Maximum(double* pRetVal) override
    {
        if (!GetElement())
            return UIA_E_ELEMENTNOTAVAILABLE;
        *pRetVal = DbgDynamicCast<CScrollBar*>(GetElement())->GetMaximum();
        return S_OK;
    }
    STDMETHODIMP get_Minimum(double* pRetVal) override
    {
        if (!GetElement())
            return UIA_E_ELEMENTNOTAVAILABLE;
        *pRetVal = DbgDynamicCast<CScrollBar*>(GetElement())->GetMinimum();
        return S_OK;
    }
    STDMETHODIMP get_LargeChange(double* pRetVal) override
    {
        if (!GetElement())
            return UIA_E_ELEMENTNOTAVAILABLE;
        *pRetVal = DbgDynamicCast<CScrollBar*>(GetElement())->GetLargeDelta();
        return S_OK;
    }
    STDMETHODIMP get_SmallChange(double* pRetVal) override
    {
        if (!GetElement())
            return UIA_E_ELEMENTNOTAVAILABLE;
        *pRetVal = DbgDynamicCast<CScrollBar*>(GetElement())->GetSmallDelta();
        return S_OK;
    }
};
inline HRESULT CScrollBar::EhUiaMakeInterface() noexcept
{
    const auto p = new CUiaScrollBar{};
    UiaSetInterface(p);
    p->Release();
    return S_OK;
}
ECK_DUI_NAMESPACE_END
ECK_NAMESPACE_END