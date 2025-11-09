#pragma once
#include "DuiCssTheme.h"

ECK_NAMESPACE_BEGIN
ECK_DUI_NAMESPACE_BEGIN
EckInlineNdCe D2D1_COLOR_F StMakeForegroundColorLight(float fOpacity)
{
    return { 0.f,0.f,0.f,fOpacity };
}
EckInlineNdCe D2D1_COLOR_F StMakeForegroundColorDark(float fOpacity)
{
    return { 1.f,1.f,1.f,fOpacity };
}
EckInlineNdCe D2D1_COLOR_F StMakeBackgroundColorLight(float fOpacity)
{
    return { 1.f,1.f,1.f,fOpacity };
}
EckInlineNdCe D2D1_COLOR_F StMakeBackgroundColorDark(float fOpacity)
{
    return { 0.f,0.f,0.f,fOpacity };
}

constexpr inline D2D1_COLOR_F SysColorLight[]
{
    StMakeForegroundColorLight(1.f),        // SysText
    StMakeBackgroundColorLight(1.f),        // SysBk
    ColorrefToD2DColorF(RGB(0, 51, 153)),   // SysMainTitle
};
constexpr inline D2D1_COLOR_F SysColorDark[]
{
    StMakeForegroundColorDark(1.f),         // SysText
    StMakeBackgroundColorDark(1.f),         // SysBk
    ColorrefToD2DColorF(RGB(0, 168, 255)),  // SysMainTitle
};
constexpr inline float StdMetrics[]
{
    14.f,
    10.f,
    10.f,

    14.f,
    10.f,
    10.f,

    4.f,
    6.f,
    2.f,

    1.f,
    1.f,
    6.f,
    6.f
};

constexpr inline std::wstring_view StdLight{ LR"___(
Button {
    geometry: FillFrameRect;
    geo-frame-color: #19000000;
    geo-fill-color: #B2FFFFFF;
}
Button:hover {
    geometry: FillFrameRect;
    geo-frame-color: #19000000;
    geo-fill-color: #19000000;
}
Button:active {
    geometry: FillFrameRect;
    geo-frame-color: #19000000;
    geo-fill-color: #33000000;
}

CircleButton {
    geometry: FillEllipse;
    geo-fill-color: #B2FFFFFF;
}
CircleButton:hover {
    geometry: FillEllipse;
    geo-fill-color: #19000000;
}
CircleButton:active {
    geometry: FillEllipse;
    geo-fill-color: #33000000;
}

Edit {
    geometry: FillFrameRect;
    geo-frame-color: #33000000;
    geo-fill-color: #CCF9F9F9;
}
Edit:hover {
    geometry: FillFrameRect;
    geo-frame-color: #33000000;
    geo-fill-color: #CCEDEDED;
}
Edit:focus {
    geometry: FillFrameRect;
    geo-frame-color: #33000000;
    geo-fill-color: #E5FFFFFF;
}

EditBottomBar {
    geometry: FillRect;
    geo-fill-color: #CC7F7F7F;
}
EditBottomBar:focus {
    geometry: FillRect;
    geo-fill-color: colorization;
}

ListSelRect {
    geometry: FillFrameRect;
    geo-frame-color: #CC0089F9;
    geo-fill-color: #660073CC;
}

ListItem:hover {
    geometry: FillRect;
    geo-fill-color: #19000000;
}
ListItem:active {
    geometry: FillRect;
    geo-fill-color: #33000000;
}
ListItem:hover:active {
    geometry: FillRect;
    geo-fill-color: #3F000000;
}

HeaderItem {
    geometry: FillRect;
    geo-fill-color: #CCFFFFFF;
}
HeaderItem:hover {
    geometry: FillRect;
    geo-fill-color: #33000000;
}
HeaderItem:active {
    geometry: FillRect;
    geo-fill-color: #3F000000;
}

TrackBar {
    geometry: FillRoundRect;
    geo-fill-color: #FFD8D8D8;
}
TrackBar:active {
    geometry: FillRoundRect;
    geo-fill-color: colorization;
}
TrackBarThumb {
    geometry: FrameRing;
    geo-ring-radius-outter: 10 10;
    geo-ring-radius-inner: 6 6;
    geo-frame-color: #FFE5E5E5;
    geo-fill-color: #FFFFFFFF;
    geo-extra-color: colorization;
}
TrackBarThumb:hover {
    geometry: FrameRing;
    geo-ring-radius-outter: 10 10;
    geo-ring-radius-inner: 7.5 7.5;
    geo-frame-color: #FFE5E5E5;
    geo-fill-color: #FFFFFFFF;
    geo-extra-color: colorization;
}
TrackBarThumb:active {
    geometry: FrameRing;
    geo-ring-radius-outter: 10 10;
    geo-ring-radius-inner: 8 8;
    geo-frame-color: #FFE5E5E5;
    geo-fill-color: #FFFFFFFF;
    geo-extra-color: colorization;
}

ScrollBar:hover {
    geometry: FillRect;
    geo-fill-color: #7FE5D8D8;
}
ScrollThumb {
    geometry: FillRect;
    geo-fill-color: #FFA59999;
}
)___" };
constexpr inline std::wstring_view StdDark{ LR"___(
Button {
    geometry: FillFrameRect;
    geo-frame-color: #33FFFFFF;
    geo-fill-color: #72000000;
}
Button:hover {
    geometry: FillFrameRect;
    geo-frame-color: #19FFFFFF;
    geo-fill-color: #19FFFFFF;
}
Button:active {
    geometry: FillFrameRect;
    geo-frame-color: #33FFFFFF;
    geo-fill-color: #33FFFFFF;
}

CircleButton {
    geometry: FillEllipse;
    geo-fill-color: #B2000000;
}
CircleButton:hover {
    geometry: FillEllipse;
    geo-fill-color: #19FFFFFF;
}
CircleButton:active {
    geometry: FillEllipse;
    geo-fill-color: #33FFFFFF;
}

Edit {
    geometry: FillFrameRect;
    geo-frame-color: #33FFFFFF;
    geo-fill-color: #CC262626;
}
Edit:hover {
    geometry: FillFrameRect;
    geo-frame-color: #33FFFFFF;
    geo-fill-color: #CC303030;
}
Edit:focus {
    geometry: FillFrameRect;
    geo-frame-color: #33FFFFFF;
    geo-fill-color: #E5000000;
}

EditBottomBar {
    geometry: FillRect;
    geo-fill-color: #CC999999;
}
EditBottomBar:focus {
    geometry: FillRect;
    geo-fill-color: colorization;
}

ListSelRect {
    geometry: FillFrameRect;
    geo-frame-color: #CC007DEA;
    geo-fill-color: #66004887;
}
ListItem:hover {
    geometry: FillRect;
    geo-fill-color: #19FFFFFF;
}
ListItem:active {
    geometry: FillRect;
    geo-fill-color: #33FFFFFF;
}
ListItem:hover:active {
    geometry: FillRect;
    geo-fill-color: #3FFFFFFF;
}

HeaderItem {
    geometry: FillRect;
    geo-fill-color: #72000000;
}
HeaderItem:hover {
    geometry: FillRect;
    geo-fill-color: #8CFFFFFF;
}
HeaderItem:active {
    geometry: FillRect;
    geo-fill-color: #99FFFFFF;
}

TrackBar {
    geometry: FillRoundRect;
    geo-fill-color: #FF3F3F3F;
}
TrackBar:active {
    geometry: FillRoundRect;
    geo-fill-color: colorization;
}
TrackBarThumb {
    geometry: FrameRing;
    geo-ring-radius-outter: 10 10;
    geo-ring-radius-inner: 6 6;
    geo-frame-color: #FF595959;
    geo-fill-color: #FF4C4C4C;
    geo-extra-color: colorization;
}
TrackBarThumb:hover {
    geometry: FrameRing;
    geo-ring-radius-outter: 10 10;
    geo-ring-radius-inner: 7.5 7.5;
    geo-frame-color: #FF595959;
    geo-fill-color: #FF4C4C4C;
    geo-extra-color: colorization;
}
TrackBarThumb:active {
    geometry: FrameRing;
    geo-ring-radius-outter: 10 10;
    geo-ring-radius-inner: 8 8;
    geo-frame-color: #FF595959;
    geo-fill-color: #FF4C4C4C;
    geo-extra-color: colorization;
}

ScrollBar:hover {
    geometry: FillRect;
    geo-fill-color: #7F4C3F3F;
}
ScrollThumb {
    geometry: FillRect;
    geo-fill-color: #FF7F7272;
}
)___" };

// 一般来说，标准主题作为其他一切主题的（直接或间接）上级主题，
// 因为提供了系统颜色和度量的实现
namespace Priv
{
    class CStdTheme : public CCssTheme
    {
    protected:
        constexpr static D2D1_COLOR_F SpCrLightTrackBarThumb{ 1.f,1.f,1.f,1.f };
        constexpr static D2D1_COLOR_F SpCrLightTrackBarThumbFrame{ 0.9f,0.9f,0.9f,1.f };
        constexpr static D2D1_COLOR_F SpCrDarkTrackBarThumb{ 0.3f,0.3f,0.3f,1.f };
        constexpr static D2D1_COLOR_F SpCrDarkTrackBarThumbFrame{ 0.35f,0.35f,0.35f,1.f };

        void SpDrawTrackBarThumb(const D2D1_RECT_F& rc, const DTB_OPT* pOpt, BOOL bDark) const
        {
            if (!pOpt)
                pOpt = &DtbOptDefault;
            if (pOpt->uFlags & DTBO_NEW_OPACITY)
            {
                auto cr = bDark ? SpCrDarkTrackBarThumb : SpCrLightTrackBarThumb;
                cr.a *= pOpt->fOpacity;
                m_pBrSolid->SetColor(cr);
            }
            else
                m_pBrSolid->SetColor(bDark ?
                    SpCrDarkTrackBarThumb : SpCrLightTrackBarThumb);
            // 内外比 8:10
            const auto rOutter = (rc.right - rc.left) / 2.f;
            const auto rInner = rOutter * 0.6f;
            D2D1_ELLIPSE Ell
            {
                .point = { (rc.left + rc.right) / 2, (rc.top + rc.bottom) / 2 },
                .radiusX = rOutter,
                .radiusY = rOutter
            };
            m_pDC->FillEllipse(Ell, m_pBrSolid.Get());

            if (pOpt->uFlags & DTBO_NEW_OPACITY)
            {
                auto cr = bDark ? SpCrDarkTrackBarThumbFrame : SpCrLightTrackBarThumbFrame;
                cr.a *= pOpt->fOpacity;
                m_pBrSolid->SetColor(cr);
            }
            else
                m_pBrSolid->SetColor(bDark ?
                    SpCrDarkTrackBarThumbFrame : SpCrLightTrackBarThumbFrame);
            m_pDC->DrawEllipse(Ell, m_pBrSolid.Get());

            Ell.radiusX = Ell.radiusY = rInner;
            m_pBrSolid->SetColor(m_crColorization);
            m_pDC->FillEllipse(Ell, m_pBrSolid.Get());
        }
    public:
        float GetMetrics(Metrics eMetrics) noexcept override { return StdMetrics[(size_t)eMetrics]; }
    };
    class CStdThemeLight : public CStdTheme
    {
    public:
        HRESULT GetSysColor(SysColor eSysColor, _Out_ D2D1_COLOR_F& cr) noexcept override
        {
            cr = SysColorLight[(size_t)eSysColor];
            return S_OK;
        }
        HRESULT DrawBackground(Part ePart, State eState,
            const D2D1_RECT_F& rc, _In_opt_ const DTB_OPT* pOpt) noexcept override
        {
            if (ePart == Part::TrackBarThumb)
            {
                SpDrawTrackBarThumb(rc, pOpt, FALSE);
                return S_OK;
            }
            return __super::DrawBackground(ePart, eState, rc, pOpt);
        }
    };
    class CStdThemeDark : public CStdTheme
    {
    public:
        HRESULT GetSysColor(SysColor eSysColor, _Out_ D2D1_COLOR_F& cr) noexcept override
        {
            cr = SysColorDark[(size_t)eSysColor];
            return S_OK;
        }
        HRESULT DrawBackground(Part ePart, State eState,
            const D2D1_RECT_F& rc, _In_opt_ const DTB_OPT* pOpt) noexcept override
        {
            if (ePart == Part::TrackBarThumb)
            {
                SpDrawTrackBarThumb(rc, pOpt, TRUE);
                return S_OK;
            }
            return __super::DrawBackground(ePart, eState, rc, pOpt);
        }
    };
}

inline CssResult StMakeTheme(ITheme*& pTheme, BOOL bDark)
{
    CSS_DOC Css;
    const auto r = CssParse(bDark ? StdDark : StdLight, Css);
    if (r != CssResult::Ok)
        return r;
    const auto p = (bDark ? static_cast<CCssTheme*>(new Priv::CStdThemeDark{}) :
        static_cast<CCssTheme*>(new Priv::CStdThemeLight{}));
    p->FillCss(Css, {});
    pTheme = p;
    return CssResult::Ok;
}
ECK_DUI_NAMESPACE_END
ECK_NAMESPACE_END