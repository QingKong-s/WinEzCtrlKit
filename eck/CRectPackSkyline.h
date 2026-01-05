#pragma once
#include "CTrivialBuffer.h"

ECK_NAMESPACE_BEGIN
class CRectPackSkyline
{
public:
    using TCoord = UINT;

    struct RECT
    {
        TCoord x, y, cx, cy;
    };
private:
    struct LINE
    {
        TCoord x;
        TCoord y;
        TCoord cx;
    };

    struct BEST_POS
    {
        TCoord y;
        TCoord cxLineLeave;// 最后一条线的未用宽度
        UINT idx;   // 含
        UINT idxEnd;// 含
    };

    TCoord m_cx{};
    TCoord m_cy{};
    CTrivialBuffer<LINE> m_vSkyline;

    BOOL BlfCanPlaceAtLeft(size_t idx, TCoord cx, TCoord cy,
        _Out_ BEST_POS& Pos) const noexcept
    {
        Pos.idx = (UINT)idx;
        TCoord y = m_vSkyline[idx].y;

        TCoord cxLeave = cx;
        for (size_t i = idx; ; ++i)
        {
            if (i >= m_vSkyline.Size())
                return FALSE;
            const auto& e = m_vSkyline[i];
            if (y < e.y)// 跨越多段天际线，需要取最高点
                y = e.y;

            if (cxLeave <= e.cx)
            {
                Pos.idxEnd = (UINT)i;
                Pos.cxLineLeave = e.cx - cxLeave;
                break;
            }
            cxLeave -= e.cx;
        }

        if (y + cy > m_cy)
            return FALSE;

        Pos.y = y;
        return TRUE;
    }

    BOOL BlfCanPlaceAtRight(size_t idx, TCoord cx, TCoord cy,
        _Out_ BEST_POS& Pos) const noexcept
    {
        Pos.idxEnd = (UINT)idx;
        TCoord y = m_vSkyline[idx].y;

        TCoord cxLeave = cx;
        for (size_t i = idx; ; --i)
        {
            const auto& e = m_vSkyline[i];
            if (y < e.y)// 跨越多段天际线，需要取最高点
                y = e.y;

            if (cxLeave <= e.cx)
            {
                Pos.idx = (UINT)i;
                Pos.cxLineLeave = e.cx - cxLeave;
                break;
            }
            cxLeave -= e.cx;

            if (i == 0)
                return FALSE;
        }

        if (y + cy > m_cy)
            return FALSE;

        Pos.y = y;
        return TRUE;
    }

    TCoord BlfCalculateWasteArea(const BEST_POS& Pos, TCoord cxNew) const noexcept
    {
        TCoord Waste{};
        TCoord cxLeave = cxNew;
        for (size_t i = Pos.idx; i <= Pos.idxEnd; ++i)
        {
            const auto& e = m_vSkyline[i];
            EckAssert(Pos.y >= e.y);
            if (cxLeave <= e.cx)
            {
                Waste += ((Pos.y - e.y) * cxLeave);
                break;
            }
            Waste += ((Pos.y - e.y) * e.cx);
            cxLeave -= e.cx;
        }
        return Waste;
    }

    void LmpMergeEqualHeight(size_t idxBegin, size_t idxEnd) noexcept
    {
        if (idxEnd >= m_vSkyline.Size())
            idxEnd = m_vSkyline.Size() - 1;

        size_t idx = idxBegin;
        for (size_t i = idxBegin + 1; ; )
        {
            if (i > idxEnd)
                break;
            if (m_vSkyline[idx].y == m_vSkyline[i].y)
            {
                m_vSkyline[idx].cx += m_vSkyline[i].cx;
                if (i == idxEnd)
                {
                    m_vSkyline.Erase(idx + 1, i - idx);
                    break;
                }
                ++i;
            }
            else
            {
                const auto c = i - idx - 1;
                m_vSkyline.Erase(idx + 1, c);
                idxEnd -= c;
                ++idx;
                i = idx + 1;
                if (idx > idxEnd || i > idxEnd)
                    break;
            }
        }
    }

    void LmPlaceRectangle(TCoord cx, TCoord cy, const BEST_POS& Pos, BOOL bRight) noexcept
    {
        // 矩形的某个顶点一定与天际线端点重合
        // 如果矩形放置在一条天际线内，则操作后天际线段数不确定
        // 如果矩形跨越多条天际线，则操作后天际线段数一定小于等于当前
        auto& e = m_vSkyline[Pos.idx];
        if (Pos.idx == Pos.idxEnd)
        {
            if (!Pos.cxLineLeave)// 水平方向上完全占用此条天际线，操作后检查前后同高，最终段数小于等于当前
            {
                EckAssert(e.y == Pos.y);
                e.y = e.y + cy;
                LmpMergeEqualHeight(Pos.idx ? Pos.idx - 1 : Pos.idx, Pos.idx + 1);
            }
            else if (Pos.idx && !bRight &&
                m_vSkyline[Pos.idx - 1].y == e.y + cy)// 检查左边高度
            {
                m_vSkyline[Pos.idx - 1].cx += cx;
                e.x += cx;
                e.cx = Pos.cxLineLeave;
                LmpMergeEqualHeight(Pos.idx - 1, Pos.idx);
            }
            else if (Pos.idx != m_vSkyline.Size() - 1 && bRight &&
                m_vSkyline[Pos.idx + 1].y == e.y + cy)// 检查右边高度
            {
                m_vSkyline[Pos.idx + 1].cx += cx;
                e.x += cx;
                e.cx = Pos.cxLineLeave;
                LmpMergeEqualHeight(Pos.idx, Pos.idx + 1);
            }
            else
            {
                if (bRight)
                {
                    e.cx = Pos.cxLineLeave;
                    m_vSkyline.Insert(Pos.idx + 1, { e.x + Pos.cxLineLeave, Pos.y + cy, cx });
                    LmpMergeEqualHeight(Pos.idx, Pos.idx + 1);// 无需检查左边
                }
                else
                {
                    m_vSkyline.Insert(Pos.idx, { e.x, Pos.y + cy, cx });
                    m_vSkyline[Pos.idx + 1].x += cx;
                    m_vSkyline[Pos.idx + 1].cx = Pos.cxLineLeave;
                    LmpMergeEqualHeight(Pos.idx ? Pos.idx - 1 : Pos.idx, Pos.idx);// 无需检查右边
                }
            }
        }
        else
        {
            EckAssert(Pos.idx < Pos.idxEnd);
            if (bRight && Pos.cxLineLeave)
            {
                e.cx = Pos.cxLineLeave;// 剩余部分
                // 新线
                m_vSkyline[Pos.idx + 1] =
                {
                    e.x + e.cx,
                    Pos.y + cy,
                    cx
                };
                m_vSkyline.Erase(Pos.idx + 2, Pos.idxEnd - Pos.idx - 1);
                LmpMergeEqualHeight(Pos.idx, Pos.idx + 1);// 无需检查左边
            }
            else
            {
                e.y = Pos.y + cy;
                e.cx = cx;
                if (!Pos.cxLineLeave)// 少删一个，作为当前矩形跨越形成的新线
                {
                    m_vSkyline.Erase(Pos.idx + 1, Pos.idxEnd - Pos.idx);
                    LmpMergeEqualHeight(Pos.idx ? Pos.idx - 1 : Pos.idx, Pos.idx + 1);
                }
                else// 少删两个
                {
                    // 剩余部分
                    auto& f = m_vSkyline[Pos.idx + 1];
                    f.x = e.x + cx;
                    f.y = m_vSkyline[Pos.idxEnd].y;
                    f.cx = Pos.cxLineLeave;

                    m_vSkyline.Erase(Pos.idx + 2, Pos.idxEnd - Pos.idx - 1);
                    // 无需检查右边
                    LmpMergeEqualHeight(Pos.idx ? Pos.idx - 1 : Pos.idx, Pos.idx);
                }
            }
        }
    }
#if ECK_OPT_SKYLINE_VALIDATE
    void DbgValidateSkyline() const noexcept
    {
        for (size_t i = 1; i < m_vSkyline.Size(); ++i)
        {
            const auto& ePrev = m_vSkyline[i - 1];
            const auto& e = m_vSkyline[i];
            EckAssert(ePrev.x + ePrev.cx == e.x);
            EckAssert(ePrev.y != e.y);
        }
        for (size_t i = 0; i < m_vSkyline.Size(); ++i)
        {
            const auto& e = m_vSkyline[i];
            EckAssert(e.x + e.cx <= m_cx);
            EckAssert(e.y <= m_cy);
        }
        EckAssert(m_vSkyline[0].x == 0);
        EckAssert(m_vSkyline[m_vSkyline.Size() - 1].x + m_vSkyline[m_vSkyline.Size() - 1].cx == m_cx);
    }
    void DbgValidateAllocatedRectangle(const RECT& rc) const noexcept
    {
        DbgValidateAllocatedRectangle(&rc, 1);
    }
#endif
public:
    CRectPackSkyline() = default;
    CRectPackSkyline(TCoord cx, TCoord cy) noexcept : m_cx{ cx }, m_cy{ cy }
    {
        m_vSkyline.Reserve(std::min((TCoord)36, cx / 4));
        m_vSkyline.PushBack({ 0, 0, cx });
    }

    void Initialize(TCoord cx, TCoord cy) noexcept
    {
        m_cx = cx, m_cy = cy;
        m_vSkyline.Reserve(std::min((TCoord)36, cx / 4));
        m_vSkyline.ReSize(1);
        m_vSkyline[0] = { 0, 0, cx };
    }

    BOOL AllocateBottomLeft(TCoord cx, TCoord cy, RECT& rcNew) noexcept
    {
        constexpr auto InvalidCoord = std::numeric_limits<TCoord>::max();

        BEST_POS Pos, BestPos;
        BestPos.y = InvalidCoord;

        for (size_t i = 0; i < m_vSkyline.Size(); ++i)
        {
            if (m_vSkyline[i].x + cx > m_cx)
                break;
            if (!BlfCanPlaceAtLeft(i, cx, cy, Pos))
                continue;
            if (Pos.y < BestPos.y)
                BestPos = Pos;
        }

        if (BestPos.y == InvalidCoord)
            return FALSE;

        rcNew.x = m_vSkyline[BestPos.idx].x;
        rcNew.y = BestPos.y;
        rcNew.cx = cx;
        rcNew.cy = cy;

        LmPlaceRectangle(cx, cy, BestPos, FALSE);
#if ECK_OPT_SKYLINE_VALIDATE
        DbgValidateSkyline();
        DbgValidateAllocatedRectangle(rcNew);
#endif
        return TRUE;
    }

    BOOL AllocateMinWaste(TCoord cx, TCoord cy, RECT& rcNew) noexcept
    {
        constexpr auto InvalidCoord = std::numeric_limits<TCoord>::max();

        BEST_POS Pos, BestPos;
        BestPos.y = InvalidCoord;
        TCoord MinWaste = InvalidCoord;
        BOOL bRight{};

        for (size_t i = 0; i < m_vSkyline.Size(); ++i)
        {
            const auto& e = m_vSkyline[i];
            if (e.x + cx <= m_cx)
            {
                const auto bL = BlfCanPlaceAtLeft(i, cx, cy, Pos);
                if (bL)
                {
                    const auto Waste = BlfCalculateWasteArea(Pos, cx);
                    if (Pos.y < BestPos.y ||
                        (Pos.y == BestPos.y && MinWaste > Waste))
                    {
                        BestPos = Pos;
                        bRight = FALSE;
                        MinWaste = Waste;
                    }
                }
            }

            if (e.x + e.cx >= cx)
            {
                const auto bR = BlfCanPlaceAtRight(i, cx, cy, Pos);
                if (bR)
                {
                    const auto Waste = BlfCalculateWasteArea(Pos, cx);
                    if (Pos.y < BestPos.y ||
                        (Pos.y == BestPos.y && MinWaste > Waste))
                    {
                        BestPos = Pos;
                        bRight = FALSE;
                        MinWaste = Waste;
                    }
                }
            }
        }
        if (BestPos.y == InvalidCoord)
            return FALSE;
        if (bRight)
            rcNew.x = m_vSkyline[BestPos.idx].x + BestPos.cxLineLeave;
        else
            rcNew.x = m_vSkyline[BestPos.idx].x;
        rcNew.y = BestPos.y;
        rcNew.cx = cx;
        rcNew.cy = cy;
        LmPlaceRectangle(cx, cy, BestPos, bRight);
#if ECK_OPT_SKYLINE_VALIDATE
        DbgValidateSkyline();
        DbgValidateAllocatedRectangle(rcNew);
#endif
        return TRUE;
    }

    EckInlineCe void GetSize(_Out_ TCoord& cx, _Out_ TCoord& cy) const noexcept
    {
        cx = m_cx;
        cy = m_cy;
    }

    BOOL IsEmpty() const noexcept
    {
        if (m_vSkyline.Size() == 1)
        {
            const auto& e = m_vSkyline.Front();
            if (e.x == 0 && e.y == 0)
            {
                EckAssert(e.cx == m_cx);
                return TRUE;
            }
        }
        return FALSE;
    }

    BOOL ReSize(TCoord cx, TCoord cy) noexcept
    {
        if (m_cx > cx || m_cy > cy)
            return FALSE;
        m_cx = cx;
        m_cy = cy;
        m_vSkyline.Back().cx = m_cx - m_vSkyline.Back().x;
        return TRUE;
    }

    BOOL ReSizeEmpty(TCoord cx, TCoord cy) noexcept
    {
        if (!IsEmpty())
            return FALSE;
        m_cx = cx;
        m_cy = cy;
        m_vSkyline.Front() = { 0, 0, m_cx };
    }

    void Clear() noexcept
    {
        m_vSkyline.ReSize(1);
        m_vSkyline.Front() = { 0, 0, m_cx };
    }

#if ECK_OPT_SKYLINE_VALIDATE
    void DbgValidateAllocatedRectangle(_In_reads_(c) const RECT* p, size_t c) const noexcept
    {
        for (size_t i = 0; i < c; ++i)
        {
            const auto& rc = p[i];

            EckAssert(rc.cx);
            EckAssert(rc.cy);
            EckAssert(rc.x + rc.cx <= m_cx);
            EckAssert(rc.y + rc.cy <= m_cy);

            size_t idxStart = m_vSkyline.Size();
            for (size_t j = 0; j < m_vSkyline.Size(); ++j)
            {
                const auto& e = m_vSkyline[j];
                if (rc.x >= e.x && rc.x < e.x + e.cx)
                {
                    idxStart = j;
                    break;
                }
            }
            EckAssert(idxStart != m_vSkyline.Size());// 必须能找到起始段

            auto Remain = rc.cx;
            size_t j = idxStart;
            auto OffsetInFirst = rc.x - m_vSkyline[idxStart].x;
            while (1)
            {
                EckAssert(j < m_vSkyline.Size());
                const auto& e = m_vSkyline[j];
                // 矩形在该段的可用宽度
                const auto Avail = e.cx - (j == idxStart ? OffsetInFirst : 0);
                // 矩形顶部必须不超过该段高度
                EckAssert(rc.y + rc.cy <= e.y);
                if (Remain <= Avail)// 矩形右端应在此段内
                    break;
                Remain -= Avail;
                ++j;
            }
        }
    }
#endif
};
ECK_NAMESPACE_END