#pragma once
#include "ILayout.h"
#include "Utility.h"
#include "WndHelper.h"

ECK_NAMESPACE_BEGIN
// 通用布局选项，低16位
// 对齐选项：所有布局器要么支持九向对齐，要么支持线对齐
// 其他选项：取决于布局类型
enum : UINT
{
    // 以下为对齐选项

    // 九向对齐，低4位
    LF_ALIGN_LT,// 左上
    LF_ALIGN_T,	// 上
    LF_ALIGN_RT,// 右上
    LF_ALIGN_L,	// 左
    LF_ALIGN_C,	// 中
    LF_ALIGN_R,	// 右
    LF_ALIGN_LB,// 左下
    LF_ALIGN_B,	// 下
    LF_ALIGN_RB,// 右下

    // 线对齐
    LF_ALIGN_NEAR = 0,
    LF_ALIGN_CENTER,
    LF_ALIGN_FAR,

    // 以下为空间分配选项，若不指定任何选项则为FLEX，对象可被任意伸缩而无额外约束
    // FILL、SCALE、FIX、IDEAL互斥

    // 类似于FLEX，但考虑理想尺寸。
    // 若空间充足则占据剩余空间，若不足则缩小，剩余空间按权重分配
    LF_FILL_WIDTH = 1u << 4, LF_FILL_HEIGHT = 1u << 5,
    LF_FILL = LF_FILL_WIDTH | LF_FILL_HEIGHT,   // 【复合】

    // 类似于FLEX，但对象被等宽高比缩放以适应，必须提供有效宽高
    LF_SCALE = 1u << 6,

    // 对象尺寸不变
    LF_FIX_WIDTH = 1u << 7, LF_FIX_HEIGHT = 1u << 8,
    LF_FIX = LF_FIX_WIDTH | LF_FIX_HEIGHT,      // 【复合】

    // 使用理想尺寸，但必要时仍减少尺寸
    LF_IDEAL_WIDTH = 1u << 9, LF_IDEAL_HEIGHT = 1u << 10,
    LF_IDEAL = LF_IDEAL_WIDTH | LF_IDEAL_HEIGHT,// 【复合】
};

class __declspec(novtable) CLayoutBase : public ILayout
{
public:
    ECK_RTTI(CLayoutBase, ILayout);
protected:
    struct ITEMBASE
    {
        ILayout* pObject;
        LYTMARGINS Margin;
        UINT uFlags;
        TLytCoord cx;
        TLytCoord cy;
    };

    TLytCoord m_x{}, m_y{}, m_cx{}, m_cy{};
    int m_iDpi{ USER_DEFAULT_SCREEN_DPI };
    BOOL m_bUseDwp{ TRUE };
    HDWP m_hDwpCurrent{};

    static void ArgCalculateRect(const ITEMBASE& e,
        const LYTRECT& rcCell, _Out_ LYTRECT& rc) noexcept
    {
        if (e.uFlags & LF_SCALE)
        {
            rc = { 0, 0, e.cx, e.cy };
            const LYTRECT rcRef
            {
                rcCell.x + e.Margin.l,
                rcCell.y + e.Margin.t,
                rcCell.x + rcCell.cx - e.Margin.r,
                rcCell.y + rcCell.cy - e.Margin.b
            };
            AdjustRectToFitAnother(rc, rcRef);
            return;
        }

        LYTSIZE size{};
        if (e.uFlags & LF_IDEAL)
            e.pObject->LoGetIdealSize(size);

        if (e.uFlags & LF_IDEAL_WIDTH)
            rc.cx = size.cx;
        else if (e.uFlags & LF_FIX_WIDTH)
            rc.cx = e.cx;
        else
            rc.cx = rcCell.cx - e.Margin.l - e.Margin.r;

        if (e.uFlags & LF_IDEAL_HEIGHT)
            rc.cy = size.cy;
        else if (e.uFlags & LF_FIX_HEIGHT)
            rc.cy = e.cy;
        else
            rc.cy = rcCell.cy - e.Margin.t - e.Margin.b;

        const auto cx1 = rc.cx + e.Margin.l + e.Margin.r;
        const auto cy1 = rc.cy + e.Margin.t + e.Margin.b;
        switch (Lf9Align(e.uFlags))
        {
        case LF_ALIGN_LT:
            rc.x = rcCell.x + e.Margin.l;
            rc.y = rcCell.y + e.Margin.t;
            break;
        case LF_ALIGN_T:
            rc.x = rcCell.x + (rcCell.cx - cx1) / 2 + e.Margin.l;
            rc.y = rcCell.y + e.Margin.t;
            break;
        case LF_ALIGN_RT:
            rc.x = rcCell.x + rcCell.cx - rc.cx - e.Margin.r;
            rc.y = rcCell.y + e.Margin.t;
            break;
        case LF_ALIGN_L:
            rc.x = rcCell.x + e.Margin.l;
            rc.y = rcCell.y + (rcCell.cy - (rc.cy + e.Margin.t + e.Margin.b)) / 2 + e.Margin.t;
            break;
        case LF_ALIGN_C:
            rc.x = rcCell.x + (rcCell.cx - cx1) / 2 + e.Margin.l;
            rc.y = rcCell.y + (rcCell.cy - (rc.cy + e.Margin.t + e.Margin.b)) / 2 + e.Margin.t;
            break;
        case LF_ALIGN_R:
            rc.x = rcCell.x + rcCell.cx - rc.cx - e.Margin.r;
            rc.y = rcCell.y + (rcCell.cy - (rc.cy + e.Margin.t + e.Margin.b)) / 2 + e.Margin.t;
            break;
        case LF_ALIGN_LB:
            rc.x = rcCell.x + e.Margin.l;
            rc.y = rcCell.y + rcCell.cy - rc.cy - e.Margin.b;
            break;
        case LF_ALIGN_B:
            rc.x = rcCell.x + (rcCell.cx - cx1) / 2 + e.Margin.l;
            rc.y = rcCell.y + rcCell.cy - rc.cy - e.Margin.b;
            break;
        case LF_ALIGN_RB:
            rc.x = rcCell.x + rcCell.cx - rc.cx - e.Margin.r;
            rc.y = rcCell.y + rcCell.cy - rc.cy - e.Margin.b;
            break;
        default:
            ECK_UNREACHABLE;
        }
    }

    static TLytCoord ArgCalculateHorizontalAlignment(
        TLytCoord xStart, TLytCoord cxCell,
        const ITEMBASE& e, TLytCoord cxObject) noexcept
    {
        switch (LfLineAlign(e.uFlags))
        {
        case LF_ALIGN_NEAR:
            return xStart + e.Margin.l;
        case LF_ALIGN_CENTER:
            return xStart + e.Margin.l +
                (cxCell - (cxObject + e.Margin.l + e.Margin.r)) / 2;
        case LF_ALIGN_FAR:
            return xStart + cxCell - cxObject - e.Margin.r;
        default:
            ECK_UNREACHABLE;
        }
    }
    static TLytCoord ArgCalculateVerticalAlignment(
        TLytCoord yStart, TLytCoord cyCell,
        const ITEMBASE& e, TLytCoord cyObject) noexcept
    {
        switch (LfLineAlign(e.uFlags))
        {
        case LF_ALIGN_NEAR:
            return yStart + e.Margin.t;
        case LF_ALIGN_CENTER:
            return yStart + e.Margin.t +
                (cyCell - (cyObject + e.Margin.t + e.Margin.b)) / 2;
        case LF_ALIGN_FAR:
            return yStart + cyCell - cyObject - e.Margin.b;
        default:
            ECK_UNREACHABLE;
        }
    }

    // 必须在放置结束时调用ArgEnd
    void ArgMoveObject(const ITEMBASE& e, const LYTRECT& rc) noexcept
    {
        const auto hWnd = e.pObject->LoGetHWND();
        if (hWnd && m_hDwpCurrent)
        {
            if (IsBitSet(e.uFlags, LF_FIX))
                m_hDwpCurrent = DeferWindowPos(m_hDwpCurrent, hWnd, nullptr,
                    int(rc.x + 0.5f), int(rc.y + 0.5f),
                    0, 0,
                    SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
            else
                m_hDwpCurrent = DeferWindowPos(m_hDwpCurrent, hWnd, nullptr,
                    int(rc.x + 0.5f), int(rc.y + 0.5f),
                    int(rc.cx + 0.5f), int(rc.cy + 0.5f),
                    SWP_NOACTIVATE | SWP_NOZORDER);
        }
        else
        {
            const auto bDwp = e.pObject->LoDwpAttach(m_hDwpCurrent);
            if (IsBitSet(e.uFlags, LF_FIX))
                e.pObject->LoSetPosition({ rc.x, rc.y });
            else
                e.pObject->LoSetRect({ rc.x, rc.y, rc.cx, rc.cy });
            e.pObject->LoCommit();
            if (bDwp)
                m_hDwpCurrent = e.pObject->LoDwpDetach();
        }
    }

    void ReCalculateDpiSize(ITEMBASE& e, int iDpiNew) noexcept
    {
        e.Margin.l = DpiScale(e.Margin.l, iDpiNew, m_iDpi);
        e.Margin.t = DpiScale(e.Margin.t, iDpiNew, m_iDpi);
        e.Margin.r = DpiScale(e.Margin.r, iDpiNew, m_iDpi);
        e.Margin.b = DpiScale(e.Margin.b, iDpiNew, m_iDpi);
        if (e.uFlags & LF_FIX)
        {
            e.cx = DpiScale(e.cx, iDpiNew, m_iDpi);
            e.cy = DpiScale(e.cy, iDpiNew, m_iDpi);
            e.pObject->LoSetSize({ e.cx, e.cy });
        }
    }
public:
    EckInlineNdCe static UINT Lf9Align(UINT u) noexcept { return GetLowNBits(u, 4); }
    EckInlineNdCe static UINT LfLineAlign(UINT u) noexcept { return GetLowNBits(u, 2); }
    // 互斥验证，若标志无效则返回FALSE
    EckInlineNdCe static BOOL LfValidateFlags(UINT u) noexcept
    {
        constexpr UINT WIDTH_EXCLUSIVE_MASK =
            LF_FILL_WIDTH | LF_FIX_WIDTH | LF_IDEAL_WIDTH | LF_SCALE;
        constexpr UINT HEIGHT_EXCLUSIVE_MASK =
            LF_FILL_HEIGHT | LF_FIX_HEIGHT | LF_IDEAL_HEIGHT | LF_SCALE;
        const auto u1 = u & WIDTH_EXCLUSIVE_MASK;
        if (u1 && (u1 & (u1 - 1)))
            return FALSE;
        const auto u2 = u & HEIGHT_EXCLUSIVE_MASK;
        if (u2 && (u2 & (u2 - 1)))
            return FALSE;
        return TRUE;
    }

    void LoSetPosition(LYTPOINT pt) noexcept override
    {
        m_x = pt.x;
        m_y = pt.y;
    }

    void LoSetSize(LYTSIZE size) noexcept override
    {
        m_cx = size.cx;
        m_cy = size.cy;
    }

    void LoSetRect(const LYTRECT& rc) noexcept override
    {
        m_x = rc.x;
        m_y = rc.y;
        m_cx = rc.cx;
        m_cy = rc.cy;
    }

    LYTPOINT LoGetPosition() noexcept override { return { m_x, m_y }; }
    LYTSIZE LoGetSize() noexcept override { return { m_cx, m_cy }; }

    BOOL LoDwpAttach(HDWP hDwp) noexcept override
    {
        if (!m_bUseDwp)
            return FALSE;
        if (m_hDwpCurrent)
        {
            EckDbgBreak();
            EndDeferWindowPos(m_hDwpCurrent);
        }
        m_hDwpCurrent = hDwp;
        return !!hDwp;
    }
    HDWP LoDwpDetach() noexcept override
    {
        const auto h = m_hDwpCurrent;
        m_hDwpCurrent = nullptr;
        return h;
    }

    // 清空对象的状态，包括已加入的对象等
    virtual void LobClear() noexcept
    {
        m_x = m_y = m_cx = m_cy = 0;
        m_bUseDwp = TRUE;
    }

    // 若调用方已通过操作底层数据结构的方式修改了对象列表，则必须调用此函数刷新
    virtual void LobRefresh() noexcept {}

    virtual size_t LobGetObjectCount() const noexcept = 0;

    EckInline void Arrange(TLytCoord x, TLytCoord y, TLytCoord cx, TLytCoord cy) noexcept
    {
        CLayoutBase::LoSetRect({ x, y, cx, cy });
        LoCommit();
    }

    EckInline void Arrange(TLytCoord cx, TLytCoord cy) noexcept { Arrange(0, 0, cx, cy); }

    EckInlineCe void SetUseDwp(BOOL b) noexcept { m_bUseDwp = b; }
    EckInlineNdCe BOOL GetUseDwp() const noexcept { return m_bUseDwp; }

    EckInlineNdCe int GetDpi() const noexcept { return m_iDpi; }
};
ECK_NAMESPACE_END