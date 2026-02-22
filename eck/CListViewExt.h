#pragma once

#include "CListView.h"
#include "CHeaderExt.h"
#include "CtrlGraphics.h"
#include "GraphicsHelper.h"

#include <map>

ECK_DISABLE_ARITHMETIC_OVERFLOW_WARNING
ECK_NAMESPACE_BEGIN
struct LVE_COLOR_PACK
{
    COLORREF crDefText = CLR_DEFAULT;		// 默认文本颜色
    COLORREF crOddLineText = CLR_DEFAULT;	// 奇数行文本颜色
    COLORREF crEvenLineText = CLR_DEFAULT;	// 偶数行文本颜色
    COLORREF crOddLineBk = CLR_DEFAULT;		// 奇数行背景颜色
    COLORREF crEvenLineBk = CLR_DEFAULT;	// 偶数行背景颜色
    COLORREF crGridLineH = CLR_DEFAULT;		// 表格线水平颜色
    COLORREF crGridLineV = CLR_DEFAULT;		// 表格线垂直颜色
    COLORREF crHeaderText = CLR_DEFAULT;	// 表头文本颜色

    constexpr static LVE_COLOR_PACK Default() { return LVE_COLOR_PACK{}; }
    constexpr static LVE_COLOR_PACK Invalid()
    {
        return { CLR_INVALID, CLR_INVALID, CLR_INVALID, CLR_INVALID,
            CLR_INVALID, CLR_INVALID, CLR_INVALID, CLR_INVALID };
    }
};

constexpr inline LVE_COLOR_PACK LveLightClr1
{
    .crDefText = CLR_INVALID,
    .crOddLineText = CLR_INVALID,
    .crEvenLineText = CLR_INVALID,
    .crOddLineBk = CLR_INVALID,
    .crEvenLineBk = 0xF8F8F8,
    .crGridLineH = CLR_INVALID,
    .crGridLineV = CLR_INVALID,
    .crHeaderText = CLR_INVALID,
};
constexpr inline LVE_COLOR_PACK LveDarkClr1
{
    .crDefText = CLR_INVALID,
    .crOddLineText = CLR_INVALID,
    .crEvenLineText = CLR_INVALID,
    .crOddLineBk = CLR_INVALID,
    .crEvenLineBk = 0x202020,
    .crGridLineH = CLR_INVALID,
    .crGridLineV = CLR_INVALID,
    .crHeaderText = CLR_INVALID,
};

struct LVE_CELL_COLOR
{
    COLORREF crText{ CLR_DEFAULT };
    COLORREF crBk{ CLR_DEFAULT };
    COLORREF crTextBk{ CLR_DEFAULT };
};

#ifdef _DEBUG
constexpr inline UINT LveItemDataMagic{ 'LEID' };
#endif

// 非所有者数据模式下项目lParam指向的结构
struct LVE_ITEM_DATA
{
#ifdef _DEBUG
    UINT dwMagic{ LveItemDataMagic };
#endif
    LVE_CELL_COLOR LineClr{};
    std::map<int, LVE_CELL_COLOR> CellClr{};
    LPARAM lParam{};
};

enum : UINT
{
    LVE_IM_COLOR_TEXT = (1u << 0),
    LVE_IM_COLOR_BK = (1u << 1),
    LVE_IM_COLOR_TEXTBK = (1u << 2),// 备用
    LVE_IM_COLOR_ALL = (LVE_IM_COLOR_TEXT | LVE_IM_COLOR_BK | LVE_IM_COLOR_TEXTBK),
    LVE_IM_LPARAM = (1u << 3),
    LVE_IM_TEXT = (1u << 4),
    LVE_IM_IMAGE = (1u << 5),
    LVE_IM_INDENT = (1u << 6),
    LVE_IM_STATE = (1u << 7),
};

struct LVE_ITEM_EXT
{
    UINT uMask;// LVE_IM_
    int idxItem;
    int idxSubItem;
    int idxImage;
    UINT uState;
    int iIndent;
    LVE_CELL_COLOR Clr;
    LPARAM lParam;
    PCWSTR pszText;
    int cchText;
};

enum class LveOd
{
    GetDispInfo,
    SetDispInfo,
};

struct LVE_EDIT_INFO
{
    RCWH rcIdeal;	// 编辑控件的理想位置和大小
    SIZE sizeExtra;	// 为了美观而添加到sizeIdeal的额外大小
    PCWSTR pszText;	// 当前文本
    int cchText;	// 当前文本长度
    Align eAlign;	// 横向对齐方式
};

/*
CListViewExt封装了ListView的常用扩展功能
===以下ListView的原始功能被忽略===
项目lParam - 由控件内部占用，不能由外部设置
所有者绘制 - 不应使用
边框选择 - 暂不支持

===以下功能效果仍相同但处理机制已变化===
整体的颜色设置 - 应使用Lve系列方法管理，背景颜色仍使用SetBackgroundColor
表格线 - LVS_EX_GRIDLINES永远不会达到ListView，而是由内部接管；增加颜色配置
所有者数据 - 新增一个回调
LVN_DELETEALLITEMS - 若用户处理该通知，则必须返回TRUE
LVN_BEGINLABELEDITW - 若用户处理编辑且指定控件准备编辑信息，则传递参数的lParam指向LVE_EDIT_INFO结构
LVN_ENDLABELEDITW - 若用户处理编辑则cchTextMax可能为负值，若其为负值，则绝不能保存编辑结果
*/
class CListViewExt : public CListView
{
public:
    using FOwnerData = LRESULT(*)(LveOd, void*, void*);
    ECK_RTTI(CListViewExt, CListView);
private:
    enum
    {
        IDT_EDIT_DELAY = 0x514B,
    };
    struct SUBITEM
    {
        CStringW rsText;
        LVE_CELL_COLOR Clr;
    };

    struct ITEM
    {
        CStringW rsText;
        int idxImage;
        LVE_CELL_COLOR Clr;
        std::vector<SUBITEM> vSubItems;
        LPARAM lParam;
    };

    // ListView信息
    CHeaderExt m_Header{};	// 表头
    HIMAGELIST m_hIL[4]{};	// 图像列表
    SIZE m_sizeIL[4]{};		// 图像列表大小
    int m_iViewType{};		// 视图类型
    int m_cMaxTileCol{};	// 平铺视图下的最大列数
    RECT m_rcTileMargin{};	// 平铺视图下的边距
    int m_cyFont{};			// 字体高度
    int m_idxEditing{ -1 };
    int m_idxEditSubItem{ -1 };
    BITBOOL m_bOwnerData : 1 = FALSE;	// 所有者数据
    BITBOOL m_bSubItemImg : 1 = FALSE;	// 显示子项图像
    BITBOOL m_bFullRowSel : 1 = FALSE;	// 整行选择
    BITBOOL m_bShowSelAlways : 1 = FALSE;	// 始终显示选择
    BITBOOL m_bBorderSelect : 1 = FALSE;// 边框选择
    BITBOOL m_bCheckBoxes : 1 = FALSE;	// 显示复选框
    BITBOOL m_bGridLines : 1 = FALSE;	// 显示表格线
    BITBOOL m_bHideLabels : 1 = FALSE;	// 隐藏标签
    BITBOOL m_bSingleSel : 1 = FALSE;	// 单选模式
    BITBOOL m_bHasFocus : 1 = FALSE;	// 是否有焦点
    BITBOOL m_bEditLabel : 1 = FALSE;	// 允许编辑

    // 图形
    HTHEME m_hTheme{};		// 主题句柄
    int m_iDpi{ USER_DEFAULT_SCREEN_DPI };	// DPI
    CMemoryDC m_DcAlpha{};		// DC，用作暗色下的颜色Alpha混合

    // 选项
    COLORREF m_crDefText = CLR_DEFAULT;		// 默认文本颜色
    COLORREF m_crOddLineText = CLR_DEFAULT;	// 奇数行文本颜色
    COLORREF m_crEvenLineText = CLR_DEFAULT;// 偶数行文本颜色
    COLORREF m_crOddLineBk = CLR_DEFAULT;	// 奇数行背景颜色
    COLORREF m_crEvenLineBk = CLR_DEFAULT;	// 偶数行背景颜色
    COLORREF m_crGridLineH = CLR_DEFAULT;	// 表格线水平颜色
    COLORREF m_crGridLineV = CLR_DEFAULT;	// 表格线垂直颜色
    COLORREF m_crHeaderText = CLR_DEFAULT;	// 表头文本颜色

    BITBOOL m_bCustomDraw : 1 = TRUE;		// 是否由控件自动绘制，保留此选项以供特殊情况使用
    BITBOOL m_bAutoDarkMode : 1 = TRUE;		// 自动处理深浅色切换
    BITBOOL m_bAlphaClrInDark : 1 = TRUE;	// 在暗色模式下将颜色移至主题背景之上
    BITBOOL m_bAutoColorPack : 1 = TRUE;	// 自动处理配色
    BITBOOL m_bAddSplitterForClr : 1 = TRUE;// 若某项填充了颜色，则补全列分隔符
    BITBOOL m_bCtrlASelectAll : 1 = TRUE;	// Ctrl+A全选
    BITBOOL m_bImplLvOdNotify : 1 = TRUE;	// 指示控件应根据m_pfnOwnerData实现ListView标准通知
    BITBOOL m_bInstalledHeaderHook : 1 = FALSE;	// [内部标志]是否已钩住表头消息
    BITBOOL m_bDoNotWrapInTile : 1 = FALSE;	// 平铺视图下禁止第一行换行
    BITBOOL m_bEnableExtEdit : 1 = TRUE;	// 启用扩展编辑
    BITBOOL m_bOwnerEdit : 1 = FALSE;		// 由父窗口处理编辑
    BITBOOL m_bPrepareEditingInfo : 1 = TRUE;	// 在父窗口处理编辑时，是否准备编辑信息
    BITBOOL m_bWaitEditDelay : 1 = FALSE;	// [内部标志]有挂起的编辑操作

    BYTE m_byColorAlpha{ 70 };	// 透明度
    int m_cyHeader{};			// 表头高度，0 = 默认
    // 项目
    std::vector<LVE_ITEM_DATA*> m_vRecycleData{};	// [NOD]回收数据
    FOwnerData m_pfnOwnerData{};	// [OD]所有者数据回调函数
    void* m_pOdProcData{};			// [OD]所有者数据回调函数参数
    // 
    const ThreadContext* m_ptc{};// 线程上下文
    CStringW m_rsTextBuf{ MAX_PATH };
    int m_cxEdge{};
    std::unique_ptr<CEditExt> m_pEdit{};
    CWindow::HSlot m_hmsEdit{};

    void GetColumnMetrics(int* px, int cCol, int dx) const noexcept
    {
        RECT rc;
        for (int i = 0; i < cCol; ++i)
        {
            m_Header.GetItemRect(i, &rc);
            px[i * 2] = dx + rc.left;
            px[i * 2 + 1] = dx + rc.right;
        }
    }

    BOOL AlphaBlendColor(HDC hDC, const RECT& rcItem, COLORREF cr) noexcept
    {
        constexpr RECT rcDst{ 0,0,1,1 };
        SetDCBrushColor(m_DcAlpha.GetDC(), cr);
        FillRect(m_DcAlpha.GetDC(), &rcDst, GetStockBrush(DC_BRUSH));
        return AlphaBlend(hDC, rcItem.left, rcItem.top, rcItem.right - rcItem.left,
            rcItem.bottom - rcItem.top,
            m_DcAlpha.GetDC(), 0, 0, 1, 1, { AC_SRC_OVER,0,m_byColorAlpha,0 });
    }

    void BitBltColor(HDC hDC, const RECT& rcItem,
        COLORREF cr, const int* pColMetrics, int cCol) noexcept
    {
        SetDCBrushColor(hDC, cr);
        FillRect(hDC, &rcItem, GetStockBrush(DC_BRUSH));
        if (m_iViewType == LV_VIEW_DETAILS && m_bAddSplitterForClr)
        {
            EckCounter(cCol, i)
            {
                if (pColMetrics[i * 2 + 1] > 0)
                    DrawListViewColumnDetail(m_hTheme, hDC,
                        pColMetrics[i * 2 + 1], rcItem.top, rcItem.bottom);
            }
        }
    }

    LRESULT OnItemPrePaint(NMLVCUSTOMDRAW* pnmlvcd) noexcept
    {
        //return CDRF_DODEFAULT;
        if (IsRectEmpty(pnmlvcd->nmcd.rc))
            return CDRF_DODEFAULT;
        const auto hDC = pnmlvcd->nmcd.hdc;
        const int idx = (int)pnmlvcd->nmcd.dwItemSpec;
        const size_t idxIL = (m_iViewType == LV_VIEW_ICON ||
            m_iViewType == LV_VIEW_TILE ? LVSIL_NORMAL : LVSIL_SMALL);
        const auto hIL = m_hIL[idxIL];
        const auto sizeIL = m_sizeIL[idxIL];
        const auto hILState = m_hIL[LVSIL_STATE];
        const auto sizeILState = m_sizeIL[LVSIL_STATE];
        const BOOL bSelected = (GetItemState(idx, LVIS_SELECTED) == LVIS_SELECTED);
        int* pColMetrics{};
        int cCol{ m_iViewType == LV_VIEW_DETAILS ? m_Header.GetItemCount() : 0 };
        RECT rc, rcItem;
        LVE_ITEM_EXT ie{};
        LVITEMW li;
        COLORREF crLine{ CLR_DEFAULT }, crText{ CLR_DEFAULT };
        const BOOL bExtInfo = (m_bOwnerData && m_pfnOwnerData);

        li.mask = LVIF_IMAGE | LVIF_INDENT | LVIF_STATE | LVIF_PARAM;
        li.iItem = idx;
        li.iSubItem = 0;
        li.stateMask = 0xFFFFFFFF;
        if (bExtInfo)
        {
            ie.uMask = LVE_IM_IMAGE | LVE_IM_INDENT | LVE_IM_STATE | LVE_IM_COLOR_ALL;
            ie.idxItem = idx;
            ie.idxSubItem = -1;
            m_pfnOwnerData(LveOd::GetDispInfo, &ie, m_pOdProcData);
            if (ie.Clr.crBk != CLR_DEFAULT)
                crLine = ie.Clr.crBk;
            if (ie.Clr.crText != CLR_DEFAULT)
                crText = ie.Clr.crText;
            li.iImage = ie.idxImage;
            li.iIndent = ie.iIndent;
            li.state = ie.uState;
            li.lParam = 0;
        }
        else
            GetItem(&li);
        const auto* const pData = (LVE_ITEM_DATA*)li.lParam;
        if (pData)
        {
            if (pData->LineClr.crBk != CLR_DEFAULT)
                crLine = pData->LineClr.crBk;
            if (pData->LineClr.crText != CLR_DEFAULT)
                crText = pData->LineClr.crText;
        }

        if (crLine == CLR_DEFAULT && m_iViewType == LV_VIEW_DETAILS)
            if (idx % 2)
            {
                if (m_crOddLineBk != CLR_DEFAULT)
                    crLine = m_crOddLineBk;
            }
            else
            {
                if (m_crEvenLineBk != CLR_DEFAULT)
                    crLine = m_crEvenLineBk;
            }
        if (crText == CLR_DEFAULT && m_iViewType == LV_VIEW_DETAILS)
            if (idx % 2)
            {
                if (m_crOddLineText != CLR_DEFAULT)
                    crText = m_crOddLineText;
            }
            else
            {
                if (m_crEvenLineText != CLR_DEFAULT)
                    crText = m_crEvenLineText;
            }
        if (crText == CLR_DEFAULT)
            if (m_crDefText != CLR_DEFAULT)
                crText = m_crDefText;
            else
                crText = m_ptc->crDefText;

        int iState;
        if (bSelected)// 选中
        {
            if (pnmlvcd->nmcd.uItemState & CDIS_HOT)
                iState = LISS_HOTSELECTED;
            else if (!m_bHasFocus)
                if (m_bShowSelAlways)
                    iState = LISS_SELECTEDNOTFOCUS;
                else
                    iState = 0;
            else
                iState = LISS_SELECTED;
        }
        else if (pnmlvcd->nmcd.uItemState & CDIS_HOT)
            iState = LISS_HOT;
        else
            iState = 0;

        if (m_iViewType == LV_VIEW_DETAILS)
        {
            if (m_bFullRowSel)
            {
                rcItem = pnmlvcd->nmcd.rc;
                if (li.iIndent)
                    rcItem.left += sizeIL.cx;
            }
            else
            {
                GetItemRect(idx, &rcItem, LVIR_SELECTBOUNDS);
                if (li.iIndent)
                    rcItem.left += sizeIL.cx;
            }
        }
        else
            rcItem = pnmlvcd->nmcd.rc;

        const BOOL bFillClrPostTheme = (m_bAlphaClrInDark && ShouldAppsUseDarkMode() && iState);
        if (m_iViewType == LV_VIEW_DETAILS)
        {
            if (m_bAddSplitterForClr)
            {
                if (!pColMetrics)
                {
                    pColMetrics = (int*)_malloca(cCol * 2 * sizeof(int));
                    EckCheckMem(pColMetrics);
                    GetColumnMetrics(pColMetrics, cCol, pnmlvcd->nmcd.rc.left);
                }
            }

            if (!bFillClrPostTheme)
            {
                if (crLine != CLR_DEFAULT)
                    BitBltColor(hDC, rcItem, crLine, pColMetrics, cCol);

                if (bExtInfo)
                {
                    RECT rcCell;
                    rcCell.top = rcItem.top;
                    rcCell.bottom = rcItem.bottom;
                    ie.uMask = LVE_IM_COLOR_BK;
                    for (ie.idxSubItem = 0; ie.idxSubItem < cCol; ++ie.idxSubItem)
                    {
                        m_pfnOwnerData(LveOd::GetDispInfo, &ie, m_pOdProcData);
                        if (ie.Clr.crBk == CLR_DEFAULT)
                            continue;
                        rcCell.left = pColMetrics[ie.idxSubItem * 2];
                        rcCell.right = pColMetrics[ie.idxSubItem * 2 + 1];
                        BitBltColor(hDC, rcCell, ie.Clr.crBk, pColMetrics, cCol);
                    }
                }
                else if (pData && !pData->CellClr.empty())
                {
                    RECT rcCell;
                    rcCell.top = rcItem.top;
                    rcCell.bottom = rcItem.bottom;
                    for (const auto& [i, cr] : pData->CellClr)
                    {
                        if (cr.crBk == CLR_DEFAULT)
                            continue;
                        rcCell.left = pColMetrics[i * 2];
                        rcCell.right = pColMetrics[i * 2 + 1];
                        BitBltColor(hDC, rcCell, cr.crBk, pColMetrics, cCol);
                    }
                }
            }
        }
        else
        {
            if (!bFillClrPostTheme && crLine != CLR_DEFAULT)
                BitBltColor(hDC, rcItem, crLine, nullptr, 0);
        }

        if (iState)
        {
            DrawThemeBackground(m_hTheme, hDC, LVP_LISTITEM, iState,
                &rcItem, nullptr);
        }

        if (bFillClrPostTheme)
            if ((m_iViewType == LV_VIEW_DETAILS))
            {
                if (crLine != CLR_DEFAULT)
                    AlphaBlendColor(hDC, rcItem, crLine);

                if (bExtInfo)
                {
                    RECT rcCell;
                    rcCell.top = rcItem.top;
                    rcCell.bottom = rcItem.bottom;
                    ie.uMask = LVE_IM_COLOR_BK;
                    for (ie.idxSubItem = 0; ie.idxSubItem < cCol; ++ie.idxSubItem)
                    {
                        m_pfnOwnerData(LveOd::GetDispInfo, &ie, m_pOdProcData);
                        if (ie.Clr.crBk == CLR_DEFAULT)
                            continue;
                        rcCell.left = pColMetrics[ie.idxSubItem * 2];
                        rcCell.right = pColMetrics[ie.idxSubItem * 2 + 1];
                        AlphaBlendColor(hDC, rcCell, ie.Clr.crBk);
                    }
                }
                else if (pData && !pData->CellClr.empty())
                {
                    RECT rcCell;
                    rcCell.top = rcItem.top;
                    rcCell.bottom = rcItem.bottom;
                    for (const auto& [i, cr] : pData->CellClr)
                    {
                        if (cr.crBk == CLR_DEFAULT)
                            continue;
                        rcCell.left = pColMetrics[i * 2];
                        rcCell.right = pColMetrics[i * 2 + 1];
                        AlphaBlendColor(hDC, rcCell, cr.crBk);
                    }
                }
            }
            else if (pData && pData->LineClr.crBk != CLR_DEFAULT)
                AlphaBlendColor(hDC, rcItem, pData->LineClr.crBk);

        // 画第一个子项的图像
        if (hIL)
        {
            // 傻逼微软
            GetItemRect(idx, &rc, LVIR_ICON);// 仅当为报表视图，此矩形才为图标矩形，否则为与状态图片的并
            RECT rc0;
            if (m_iViewType == LV_VIEW_DETAILS)
            {
                rc0 = { 0,0,sizeIL.cx,sizeIL.cy };
                CenterRect(rc0, rc);
            }
            else
            {
                rc0.left = rc.left + (rc.right - rc.left - (sizeIL.cx + sizeILState.cx)) / 2 +
                    sizeILState.cx;
                rc0.right = rc0.left + sizeIL.cx;

                rc0.top = rc.top + (rc.bottom - rc.top - sizeIL.cy) / 2;
                rc0.bottom = rc0.top + sizeIL.cy;
            }
            switch (m_iViewType)
            {
            case LV_VIEW_ICON:
                if (!m_bHideLabels)
                    rc0.top += m_cxEdge;
                break;
            case LV_VIEW_SMALLICON:
                if (!m_bHideLabels)
                    rc0.left += m_cxEdge;
                break;
            case LV_VIEW_LIST:
                rc0.left += m_cxEdge;
                break;
            }
            ImageList_Draw(hIL, li.iImage, hDC,
                rc0.left, rc0.top,
                ILD_NORMAL | ILD_TRANSPARENT | (li.state & LVIS_OVERLAYMASK));

            if (hILState)
            {
                const int idxState = ((li.state & LVIS_STATEIMAGEMASK) >> 12) - 1;
                if (idxState >= 0)
                {
                    if (m_iViewType == LV_VIEW_DETAILS)
                    {
                        rc.right = rc.left;
                        rc.left -= (sizeILState.cx + m_cxEdge);
                        rc0 = { 0,0,sizeILState.cx,sizeILState.cy };
                        CenterRect(rc0, rc);
                    }
                    else
                    {
                        rc0.left -= (sizeILState.cx + m_cxEdge);
                        rc0.top = rc.top + (rc.bottom - rc.top - sizeILState.cy) / 2;
                    }
                    ImageList_Draw(hILState, idxState, hDC,
                        rc0.left, rc0.top,
                        ILD_NORMAL | ILD_TRANSPARENT);
                }
            }
        }
        // 画文本
        if (bExtInfo)
        {
            ie.uMask = LVE_IM_TEXT | LVE_IM_IMAGE;
            ie.idxItem = idx;
        }
        else
        {
            li.mask = LVIF_TEXT | LVIF_IMAGE;
            li.iItem = idx;
        }

        // HACK : 增加文本背景支持
        SetBkMode(hDC, TRANSPARENT);
        if (m_iViewType == LV_VIEW_DETAILS && pData && pData->CellClr.contains(0))
        {
            const auto it = pData->CellClr.find(0);
            if (it->second.crText != CLR_DEFAULT)
            {
                SetTextColor(hDC, it->second.crText);
                goto SkipTextColor;
            }
        }
        SetTextColor(hDC, crText);
    SkipTextColor:
        int cchText{ -1 };
        if (m_iViewType == LV_VIEW_DETAILS)
        {
            HDITEMW hdi;
            hdi.mask = HDI_FORMAT;
            int cxImg = 0;
            for (li.iSubItem = 0; li.iSubItem < cCol; ++li.iSubItem)
            {
                if (bExtInfo)
                {
                    ie.uMask = LVE_IM_TEXT | LVE_IM_IMAGE;
                    ie.idxSubItem = li.iSubItem;
                    ie.pszText = m_rsTextBuf.Data();
                    ie.cchText = m_rsTextBuf.Size();
                    m_pfnOwnerData(LveOd::GetDispInfo, &ie, m_pOdProcData);
                    li.pszText = (PWSTR)ie.pszText;
                    li.iImage = ie.idxImage;
                    cchText = ie.cchText;
                }
                else
                {
                    li.pszText = m_rsTextBuf.Data();
                    li.cchTextMax = m_rsTextBuf.Size();
                    GetItem(&li);
                }
                // 如果可能，绘制后续子项图像
                if (li.iImage >= 0 && hIL)
                    if (li.iSubItem == 0)
                        cxImg = m_cxEdge;
                    else if (m_bSubItemImg)
                    {
                        RECT rcImg;
                        GetSubItemRect(idx, li.iSubItem, &rcImg, LVIR_ICON);
                        cxImg = rcImg.right - rcImg.left + m_cxEdge * 3;
                        ImageList_Draw(hIL, li.iImage, hDC,
                            rcImg.left,
                            (pnmlvcd->nmcd.rc.bottom - pnmlvcd->nmcd.rc.top -
                                (m_sizeIL[idxIL].cy)) / 2 + pnmlvcd->nmcd.rc.top,
                            ILD_NORMAL | ILD_TRANSPARENT);
                    }
                    else
                        cxImg = m_cxEdge * 2;
                else
                    cxImg = m_cxEdge * 2;

                m_Header.GetItem(li.iSubItem, &hdi);
                UINT uDtFlags = DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX | DT_SINGLELINE;
                if (IsBitSet(hdi.fmt, HDF_RIGHT))
                    uDtFlags |= DT_RIGHT;
                else if (IsBitSet(hdi.fmt, HDF_CENTER))
                    uDtFlags |= DT_CENTER;
                GetSubItemRect(idx, li.iSubItem, &rc, LVIR_LABEL);
                rc.left += cxImg;
                BOOL bRestoreTextColor = FALSE;
                if (bExtInfo)
                {
                    ie.uMask = LVE_IM_COLOR_TEXT;
                    ie.idxSubItem = li.iSubItem;
                    m_pfnOwnerData(LveOd::GetDispInfo, &ie, m_pOdProcData);
                    if (ie.Clr.crText != CLR_DEFAULT)
                    {
                        SetTextColor(hDC, ie.Clr.crText);
                        bRestoreTextColor = TRUE;
                    }
                }
                else if (pData)
                {
                    const auto it = pData->CellClr.find(li.iSubItem);
                    if (it != pData->CellClr.end() && it->second.crText != CLR_DEFAULT)
                    {
                        SetTextColor(hDC, it->second.crText);
                        bRestoreTextColor = TRUE;
                    }
                }
                DrawTextW(hDC, li.pszText, cchText, &rc, uDtFlags);
                if (bRestoreTextColor)
                    SetTextColor(hDC, crText);
            }
        }
        else if (m_iViewType == LV_VIEW_TILE)
        {
            LVTILEINFO lvti;
            const auto pTileCol = (int*)_malloca((m_cMaxTileCol + 1) * sizeof(int) * 2);
            const auto pTileColFmt = pTileCol + (m_cMaxTileCol + 1);
            *pTileCol = 0;
            *pTileColFmt = LVCFMT_LEFT;
            lvti.cbSize = sizeof(LVTILEINFO);
            lvti.iItem = idx;
            lvti.cColumns = m_cMaxTileCol;
            lvti.puColumns = (UINT*)pTileCol + 1;
            lvti.piColFmt = pTileColFmt + 1;
            GetTileInfomation(&lvti);
            ++lvti.cColumns;

            GetItemRect(idx, &rc, LVIR_LABEL);
            rc.left += m_rcTileMargin.left;
            rc.right -= m_rcTileMargin.right;
            rc.top += m_rcTileMargin.top;
            rc.bottom -= m_rcTileMargin.bottom;
            const int cyOrg = rc.bottom - rc.top;
            rc.top += ((rc.bottom - rc.top) - (lvti.cColumns) * m_cyFont) / 2;
            rc.bottom = rc.top + lvti.cColumns * m_cyFont;
            const int yBottom = rc.bottom;
            EckCounter(lvti.cColumns, i)
            {
                li.iSubItem = pTileCol[i];
                if (bExtInfo)
                {
                    ie.uMask = LVE_IM_TEXT;
                    ie.idxSubItem = li.iSubItem;
                    ie.pszText = m_rsTextBuf.Data();
                    ie.cchText = m_rsTextBuf.Size();
                    m_pfnOwnerData(LveOd::GetDispInfo, &ie, m_pOdProcData);
                    li.pszText = (PWSTR)ie.pszText;
                    li.iImage = ie.idxImage;
                    cchText = ie.cchText;
                }
                else
                {
                    li.pszText = m_rsTextBuf.Data();
                    li.cchTextMax = m_rsTextBuf.Size();
                    GetItem(&li);
                    cchText = (int)wcslen(li.pszText);
                }

                UINT uDtFlags{ DT_NOPREFIX };
                if (li.iSubItem == 0 && !m_bDoNotWrapInTile)
                {
                    SIZE size;
                    GetTextExtentPoint32W(hDC, li.pszText, cchText, &size);
                    // 处理第一行换行
                    if (size.cx > rc.right - rc.left)
                    {
                        uDtFlags |= (DT_END_ELLIPSIS | DT_WORDBREAK | DT_EDITCONTROL);
                        if (cyOrg >= int(m_cyFont * (lvti.cColumns + 1)))
                            rc.top -= (m_cyFont / 2);
                        rc.bottom = rc.top + m_cyFont * 2;
                        goto TileWraped;
                    }
                }
                if (IsBitSet(pTileColFmt[i], LVCFMT_RIGHT))
                    uDtFlags |= DT_RIGHT;
                else if (IsBitSet(pTileColFmt[i], LVCFMT_CENTER))
                    uDtFlags |= DT_CENTER;
                uDtFlags |= (DT_END_ELLIPSIS | DT_SINGLELINE);
                rc.bottom = rc.top + m_cyFont;
            TileWraped:;
                if (bExtInfo)
                {
                    ie.uMask = LVE_IM_COLOR_TEXT;
                    ie.idxSubItem = li.iSubItem;
                    m_pfnOwnerData(LveOd::GetDispInfo, &ie, m_pOdProcData);
                    if (ie.Clr.crText != CLR_DEFAULT)
                    {
                        SetTextColor(hDC, ie.Clr.crText);
                        goto TileTextColored;
                    }
                }
                else if (pData)
                {
                    const auto it = pData->CellClr.find(li.iSubItem);
                    if (it != pData->CellClr.end() && it->second.crText != CLR_DEFAULT)
                    {
                        SetTextColor(hDC, it->second.crText);
                        goto TileTextColored;
                    }
                }

                if (li.iSubItem == 0)
                    SetTextColor(hDC, crText);
                else
                    SetTextColor(hDC, m_ptc->crGray1);
            TileTextColored:;
                DrawTextW(hDC, li.pszText, cchText, &rc, uDtFlags);
                rc.top = rc.bottom;
                if (rc.top >= yBottom)
                    break;
            }
            _freea(pTileCol);
        }
        else
        {
            if (bExtInfo)
            {
                ie.idxSubItem = 0;
                m_pfnOwnerData(LveOd::GetDispInfo, &ie, m_pOdProcData);
                li.pszText = (PWSTR)ie.pszText;
                li.iImage = ie.idxImage;
                cchText = ie.cchText;
            }
            else
                GetItem(&li);
            GetItemRect(idx, &rc, LVIR_LABEL);
            UINT uDtFlags;

            if (m_iViewType == LV_VIEW_ICON && (bSelected || (pnmlvcd->nmcd.uItemState & CDIS_FOCUS)))
                uDtFlags = DT_CENTER | DT_NOPREFIX | DT_WORDBREAK | DT_EDITCONTROL;
            else
            {
                uDtFlags = DT_END_ELLIPSIS | DT_NOPREFIX | DT_SINGLELINE;
                if (m_iViewType == LV_VIEW_SMALLICON || m_iViewType == LV_VIEW_LIST)
                    uDtFlags |= DT_VCENTER;
            }
            if (!((m_iViewType == LV_VIEW_SMALLICON || m_iViewType == LV_VIEW_ICON) && m_bHideLabels))
                DrawTextW(hDC, li.pszText, cchText, &rc, uDtFlags);
        }
        _freea(pColMetrics);
        return CDRF_SKIPDEFAULT;
    }

    void UpdateLvExOptions(DWORD dwLvExStyle) noexcept
    {
        m_bSubItemImg = IsBitSet(dwLvExStyle, LVS_EX_SUBITEMIMAGES);
        m_bFullRowSel = IsBitSet(dwLvExStyle, LVS_EX_FULLROWSELECT);
        m_bBorderSelect = IsBitSet(dwLvExStyle, LVS_EX_BORDERSELECT);
        m_bCheckBoxes = IsBitSet(dwLvExStyle, LVS_EX_CHECKBOXES);
        //m_bGridLines = IsBitSet(dwLvExStyle, LVS_EX_GRIDLINES);// Special handled
        m_bHideLabels = IsBitSet(dwLvExStyle, LVS_EX_HIDELABELS);
    }

    void UpdateStyleOptions(DWORD dwStyle) noexcept
    {
        m_bShowSelAlways = IsBitSet(dwStyle, LVS_SHOWSELALWAYS);
        m_bOwnerData = IsBitSet(dwStyle, LVS_OWNERDATA);
        m_bSingleSel = IsBitSet(dwStyle, LVS_SINGLESEL);
        //m_bEditLabel = IsBitSet(dwStyle, LVS_EDITLABELS);// Special handled
    }

    void HandleThemeChange() noexcept
    {
        if (m_bAutoDarkMode)
        {
            SetBackgroundColor(m_ptc->crDefBkg);
            if (m_bAutoColorPack)
                if (ShouldAppsUseDarkMode())
                    LveSetColorPack(LveDarkClr1);
                else
                    LveSetColorPack(LveLightClr1);
        }
    }

    void InitializeForNewWindow(HWND hWnd) noexcept
    {
        if (Style & LVS_EDITLABELS)
        {
            Style &= ~LVS_EDITLABELS;
            m_bEditLabel = TRUE;
        }
        else
            m_bEditLabel = FALSE;
        m_ptc = PtcCurrent();
        m_cxEdge = DaGetSystemMetrics(SM_CXEDGE, m_iDpi);
        m_DcAlpha.Create(hWnd, 1, 1);
        if (const auto hHeader = GetHeaderControl())
            m_Header.AttachNew(hHeader);
        m_hTheme = OpenThemeData(hWnd, L"ListView");
        m_iViewType = (int)GetView();
        UpdateStyleOptions(Style);
        UpdateLvExOptions(GetLVExtendStyle());
        m_iDpi = GetDpi(hWnd);
        m_bHasFocus = (GetFocus() == hWnd);
        HandleThemeChange();
    }

    void CleanupForDestroyWindow() noexcept
    {
        CloseThemeData(m_hTheme);
        m_hTheme = nullptr;
        if (m_bInstalledHeaderHook)
            m_Header.GetSignal().Disconnect(MHI_LVE_HEADER_HEIGHT);
        (void)m_Header.DetachNew();
        m_hIL[0] = m_hIL[1] = m_hIL[2] = m_hIL[3] = nullptr;
        m_sizeIL[0] = m_sizeIL[1] = m_sizeIL[2] = m_sizeIL[3] = {};
        m_cMaxTileCol = 0;
        m_rcTileMargin = {};
        m_cyFont = 0;
        LveSetColorPack({});
        m_bCustomDraw = m_bAutoDarkMode = m_bAlphaClrInDark =
            m_bAutoColorPack = m_bAddSplitterForClr = m_bCtrlASelectAll = TRUE;
        m_byColorAlpha = 70;
        m_cyHeader = 0;
        m_pfnOwnerData = nullptr;
        m_pOdProcData = nullptr;
    }

    void CancelExtendEdit(BOOL bSave) noexcept
    {
        /*
        * 以下情况需要取消编辑：
        * 排序
        * 设置视图
        * 取消编辑
        * 滚动
        * 编辑组件失去焦点
        * 编辑组件按键（Enter、Esc）
        * 窗口位置大小改变
        * 设置风格
        * 鼠标按下
        * 删除项目
        * 表头变化
        * 表头鼠标操作
        */
        if (!m_bEnableExtEdit || !m_pEdit || m_idxEditing < 0 ||
            m_pEdit->IsValid() != !m_bOwnerEdit)
            return;
        NMLVDISPINFOW nm{};
        nm.item.iItem = m_idxEditing;
        nm.item.iSubItem = m_idxEditSubItem;
        m_idxEditing = m_idxEditSubItem = -1;
        if (m_pEdit->IsValid())
        {
            const auto rsText = m_pEdit->GetText();
            nm.item.pszText = (PWSTR)rsText.Data();
            nm.item.cchTextMax = rsText.Size();
            if (FillNmhdrAndSendNotify(nm, LVN_ENDLABELEDITW))
            {
                if (bSave)
                    if (m_pfnOwnerData)
                    {
                        LVE_ITEM_EXT ie;
                        ie.uMask = LVE_IM_TEXT;
                        ie.idxItem = nm.item.iItem;
                        ie.idxSubItem = nm.item.iSubItem;
                        ie.pszText = rsText.Data();
                        ie.cchText = rsText.Size();
                        m_pfnOwnerData(LveOd::SetDispInfo, &ie, m_pOdProcData);
                    }
                    else
                    {
                        LVITEMW li;
                        li.mask = LVIF_TEXT;
                        li.iItem = nm.item.iItem;
                        li.iSubItem = nm.item.iSubItem;
                        li.pszText = (PWSTR)rsText.Data();
                        SetItem(&li);
                    }
            }
            m_pEdit->Destroy();
        }
        else
        {
            nm.item.cchTextMax = (bSave ? 0 : -1);
            FillNmhdrAndSendNotify(nm, LVN_ENDLABELEDITW);
        }
    }

    void EnterEditDelay(int idx, int idxSubItemDisplay) noexcept
    {
        m_bWaitEditDelay = TRUE;
        m_idxEditing = idx;
        m_idxEditSubItem = idxSubItemDisplay;
        SetTimer(HWnd, IDT_EDIT_DELAY, GetDoubleClickTime(), nullptr);
    }

    void CancelEditDelay() noexcept
    {
        if (!m_bWaitEditDelay)
            return;
        KillTimer(HWnd, IDT_EDIT_DELAY);
        m_bWaitEditDelay = FALSE;
    }

    LRESULT OnMessageEdit(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, SlotCtx& Ctx) noexcept
    {
        switch (uMsg)
        {
        case WM_KILLFOCUS:
            if (InSendMessage())
                ReplyMessage(0);
            CancelExtendEdit(TRUE);
            break;
        case WM_KEYDOWN:
            if (wParam == VK_RETURN)
                CancelExtendEdit(TRUE);
            else if (wParam == VK_ESCAPE)
                CancelExtendEdit(FALSE);
            break;
        case WM_GETDLGCODE:
            Ctx.Processed();
            return DLGC_WANTALLKEYS | DLGC_HASSETSEL;
        }
        return 0;
    }

    void EnterEdit() noexcept
    {
        LVE_EDIT_INFO edi{};
        if (!m_bOwnerEdit || m_bPrepareEditingInfo)
        {
            if (m_pfnOwnerData)
            {
                LVE_ITEM_EXT ie;
                ie.uMask = LVE_IM_TEXT;
                ie.idxItem = m_idxEditing;
                ie.idxSubItem = m_idxEditSubItem;
                ie.pszText = m_rsTextBuf.Data();
                ie.cchText = m_rsTextBuf.Size();
                m_pfnOwnerData(LveOd::GetDispInfo, &ie, m_pOdProcData);
                edi.pszText = ie.pszText;
                edi.cchText = ie.cchText;
            }
            else
            {
                LVITEMW li;
                li.mask = LVIF_TEXT;
                li.iItem = m_idxEditing;
                li.iSubItem = m_idxEditSubItem;
                li.pszText = m_rsTextBuf.Data();
                li.cchTextMax = m_rsTextBuf.Size();
                GetItem(&li);
                edi.pszText = li.pszText;
                edi.cchText = (int)wcslen(li.pszText);
            }

            const auto hFont = HFont;
            const auto hOld = SelectObject(m_DcAlpha.GetDC(), hFont);
            GetTextExtentPoint32W(m_DcAlpha.GetDC(), edi.pszText, edi.cchText,
                (SIZE*)&edi.rcIdeal + 1);
            GetTextExtentPoint32W(m_DcAlpha.GetDC(), L"AA", 2, &edi.sizeExtra);
            SelectObject(m_DcAlpha.GetDC(), hOld);


            RCWH rcClient;
            GetClientRect(HWnd, (RECT*)&rcClient);

            RECT rc;
            if (m_iViewType == LV_VIEW_DETAILS)
                GetSubItemRect(m_idxEditing, m_idxEditSubItem,
                    &rc, LVIR_LABEL);
            else
                GetItemRect(m_idxEditing, &rc, LVIR_LABEL);
            edi.rcIdeal.cy = (edi.sizeExtra.cy * 4 / 3);
            if (edi.rcIdeal.cx < rc.right - rc.left)
                edi.rcIdeal.cx = rc.right - rc.left;
            else
                edi.rcIdeal.cx += edi.sizeExtra.cx;
            edi.rcIdeal.x = rc.left;
            edi.rcIdeal.y = rc.top + (rc.bottom - rc.top - edi.rcIdeal.cy) / 2;

            if (m_iViewType == LV_VIEW_DETAILS)
            {
                HDITEMW hdi;
                hdi.mask = HDI_FORMAT;
                m_Header.GetItem(m_idxEditSubItem, &hdi);
                if (hdi.fmt & HDF_RIGHT)
                {
                    edi.eAlign = Align::Far;
                    edi.rcIdeal.x = rc.right - edi.rcIdeal.cx;
                }
                else if (hdi.fmt & HDF_CENTER)
                {
                    edi.eAlign = Align::Center;
                    edi.rcIdeal.x = rc.left + (rc.right - rc.left - edi.rcIdeal.cx) / 2;
                }
                else
                    edi.eAlign = Align::Near;
            }
            else
            {
                edi.eAlign = Align::Near;
                edi.rcIdeal.x = rc.left + (rc.right - rc.left - edi.rcIdeal.cx) / 2;
            }
            if (IsRectInclude(edi.rcIdeal, rcClient))
                AdjustRectIntoAnother(edi.rcIdeal, rcClient);
        }

        if (m_bOwnerEdit)
        {
            NMLVDISPINFOW nm{};
            nm.item.iSubItem = m_idxEditSubItem;
            nm.item.lParam = (LPARAM)&edi;
            if (FillNmhdrAndSendNotify(nm, LVN_BEGINLABELEDITW))
                m_idxEditing = -1;
        }
        else
        {
            if (!m_pEdit)
                m_pEdit = std::make_unique<CEditExt>();

            DWORD dwStyle = WS_VISIBLE | WS_CHILD;
            if (edi.eAlign == Align::Far)
                dwStyle |= ES_RIGHT;
            else if (edi.eAlign == Align::Center)
                dwStyle |= ES_CENTER;
            m_pEdit->Create(edi.pszText, dwStyle, 0,
                edi.rcIdeal.x, edi.rcIdeal.y, edi.rcIdeal.cx, edi.rcIdeal.cy,
                HWnd, 0);
            m_pEdit->SelectAll();
            SetFocus(m_pEdit->HWnd);
            if (!m_hmsEdit)
                m_hmsEdit = m_pEdit->GetSignal().Connect(this, &CListViewExt::OnMessageEdit);
            m_pEdit->HFont = HFont;
            m_pEdit->SetFrameType(5);
            m_pEdit->FrameChanged();
        }
    }
public:
    ECKPROP(LveGetTextColor, LveSetTextColor)               COLORREF TextColor;
    ECKPROP(LveGetOddLineTextColor, LveSetOddLineTextColor) COLORREF OddLineTextColor;
    ECKPROP(LveGetEvenLineTextColor, LveSetEvenLineTextColor)               COLORREF EvenLineTextColor;
    ECKPROP(LveGetOddLineBackgroundColor, LveSetOddLineBackgroundColor)     COLORREF OddLineBkColor;
    ECKPROP(LveGetEvenLineBackgroundColor, LveSetEvenLineBackgroundColor)   COLORREF EvenLineBkColor;
    ECKPROP(LveGetGridLineHColor, LveSetGridLineHColor)     COLORREF GridLineHColor;
    ECKPROP(LveGetGridLineVColor, LveSetGridLineVColor)     COLORREF GridLineVColor;
    ECKPROP(LveGetHeaderTextColor, LveSetHeaderTextColor)   COLORREF HeaderTextColor;
    ECKPROP(LveGetCustomDraw, LveSetCustomDraw)             BOOL CustomDraw;
    ECKPROP(LveGetAutoDarkMode, LveSetAutoDarkMode)         BOOL AutoDarkMode;
    ECKPROP(LveGetAlphaColorInDark, LveSetAlphaColorInDark) BOOL AlphaColorInDark;
    ECKPROP(LveGetAlphaValue, LveSetAlphaValue)             BYTE AlphaValue;
    ECKPROP(LveGetAutoColorPack, LveSetAutoColorPack)       BOOL AutoColorPack;
    ECKPROP(LveGetAddSplitterForColor, LveSetAddSplitterForColor) BOOL AddSplitterForClr;
    ECKPROP(LveGetCtrlASelectAll, LveSetCtrlASelectAll)     BOOL CtrlASelectAll;
    ECKPROP(LveGetImplementOwnerDataNotify, LveSetImplementOwnerDataNotify)	BOOL ImplOwnerDataNotify;
    ECKPROP(LveGetDoNotWrapInTile, LveSetDoNotWrapInTile)   BOOL DoNotWrapInTile;
    ECKPROP(LveGetOwnerDataBufferSize, LveSetOwnerDataBufferSize) int OwnerDataBufferSize;
    ECKPROP_W(LveSetHeaderHeight)                           int HeaderHeight;

    ~CListViewExt()
    {
        for (const auto e : m_vRecycleData)
            delete e;
        m_vRecycleData.clear();
        m_hmsEdit = nullptr;
    }

    void AttachNew(HWND hWnd) noexcept override
    {
        CWindow::AttachNew(hWnd);
        InitializeForNewWindow(hWnd);
    }

    void DetachNew() noexcept override
    {
        CWindow::DetachNew();
        CleanupForDestroyWindow();
    }

    LRESULT OnMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept override
    {
        switch (uMsg)
        {
        case WM_PRINTCLIENT:
        case WM_PAINT:
        {
            if (!m_bGridLines || (m_iViewType != LV_VIEW_DETAILS))
                break;
            PAINTSTRUCT ps;
            BeginPaint(hWnd, wParam, ps);
            CListView::OnMessage(hWnd, WM_PAINT, (WPARAM)ps.hdc, 0);

            RECT rcItem;
            const auto cCol = m_Header.GetItemCount();

            if (m_crGridLineH != CLR_NONE)
            {
                GetItemRect(0, &rcItem, LVIR_BOUNDS);
                const int cyItem = rcItem.bottom - rcItem.top;
                m_Header.GetItemRect(0, &rcItem);
                int y = rcItem.bottom;

                SetDCPenColor(ps.hdc, m_crGridLineH == CLR_DEFAULT ?
                    m_ptc->crDefText : m_crGridLineH);
                const auto hOld = SelectObject(ps.hdc, GetStockObject(DC_PEN));
                for (; y <= ps.rcPaint.bottom; y += cyItem)
                {
                    if (y < ps.rcPaint.top)
                        continue;
                    MoveToEx(ps.hdc, ps.rcPaint.left, y, nullptr);
                    LineTo(ps.hdc, ps.rcPaint.right, y);
                }
                SelectObject(ps.hdc, hOld);
            }

            if (m_crGridLineV != CLR_NONE)
            {
                const int oxHeader = -ScbGetPosition(SB_HORZ);
                SetDCPenColor(ps.hdc, m_crGridLineV == CLR_DEFAULT ?
                    m_ptc->crDefText : m_crGridLineV);
                const auto hOld = SelectObject(ps.hdc, GetStockObject(DC_PEN));
                for (int i = 0; i < cCol - 1; ++i)
                {
                    m_Header.GetItemRect(i, &rcItem);
                    if (rcItem.right + oxHeader < ps.rcPaint.left)
                        continue;

                    MoveToEx(ps.hdc, rcItem.right + oxHeader,
                        std::max(ps.rcPaint.top, rcItem.bottom), nullptr);
                    LineTo(ps.hdc, rcItem.right + oxHeader, ps.rcPaint.bottom);
                }
                SelectObject(ps.hdc, hOld);
            }
            EndPaint(hWnd, wParam, ps);
            return 0;
        }
        break;

        case WM_NOTIFY:
        {
            if (m_bEditLabel && m_bEnableExtEdit && m_Header.HWnd == ((NMHDR*)lParam)->hwndFrom)
            {
                switch (((NMHDR*)lParam)->code)
                {
                case HDN_ITEMCHANGEDW:
                case HDN_BEGINTRACKW:
                case HDN_BEGINDRAG:
                case HDN_BEGINFILTEREDIT:
                case HDN_DIVIDERDBLCLICKW:
                    CancelExtendEdit(FALSE);
                    break;
                }
            }
        }
        break;

        case WM_TIMER:
        {
            if (wParam == IDT_EDIT_DELAY)
            {
                if (m_bWaitEditDelay)
                {
                    CancelEditDelay();
                    EnterEdit();
                }
            }
        }
        break;

        case WM_LBUTTONDOWN:
        {
            if (m_bEditLabel && m_bEnableExtEdit && wParam == MK_LBUTTON)
            {
                CancelEditDelay();
                CancelExtendEdit(TRUE);
                LVHITTESTINFO lvhti;
                lvhti.pt = ECK_GET_PT_LPARAM(lParam);
                if (m_iViewType == LV_VIEW_DETAILS)
                    SubItemHitTest(&lvhti);
                else
                {
                    HitTest(&lvhti);
                    lvhti.iSubItem = 0;
                }

                if (lvhti.iItem >= 0 &&
                    GetItemState(lvhti.iItem, LVIS_SELECTED) == LVIS_SELECTED)
                {
                    EnterEditDelay(lvhti.iItem, lvhti.iSubItem);
                }
            }
        }
        break;

        case WM_LBUTTONUP:
        case WM_LBUTTONDBLCLK:
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:
        case WM_RBUTTONDBLCLK:
        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP:
        case WM_MBUTTONDBLCLK:
        case WM_MOUSEWHEEL:
        case WM_MOUSEHWHEEL:
            if (m_bEditLabel && m_bEnableExtEdit)
                CancelExtendEdit(TRUE);
            break;
        case LVM_SORTITEMS:
        case LVM_SORTITEMSEX:
        case LVM_SORTGROUPS:
        case LVM_SCROLL:
        case LVM_DELETEITEM:
        case LVM_DELETEALLITEMS:
        case WM_WINDOWPOSCHANGED:
        case WM_HSCROLL:
        case WM_VSCROLL:
            if (m_bEditLabel && m_bEnableExtEdit)
                CancelExtendEdit(FALSE);
            break;

        case WM_KEYDOWN:
            if (wParam == 'A' && !m_bSingleSel)
                if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
                {
                    SetItemState(-1, LVIS_SELECTED, LVIS_SELECTED);
                    return 0;
                }
            break;

        case WM_SETFOCUS:
            m_bHasFocus = TRUE;
            break;

        case WM_KILLFOCUS:
            m_bHasFocus = FALSE;
            break;

        case WM_THEMECHANGED:
        {
            CloseThemeData(m_hTheme);
            m_hTheme = OpenThemeData(hWnd, L"ListView");
            if (m_ptc)
                HandleThemeChange();
        }
        break;

        case WM_SETFONT:
        {
            const auto hFont = (HFONT)wParam;
            const auto hOld = (HFONT)SelectObject(m_DcAlpha.GetDC(), hFont);
            TEXTMETRICW tm;
            GetTextMetricsW(m_DcAlpha.GetDC(), &tm);
            m_cyFont = tm.tmHeight;
            SelectObject(m_DcAlpha.GetDC(), hOld);
        }
        break;

        case WM_DPICHANGED_BEFOREPARENT:
            m_iDpi = GetDpi(hWnd);
            m_cxEdge = DaGetSystemMetrics(SM_CXEDGE, m_iDpi);
            break;

        case WM_STYLECHANGED:
            if (wParam == GWL_STYLE)
                UpdateStyleOptions(((STYLESTRUCT*)lParam)->styleNew);
            if (m_bEditLabel && m_bEnableExtEdit)
                CancelExtendEdit(FALSE);
            break;

        case WM_STYLECHANGING:
            if (wParam == GWL_STYLE && m_bEnableExtEdit)
            {
                const auto p = (STYLESTRUCT*)lParam;
                if ((p->styleNew ^ p->styleOld) & LVS_EDITLABELS)
                {
                    m_bEditLabel = !!(p->styleNew & LVS_EDITLABELS);
                    p->styleNew &= ~LVS_EDITLABELS;
                }
            }
            break;

        case WM_CREATE:
        {
            const auto lResult = CListView::OnMessage(hWnd, uMsg, wParam, lParam);
            if (!lResult)
                InitializeForNewWindow(hWnd);
            return lResult;
        }
        break;

        case WM_DESTROY:
            CleanupForDestroyWindow();
            break;

        case LVM_SETTILEVIEWINFO:
        {
            const auto lResult = CListView::OnMessage(hWnd, uMsg, wParam, lParam);
            if (lResult)
            {
                const auto* const p = (LVTILEVIEWINFO*)lParam;
                if (p->dwMask & LVTVIM_COLUMNS)
                    m_cMaxTileCol = std::max(m_cMaxTileCol, p->cLines);
                if (p->dwMask & LVTVIM_LABELMARGIN)
                    m_rcTileMargin = p->rcLabelMargin;
            }
            return lResult;
        }
        break;

        case LVM_SETVIEW:
        {
            if (m_bEditLabel && m_bEnableExtEdit)
                CancelExtendEdit(FALSE);
            const auto lResult = CListView::OnMessage(hWnd, uMsg, wParam, lParam);
            if (!m_Header.IsValid())
                if (const auto hHeader = GetHeaderControl())
                    m_Header.AttachNew(hHeader);
            if (lResult == 1)
                m_iViewType = (int)wParam;
            return lResult;
        }
        break;

        case LVM_SETIMAGELIST:
        {
            const auto lResult = CListView::OnMessage(hWnd, uMsg, wParam, lParam);
            if (wParam >= 0 && wParam <= 3)
            {
                m_hIL[wParam] = (HIMAGELIST)lParam;
                ImageList_GetIconSize(m_hIL[wParam],
                    (int*)&m_sizeIL[wParam].cx, (int*)&m_sizeIL[wParam].cy);
            }
            return lResult;
        }
        break;

        case LVM_SETEXTENDEDLISTVIEWSTYLE:
        {
            if ((wParam & LVS_EX_GRIDLINES) || !wParam)
            {
                m_bGridLines = (lParam & LVS_EX_GRIDLINES);
                wParam &= ~LVS_EX_GRIDLINES;
                lParam &= ~LVS_EX_GRIDLINES;
            }
            const auto lResult = CListView::OnMessage(hWnd, uMsg, wParam, lParam);
            if ((wParam & LVS_EX_CHECKBOXES) || !wParam)
            {
                m_hIL[LVSIL_STATE] = GetImageList(LVSIL_STATE);
                ImageList_GetIconSize(m_hIL[LVSIL_STATE],
                    (int*)&m_sizeIL[LVSIL_STATE].cx, (int*)&m_sizeIL[LVSIL_STATE].cy);
            }
            UpdateLvExOptions(GetLVExtendStyle());
            return lResult;
        }
        break;
        }
        return CListView::OnMessage(hWnd, uMsg, wParam, lParam);
    }

    LRESULT OnNotifyMessage(HWND hParent, UINT uMsg,
        WPARAM wParam, LPARAM lParam, BOOL& bProcessed) noexcept override
    {
        switch (uMsg)
        {
        case WM_NOTIFY:
        {
            switch (((NMHDR*)lParam)->code)
            {
            case LVN_GETDISPINFOW:
            {
                if (!m_bImplLvOdNotify || !m_pfnOwnerData)
                    break;
                bProcessed = TRUE;
                const auto p = (NMLVDISPINFOW*)lParam;
                LVE_ITEM_EXT ie;
                ie.idxItem = p->item.iItem;
                ie.idxSubItem = p->item.iSubItem;
                ie.uMask = 0;
                if (p->item.mask & LVIF_TEXT)
                {
                    ie.uMask |= LVE_IM_TEXT;
                    ie.pszText = m_rsTextBuf.Data();
                    ie.cchText = m_rsTextBuf.Size();
                }
                if (p->item.mask & LVIF_IMAGE)
                    ie.uMask |= LVE_IM_IMAGE;
                if (p->item.mask & LVIF_STATE)
                    ie.uMask |= LVE_IM_STATE;
                m_pfnOwnerData(LveOd::GetDispInfo, &ie, m_pOdProcData);
                if (ie.uMask & LVE_IM_TEXT)
                {
                    if (!m_rsTextBuf.IsEmpty())
                        p->item.pszText = (PWSTR)ie.pszText;
                    else
                    {
                        const int cchMax = std::min(p->item.cchTextMax - 1, ie.cchText);
                        wmemcpy(p->item.pszText, ie.pszText, cchMax);
                        p->item.pszText[cchMax] = L'\0';
                    }
                }
                p->item.iImage = ie.idxImage;
                p->item.state = ie.uState;
                p->item.stateMask = ie.uState;
            }
            break;

            case NM_CUSTOMDRAW:
            {
                if (!m_bCustomDraw)
                    break;
                bProcessed = TRUE;
                const auto pnmlvcd = (NMLVCUSTOMDRAW*)lParam;
                switch (pnmlvcd->nmcd.dwDrawStage)
                {
                case CDDS_PREPAINT:
                    return CDRF_NOTIFYITEMDRAW;
                case CDDS_ITEMPREPAINT:
                    return OnItemPrePaint(pnmlvcd);
                }
            }
            break;

            case LVN_DELETEITEM:
            {
                if (m_bOwnerData)
                    break;
                const auto p = (NMLISTVIEW*)lParam;
                if (p->lParam)
                {
                    EckAssert(((LVE_ITEM_DATA*)p->lParam)->dwMagic == LveItemDataMagic);
                    m_vRecycleData.push_back((LVE_ITEM_DATA*)p->lParam);
                }
            }
            break;

            case LVN_BEGINLABELEDITW:
            {
                if (!m_bEnableExtEdit)
                    break;
                bProcessed = TRUE;
                //OnBeginExtEdit(hParent, (NMLVDISPINFOW*)lParam);
                return TRUE;// 取消编辑
            }
            break;

            case LVN_ENDLABELEDITW:
            {
                int a{};
            }
            }
        }
        break;
        }
        return __super::OnNotifyMessage(hParent, uMsg, wParam, lParam, bProcessed);
    }

    EckInlineCe void LveSetTextColor(COLORREF cr) noexcept { m_crDefText = cr; }
    EckInlineNdCe COLORREF LveGetTextColor() const noexcept { return m_crDefText; }

    EckInlineCe void LveSetHeaderTextColor(COLORREF cr) noexcept { m_crHeaderText = cr; }
    EckInlineNdCe COLORREF LveGetHeaderTextColor() const noexcept { return m_crHeaderText; }

    EckInlineCe void LveSetOddLineTextColor(COLORREF cr) noexcept { m_crOddLineText = cr; }
    EckInlineNdCe COLORREF LveGetOddLineTextColor() const noexcept { return m_crOddLineText; }

    EckInlineCe void LveSetEvenLineTextColor(COLORREF cr) noexcept { m_crEvenLineText = cr; }
    EckInlineNdCe COLORREF LveGetEvenLineTextColor() const noexcept { return m_crEvenLineText; }

    EckInlineCe void LveSetOddLineBackgroundColor(COLORREF cr) noexcept { m_crOddLineBk = cr; }
    EckInlineNdCe COLORREF LveGetOddLineBackgroundColor() const noexcept { return m_crOddLineBk; }

    EckInlineCe void LveSetEvenLineBackgroundColor(COLORREF cr) noexcept { m_crEvenLineBk = cr; }
    EckInlineNdCe COLORREF LveGetEvenLineBackgroundColor() const noexcept { return m_crEvenLineBk; }

    EckInlineCe void LveSetGridLineHColor(COLORREF cr) noexcept { m_crGridLineH = cr; }
    EckInlineNdCe COLORREF LveGetGridLineHColor() const noexcept { return m_crGridLineH; }

    EckInlineCe void LveSetGridLineVColor(COLORREF cr) noexcept { m_crGridLineV = cr; }
    EckInlineNdCe COLORREF LveGetGridLineVColor() const noexcept { return m_crGridLineV; }

    constexpr void LveSetColorPack(const LVE_COLOR_PACK& cp) noexcept
    {
        if (cp.crDefText != CLR_INVALID)
            m_crDefText = cp.crDefText;
        if (cp.crHeaderText != CLR_INVALID)
            m_crHeaderText = cp.crHeaderText;
        if (cp.crOddLineText != CLR_INVALID)
            m_crOddLineText = cp.crOddLineText;
        if (cp.crEvenLineText != CLR_INVALID)
            m_crEvenLineText = cp.crEvenLineText;
        if (cp.crOddLineBk != CLR_INVALID)
            m_crOddLineBk = cp.crOddLineBk;
        if (cp.crEvenLineBk != CLR_INVALID)
            m_crEvenLineBk = cp.crEvenLineBk;
        if (cp.crGridLineH != CLR_INVALID)
            m_crGridLineH = cp.crGridLineH;
        if (cp.crGridLineV != CLR_INVALID)
            m_crGridLineV = cp.crGridLineV;
    }

    void LveSetItem(const LVE_ITEM_EXT& ie) noexcept
    {
        EckAssert(ie.idxItem >= 0 && ie.idxItem < GetItemCount());
        EckAssert(ie.idxSubItem < m_Header.GetItemCount());
        LVE_ITEM_DATA* pData;
        LVITEMW li;
        li.mask = LVIF_PARAM;
        li.iItem = ie.idxItem;
        li.iSubItem = 0;
        li.lParam = 0;
        GetItem(&li);
        if (li.lParam)
        {
            pData = (LVE_ITEM_DATA*)li.lParam;
            EckAssert(pData->dwMagic == LveItemDataMagic);
        }
        else
        {
            if (m_vRecycleData.empty())
                pData = new LVE_ITEM_DATA{};
            else
            {
                pData = m_vRecycleData.back();
                m_vRecycleData.pop_back();
                *pData = {};
            }
            li.lParam = (LPARAM)pData;
            SetItem(&li);
        }

        if (ie.idxSubItem < 0)
        {
            if (ie.uMask & LVE_IM_COLOR_BK)
                pData->LineClr.crBk = ie.Clr.crBk;
            if (ie.uMask & LVE_IM_COLOR_TEXT)
                pData->LineClr.crText = ie.Clr.crText;
            if (ie.uMask & LVE_IM_COLOR_TEXTBK)
                pData->LineClr.crTextBk = ie.Clr.crTextBk;
        }
        else if (ie.uMask & LVE_IM_COLOR_ALL)
        {
            auto& e = pData->CellClr[ie.idxSubItem];
            if (ie.uMask & LVE_IM_COLOR_BK)
                e.crBk = ie.Clr.crBk;
            if (ie.uMask & LVE_IM_COLOR_TEXT)
                e.crText = ie.Clr.crText;
            if (ie.uMask & LVE_IM_COLOR_TEXTBK)
                e.crTextBk = ie.Clr.crTextBk;
        }

        if (ie.uMask & LVE_IM_LPARAM)
            pData->lParam = ie.lParam;
    }

    BOOL LveGetItem(LVE_ITEM_EXT& ie) const noexcept
    {
        EckAssert(ie.idxItem >= 0 && ie.idxItem < GetItemCount());
        EckAssert(ie.idxSubItem < m_Header.GetItemCount());
        LVITEMW li;
        li.mask = LVIF_PARAM;
        li.iItem = ie.idxItem;
        li.iSubItem = 0;
        li.lParam = 0;
        GetItem(&li);
        if (!li.lParam)
            return FALSE;
        const auto pData = (LVE_ITEM_DATA*)li.lParam;
        EckAssert(pData->dwMagic == LveItemDataMagic);
        if (ie.uMask & LVE_IM_COLOR_ALL)
            if (ie.idxSubItem < 0)
            {
                if (ie.uMask & LVE_IM_COLOR_BK)
                    ie.Clr.crBk = pData->LineClr.crBk;
                if (ie.uMask & LVE_IM_COLOR_TEXT)
                    ie.Clr.crText = pData->LineClr.crText;
                if (ie.uMask & LVE_IM_COLOR_TEXTBK)
                    ie.Clr.crTextBk = pData->LineClr.crTextBk;
            }
            else
            {
                const auto it = pData->CellClr.find(ie.idxSubItem);
                if (it == pData->CellClr.end())
                    return FALSE;
                const auto& e = it->second;
                if (ie.uMask & LVE_IM_COLOR_BK)
                    ie.Clr.crBk = e.crBk;
                if (ie.uMask & LVE_IM_COLOR_TEXT)
                    ie.Clr.crText = e.crText;
                if (ie.uMask & LVE_IM_COLOR_TEXTBK)
                    ie.Clr.crTextBk = e.crTextBk;
            }
        if (ie.uMask & LVE_IM_LPARAM)
            ie.lParam = pData->lParam;
        return TRUE;
    }

    /// <summary>
    /// 置所有者数据回调。
    /// 提供一种在适用场合下绕过文本长度限制和二次复制的机制
    /// </summary>
    /// <param name="pfn"></param>
    /// <param name="pParam"></param>
    void LveSetOwnerDataProc(FOwnerData pfn, void* pParam) noexcept
    {
        m_pfnOwnerData = pfn;
        m_pOdProcData = pParam;
    }

    void LveSetHeaderHeight(int cy) noexcept
    {
        m_cyHeader = cy;
        if (m_cyHeader > 0)
        {
            if (!m_bInstalledHeaderHook)
            {
                m_Header.GetSignal().Connect(
                    [this](HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, SlotCtx& Ctx)->LRESULT
                    {
                        if (uMsg == HDM_LAYOUT && m_cyHeader > 0)
                        {
                            Ctx.Processed();
                            const auto lResult = m_Header.OnMessage(hWnd, uMsg, wParam, lParam);
                            const auto phdlo = (HDLAYOUT*)lParam;
                            phdlo->prc->top = m_cyHeader;// 这个矩形是ListView工作区的矩形，就是表头矩形的补集
                            phdlo->pwpos->cy = m_cyHeader;
                            return lResult;
                        }
                        return 0;
                    }, MHI_LVE_HEADER_HEIGHT);
                m_bInstalledHeaderHook = TRUE;
            }
        }
        else if (m_bInstalledHeaderHook)
        {
            m_Header.GetSignal().Disconnect(MHI_LVE_HEADER_HEIGHT);
            m_bInstalledHeaderHook = FALSE;
        }
        // force recalc it
        Update();
        RECT rc;
        GetClientRect(HWnd, &rc);
        SendMessage(WM_SIZE, 0, MAKELPARAM(rc.right, rc.bottom));
    }

    CHeaderExt& LveGetHeader() noexcept { return m_Header; }

    EckInlineCe void LveSetCustomDraw(BOOL b) noexcept { m_bCustomDraw = b; }
    EckInlineNdCe BOOL LveGetCustomDraw() const noexcept { return m_bCustomDraw; }

    EckInlineCe void LveSetAutoDarkMode(BOOL b) noexcept { m_bAutoDarkMode = b; }
    EckInlineNdCe BOOL LveGetAutoDarkMode() const noexcept { return m_bAutoDarkMode; }

    EckInlineCe void LveSetAlphaColorInDark(BOOL b) noexcept { m_bAlphaClrInDark = b; }
    EckInlineNdCe BOOL LveGetAlphaColorInDark() const noexcept { return m_bAlphaClrInDark; }

    EckInlineCe void LveSetAlphaValue(BYTE b) noexcept { m_byColorAlpha = b; }
    EckInlineNdCe BYTE LveGetAlphaValue() const noexcept { return m_byColorAlpha; }

    EckInlineCe void LveSetAutoColorPack(BOOL b) noexcept { m_bAutoColorPack = b; }
    EckInlineNdCe BOOL LveGetAutoColorPack() const noexcept { return m_bAutoColorPack; }

    EckInlineCe void LveSetAddSplitterForColor(BOOL b) noexcept { m_bAddSplitterForClr = b; }
    EckInlineNdCe BOOL LveGetAddSplitterForColor() const noexcept { return m_bAddSplitterForClr; }

    EckInlineCe void LveSetCtrlASelectAll(BOOL b) noexcept { m_bCtrlASelectAll = b; }
    EckInlineNdCe BOOL LveGetCtrlASelectAll() const noexcept { return m_bCtrlASelectAll; }

    EckInlineCe void LveSetImplementOwnerDataNotify(BOOL b) noexcept { m_bImplLvOdNotify = b; }
    EckInlineNdCe BOOL LveGetImplementOwnerDataNotify() const noexcept { return m_bImplLvOdNotify; }

    EckInlineCe void LveSetDoNotWrapInTile(BOOL b) noexcept { m_bDoNotWrapInTile = b; }
    EckInlineNdCe BOOL LveGetDoNotWrapInTile() const noexcept { return m_bDoNotWrapInTile; }

    EckInline void LveSetOwnerDataBufferSize(int cch) noexcept { m_rsTextBuf.ReSize(cch); }
    EckInlineNdCe int LveGetOwnerDataBufferSize() const noexcept { return m_rsTextBuf.Size(); }
};
ECK_NAMESPACE_END