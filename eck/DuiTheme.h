#pragma once
#include "UiTheme.h"
#include "Color.h"

#include <any>

ECK_NAMESPACE_BEGIN
ECK_DUI_NAMESPACE_BEGIN
using UiBasic::CColorCollection;
using CMetricCollection = UiBasic::CMetricCollection<float>;

enum class TmResult
{
    Ok,
    NoStyle,
    NotSupport,
    NoFont,
    InvalidType,
};

class CElement;

enum : UINT
{
    SfFore,
    SfBack,
    SfBorder,
};

struct SimpleStyle
{
    union
    {
        struct
        {
            UINT CrFore;
            UINT CrBack;
            UINT CrBorder;
        };
        UINT Cr[3];
    };

    float rRound;
    float cxBorder;
    union
    {
        struct
        {
            BOOLEAN bForeArgb;
            BOOLEAN bBackArgb;
            BOOLEAN bBorderArgb;
        };
        UINT bArgb[3];
    };

    EckInlineNdCe BOOL HasRoundConer() const noexcept { return rRound >= 1.f; }
    EckInlineNdCe BOOL HasBorder() const noexcept
    {
        return cxBorder >= 1.f && CrBorder != IdTmInvalid;
    }
};

class CThemeBase : public CReferenceCounted
{
protected:
    RcPtr<CColorCollection> m_pColor{};
    RcPtr<CMetricCollection> m_pMetric{};
public:
    virtual TmResult Draw(
        CElement* pEle,
        const SimpleStyle* pStyle,
        UINT idPart,
        const D2D1_RECT_F& rc,
        _In_opt_ const D2D1_RECT_F* prcClip = nullptr) noexcept = 0;

    virtual TmResult SetExtra(UINT idExtra, const std::any& Val) noexcept
    {
        return TmResult::NotSupport;
    }
    virtual std::optional<std::any> GetExtra(UINT idExtra) const noexcept
    {
        return std::nullopt;
    }

    void SetColorCollection(CColorCollection* p) noexcept { m_pColor = p; }
    auto& GetColorCollection() const noexcept { return m_pColor; }

    void SetMetricCollection(CMetricCollection* p) noexcept { m_pMetric = p; }
    auto& GetMetricCollection() const noexcept { return m_pMetric; }

    std::optional<UINT> GetColorOptional(UINT id) const noexcept
    {
        if (!m_pColor || id == IdTmInvalid)
            return std::nullopt;
        return m_pColor->Get(id);
    }
    UINT GetColor(UINT id, UINT argbDef = 0) const noexcept
    {
        if (id == IdTmInvalid)
            return argbDef;
        const auto v = GetColorOptional(id);
        return v.value_or(argbDef);
    }
    UINT GetStyleColor(const SimpleStyle* pStyle, UINT sf, UINT argbDef = 0) const noexcept
    {
        const auto u = pStyle->Cr[sf];
        const auto bArgb = pStyle->bArgb[sf];
        if (bArgb)
            return u;
        else
            return GetColor(u, argbDef);
    }

    std::optional<float> GetMetricOptional(UINT id) const noexcept
    {
        if (!m_pMetric)
            return std::nullopt;
        return m_pMetric->Get(id);
    }
    float GetMetric(UINT id, float dDef = 0.f) const noexcept
    {
        const auto v = GetMetricOptional(id);
        return v.value_or(dDef);
    }
};

inline std::optional<ARGB> TmSsLerpColor(CThemeBase* pTheme,
    BOOL bArgbOrId, UINT cr1, UINT cr2, float fLerp) noexcept
{
    std::optional<ARGB> oc1, oc2;
    oc1 = bArgbOrId ? cr1 : pTheme->GetColorOptional(cr1);
    oc2 = bArgbOrId ? cr2 : pTheme->GetColorOptional(cr2);
    if (!oc1 || !oc2)
        return std::nullopt;
    if (!oc1)
        oc1 = 0;
    if (!oc2)
        oc2 = 0;
    return LerpArgb(*oc1, *oc2, fLerp);
}

inline SimpleStyle TmSsLerp(
    CThemeBase* pTheme,
    const SimpleStyle& s1,
    const SimpleStyle& s2,
    float fLerp) noexcept
{
    SimpleStyle s;
    s.rRound = s1.rRound + (s2.rRound - s1.rRound) * fLerp;
    s.cxBorder = s1.cxBorder + (s2.cxBorder - s1.cxBorder) * fLerp;

    const auto crFore = TmSsLerpColor(pTheme, s1.bForeArgb, s1.CrFore, s2.CrFore, fLerp);
    if (crFore)
    {
        s.CrFore = *crFore;
        s.bForeArgb = TRUE;
    }
    else
        s.CrFore = IdTmInvalid;
    const auto crBack = TmSsLerpColor(pTheme, s1.bBackArgb, s1.CrBack, s2.CrBack, fLerp);
    if (crBack)
    {
        s.CrBack = *crBack;
        s.bBackArgb = TRUE;
    }
    else
        s.CrBack = IdTmInvalid;
    const auto crBorder = TmSsLerpColor(pTheme, s1.bBorderArgb, s1.CrBorder, s2.CrBorder, fLerp);
    if (crBorder)
    {
        s.CrBorder = *crBorder;
        s.bBorderArgb = TRUE;
    }
    else
        s.CrBorder = IdTmInvalid;
    return s;
}

enum : UINT
{
    IdTmDummy = IdTmSystemBegin,

    IdTmEnd,
};

EckInlineNd UINT TmNextResourceId() noexcept
{
    static std::atomic<UINT> Id{ IdTmEnd };
    return Id.fetch_add(1, std::memory_order_relaxed);
}

inline RcPtr<CMetricCollection> TmsMakeMetricCollection() noexcept
{
    const auto p = RcPtr<CMetricCollection>::Make();
    p->Set(IdMeScrollBarWidth, 16.f);
    p->Set(IdMeScrollThumbWidth, 12.f);
    p->Set(IdMePaddingOuter, 6.f);
    p->Set(IdMePaddingInner, 4.f);
    p->Set(IdMeFocusPadding, 2.f);
    p->Set(IdMeMinimumScrollThumb, 15.f);
    return p;
}
inline RcPtr<CMetricCollection> TmsMetricCollection() noexcept
{
    static auto p{ TmsMakeMetricCollection() };
    return p;
}

inline RcPtr<CColorCollection> TmsMakeColorCollectionLight() noexcept
{
    const auto p = RcPtr<CColorCollection>::Make();
    p->Set(IdCrFore, CrlFore);
    p->Set(IdCrForeDisabled, CrlBack);

    p->Set(IdCrBorder, CrlBorder);
    p->Set(IdCrBorderHot, CrlBorderHot);
    p->Set(IdCrBorderPressed, CrlBorderPressed);
    p->Set(IdCrBorderDisabled, CrlBorderDisabled);

    p->Set(IdCrBack, CrlBack);
    p->Set(IdCrBackHot, CrlBackHot);
    p->Set(IdCrBackPressed, CrlBackPressed);
    p->Set(IdCrBackDisabled, CrlBackDisabled);

    p->Set(IdCrAccent, CrlAccent);
    p->Set(IdCrAccentHot, CrlAccentHot);
    p->Set(IdCrAccentPressed, CrlAccentPressed);
    p->Set(IdCrAccentDisabled, CrlAccentDisabled);
    p->Set(IdCrAccentFore, CrlAccentFore);
    p->Set(IdCrAccentForeDisabled, CrlAccentForeDisabled);

    p->Set(IdCrDanger, CrlDanger);
    p->Set(IdCrDangerHot, CrlDangerHot);
    p->Set(IdCrDangerPressed, CrlDangerPressed);
    return p;
}
inline RcPtr<CColorCollection> TmsColorCollectionLight() noexcept
{
    static auto p{ TmsMakeColorCollectionLight() };
    return p;
}
inline RcPtr<CColorCollection> TmsMakeColorCollectionDark() noexcept
{
    const auto p = RcPtr<CColorCollection>::Make();
    p->Set(IdCrFore, CrdFore);
    p->Set(IdCrForeDisabled, CrdBack);

    p->Set(IdCrBorder, CrdBorder);
    p->Set(IdCrBorderHot, CrdBorderHot);
    p->Set(IdCrBorderPressed, CrdBorderPressed);
    p->Set(IdCrBorderDisabled, CrdBorderDisabled);

    p->Set(IdCrBack, CrdBack);
    p->Set(IdCrBackHot, CrdBackHot);
    p->Set(IdCrBackPressed, CrdBackPressed);
    p->Set(IdCrBackDisabled, CrdBackDisabled);

    p->Set(IdCrAccent, CrdAccent);
    p->Set(IdCrAccentHot, CrdAccentHot);
    p->Set(IdCrAccentPressed, CrdAccentPressed);
    p->Set(IdCrAccentDisabled, CrdAccentDisabled);
    p->Set(IdCrAccentFore, CrdAccentFore);
    p->Set(IdCrAccentForeDisabled, CrdAccentForeDisabled);

    p->Set(IdCrDanger, CrdDanger);
    p->Set(IdCrDangerHot, CrdDangerHot);
    p->Set(IdCrDangerPressed, CrdDangerPressed);
    return p;
}
inline RcPtr<CColorCollection> TmsColorCollectionDark() noexcept
{
    static auto p{ TmsMakeColorCollectionDark() };
    return p;
}

EckInlineNdCe SimpleStyle TmsSsMakeNormal(float cxBorder = 0.f, float r = 0.f) noexcept
{
    return { IdCrFore, IdCrBack, IdCrBorder, r, cxBorder };
}
EckInlineNdCe SimpleStyle TmsSsMakeHot(float cxBorder = 0.f, float r = 0.f) noexcept
{
    return { IdCrFore, IdCrBackHot, IdCrBorderHot, r, cxBorder };
}
EckInlineNdCe SimpleStyle TmsSsMakePressed(float cxBorder = 0.f, float r = 0.f) noexcept
{
    return { IdCrFore, IdCrBackPressed, IdCrBorderPressed, r, cxBorder };
}
EckInlineNdCe SimpleStyle TmsSsMakeDisabled(float cxBorder = 0.f, float r = 0.f) noexcept
{
    return { IdCrFore, IdCrBackDisabled, IdCrBorderDisabled, r, cxBorder };
}
ECK_DUI_NAMESPACE_END
ECK_NAMESPACE_END