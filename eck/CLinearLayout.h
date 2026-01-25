#pragma once
#include "CLayoutBase.h"
#include "CTrivialBuffer.h"

ECK_NAMESPACE_BEGIN
// 
// 线性布局
// 在非布局流动方向上支持线对齐
// 支持所有其他布局选项
//

enum : UINT
{
    LLFP_IDEAL_SIZE_CACHE = 1u << 31,   // 理想尺寸有效
    LLFP_CUT = 1u << 30,                // 从迭代中排除
};

// 不得直接实例化此类
class __declspec(novtable) CLinearLayoutBase : public CLayoutBase
{
public:
    ECK_RTTI(CLinearLayoutBase);
protected:
    struct ITEM : ITEMBASE
    {
        UINT uWeight;
        TLytCoord cxIdeal;
        TLytCoord cyIdeal;
        TLytCoord dAlloc;
    };

    CTrivialBuffer<ITEM> m_vItem{};
    TLytCoord m_dFixedSum{};
    UINT m_uWeightSum : 31{};
    UINT m_bHasIdeal : 1{};
public:
    void LoShow(BOOL bShow) noexcept override
    {
        for (const auto& e : m_vItem)
            e.pObject->LoShow(bShow);
    }

    void LoInitializeDpi(int iDpi) noexcept override
    {
        m_iDpi = iDpi;
        for (const auto& e : m_vItem)
            e.pObject->LoInitializeDpi(iDpi);
    }

    void LoOnDpiChanged(int iDpi) noexcept override
    {
        LobRefresh();
        for (auto& e : m_vItem)
        {
            ReCalculateDpiSize(e, iDpi);
            e.pObject->LoOnDpiChanged(iDpi);
        }
        m_iDpi = iDpi;
    }

    // 清除布局内容
    void LobClear() noexcept override
    {
        __super::LobClear();
        m_vItem.Clear();
        m_dFixedSum = 0;
        m_uWeightSum = 0;
    }

    size_t LobGetObjectCount() const noexcept override { return m_vItem.Size(); }

    EckInlineNdCe auto& GetList() noexcept { return m_vItem; }
    EckInlineNdCe auto& GetList() const noexcept { return m_vItem; }
};
ECK_RTTI_IMPL_BASE_INLINE(CLinearLayoutBase, CLayoutBase);

class CLinearLayoutV : public CLinearLayoutBase
{
public:
    ECK_RTTI(CLinearLayoutV);
private:
    static void PreArrangeObject(ITEM& e, _Inout_ TLytCoord& dLeave) noexcept
    {
        LYTSIZE sizeIdeal{};
        const auto bHasIdealSize =
            (e.uFlags & (LF_FILL_HEIGHT | LF_IDEAL)) ?
            e.pObject->LoGetIdealSize(sizeIdeal) :
            FALSE;
        if (bHasIdealSize)
        {
            e.cxIdeal = sizeIdeal.cx;
            e.cyIdeal = sizeIdeal.cy;
            e.uFlags |= LLFP_IDEAL_SIZE_CACHE;
        }
        else
            e.uFlags &= ~LLFP_IDEAL_SIZE_CACHE;
        e.uFlags &= ~LLFP_CUT;

        if (e.uFlags & LF_FIX_HEIGHT)// 固定尺寸
        {
            e.dAlloc = e.cy;
            e.uFlags |= LLFP_CUT;
        }
        else if (e.uFlags & LF_IDEAL_HEIGHT)// 固定为理想尺寸
        {
            e.dAlloc = e.cyIdeal;
            e.uFlags |= LLFP_CUT;
            dLeave -= e.cyIdeal;
        }
    }

    void OnAddObject(ITEM& e) noexcept
    {
        if (e.uFlags & (LF_FIX | LF_SCALE))
        {
            const auto size = e.pObject->LoGetSize();
            e.cx = size.cx;
            e.cy = size.cy;
            if (e.uFlags & LF_FIX_HEIGHT)
                m_dFixedSum += size.cy;
        }
        m_dFixedSum += (e.Margin.t + e.Margin.b);
        if (!(e.uFlags & (LF_FIX_HEIGHT | LF_IDEAL_HEIGHT)))
            m_uWeightSum += e.uWeight;
        if (e.uFlags & LF_IDEAL_HEIGHT)
            m_bHasIdeal = TRUE;
    }
public:
    void LoCommit() noexcept override
    {
        //
        // 将理想尺寸从剩余空间中排除
        //

        TLytCoord dLeave = m_cy - m_dFixedSum;
        BOOL bPrepared{};
        if (m_bHasIdeal)
        {
            for (auto& e : m_vItem)
                PreArrangeObject(e, dLeave);
            bPrepared = TRUE;
        }
        //
        // 假设空间充足，为带权对象做有下限（即理想尺寸）的分配
        //

        UINT wSum = m_uWeightSum;
        UINT wShrinkableSum{};// 可收缩对象的权重和
        EckLoop()
        {
            UINT wNewSum{};
            BOOL bClamped{};
            for (auto& e : m_vItem)
            {
                if (!bPrepared)
                    PreArrangeObject(e, dLeave);
                if (e.uFlags & LLFP_CUT)
                    continue;

                const auto dTemp = dLeave * e.uWeight / wSum;
                if (IsBitSet(e.uFlags, LF_FILL_HEIGHT | LLFP_IDEAL_SIZE_CACHE) &&
                    dTemp < e.cyIdeal)// FILL，且有理想尺寸，且比例分配到的尺寸过小
                {
                    e.dAlloc = e.cyIdeal;
                    dLeave -= e.cyIdeal;
                    e.uFlags |= LLFP_CUT;
                    bClamped = TRUE;
                }
                else
                    wNewSum += e.uWeight;
            }

            if (bClamped)
            {
                bPrepared = TRUE;
                wSum = wNewSum;
            }
            else// 已收敛
            {
                wNewSum = 0;
                for (auto& e : m_vItem)
                {
                    if (!(e.uFlags & LLFP_CUT))
                        e.dAlloc = dLeave * e.uWeight / wSum;
                    if (!(e.uFlags & (LF_FIX_HEIGHT | LF_IDEAL_HEIGHT)))
                        wNewSum += e.uWeight;
                    if (e.uFlags & LF_FILL_HEIGHT)
                        wShrinkableSum += e.uWeight;
                }
                wSum = wNewSum;
                break;
            }
        }
        //
        // 若空间不足，收缩可收缩的对象，完成所有对象的排列
        //

        const auto bShrink = (dLeave < 0);
        TLytCoord x, y = m_y, cx, cy;
        for (auto& e : m_vItem)
        {
            if (e.uFlags & LF_FILL_HEIGHT)
            {
                if (bShrink)
                    e.dAlloc += (dLeave * e.uWeight / wShrinkableSum);
            }

            cy = e.dAlloc;
            if (cy < 0)
                cy = 0;

            if (e.uFlags & LF_FIX_WIDTH)
                cx = e.cx;
            else if (e.uFlags & LF_IDEAL_WIDTH)
                cx = e.cxIdeal;
            else if (e.uFlags & LF_SCALE)
                cx = std::min(cy * e.cx / e.cy, m_cx - e.Margin.l - e.Margin.r);
            else
                cx = m_cx - e.Margin.l - e.Margin.r;

            x = ArgCalculateHorizontalAlignment(m_x, m_cx, e, cx);
            ArgMoveObject(e, { x, y + e.Margin.t, cx, cy });
            y += (cy + e.Margin.t + e.Margin.b);
        }
    }

    void LobRefresh() noexcept override
    {
        m_dFixedSum = 0;
        m_uWeightSum = 0;
        m_bHasIdeal = FALSE;
        for (auto& e : m_vItem)
            OnAddObject(e);
    }

    size_t Add(ILayout* pObject, const LYTMARGINS& Margin = {},
        UINT uFlags = 0u, UINT uWeight = 0u) noexcept
    {
        EckAssert(LfValidateFlags(uFlags));
        auto& e = m_vItem.PushBack();
        e.pObject = pObject;
        e.Margin = Margin;
        e.uFlags = uFlags;
        e.uWeight = uWeight;
        OnAddObject(e);
        return m_vItem.Size() - 1;
    }
};
ECK_RTTI_IMPL_BASE_INLINE(CLinearLayoutV, CLinearLayoutBase);

class CLinearLayoutH : public CLinearLayoutBase
{
public:
    ECK_RTTI(CLinearLayoutH);
private:
    static void PreArrangeObject(ITEM& e, _Inout_ TLytCoord& dLeave) noexcept
    {
        LYTSIZE sizeIdeal{};
        const auto bHasIdealSize =
            (e.uFlags & (LF_FILL_WIDTH | LF_IDEAL)) ?
            e.pObject->LoGetIdealSize(sizeIdeal) :
            FALSE;
        if (bHasIdealSize)
        {
            e.cxIdeal = sizeIdeal.cx;
            e.cyIdeal = sizeIdeal.cy;
            e.uFlags |= LLFP_IDEAL_SIZE_CACHE;
        }
        else
            e.uFlags &= ~LLFP_IDEAL_SIZE_CACHE;
        e.uFlags &= ~LLFP_CUT;

        if (e.uFlags & LF_FIX_WIDTH)// 固定尺寸
        {
            e.dAlloc = e.cx;
            e.uFlags |= LLFP_CUT;
        }
        else if (e.uFlags & LF_IDEAL_WIDTH)// 固定为理想尺寸
        {
            e.dAlloc = e.cxIdeal;
            e.uFlags |= LLFP_CUT;
            dLeave -= e.cxIdeal;
        }
    }

    void OnAddObject(ITEM& e) noexcept
    {
        if (e.uFlags & (LF_FIX | LF_SCALE))
        {
            const auto size = e.pObject->LoGetSize();
            e.cx = size.cx;
            e.cy = size.cy;
            if (e.uFlags & LF_FIX_WIDTH)
                m_dFixedSum += size.cx;
        }
        m_dFixedSum += (e.Margin.l + e.Margin.r);
        if (!(e.uFlags & (LF_FIX_WIDTH | LF_IDEAL_WIDTH)))
            m_uWeightSum += e.uWeight;
        if (e.uFlags & LF_IDEAL_WIDTH)
            m_bHasIdeal = TRUE;
    }
public:
    void LoCommit() noexcept override
    {
        //
        // 将理想尺寸从剩余空间中排除
        //

        TLytCoord dLeave = m_cx - m_dFixedSum;
        BOOL bPrepared{};
        if (m_bHasIdeal)
        {
            for (auto& e : m_vItem)
                PreArrangeObject(e, dLeave);
            bPrepared = TRUE;
        }
        //
        // 假设空间充足，为带权对象做有下限（即理想尺寸）的分配
        //

        UINT wSum = m_uWeightSum;
        UINT wShrinkableSum{};// 可收缩对象的权重和
        EckLoop()
        {
            UINT wNewSum{};
            BOOL bClamped{};
            for (auto& e : m_vItem)
            {
                if (!bPrepared)
                    PreArrangeObject(e, dLeave);
                if (e.uFlags & LLFP_CUT)
                    continue;

                const auto dTemp = dLeave * e.uWeight / wSum;
                if (IsBitSet(e.uFlags, LF_FILL_WIDTH | LLFP_IDEAL_SIZE_CACHE) &&
                    dTemp < e.cxIdeal)// FILL，且有理想尺寸，且比例分配到的尺寸过小
                {
                    e.dAlloc = e.cxIdeal;
                    dLeave -= e.cxIdeal;
                    e.uFlags |= LLFP_CUT;
                    bClamped = TRUE;
                }
                else
                    wNewSum += e.uWeight;
            }

            if (bClamped)
            {
                bPrepared = TRUE;
                wSum = wNewSum;
            }
            else// 已收敛
            {
                wNewSum = 0;
                for (auto& e : m_vItem)
                {
                    if (!(e.uFlags & LLFP_CUT))
                        e.dAlloc = dLeave * e.uWeight / wSum;
                    if (!(e.uFlags & (LF_FIX_WIDTH | LF_IDEAL_WIDTH)))
                        wNewSum += e.uWeight;
                    if (e.uFlags & LF_FILL_WIDTH)
                        wShrinkableSum += e.uWeight;
                }
                wSum = wNewSum;
                break;
            }
        }
        //
        // 若空间不足，收缩可收缩的对象，完成所有对象的排列
        //

        const auto bShrink = (dLeave < 0);
        TLytCoord x = m_x, y, cx, cy;
        for (auto& e : m_vItem)
        {
            if (e.uFlags & LF_FILL_WIDTH)
            {
                if (bShrink)
                    e.dAlloc += (dLeave * e.uWeight / wShrinkableSum);
            }

            cx = e.dAlloc;
            if (cx < 0)
                cx = 0;

            if (e.uFlags & LF_FIX_HEIGHT)
                cy = e.cy;
            else if (e.uFlags & LF_IDEAL_HEIGHT)
                cy = e.cyIdeal;
            else if (e.uFlags & LF_SCALE)
                cy = std::min(cx * e.cy / e.cx, m_cy - e.Margin.t - e.Margin.b);
            else
                cy = m_cy - e.Margin.t - e.Margin.b;

            y = ArgCalculateVerticalAlignment(m_y, m_cy, e, cy);
            ArgMoveObject(e, { x + e.Margin.l, y, cx, cy });
            x += (cx + e.Margin.l + e.Margin.r);
        }
    }

    void LobRefresh() noexcept override
    {
        m_dFixedSum = 0;
        m_uWeightSum = 0;
        m_bHasIdeal = FALSE;
        for (auto& e : m_vItem)
            OnAddObject(e);
    }

    size_t Add(ILayout* pObject, const LYTMARGINS& Margin = {},
        UINT uFlags = 0u, UINT uWeight = 0u) noexcept
    {
        EckAssert(LfValidateFlags(uFlags));
        auto& e = m_vItem.PushBack();
        e.pObject = pObject;
        e.Margin = Margin;
        e.uFlags = uFlags;
        e.uWeight = uWeight;
        OnAddObject(e);
        return m_vItem.Size() - 1;
    }
};
ECK_RTTI_IMPL_BASE_INLINE(CLinearLayoutH, CLinearLayoutBase);
ECK_NAMESPACE_END