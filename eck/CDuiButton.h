#pragma once
#include "DuiBase.h"

ECK_NAMESPACE_BEGIN
ECK_DUI_NAMESPACE_BEGIN
class CButton : public CElement
{
public:
    enum : UINT
    {
        SsNormal,
        SsHot,
        SsPressed,
        SsDisabled,
        SsMax
    };
private:
    ComPtr<IDWriteTextLayout> m_pLayout{};
    RcPtr<CBitmap> m_pBitmap{};
    BOOLEAN m_bSpacePressed{};
    BOOLEAN m_bAutoScale{ TRUE };
    BYTE m_eInter{ D2D1_INTERPOLATION_MODE_LINEAR };
    float m_fOpacity{ 1.f };
    SimpleStyle m_Style[SsMax]
    {
        TmsSsMakeNormal(1.f),
        TmsSsMakeHot(1.f),
        TmsSsMakePressed(1.f),
        TmsSsMakeDisabled(1.f),
    };

    Kw::Vec2 CalculateBitmapSize() const noexcept
    {
        EckAssert(m_pBitmap.Get());
        const auto rc = m_pBitmap->GetActualSourceRect();
        const auto cx = rc.right - rc.left;
        const auto cy = rc.bottom - rc.top;
        if (m_bAutoScale)
        {
            const float dOuter = GetTheme()->GetMetric(IdMePaddingOuter);
            const auto cyNew = GetHeight() - dOuter * 2;
            return { cyNew * cx / cy, cyNew };
        }
        return { cx, cy };
    }

    void UpdateTextLayout(PCWSTR pszText, int cchText) noexcept
    {
        const float dOuter = GetTheme()->GetMetric(IdMePaddingOuter);
        const float dInner = GetTheme()->GetMetric(IdMePaddingInner);

        float cxMax = GetWidth() - dOuter * 2.f;
        if (m_pBitmap.Get())
            cxMax -= (CalculateBitmapSize().x + dInner);

        g_pDwFactory->CreateTextLayout(
            pszText, cchText, GetTextFormat(),
            cxMax, GetHeight() - dOuter * 2,
            m_pLayout.AtClear());
    }

    void UpdateTextLayout() noexcept
    {
        UpdateTextLayout(GetText().Data(), GetText().Size());
    }

    EckInlineNdCe static UINT TmSimpleStyleFromThemeState(UINT sa) noexcept
    {
        if (sa & SaDisable)
            return SsDisabled;
        if (sa & SaPressed)
            return SsPressed;
        if (sa & SaHot)
            return SsHot;
        return SsNormal;
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
            const auto& Ss = m_Style[TmSimpleStyleFromThemeState(TmGetState())];
            GetTheme()->Draw(
                this,
                &Ss,
                IdPtNormal,
                GetRectInClientD2D(),
                nullptr);

            const float dInner = GetTheme()->GetMetric(IdMePaddingInner);

            DWRITE_TEXT_METRICS tm;
            if (m_pLayout.Get())
                m_pLayout->GetMetrics(&tm);
            else
                tm.width = tm.height = 0;

            D2D1_RECT_F rc;
            if (m_pBitmap.Get())
            {
                const auto sizeBitmap = CalculateBitmapSize();
                rc.left = (GetWidth() - sizeBitmap.x -
                    (tm.width ? (dInner - tm.width) : 0.f)) / 2.f;
                rc.top = (GetHeight() - sizeBitmap.y) / 2.f;
                rc.right = rc.left + sizeBitmap.x;
                rc.bottom = rc.top + sizeBitmap.y;
                GetDC()->DrawBitmap(m_pBitmap->Get(), &rc, m_fOpacity,
                    (D2D1_INTERPOLATION_MODE)m_eInter, m_pBitmap->GetSourceRect());
                rc.left = rc.right + dInner;
            }
            else
                rc.left = (GetWidth() - tm.width) / 2.f;

            if (m_pLayout.Get())
            {
                Kw::Vec2 pt{ rc.left, (GetHeight() - tm.height) / 2.f };
                pt += GetOffsetInClient();
                GetDC()->DrawTextLayout(
                    Kw::MakeD2DPointF(pt),
                    m_pLayout.Get(),
                    GetWindow().CcSetBrushColor(
                        ArgbToD2DColorF(GetTheme()->GetColor(Ss.CrFore))));
            }

            DbgDrawFrame();
            EndPaint(ps);
        }
        return 0;

        case WM_MOUSEMOVE:
            if (TmState() & SaHot)
                break;
            TmState() |= SaHot;
            Invalidate();
            break;
        case WM_MOUSELEAVE:
            if (!(TmState() & SaHot))
                break;
            TmState() &= ~SaHot;
            Invalidate();
            break;
        case WM_LBUTTONDOWN:
        case WM_LBUTTONDBLCLK:
            SetFocus();
            TmState() |= (SaPressed | SapLButtonDown);
            GetContainer()->EleSetCapture((THost*)this);
            Invalidate();
            break;
        case WM_LBUTTONUP:
            if (TmState() & SapLButtonDown)
                GetContainer()->EleReleaseCapture();
            Invalidate();
            break;
        case WM_CAPTURECHANGED:
            if (TmState() & SapLButtonDown)
            {
                TmState() &= ~(SaPressed | SapLButtonDown);
                Invalidate();
            }
            break;

        case WM_KEYDOWN:
        {
            if (wParam != VK_SPACE || m_bSpacePressed)
                break;
            m_bSpacePressed = TRUE;
            if (!(TmState() & SaPressed))
            {
                TmState() |= SaPressed;
                Invalidate();
            }
        }
        break;
        case WM_KEYUP:
        {
            if (wParam != VK_SPACE || !m_bSpacePressed)
                break;
            m_bSpacePressed = FALSE;
            if (TmState() & SaPressed)
            {
                EvtClick();
                TmState() &= ~SaPressed;
                Invalidate();
            }
        }
        break;

        case WM_SETFOCUS:
            if (TmState() & SaFocus)
                break;
            TmState() |= SaFocus;
            Invalidate();
            break;
        case WM_KILLFOCUS:
            if (!(TmState() & SaFocus))
                break;
            TmState() &= ~SaFocus;
            Invalidate();
            break;
        case WM_STYLECHANGED:
            if (!(((UINT)wParam ^ GetStyle()) & DES_DISABLE))
                break;
            if (GetStyle() & DES_DISABLE)
            {
                TmState() |= SaDisable;
                Invalidate();
            }
            break;

        case WM_SIZE:
            UpdateTextLayout();
            return 0;

        case WM_SETTEXT:
            UpdateTextLayout((PCWSTR)lParam, (int)wParam);
            Invalidate();
            return 0;
        case WM_SETFONT:
            UpdateTextLayout();
            return 0;

        case WM_CREATE:
            if (GetStyle() & DES_DISABLE)
            {
                TmState() |= SaDisable;
                Invalidate();
            }
            SetTheme(TmDefaultTheme(TmIsDarkMode()).Get());
            UpdateTextLayout();
            break;
        case WM_DESTROY:
        {
            m_pLayout.Clear();
            m_pBitmap.Clear();
        }
        return 0;
        }
        return __super::OnEvent(uMsg, wParam, lParam);
    }

    void EvtClick() noexcept
    {
        ELENMHDR nm{ ENC_COMMAND };
        SendNotify(&nm);
    }

    EckInline void SetIcon(RcPtr<CBitmap> p) noexcept { m_pBitmap = p; }
    EckInline RcPtr<CBitmap> GetIcon() noexcept { return m_pBitmap; }
};

class CTmButton : public CThemeBase
{
public:
    TmResult Draw(
        CElement* pEle,
        const SimpleStyle* pStyle,
        UINT idPart,
        const D2D1_RECT_F& rc,
        _In_opt_ const D2D1_RECT_F* prcClip) noexcept override
    {
        if (idPart != IdPtNormal)
            return TmResult::NotSupport;
        return pEle->TmGenericDrawBackground(pStyle, rc);
    }
};
inline RcPtr<CThemeBase> CButton::TmMakeDefaultTheme(BOOL bDark) noexcept
{
    const auto p = RcPtr<CTmButton>::Make();
    p->SetMetricCollection(TmsMetricCollection().Get());
    p->SetColorCollection(bDark ?
        TmsColorCollectionDark().Get() :
        TmsColorCollectionLight().Get());
    return p;
}

class CUiaButton : public CUnknownAppend<CUiaBase, IInvokeProvider>
{
    STDMETHODIMP GetPatternProvider(PATTERNID idPattern, IUnknown** pRetVal) override
    {
        if (idPattern == UIA_InvokePatternId)
        {
            *pRetVal = static_cast<IInvokeProvider*>(this);
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
            pRetVal->intVal = UIA_ButtonControlTypeId;
            return S_OK;
        }
        return CUiaBase::GetPropertyValue(idProp, pRetVal);
    }

    STDMETHODIMP Invoke() override
    {
        if (!GetElement())
            return UIA_E_ELEMENTNOTAVAILABLE;
        if (GetElement()->GetStyle() & DES_DISABLE)
            return UIA_E_ELEMENTNOTENABLED;
        DbgDynamicCast<CButton*>(GetElement())->EvtClick();
        return S_OK;
    }
};
inline HRESULT CButton::EhUiaMakeInterface() noexcept
{
    const auto p = new CUiaButton{};
    UiaSetInterface(p);
    p->Release();
    return S_OK;
}
ECK_DUI_NAMESPACE_END
ECK_NAMESPACE_END