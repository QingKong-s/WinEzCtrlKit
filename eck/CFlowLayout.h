#pragma once
#include "CLayoutBase.h"
#include "CTrivialBuffer.h"

ECK_NAMESPACE_BEGIN
// 
// 流式布局
// 在非布局流动方向上支持单行对齐
// 支持FIX和IDEAL，其余选项均被视为FLEX（特别注意不支持SCALE）
//

enum : UINT
{
    // 在前方插入换行
    FLF_BREAKLINE = 1u << 31,
};

// 不得直接实例化此类
class __declspec(novtable) CFlowLayoutBase : public CLayoutBase
{
public:
    ECK_RTTI(CFlowLayoutBase, CLayoutBase);
protected:
    struct ITEM : ITEMBASE
    {
        TLytCoord xy;
    };

    CTrivialBuffer<ITEM> m_vItem{};
    TLytCoord m_dTemp{};

    static void UpdateObjectIdealSize(ITEM& e) noexcept
    {
        LYTSIZE sizeIdeal{};
        const auto bHasIdealSize =
            (e.uFlags & LF_IDEAL) ?
            e.pObject->LoGetIdealSize(sizeIdeal) :
            FALSE;
        if (bHasIdealSize)
        {
            if (e.uFlags & LF_IDEAL_WIDTH)
                e.cx = sizeIdeal.cx + e.cxExtra;
            if (e.uFlags & LF_IDEAL_HEIGHT)
                e.cy = sizeIdeal.cy + e.cyExtra;
        }
    }
public:
    void LoShow(BOOL bShow) noexcept override
    {
        for (const auto& e : m_vItem)
            e.pObject->LoShow(bShow);
    }

    void LoInitializeDpi(int iDpi) noexcept override
    {
        m_iDpi = iDpi;
        for (auto& e : m_vItem)
            e.pObject->LoInitializeDpi(iDpi);
    }

    void LoOnDpiChanged(int iDpi) noexcept override
    {
        for (auto& e : m_vItem)
        {
            ReCalculateDpiSize(e, iDpi);
            e.pObject->LoOnDpiChanged(iDpi);
        }
        m_iDpi = iDpi;
    }

    void LobClear() noexcept override
    {
        __super::LobClear();
        m_vItem.Clear();
    }

    size_t LobGetObjectCount() const noexcept override { return m_vItem.Size(); }

    EckInlineNdCe auto& GetList() noexcept { return m_vItem; }
    EckInlineNdCe auto& GetList() const noexcept { return m_vItem; }
};

class CFlowLayoutV : public CFlowLayoutBase
{
public:
    ECK_RTTI(CFlowLayoutV, CFlowLayoutBase);
private:
    void ArrangeObject(ITEM& e, TLytCoord xLine, TLytCoord cxLineMax) noexcept
    {
        if (!(e.uFlags & (LF_FIX_HEIGHT | LF_IDEAL_HEIGHT)))
            e.cx = cxLineMax - e.Margins.l - e.Margins.r;
        const auto x = ArgCalculateHorizontalAlignment(
            xLine, cxLineMax, e, e.cx);
        ArgMoveObject(e, { x, e.xy, e.cx, e.cy });
    }

    void OnAddObject(ITEM& e) noexcept
    {
        if (e.uFlags & LF_FIX)
        {
            const auto size = e.pObject->LoGetSize();
            if (e.uFlags & LF_FIX_WIDTH)
                e.cx = size.cx;
            if (e.uFlags & LF_FIX_HEIGHT)
                e.cy = size.cy;
        }
        UpdateObjectIdealSize(e);
        // 更新尺寸和理想尺寸
        m_dTemp += (e.cy + e.Margins.t + e.Margins.b);
        m_cyIdeal = m_cy = std::max(m_cy, m_dTemp);
        m_cxIdeal = m_cx = std::max(m_cx, e.cx + e.Margins.l + e.Margins.r);
    }
public:
    void LoCommit() noexcept override
    {
        TLytCoord x = m_x;
        TLytCoord y = m_y;
        TLytCoord cxLineMax{};
        size_t idxInLine{};
        EckCounter(m_vItem.Size(), i)
        {
            auto& e = m_vItem[i];
            const auto yt = y + e.Margins.t + e.Margins.b + e.cy;
            if (!(e.uFlags & FLF_BREAKLINE) &&
                (yt <= m_y + m_cy || idxInLine == 0/*不管有没有空间，至少要放下一个控件*/))
            {
                const auto cxReal = e.cx + e.Margins.l + e.Margins.r;
                if (cxReal > cxLineMax)
                    cxLineMax = cxReal;
                e.xy = y + e.Margins.t;
                ++idxInLine;
                y = yt;
            }
            else// 开始新行
            {
                // 归位上一行
                for (size_t j = i - idxInLine; j < i; ++j)
                    ArrangeObject(m_vItem[j], x, cxLineMax);
                // 重置行参数
                y = m_y;
                x += cxLineMax;
                cxLineMax = 0;
                idxInLine = 0;
                // 本行第一个控件
                const auto cxReal = e.cx + e.Margins.l + e.Margins.r;
                if (cxReal > cxLineMax)
                    cxLineMax = cxReal;
                e.xy = y + e.Margins.t;
                ++idxInLine;
                y += (e.Margins.t + e.Margins.b + e.cy);
            }
        }
        for (size_t j = m_vItem.Size() - idxInLine; j < m_vItem.Size(); ++j)
            ArrangeObject(m_vItem[j], x, cxLineMax);
    }

    void LobRefresh() noexcept override
    {
        m_cxIdeal = m_cyIdeal = m_dTemp = 0;
        for (auto& e : m_vItem)
        {
            UpdateObjectIdealSize(e);
            m_dTemp += (e.cy + e.Margins.t + e.Margins.b);
            m_cyIdeal = std::max(m_cyIdeal, m_dTemp);
            m_cxIdeal = std::max(m_cxIdeal, e.cx + e.Margins.l + e.Margins.r);
        }
    }

    void LobAddObject(const LOB_PARAM& Param) noexcept override
    {
        auto& e = m_vItem.PushBack();
        AssignItem(e, Param);
        OnAddObject(e);
    }
};

class CFlowLayoutH : public CFlowLayoutBase
{
public:
    ECK_RTTI(CFlowLayoutH, CFlowLayoutBase);
private:
    void ArrangeObject(ITEM& e, TLytCoord yLine, TLytCoord cyLineMax) noexcept
    {
        if (!(e.uFlags & (LF_FIX_HEIGHT | LF_IDEAL_HEIGHT)))
            e.cy = cyLineMax - e.Margins.t - e.Margins.b;
        const auto y = ArgCalculateVerticalAlignment(
            yLine, cyLineMax, e, e.cy);
        ArgMoveObject(e, { e.xy, y, e.cx, e.cy });
    }

    void OnAddObject(ITEM& e) noexcept
    {
        if (e.uFlags & LF_FIX)
        {
            const auto size = e.pObject->LoGetSize();
            if (e.uFlags & LF_FIX_WIDTH)
                e.cx = size.cx;
            if (e.uFlags & LF_FIX_HEIGHT)
                e.cy = size.cy;
        }
        UpdateObjectIdealSize(e);
        // 更新尺寸和理想尺寸
        m_dTemp += (e.cx + e.Margins.l + e.Margins.r);
        m_cxIdeal = m_cx = std::max(m_cx, m_dTemp);
        m_cyIdeal = m_cy = std::max(m_cy, e.cy + e.Margins.t + e.Margins.b);
    }
public:
    void LoCommit() noexcept override
    {
        TLytCoord x = m_x;
        TLytCoord y = m_y;
        TLytCoord cyLineMax{};
        size_t idxInLine{};
        EckCounter(m_vItem.Size(), i)
        {
            auto& e = m_vItem[i];
            const auto xt = x + e.Margins.l + e.Margins.r + e.cx;
            if (!(e.uFlags & FLF_BREAKLINE) &&
                (xt <= m_x + m_cx || idxInLine == 0/*不管有没有空间，至少要放下一个控件*/))
            {
                const auto cyReal = e.cy + e.Margins.t + e.Margins.b;
                if (cyReal > cyLineMax)
                    cyLineMax = cyReal;
                e.xy = x + e.Margins.l;
                ++idxInLine;
                x = xt;
            }
            else// 开始新行
            {
                // 归位上一行
                for (size_t j = i - idxInLine; j < i; ++j)
                    ArrangeObject(m_vItem[j], y, cyLineMax);
                // 重置行参数
                x = m_x;
                y += cyLineMax;
                cyLineMax = 0;
                idxInLine = 0;
                // 本行第一个控件
                const auto cyReal = e.cy + e.Margins.t + e.Margins.b;
                if (cyReal > cyLineMax)
                    cyLineMax = cyReal;
                e.xy = x + e.Margins.l;
                ++idxInLine;
                x += (e.Margins.l + e.Margins.r + e.cx);
            }
        }
        for (size_t j = m_vItem.Size() - idxInLine; j < m_vItem.Size(); ++j)
            ArrangeObject(m_vItem[j], y, cyLineMax);
    }

    void LobRefresh() noexcept override
    {
        m_cxIdeal = m_cyIdeal = m_dTemp = 0;
        for (auto& e : m_vItem)
        {
            UpdateObjectIdealSize(e);
            m_dTemp += (e.cx + e.Margins.l + e.Margins.r);
            m_cxIdeal = std::max(m_cxIdeal, m_dTemp);
            m_cyIdeal = std::max(m_cyIdeal, e.cy + e.Margins.t + e.Margins.b);
        }
    }

    void LobAddObject(const LOB_PARAM& Param) noexcept override
    {
        auto& e = m_vItem.PushBack();
        AssignItem(e, Param);
        OnAddObject(e);
    }
};
ECK_NAMESPACE_END