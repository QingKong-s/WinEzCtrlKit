#include "CReferenceCounted.h"

ECK_NAMESPACE_BEGIN
ECK_UIBASIC_NAMESPACE_BEGIN
namespace Declaration
{
    enum : UINT
    {
        IdTmInvalid,

        // Color

        IdCrFore,
        IdCrForeDisabled,

        IdCrBorder,
        IdCrBorderHot,
        IdCrBorderPressed,
        IdCrBorderDisabled,

        IdCrBack,
        IdCrBackHot,
        IdCrBackPressed,
        IdCrBackDisabled,

        IdCrAccent,
        IdCrAccentHot,
        IdCrAccentPressed,
        IdCrAccentDisabled,
        IdCrAccentFore,
        IdCrAccentForeDisabled,

        IdCrDanger,
        IdCrDangerHot,
        IdCrDangerPressed,

        // Metric

        IdMeScrollBarWidth,
        IdMeScrollThumbWidth,
        IdMePaddingOuter,
        IdMePaddingInner,
        IdMeFocusPadding,
        IdMeMinimumScrollThumb,

        // Part

        IdPtNormal,

        IdTmSystemBegin,
    };
}

class CColorCollection final : public CReferenceCountedT<CColorCollection>
{
private:
    struct ITEM
    {
        UINT id;
        UINT argb;
    };
    std::vector<ITEM> m_vItem{};

    EckInlineNdCe auto LowerBound(UINT id) noexcept
    {
        return std::lower_bound(m_vItem.begin(), m_vItem.end(), id,
            [](const ITEM& e, UINT id) { return e.id < id; });
    }
    EckInlineNdCe auto LowerBound(UINT id) const noexcept
    {
        return std::lower_bound(m_vItem.begin(), m_vItem.end(), id,
            [](const ITEM& e, UINT id) { return e.id < id; });
    }
public:
    void Set(UINT id, UINT argb) noexcept
    {
        const auto it = LowerBound(id);
        if (it != m_vItem.end() && it->id == id)
            it->argb = argb;
        else
            m_vItem.emplace(it, id, argb);
    }
    std::optional<UINT> Get(UINT id) noexcept
    {
        const auto it = LowerBound(id);
        if (it != m_vItem.end() && it->id == id)
            return { it->argb };
        else
            return std::nullopt;
    }
};

template<class T>
class CMetricCollection final : public CReferenceCountedT<CMetricCollection<T>>
{
private:
    struct ITEM
    {
        UINT id;
        T d;
    };
    std::vector<ITEM> m_vItem{};

    EckInlineNdCe auto LowerBound(UINT id) noexcept
    {
        return std::lower_bound(m_vItem.begin(), m_vItem.end(), id,
            [](const ITEM& e, UINT id) {return e.id < id; });
    }
    EckInlineNdCe auto LowerBound(UINT id) const noexcept
    {
        return std::lower_bound(m_vItem.begin(), m_vItem.end(), id,
            [](const ITEM& e, UINT id) {return e.id < id; });
    }
public:
    void Set(UINT id, T d) noexcept
    {
        const auto it = LowerBound(id);
        if (it != m_vItem.end() && it->id == id)
            it->d = d;
        else
            m_vItem.emplace(it, id, d);
    }
    std::optional<T> Get(UINT id) noexcept
    {
        const auto it = LowerBound(id);
        if (it != m_vItem.end() && it->id == id)
            return { it->d };
        else
            return std::nullopt;
    }
};
ECK_UIBASIC_NAMESPACE_END
ECK_NAMESPACE_END