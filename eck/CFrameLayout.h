#pragma once
#include "CLayoutBase.h"
#include "CTrivialBuffer.h"

ECK_NAMESPACE_BEGIN
// 
// 帧布局
// 支持九向对齐
// 支持FIX、IDEAL和SCALE，其余均被视为FLEX
//

class CFrameLayout : public CLayoutBase
{
public:
    ECK_RTTI(CFrameLayout, CLayoutBase);
private:
    struct ITEM : ITEMBASE {};

    CTrivialBuffer<ITEM> m_vItem{};

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
        m_cxIdeal = m_cx = std::max(m_cx, e.cx + e.Margin.l + e.Margin.r);
        m_cyIdeal = m_cy = std::max(m_cy, e.cy + e.Margin.t + e.Margin.b);
    }
public:
    void LoShow(BOOL bShow) noexcept override
    {
        for (const auto& e : m_vItem)
            e.pObject->LoShow(bShow);
    }

    void LoCommit() noexcept override
    {
        LYTRECT rc;
        for (const auto& e : m_vItem)
        {
            ArgCalculateRect(e, { m_x, m_y, m_cx, m_cy }, rc);
            ArgMoveObject(e, rc);
        }
    }

    void LoInitializeDpi(int iDpi) noexcept override
    {
        m_iDpi = iDpi;
        for (const auto& e : m_vItem)
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
        LobRefresh();
    }

    void LobRefresh() noexcept override
    {
        m_cx = m_cy = m_cxIdeal = m_cyIdeal = 0;
        for (auto& e : m_vItem)
            OnAddObject(e);
    }

    void LobClear() noexcept override
    {
        __super::LobClear();
        m_vItem.Clear();
    }

    size_t LobGetObjectCount() const noexcept override { return m_vItem.Size(); }

    void LobUpdateIdealSize() noexcept override
    {
        m_cxIdeal = m_cyIdeal = 0;
        for (auto& e : m_vItem)
        {
            UpdateObjectIdealSize(e);
            m_cxIdeal = std::max(m_cxIdeal, e.cx + e.Margin.l + e.Margin.r);
            m_cyIdeal = std::max(m_cyIdeal, e.cy + e.Margin.t + e.Margin.b);
        }
    }

    void LobAddObject(const LOB_PARAM& Param) noexcept override
    {
        auto& e = m_vItem.PushBack();
        AssignItem(e, Param);
        OnAddObject(e);
    }

    void ShowFrame(int idx) noexcept
    {
        EckAssert(idx >= 0 && idx < (int)m_vItem.Size());
        int i{};
        m_vItem[idx].pObject->LoShow(TRUE);
        for (; i < idx; ++i)
            m_vItem[i].pObject->LoShow(FALSE);
        for (i = idx + 1; i < (int)m_vItem.Size(); ++i)
            m_vItem[i].pObject->LoShow(FALSE);
    }

    EckInlineNdCe auto& GetList() noexcept { return m_vItem; }
    EckInlineNdCe auto& GetList() const noexcept { return m_vItem; }
};
ECK_NAMESPACE_END