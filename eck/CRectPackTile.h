#pragma once
#include "CTrivialBuffer.h"
#include "CSelectionRange.h"

ECK_NAMESPACE_BEGIN
class CRectPackTile
{
public:
    using TId = UINT;
    using TCoord = USHORT;
private:
    struct Page
    {
        TCoord cCol;
        TCoord cx, cy;
        TId idxBegin;// 本页第一个磁贴的线性索引
    };
    CTrivialBuffer<Page> m_vPage{};
    CSelRange m_rgFree{};
    TCoord m_cxTile{}, m_cyTile{};
    TCoord m_cxPage{}, m_cyPage{};
    TCoord m_cxPageMax{}, m_cyPageMax{};
    TId m_cTile{};
    BOOLEAN m_bIncPageSize{ TRUE };

    // 取线性索引所在页
    constexpr TId FlatIndexToPage(TId idxFlat) const noexcept
    {
        const auto it = std::lower_bound(
            m_vPage.begin(), m_vPage.end(), idxFlat,
            [](const Page& e, TId idx) { return e.idxBegin < idx; });
        if (it != m_vPage.end() && idxFlat >= it->idxBegin)
            return TId(it - m_vPage.begin());
        EckAssert(it != m_vPage.begin());
        return TId(it - 1 - m_vPage.begin());
    }
public:
    constexpr CRectPackTile(
        TCoord cxTile, TCoord cyTile,
        TCoord cxPage, TCoord cyPage,
        TCoord cxPageMax, TCoord cyPageMax,
        BOOLEAN bIncPageSize = TRUE) noexcept :
        m_cxTile{ cxTile }, m_cyTile{ cyTile },
        m_cxPage{ cxPage }, m_cyPage{ cyPage },
        m_cxPageMax{ cxPageMax }, m_cyPageMax{ cyPageMax },
        m_bIncPageSize{ bIncPageSize }
    {
        if (m_cxPage > m_cxPageMax)
            m_cxPage = m_cxPageMax;
        if (m_cyPage > m_cyPageMax)
            m_cyPage = m_cyPageMax;
    }

    // 返回线性索引
    TId Allocate(
        _Out_ TId& idxPage,
        _Out_ TCoord& x,
        _Out_ TCoord& y) noexcept
    {
        if (m_rgFree.IsEmpty())// 需要分配新页
        {
            auto& e = m_vPage.PushBack();
            e.cx = m_cxPage;
            e.cy = m_cyPage;
            e.idxBegin = m_cTile;
            e.cCol = m_cxPage / m_cxTile;
            m_cTile += ((TId)e.cCol * (e.cy / m_cyTile));
            m_rgFree.IncludeRange(e.idxBegin, m_cTile - 1);

            if (m_bIncPageSize)
            {
                if (m_cxPage < m_cyPage)
                    m_cxPage = std::min(TCoord(m_cxPage * 3 / 2), m_cxPageMax);
                else
                    m_cyPage = std::min(TCoord(m_cyPage * 3 / 2), m_cyPageMax);
            }
        }
        const auto idx = m_rgFree.GetFirstSelected();
        m_rgFree.ExcludeItem(idx);
        GetTilePosition(idx, idxPage, x, y);
        return idx;
    }

    void Free(TId id) noexcept
    {
        m_rgFree.IncludeItem(id);
    }

    constexpr void GetTilePosition(
        TId uId,
        _Out_ TId& idxPage,
        _Out_ TCoord& x,
        _Out_ TCoord& y) const noexcept
    {
        idxPage = FlatIndexToPage(uId);
        const auto& e = m_vPage[idxPage];
        const auto idxInPage = uId - e.idxBegin;
        const auto idxCol = idxInPage % e.cCol;
        const auto idxRow = idxInPage / e.cCol;
        x = TCoord(idxCol * m_cxTile);
        y = TCoord(idxRow * m_cyTile);
    }

    EckInlineCe void SetCurrentPageSize(TCoord cx, TCoord cy) noexcept
    {
        m_cxPage = std::min(cx, m_cxPageMax);
        m_cyPage = std::min(cy, m_cyPageMax);
    }
    EckInlineCe void GetCurrentPageSize(_Out_ TCoord& cx, _Out_ TCoord& cy) const noexcept
    {
        cx = m_cxPage;
        cy = m_cyPage;
    }

    EckInlineCe void GetTileSize(_Out_ TCoord& cx, _Out_ TCoord& cy) const noexcept
    {
        cx = m_cxTile;
        cy = m_cyTile;
    }

    EckInlineCe void GetPageSize(TId idxPage, _Out_ TCoord& cx, _Out_ TCoord& cy) const noexcept
    {
        const auto& e = m_vPage[idxPage];
        cx = e.cx;
        cy = e.cy;
    }

    EckInlineNdCe BOOL HasFreeTile() const noexcept { return !m_rgFree.IsEmpty(); }
    EckInlineNdCe UINT GetPageCount() const noexcept { return (UINT)m_vPage.Size(); }
    EckInlineNdCe UINT GetCount() const noexcept { return m_cTile - m_rgFree.CountIncluded(); }
};
ECK_NAMESPACE_END