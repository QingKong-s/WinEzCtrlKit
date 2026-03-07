#pragma once
#include "EuiDef.h"
#include "UiTheme.h"
#include "CSrwLock.h"
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

class CThemeStyleCollection : public CReferenceCounted
{
public:
    struct STYLE
    {
        UINT uState{};

        UINT argbFore{};
        UINT argbBack{};
        UINT argbBorder{};

        Align eAlignV{};
        Align eAlignH{};
        USHORT rRound{};
        USHORT dLeft{};
        USHORT dTop{};
        USHORT dRight{};
        USHORT dBottom{};

        BkImgMode eImgModeFore{};
        BkImgMode eImgModeBack{};

        RcPtr<CImage> pImgFore{};
        RcPtr<CImage> pImgBack{};
        RcPtr<CFont> pFont{};
    };
private:
    std::vector<STYLE> m_vStyle{};
public:
    EckInlineNdCe auto& GetList() const noexcept { return m_vStyle; }
    EckInlineNdCe auto& GetList() noexcept { return m_vStyle; }

    const STYLE* FindStyle(UINT uState) const noexcept
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
    IdTmBegin = 1,

    IdCrBackground,
    IdCrForeground,
    IdCrBorder,

    IdMeScrollBarWidth,
    IdMeScrollThumbWidth,
    IdMePadding,

    IdPtNormal,

    IdSaNormal,
    IdSaHot,
    IdSaActive,
    IdSaDisable,
    IdSaFocus,

    IdTmEnd,
};

enum : UINT
{
    SaNormal = 0u,
    SaHot = 1u << 0,
    SaActive = 1u << 1,
    SaDisable = 1u << 2,
    SaFocus = 1u << 3,
    SaSelected = 1u << 4,
    SaMixed = 1u << 5,

    SapLButtonDown = 1u << 31,
};

EckInlineNd UINT TmNextResourceId() noexcept
{
    static std::atomic<UINT> Id{ IdTmEnd };
    return ++Id;
}
ECK_EUI_NAMESPACE_END
ECK_NAMESPACE_END