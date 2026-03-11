#pragma once
#include "DuiBase.h"

ECK_NAMESPACE_BEGIN
ECK_DUI_NAMESPACE_BEGIN
class CButton : public CElement
{
private:
    ComPtr<IDWriteTextLayout> m_pLayout{};
    RcPtr<CBitmap> m_pBitmap{};
    BOOLEAN m_bAutoScale{ TRUE };
    BYTE m_eInter{ D2D1_INTERPOLATION_MODE_LINEAR };
    float m_fOpacity{ 1.f };

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
            m_pLayout.AddrOfClear());
    }

    void UpdateTextLayout() noexcept
    {
        UpdateTextLayout(GetText().Data(), GetText().Size());
    }

    void TmpStateChanged() noexcept
    {
        Invalidate();
        GetWindow().RdRenderAndPresent();
    }
public:
    static RcPtr<CThemeBase> TmMakeDefaultTheme() noexcept;
    static RcPtr<CThemeBase> TmDefaultTheme() noexcept;

    static RcPtr<CThemeStyle> TmMakeDefaultStyle(BOOL bDarkMode) noexcept
    {
        auto p = RcPtr<CThemeStyle>::Make();
        auto& vStyle = p->GetList();
        vStyle.resize(3);
        vStyle[0] =
        {
            .uState = SaNormal,
            .argbFore = (bDarkMode ? 0xFF'FFFFFF : 0xFF'000000),
            .argbBack = (bDarkMode ? 0xFF'727272 : 0xFF'F0F0F0),
            .argbBorder = (bDarkMode ? 0xFF'727272 : 0xFF'C0C0C0),
        };
        vStyle[1] =
        {
            .uState = SaHot,
            .argbFore = (bDarkMode ? 0xFF'FFFFFF : 0xFF'000000),
            .argbBack = (bDarkMode ? 0xFF'727272 : 0xFF'E0E0E0),
            .argbBorder = (bDarkMode ? 0xFF'727272 : 0xFF'C0C0C0),
        };
        vStyle[2] =
        {
            .uState = SaActive,
            .argbFore = (bDarkMode ? 0xFF'FFFFFF : 0xFF'000000),
            .argbBack = (bDarkMode ? 0xFF'727272 : 0xFF'C0C0C0),
            .argbBorder = (bDarkMode ? 0xFF'727272 : 0xFF'C0C0C0),
        };
        p->SetBorderWidth(1);
        return p;
    }

    HRESULT EhUiaMakeInterface() noexcept;

    LRESULT OnEvent(UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept override
    {
        switch (uMsg)
        {
        case WM_PAINT:
        {
            PAINTINFO ps;
            BeginPaint(ps, wParam, lParam);
            const CThemeStyle::Style* pSubStyle;
            GetTheme()->Draw(this, IdPtNormal, GetRectInClientD2D(), &pSubStyle);

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

            if (m_pLayout.Get() && pSubStyle)
            {
                Kw::Vec2 pt{ rc.left, (GetHeight() - tm.height) / 2.f };
                pt += GetOffsetInClient();
                GetDC()->DrawTextLayout(
                    ReinterpretValue<D2D1_POINT_2F>(pt),
                    m_pLayout.Get(),
                    GetWindow().CcSetBrushColor(ArgbToD2DColorF(pSubStyle->argbFore)));
            }

            DbgDrawFrame();
            EndPaint(ps);
        }
        return 0;

        case WM_MOUSEMOVE:
            if (TmState() & SaHot)
                break;
            TmState() |= SaHot;
            TmpStateChanged();
            break;
        case WM_MOUSELEAVE:
            if (!(TmState() & SaHot))
                break;
            TmState() &= ~SaHot;
            TmpStateChanged();
            break;
        case WM_LBUTTONDOWN:
        case WM_LBUTTONDBLCLK:
            SetFocus();
            TmState() |= (SaActive | SapLButtonDown);
            GetContainer()->EleSetCapture((THost*)this);
            TmpStateChanged();
            break;
        case WM_LBUTTONUP:
            if (TmState() & SapLButtonDown)
                GetContainer()->EleReleaseCapture();
            TmpStateChanged();
            break;
        case WM_CAPTURECHANGED:
            if (TmState() & SapLButtonDown)
            {
                TmState() &= ~(SaActive | SapLButtonDown);
                TmpStateChanged();
            }
            break;
        case WM_SETFOCUS:
            if (TmState() & SaFocus)
                break;
            TmState() |= SaFocus;
            TmpStateChanged();
            break;
        case WM_KILLFOCUS:
            if (!(TmState() & SaFocus))
                break;
            TmState() &= ~SaFocus;
            TmpStateChanged();
            break;
        case WM_STYLECHANGED:
            if (!(((UINT)wParam ^ GetStyle()) & DES_DISABLE))
                break;
            if (GetStyle() & DES_DISABLE)
            {
                TmState() |= SaDisable;
                TmpStateChanged();
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
                TmpStateChanged();
            }
            SetTheme(TmDefaultTheme().Get());
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
        ELENMHDR nm{ ENM_COMMAND };
        SendNotify(&nm);
    }

    EckInline void SetIcon(RcPtr<CBitmap> p) noexcept { m_pBitmap = p; }
    EckInline RcPtr<CBitmap> GetIcon() noexcept { return m_pBitmap; }
};

class CTmButton : public CThemeBase
{
public:
    TmResult Draw(CElement* pEle, UINT idPart, const D2D1_RECT_F& rc,
        _Out_opt_ const CThemeStyle::Style** ppStyle) noexcept override
    {
        if (idPart != IdPtNormal)
        {
            if (ppStyle)
                *ppStyle = nullptr;
            return TmResult::NotSupport;
        }
        const auto pStyle = pEle->TmSelectSubStyle();
        if (ppStyle)
            *ppStyle = pStyle;
        if (!pStyle)
            return TmResult::NoStyle;
        return pEle->TmGenericDrawBackground(pStyle, rc);
    }
};
inline RcPtr<CThemeBase> CButton::TmMakeDefaultTheme() noexcept
{
    const auto p = RcPtr<CTmButton>::Make();
    p->SetMetricCollection(TmDefaultMetricCollection().Get());
    return p;
}
inline RcPtr<CThemeBase> CButton::TmDefaultTheme() noexcept
{
    static auto p{ TmMakeDefaultTheme() };
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

    STDMETHODIMP Invoke() override
    {
        DbgDynamicCast<CButton*>(m_pEle)->EvtClick();
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