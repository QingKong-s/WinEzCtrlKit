#pragma once
#include "CMemorySet.h"

ECK_NAMESPACE_BEGIN
class CColorReducing
{
private:
    struct NODE
    {
        UINT r, g, b;       // 颜色和
        UINT cChild : 4;    // 子节点数
        UINT bLeaf : 1;     // 叶节点
        UINT bCut : 1;      // 是否可裁剪
        UINT cColor : 26;   // 颜色数
        NODE* pChild[8];    // 子节点
        NODE* pNext;        // 同一层的后继节点，仅供常规节点使用
    };

    CMemorySet m_MemSet{};
    NODE m_Root{};
    NODE* m_apLast[8]{ &m_Root };
    UINT m_cValid;      // 不重复的颜色数
    UINT m_cMax;        // 最大颜色数
    BOOL m_bNotInit;    // 是否非首次运行

    constexpr UINT ComputeColorError(NODE* p) noexcept
    {
        if (!p->cChild)
            return 0;
        const auto AvgR = p->r / p->cColor;
        const auto AvgG = p->g / p->cColor;
        const auto AvgB = p->b / p->cColor;
        UINT t{};
        for (auto& e : p->pChild)
        {
            if (!e)
                continue;
            const auto ChildAvgR = e->r / e->cColor;
            const auto ChildAvgG = e->g / e->cColor;
            const auto ChildAvgB = e->b / e->cColor;

            const auto dr = (ChildAvgR > AvgR) ? (ChildAvgR - AvgR) : (AvgR - ChildAvgR);
            const auto dg = (ChildAvgG > AvgG) ? (ChildAvgG - AvgG) : (AvgG - ChildAvgG);
            const auto db = (ChildAvgB > AvgB) ? (ChildAvgB - AvgB) : (AvgB - ChildAvgB);

            t += (dr * dr + dg * dg + db * db) * e->cColor;
        }
        return t;
    }

    constexpr void RemoveChild(NODE* p) noexcept
    {
        if (!p->bCut)
            return;
        p->bCut = FALSE;
        for (auto e : p->pChild)
        {
            if (e)
                RemoveChild(e);
        }
    }

    constexpr void ReduceColor() noexcept
    {
        NODE* pCurr{};
        UINT MinError{ 0xFFFFFFFF };
        // 从最深层开始搜索
        for (int i = 7; i >= 0; --i)
        {
            auto p{ m_apLast[i] };
            while (p)
            {
                if (p->cChild && p->bCut)
                {
                    UINT error = ComputeColorError(p);
                    // 选择误差最小的节点进行合并
                    if (error < MinError)
                    {
                        MinError = error;
                        pCurr = p;
                    }
                }
                p = p->pNext;
            }
            if (pCurr)
                break;
            else
                m_apLast[i] = nullptr;
        }
        EckAssert(pCurr);
        pCurr->bLeaf = TRUE;
        m_cValid -= (pCurr->cChild - 1);
        RemoveChild(pCurr);
    }
public:
    CColorReducing() noexcept
    {
        m_MemSet.SetPageSize(4096 * 30);
        m_Root.bCut = TRUE;
    }
    CColorReducing(UINT cMaxColor) noexcept : CColorReducing{}
    {
        m_cMax = cMaxColor;
    }

    // 最大颜色数不会被重置
    void Reset() noexcept
    {
        m_MemSet.ClearRecord();
        m_Root = {};
        m_bNotInit = TRUE;
        m_Root.bCut = TRUE;
        ZeroMemory(m_apLast, sizeof(m_apLast));
    }

    void AddColor(UINT r, UINT g, UINT b) noexcept
    {
        UINT nCurrLevel{}, nShift{ 7 }, idx;
        NODE* pCurrNode{ &m_Root };
        pCurrNode->r += r;
        pCurrNode->g += g;
        pCurrNode->b += b;
        pCurrNode->cColor += 1;
        for (; nCurrLevel < 8; ++nCurrLevel, --nShift)
        {
            idx = (((r >> nShift) << 2) & 0b100) |
                (((g >> nShift) << 1) & 0b010) |
                ((b >> nShift) & 0b001);
            if (!pCurrNode->pChild[idx])
            {
                const auto p = (NODE*)m_MemSet.Allocate(sizeof(NODE));
                pCurrNode->pChild[idx] = p;
                if (!m_bNotInit)
                    ZeroMemory(p, sizeof(NODE));
                if (nCurrLevel == 7)
                {
                    p->bLeaf = TRUE;
                    ++m_cValid;
                    while (m_cValid > m_cMax)
                        ReduceColor();
                }
                else
                {
                    p->pNext = m_apLast[nCurrLevel + 1];
                    m_apLast[nCurrLevel + 1] = p;
                }
                p->bCut = TRUE;

                ++pCurrNode->cChild;
            }
            pCurrNode = pCurrNode->pChild[idx];
            pCurrNode->r += r;
            pCurrNode->g += g;
            pCurrNode->b += b;
            pCurrNode->cColor += 1;
            if (pCurrNode->bLeaf)
                break;
        }
    }

    BOOL GetNearestColor(UINT r, UINT g, UINT b,
        _Out_ UINT& rOut, _Out_ UINT& gOut, _Out_ UINT& bOut) noexcept
    {
        UINT nCurrLevel{}, nShift{ 7 }, idx;
        NODE* p{ &m_Root };
        if (p->bLeaf)
        {
            rOut = p->r / p->cColor;
            gOut = p->g / p->cColor;
            bOut = p->b / p->cColor;
            return TRUE;
        }
        for (; nCurrLevel < 8; ++nCurrLevel, --nShift)
        {
            idx = (((r >> nShift) << 2) & 0b100) |
                (((g >> nShift) << 1) & 0b010) |
                ((b >> nShift) & 0b001);
            p = p->pChild[idx];
            if (!p)
                break;
            if (p->bLeaf)
            {
                rOut = p->r / p->cColor;
                gOut = p->g / p->cColor;
                bOut = p->b / p->cColor;
                return TRUE;
            }
        }
        rOut = gOut = bOut = 0;
        return FALSE;
    }

    // 返回实际颜色数
    UINT GetPalette(_Out_writes_(cMaxColor) UINT* pColors, UINT cMaxColor) noexcept
    {
        std::vector<NODE*> s{};
        s.reserve(256);
        s.push_back(&m_Root);
        UINT c = 0;
        while (!s.empty() && c < cMaxColor)
        {
            const auto p = s.back();
            s.pop_back();
            if (p->bLeaf)
            {
                *pColors++ = ((BYTE(p->r / p->cColor) << 16) |
                    (BYTE(p->g / p->cColor) << 8) |
                    BYTE(p->b / p->cColor));
                ++c;
            }
            else
            {
                for (auto e : p->pChild)
                {
                    if (e)
                        s.push_back(e);
                }
            }
        }
        return c;
    }

    // 减色直到颜色数少于设定的最大值，返回实际颜色数
    UINT Reduce() noexcept
    {
        while (m_cValid > m_cMax)
            ReduceColor();
        return m_cValid;
    }
};
ECK_NAMESPACE_END