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

class CThemeStyle : public CReferenceCounted
{
public:
    struct Style
    {
        UINT uState{};

        UINT argbFore{};
        UINT argbBack{};
        UINT argbBorder{};

        float rRound{};
        float dLeft{};
        float dTop{};
        float dRight{};
        float dBottom{};
    };
private:
    std::vector<Style> m_vStyle{};
public:
    EckInlineNdCe auto& GetList() const noexcept { return m_vStyle; }
    EckInlineNdCe auto& GetList() noexcept { return m_vStyle; }

    const Style* FindStyle(UINT uState) const noexcept
    {
        for (auto& e : m_vStyle)
        {
            if ((e.uState & uState) == uState)
                return &e;
        }
        return nullptr;
    }

    void SetBorderWidth(float d) noexcept
    {
        for (auto& e : m_vStyle)
            e.dLeft = e.dTop = e.dRight = e.dBottom = d;
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
        UINT idPart,
        const D2D1_RECT_F& rc,
        _Out_opt_ const CThemeStyle::Style** ppStyle = nullptr) noexcept = 0;

    void SetColorCollection(CColorCollection* p) noexcept { m_pColor = p; }
    auto GetColorCollection() const noexcept { return m_pColor; }

    void SetMetricCollection(CMetricCollection* p) noexcept { m_pMetric = p; }
    auto GetMetricCollection() const noexcept { return m_pMetric; }

    UINT GetColor(UINT id) const noexcept
    {
        const auto v = m_pColor->Get(id);
        return v.value_or(0);
    }
    float GetMetric(UINT id) const noexcept
    {
        const auto v = m_pMetric->Get(id);
        return v.value_or(0);
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

inline RcPtr<CMetricCollection> TmMakeDefaultMetricCollection() noexcept
{
    const auto p = RcPtr<CMetricCollection>::Make();
    p->Set(IdMePaddingInner, 4.f);
    p->Set(IdMePaddingOuter, 6.f);
    return p;
}
inline RcPtr<CMetricCollection> TmDefaultMetricCollection() noexcept
{
    static auto p{ TmMakeDefaultMetricCollection() };
    return p;
}
ECK_DUI_NAMESPACE_END
ECK_NAMESPACE_END