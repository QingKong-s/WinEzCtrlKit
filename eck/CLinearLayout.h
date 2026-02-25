#pragma once
#include "CLayoutBase.h"
#include "CTrivialBuffer.h"

ECK_NAMESPACE_BEGIN
// 
// 线性布局
// 在非布局流动方向上支持单行对齐
// 支持所有其他布局选项
//

enum : UINT
{
    LLFP_CUT = 1u << 31,                // 从迭代中排除
};

// 不得直接实例化此类
class __declspec(novtable) CLinearLayoutBase : public CLayoutBase
{
public:
    ECK_RTTI(CLinearLayoutBase, CLayoutBase);
protected:
    struct ITEM : ITEMBASE
    {
        UINT uWeight;
        TLytCoord dAlloc;
    };

    CTrivialBuffer<ITEM> m_vItem{};
    TLytCoord m_dIdealSum{};
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
        for (auto& e : m_vItem)
        {
            ReCalculateDpiSize(e, iDpi);
            e.pObject->LoOnDpiChanged(iDpi);
        }
        m_iDpi = iDpi;
        LobRefresh();
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

class CLinearLayoutV : public CLinearLayoutBase
{
public:
    ECK_RTTI(CLinearLayoutV, CLinearLayoutBase);
private:
    static void PreArrangeObject(ITEM& e) noexcept
    {
        e.uFlags &= ~LLFP_CUT;
        if (e.uFlags & LF_FIX_HEIGHT)// 固定尺寸
        {
            e.dAlloc = e.cy;
            e.uFlags |= LLFP_CUT;
        }
        else if (e.uFlags & LF_IDEAL_HEIGHT)// 固定为理想尺寸
        {
            e.dAlloc = e.cy;
            e.uFlags |= LLFP_CUT;
        }
    }

    void UpdateObjectIdealSize(ITEM& e) noexcept
    {
        LYTSIZE sizeIdeal{};
        const auto bHasIdealSize =
            (e.uFlags & (LF_FILL_HEIGHT | LF_IDEAL)) ?
            e.pObject->LoGetIdealSize(sizeIdeal) :
            FALSE;
        if (bHasIdealSize)
        {
            if (e.uFlags & (LF_FILL_WIDTH | LF_IDEAL_WIDTH))
                e.cx = sizeIdeal.cx + e.cxExtra;
            if (e.uFlags & (LF_FILL_HEIGHT | LF_IDEAL_HEIGHT))
                e.cy = sizeIdeal.cy + e.cyExtra;
            if (e.uFlags & LF_IDEAL_HEIGHT)
                m_dIdealSum += (e.cy + e.Margins.t + e.Margins.b);
        }
    }

    void OnAddObject(ITEM& e) noexcept
    {
        // 需要绝对尺寸
        if (e.uFlags & (LF_FIX | LF_SCALE))
        {
            const auto size = e.pObject->LoGetSize();
            if (e.uFlags & (LF_FIX_WIDTH | LF_SCALE))
                e.cx = size.cx;
            if (e.uFlags & (LF_FIX_HEIGHT | LF_SCALE))
                e.cy = size.cy;
            if (e.uFlags & LF_FIX_HEIGHT)
                m_dFixedSum += size.cy;
        }
        // 需要理想尺寸
        UpdateObjectIdealSize(e);
        // 更新尺寸和理想尺寸
        m_cxIdeal = m_cx = std::max(m_cx, e.cx + e.Margins.l + e.Margins.r);
        m_cy += (e.cy + e.Margins.t + e.Margins.b);
        m_cyIdeal = m_cy;
        // 更新固定尺寸和与权重
        m_dFixedSum += (e.Margins.t + e.Margins.b);
        if (!(e.uFlags & (LF_FIX_HEIGHT | LF_IDEAL_HEIGHT)))
            m_uWeightSum += e.uWeight;
        if (e.uFlags & LF_IDEAL_HEIGHT)
            m_bHasIdeal = TRUE;
    }
public:
    void LoCommit() noexcept override
    {
        //
        // 假设空间充足，为带权对象做有下限（即理想尺寸）的分配
        //

        TLytCoord dLeave = m_cy - m_dFixedSum - m_dIdealSum;
        BOOL bPrepared{};

        UINT wSum = m_uWeightSum;
        UINT wShrinkableSum{};// 可收缩对象的权重和
        EckLoop()
        {
            UINT wNewSum{};
            BOOL bClamped{};
            for (auto& e : m_vItem)
            {
                if (!bPrepared)
                    PreArrangeObject(e);
                if (e.uFlags & LLFP_CUT)
                    continue;

                const auto dTemp = dLeave * e.uWeight / wSum;
                if (IsBitSet(e.uFlags, LF_FILL_HEIGHT) &&
                    dTemp < e.cy)// FILL，且有理想尺寸，且比例分配到的尺寸过小
                {
                    e.dAlloc = e.cy;
                    dLeave -= e.cy;
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

            if (e.uFlags & (LF_FIX_WIDTH | LF_IDEAL_WIDTH))
                cx = e.cx;
            else if (e.uFlags & LF_SCALE)
                cx = std::min(cy * e.cx / e.cy, m_cx - e.Margins.l - e.Margins.r);
            else
                cx = m_cx - e.Margins.l - e.Margins.r;

            x = ArgCalculateHorizontalAlignment(m_x, m_cx, e, cx);
            ArgMoveObject(e, { x, y + e.Margins.t, cx, cy });
            y += (cy + e.Margins.t + e.Margins.b);
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

    void LobUpdateIdealSize() noexcept override
    {
        m_cxIdeal = m_cyIdeal = m_dIdealSum = 0;
        for (auto& e : m_vItem)
        {
            UpdateObjectIdealSize(e);
            m_cyIdeal += (e.cy + e.Margins.t + e.Margins.b);
            m_cxIdeal = std::max(m_cxIdeal, e.cx + e.Margins.l + e.Margins.r);
        }
    }

    void LobAddObject(const LOB_PARAM& Param) noexcept override
    {
        EckAssert(LfValidateFlags(Param.uFlags));
        auto& e = m_vItem.PushBack();
        AssignItem(e, Param);
        e.uWeight = Param.uWeight;
        OnAddObject(e);
    }
};

class CLinearLayoutH : public CLinearLayoutBase
{
public:
    ECK_RTTI(CLinearLayoutH, CLinearLayoutBase);
private:
    static void PreArrangeObject(ITEM& e) noexcept
    {
        e.uFlags &= ~LLFP_CUT;
        if (e.uFlags & LF_FIX_WIDTH)// 固定尺寸
        {
            e.dAlloc = e.cx;
            e.uFlags |= LLFP_CUT;
        }
        else if (e.uFlags & LF_IDEAL_WIDTH)// 固定为理想尺寸
        {
            e.dAlloc = e.cx;
            e.uFlags |= LLFP_CUT;
        }
    }

    void UpdateObjectIdealSize(ITEM& e) noexcept
    {
        LYTSIZE sizeIdeal{};
        const auto bHasIdealSize =
            (e.uFlags & (LF_FILL_WIDTH | LF_IDEAL)) ?
            e.pObject->LoGetIdealSize(sizeIdeal) :
            FALSE;
        if (bHasIdealSize)
        {
            if (e.uFlags & (LF_FILL_WIDTH | LF_IDEAL_WIDTH))
                e.cx = sizeIdeal.cx + e.cxExtra;
            if (e.uFlags & (LF_FILL_HEIGHT | LF_IDEAL_HEIGHT))
                e.cy = sizeIdeal.cy + e.cyExtra;
            if (e.uFlags & LF_IDEAL_WIDTH)
                m_dIdealSum += (e.cx + e.Margins.l + e.Margins.r);
        }
    }

    void OnAddObject(ITEM& e) noexcept
    {
        // 需要绝对尺寸
        if (e.uFlags & (LF_FIX | LF_SCALE))
        {
            const auto size = e.pObject->LoGetSize();
            if (e.uFlags & (LF_FIX_WIDTH | LF_SCALE))
                e.cx = size.cx;
            if (e.uFlags & (LF_FIX_HEIGHT | LF_SCALE))
                e.cy = size.cy;
            if (e.uFlags & LF_FIX_WIDTH)
                m_dFixedSum += size.cx;
        }
        // 需要理想尺寸
        UpdateObjectIdealSize(e);
        // 更新尺寸和理想尺寸
        m_cx += (e.cx + e.Margins.l + e.Margins.r);
        m_cxIdeal = m_cx;
        m_cyIdeal = m_cy = std::max(m_cy, e.cy + e.Margins.t + e.Margins.b);
        // 更新固定尺寸和与权重
        m_dFixedSum += (e.Margins.l + e.Margins.r);
        if (!(e.uFlags & (LF_FIX_WIDTH | LF_IDEAL_WIDTH)))
            m_uWeightSum += e.uWeight;
        if (e.uFlags & LF_IDEAL_WIDTH)
            m_bHasIdeal = TRUE;
    }
public:
    void LoCommit() noexcept override
    {
        //
        // 假设空间充足，为带权对象做有下限（即理想尺寸）的分配
        //

        TLytCoord dLeave = m_cx - m_dFixedSum - m_dIdealSum;
        BOOL bPrepared{};

        UINT wSum = m_uWeightSum;
        UINT wShrinkableSum{};// 可收缩对象的权重和
        EckLoop()
        {
            UINT wNewSum{};
            BOOL bClamped{};
            for (auto& e : m_vItem)
            {
                if (!bPrepared)
                    PreArrangeObject(e);
                if (e.uFlags & LLFP_CUT)
                    continue;

                const auto dTemp = dLeave * e.uWeight / wSum;
                if (IsBitSet(e.uFlags, LF_FILL_WIDTH) &&
                    dTemp < e.cx)// FILL，且有理想尺寸，且比例分配到的尺寸过小
                {
                    e.dAlloc = e.cx;
                    dLeave -= e.cx;
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

            if (e.uFlags & (LF_FIX_HEIGHT | LF_IDEAL_HEIGHT))
                cy = e.cy;
            else if (e.uFlags & LF_SCALE)
                cy = std::min(cx * e.cy / e.cx, m_cy - e.Margins.t - e.Margins.b);
            else
                cy = m_cy - e.Margins.t - e.Margins.b;

            y = ArgCalculateVerticalAlignment(m_y, m_cy, e, cy);
            ArgMoveObject(e, { x + e.Margins.l, y, cx, cy });
            x += (cx + e.Margins.l + e.Margins.r);
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

    void LobUpdateIdealSize() noexcept override
    {
        m_cxIdeal = m_cyIdeal = m_dIdealSum = 0;
        for (auto& e : m_vItem)
        {
            UpdateObjectIdealSize(e);
            m_cxIdeal += (e.cx + e.Margins.l + e.Margins.r);
            m_cyIdeal = std::max(m_cyIdeal, e.cy + e.Margins.t + e.Margins.b);
        }
    }

    void LobAddObject(const LOB_PARAM& Param) noexcept override
    {
        EckAssert(LfValidateFlags(Param.uFlags));
        auto& e = m_vItem.PushBack();
        AssignItem(e, Param);
        e.uWeight = Param.uWeight;
        OnAddObject(e);
    }
};
ECK_NAMESPACE_END