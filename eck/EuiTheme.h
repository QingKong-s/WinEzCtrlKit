#pragma once
#include "EuiDef.h"
#include "UiTheme.h"
#include "GraphicsHelper.h"

ECK_NAMESPACE_BEGIN
ECK_EUI_NAMESPACE_BEGIN
using UiBasic::CColorCollection;
using CMetricCollection = UiBasic::CMetricCollection<int>;

enum class TmResult
{
    Ok,
    NoStyle,
    NotSupport,
    NoFont,
};

class CElement;
class CThemeBase : public CReferenceCounted
{
protected:
    RcPtr<CColorCollection> m_pColor{};
    RcPtr<CMetricCollection> m_pMetric{};
public:
    virtual TmResult Draw(CElement* pEle, UINT idPart, const RECT& rc) noexcept = 0;

    void SetColorCollection(CColorCollection* p) noexcept { m_pColor = p; }
    auto GetColorCollection() const noexcept { return m_pColor; }

    void SetMetricCollection(CMetricCollection* p) noexcept { m_pMetric = p; }
    auto GetMetricCollection() const noexcept { return m_pMetric; }
};

class CThemeStyle : public CReferenceCounted
{
public:
    struct Style
    {
        UINT uState{};

        UINT argbFore{};
        UINT argbBack{};
        UINT argbBorder{};

        USHORT rRound{};
        USHORT dLeft{};
        USHORT dTop{};
        USHORT dRight{};
        USHORT dBottom{};

        Align eAlignV{};
        Align eAlignH{};
        ImageMode eImgModeFore{};
        ImageMode eImgModeBack{};

        RcPtr<CImage> pImgFore{};
        RcPtr<CImage> pImgBack{};
        RcPtr<CFont> pFont{};
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

    void SetBorderWidth(int d) noexcept
    {
        for (auto& e : m_vStyle)
            e.dLeft = e.dTop = e.dRight = e.dBottom = (USHORT)d;
    }

    void SetFont(CFont* p) noexcept
    {
        for (auto& e : m_vStyle)
            e.pFont = p;
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
ECK_EUI_NAMESPACE_END
ECK_NAMESPACE_END