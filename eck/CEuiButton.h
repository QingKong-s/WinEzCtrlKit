#pragma once
#include "EuiBase.h"

ECK_NAMESPACE_BEGIN
ECK_EUI_NAMESPACE_BEGIN
class CButton : public CElement
{
public:
    static RcPtr<CThemeBase> TmDefaultTheme() noexcept;

    static RcPtr<CThemeStyle> TmMakeDefaultStyle(BOOL bDarkMode) noexcept
    {
        auto p = RcPtr<CThemeStyle>::Make();
        auto& vStyle = p->GetList();
        vStyle.resize(3);
        vStyle[0] =
        {
            .uState = SaNormal,
            .argbBack = (bDarkMode ? 0xFF'727272 : 0xFF'F0F0F0),
            .argbBorder = (bDarkMode ? 0xFF'727272 : 0xFF'C0C0C0),
        };
        vStyle[1] =
        {
            .uState = SaHot,
            .argbBack = (bDarkMode ? 0xFF'727272 : 0xFF'E0E0E0),
            .argbBorder = (bDarkMode ? 0xFF'727272 : 0xFF'C0C0C0),
        };
        vStyle[2] =
        {
            .uState = SaPressed,
            .argbBack = (bDarkMode ? 0xFF'727272 : 0xFF'C0C0C0),
            .argbBorder = (bDarkMode ? 0xFF'727272 : 0xFF'C0C0C0),
        };
        p->SetBorderWidth(1);
        return p;
    }

    HRESULT EhUiaMakeInterface() noexcept override;

    LRESULT OnEvent(UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept override
    {
        const auto uOldThemeState = TmGetState();
        TmOnEvent(uMsg, wParam, lParam);
        switch (uMsg)
        {
        case WM_PAINT:
        {
            PAINTINFO ps;
            BeginPaint(ps, wParam, lParam);
            GetTheme()->Draw(this, IdPtNormal, GetRectInClient());
            EndPaint(ps);
        }
        return 0;
        case WM_LBUTTONDOWN:
        case WM_LBUTTONDBLCLK:
            SetFocus();
            break;
        case WM_LBUTTONUP:
            if (!(TmGetState() & SaPressed) &&
                uOldThemeState & SaPressed)
                EvtClick();
            break;
        case WM_SETFOCUS:
        case WM_KILLFOCUS:
            if (IsShowingFocus())
                Redraw();
            break;
        case EWM_SHOWFOCUS:
            Redraw();
            break;
        case WM_CREATE:
            SetTheme(TmDefaultTheme().Get());
            break;
        }
        if ((TmGetState() ^ uOldThemeState) & (SaHot | SaPressed | SaDisable))
            Redraw();
        return __super::OnEvent(uMsg, wParam, lParam);
    }

    void EvtClick() noexcept
    {
        ELENMHDR nm{ ENC_COMMAND };
        SendNotify(&nm);
    }
};

class CTmButton : public CThemeBase
{
public:
    TmResult Draw(CElement* pEle, UINT idPart, const RECT& rc) noexcept override
    {
        if (idPart != IdPtNormal)
            return TmResult::NotSupport;
        const auto pStyle = TmSelectSubStyle(pEle);
        if (!pStyle)
            return TmResult::NoStyle;
        const auto r = TmGenericDrawBackground(pEle, pStyle, rc);
        if (r != TmResult::Ok)
            return r;
        const auto uDt = DT_SINGLELINE | DT_CENTER | DT_VCENTER |
            ((pEle->GetStyle() & DES_NO_CLIP) ? 0 : DT_NOCLIP);
        return TmGenericDrawText(pEle, pStyle, rc, uDt);
    }
};
inline RcPtr<CThemeBase> CButton::TmDefaultTheme() noexcept
{
    static CTmButton s_Theme{};
    return RcPtr<CThemeBase>{ static_cast<CThemeBase*>(&s_Theme) };
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
ECK_EUI_NAMESPACE_END
ECK_NAMESPACE_END