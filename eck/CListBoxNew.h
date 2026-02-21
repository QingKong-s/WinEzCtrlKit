#pragma once
#include "CWindow.h"
#include "CtrlGraphics.h"
#include "CSelectionRange.h"

ECK_NAMESPACE_BEGIN
enum : UINT
{
    // 项目标志
    LBN_IF_SEL = 1u << 0,		// 选中

    // 搜索
    LBN_SF_CASEINSENSITIVE = 1u << 0,	// 不区分大小写
    LBN_SF_WHOLE = 1u << 1,		// 完全匹配
};

struct LBNITEM
{
    PCWSTR pszText;
    int cchText;
    int idxItem;
};

struct NMLBNGETDISPINFO
{
    NMHDR nmhdr;
    LBNITEM Item;
};

struct NMLBNDRAG
{
    NMHDR nmhdr;
    int idx;
    UINT uKeyFlags;
};

struct NMLBNITEMCHANGED
{
    NMHDR nmhdr;
    int idx;
    int idxEnd;
    UINT uFlagsNew;
    UINT uFlagsOld;
};

struct NMLBNSEARCH
{
    NMHDR nmhdr;
    LBNITEM Item;// idxItem表示起始项目（含）
};

#pragma pack(push, ECK_CTRLDATA_ALIGN)
struct CTRLDATA_LBN
{
    int iVer = 0;


    // 
};
#pragma pack(pop)
/*
* LBN产生的通知
* 特定通知：
* NM_LBN_GETDISPINFO	请求项目显示信息，若处理该通知，返回TRUE
* NM_LBN_BEGINDRAG		开始拖动项目
* NM_LBN_ENDDRAG		结束拖动项目
* NM_LBN_DISMISS		组合框应隐藏列表
* NM_LBN_ITEMCHANGED	项目状态改变
* NM_LBN_ITEMSTANDBY	调用SetItemCount时发送
* 标准通知：
* NM_SETFOCUS
* NM_KILLFOCUS
* NM_RCLICK
* NM_CUSTOMDRAW
*/
class CListBoxNew : public CWindow
{
public:
    ECK_RTTI(CListBoxNew, CWindow);
    ECK_CWND_SINGLEOWNER(CListBoxNew);
    ECK_CWND_CREATE_CLS_HINST(WCN_LISTBOXNEW, g_hInstance);
private:
    CSelRange m_SelRange{};
    int m_cItem{};

    CStringW m_rsTextBuf{};

    HWND m_hComboBox{};	// 关联的组合框，可以是除自身外的任何窗口
    HWND m_hParent{};	// 接收通知的父窗口

    CMemoryDC m_DC{};
    HFONT m_hFont{};

    HTHEME m_hTheme{};

    int m_cxClient{},
        m_cyClient{};

    COLORREF m_crBkg{ CLR_DEFAULT };
    int m_idxSel{ -1 };
    int m_idxHot{ -1 };
    int m_idxTop{ -1 };
    int m_idxMark{ -1 };
    int m_idxFocus{ -1 };
    int m_oyTop{};

    int m_cyItem{ 24 };
    int m_cyFont{};

    BITBOOL m_bMultiSel : 1 = FALSE;	// 多选
    BITBOOL m_bExtendSel : 1 = FALSE;	// 扩展多选
    BITBOOL m_bAllowDrag : 1 = FALSE;	// 允许拖放项目
    BITBOOL m_bAutoItemHeight : 1 = TRUE;		// 自动计算项目高度

#ifdef _DEBUG
    BITBOOL m_bDbgDrawMarkItem : 1 = 0;	// [调试]绘制标记项目
#endif
    BITBOOL m_bHasFocus : 1 = FALSE;	// 是否有焦点
    BITBOOL m_bLBtnDown : 1 = FALSE;	// 鼠标左键已按下
    BITBOOL m_bRBtnDown : 1 = FALSE;	// 鼠标右键已按下
    BITBOOL m_bFocusIndicatorVisible : 1 = Dbg;	// 焦点指示器是否可见
    BITBOOL m_bNmDragging : 1 = FALSE;			// 正在拖放项目，产生NM_LBN_BEGINDRAG时设置为TRUE
    BITBOOL m_bTrackComboBoxList : 1 = FALSE;	// 正在作为组合框的下拉列表显示
    BITBOOL m_bProtectCapture : 1 = FALSE;		// 允许其他窗口占用鼠标捕获，通常用于弹出下拉列表时显示菜单等
    BITBOOL m_bGenItemNotify : 1 = FALSE;		// 是否生成项目通知

    int m_iDpi{ USER_DEFAULT_SCREEN_DPI };

    void UpdateFontMetrics() noexcept
    {
        TEXTMETRICW tm;
        GetTextMetricsW(m_DC.GetDC(), &tm);
        m_cyFont = tm.tmHeight;
    }

    void UpdateDefaultItemHeight() noexcept
    {
        EckAssert(m_bAutoItemHeight);
        m_cyItem = m_cyFont + DpiScale(MetricsExtraV, m_iDpi);
    }

    LRESULT NotifyItemChanged(int idxBegin, int idxEnd, UINT uOldFlags, UINT uNewFlags) noexcept
    {
        if (!m_bGenItemNotify)
            return 0;
        NMLBNITEMCHANGED nm;
        nm.idx = idxBegin;
        nm.idxEnd = idxEnd;
        nm.uFlagsOld = uOldFlags;
        nm.uFlagsNew = uNewFlags;
        return FillNmhdrAndSendNotify(nm, m_hParent, NM_LBN_ITEMCHANGED);
    }

    LRESULT NotifyItemChanged(int idx, UINT uOldFlags, UINT uNewFlags) noexcept
    {
        return NotifyItemChanged(idx, idx, uOldFlags, uNewFlags);
    }

    BOOL OnCreate(HWND hWnd, CREATESTRUCTW* pcs) noexcept
    {
        m_hParent = pcs->hwndParent;

        m_iDpi = GetDpi(hWnd);

        m_DC.Create(hWnd);
        SetBkMode(m_DC.GetDC(), TRANSPARENT);

        SetItemsViewTheme();
        m_hTheme = OpenThemeData(hWnd, L"ListView");
        return TRUE;
    }

    void OnPaint(HWND hWnd, WPARAM wParam) noexcept
    {
        const auto* const ptc = PtcCurrent();
        PAINTSTRUCT ps;
        BeginPaint(hWnd, wParam, ps);

        LRESULT lRet;
        NMCUSTOMDRAWEXT ne;
        FillNmhdr(ne, NM_CUSTOMDRAW);
        ne.hdc = m_DC.GetDC();
        ne.lItemlParam = 0;
        ne.crBk = CLR_DEFAULT;
        ne.crText = CLR_DEFAULT;
        ne.iStateId = 0;
        ne.iPartId = 0;

        ne.dwDrawStage = CDDS_PREERASE;
        ne.rc = ps.rcPaint;
        ne.dwItemSpec = 0;
        ne.uItemState = 0;
        ne.lItemlParam = 0;
        lRet = SendNotify(ne, m_hParent);
        if (!(lRet & CDRF_SKIPDEFAULT))
        {
            if (ne.crBk != CLR_DEFAULT)
                SetDCBrushColor(ne.hdc, ne.crBk);
            else
                SetDCBrushColor(ne.hdc,
                    (m_crBkg == CLR_DEFAULT) ? ptc->crDefBkg : m_crBkg);
            FillRect(ne.hdc, &ps.rcPaint, GetStockBrush(DC_BRUSH));
        }
        if (lRet & CDRF_NOTIFYPOSTERASE)
        {
            ne.dwDrawStage = CDDS_POSTERASE;
            SendNotify(ne, m_hParent);
        }

        if (!GetItemCount())
            goto SkipDrawItem;

        ne.dwDrawStage = CDDS_PREPAINT;
        lRet = SendNotify(ne, m_hParent);
        if (!(lRet & CDRF_SKIPDEFAULT))
        {
            const auto idxTop = (int)std::max(m_idxTop + (int)ps.rcPaint.top / m_cyItem - 1, m_idxTop);
            const auto idxBottom = (int)std::min(m_idxTop + (int)ps.rcPaint.bottom / m_cyItem + 1,
                GetItemCount() - 1);
            if (idxTop >= 0 && idxBottom >= 0)
            {
                GetItemRect(idxTop, ne.rc);
                for (ne.dwItemSpec = idxTop; ne.dwItemSpec <= idxBottom;
                    ++ne.dwItemSpec)
                {
                    PaintItem(ne, lRet & CDRF_NOTIFYITEMDRAW);
                    ne.rc.top += m_cyItem;
                    ne.rc.bottom += m_cyItem;
                }
            }
        }
        if (lRet & CDRF_NOTIFYPOSTPAINT)
        {
            ne.dwDrawStage = CDDS_POSTPAINT;
            SendNotify(ne, m_hParent);
        }
    SkipDrawItem:
        BitBltPs(&ps, ne.hdc);
        EndPaint(hWnd, wParam, ps);
    }

    void OnVScroll(HWND hWnd, HWND hCtrl, UINT uCode, int iPos) noexcept
    {
        SCROLLINFO si;
        si.cbSize = sizeof(si);
        si.fMask = SIF_ALL;
        ScbGetInfomation(SB_VERT, &si);
        const int yOld = si.nPos;
        switch (uCode)
        {
        case SB_TOP:
            si.nPos = si.nMin;
            break;
        case SB_BOTTOM:
            si.nPos = si.nMax;
            break;
        case SB_LINEUP:
            si.nPos -= m_cyItem;
            break;
        case SB_LINEDOWN:
            si.nPos += m_cyItem;
            break;
        case SB_PAGEUP:
            si.nPos -= si.nPage;
            break;
        case SB_PAGEDOWN:
            si.nPos += si.nPage;
            break;
        case SB_THUMBTRACK:
            si.nPos = si.nTrackPos;
            break;
        }

        si.fMask = SIF_POS;
        ScbSetInfomation(SB_VERT, &si);
        ScbGetInfomation(SB_VERT, &si);
        ReCalculateTopItem();
        if (si.nPos != yOld)
        {
            ScrollWindowEx(hWnd, 0, yOld - si.nPos, nullptr, nullptr,
                nullptr, nullptr, SW_INVALIDATE);
            UpdateWindow(hWnd);
        }
    }

    void SelectItemForClick(int idx) noexcept
    {
        int idxChangedBegin = -1, idxChangedEnd = -1;
        const int idxOldFocus = m_idxFocus;
        if (idx >= 0)
            m_idxFocus = idx;
        if (m_bExtendSel)
        {
            if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
            {
                if (idx >= 0)
                {
                    m_idxMark = idx;
                    const auto uOld = m_SelRange.IsSelected(idx) ? LBN_IF_SEL : 0;
                    m_SelRange.InvertRange(idx, idx);
                    NotifyItemChanged(idx, uOld, uOld ^ LBN_IF_SEL);
                    RedrawItem(idx);
                    if (idxOldFocus >= 0 && idxOldFocus != idx)
                        RedrawItem(idxOldFocus);
                }
            }
            else if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
            {
                if (m_idxMark >= 0 && idx >= 0)
                    SelectRangeForClick(std::min(m_idxMark, idx),
                        std::max(m_idxMark, idx));
            }
            else
            {
                DeselectAll(idxChangedBegin, idxChangedEnd);
                if (idxChangedBegin >= 0)
                    RedrawItem(idxChangedBegin, idxChangedEnd);
                if (idx >= 0)
                {
                    m_SelRange.IncludeRange(idx, idx);
                    m_idxMark = idx;
                    NotifyItemChanged(idx, 0u, LBN_IF_SEL);
                    if (idxChangedBegin < 0 || (idx < idxChangedBegin || idx > idxChangedEnd))
                        RedrawItem(idx);
                }
            }
        }
        else if (m_bMultiSel)
        {
            if (idx >= 0)
            {
                const auto uOld = m_SelRange.IsSelected(idx) ? LBN_IF_SEL : 0;
                m_SelRange.InvertRange(idx, idx);
                NotifyItemChanged(idx, uOld, uOld ^ LBN_IF_SEL);
                RedrawItem(idx);
                if (idxOldFocus >= 0 && idxOldFocus != idx)
                    RedrawItem(idxOldFocus);
            }
        }
        else
        {
            if (m_idxSel != idx)
            {
                std::swap(m_idxSel, idx);
                NotifyItemChanged(m_idxSel, 0u, LBN_IF_SEL);
                NotifyItemChanged(idx, LBN_IF_SEL, 0u);
                if (m_idxSel >= 0)
                    RedrawItem(m_idxSel);
                if (idx >= 0)
                    RedrawItem(idx);
            }
        }
    }

    void SelectRangeForClick(int idxBegin, int idxEnd) noexcept
    {
        EckAssert(m_bExtendSel);
        int i;
        int idx0 = -1, idx1 = -1;
        // 清除前面选中
        for (const auto& e : m_SelRange.GetList())
        {
            if (e.idxEnd < idxBegin)
            {
                if (idx0 < 0)
                    idx0 = e.idxBegin;
                idx1 = e.idxEnd;
                NotifyItemChanged(e.idxBegin, e.idxEnd, LBN_IF_SEL, 0u);
            }
            else if (e.idxBegin < idxBegin)
            {
                if (idx0 < 0)
                    idx0 = e.idxBegin;
                idx1 = idxBegin - 1;
                NotifyItemChanged(e.idxBegin, idxBegin - 1, LBN_IF_SEL, 0u);
                break;
            }
        }
        if (idxBegin > 0)
            m_SelRange.ExcludeRange(0, idxBegin - 1);
        // 范围选中
        for (i = idxBegin; i <= idxEnd; ++i)
        {
            if (!m_SelRange.IsSelected(i))
            {
                if (idx0 < 0)
                    idx0 = i;
                idx1 = i;
                NotifyItemChanged(i, i, 0u, LBN_IF_SEL);
            }
        }
        m_SelRange.IncludeRange(idxBegin, idxEnd);
        // 清除后面选中
        for (auto it = m_SelRange.GetList().rbegin(); it != m_SelRange.GetList().rend(); ++it)
        {
            if (it->idxBegin > idxEnd)
            {
                if (idx0 < 0)
                    idx0 = it->idxBegin;
                idx1 = it->idxEnd;
                NotifyItemChanged(it->idxBegin, it->idxEnd, LBN_IF_SEL, 0u);
            }
            else if (it->idxEnd > idxEnd)
            {
                if (idx0 < 0)
                    idx0 = idxEnd + 1;
                idx1 = it->idxEnd;
                NotifyItemChanged(idxEnd + 1, it->idxEnd, LBN_IF_SEL, 0u);
                break;
            }
        }
        if (idxEnd < GetItemCount() - 1)
            m_SelRange.ExcludeRange(idxEnd + 1, GetItemCount() - 1);
        if (idx0 >= 0)
            RedrawItem(idx0, idx1);
    }

    void BeginDraggingSelect(int idxBegin) noexcept
    {
        MSG msg;
        const auto srOld = m_SelRange;
        int idxOldSelBegin = -1, idxOldSelEnd = -1,
            idxOld0 = idxBegin, idxOld1 = -1;
        while (GetCapture() == HWnd)// 如果捕获改变则应立即退出拖动循环
        {
            if (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
            {
                switch (msg.message)
                {
                case WM_LBUTTONUP:
                case WM_LBUTTONDOWN:
                case WM_RBUTTONUP:
                case WM_RBUTTONDOWN:
                case WM_MBUTTONUP:
                case WM_MBUTTONDOWN:
                    goto ExitDraggingLoop;

                case WM_KEYDOWN:
                    if (msg.wParam == VK_ESCAPE)// ESC退出拖放是银河系的惯例
                        goto ExitDraggingLoop;
                    [[fallthrough]];
                case WM_CHAR:
                case WM_KEYUP:
                    break;// eat it

                case WM_MOUSEWHEEL:
                {
                    SetRedraw(FALSE);// 暂时禁止重画
                    TranslateMessage(&msg);
                    DispatchMessageW(&msg);
                    SetRedraw(TRUE);
                }
                break;

                case WM_MOUSEMOVE:
                {
                    const POINT pt ECK_GET_PT_LPARAM(msg.lParam);
                    //----------滚动
                    int yDelta = 0;
                    if (pt.y < 0)
                        yDelta = pt.y;
                    else if (pt.y > m_cyClient)
                        yDelta = pt.y - m_cyClient;

                    if (yDelta)
                    {
                        SetRedraw(FALSE);
                        ScrollV(yDelta);
                        SetRedraw(TRUE);
                    }
                    //----------选中
                    int idxCurr = HitTest(0, pt.y);
                    if (idxCurr < 0)
                    {
                        if (pt.y <= 0)
                            idxCurr = m_idxTop;
                        else
                        {
                            idxCurr = m_idxTop + (m_cyClient - m_oyTop) / m_cyItem;
                            if (idxCurr >= GetItemCount())
                                idxCurr = GetItemCount() - 1;
                        }
                    }

                    if (m_bExtendSel)
                    {
                        // 闭区间
                        const int idxSelBegin = std::min(m_idxMark, idxCurr),
                            idxSelEnd = std::max(m_idxMark, idxCurr);
                        // 闭区间
                        const int idx0 = std::min(idxSelBegin, idxOld0),
                            idx1 = std::max(idxSelEnd, idxOld1);
                        if (idxOldSelBegin != idxSelBegin || idxOldSelEnd != idxSelEnd ||
                            idxOld0 != idx0 || idxOld1 != idx1)
                        {
                            for (int i = idx0; i <= idx1; ++i)
                            {
                                if (idxSelBegin <= i && i <= idxSelEnd)// 当前应选中
                                {
                                    if (!m_SelRange.IsSelected(i))
                                    {
                                        m_SelRange.IncludeRange(i, i);
                                        NotifyItemChanged(i, 0u, LBN_IF_SEL);
                                    }
                                }
                                else
                                {
                                    if (srOld.IsSelected(i))
                                    {
                                        if (!m_SelRange.IsSelected(i))
                                        {
                                            m_SelRange.IncludeRange(i, i);
                                            NotifyItemChanged(i, 0u, LBN_IF_SEL);
                                        }
                                    }
                                    else
                                    {
                                        if (m_SelRange.IsSelected(i))
                                        {
                                            m_SelRange.ExcludeRange(i, i);
                                            NotifyItemChanged(i, LBN_IF_SEL, 0u);
                                        }
                                    }
                                }
                            }
                            idxOld0 = idx0;
                            idxOld1 = idx1;
                            idxOldSelBegin = idxSelBegin;
                            idxOldSelEnd = idxSelEnd;
                        }
                    }
                    else
                    {
                        SelectItemForClick(idxCurr);
                    }

                    Redraw();
                    UpdateWindow(HWnd);
                }
                break;

                case WM_QUIT:
                    PostQuitMessage((int)msg.wParam);// re-throw
                    goto ExitDraggingLoop;

                default:
                    TranslateMessage(&msg);
                    DispatchMessageW(&msg);
                    break;
                }
            }
            else
                WaitMessage();
        }
    ExitDraggingLoop:
        ReleaseCapture();
        m_bLBtnDown = FALSE;
    }

    void OnLButtonDown(HWND hWnd, BOOL bDoubleClick, int x, int y, UINT uKeyFlags) noexcept
    {
        if (m_hComboBox)
        {
            EckAssert(m_bTrackComboBoxList);
            RECT rc;
            GetWindowRect(hWnd, &rc);
            ScreenToClient(hWnd, &rc);
            if (!PtInRect(rc, POINT{ x,y }))// 光标在窗口外，关闭列表
            {
                NMHDR nm;
                FillNmhdr(nm, NM_LBN_DISMISS);
                SendMessageW(m_hComboBox, WM_NOTIFY, nm.idFrom, (LPARAM)&nm);
                return;
            }
            else if (rc = { 0,0,m_cxClient,m_cyClient };
                !PtInRect(rc, POINT{ x,y }))// 试图拖动滚动条
            {
                POINT ptScr{ x,y };
                ClientToScreen(hWnd, &ptScr);
                EckAssert(GetCapture() == hWnd);
                m_bLBtnDown = FALSE;
                CbBeginProtectCapture();
                ReleaseCapture();
                __super::OnMessage(hWnd, WM_NCLBUTTONDOWN,
                    __super::OnMessage(hWnd, WM_NCHITTEST, 0, POINTTOPOINTS(ptScr)),
                    POINTTOPOINTS(ptScr));
                SetCapture(hWnd);
                CbEndProtectCapture();
                return;
            }
            else// 通常情况
            {
                m_bLBtnDown = TRUE;

                const int idx = HitTest(x, y);
                if (idx < 0)
                    return;
                POINT ptScr{ x,y };
                ClientToScreen(hWnd, &ptScr);
                SelectItemForClick(idx);
                return;
            }
        }
        else
            SetFocus(hWnd);

        const int idx = HitTest(x, y);
        if (idx < 0)
            return;

        POINT ptScr{ x,y };
        ClientToScreen(hWnd, &ptScr);
        SelectItemForClick(idx);
        /*
        * 拖动动作：
        * 模式		禁止拖放时	允许拖放时
        * ------------------------------
        * 单选		跟随选中		无
        * 多选		无			无
        * 扩展多选	范围选择		无
        */
        if (IsMouseMovedBeforeDragging(hWnd, ptScr.x, ptScr.y, 0))// YEILD, ReleaseCapture
        {
            if (!IsValid())// revalidate
                return;
            m_bLBtnDown = TRUE;
            SetCapture(hWnd);

            int idxHot{ -1 };
            std::swap(idxHot, m_idxHot);
            if (idxHot >= 0)
                RedrawItem(idxHot);

            if (m_bAllowDrag)
            {
                m_bNmDragging = TRUE;
                NMLBNDRAG nm;
                nm.idx = idx;
                nm.uKeyFlags = uKeyFlags;
                FillNmhdrAndSendNotify(nm, m_hParent, NM_LBN_BEGINDRAG);
            }
            else if (m_bExtendSel || (!m_bExtendSel && !m_bMultiSel))// 扩展多选或单选
                BeginDraggingSelect(idx);
        }
    }

    void OnMouseWheel(HWND hWnd, int xPos, int yPos, int zDelta, UINT uKeys) noexcept
    {
        SCROLLINFO si;
        si.cbSize = sizeof(SCROLLINFO);
        si.fMask = SIF_POS;
        ScbGetInfomation(SB_VERT, &si);
        const int yOld = si.nPos;
        UINT cScrollLines;
        SystemParametersInfoW(SPI_GETWHEELSCROLLLINES, 0, &cScrollLines, 0);
        int d = -zDelta / WHEEL_DELTA * (m_cyItem * cScrollLines);
        if (d > m_cyClient)
            d = m_cyClient * 5 / 6;
        si.nPos += d;
        ScbSetInfomation(SB_VERT, &si);
        ScbGetInfomation(SB_VERT, &si);
        ReCalculateTopItem();
        if (si.nPos != yOld)
        {
            ScrollWindowEx(hWnd, 0, yOld - si.nPos, nullptr, nullptr,
                nullptr, nullptr, SW_INVALIDATE);
            UpdateWindow(hWnd);
        }
    }

    void ReCalculateTopItem() noexcept
    {
        const int ySB = ScbGetPosition(SB_VERT);
        m_idxTop = ySB / m_cyItem;
        m_oyTop = m_idxTop * m_cyItem - ySB;
    }

    void PaintItem(NMCUSTOMDRAWEXT& ne, BOOL bNotifyItemDraw) noexcept
    {
        const auto* const ptc = PtcCurrent();
        const int idx = (int)ne.dwItemSpec;
        int iState{};
        ne.crText = ptc->crDefText;
        if (m_idxHot == idx)
            if (IsItemSelected(idx))
            {
                iState = LISS_HOTSELECTED;
                ne.crText = ptc->crHiLightText;
            }
            else
                iState = LISS_HOT;
        else
            if (IsItemSelected(idx))
            {
                iState = LISS_SELECTED;
                ne.crText = ptc->crHiLightText;
            }

        ne.dwDrawStage = CDDS_ITEMPREPAINT;
        ne.crBk = CLR_DEFAULT;
        ne.iStateId = iState;
        ne.iPartId = LVP_LISTITEM;
        const auto lRet = bNotifyItemDraw ? SendNotify(ne, m_hParent) : 0;
        if (!(lRet & CDRF_SKIPDEFAULT))
        {
            BOOL bFillBk{};
            if (ne.crBk != CLR_DEFAULT)
            {
                if (ShouldAppsUseDarkMode())
                    bFillBk = TRUE;
                else
                {
                    SetDCBrushColor(ne.hdc, ne.crBk);
                    FillRect(ne.hdc, &ne.rc, GetStockBrush(DC_BRUSH));
                }
            }
            if (iState)
                DrawThemeBackground(m_hTheme, ne.hdc,
                    LVP_LISTITEM, iState, &ne.rc, nullptr);
            if (bFillBk)
                AlphaBlendColor(ne.hdc, ne.rc, ne.crBk);

            NMLBNGETDISPINFO nm{};
            nm.Item.idxItem = idx;
            if (RequestItem(nm) && nm.Item.cchText > 0)
            {
                RECT rc{ ne.rc };
                rc.left += DaGetSystemMetrics(SM_CXEDGE, m_iDpi);
                SetTextColor(ne.hdc, ne.crText);
                DrawTextW(ne.hdc, nm.Item.pszText, nm.Item.cchText, &rc,
                    DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX | DT_NOCLIP | DT_END_ELLIPSIS);
            }

            if (!(lRet & CDRF_SKIPPOSTPAINT) &&
                m_bFocusIndicatorVisible &&
                m_idxFocus == idx)
            {
                RECT rc{ ne.rc };
                InflateRect(rc, -2, -2);
                DrawFocusRect(ne.hdc, &rc);
            }
        }
#ifdef _DEBUG
        if (m_bDbgDrawMarkItem && idx == m_idxMark)
        {
            const auto crOld = SetTextColor(ne.hdc, Colorref::Red);
            DrawTextW(ne.hdc, L"Mark", -1, (RECT*)&ne.rc,
                DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX | DT_NOCLIP);
            SetTextColor(ne.hdc, crOld);
        }
#endif
        if (lRet & CDRF_NOTIFYPOSTPAINT)
        {
            ne.dwDrawStage = CDDS_ITEMPOSTPAINT;
            SendNotify(ne, m_hParent);
        }
    }

    void ReCalculateScrollBar() noexcept
    {
        SCROLLINFO si;
        si.cbSize = sizeof(SCROLLINFO);
        si.fMask = SIF_PAGE | SIF_RANGE;
        si.nMin = 0;
        si.nMax = GetItemCount() * m_cyItem;
        si.nPage = m_cyClient;
        ScbSetInfomation(SB_VERT, &si);
        ReCalculateTopItem();
    }

    void CheckOldData() noexcept
    {
        const int c = GetItemCount();
        if (m_idxHot >= c)
            m_idxHot = -1;
        if (m_idxFocus >= c)
            m_idxFocus = -1;
        if (m_idxMark >= c)
            m_idxMark = -1;
        if (m_idxSel >= c)
            m_idxSel = -1;
        m_SelRange.ExcludeRange(c, INT_MAX);
    }

    LRESULT CbNotifyDismiss() noexcept
    {
        NMHDR nm;
        return FillNmhdrAndSendNotify(nm, m_hParent, NM_LBN_DISMISS);
    }
public:
    LRESULT OnMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept override
    {
        switch (uMsg)
        {
        case WM_MOUSELEAVE:
        {
            int t = -1;
            std::swap(t, m_idxHot);
            RECT rcItem;
            if (t >= 0)
            {
                GetItemRect(t, rcItem);
                Redraw(rcItem);
            }
        }
        break;

        case WM_MOUSEMOVE:
        {
            if (!m_bLBtnDown)
            {
                const POINT pt ECK_GET_PT_LPARAM(lParam);
                int idxHot = HitTest(pt.x, pt.y);
                if (idxHot == m_idxHot)
                    break;
                std::swap(idxHot, m_idxHot);
                if (idxHot >= 0)
                    RedrawItem(idxHot);
                if (m_idxHot >= 0)
                    RedrawItem(m_idxHot);
            }

            TRACKMOUSEEVENT tme;
            tme.cbSize = sizeof(TRACKMOUSEEVENT);
            tme.dwFlags = TME_LEAVE;
            tme.hwndTrack = hWnd;
            TrackMouseEvent(&tme);
        }
        break;

        case WM_SIZE:
        {
            ECK_GET_SIZE_LPARAM(m_cxClient, m_cyClient, lParam);
            ScbSetPage(SB_VERT, m_cyClient);
            ReCalculateTopItem();
            m_DC.ReSize(hWnd, m_cxClient, m_cyClient);
        }
        break;

        case WM_PRINTCLIENT:
        case WM_PAINT:
            OnPaint(hWnd, wParam);
            return 0;

        case WM_VSCROLL:
            return HANDLE_WM_VSCROLL(hWnd, wParam, lParam, OnVScroll);

        case WM_MOUSEWHEEL:
            HANDLE_WM_MOUSEWHEEL(hWnd, wParam, lParam, OnMouseWheel);
            break;

        case WM_RBUTTONDOWN:
        {
            if (m_hComboBox)
            {
                const RECT rcClient{ 0,0,m_cxClient,m_cyClient };
                if (!PtInRect(&rcClient, ECK_GET_PT_LPARAM(lParam)))
                    break;
                m_bRBtnDown = TRUE;
                // 无需捕获鼠标
                EckAssert(m_bTrackComboBoxList);
            }
            else
            {
                m_bRBtnDown = TRUE;
                SetCapture(hWnd);
            }
        }
        break;

        case WM_RBUTTONUP:
        {
            const RECT rcClient{ 0,0,m_cxClient,m_cyClient };
            if (m_hComboBox)
            {
                if (!PtInRect(&rcClient, ECK_GET_PT_LPARAM(lParam)))// 客户区之外，可能正在右击滚动条
                {
                    CbBeginProtectCapture();
                    const auto lResult = __super::OnMessage(hWnd, uMsg, wParam, lParam);
                    SetCapture(hWnd);
                    CbEndProtectCapture();
                    return lResult;
                }
            }
            if (m_bRBtnDown)
            {
                if (!m_hComboBox)
                    ReleaseCapture();
                m_bRBtnDown = FALSE;
                NMMOUSENOTIFY nm;
                nm.pt = ECK_GET_PT_LPARAM(lParam);
                nm.uKeyFlags = (UINT)wParam;
                FillNmhdrAndSendNotify(nm, m_hParent, NM_RCLICK);
            }
        }
        break;

        case WM_LBUTTONDOWN:
            HANDLE_WM_LBUTTONDOWN(hWnd, wParam, lParam, OnLButtonDown);
            break;

        case WM_LBUTTONUP:
        {
            if (m_bLBtnDown)
            {
                m_bLBtnDown = FALSE;
                POINT pt ECK_GET_PT_LPARAM(lParam);
                ReleaseCapture();
                if (!m_hComboBox && m_bNmDragging)
                {
                    NMLBNDRAG nm;
                    nm.idx = HitTest(pt.x, pt.y);
                    nm.uKeyFlags = (UINT)wParam;
                    FillNmhdrAndSendNotify(nm, m_hParent, NM_LBN_ENDDRAG);
                }
            }
        }
        break;

        case WM_KEYDOWN:
        {
            switch (wParam)
            {
            case VK_DOWN:
            case VK_RIGHT:
            {
                EckAssert(m_idxFocus < GetItemCount());
                if (m_idxFocus < 0)
                    break;
                if (m_idxFocus == GetItemCount() - 1)
                {
                    EnsureVisible(m_idxFocus);
                    break;
                }
                ++m_idxFocus;
                if (m_bExtendSel)
                    SelectItemForClick(m_idxFocus);
                else if (m_bMultiSel)
                {
                    if (m_bFocusIndicatorVisible)
                        RedrawItem(m_idxFocus - 1, m_idxFocus);
                }
                else
                    SelectItemForClick(m_idxFocus);
                EnsureVisible(m_idxFocus);
            }
            return 0;

            case VK_UP:
            case VK_LEFT:
            {
                EckAssert(m_idxFocus < GetItemCount());
                if (m_idxFocus < 0)
                    break;
                if (m_idxFocus == 0)
                {
                    EnsureVisible(m_idxFocus);
                    break;
                }
                --m_idxFocus;
                if (m_bExtendSel)
                    SelectItemForClick(m_idxFocus);
                else if (m_bMultiSel)
                {
                    if (m_bFocusIndicatorVisible)
                        RedrawItem(m_idxFocus, m_idxFocus + 1);
                }
                else
                    SelectItemForClick(m_idxFocus);
                EnsureVisible(m_idxFocus);
            }
            return 0;

            case VK_SPACE:
            {
                if (m_bMultiSel && m_idxFocus >= 0)
                {
                    const auto uOld = m_SelRange.IsSelected(m_idxFocus) ? LBN_IF_SEL : 0;
                    m_SelRange.InvertRange(m_idxFocus, m_idxFocus);
                    NotifyItemChanged(m_idxFocus, uOld, uOld ^ LBN_IF_SEL);
                    RedrawItem(m_idxFocus);
                }
            }
            return 0;

            case VK_PRIOR:
            {
                const int idxOldFocus = m_idxFocus;
                int idx = m_idxFocus;
                if (idx == m_idxTop)
                {
                    idx -= ((m_cyClient - (m_cyItem + m_oyTop)) / m_cyItem);
                    if (idx < 0)
                        idx = 0;
                }
                else
                    idx = m_idxTop;

                if (m_bExtendSel)
                    SelectItemForClick(idx);
                else if (m_bMultiSel)
                {
                    m_idxFocus = idx;
                    if (m_idxFocus >= 0)
                        RedrawItem(m_idxFocus);
                    if (idxOldFocus >= 0 && idxOldFocus != m_idxFocus)
                        RedrawItem(idxOldFocus);
                }
                else
                    SelectItemForClick(idx);
                EnsureVisible(idx);
            }
            return 0;
            case VK_NEXT:
            {
                int idxBottom = m_idxTop + (m_cyClient - (m_cyItem + m_oyTop)) / m_cyItem;
                if (idxBottom >= GetItemCount())
                    idxBottom = GetItemCount() - 1;

                const int idxOldFocus = m_idxFocus;
                int idx = m_idxFocus;
                if (idx == idxBottom)
                {
                    idx += (m_cyClient / m_cyItem);
                    if (idx >= GetItemCount())
                        idx = GetItemCount() - 1;
                }
                else
                    idx = idxBottom;

                if (m_bExtendSel)
                    SelectItemForClick(idx);
                else if (m_bMultiSel)
                {
                    m_idxFocus = idx;
                    if (m_idxFocus >= 0)
                        RedrawItem(m_idxFocus);
                    if (idxOldFocus >= 0 && idxOldFocus != m_idxFocus)
                        RedrawItem(idxOldFocus);
                }
                else
                    SelectItemForClick(idx);
                EnsureVisible(idx);
            }
            return 0;
            }
        }
        break;

        case WM_CAPTURECHANGED:
        {
            if (m_hComboBox)
            {
                m_bLBtnDown = FALSE;
                m_bRBtnDown = FALSE;
                if (!m_bProtectCapture)
                    CbNotifyDismiss();
            }
            else
            {
                m_bRBtnDown = FALSE;
                if (m_bLBtnDown)
                {
                    POINT pt;
                    GetCursorPos(&pt);
                    ScreenToClient(hWnd, &pt);
                    OnMessage(hWnd, WM_LBUTTONUP, 0, POINTTOPOINTS(pt));
                }
            }
        }
        break;

        case WM_SETFOCUS:
        {
            m_bHasFocus = TRUE;

            NMFOCUS nm;
            nm.hWnd = (HWND)wParam;
            FillNmhdrAndSendNotify(nm, m_hParent, NM_SETFOCUS);
        }
        break;

        case WM_KILLFOCUS:
        {
            m_bHasFocus = FALSE;

            NMFOCUS nm;
            nm.hWnd = (HWND)wParam;
            FillNmhdrAndSendNotify(nm, m_hParent, NM_KILLFOCUS);
        }
        break;

        case WM_SETFONT:
        {
            m_hFont = (HFONT)wParam;
            SelectObject(m_DC.GetDC(), m_hFont);
            UpdateFontMetrics();
            if (m_bAutoItemHeight)
            {
                UpdateDefaultItemHeight();
                ReCalculateScrollBar();
            }
            if (lParam)
                Redraw();
        }
        return 0;

        case WM_GETFONT:
            return (LRESULT)m_hFont;

        case WM_THEMECHANGED:
        {
            CloseThemeData(m_hTheme);
            m_hTheme = OpenThemeData(hWnd, L"ListView");
        }
        return 0;

        case WM_DPICHANGED_BEFOREPARENT:
        {
            m_iDpi = GetDpi(hWnd);
            if (m_bAutoItemHeight)
            {
                UpdateDefaultItemHeight();
                ReCalculateScrollBar();
            }
        }
        return 0;

        case WM_CREATE:
            HANDLE_WM_CREATE(hWnd, wParam, lParam, OnCreate);
            break;

        case WM_DESTROY:
        {
            CloseThemeData(m_hTheme);
            m_hTheme = nullptr;
            m_hFont = nullptr;
            m_cyItem = 24;
            m_idxSel = m_idxHot = m_idxTop = m_idxMark = m_idxFocus = -1;
            m_oyTop = 0;
            m_SelRange.Clear();
            m_bHasFocus = m_bLBtnDown =
                m_bNmDragging = m_bTrackComboBoxList = FALSE;
            m_hComboBox = nullptr;
        }
        break;
        }

        return CWindow::OnMessage(hWnd, uMsg, wParam, lParam);
    }

    void SetItemCount(int cItem) noexcept
    {
        m_cItem = cItem;
        CheckOldData();
        ReCalculateScrollBar();
        NMHDR nm;
        FillNmhdrAndSendNotify(nm, m_hParent, NM_LBN_ITEMSTANDBY);
    }

    EckInlineNdCe int GetItemCount() noexcept { return m_cItem; }

    [[nodiscard]] int HitTest(int x, int y) noexcept
    {
        if (x < 0 || x > m_cxClient || y < 0 || y > m_cyClient)
            return -1;
        const int idx = m_idxTop + (y - m_oyTop) / m_cyItem;
        if (idx >= GetItemCount())
            return -1;
        else
            return idx;
    }

    EckInlineNdCe int GetItemY(int idx) noexcept
    {
        return m_oyTop + (m_cyItem * (idx - m_idxTop));
    }

    EckInline void GetItemRect(int idx, RECT& rc) noexcept
    {
        rc = { 0,GetItemY(idx),m_cxClient };
        rc.bottom = rc.top + m_cyItem;
    }

    void SetCurrentSelection(int idx) noexcept
    {
        EckAssert(idx < GetItemCount());
        if (idx < 0)
        {
            if (m_bMultiSel)
            {
                int idx0, idx1;
                DeselectAll(idx0, idx1);
                if (idx0 >= 0)
                    RedrawItem(idx0, idx1);
            }
            else
            {
                int idx{ -1 };
                std::swap(idx, m_idxSel);
                if (idx >= 0)
                    RedrawItem(idx);
            }
        }
        else
            SelectItemForClick(idx);
    }

    EckInlineNdCe int GetCurrentSelection() const noexcept { return m_idxSel; }

    EckInlineNdCe BOOL IsItemSelected(int idx) noexcept
    {
        if (m_bMultiSel || m_bExtendSel)
            return m_SelRange.IsSelected(idx);
        else
            return m_idxSel == idx;
    }

    EckInlineNdCe auto& GetSelectionRange() const noexcept { return m_SelRange; }
    EckInlineNdCe auto& GetSelectionRange() noexcept { return m_SelRange; }

    void EnsureVisible(int idx) noexcept
    {
        if (idx < 0 || idx >= GetItemCount())
            return;
        RECT rc;
        GetItemRect(idx, rc);
        if (rc.bottom >= m_cyClient)
            ScrollV(rc.bottom - m_cyClient);
        else if (rc.top <= 0)
            ScrollV(rc.top);
    }

    void ScrollV(int yDelta) noexcept
    {
        SCROLLINFO si;
        si.cbSize = sizeof(si);
        si.fMask = SIF_ALL;
        ScbGetInfomation(SB_VERT, &si);
        const int yOld = si.nPos;
        si.nPos += yDelta;
        si.fMask = SIF_POS;
        ScbSetInfomation(SB_VERT, &si);
        ScbGetInfomation(SB_VERT, &si);
        ReCalculateTopItem();
        ScrollWindow(HWnd, 0, yOld - si.nPos, nullptr, nullptr);
        UpdateWindow(HWnd);
    }

    void RedrawItem(int idx) noexcept
    {
        EckAssert(idx >= 0 && idx < GetItemCount());
        RECT rc;
        GetItemRect(idx, rc);
        Redraw(rc);
    }

    void RedrawItem(int idxFrom, int idxTo) noexcept
    {
        EckAssert(idxFrom >= 0 && idxFrom < GetItemCount());
        EckAssert(idxTo >= 0 && idxTo < GetItemCount());
        EckAssert(idxFrom <= idxTo);
        RECT rc;
        GetItemRect(idxFrom, rc);
        rc.bottom += (idxTo - idxFrom) * m_cyItem;
        Redraw(rc);
    }

    void DeselectAll(int& idxChangedBegin, int& idxChangedEnd) noexcept
    {
        int idx0 = -1, idx1 = -1;
        if (!m_SelRange.GetList().empty())
        {
            idx0 = m_SelRange.GetList().front().idxBegin;
            idx1 = m_SelRange.GetList().back().idxEnd;
        }
        for (const auto& e : m_SelRange.GetList())
            NotifyItemChanged(e.idxBegin, e.idxEnd, LBN_IF_SEL, 0);

        m_SelRange.Clear();
        idxChangedBegin = idx0;
        idxChangedEnd = idx1;
    }

    void DeselectAll() noexcept
    {
        int idx0, idx1;
        DeselectAll(idx0, idx1);
        if (idx0 >= 0)
            RedrawItem(idx0, idx1);
    }

    void SetItemHeight(int cy) noexcept
    {
        m_bAutoItemHeight = FALSE;
        m_cyItem = cy;
        ReCalculateScrollBar();
    }

    EckInline int GetItemHeight() const noexcept { return m_cyItem; }

#pragma region 组合框交互
    EckInline void SetComboBox(HWND h) noexcept
    {
        m_hComboBox = h;
        if (h)
            m_hParent = h;
        else
            m_hParent = GetParent(HWnd);
    }

    EckInlineNdCe HWND GetComboBox() const noexcept { return m_hComboBox; }

    EckInline void CbEnterTrack() noexcept
    {
        SetCapture(HWnd);
        m_bTrackComboBoxList = TRUE;
    }

    EckInline void CbLeaveTrack() noexcept
    {
        if (m_bTrackComboBoxList)
        {
            ReleaseCapture();
            m_bTrackComboBoxList = FALSE;
        }
    }

    EckInline void CbBeginProtectCapture() noexcept
    {
        EckAssert(m_hComboBox && !m_bProtectCapture);
        m_bProtectCapture = TRUE;
    }

    EckInline void CbEndProtectCapture() noexcept
    {
        EckAssert(m_hComboBox && m_bProtectCapture);
        m_bProtectCapture = FALSE;
    }
#pragma endregion 组合框交互

    EckInline constexpr void SetMultiSel(BOOL b) noexcept { m_bMultiSel = b; }
    EckInline constexpr BOOL GetMultiSel() const noexcept { return m_bMultiSel; }

    EckInline constexpr void SetExtendSel(BOOL b) noexcept { m_bExtendSel = b; }
    EckInline constexpr BOOL GetExtendSel() const noexcept { return m_bExtendSel; }

    void SetAutoItemHeight(BOOL b) noexcept
    {
        m_bAutoItemHeight = b;
        if (b)
        {
            UpdateDefaultItemHeight();
            ReCalculateScrollBar();
        }
    }
    EckInlineNdCe BOOL GetAutoItemHeight() const noexcept { return m_bAutoItemHeight; }

    EckInlineCe void SetNotifyParentWindow(HWND h) noexcept { m_hParent = h; }

    EckInline void SetTextBufferSize(int cch) noexcept { m_rsTextBuf.ReSize(cch); }
    EckInlineNdCe int GetTextBufferSize() const noexcept { return m_rsTextBuf.Size(); }

    LRESULT RequestItem(NMLBNGETDISPINFO& nm) noexcept
    {
        EckAssert(nm.Item.idxItem >= 0 && nm.Item.idxItem < GetItemCount());
        nm.Item.pszText = m_rsTextBuf.Data();
        nm.Item.cchText = m_rsTextBuf.Size();
        return FillNmhdrAndSendNotify(nm, m_hParent, NM_LBN_GETDISPINFO);
    }

    int SearchItem(PCWSTR pszText, int cchText = -1,
        UINT uFlags = 0, int idxStart = 0) noexcept
    {
        EckAssert(idxStart >= 0 && idxStart < GetItemCount());
        if (!pszText)
            return -1;
        NMLBNSEARCH nm;
        nm.Item.pszText = pszText;
        nm.Item.cchText = cchText < 0 ? (int)wcslen(pszText) : cchText;
        nm.Item.idxItem = idxStart;
        if (FillNmhdrAndSendNotify(nm, m_hParent, NM_LBN_SEARCH))
            return nm.Item.idxItem;
        else
        {
            NMLBNGETDISPINFO nm;
            FillNmhdr(nm, NM_LBN_GETDISPINFO);
            for (nm.Item.idxItem = idxStart; nm.Item.idxItem < GetItemCount(); ++nm.Item.idxItem)
            {
                nm.Item.pszText = m_rsTextBuf.Data();
                nm.Item.cchText = m_rsTextBuf.Size();
                if (FillNmhdrAndSendNotify(nm, m_hParent, NM_LBN_GETDISPINFO))
                {
                    if (uFlags & LBN_SF_CASEINSENSITIVE)
                        if (uFlags & LBN_SF_WHOLE)
                        {
                            if (_wcsicmp(nm.Item.pszText, pszText) == 0)
                                return nm.Item.idxItem;
                        }
                        else
                        {
                            if (StrStrIW(nm.Item.pszText, pszText))
                                return nm.Item.idxItem;
                        }
                    else
                        if (uFlags & LBN_SF_WHOLE)
                        {
                            if (wcscmp(nm.Item.pszText, pszText) == 0)
                                return nm.Item.idxItem;
                        }
                        else
                        {
                            if (wcsstr(nm.Item.pszText, pszText))
                                return nm.Item.idxItem;
                        }
                }
            }
        }
        return -1;
    }

    EckInlineCe void SetGenerateItemNotify(BOOL b) noexcept { m_bGenItemNotify = b; }
    EckInlineNdCe BOOL GetGenerateItemNotify() const noexcept { return m_bGenItemNotify; }

    EckInlineNdCe HTHEME GetHTheme() const noexcept { return m_hTheme; }
};
ECK_NAMESPACE_END