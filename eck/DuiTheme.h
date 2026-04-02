#pragma once
#include "DuiDef.h"
#include "UiTheme.h"

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
};

class CElement;

// 简单四态颜色支持

enum : UINT
{
    SsNormal,
    SsHot,
    SsPressed,
    SsDisabled,

    SsMax
};

struct SimpleStyle
{
    UINT idCrFore;
    UINT idCrBack;
    UINT idCrBorder;

    float rRound;
    float cxBorder;

    EckInlineNdCe BOOL HasRoundConer() const noexcept { return rRound >= 1.f; }
    EckInlineNdCe BOOL HasBorder() const noexcept
    {
        return cxBorder >= 1.f && idCrBorder != IdTmInvalid;
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

    void SetColorCollection(CColorCollection* p) noexcept { m_pColor = p; }
    auto& GetColorCollection() const noexcept { return m_pColor; }

    void SetMetricCollection(CMetricCollection* p) noexcept { m_pMetric = p; }
    auto& GetMetricCollection() const noexcept { return m_pMetric; }

    std::optional<UINT> GetColorOptional(UINT id) const noexcept
    {
        if (!m_pColor)
            return std::nullopt;
        return m_pColor->Get(id);
    }
    UINT GetColor(UINT id, UINT argbDef = 0) const noexcept
    {
        const auto v = GetColorOptional(id);
        return v.value_or(argbDef);
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
EckInlineNdCe UINT TmSimpleStyleFromThemeState(UINT sa) noexcept
{
    if (sa & SaDisable)
        return SsDisabled;
    if (sa & SaPressed)
        return SsPressed;
    if (sa & SaHot)
        return SsHot;
    return SsNormal;
}
ECK_DUI_NAMESPACE_END
ECK_NAMESPACE_END