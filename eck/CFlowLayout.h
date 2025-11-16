#pragma once
#include "CLayoutBase.h"

ECK_NAMESPACE_BEGIN
enum :UINT
{
    // 在前方插入换行
    FLF_BREAKLINE = 1u << 15,
};

class CFlowLayoutBase : public CLayoutBase
{
public:
    ECK_RTTI(CFlowLayoutBase);
protected:
    struct ITEM : ITEMBASE
    {
        RCWH rcPos{};

        ITEM() = default;
        constexpr ITEM(ILayout* pCtrl, const MARGINS& Margin,
            UINT uFlags, short cx, short cy) noexcept
            : ITEMBASE{ pCtrl, Margin, uFlags, cx, cy }
        {
        }
    };

    std::vector<ITEM> m_vItem{};
public:
    size_t Add(ILayout* pCtrl,
        const MARGINS& Margin = {}, const UINT uFlags = 0) noexcept
    {
        const auto size = pCtrl->LoGetSize();
        m_vItem.emplace_back(pCtrl, Margin, uFlags, (short)size.cx, (short)size.cy);
        return m_vItem.size() - 1;
    }

    void LoOnDpiChanged(int iDpi) noexcept override
    {
        Refresh();
        for (auto& e : m_vItem)
        {
            ReCalcDpiSize(e, iDpi);
            e.pCtrl->LoOnDpiChanged(iDpi);
        }
        m_iDpi = iDpi;
    }

    void LoInitDpi(int iDpi) noexcept override
    {
        m_iDpi = iDpi;
        for (auto& e : m_vItem)
            e.pCtrl->LoInitDpi(iDpi);
    }

    void Refresh() noexcept override
    {
        for (auto& e : m_vItem)
        {
            const auto size = e.pCtrl->LoGetSize();
            e.cx = (short)size.cx;
            e.cy = (short)size.cy;
        }
    }

    void Clear() noexcept override
    {
        CLayoutBase::Clear();
        m_vItem.clear();
    }

    EckInlineNdCe auto& GetList() noexcept { return m_vItem; }

    void LoShow(BOOL bShow) noexcept override
    {
        for (const auto& e : GetList())
            e.pCtrl->LoShow(bShow);
    }
};
ECK_RTTI_IMPL_BASE_INLINE(CFlowLayoutBase, CLayoutBase);

class CFlowLayoutH final : public CFlowLayoutBase
{
public:
    ECK_RTTI(CFlowLayoutH);
private:
    void MoveCtrl(ITEM& e, HDWP& hDwp, int cyLineMax, int y) noexcept
    {
        if (e.uFlags & LF_FIX_HEIGHT)
            switch (GetSingleAlignFromFlags(e.uFlags))
            {
            case LF_ALIGN_NEAR:
                e.rcPos.y = y;
                break;
            case LF_ALIGN_CENTER:
                e.rcPos.y = y + (cyLineMax -
                    (e.rcPos.cy + e.Margin.cyTopHeight + e.Margin.cyBottomHeight)) / 2;
                break;
            case LF_ALIGN_FAR:
                e.rcPos.y = y + cyLineMax - e.rcPos.cy - e.Margin.cyBottomHeight;
                break;
            default:
                ECK_UNREACHABLE;
            }
        else if (e.uFlags & LF_FILL_HEIGHT)
        {
            e.rcPos.y = y + e.Margin.cyTopHeight;
            e.rcPos.cy = cyLineMax - e.Margin.cyBottomHeight - e.Margin.cyTopHeight;
        }

        if (const auto hWnd = e.pCtrl->LoGetHWND())
        {
            if (e.uFlags & LF_FIX_HEIGHT)
                hDwp = DeferWindowPos(hDwp, hWnd, nullptr, e.rcPos.x, e.rcPos.y, 0, 0,
                    SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);
            else
                hDwp = DeferWindowPos(hDwp, hWnd, nullptr,
                    e.rcPos.x, e.rcPos.y, e.rcPos.cx, e.rcPos.cy,
                    SWP_NOZORDER | SWP_NOACTIVATE);
        }
        else
        {
            if (e.uFlags & LF_FIX_HEIGHT)
                e.pCtrl->LoSetPos(e.rcPos.x, e.rcPos.y);
            else
                e.pCtrl->LoSetPosSize(e.rcPos.x, e.rcPos.y, e.rcPos.cx, e.rcPos.cy);
            e.pCtrl->LoCommit();
        }
    }
public:
    void LoCommit() noexcept override
    {
        HDWP hDwp = PreArrange(m_vItem.size());
        int x = m_x;
        int y = m_y;
        int cxAppr, cyAppr;
        int cyLineMax = 0;
        int idxInLine = 0;
        EckCounter(m_vItem.size(), i)
        {
            auto& e = m_vItem[i];
            if (IsBitSet(e.uFlags, LF_FIX))
            {
                cxAppr = e.cx;
                cyAppr = e.cy;
            }
            else
            {
                const auto size = e.pCtrl->LoGetAppropriateSize();
                cxAppr = (e.uFlags & LF_FIX_WIDTH) ? e.cx : size.cx;
                cyAppr = (e.uFlags & LF_FIX_HEIGHT) ? e.cy : size.cy;
            }

            const auto xt = x + e.Margin.cxLeftWidth + e.Margin.cxRightWidth + cxAppr;
            if (!(e.uFlags & FLF_BREAKLINE) &&
                (xt <= m_x + m_cx || idxInLine == 0/*无论有没有空间，至少要放下一个控件*/))
            {
                const int cyReal = cyAppr + e.Margin.cyTopHeight + e.Margin.cyBottomHeight;
                if (cyReal > cyLineMax)
                    cyLineMax = cyReal;
                e.rcPos.x = x + e.Margin.cxLeftWidth;
                e.rcPos.cx = cxAppr;
                e.rcPos.cy = cyAppr;
                ++idxInLine;
                x = xt;
            }
            else// 开始新行
            {
                // 归位上一行
                for (size_t j = i - idxInLine; j < i; ++j)
                    MoveCtrl(m_vItem[j], hDwp, cyLineMax, y);

                // 重置行参数
                x = m_x;
                y += cyLineMax;
                cyLineMax = 0;
                idxInLine = 0;

                // 本行第一个控件
                const int cyReal = cyAppr + e.Margin.cyTopHeight + e.Margin.cyBottomHeight;
                if (cyReal > cyLineMax)
                    cyLineMax = cyReal;
                e.rcPos.x = x + e.Margin.cxLeftWidth;
                e.rcPos.cx = cxAppr;
                e.rcPos.cy = cyAppr;
                ++idxInLine;
                x += (e.Margin.cxLeftWidth + e.Margin.cxRightWidth + cxAppr);
            }
        }
        for (size_t j = m_vItem.size() - idxInLine; j < m_vItem.size(); ++j)
            MoveCtrl(m_vItem[j], hDwp, cyLineMax, y);
        PostArrange(hDwp);
    }

    SIZE LoGetAppropriateSize() noexcept override
    {
        int cx{}, cy{};
        for (const auto& e : m_vItem)
        {
            cx += e.cx + e.Margin.cxLeftWidth + e.Margin.cxRightWidth;
            cy = std::max(cy, e.cy + e.Margin.cyTopHeight + e.Margin.cyBottomHeight);
        }
        return { cx,cy };
    }
};
ECK_RTTI_IMPL_BASE_INLINE(CFlowLayoutH, CFlowLayoutBase);

class CFlowLayoutV final :public CFlowLayoutBase
{
public:
    ECK_RTTI(CFlowLayoutV);
private:
    void MoveCtrl(ITEM& e, HDWP& hDwp, int cxLineMax, int x) noexcept
    {
        if (e.uFlags & LF_FIX_WIDTH)
            switch (GetSingleAlignFromFlags(e.uFlags))
            {
            case LF_ALIGN_NEAR:
                e.rcPos.x = x;
                break;
            case LF_ALIGN_CENTER:
                e.rcPos.x = x + (cxLineMax - (e.rcPos.cx + e.Margin.cxLeftWidth + e.Margin.cxRightWidth)) / 2;
                break;
            case LF_ALIGN_FAR:
                e.rcPos.x = x + cxLineMax - e.rcPos.cx - e.Margin.cxRightWidth;
                break;
            default:
                ECK_UNREACHABLE;
            }
        else if (e.uFlags & LF_FILL_WIDTH)
        {
            e.rcPos.x = x + e.Margin.cxLeftWidth;
            e.rcPos.cx = cxLineMax - e.Margin.cxLeftWidth - e.Margin.cxRightWidth;
        }

        if (const auto hWnd = e.pCtrl->LoGetHWND())
        {
            if (e.uFlags & LF_FIX_WIDTH)
                hDwp = DeferWindowPos(hDwp, hWnd, nullptr, e.rcPos.x, e.rcPos.y, 0, 0,
                    SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);
            else
                hDwp = DeferWindowPos(hDwp, hWnd, nullptr, e.rcPos.x, e.rcPos.y, e.rcPos.cx, e.rcPos.cy,
                    SWP_NOZORDER | SWP_NOACTIVATE);
        }
        else
        {
            if (e.uFlags & LF_FIX_WIDTH)
                e.pCtrl->LoSetPos(e.rcPos.x, e.rcPos.y);
            else
                e.pCtrl->LoSetPosSize(e.rcPos.x, e.rcPos.y, e.rcPos.cx, e.rcPos.cy);
            e.pCtrl->LoCommit();
        }
    }
public:
    void LoCommit() noexcept override
    {
        HDWP hDwp = PreArrange(m_vItem.size());
        int x = m_x;
        int y = m_y;
        int cxAppr, cyAppr;
        int cxLineMax = 0;
        int idxInLine = 0;
        EckCounter(m_vItem.size(), i)
        {
            auto& e = m_vItem[i];
            if (IsBitSet(e.uFlags, LF_FIX))
            {
                cxAppr = e.cx;
                cyAppr = e.cy;
            }
            else
            {
                const auto size = e.pCtrl->LoGetAppropriateSize();
                cxAppr = (e.uFlags & LF_FIX_WIDTH) ? e.cx : size.cx;
                cyAppr = (e.uFlags & LF_FIX_HEIGHT) ? e.cy : size.cy;
            }

            const auto yt = y + e.Margin.cyTopHeight + e.Margin.cyBottomHeight + cyAppr;
            if (!(e.uFlags & FLF_BREAKLINE) &&
                (yt <= m_y + m_cy || idxInLine == 0/*无论有没有空间，至少要放下一个控件*/))
            {
                const int cxReal = cxAppr + e.Margin.cxLeftWidth + e.Margin.cxRightWidth;
                if (cxReal > cxLineMax)
                    cxLineMax = cxReal;
                e.rcPos.y = y + e.Margin.cyTopHeight;
                e.rcPos.cy = cyAppr;
                e.rcPos.cx = cxAppr;
                ++idxInLine;
                y = yt;
            }
            else// 开始新行
            {
                // 归位上一行
                for (size_t j = i - idxInLine; j < i; ++j)
                    MoveCtrl(m_vItem[j], hDwp, cxLineMax, x);

                // 重置行参数
                y = m_y;
                x += cxLineMax;
                cxLineMax = 0;
                idxInLine = 0;

                // 本行第一个控件
                const int cxReal = cxAppr + e.Margin.cxLeftWidth + e.Margin.cxRightWidth;
                if (cxReal > cxLineMax)
                    cxLineMax = cxReal;
                e.rcPos.y = y + e.Margin.cyTopHeight;
                e.rcPos.cy = cyAppr;
                e.rcPos.cx = cxAppr;
                ++idxInLine;
                y += (e.Margin.cyTopHeight + e.Margin.cyBottomHeight + cyAppr);
            }
        }
        for (size_t j = m_vItem.size() - idxInLine; j < m_vItem.size(); ++j)
            MoveCtrl(m_vItem[j], hDwp, cxLineMax, x);
        PostArrange(hDwp);
    }

    SIZE LoGetAppropriateSize() noexcept override
    {
        int cx{}, cy{};
        for (const auto& e : m_vItem)
        {
            cx = std::max(cx, e.cx + e.Margin.cxLeftWidth + e.Margin.cxRightWidth);
            cy += e.cy + e.Margin.cyTopHeight + e.Margin.cyBottomHeight;
        }
        return { cx,cy };
    }
};
ECK_RTTI_IMPL_BASE_INLINE(CFlowLayoutV, CFlowLayoutBase);
ECK_NAMESPACE_END