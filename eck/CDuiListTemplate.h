#pragma once
#include "CDuiScrollBar.h"
#include "CDuiHeader.h"
#include "CD2DImageList.h"

ECK_NAMESPACE_BEGIN
ECK_DUI_NAMESPACE_BEGIN
struct NMLTITEMINDEX : DUINMHDR
{
    int idx;
    int idxGroup;
};

struct NMLTITEMCHANGE : NMLTITEMINDEX
{
    UINT uFlagsOld;
    UINT uFlagsNew;
};

struct NMLTHOTITEMCHANGE : NMLTITEMINDEX
{
    int idxOld;
    int idxGroupOld;
};

struct NMLTSCROLLED : DUINMHDR
{
    int idxBegin;
    int idxGroupBegin;
    int idxEnd;
    int idxGroupEnd;
};

struct NMLTCUSTOMDRAW : NMECUSTOMDRAW
{
    BITBOOL bColorText : 1;
    int idxSub;
    int idxGroup;
    D2D1_COLOR_F crText;
};

struct LE_HITTEST
{
    POINT pt;
    int idxGroup;
    BOOLEAN bHitGroupHeader;
    BOOLEAN bHitGroupImage;
};

// 项目标志
enum
{
    LEIF_SELECTED = (1u << 0),
};


class CListTemplate : public CElem
{
public:
    enum class Type : BYTE
    {
        List,
        Icon,
        Report,
    };
    constexpr static float
        CyInsertMark = 3,
        CyDefHeader = 30,
        CyGroupLine = 1;
    enum class ListPart
    {
        GroupHeader,
        GroupText,
        Item,
        GroupImg,
    };
protected:
    struct SUBITEM
    {
        ComPtr<IDWriteTextLayout> pLayout{};
    };
    struct ITEM
    {
        ComPtr<IDWriteTextLayout> pLayout{};
        UINT uFlags{};
        float y{};
        std::vector<SUBITEM> vSubItem{};
    };
    struct GROUP
    {
        ComPtr<IDWriteTextLayout> pLayout{};
        UINT uFlags{};
        float y{};
        std::vector<ITEM> Item{};
    };

    std::vector<ITEM> m_vItem{};// 项目
    std::vector<GROUP> m_Group{};// 组
    CScrollBar m_SBV{}, m_SBH{};
    CHeader m_Header{};
    ID2D1SolidColorBrush* m_pBrush{};
    CInertialScrollView* m_psvV{};
    CInertialScrollView* m_psvH{};
    CD2DImageList* m_pImgList{};
    CD2DImageList* m_pImgListGroup{};
    IDWriteTextFormat* m_pTfGroup{};
    //---通用
    int m_idxHot{ -1 };			// 热点项
    int m_idxSel{ -1 };			// 选中的项的索引，仅用于单选
    int m_idxInsertMark{ -1 };	// 插入标记应当显示在哪一项之前
    int m_idxFocus{ -1 };		// 焦点项
    int m_idxMark{ -1 };		// 标记项

    float m_cyTopExtra{};			// 顶部空白
    float m_cyBottomExtra{};		// 底部空白

    float m_cyItem{ 40 };			// 项目高度
    float m_cyPadding{ 3 };		// 项目间距

    float m_oyTopItem{};			// 小于等于零的值，指示第一可见项的遮挡高度
    int m_idxTop{};				// 第一个可见项
    //---分组模式
    int m_idxHotItemGroup{ -1 };	// 热点项所在的组
    int m_idxSelItemGroup{ -1 };	// 选中项所在的组
    int m_idxInsertMarkGroup{ -1 };	// 插入标记所在的组
    int m_idxFocusItemGroup{ -1 };	// 焦点项所在的组
    int m_idxMarkItemGroup{ -1 };	// 标记项所在的组
    float m_cyGroupHeader{ 40 };	// 组头高度
    int m_idxTopGroup{};		// 第一个可见组
    int m_idxHotGroup{ -1 };	// 热点组
    float m_cxGroupImage{};		// 组图片宽度
    float m_cyGroupImage{};		// 组图片高度
    float m_cyGroupItemTopPadding{};		// 项目区顶部空白
    float m_cyGroupItemBottomPadding{};	// 项目区底部空白
    //---图标模式
    float m_cxItem{ 40 };			// 项目宽度
    float m_cxPadding{ 3 };		// 水平项目间距
    int m_cItemPerRow{};		// 每行项目数
    //---
    POINT m_ptDragSelStart{};	// 拖动选择起始点
    D2D1_RECT_F m_rcDragSel{};	// 当前选择矩形
    float m_dCursorToItemMax{};	// 鼠标指针到项目的最大距离

    Type m_eView{ Type::List };	// 视图类型
    BYTE m_eInterMode{ D2D1_INTERPOLATION_MODE_LINEAR };

    BITBOOL m_bSingleSel : 1{};	// 单选
    BITBOOL m_bGroup : 1{};		// 分组
    BITBOOL m_bGroupImage : 1{};// 显示组图片
    BITBOOL m_bItemNotify : 1{};// 启用项目通知
    BITBOOL m_bToggleSel : 1{};	// 左键按下时切换项目选中
    BITBOOL m_bEnableDragSel : 1{ TRUE };	// 启用拖动选择
    BITBOOL m_bDeSelInSpace : 1{ TRUE };	// 点击空白处时取消所有选中
    BITBOOL m_bDbgIndex : 1{};	// 显示索引
    BITBOOL m_bCustomDraw : 1{};// 自定义绘制

    BITBOOL m_bDraggingSel : 1{};	// 正在拖动选择


    virtual void GRPaintGroup(const D2D1_RECT_F& rcPaint, NMLTCUSTOMDRAW& nm, LRESULT r) {}

    virtual void LVPaintSubItem(const D2D1_RECT_F& rcPaint, NMLTCUSTOMDRAW& nm, LRESULT r) {}

    virtual void LVPaintItem(const D2D1_RECT_F& rcPaint, NMLTCUSTOMDRAW& nm, LRESULT r)
    {
        nm.idxSub = 0;
        nm.bColorText = FALSE;
        if (m_bGroup)
        {
            GetGroupPartRect(ListPart::Item, nm.idx, nm.idxGroup, nm.rc);
            if ((m_Group[nm.idxGroup].Item[nm.idx].uFlags & LEIF_SELECTED) ||
                (m_bSingleSel && m_idxSel == nm.idx && m_idxSelItemGroup == nm.idxGroup))
                if (nm.idx == m_idxHot && nm.idxGroup == m_idxHotItemGroup)
                    nm.eState = State::HotSelected;
                else
                    nm.eState = State::Selected;
            else if (nm.idx == m_idxHot && nm.idxGroup == m_idxHotItemGroup)
                nm.eState = State::Hot;
            else
                nm.eState = State::None;
            if (r & CDRF_NOTIFYITEMDRAW)
            {
                nm.dwStage = CDDS_ITEMPREPAINT;
                r = GenElemNotify(&nm);
            }
            if (!(r & CDRF_SKIPDEFAULT))
            {
                if (nm.eState != State::None)
                    GetTheme()->DrawBackground(Part::ListItem,
                        nm.eState, nm.rc, nullptr);
                GetGroupSubItemRect(nm.idx, 0, nm.idxGroup, nm.rc);
                LVPaintSubItem(rcPaint, nm, r);
                if (m_eView == Type::Report)
                    for (nm.idxSub = 1; nm.idxSub < m_Header.GetItemCount(); ++nm.idxSub)
                    {
                        GetGroupSubItemRect(nm.idx, nm.idxSub, nm.idxGroup, nm.rc);
                        LVPaintSubItem(rcPaint, nm, r);
                    }
            }
        }
        else
        {
            GetItemRect(nm.idx, nm.rc);
            if ((m_vItem[nm.idx].uFlags & LEIF_SELECTED) ||
                (m_bSingleSel && m_idxSel == nm.idx))
                if (m_idxHot == nm.idx)
                    nm.eState = State::HotSelected;
                else
                    nm.eState = State::Selected;
            else if (m_idxHot == nm.idx)
                nm.eState = State::Hot;
            else
                nm.eState = State::None;
            if (r & CDRF_NOTIFYITEMDRAW)
            {
                nm.dwStage = CDDS_ITEMPREPAINT;
                r = GenElemNotify(&nm);
            }
            if (!(r & CDRF_SKIPDEFAULT))
            {
                if (nm.eState != State::None)
                    GetTheme()->DrawBackground(Part::ListItem,
                        nm.eState, nm.rc, nullptr);
                GetSubItemRect(nm.idx, 0, nm.rc);
                LVPaintSubItem(rcPaint, nm, r);
#if _DEBUG
                if (m_bDbgIndex)
                {
                    CRefStrW rs{};
                    rs.Format(L"%d", nm.idx);
                    if (GetTextFormat())
                        m_pDC->DrawTextW(rs.Data(), rs.Size(),
                            GetTextFormat(), nm.rc, m_pBrush);
                }
#endif// _DEBUG
                if (m_eView == Type::Report)
                    for (nm.idxSub = 1; nm.idxSub < m_Header.GetItemCount(); ++nm.idxSub)
                    {
                        GetSubItemRect(nm.idx, nm.idxSub, nm.rc);
                        LVPaintSubItem(rcPaint, nm, r);
                    }
            }
        }
    }

    virtual void IVPaintItem(const D2D1_RECT_F& rcPaint, NMLTCUSTOMDRAW& nm, LRESULT r) {}

    virtual void PostPaint(ELEMPAINTSTRU& ps) {}

    void ReCalcHScroll()
    {
        if (m_eView == Type::Report)
        {
            m_psvH->SetPage(GetWidthF());
            m_psvH->SetRange(0, m_Header.GetContentWidth() + GetRealGroupImageWidth());
            m_SBH.SetVisible(m_psvH->IsVisible());
        }
    }

    void ReCalcScroll()
    {
        if (!m_cyItem || !m_cxItem)
            return;
        ReCalcHScroll();
        m_psvV->SetPage(GetHeightF());
        if (m_bGroup)
            return;
        switch (m_eView)
        {
        case Type::List:
        case Type::Report:
        {
            m_psvV->SetRange(-m_cyTopExtra, GetItemCount() *
                (m_cyItem + m_cyPadding) + m_cyBottomExtra);
        }
        break;
        case Type::Icon:
        {
            m_cItemPerRow = int((GetWidthF() + m_cxPadding) / (m_cxItem + m_cxPadding));
            const int cItemV = (GetItemCount() - 1) / m_cItemPerRow + 1;
            m_psvV->SetRange(-m_cyTopExtra, cItemV *
                (m_cyItem + m_cyPadding) + m_cyBottomExtra);
        }
        break;
        default:
            ECK_UNREACHABLE;
        }
    }

    void ReCalculateTopItem()
    {
        if (!m_cyItem || !m_cxItem)
            return;
        switch (m_eView)
        {
        case Type::List:
        case Type::Report:
        {
            if (m_bGroup)
            {
                if (m_Group.empty())
                {
                    m_idxTopGroup = m_idxTop = 0;
                    return;
                }
                const auto it = std::lower_bound(m_Group.begin(), m_Group.end(),
                    m_psvV->GetPosition(), [](const GROUP& x, float iPos)
                    {
                        return x.y < iPos;
                    });
                EckAssert(it != m_Group.end());
                if (it == m_Group.begin())
                    m_idxTopGroup = 0;
                else
                    m_idxTopGroup = (int)std::distance(m_Group.begin(), it - 1);
                EckDbgPrint(m_idxTopGroup);
                const auto& e = m_Group[m_idxTopGroup];
                m_oyTopItem = e.y - m_psvV->GetPosition();
                m_idxTop = int((m_psvV->GetPosition() -
                    (e.y + m_cyGroupHeader + m_cyGroupItemTopPadding)) / m_cyItem);
                if (m_idxTop < 0)
                    m_idxTop = 0;
            }
            else
            {
                m_idxTop = int(m_psvV->GetPosition() / (m_cyItem + m_cyPadding));
                m_oyTopItem = m_idxTop * (m_cyItem + m_cyPadding) - m_psvV->GetPosition();
            }
        }
        break;
        case Type::Icon:
        {
            const int cItemV = int(m_psvV->GetPosition() / (m_cyItem + m_cyPadding));
            m_idxTop = cItemV * m_cItemPerRow;
            m_oyTopItem = cItemV * (m_cyItem + m_cyPadding) - m_psvV->GetPosition();
        }
        break;
        default: ECK_UNREACHABLE;
        }
    }

    void ArrangeHeader(BOOL bRePos = FALSE)
    {
        auto cxContent = m_Header.GetContentWidth();
        cxContent = std::max(cxContent, GetWidthF());
        if (bRePos)
        {
            D2D1_RECT_F rc;
            rc.left = -m_psvH->GetPosition();
            if (m_bGroup && m_bGroupImage)
                rc.left += m_cxGroupImage;
            rc.right = rc.left + cxContent;
            rc.top = 0;
            rc.bottom = m_Header.GetHeightF();
            m_Header.SetRect(rc);
        }
        else
        {
            if (m_Header.GetWidthF() != cxContent)
                m_Header.SetSize(cxContent, m_Header.GetHeightF());
        }
    }

    EckInlineNdCe float GetRealGroupImageWidth() const
    {
        return (m_bGroupImage && m_bGroup) ? m_cxGroupImage : 0;
    }

    // 从X坐标计算逻辑项目索引，索引相对当前可见范围
    EckInline int IVLogItemFromX(float x) const
    {
        EckAssert(m_eView == Type::Icon);
        return int(x / (m_cxItem + m_cxPadding));
    }
    // 从Y坐标计算逻辑项目索引，索引相对当前可见范围
    // m_idxTop所在行的下一行记为0，上一行记为-1
    EckInline int IVLogItemFromY(float y) const
    {
        EckAssert(m_eView == Type::Icon);
        const auto i = int((y - m_oyTopItem) / (m_cyItem + m_cyPadding));
        return (y - m_oyTopItem < 0) ? i - 1 : i;
    }

    EckInline std::pair<float, float> IVGetItemXY(int idx) const
    {
        EckAssert(m_eView == Type::Icon);
        const int idxV = (idx - m_idxTop) / m_cItemPerRow;
        return
        {
            (abs(idx - m_idxTop) % m_cItemPerRow) * (m_cxItem + m_cxPadding),
            m_oyTopItem + idxV * (m_cyItem + m_cyPadding)
        };
    }

    // 由索引得到Y坐标
    EckInline float LVGetItemY(int idx) const
    {
        EckAssert(m_eView == Type::List || m_eView == Type::Report);
        return m_oyTopItem + (idx - m_idxTop) * (m_cyItem + m_cyPadding);
    }
    // 由Y坐标得到索引
    EckInline int LVItemFromY(float y) const
    {
        EckAssert(m_eView == Type::List || m_eView == Type::Report);
        return m_idxTop + int((y - m_oyTopItem) / (m_cyItem + m_cyPadding));
    }

    void CalcItemRangeInRect(const D2D1_RECT_F& rc, _Out_ int& idxBegin, _Out_ int& idxEnd)
    {
        if (!GetItemCount())
        {
            idxBegin = idxEnd = -1;
            return;
        }
        switch (m_eView)
        {
        case Type::List:
        case Type::Report:
            idxBegin = LVItemFromY(rc.top);
            idxBegin = std::clamp(idxBegin, 0, GetItemCount() - 1);
            idxEnd = LVItemFromY(rc.bottom);
            idxEnd = std::clamp(idxEnd, 0, GetItemCount() - 1);
            break;
        case Type::Icon:
        {
            int idxX = IVLogItemFromX(rc.left);
            idxX = std::clamp(idxX, 0, m_cItemPerRow - 1);
            int idxY = IVLogItemFromY(rc.top);
            idxBegin = m_idxTop + idxX + idxY * m_cItemPerRow;

            idxX = IVLogItemFromX(rc.right);
            idxX = std::clamp(idxX, 0, m_cItemPerRow - 1);
            idxY = IVLogItemFromY(rc.bottom);
            idxEnd = m_idxTop + idxX + idxY * m_cItemPerRow;
        }
        break;
        default: ECK_UNREACHABLE;
        }
    }

    // 由Y坐标得到组，坐标相对元素
    int GRGroupFromY(float y, _Out_ int& idxItemInGroup) const
    {
        y += m_psvV->GetPosition();
        auto it = std::lower_bound(m_Group.begin(), m_Group.end(), y,
            [](const GROUP& x, float iPos)
            {
                return x.y < iPos;
            });
        if (it == m_Group.begin())
            ;
        else if (it == m_Group.end())
            it = (m_Group.rbegin() + 1).base();
        else
            --it;
        int idxGroup = (int)std::distance(m_Group.begin(), it);
        const float yGroupBottom = it->y +
            m_cyGroupHeader + m_cyGroupItemTopPadding;
        const float yInItem = y - yGroupBottom;
        idxItemInGroup = int(yInItem / (m_cyItem + m_cyPadding));
        if (idxItemInGroup < 0)
            idxItemInGroup = 0;
        else if (idxItemInGroup >= (int)it->Item.size())
        {
            if (idxGroup + 1 >= GetGroupCount())
                idxItemInGroup = (int)it->Item.size() - 1;
            else
            {
                ++idxGroup;
                idxItemInGroup = 0;
            }
        }
        return idxGroup;
    }

    void CalcItemRangeInRect(const D2D1_RECT_F& rc, _Out_ int& idxBegin, _Out_ int& idxEnd,
        _Out_ int& idxGroupBegin, _Out_ int& idxGroupEnd)
    {
        idxGroupBegin = GRGroupFromY(rc.top, idxBegin);
        idxGroupEnd = GRGroupFromY(rc.bottom, idxEnd);
    }

    void GRDragSelMouseMove(POINT pt, WPARAM wParam)
    {
        const auto dy = m_psvV->GetPosition();
        EckAssert(m_bDraggingSel);
        D2D1_RECT_F rcOld{ m_rcDragSel };
        OffsetRect(rcOld, 0, -dy);

        m_rcDragSel = MakeD2DRectF(MakeRect(pt, POINT{
            m_ptDragSelStart.x,
            m_ptDragSelStart.y - (int)dy }));

        int idxBegin, idxEnd, idxGroupBegin, idxGroupEnd;
        D2D1_RECT_F rcItem;
        D2D1_RECT_F rcJudge;
        UnionRect(rcJudge, rcOld, m_rcDragSel);
        if (IsRectEmpty(rcJudge))
            goto Skip;

        CalcItemRangeInRect(rcJudge, idxBegin, idxEnd, idxGroupBegin, idxGroupEnd);
        if (idxBegin < 0 || idxEnd < 0)
            goto Skip;
        if (idxGroupBegin < 0)
            idxGroupBegin = 0;
        if (idxGroupEnd >= GetGroupCount())
            idxGroupEnd = GetGroupCount() - 1;
        for (int i = idxGroupBegin; i <= idxGroupEnd; ++i)
        {
            auto& f = m_Group[i];
            int j0, j1;
            if (i == idxGroupBegin)
                j0 = idxBegin;
            else
                j0 = 0;
            if (i == idxGroupEnd)
                j1 = idxEnd;
            else
                j1 = (int)f.Item.size() - 1;
            for (int j = j0; j <= j1; ++j)
            {
                auto& e = f.Item[j];
                GetGroupPartRect(ListPart::Item, j, i, rcItem);
                const BOOL bIntersectOld = IsRectsIntersect(rcItem, rcOld);
                const BOOL bIntersectNew = IsRectsIntersect(rcItem, m_rcDragSel);
                if (wParam & MK_CONTROL)
                {
                    if (bIntersectOld != bIntersectNew)
                        e.uFlags ^= LEIF_SELECTED;// 翻转选中状态
                }
                else
                {
                    if (bIntersectOld && !bIntersectNew)
                        e.uFlags &= ~LEIF_SELECTED;// 先前选中但是现在未选中，清除选中状态
                    else if (!bIntersectOld && bIntersectNew)
                        e.uFlags |= LEIF_SELECTED;// 先前未选中但是现在选中，设置选中状态
                    // mark设为离光标最远的选中项（标准ListView的行为）
                    if (bIntersectNew && !(wParam & (MK_CONTROL | MK_SHIFT)))
                    {
                        const float d = (pt.x - rcItem.left) * (pt.x - rcItem.left) +
                            (pt.y - rcItem.top) * (pt.y - rcItem.top);
                        if (d > m_dCursorToItemMax)
                        {
                            m_dCursorToItemMax = d;
                            m_idxMark = i;
                        }
                    }
                }
            }
        }
    Skip:
        OffsetRect(m_rcDragSel, 0.f, dy);
        InvalidateRect();
    }

    void DragSelMouseMove(POINT pt, WPARAM wParam)
    {
        const auto dy = (float)m_psvV->GetPosition();
        EckAssert(m_bDraggingSel);
        D2D1_RECT_F rcOld{ m_rcDragSel };
        OffsetRect(rcOld, 0, -dy);

        m_rcDragSel = MakeD2DRectF(MakeRect(pt, POINT{
            m_ptDragSelStart.x,
            m_ptDragSelStart.y - (int)dy }));

        D2D1_RECT_F rcJudge;
        UnionRect(rcJudge, rcOld, m_rcDragSel);

        int idxBegin, idxEnd;
        D2D1_RECT_F rcItem;
        CalcItemRangeInRect(rcJudge, idxBegin, idxEnd);
        if (idxEnd < 0)
            goto Skip;
        if (idxBegin < 0)
            idxBegin = 0;
        if (idxEnd >= GetItemCount())
            idxEnd = GetItemCount() - 1;
        for (int i = idxBegin; i <= idxEnd; ++i)
        {
            auto& e = m_vItem[i];
            GetItemRect(i, rcItem);
            const BOOL bIntersectOld = IsRectsIntersect(rcItem, rcOld);
            const BOOL bIntersectNew = IsRectsIntersect(rcItem, m_rcDragSel);
            if (wParam & MK_CONTROL)
            {
                if (bIntersectOld != bIntersectNew)
                    e.uFlags ^= LEIF_SELECTED;// 翻转选中状态
            }
            else
            {
                if (bIntersectOld && !bIntersectNew)
                    e.uFlags &= ~LEIF_SELECTED;// 先前选中但是现在未选中，清除选中状态
                else if (!bIntersectOld && bIntersectNew)
                    e.uFlags |= LEIF_SELECTED;// 先前未选中但是现在选中，设置选中状态
                // mark设为离光标最远的选中项（标准ListView的行为）
                if (bIntersectNew && !(wParam & (MK_CONTROL | MK_SHIFT)))
                {
                    const float d = (pt.x - rcItem.left) * (pt.x - rcItem.left) +
                        (pt.y - rcItem.top) * (pt.y - rcItem.top);
                    if (d > m_dCursorToItemMax)
                    {
                        m_dCursorToItemMax = d;
                        m_idxMark = i;
                    }
                }
            }
        }
    Skip:
        OffsetRect(m_rcDragSel, 0.f, dy);
        InvalidateRect();
    }

    void OnPaint(WPARAM wParam, LPARAM lParam)
    {
        ELEMPAINTSTRU ps;
        BeginPaint(ps, wParam, lParam);
        NMLTCUSTOMDRAW nm{};
        nm.uCode = EE_CUSTOMDRAW;
        nm.dwStage = CDDS_PREPAINT;
        LRESULT r{};
        if (m_bCustomDraw)
            if ((r = GenElemNotify(&nm)) & CDRF_SKIPDEFAULT)
                goto SkipDef;

        switch (m_eView)
        {
        case Type::Report:
            if (!m_Header.GetItemCount())
                goto EndPaintLabel;
            [[fallthrough]];
        case Type::List:
        {
            if (m_bGroup)
            {
                if (m_Group.empty())
                    goto EndPaintLabel;

                const auto iSbPos = m_psvV->GetPosition();
                auto it = std::lower_bound(m_Group.begin() + m_idxTopGroup, m_Group.end(),
                    ps.rcfClipInElem.top + iSbPos, [](const GROUP& x, float iPos)
                    {
                        return x.y < iPos;
                    });
                if (it != m_Group.begin())
                    --it;
                nm.idxGroup = (int)std::distance(m_Group.begin(), it);
                for (; nm.idxGroup < GetGroupCount(); ++nm.idxGroup)
                {
                    const auto& e = m_Group[nm.idxGroup];
                    if (e.y >= ps.rcfClipInElem.bottom + iSbPos)
                        break;
                    nm.bColorText = FALSE;
                    LRESULT r2{};
                    if (r & CDRF_NOTIFYITEMDRAW)
                    {
                        nm.dwStage = CDDS_ITEMPREPAINT;
                        r2 = GenElemNotify(&nm);
                    }
                    GRPaintGroup(ps.rcfClipInElem, nm, r2);
                    nm.idx = (nm.idxGroup == m_idxTopGroup ? m_idxTop : 0);
                    for (; nm.idx < (int)e.Item.size(); ++nm.idx)
                    {
                        if (e.Item[nm.idx].y >= ps.rcfClipInElem.bottom + iSbPos)
                            break;
                        LVPaintItem(ps.rcfClipInElem, nm, r);
                    }
                }
            }
            else
            {
                if (!GetItemCount())
                    goto EndPaintLabel;
                nm.idxGroup = -1;

                const int idxBegin = std::max(LVItemFromY(ps.rcfClipInElem.top), 0);
                const int idxEnd = std::min(LVItemFromY(ps.rcfClipInElem.bottom), GetItemCount() - 1);
                for (nm.idx = idxBegin; nm.idx <= idxEnd; ++nm.idx)
                    LVPaintItem(ps.rcfClipInElem, nm, r);
            }
        }
        break;
        case Type::Icon:
        {
            if (!GetItemCount())
                goto EndPaintLabel;
            nm.idxGroup = -1;
            int idxBegin, idxX, idxY;

            idxX = IVLogItemFromX(ps.rcfClipInElem.left + 1);
            if (idxX < 0 || idxX >= m_cItemPerRow)
                idxBegin = -1;
            else
            {
                idxY = IVLogItemFromY(ps.rcfClipInElem.top + 1);
                idxBegin = m_idxTop + idxX + idxY * m_cItemPerRow;
                if (idxBegin < 0 || idxBegin >= GetItemCount())
                    idxBegin = -1;
            }

            if (idxBegin >= 0)
                for (int i = idxBegin; i < GetItemCount(); ++i)
                {
                    if (IVGetItemXY(i).first >= ps.rcfClipInElem.right)// 需要下移一行
                    {
                        i = idxBegin + m_cItemPerRow;
                        idxBegin = i;
                        if (i >= GetItemCount())
                            break;
                    }

                    if (IVGetItemXY(i).second >= ps.rcfClipInElem.bottom)// Y方向重画完成
                        break;
                    nm.idx = i;
                    IVPaintItem(ps.rcfClipInElem, nm, r);
                }
        }
        break;
        default: ECK_UNREACHABLE;
        }
    SkipDef:
        if (m_idxInsertMark >= 0)
        {
            D2D1_RECT_F rcIm;
            GetInsertMarkRect(rcIm);
            if (rcIm.bottom > 0.f && rcIm.top < GetHeightF())
            {
                // TODO：插入标记
            }
        }

        if (m_bDraggingSel)
        {
            auto rc{ m_rcDragSel };
            OffsetRect(rc, 0.f, (float)-m_psvV->GetPosition());
            GetTheme()->DrawBackground(Part::ListSelRect,
                State::None, rc, nullptr);
        }

        PostPaint(ps);

        if (r & CDRF_NOTIFYPOSTPAINT)
        {
            nm.dwStage = CDDS_POSTPAINT;
            GenElemNotify(&nm);
        }
    EndPaintLabel:
        ECK_DUI_DBG_DRAW_FRAME;
        EndPaint(ps);
    }

    void OnVScroll()
    {
        ReCalculateTopItem();
        InvalidateRect();
        NMLTSCROLLED nm{ LTE_SCROLLED };
        CalcItemRangeInRect(GetViewRectF(), nm.idxBegin, nm.idxEnd);
        GenElemNotify(&nm);
    }

    void IVGetItemRangeRect(int idxBegin, int idxEnd, _Out_ D2D1_RECT_F& rc)
    {
        EckAssert(m_eView == Type::Icon);
        if (idxBegin == idxEnd)
            GetItemRect(idxBegin, rc);
        else
        {
            auto [x1, y1] = IVGetItemXY(idxBegin);
            auto [x2, y2] = IVGetItemXY(idxEnd);
            if (y1 == y2)
            {
                rc =
                {
                    x1,
                    y1,
                    x2 + m_cxItem,
                    y2 + m_cyItem
                };
            }
            else
            {
                rc =
                {
                    0,
                    y1,
                    GetWidthF(),
                    y2 + m_cyItem
                };
            }
        }
    }
public:
    LRESULT OnEvent(UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept override
    {
        switch (uMsg)
        {
        case WM_PAINT:
            OnPaint(wParam, lParam);
            return 0;

        case WM_MOUSEMOVE:
        {
            ECK_DUILOCK;
            LE_HITTEST ht{ ECK_GET_PT_LPARAM(lParam) };
            if (m_bDraggingSel)
            {
                if (ht.pt.x < 0) ht.pt.x = 0;
                if (ht.pt.y < 0) ht.pt.y = 0;
                if (ht.pt.x >= GetWidthF()) ht.pt.x = int(GetWidthF() - 1.f);
                if (ht.pt.y >= GetHeightF()) ht.pt.y = int(GetHeightF() - 1.f);
                if (m_bGroup)
                    GRDragSelMouseMove(ht.pt, wParam);
                else
                    DragSelMouseMove(ht.pt, wParam);
                return 0;
            }

            int idx = HitTest(ht);
            NMLTHOTITEMCHANGE nm{ LTE_HOTITEMCHANED };
            if (m_bGroup)
            {
                if (idx != m_idxHot || m_idxHotItemGroup != ht.idxGroup)
                {
                    nm.idx = idx;
                    nm.idxGroup = ht.idxGroup;
                    nm.idxOld = m_idxHot;
                    nm.idxGroupOld = m_idxHotItemGroup;
                    if (GenElemNotify(&nm))
                        break;
                    std::swap(idx, m_idxHot);
                    std::swap(ht.idxGroup, m_idxHotItemGroup);
                    if (ht.idxGroup >= 0 && idx >= 0)
                        RedrawGroupItem(ht.idxGroup, idx);
                    if (m_idxHotItemGroup >= 0 && m_idxHot >= 0)
                        RedrawGroupItem(m_idxHotItemGroup, m_idxHot);
                }
            }
            else if (idx != m_idxHot)
            {
                nm.idx = idx;
                nm.idxGroup = -1;
                nm.idxOld = m_idxHot;
                nm.idxGroupOld = -1;
                if (GenElemNotify(&nm))
                    break;
                std::swap(m_idxHot, idx);
                if (idx >= 0)
                    RedrawItem(idx);
                if (m_idxHot >= 0)
                    RedrawItem(m_idxHot);
            }
        }
        return 0;

        case WM_MOUSELEAVE:
        {
            ECK_DUILOCK;
            if (m_bGroup)
            {
                int idx = -1, idxGroup = -1;
                std::swap(idx, m_idxHot);
                std::swap(idxGroup, m_idxHotItemGroup);
                if (idxGroup >= 0 && idx >= 0)
                    RedrawGroupItem(idxGroup, idx);
            }
            else if (m_idxHot >= 0)
            {
                int idx = -1;
                std::swap(m_idxHot, idx);
                RedrawItem(idx);
            }
        }
        return 0;

        case WM_MOUSEWHEEL:
        {
            ECK_DUILOCK;
            if (wParam & MK_SHIFT)
                m_psvH->OnMouseWheel2(-GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA);
            else
                m_psvV->OnMouseWheel2(-GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA);
            GetWnd()->WakeRenderThread();
        }
        return 0;

        case WM_NOTIFY:
        {
            ECK_DUILOCK;
            if ((wParam == (WPARAM)&m_SBV) &&
                (((DUINMHDR*)lParam)->uCode == EE_VSCROLL))
            {
                OnVScroll();
                return TRUE;
            }
            else if ((wParam == (WPARAM)&m_SBH) &&
                (((DUINMHDR*)lParam)->uCode == EE_HSCROLL))
            {
                m_Header.SetPosition(-m_psvH->GetPosition() + (int)GetRealGroupImageWidth(), 0);
                InvalidateRect();
                return TRUE;
            }
            else if ((wParam == (WPARAM)&m_Header))
                switch (((DUINMHDR*)lParam)->uCode)
                {
                case HEE_GETDISPINFO:
                    return GenElemNotify((NMHEDISPINFO*)lParam);
                case HEE_WIDTHCHANGED:
                {
                    EckAssert(m_eView == Type::Report);
                    const auto* const p = (NMHEITEMNOTIFY*)lParam;
                    ArrangeHeader(TRUE);
                    ReCalcHScroll();
                    D2D1_RECT_F rc;
                    m_Header.GetItemRect(p->idx, rc);
                    if (m_bGroup)
                    {
                        if (p->idx)
                            for (auto& e : m_Group)
                                for (auto& f : e.Item)
                                    f.vSubItem[p->idx - 1].pLayout.Clear();
                        else
                            for (auto& e : m_Group)
                                for (auto& f : e.Item)
                                    f.pLayout.Clear();
                        rc.left += GetRealGroupImageWidth();
                    }
                    else
                    {
                        if (p->idx)
                            for (auto& e : m_vItem)
                                e.vSubItem[p->idx - 1].pLayout.Clear();
                        else
                            for (auto& e : m_vItem)
                                e.pLayout.Clear();
                    }
                    rc.right = GetWidthF();
                    rc.bottom = GetHeightF();
                    InvalidateRect(rc);
                }
                return 0;
                }
        }
        break;

        case WM_SIZE:
        {
            ECK_DUILOCK;
            ReCalculateTopItem();
            ReCalcScroll();
            const auto cxSB = GetTheme()->GetMetrics(Metrics::CxVScroll);
            m_SBV.SetRect({
                GetWidthF() - cxSB,
                m_cyTopExtra,
                GetWidthF(),
                GetHeightF() - m_cyBottomExtra });
            m_SBH.SetRect({ 0,GetHeightF() - cxSB,GetWidthF(),GetHeightF() });
            ArrangeHeader();
        }
        return 0;

        case WM_LBUTTONDOWN:
        {
            ECK_DUILOCK;
            POINT pt ECK_GET_PT_LPARAM(lParam);
            SetFocus();
            LE_HITTEST ht{ pt };
            int idx = HitTest(ht);

            D2D1_RECT_F rcInvalid;
            if (idx >= 0)
            {
                D2D1_RECT_F rcItem;
                BOOL bDid;
                if (m_bToggleSel && !m_bSingleSel)
                    bDid = ToggleSelectItemForClick(idx, ht.idxGroup);
                else
                {
                    if (!(wParam & MK_CONTROL))
                        DeselectAll(rcInvalid);
                    bDid = SelectItemForClick(idx, ht.idxGroup);
                }
                if (bDid)
                {
                    if (m_bGroup)
                        GetGroupPartRect(ListPart::Item, idx, ht.idxGroup, rcItem);
                    else
                        GetItemRect(idx, rcItem);
                    UnionRect(rcInvalid, rcInvalid, rcItem);
                }
                InvalidateRect(rcInvalid);
            }
            else
            {
                ClientToScreen(GetWnd()->HWnd, &pt);
                if (!m_bSingleSel && m_bEnableDragSel &&
                    IsMouseMovedBeforeDragging(GetWnd()->HWnd, pt.x, pt.y))
                {
                    if (!GetWnd()->IsValid())
                        return 0;
                    SetCapture();
                    if (!(wParam & (MK_CONTROL | MK_SHIFT)))
                    {
                        D2D1_RECT_F rcInvalid;
                        DeselectAll(rcInvalid);
                        InvalidateRect(rcInvalid);
                    }
                    m_bDraggingSel = TRUE;
                    m_rcDragSel = {};
                    if (ht.pt.x < 0) ht.pt.x = 0;
                    if (ht.pt.y < 0) ht.pt.y = 0;
                    if (ht.pt.x >= GetWidthF()) ht.pt.x = int(GetWidthF() - 1);
                    if (ht.pt.y >= GetHeightF()) ht.pt.y = int(GetHeightF() - 1);
                    m_ptDragSelStart = ht.pt;
                    m_ptDragSelStart.y += (int)m_psvV->GetPosition();
                    m_dCursorToItemMax = INT_MIN;
                }
                else if (m_bDeSelInSpace)
                {
                    DeselectAll(rcInvalid);
                    InvalidateRect(rcInvalid);
                }
            }
        }
        return 0;

        case WM_LBUTTONUP:
        {
            if (m_bDraggingSel)
            {
                m_bDraggingSel = FALSE;
                ReleaseCapture();
                InvalidateRect();
            }
        }
        return 0;

        case WM_CAPTURECHANGED:
        {
            if (m_bDraggingSel)
            {
                m_bDraggingSel = FALSE;
                InvalidateRect();
            }
        }
        return 0;

        case WM_SETFONT:
            if (m_Header.TlIsValid())
                m_Header.SetTextFormat(GetTextFormat());
            break;

        case WM_CREATE:
        {
            m_pDC->CreateSolidColorBrush({}, &m_pBrush);

            m_Header.Create(nullptr, /*DES_VISIBLE*/0, 0,
                0, 0, 0, CyDefHeader, this);

            m_SBV.Create(nullptr, DES_VISIBLE, 0,
                0, 0, 0, 0, this);
            m_psvV = m_SBV.GetScrollView();
            m_psvV->AddRef();
            m_psvV->SetMinThumbSize(CxyMinScrollThumb);
            m_psvV->SetCallback([](float fPos, float fPrevPos, LPARAM lParam)
                {
                    auto pThis = (CListTemplate*)lParam;
                    pThis->ReCalculateTopItem();
                    pThis->InvalidateRect();
                    if (pThis->m_psvV->IsStop() && pThis->GetItemCount())
                    {
                        NMLTSCROLLED nm{ LTE_SCROLLED };
                        pThis->CalcItemRangeInRect(
                            pThis->GetViewRectF(), nm.idxBegin, nm.idxEnd);
                        pThis->GenElemNotify(&nm);
                    }
                }, (LPARAM)this);
            m_psvV->SetDelta(80);

            m_SBH.Create(nullptr, 0, 0,
                0, 0, 0, 0, this);
            m_SBH.SetHorizontal(TRUE);
            m_psvH = m_SBH.GetScrollView();
            m_psvH->AddRef();
            m_psvH->SetMinThumbSize(CxyMinScrollThumb);
            m_psvH->SetCallback([](float fPos, float fPrevPos, LPARAM lParam)
                {
                    auto pThis = (CListTemplate*)lParam;
                    pThis->m_Header.SetPosition(-fPos + pThis->GetRealGroupImageWidth(), 0);
                    pThis->InvalidateRect();
                }, (LPARAM)this);
            m_psvH->SetDelta(40);
        }
        return 0;

        case WM_DESTROY:
        {
            m_idxTop = 0;
            m_idxHot = -1;
            m_idxSel = -1;
            m_idxInsertMark = -1;

            m_cyItem = 0;
            m_cyPadding = 0;
            m_oyTopItem = 0;

            SafeRelease(m_pBrush);
            m_vItem.clear();
            m_bSingleSel = FALSE;

            SafeRelease(m_psvV);
            SafeRelease(m_psvH);
            SafeRelease(m_pImgList);
            SafeRelease(m_pImgListGroup);
            SafeRelease(m_pTfGroup);
        }
        return 0;
        }
        return __super::OnEvent(uMsg, wParam, lParam);
    }

    EckInline void SetItemCount(int c) noexcept
    {
        ECK_DUILOCK;
        if (m_eView == Type::Report)
        {
            const auto idxBegin = std::max(0, GetItemCount() - 1);
            m_vItem.resize(c);
            for (int i = idxBegin; i < c; ++i)
                m_vItem[i].vSubItem.resize(m_Header.GetItemCount());
        }
        else
            m_vItem.resize(c);
    }
    EckInlineNdCe int GetItemCount() const noexcept { return (int)m_vItem.size(); }

    void GetItemRect(int idx, D2D1_RECT_F& rc) const
    {
        switch (m_eView)
        {
        case Type::List:
            rc.left = 0;
            rc.right = GetWidthF();
            rc.top = LVGetItemY(idx);
            rc.bottom = rc.top + m_cyItem;
            break;
        case Type::Report:
        {
            rc.left = -m_psvH->GetPosition();
            rc.right = rc.left + m_Header.GetContentWidth();
            rc.top = LVGetItemY(idx);
            rc.bottom = rc.top + m_cyItem;
        }
        break;
        case Type::Icon:
        {
            const auto xy = IVGetItemXY(idx);
            rc.left = xy.first;
            rc.right = rc.left + m_cxItem;
            rc.top = xy.second;
            rc.bottom = rc.top + m_cyItem;
        }
        break;
        default:
            ECK_UNREACHABLE;
        }
    }

    void GetSubItemRect(int idx, int idxSub, D2D1_RECT_F& rc) const noexcept
    {
        EckAssert(idx < GetItemCount());
        if (m_eView != Type::Report)
        {
            GetItemRect(idx, rc);
            return;
        }
        m_Header.GetItemRect(idxSub, rc);
        rc.left -= m_psvH->GetPosition();
        rc.right -= m_psvH->GetPosition();
        rc.top = LVGetItemY(idx);
        rc.bottom = rc.top + m_cyItem;
    }

    void GetGroupPartRect(ListPart ePart, int idxItemInGroup,
        int idxGroup, _Out_ D2D1_RECT_F& rc) const noexcept
    {
        auto& e = m_Group[idxGroup];
        float dx, cxContent;
        switch (m_eView)
        {
        case Type::List:
            dx = 0;
            cxContent = GetWidthF();
            break;
        case Type::Report:
            dx = -m_psvH->GetPosition();
            cxContent = m_Header.GetContentWidth();
            break;
        default: ECK_UNREACHABLE;
        }
        switch (ePart)
        {
        case ListPart::GroupHeader:
            rc =
            {
                dx,
                e.y - m_psvV->GetPosition(),
                dx + cxContent,
                e.y - m_psvV->GetPosition() + m_cyGroupHeader,
            };
            break;
        case ListPart::GroupText:
            rc = {};
            EckDbgBreak();
            break;
        case ListPart::Item:
            rc =
            {
                dx + m_cxGroupImage,
                e.Item[idxItemInGroup].y - m_psvV->GetPosition(),
                dx + m_cxGroupImage + cxContent,
                e.Item[idxItemInGroup].y - m_psvV->GetPosition() + m_cyItem,
            };
            break;
        case ListPart::GroupImg:
        {
            const auto y = e.y - m_psvV->GetPosition() +
                m_cyGroupHeader + m_cyGroupItemTopPadding;
            rc = { dx,y,dx + m_cxGroupImage,y + m_cyGroupImage };
        }
        break;
        default: ECK_UNREACHABLE;
        }
    }

    void GetGroupSubItemRect(int idx, int idxSub, int idxGroup, D2D1_RECT_F& rc) const noexcept
    {
        EckAssert(idxGroup < GetGroupCount());
        EckAssert(idx < (int)m_Group[idxGroup].Item.size());
        GetGroupPartRect(ListPart::Item, idx, idxGroup, rc);
        if (m_eView != Type::Report)
            return;
        D2D1_RECT_F rc2;
        m_Header.GetItemRect(idxSub, rc2);
        if (m_bGroupImage)
        {
            rc2.left += m_cxGroupImage;
            rc2.right += m_cxGroupImage;
        }
        rc.left = rc2.left - m_psvH->GetPosition();
        rc.right = rc2.right - m_psvH->GetPosition();
    }

    int HitTest(LE_HITTEST& leht) const
    {
        leht.bHitGroupHeader = FALSE;
        leht.bHitGroupImage = FALSE;
        if (!PtInRect(GetViewRectF(), leht.pt))
            return -1;

        switch (m_eView)
        {
        case Type::List:
        case Type::Report:
        {
            if (m_bGroup)
            {
                if (!GetGroupCount())
                    return -1;
                //---测试组
                const auto y = leht.pt.y + m_psvV->GetPosition();
                auto it = std::lower_bound(m_Group.begin() + m_idxTopGroup, m_Group.end(), y,
                    [](const GROUP& x, float iPos)
                    {
                        return x.y < iPos;
                    });
                if (it == m_Group.begin())
                    return -1;
                else if (it == m_Group.end())
                    it = (m_Group.rbegin() + 1).base();
                else
                    --it;
                const int idxGroup = (int)std::distance(m_Group.begin(), it);
                leht.idxGroup = idxGroup;
                // 测试组部件
                const auto yGroupBottom = it->y +
                    m_cyGroupHeader + m_cyGroupItemTopPadding;
                if (y > it->y && y < yGroupBottom)
                {
                    leht.bHitGroupHeader = TRUE;
                    return -1;
                }
                if (m_bGroupImage && leht.pt.x < m_cxGroupImage)
                {
                    if (y >= yGroupBottom &&
                        y < yGroupBottom + m_cyGroupImage)
                        leht.bHitGroupImage = TRUE;
                    return -1;
                }
                //---测试组项
                const auto yInItem = y - yGroupBottom;
                if (yInItem >= 0)
                {
                    int idx = int(yInItem / (m_cyItem + m_cyPadding));
                    if (yInItem > (idx + 1) * (m_cyItem + m_cyPadding) - m_cyPadding)
                        return -1;// 命中项目间隔
                    if (idx < (int)it->Item.size())
                        return idx;
                }
                return -1;
            }
            if (!GetItemCount())
                return -1;
            if (m_eView == Type::Report &&
                leht.pt.x > m_Header.GetContentWidth())
                return -1;
            const int idx = LVItemFromY((float)leht.pt.y);
            if (leht.pt.y >= LVGetItemY(idx) + m_cyItem)
                return -1;// 命中项目间隔
            if (idx >= 0 && idx < GetItemCount())
                return idx;
            else
                return -1;
        }
        break;

        case Type::Icon:
        {
            if (!GetItemCount())
                return -1;
            const int idxX = IVLogItemFromX((float)leht.pt.x);
            if (idxX < 0 || idxX >= m_cItemPerRow)
                return -1;
            const int idxY = IVLogItemFromY((float)leht.pt.y);
            const int idx = m_idxTop + idxX + idxY * m_cItemPerRow;
            if (idx < 0 || idx >= GetItemCount())
                return -1;
            const auto [x, y] = IVGetItemXY(idx);
            if (leht.pt.x >= x + m_cxItem ||
                leht.pt.y >= y + m_cyItem)
                return -1;// 命中项目间隔
            return idx;
        }
        break;
        default: ECK_UNREACHABLE;
        }
    }

    void DeselectAll(_Out_ D2D1_RECT_F& rcInvalid)
    {
        NMLTITEMCHANGE nm{ LTE_ITEMCHANED };
        nm.uFlagsOld = LEIF_SELECTED;
        nm.uFlagsNew = 0;
        if (m_bGroup)
        {
            if (m_bSingleSel)
            {
                if (m_idxSel >= 0 && m_idxSelItemGroup >= 0)
                {
                    if (m_bItemNotify)
                    {
                        nm.idx = m_idxSel;
                        nm.idxGroup = m_idxSelItemGroup;
                        if (GenElemNotify(&nm))
                        {
                            rcInvalid = {};
                            return;
                        }
                    }
                    GetGroupPartRect(ListPart::Item, m_idxSel,
                        m_idxSelItemGroup, rcInvalid);
                }
                m_idxSel = m_idxSelItemGroup = -1;
            }
            else
            {
                const auto iSbPos = m_psvV->GetPosition();
                const auto cy = GetHeightF();
                float y0{ FLT_MAX }, y1{ FLT_MIN }, x{ FLT_MAX };
                EckCounter(GetGroupCount(), i)
                {
                    auto& e = m_Group[i];
                    if (e.uFlags & LEIF_SELECTED)
                    {
                        // TODO: 通知组改变
                        e.uFlags &= ~LEIF_SELECTED;
                        const auto y = e.y - iSbPos;
                        if ((y > -m_cyGroupHeader && y < GetHeightF()))
                        {
                            if (y0 == FLT_MAX)
                                y0 = y;
                            y1 = std::max(y1, y + m_cyGroupHeader);
                            x = std::min(x, 0.f);
                        }
                    }
                    EckCounter((int)e.Item.size(), j)
                    {
                        auto& f = e.Item[j];
                        if (f.uFlags & LEIF_SELECTED)
                        {
                            if (m_bItemNotify)
                            {
                                nm.idx = j;
                                nm.idxGroup = i;
                                if (GenElemNotify(&nm))
                                    goto SkipItem;
                            }
                            f.uFlags &= ~LEIF_SELECTED;
                            const auto y = f.y - iSbPos;
                            if ((y > -m_cyItem && y < GetHeightF()))
                            {
                                if (y0 == INT_MAX)
                                    y0 = y;
                                y1 = std::max(y1, y + m_cyItem);
                            }
                            x = std::min(x, m_cxGroupImage);
                        }
                    SkipItem:
                        ++i;
                    }
                }
                rcInvalid = { x, y0, GetWidthF(), y1 };
            }
        }
        else
        {
            if (m_bSingleSel)
            {
                if (m_idxSel >= 0)
                {
                    if (m_bItemNotify)
                    {
                        nm.idx = m_idxSel;
                        nm.idxGroup = -1;
                        if (GenElemNotify(&nm))
                        {
                            rcInvalid = {};
                            return;
                        }
                    }
                    GetItemRect(m_idxSel, rcInvalid);
                    m_idxSel = -1;
                }
                else
                    rcInvalid = {};
            }
            else
            {
                int idx0 = -1, idx1 = 0;
                EckCounter(GetItemCount(), i)
                {
                    auto& e = m_vItem[i];
                    if (e.uFlags & LEIF_SELECTED)
                    {
                        if (m_bItemNotify)
                        {
                            nm.idx = i;
                            nm.idxGroup = -1;
                            if (GenElemNotify(&nm))
                                goto SkipItem2;
                        }
                        e.uFlags &= ~LEIF_SELECTED;
                        if (idx0 < 0)
                            idx0 = i;
                        idx1 = i;
                    }
                SkipItem2:;
                }
                if (idx0 >= 0)
                {
                    if (m_eView == Type::Icon)
                        IVGetItemRangeRect(idx0, idx1, rcInvalid);
                    else
                    {
                        GetItemRect(idx0, rcInvalid);
                        D2D1_RECT_F rc;
                        GetItemRect(idx1, rc);
                        UnionRect(rcInvalid, rcInvalid, rc);
                    }
                }
                else
                    rcInvalid = {};
            }
        }
    }

    BOOL SelectItemForClick(int idx, int idxGroup = -1)
    {
        NMLTITEMINDEX nm{ EE_CLICK };
        nm.idx = idx;
        nm.idxGroup = idxGroup;
        GenElemNotify(&nm);
        if (m_bItemNotify)
        {
            NMLTITEMCHANGE nm2{ LTE_ITEMCHANED };
            nm2.uFlagsOld = 0;
            nm2.uFlagsNew = LEIF_SELECTED;
            nm2.idx = idx;
            nm2.idxGroup = idxGroup;
            if (GenElemNotify(&nm2))
                return FALSE;
        }
        m_idxFocus = idx;
        m_idxFocusItemGroup = idxGroup;
        m_idxMark = idx;
        m_idxMarkItemGroup = idxGroup;
        if (m_bSingleSel)
        {
            m_idxSel = idx;
            m_idxSelItemGroup = idxGroup;
        }
        else if (m_bGroup)
            m_Group[idxGroup].Item[idx].uFlags |= LEIF_SELECTED;
        else
            m_vItem[idx].uFlags |= LEIF_SELECTED;
        return TRUE;
    }

    BOOL ToggleSelectItemForClick(int idx, int idxGroup = -1)
    {
        if (m_bSingleSel)
            return FALSE;
        NMLTITEMINDEX nm{ EE_CLICK };
        nm.idx = idx;
        nm.idxGroup = idxGroup;
        GenElemNotify(&nm);
        auto& dwFlags = m_bGroup ?
            m_Group[idxGroup].Item[idx].uFlags :
            m_vItem[idx].uFlags;
        if (m_bItemNotify)
        {
            NMLTITEMCHANGE nm2{ LTE_ITEMCHANED };
            nm2.uFlagsOld = dwFlags;
            nm2.uFlagsNew = dwFlags ^ LEIF_SELECTED;
            nm2.idx = idx;
            nm2.idxGroup = idxGroup;
            if (GenElemNotify(&nm2))
                return FALSE;
        }
        m_idxFocus = idx;
        m_idxFocusItemGroup = idxGroup;
        m_idxMark = idx;
        m_idxMarkItemGroup = idxGroup;
        dwFlags ^= LEIF_SELECTED;
        return TRUE;
    }

    void SetImageList(CD2DImageList* pImgList)
    {
        ECK_DUILOCK;
        std::swap(m_pImgList, pImgList);
        if (m_pImgList)
            m_pImgList->AddRef();
        if (pImgList)
            pImgList->Release();
    }

    void SetGroupImageList(CD2DImageList* pImgList)
    {
        ECK_DUILOCK;
        std::swap(m_pImgListGroup, pImgList);
        if (m_pImgListGroup)
            m_pImgListGroup->AddRef();
        if (pImgList)
            pImgList->Release();
    }

    void GetInsertMarkRect(D2D1_RECT_F& rc) const
    {
        if (m_idxInsertMark < 0)
        {
            rc = {};
            return;
        }
        rc.left = 0.f;
        rc.right = GetWidthF();
        rc.top = LVGetItemY(m_idxInsertMark) - CyInsertMark * 2.f;
        rc.bottom = rc.top + CyInsertMark * 5.f;
    }

    void SetInsertMark(int idx, BOOL bRedraw = TRUE)
    {
        m_idxInsertMark = idx;
        if (bRedraw)
        {
            D2D1_RECT_F rc;
            GetInsertMarkRect(rc);
            InvalidateRect(rc);
        }
    }

    void InvalidateCache(int idx)
    {
        if (idx < 0)
            for (auto& e : m_vItem)
                e.pLayout.Clear();
        else
        {
            EckAssert(idx < GetItemCount());
            m_vItem[idx].pLayout.Clear();
            if (m_eView == Type::Report)
                for (auto& e : m_vItem[idx].vSubItem)
                    e.pLayout.Clear();
        }
    }
    void InvalidateCache()
    {
        for (int i = 0; i < GetItemCount(); ++i)
            InvalidateCache(i);
    }

    void RedrawItem(int idxBegin, int idxEnd)
    {
        EckAssert(idxEnd >= idxBegin);
        D2D1_RECT_F rc;
        switch (m_eView)
        {
        case Type::List:
        {
            rc =
            {
                0,
                LVGetItemY(idxBegin),
                GetWidthF(),
                LVGetItemY(idxEnd) + m_cyItem
            };
        }
        break;
        case Type::Report:
        {
            rc =
            {
                (float)-m_psvH->GetPosition(),
                LVGetItemY(idxBegin),
                std::min(GetWidthF(), m_Header.GetContentWidth()),
                LVGetItemY(idxEnd) + m_cyItem
            };
        }
        break;
        case Type::Icon:
            IVGetItemRangeRect(idxBegin, idxEnd, rc);
            break;
        default: ECK_UNREACHABLE;
        }
        InvalidateRect(rc);
    }

    void RedrawItem(int idx)
    {
        D2D1_RECT_F rc;
        GetItemRect(idx, rc);
        InvalidateRect(rc);
    }

    void RedrawGroupItem(int idxGroup, int idxItemInGroup)
    {
        EckAssert(GetGroup());
        D2D1_RECT_F rc;
        GetGroupPartRect(ListPart::Item, idxItemInGroup, idxGroup, rc);
        InvalidateRect(rc);
    }

    void ReCalc(int idxGroupBegin = 0)
    {
        if (m_bGroup)
        {
            float y{};
            for (size_t i = idxGroupBegin; i < m_Group.size(); ++i)
            {
                auto& Group = m_Group[i];
                Group.y = y;
                y += (m_cyGroupHeader + m_cyGroupItemTopPadding);
                const auto yImg = y;
                for (auto& Item : Group.Item)
                {
                    Item.y = y;
                    y += (m_cyItem + m_cyPadding);
                }
                y -= m_cyPadding;;
                y += m_cyGroupItemBottomPadding;
                if (m_bGroupImage && y < yImg + m_cyGroupImage)
                    y = yImg + m_cyGroupImage;
            }
            m_psvV->SetRange(-m_cyTopExtra, y + m_cyBottomExtra);
        }
        ReCalcScroll();
        ReCalculateTopItem();
    }

    EckInline void SetHeaderHeight(float cy)
    {
        m_Header.SetSize(m_Header.GetWidthF(), cy);
    }
    EckInlineNd float GetHeaderHeight() const noexcept { return m_Header.GetHeightF(); }

    void SetColumnCount(int cItem,
        _In_reads_opt_(cItem) const float* pcx = nullptr) noexcept
    {
        EckAssert(m_eView == Type::Report);
        m_Header.SetItemCount(cItem, pcx);
        if (m_bGroup)
            for (auto& e : m_Group)
                for (auto& f : e.Item)
                    f.vSubItem.resize(cItem - 1);
        else
            for (auto& e : m_vItem)
                e.vSubItem.resize(cItem - 1);
    }

    EckInline void SetGroupCount(int cGroups)
    {
        ECK_DUILOCK;
        m_Group.resize(cGroups);
    }
    EckInlineNdCe int GetGroupCount() const { return (int)m_Group.size(); }

    EckInline void SetGroupItemCount(int idxGroup, int cItems)
    {
        ECK_DUILOCK;
        auto& v = m_Group[idxGroup].Item;
        if (m_eView == Type::Report)
        {
            const auto idxBegin = std::max(0, (int)v.size() - 1);
            v.resize(cItems);
            for (int i = idxBegin; i < cItems; ++i)
                v[i].vSubItem.resize(m_Header.GetItemCount());
        }
        else
            v.resize(cItems);
    }
    EckInlineNdCe int GetGroupItemCount(int idxGroup) const { return (int)m_Group[idxGroup].Item.size(); }

    void SetGroupTextFormat(IDWriteTextFormat* pTf)
    {
        ECK_DUILOCK;
        std::swap(m_pTfGroup, pTf);
        if (m_pTfGroup)
            m_pTfGroup->AddRef();
        if (pTf)
            pTf->Release();
    }

    void SetView(Type eView) noexcept
    {
        ECK_DUILOCK;
        m_eView = eView;
        if (m_eView == Type::Report)
            m_Header.SetVisible(TRUE);
        else
            m_Header.SetVisible(FALSE);
    }
    EckInlineNdCe Type GetView() const noexcept { return m_eView; }

    UINT SetItemState(int idx, UINT uFlags, int idxGroup = -1)
    {
        auto& uFlags0 = m_bGroup ?
            m_Group[idxGroup].Item[idx].uFlags :
            m_vItem[idx].uFlags;
        const auto uOld = uFlags0;
        uFlags0 = uFlags;
        return uOld;
    }
    UINT GetItemState(int idx, int idxGroup = -1) const
    {
        if (m_bGroup)
            return m_Group[idxGroup].Item[idx].uFlags;
        else
            if (m_bSingleSel)
                return m_idxSel == idx ? LEIF_SELECTED : 0;
            else
                return m_vItem[idx].uFlags;
    }

    EckInline int GetCurrentSelection() const noexcept
    {
        if (m_bSingleSel)
            return m_idxSel;
        else
            return -1;
    }

    void GetVisibleRange(_Out_ int& idxTop, _Out_ int& idxBottom)
    {
        if (!GetItemCount())
        {
            idxTop = -1;
            idxBottom = -2;
            return;
        }
        CalcItemRangeInRect(GetViewRectF(), idxTop, idxBottom);
        if (idxBottom >= GetItemCount())
            idxBottom = GetItemCount() - 1;
    }

    BOOL EnsureVisible(BOOL bSmooth, int idx, int idxGroup = -1)
    {
        if (!GetItemCount() || idx < 0 || idx >= GetItemCount())
            return FALSE;
        D2D1_RECT_F rcItem;
        if (m_bGroup)
            GetGroupPartRect(ListPart::Item, idx, idxGroup, rcItem);
        else
            GetItemRect(idx, rcItem);
        float d;
        if (rcItem.top < m_cyTopExtra)
            d = rcItem.top - m_cyTopExtra;
        else if (rcItem.bottom > GetHeightF() - m_cyBottomExtra)
            d = rcItem.bottom - GetHeightF() + m_cyBottomExtra;
        else
            return FALSE;
        if (bSmooth)
        {
            m_psvV->SmoothScrollDelta(d);
            GetWnd()->WakeRenderThread();
        }
        else
        {
            m_psvV->SetPosition(d + m_psvV->GetPosition());
            OnVScroll();
        }
        return TRUE;
    }

    EckInline void UpdateHeaderLayout() noexcept { ArrangeHeader(TRUE); }

    EckInlineNdCe int GetTopItem() noexcept { return m_idxTop; }

    EckInlineCe void SetGroupImageWidth(float cx) noexcept { m_cxGroupImage = cx; }
    EckInlineNdCe float GetGroupImageWidth() const noexcept { return m_cxGroupImage; }

    EckInlineCe void SetGroupImageHeight(float cy) noexcept { m_cyGroupImage = cy; }
    EckInlineNdCe float GetGroupImageHeight() const noexcept { return m_cyGroupImage; }

    EckInlineNdCe auto& GetScrollBarV() noexcept { return m_SBV; }
    EckInlineNdCe auto& GetScrollBarH() noexcept { return m_SBH; }
    EckInlineNdCe auto& GetHeader() noexcept { return m_Header; }

    EckInlineCe void SetItemHeight(float cy) noexcept { m_cyItem = cy; }
    EckInlineNdCe float GetItemHeight() const noexcept { return m_cyItem; }

    EckInlineCe void SetItemPadding(float cy) noexcept { m_cyPadding = cy; }
    EckInlineNdCe float GetItemPadding() const noexcept { return m_cyPadding; }

    EckInlineCe void SetItemWidth(float cx) noexcept
    {
        EckAssert(m_eView == Type::Icon);
        m_cxItem = cx;
    }
    EckInlineNdCe float GetItemWidth() const noexcept { return m_cxItem; }

    EckInlineCe void SetItemPaddingH(float cx) { m_cxPadding = cx; }
    EckInlineNdCe float GetItemPaddingH() const noexcept { return m_cxPadding; }

    EckInlineCe void SetSingleSel(BOOL bSingleSel) noexcept { m_bSingleSel = bSingleSel; }
    EckInlineNdCe BOOL GetSingleSel() const noexcept { return m_bSingleSel; }

    EckInlineCe void SetTopExtraSpace(float cy) noexcept { m_cyTopExtra = cy; }
    EckInlineNdCe float GetTopExtraSpace() const noexcept { return m_cyTopExtra; }

    EckInlineCe void SetBottomExtraSpace(float cy) noexcept { m_cyBottomExtra = cy; }
    EckInlineNdCe float GetBottomExtraSpace() const noexcept { return m_cyBottomExtra; }

    EckInlineCe void SetGroup(BOOL bGroup) noexcept { m_bGroup = bGroup; }
    EckInlineNdCe BOOL GetGroup() const noexcept { return m_bGroup; }

    EckInlineCe void SetGroupImage(BOOL b) noexcept { m_bGroupImage = b; }
    EckInlineNdCe BOOL GetGroupImage() const noexcept { return m_bGroupImage; }

    EckInlineCe void SetGroupItemTopPadding(float cy) noexcept { m_cyGroupItemTopPadding = cy; }
    EckInlineNdCe float GetGroupItemTopPadding() const noexcept { return m_cyGroupItemTopPadding; }

    EckInlineCe void SetGroupItemBottomPadding(float cy) noexcept { m_cyGroupItemBottomPadding = cy; }
    EckInlineNdCe float GetGroupItemBottomPadding() const noexcept { return m_cyGroupItemBottomPadding; }

    EckInlineCe void SetItemNotify(BOOL b) noexcept { m_bItemNotify = b; }
    EckInlineNdCe BOOL GetItemNotify() const noexcept { return m_bItemNotify; }

    EckInlineCe void SetToggleSel(BOOL b) noexcept { m_bToggleSel = b; }
    EckInlineNdCe BOOL GetToggleSel() const noexcept { return m_bToggleSel; }

    EckInlineCe void SetAllowDragSel(BOOL b) noexcept { m_bEnableDragSel = b; }
    EckInlineNdCe BOOL GetAllowDragSel() const noexcept { return m_bEnableDragSel; }

    EckInlineCe void SetDeSelInSpace(BOOL b) noexcept { m_bDeSelInSpace = b; }
    EckInlineNdCe BOOL GetDeSelInSpace() const noexcept { return m_bDeSelInSpace; }

    EckInlineCe void SetDbgIndex(BOOL b) noexcept { m_bDbgIndex = b; }
    EckInlineNdCe BOOL GetDbgIndex() const noexcept { return m_bDbgIndex; }

    EckInlineCe void SetCustomDraw(BOOL b) noexcept { m_bCustomDraw = b; }
    EckInlineNdCe BOOL GetCustomDraw() const noexcept { return m_bCustomDraw; }

    EckInlineCe void SetInterpolationMode(D2D1_INTERPOLATION_MODE e) { m_eInterMode = (BYTE)e; }
    EckInlineNdCe D2D1_INTERPOLATION_MODE GetInterpolationMode() const noexcept { return (D2D1_INTERPOLATION_MODE)m_eInterMode; }
};
ECK_DUI_NAMESPACE_END
ECK_NAMESPACE_END