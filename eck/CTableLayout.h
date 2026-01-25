#pragma once
#include "CLayoutBase.h"
#include "CTrivialBuffer.h"

ECK_NAMESPACE_BEGIN
// 
// 表格布局
// 支持九向对齐
// 
// 表格行列的有效标志如下，可使用任一方向的标志，如指定FIX_WIDTH、FIX_HEIGHT
// 或两者同时指定均可表示FIX的含义
//   FIX    行列尺寸是由调用方指定的固定值
//   IDEAL  行列尺寸取决于内容
//   FLEX   行列尺寸按权重分配剩余空间
// 
// 若对象所处的某行某列指定了IDEAL，则布局器将考虑对象尺寸，
// 计算对象尺寸时支持FIX、IDEAL，若未指定上述标志，则认为对象尺寸为0
// 
// 排列时支持FIX、IDEAL、SCALE，其余标志均被视为FLEX
// 
// TODO 跨行跨列
//

namespace Priv
{
    class CTableManager
    {
    public:
        struct ROWCOL
        {
            USHORT uWeight;
            USHORT uFlags;// 空间分配选项，LF_
            TLytCoord xy;
            TLytCoord d;
        };
    private:
        CTrivialBuffer<ROWCOL> m_RowCol{};// 行 | 列
        USHORT m_cRow{}, m_cCol{};
        USHORT m_uWeightSumRow{}, m_uWeightSumCol{};
        BOOLEAN m_bHasIdeal{};
    public:
        void ReSize(USHORT cRow, USHORT cCol) noexcept
        {
            m_cRow = cRow;
            m_cCol = cCol;
            m_RowCol.ReSize(cRow + cCol);
        }

        void Clear() noexcept
        {
            m_cRow = m_cCol = 0;
            m_bHasIdeal = TRUE;
            m_RowCol.Clear();
        }

        auto& Row(USHORT idx) noexcept
        {
            EckAssert(idx < m_cRow);
            return m_RowCol[idx];
        }
        auto& Column(USHORT idx) noexcept
        {
            EckAssert(idx < m_cCol);
            return m_RowCol[idx + m_cRow];
        }

        EckInlineNdCe USHORT GetRowCount() const noexcept { return m_cRow; }
        EckInlineNdCe USHORT GetColumnCount() const noexcept { return m_cCol; }

        EckInlineCe void SetHasIdeal(BOOL b) noexcept { m_bHasIdeal = b; }
        EckInlineNdCe BOOL HasIdeal() noexcept { return m_bHasIdeal; }

        std::span<ROWCOL> Row() noexcept { return m_RowCol.SubSpan(0, m_cRow); }
        std::span<ROWCOL> Column() noexcept { return m_RowCol.SubSpan(m_cRow, m_cCol); }
        std::span<ROWCOL> RowColumn() noexcept { return m_RowCol.ToSpan(); }

        EckInlineNdCe USHORT GetRowWeightSum() const noexcept { return m_uWeightSumRow; }
        EckInlineNdCe USHORT GetColumnWeightSum() const noexcept { return m_uWeightSumCol; }

        void RefreshRowWeight() noexcept
        {
            m_uWeightSumRow = 0;
            for (const auto& e : Row())
                if (!(e.uFlags & (LF_FIX | LF_IDEAL)))
                    m_uWeightSumRow += e.uWeight;
        }
        void RefreshColumnWeight() noexcept
        {
            m_uWeightSumCol = 0;
            for (const auto& e : Column())
                if (!(e.uFlags & (LF_FIX | LF_IDEAL)))
                    m_uWeightSumCol += e.uWeight;
        }
    };
}

class CTableLayout : public CLayoutBase
{
public:
    ECK_RTTI(CTableLayout);
private:
    struct ITEM : ITEMBASE
    {
        USHORT idxRow;
        USHORT idxCol;
        USHORT cRowSpan;
        USHORT cColSpan;
    };

    CTrivialBuffer<ITEM> m_vItem{};
    Priv::CTableManager m_Table{};

    void OnAddObject(ITEM& e) noexcept
    {
        if (e.uFlags & LF_FIX)
        {
            const auto size = e.pObject->LoGetSize();
            e.cx = size.cx;
            e.cy = size.cy;
        }
    }
public:
    void LoCommit() noexcept override
    {
        // 若需要计算行列理想尺寸，则给一个初值
        for (auto& e : m_Table.RowColumn())
        {
            if (!(e.uFlags & LF_FIX))
                e.d = 0;
        }
        // 计算行列的理想尺寸，并从当前布局尺寸中减去已固定部分
        TLytCoord d;
        TLytCoord cxLeave = m_cx, cyLeave = m_cy;
        BOOL bMergeCell{};
        for (auto& e : m_vItem)
        {
            LYTSIZE sizeIdeal{};
            auto& Row = m_Table.Row(e.idxRow);
            auto& Col = m_Table.Column(e.idxCol);
            if (e.uFlags & LF_IDEAL)
            {
                e.pObject->LoGetIdealSize(sizeIdeal);
                if (e.uFlags & LF_IDEAL_WIDTH)
                    e.cx = sizeIdeal.cx;
                if (e.uFlags & LF_IDEAL_HEIGHT)
                    e.cy = sizeIdeal.cy;
            }

            if (e.cRowSpan == 1)
                if (Row.uFlags & LF_IDEAL)
                {
                    d = e.Margin.t + e.Margin.b;
                    if (e.uFlags & (LF_FIX_HEIGHT | LF_IDEAL_HEIGHT))
                        d += e.cy;
                    if (d > Row.d)
                    {
                        cyLeave -= (d - Row.d);
                        Row.d = d;
                    }
                }
                else if (Row.uFlags & LF_FIX)
                    cyLeave -= Row.d;

            if (e.cColSpan == 1)
                if (Col.uFlags & LF_IDEAL)
                {
                    d = e.Margin.l + e.Margin.r;
                    if (e.uFlags & (LF_FIX_WIDTH | LF_IDEAL_WIDTH))
                        d += e.cx;
                    if (d > Col.d)
                    {
                        cxLeave -= (d - Col.d);
                        Col.d = d;
                    }
                }
                else if (Col.uFlags & LF_FIX)
                    cxLeave -= Col.d;
        }
        // 计算FLEX行列尺寸，排列所有行列
        TLytCoord xy = m_y;
        for (auto& e : m_Table.Row())
        {
            if (!(e.uFlags & (LF_FIX | LF_IDEAL)))
                e.d = cyLeave * e.uWeight / m_Table.GetRowWeightSum();
            e.xy = xy;
            xy += e.d;
        }
        xy = m_x;
        for (auto& e : m_Table.Column())
        {
            if (!(e.uFlags & (LF_FIX | LF_IDEAL)))
                e.d = cxLeave * e.uWeight / m_Table.GetColumnWeightSum();
            e.xy = xy;
            xy += e.d;
        }
        // 排列对象
        LYTRECT rc;
        for (auto& e : m_vItem)
        {
            const auto idxBeginRow = e.idxRow;
            const auto idxEndRow = e.idxRow + e.cRowSpan - 1;
            const auto idxBeginCol = e.idxCol;
            const auto idxEndCol = e.idxCol + e.cColSpan - 1;
            TLytCoord cx{}, cy{};
            for (auto i = idxBeginRow; i <= idxEndRow; ++i)
                cy += m_Table.Row(i).d;
            for (auto i = idxBeginCol; i <= idxEndCol; ++i)
                cx += m_Table.Column(i).d;

            ArgCalculateRect(e,
                {
                    m_Table.Column(e.idxCol).xy,
                    m_Table.Row(e.idxRow).xy,
                    cx,
                    cy
                },
                rc);
            ArgMoveObject(e, rc);
        }
    }

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
        m_Table.Clear();
    }

    void LobRefresh() noexcept override
    {
        for (auto& e : m_vItem)
            OnAddObject(e);
    }

    size_t LobGetObjectCount() const noexcept override { return m_vItem.Size(); }

    size_t Add(ILayout* pObject, USHORT idxRow, USHORT idxCol,
        const LYTMARGINS& Margin = {}, UINT uFlags = 0,
        USHORT cRowSpan = 1, USHORT cColSpan = 1) noexcept
    {
        auto& e = m_vItem.PushBack();
        e.pObject = pObject;
        e.Margin = Margin;
        e.uFlags = uFlags;
        e.idxRow = idxRow;
        e.idxCol = idxCol;
        e.cRowSpan = cRowSpan;
        e.cColSpan = cColSpan;
        OnAddObject(e);
        return m_vItem.Size() - 1;
    }

    EckInlineNdCe auto& GetList() noexcept { return m_vItem; }
    EckInlineNdCe auto& GetTable() noexcept { return m_Table; }

    void SetRowFixed(USHORT idx, TLytCoord cy) noexcept
    {
        auto& e = m_Table.Row(idx);
        e.uFlags = LF_FIX;
        e.d = cy;
    }
    void SetRowIdeal(USHORT idx, USHORT uWeight) noexcept
    {
        auto& e = m_Table.Row(idx);
        e.uFlags = LF_IDEAL;
        e.uWeight = uWeight;
    }
    void SetRowFlex(USHORT idx, USHORT uWeight) noexcept
    {
        auto& e = m_Table.Row(idx);
        e.uFlags = 0;
        e.uWeight = uWeight;
    }

    void SetAllRowFixed(TLytCoord cy) noexcept
    {
        for (auto& e : m_Table.Row())
        {
            e.uFlags = LF_FIX;
            e.d = cy;
        }
    }
    void SetAllRowIdeal(USHORT uWeight) noexcept
    {
        for (auto& e : m_Table.Row())
        {
            e.uFlags = LF_IDEAL;
            e.uWeight = uWeight;
        }
    }
    void SetAllRowFlex(USHORT uWeight) noexcept
    {
        for (auto& e : m_Table.Row())
        {
            e.uFlags = 0;
            e.uWeight = uWeight;
        }
    }

    void SetColumnFixed(USHORT idx, TLytCoord cx) noexcept
    {
        auto& e = m_Table.Column(idx);
        e.uFlags = LF_FIX;
        e.d = cx;
    }
    void SetColumnIdeal(USHORT idx, USHORT uWeight) noexcept
    {
        auto& e = m_Table.Column(idx);
        e.uFlags = LF_IDEAL;
        e.uWeight = uWeight;
    }
    void SetColumnFlex(USHORT idx, USHORT uWeight) noexcept
    {
        auto& e = m_Table.Column(idx);
        e.uFlags = 0;
        e.uWeight = uWeight;
    }

    void SetAllColumnFixed(TLytCoord cx) noexcept
    {
        for (auto& e : m_Table.Column())
        {
            e.uFlags = LF_FIX;
            e.d = cx;
        }
    }
    void SetAllColumnIdeal(USHORT uWeight) noexcept
    {
        for (auto& e : m_Table.Column())
        {
            e.uFlags = LF_IDEAL;
            e.uWeight = uWeight;
        }
    }
    void SetAllColumnFlex(USHORT uWeight) noexcept
    {
        for (auto& e : m_Table.Column())
        {
            e.uFlags = 0;
            e.uWeight = uWeight;
        }
    }

    void ReSizeTable(USHORT cRow, USHORT cCol) noexcept { m_Table.ReSize(cRow, cCol); }

    void RefreshTableRowWeight() noexcept { m_Table.RefreshRowWeight(); }
    void RefreshTableColumnWeight() noexcept { m_Table.RefreshColumnWeight(); }
    void RefreshTableWeight() noexcept
    {
        RefreshTableRowWeight();
        RefreshTableColumnWeight();
    }

    EckInlineCe void SetHasIdeal(BOOL b) noexcept { m_Table.SetHasIdeal(b); }
};
ECK_RTTI_IMPL_BASE_INLINE(CTableLayout, CLayoutBase);
ECK_NAMESPACE_END