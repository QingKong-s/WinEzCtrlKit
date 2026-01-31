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

    void OnAddObject(ITEM& e) noexcept
    {
        if (e.uFlags & LF_FIX)
        {
            const auto size = e.pObject->LoGetSize();
            e.cx = size.cx;
            e.cy = size.cy;
            if (e.uFlags & LF_FIX_WIDTH)
                OnAddFixedWidthObject(e);
            if (e.uFlags & LF_FIX_HEIGHT)
                OnAddFixedHeightObject(e);
        }
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
        for (auto& e : m_vItem)
            OnAddObject(e);
    }

    void LobClear() noexcept override
    {
        __super::LobClear();
        m_vItem.Clear();
    }

    size_t LobGetObjectCount() const noexcept override { return m_vItem.Size(); }

    size_t Add(ILayout* pCtrl, const LYTMARGINS& Margin = {}, UINT uFlags = 0u) noexcept
    {
        auto& e = m_vItem.PushBack();
        e.pObject = pCtrl;
        e.Margin = Margin;
        e.uFlags = uFlags;
        OnAddObject(e);
        return m_vItem.Size() - 1;
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