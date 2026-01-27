#pragma once
#include "DuiCssParse.h"
#include "DuiTheme.h"
#include "StringUtility.h"

ECK_NAMESPACE_BEGIN
ECK_DUI_NAMESPACE_BEGIN
struct CSSTHEME_FILL_PARAM
{
    BOOLEAN bRemoveFilled;
};

inline void CssFillDrawEntry(_Inout_ THEME_DRAW_ENTRY& Draw,
    const std::vector<CSS_STYLE>& vStyle, const CSSTHEME_FILL_PARAM& Param) noexcept
{
    Draw.eDraw = ThemeDraw::None;
    for (const auto& e : vStyle)
    {
#define ECK_CSS_HIT_STYLE(Str) TcsEqualLen2I(e.svKey.data(), e.svKey.size(), EckStrAndLen(Str))
        if (ECK_CSS_HIT_STYLE(L"background-color") ||
            ECK_CSS_HIT_STYLE(L"color"))
        {
            if (Draw.eDraw != ThemeDraw::None)
                continue;
            Draw.eDraw = ThemeDraw::Solid;
            if (CssIsColorization(e.svValue))
                Draw.bColorization = TRUE;
            else
                CssParseColor(e.svValue, Draw.Color);
        }
        //
        else if (ECK_CSS_HIT_STYLE(L"geometry"))
        {
            if (Draw.eDraw != ThemeDraw::None)
                continue;
            Draw.eDraw = ThemeDraw::Geometry;
            Draw.f = 1.f;
            Draw.Geometry.eType = CssNcToGeometryType(e.svValue);
        }
        else if (ECK_CSS_HIT_STYLE(L"geo-frame-color"))
        {
            if (Draw.eDraw != ThemeDraw::Geometry &&
                Draw.eDraw != ThemeDraw::None)
                continue;
            if (CssIsColorization(e.svValue))
                Draw.bColorization = TRUE;
            else
                CssParseColor(e.svValue, Draw.Geometry.crFrame);
        }
        else if (ECK_CSS_HIT_STYLE(L"geo-fill-color"))
        {
            if (Draw.eDraw != ThemeDraw::Geometry &&
                Draw.eDraw != ThemeDraw::None)
                continue;
            if (CssIsColorization(e.svValue))
                Draw.bColorization = TRUE;
            else
                CssParseColor(e.svValue, Draw.Geometry.crFill);
        }
        else if (ECK_CSS_HIT_STYLE(L"geo-frame-width"))
        {
            if (Draw.eDraw == ThemeDraw::Geometry &&
                Draw.eDraw == ThemeDraw::None)
                Draw.f = 1.f;// TODO
        }
        //
#undef ECK_CSS_HIT_STYLE
    }
}

class CCssTheme : public CThemeCommonBase
{
protected:
    CThemeDrawList m_List{};
    ComPtr<ID2D1SolidColorBrush> m_pBrSolid{};
public:
    HRESULT FillCss(const CSS_DOC& Css, const CSSTHEME_FILL_PARAM& Param) noexcept
    {
        for (const auto& e : Css.vSel)
        {
            if (e.eType != CssSelType::Tag)
                continue;
            const auto ePart = CssNcTagToPart(e.svName);
            if (ePart == Part::Invalid)
                continue;
            const auto eState = CssNcPseudoToState(e.svPseudo);
            if (eState == State::None)
                continue;
            CssFillDrawEntry(m_List.Add(ePart, eState), e.vStyle, Param);
        }
        return S_OK;
    }

    HRESULT RealizeForDC(ID2D1DeviceContext* pDC) noexcept override
    {
        pDC->CreateSolidColorBrush({}, m_pBrSolid.AddrOfClear());
        return __super::RealizeForDC(pDC);
    }
    HRESULT DrawBackground(Part ePart, State eState,
        const D2D1_RECT_F& rc, _In_opt_ const DTB_OPT* pOpt) noexcept override
    {
        const auto it = m_List.LookupPartState(ePart, eState);
        if (it == m_List.InvalidIterator() ||
            it->eDraw == ThemeDraw::None)
            return __super::DrawBackground(ePart, eState, rc, pOpt);
        if (!pOpt)
            pOpt = &DtbOptDefault;
        if (pOpt->uFlags & DTBO_CLIP_RECT)
            m_pDC->PushAxisAlignedClip(pOpt->rcClip, D2D1_ANTIALIAS_MODE_ALIASED);
        HRESULT hr = S_OK;
        switch (it->eDraw)
        {
        case ThemeDraw::Solid:
        {
            if (pOpt->uFlags & DTBO_NEW_OPACITY)
            {
                auto cr{ it->Color };
                cr.a *= pOpt->fOpacity;
                m_pBrSolid->SetColor(cr);
            }
            else
                m_pBrSolid->SetColor(it->Color);
            m_pDC->FillRectangle(rc, m_pBrSolid.Get());
        }
        break;
        case ThemeDraw::Image:
        case ThemeDraw::ImageNineGrid:
        {
            const auto b9Grid = (it->eDraw == ThemeDraw::ImageNineGrid);
            const auto fOpacity = pOpt->uFlags & DTBO_NEW_OPACITY ?
                pOpt->fOpacity : it->f;
            const auto eInterMode = pOpt->uFlags & DTBO_NEW_INTERPOLATION_MODE ?
                pOpt->eInterpolationMode : D2D1_INTERPOLATION_MODE_LINEAR;
            if (it->bNoStretch)
            {
                D2D1_RECT_F rcDst{ it->Img.rcSrc };
                CenterRect(rcDst, rc);
                if (b9Grid)
                    DrawImageFromGrid(m_pDC.Get(), it->Img.pBmp, rcDst, it->Img.rcSrc,
                        it->Img.Margins, eInterMode, fOpacity);
                else
                    m_pDC->DrawBitmap(it->Img.pBmp, rcDst, fOpacity, eInterMode,
                        it->Img.rcSrc);
            }
            else
            {
                if (b9Grid)
                    DrawImageFromGrid(m_pDC.Get(), it->Img.pBmp, rc, it->Img.rcSrc,
                        it->Img.Margins, eInterMode, fOpacity);
                else
                    m_pDC->DrawBitmap(it->Img.pBmp, rc, fOpacity, eInterMode,
                        it->Img.rcSrc);
            }
        }
        break;
        case ThemeDraw::Geometry:
        {
            THEME_DRAW_GEO_INFO Info
            {
                .pOpt = pOpt,
                .pDC = m_pDC.Get(),
                .pBrush = m_pBrSolid.Get(),
                .eType = it->Geometry.eType,
                .bNoStretch = it->bNoStretch,

                .cxBorder = it->f,
                // HACK: 分离边框颜色
                .crFrame = it->bColorization ? m_crColorization : it->Geometry.crFrame,
                .crFill = it->bColorization ? m_crColorization : it->Geometry.crFill,
            };
            if (Info.pOpt->uFlags & DTBO_NEW_OPACITY)
            {
                Info.crFrame.a *= pOpt->fOpacity;
                Info.crFill.a *= pOpt->fOpacity;
            }
            hr = ThDrawGeometry(Info, rc, it->Geometry.Param) ? S_OK : E_UNEXPECTED;
        }
        break;
        default:
            return __super::DrawBackground(ePart, eState, rc, pOpt);
        }
        if (pOpt->uFlags & DTBO_CLIP_RECT)
            m_pDC->PopAxisAlignedClip();
        return S_OK;
    }
    HRESULT GetColor(Part ePart, State eState,
        ClrPart eClrPart, _Out_ D2D1_COLOR_F& cr) noexcept override
    {
        const auto it = m_List.LookupPartState(ePart, eState);
        if (it != m_List.InvalidIterator())
        {
            switch (it->eDraw)
            {
            case ThemeDraw::Solid:
                cr = it->Color;
                return S_OK;
            case ThemeDraw::Geometry:
                switch (eClrPart)
                {
                case ClrPart::Bk:
                    cr = it->Geometry.crFrame;
                    return S_OK;
                case ClrPart::Border:
                    cr = it->Geometry.crFrame;
                    return S_OK;
                }
            }
            return HRESULT_FROM_WIN32(ERROR_UNKNOWN_PROPERTY);
        }
        return __super::GetColor(ePart, eState, eClrPart, cr);
    }
};

// 附加于CElem
class CCssInfo
{
private:
    BYTE m_cchId{};
    BYTE m_cchClass{};
    BITBOOL m_bPointerEvent : 1{};
    CCssTheme m_Theme{};// 特定于控件的主题

    // WCHAR szId[m_cchId + 1];
    // WCHAR szClass[m_cchClass + 1];

    CCssInfo() = default;
public:
    static CCssInfo* New(std::wstring_view svId, std::wstring_view svClass) noexcept
    {
        const auto p = (CCssInfo*)malloc(sizeof(CCssInfo) +
            (svId.size() + 1 + svClass.size() + 1) * sizeof(WCHAR));
        EckCheckMem(p);
        new (p) CCssInfo{};
        p->m_cchId = (BYTE)svId.size();
        p->m_cchClass = (BYTE)svClass.size();
        auto pszDst = PWCH(p + 1);
        TcsCopyLenEnd(pszDst, svId.data(), svId.size());
        pszDst += (svId.size() + 1);
        TcsCopyLenEnd(pszDst, svClass.data(), svClass.size());
        return p;
    }
    static void Delete(CCssInfo* p) noexcept
    {
        p->~CCssInfo();
        free(p);
    }

    void GetTheme(_Out_ ITheme*& pTheme) noexcept
    {
        pTheme = &m_Theme;
        pTheme->AddRef();
    }

    EckInlineNdCe std::wstring_view GetId() const { return { PCWSTR(this + 1), m_cchId }; }
    EckInlineNdCe std::wstring_view GetClass() const { return { PCWSTR(this + 1) + m_cchId + 1, m_cchClass }; }
};
ECK_DUI_NAMESPACE_END
ECK_NAMESPACE_END