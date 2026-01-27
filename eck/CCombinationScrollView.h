#pragma once
#include "CScrollView.h"
#include "ILayout.h"

ECK_NAMESPACE_BEGIN
enum : UINT
{
    CSVIF_FIXED = 1u << 0,
    CSVIF_VIEW_HEIGHT = 1u << 1,
};

class CCombinationScrollView : public CScrollView
{
private:
    struct ITEM
    {
        ILayout* pLayout;	// 可布局对象，可能为nullptr
        IScroll* pScroll;	// 可滚动对象
        UINT uFlags;		// CSVIF_*
        int iPrepending;	// 前导空白
        int iAppending;		// 尾随空白
        BOOL bVisible;		// 缓存是否可见
    };
    std::vector<ITEM> m_vItem{};


    int GetItemViewSize(const ITEM& e) const
    {
        if (e.pScroll)
        {
            if (e.uFlags & CSVIF_FIXED)
                return e.pScroll->ScrGetViewSize();
            else
                return GetViewSize();
        }
        if (e.pLayout)
        {
            const auto size = e.pLayout->LoGetSize();
            if (e.uFlags & CSVIF_VIEW_HEIGHT)
                return size.cy;
            else
                return size.cx;
        }
        ECK_UNREACHABLE;
    }

    void RePosAndReScroll()
    {
        const auto iFirst = GetPosition();
        const auto iLast = iFirst + GetViewSize();
        BOOL bFoundFirst{};
        int n{}, pos;
        for (const auto& e : m_vItem)
        {
            const auto Size = GetItemViewSize(e);
            n += (Size + e.iPrepending + e.iAppending);
            // 找到第一个可见项
            if (n >= iFirst && !bFoundFirst)
            {
                bFoundFirst = TRUE;
                pos = n - Size - e.iAppending - iFirst;
            }
            const auto bVisible = bFoundFirst && (pos + iFirst) < iLast;
            if (e.pLayout)// 可视控制
                e.pLayout->LoShow(bVisible);
            if (bVisible)// 在可视范围
            {
                if (e.uFlags & CSVIF_FIXED)
                {
                    if (e.pLayout)
                    {
                        auto pt = e.pLayout->LoGetPosition();
                        if (e.uFlags & CSVIF_VIEW_HEIGHT)
                            pt.y = pos;
                        else
                            pt.x = pos;
                        e.pLayout->LoSetPosition(pt);
                        e.pLayout->LoCommit();
                    }
                }
                else
                {

                }
                pos += (Size + e.iAppending);
            }
        }
    }
public:
    void Add(ILayout* pLayout, IScroll* pScroll, UINT uFlags = 0u,
        int iPrepending = 0, int iAppending = 0)
    {
        EckAssert(pLayout || pScroll);
        m_vItem.emplace_back(pLayout, pScroll, uFlags, iPrepending, iAppending);
    }

    void ReCalc(int idxBegin = 0)
    {
        int n{};
        for (size_t i = 0; i < m_vItem.size(); ++i)
        {
            auto& e = m_vItem[i];
            n += (GetItemViewSize(e) + e.iPrepending + e.iAppending);
        }
        SetMaximum(n);
    }

    EckInline void Commit() { RePosAndReScroll(); }
};
ECK_NAMESPACE_END