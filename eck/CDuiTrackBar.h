#pragma once
#include "DuiBase.h"

ECK_NAMESPACE_BEGIN
ECK_DUI_NAMESPACE_BEGIN
class CTrackBar : public CElement, public ITimeLine
{
public:
    const static inline UINT IdPtTrackH = TmNextResourceId();
    const static inline UINT IdPtTrackV = TmNextResourceId();
    const static inline UINT IdPtActiveTrackH = TmNextResourceId();
    const static inline UINT IdPtActiveTrackV = TmNextResourceId();
    const static inline UINT IdPtThumb = TmNextResourceId();
    const static inline UINT IdCrThumb = TmNextResourceId();

    enum : UINT
    {
        SsTrack,
        SsTrackActive,
        SsTrackDisabled,
        SsThumb,
        SsThumbDisabled,
        SsMax
    };

    constexpr static float MeTrackSize = 6.f;
    constexpr static float MeThumbSize = 16.f;
private:
    CEasingCurveLite<Easing::FOutCubic> m_ec{};

    float m_fPos{};
    float m_fMin{};
    float m_fMax{ 100.0f };
    float m_fSmallDelta{ 1.0f };
    float m_fLargeDelta{ 10.0f };

    float m_fDragPos{};
    float m_cxyTrack{ MeTrackSize };
    float m_cxyThumb{ MeThumbSize };

    int m_msLastDuration{};

    BITBOOL m_bAnActive : 1{};

    BITBOOL m_bVertical : 1{};
    BITBOOL m_bThumbTrack : 1{};
    BITBOOL m_bTransparentSpace : 1{};  // 空白部分穿透鼠标
    BITBOOL m_bThinTrack : 1{};         // 轨道正常情况下显示为尺寸的一半，点燃时显示全尺寸
    BITBOOL m_bAutoTrackSize : 1{};     // 根据控件尺寸自动调整轨道尺寸

    SimpleStyle m_Style[SsMax]
    {
        // Track
        { IdTmInvalid,        IdCrBack,           IdTmInvalid },
        // Track Active
        { IdTmInvalid,        IdCrAccent,         IdTmInvalid },
        // Track Disabled
        { IdTmInvalid,        IdCrAccentDisabled, IdTmInvalid },
        // Thumb
        { IdCrAccent,         IdCrBack,           IdCrBorder, FLT_MAX, 1.f },
        // Thumb Disabled
        { IdCrAccentDisabled, IdCrBackDisabled,   IdCrBorderDisabled, FLT_MAX, 1.f },
    };

    // 计算轨道在当前状态下的实际尺寸，考虑动画
    constexpr float CalculateTrackShortSide() const noexcept
    {
        if (m_bThinTrack)
            if (m_bAnActive)
                return m_cxyTrack / 2.f + m_cxyTrack / 2.f * m_ec.K;
            else
                return ((TmGetState() & SaHot) ? m_cxyTrack : (m_cxyTrack / 2.f));
        else
            return m_cxyTrack;
    }

    // 返回CalculateTrackShortSide的结果
    constexpr float GetTrackRect(_Out_ Kw::Rect& rc) const noexcept
    {
        rc = GetViewRect();
        const auto cxyTrack = CalculateTrackShortSide();
        const auto dCap = GetTrackCapSpacing();
        if (m_bVertical)
        {
            rc.left += (rc.right - rc.left - cxyTrack) / 2.f;
            rc.right = rc.left + cxyTrack;
            rc.top += dCap;
            rc.bottom -= dCap;
        }
        else
        {
            rc.top += (rc.bottom - rc.top - cxyTrack) / 2.f;
            rc.bottom = rc.top + cxyTrack;
            rc.left += dCap;
            rc.right -= dCap;
        }
        return cxyTrack;
    }

    constexpr void GetThumbRect(float cxyTrack,
        const Kw::Rect& rcTrack, _Out_ Kw::Rect& rc) const noexcept
    {
        const auto cxy = m_bThinTrack ?
            (GetTrackCapSpacing() * m_ec.K) : GetTrackCapSpacing();
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
    constexpr void GetThumbRect(_Out_ Kw::Rect& rc) const noexcept
    {
        const auto cxy = GetTrackRect(rc);
        const float fScale = (GetTrackPosition() - m_fMin) / (m_fMax - m_fMin);
        if (m_bVertical)
            rc.bottom = rc.top + (rc.bottom - rc.top) * fScale;
        else
            rc.right = rc.left + (rc.right - rc.left) * fScale;
        GetThumbRect(cxy, rc, rc);
    }

    constexpr void SetDragPosition(float fPos) noexcept
    {
        m_fDragPos = fPos;
        if (m_fDragPos < m_fMin)
            m_fDragPos = m_fMin;
        else if (m_fDragPos > m_fMax)
            m_fDragPos = m_fMax;
    }

    void ChangePosition(float fPos, BOOL bDragging, BOOL bUpdateNow = TRUE) noexcept
    {
        Kw::Rect rcOldThumb;
        GetThumbRect(rcOldThumb);

        if (bDragging)
        {
            SetDragPosition(fPos);
            if (m_bThumbTrack)
                EvtPositionChanged();
        }
        else
            m_fPos = fPos;

        Kw::Rect rcThumb;
        GetThumbRect(rcThumb);
        UnionRect(rcThumb, rcThumb, rcOldThumb);
        Invalidate(rcThumb, bUpdateNow);
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
        {
            PAINTINFO ps;
            BeginPaint(ps, wParam, lParam);

            const auto bDisabled = (GetStyle() & DES_DISABLE);
            Kw::Rect rcTrack;
            const auto cxyTrack = GetTrackRect(rcTrack);
            ElementToClient(rcTrack);

            GetTheme()->Draw(
                this,
                &m_Style[SsTrack],
                m_bVertical ? IdPtTrackV : IdPtTrackH,
                MakeD2DRectF(rcTrack),
                &ps.rcfClip);

            const float fScale = (GetTrackPosition() - m_fMin) / (m_fMax - m_fMin);
            if (m_bVertical)
                rcTrack.bottom = rcTrack.top + (rcTrack.bottom - rcTrack.top) * fScale;
            else
                rcTrack.right = rcTrack.left + (rcTrack.right - rcTrack.left) * fScale;
            GetTheme()->Draw(
                this,
                &m_Style[bDisabled ? SsTrackDisabled : SsTrackActive],
                m_bVertical ? IdPtActiveTrackV : IdPtActiveTrackH,
                MakeD2DRectF(rcTrack),
                &ps.rcfClip);

            if (!m_bThinTrack || ((TmGetState() & SaHot) || m_bAnActive))
            {
                GetThumbRect(cxyTrack, rcTrack, rcTrack);
                GetTheme()->Draw(
                    this,
                    &m_Style[bDisabled ? SsThumbDisabled : SsThumb],
                    IdPtThumb,
                    MakeD2DRectF(rcTrack),
                    &ps.rcfClip);
            }

            DbgDrawFrame();
            EndPaint(ps);
        }
        return 0;

        case WM_NCHITTEST:
        {
            if (m_bTransparentSpace)
            {
                auto pt = *(Kw::Vec2*)lParam;
                ClientToElement(pt);
                Kw::Rect rcTrack;
                GetTrackRect(rcTrack);
                if (!PointInRect(rcTrack, pt))
                    return HTTRANSPARENT;
            }
        }
        break;

        case WM_MOUSEMOVE:
        {
            if (TmState() & SapLButtonDown)
                ChangePosition(HitTest(*(Kw::Vec2*)lParam), TRUE);
            else if (!(TmState() & SaHot))
            {
                TmState() |= SaHot;
                if (m_bThinTrack)
                {
                    m_ec.Start(0.f, 1.f, m_bAnActive);
                    m_bAnActive = TRUE;
                    GetWindow().KctWake();
                }
            }
        }
        return 0;

        case WM_MOUSELEAVE:
        {
            if (!(TmState() & SapLButtonDown) && (TmState() & SaHot))
            {
                TmState() &= ~SaHot;
                if (m_bThinTrack)
                {
                    m_ec.Start(1.f, 0.f, m_bAnActive);
                    m_bAnActive = TRUE;
                    GetWindow().KctWake();
                }
            }
        }
        return 0;

        case WM_LBUTTONDOWN:
        case WM_LBUTTONDBLCLK:
        {
            const auto& pt = *(Kw::Vec2*)lParam;

            Kw::Rect rcThumb;
            GetThumbRect(rcThumb);
            if (PointInRect(rcThumb, pt))
            {
                TmState() |= SapLButtonDown;
                SetCapture();
            }
            else
                ChangePosition(HitTest(pt), FALSE);
        }
        return 0;

        case WM_LBUTTONUP:
        case WM_CAPTURECHANGED:
        {
            if (!(TmState() & SapLButtonDown))
                break;
            TmState() &= ~SapLButtonDown;
            ReleaseCapture();

            const auto& pt = *(Kw::Vec2*)lParam;
            ChangePosition(HitTest(pt), FALSE);
        }
        return 0;

        case WM_CREATE:
            GetWindow().KctRegisterTimeLine(this);
            SetTheme(TmDefaultTheme(TmIsDarkMode()).Get());
            [[fallthrough]];
        case WM_SIZE:
            if (m_bAutoTrackSize)
                m_cxyTrack = (m_bVertical ? GetWidth() : GetHeight()) * 0.6f;
            break;

        case WM_DESTROY:
            GetWindow().KctUnregisterTimeLine(this);
            break;
        }
        return __super::OnEvent(uMsg, wParam, lParam);
    }

    void TlTick(int ms) noexcept override
    {
        m_msLastDuration = ms;
        m_bAnActive = m_ec.Tick((float)ms, 200);
        Invalidate(FALSE);
    }
    BOOL TlIsValid() noexcept override { return m_bAnActive; }
    int TlGetCurrentInterval() noexcept override { return (int)m_msLastDuration; }

    constexpr void SetRange(float fMin, float fMax) noexcept
    {
        m_fMin = fMin;
        m_fMax = fMax;
        if (m_fPos < m_fMin)
            m_fPos = m_fMin;
        else if (m_fPos > m_fMax)
            m_fPos = m_fMax;
    }
    EckInlineNdCe float GetMinimum() const noexcept { return m_fMin; }
    EckInlineNdCe float GetMaximum() const noexcept { return m_fMax; }

    constexpr void SetTrackPosition(float fPos) noexcept
    {
        m_fPos = fPos;
        if (m_fPos < m_fMin)
            m_fPos = m_fMin;
        else if (m_fPos > m_fMax)
            m_fPos = m_fMax;
    }
    EckInlineNdCe float GetTrackPosition() const noexcept
    {
        return (TmGetState() & SapLButtonDown) ? m_fDragPos : m_fPos;
    }

    EckInlineCe void SetVertical(BOOL bVertical) noexcept { m_bVertical = bVertical; }
    EckInlineNdCe BOOL GetVertical() const noexcept { return m_bVertical; }

    EckInlineCe void SetTrackSize(float f) noexcept { m_cxyTrack = f; }
    EckInlineNdCe float GetTrackSize() const noexcept { return m_cxyTrack; }

    EckInlineCe void SetThumbTrack(BOOL b) noexcept { m_bThumbTrack = b; }
    EckInlineNdCe BOOL GetThumbTrack() const noexcept { return m_bThumbTrack; }

    EckInlineCe void SetTransparentSpace(BOOL b) noexcept { m_bTransparentSpace = b; }
    EckInlineNdCe BOOL GetTransparentSpace() const noexcept { return m_bTransparentSpace; }

    EckInlineCe void SetThinTrack(BOOL b) noexcept { m_bThinTrack = b; }
    EckInlineNdCe BOOL GetThinTrack() const noexcept { return m_bThinTrack; }

    EckInlineCe void SetAutoTrackSize(BOOL b) { m_bAutoTrackSize = b; }
    EckInlineNdCe BOOL GetAutoTrackSize() const { return m_bAutoTrackSize; }

    EckInlineCe void SetSmallDelta(float f) noexcept { m_fSmallDelta = f; }
    EckInlineNdCe float GetSmallDelta() const noexcept { return m_fSmallDelta; }
    EckInlineCe void SetLargeDelta(float f) noexcept { m_fLargeDelta = f; }
    EckInlineNdCe float GetLargeDelta() const noexcept { return m_fLargeDelta; }

    constexpr float HitTest(Kw::Vec2 pt) noexcept
    {
        Kw::Rect rcTrack;
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
    EckInlineNdCe float GetTrackCapSpacing() const noexcept { return m_cxyThumb; }

    void EvtPositionChanged() noexcept
    {
        ELENMHDR nm{ ENC_POSCHANGED };
        SendNotify(&nm);
    }
};


class CTmTrackBar : public CThemeBase
{
public:
    TmResult Draw(
        CElement* pEle,
        const SimpleStyle* pStyle,
        UINT idPart,
        const D2D1_RECT_F& rc,
        _In_opt_ const D2D1_RECT_F* prcClip) noexcept override
    {
        if (idPart == CTrackBar::IdPtTrackH ||
            idPart == CTrackBar::IdPtTrackV ||
            idPart == CTrackBar::IdPtActiveTrackH ||
            idPart == CTrackBar::IdPtActiveTrackV)
        {
            const auto cxShortSide =
                (idPart == CTrackBar::IdPtTrackH || idPart == CTrackBar::IdPtActiveTrackH) ?
                (rc.bottom - rc.top) :
                (rc.right - rc.left);
            if (pStyle->rRound > cxShortSide / 2.f)
            {
                auto NewStyle{ *pStyle };
                NewStyle.rRound = cxShortSide / 2.f;
                pEle->TmGenericDrawBackground(&NewStyle, rc);
            }
            else
                pEle->TmGenericDrawBackground(pStyle, rc);
        }
        else if (idPart == CTrackBar::IdPtThumb)
        {
            const auto cxRect = rc.right - rc.left;
            float r;
            if (pStyle->rRound > cxRect / 2.f)
            {
                auto NewStyle{ *pStyle };
                r = NewStyle.rRound = cxRect / 2.f;
                pEle->TmGenericDrawBackground(&NewStyle, rc);
            }
            else
            {
                r = pStyle->rRound;
                pEle->TmGenericDrawBackground(pStyle, rc);
            }

            if (pStyle->CrFore != IdTmInvalid)
            {
                const auto pBrush = pEle->GetWindow().CcSetBrushColor(
                    ArgbToD2DColorF(GetColor(pStyle->CrFore)));
                const auto r = cxRect / 2.f;
                const D2D1_ELLIPSE Ell
                {
                    { (rc.left + rc.right) / 2.f, (rc.top + rc.bottom) / 2.f },
                    r * 0.6f, r * 0.6f
                };
                pEle->GetDC()->FillEllipse(Ell, pBrush);
            }
            return TmResult::Ok;
        }
    }
};
inline RcPtr<CThemeBase> CTrackBar::TmMakeDefaultTheme(BOOL bDark) noexcept
{
    const auto p = RcPtr<CTmTrackBar>::Make();
    p->SetMetricCollection(TmsMetricCollection().Get());
    p->SetColorCollection(bDark ?
        TmsColorCollectionDark().Get() :
        TmsColorCollectionLight().Get());
    return p;
}

class CUiaTrackBar : public CUnknownAppend<CUiaBase, IRangeValueProvider>
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
            pRetVal->intVal = UIA_SliderControlTypeId;
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
        const auto pEle = DbgDynamicCast<CTrackBar*>(GetElement());
        pEle->SetTrackPosition((float)Val);
        pEle->Invalidate();
        pEle->EvtPositionChanged();
        return S_OK;
    }
    STDMETHODIMP get_Value(double* pRetVal) override
    {
        if (!GetElement())
            return UIA_E_ELEMENTNOTAVAILABLE;
        *pRetVal = DbgDynamicCast<CTrackBar*>(GetElement())->GetTrackPosition();
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
        *pRetVal = DbgDynamicCast<CTrackBar*>(GetElement())->GetMaximum();
        return S_OK;
    }
    STDMETHODIMP get_Minimum(double* pRetVal) override
    {
        if (!GetElement())
            return UIA_E_ELEMENTNOTAVAILABLE;
        *pRetVal = DbgDynamicCast<CTrackBar*>(GetElement())->GetMinimum();
        return S_OK;
    }
    STDMETHODIMP get_LargeChange(double* pRetVal) override
    {
        if (!GetElement())
            return UIA_E_ELEMENTNOTAVAILABLE;
        *pRetVal = DbgDynamicCast<CTrackBar*>(GetElement())->GetLargeDelta();
        return S_OK;
    }
    STDMETHODIMP get_SmallChange(double* pRetVal) override
    {
        if (!GetElement())
            return UIA_E_ELEMENTNOTAVAILABLE;
        *pRetVal = DbgDynamicCast<CTrackBar*>(GetElement())->GetSmallDelta();
        return S_OK;
    }
};
inline HRESULT CTrackBar::EhUiaMakeInterface() noexcept
{
    const auto p = new CUiaTrackBar{};
    UiaSetInterface(p);
    p->Release();
    return S_OK;
}
ECK_DUI_NAMESPACE_END
ECK_NAMESPACE_END