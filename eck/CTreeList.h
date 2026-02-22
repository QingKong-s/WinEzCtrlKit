#pragma once
#include "CHeaderExt.h"
#include "CEditExt.h"
#include "CToolTip.h"
#include "GraphicsHelper.h"
#include "CtrlGraphics.h"
#include "CScrollBar.h"

#include <vssym32.h>

ECK_NAMESPACE_BEGIN
// 项目标志
enum : UINT
{
    TLIF_CHECKED = (1u << 0),
    TLIF_PARTIALCHECKED = (1u << 1),// 不要修改前两个标志的值
    TLIF_HASNOTSIBLING = (1u << 2),
    TLIF_CLOSED = (1u << 3),
    TLIF_SELECTED = (1u << 4),
    TLIF_DISABLED = (1u << 5),
    TLIF_INVISIBLE = (1u << 6),
    TLIF_HASCHILDREN = (1u << 7),
    TLIF_DISABLECHECKBOX = (1u << 8),
};

// TLITEM掩码
enum : UINT
{
    TLIM_TEXT = (1u << 0),

    TLIM_ALL = TLIM_TEXT,
};

// 部件
enum
{
    TLIP_NONE,		// 空白
    TLIP_EXPANDBTN,	// 展开按钮
    TLIP_ICON,		// 图标
    TLIP_TEXT,		// 文本
    TLIP_CHECKBOX,	// 检查框
};

// 命中测试标志
enum : UINT
{
    TLHTF_ONLYITEM = (1u << 0),
    TLHTF_NOHITTEXTLABEL = (1u << 1),

    TLHTF_NULLTEXT = (1u << 2),
};

// 节点
struct TLNODE
{
    USHORT uFlags = 0u;	// TLIF_标志
    short iLevel = 0;	// 层次，根节点为1，此后逐层+1
    int idxParent = -1;	// 父节点索引，-1 = 根节点
    int idxImg = -1;	// 图像列表索引，-1 = 无效
    int idxLastEnd = -1;// 插入列表时用，上一个最后插入的子项索引
};

// 项目
struct TLITEM
{
    UINT uMask;
    int idxSubItem;
    TLNODE* pNode;
    int cchText;
    PCWSTR pszText;
};

// 命中测试结构
struct TLHITTEST
{
    POINT pt;
    int iPart;
    UINT uFlags;
    int idxSubItemDisplay;
};

struct NMTLFILLCHILDREN
{
    NMHDR nmhdr;
    BOOL bQueryRoot;	// 是否请求根节点
    TLNODE* pParent;	// 正在请求其子项目的父项
    int cChildren;		// 根节点的数量
    TLNODE** pChildren;	// 所有子项的数组
};

struct NMTLFILLALLFLATITEM
{
    NMHDR nmhdr;
    int cItem;			// 项目数
    TLNODE** pItems;	// 所有项目的数组
};

struct NMTLGETDISPINFO
{
    NMHDR nmhdr;
    TLITEM Item;
};

struct NMTLTLNODEEXPANDED
{
    NMHDR nmhdr;
    TLNODE* pNode;
};

struct NMTLCUSTOMDRAW : NMCUSTOMDRAWEXT
{
    TLNODE* pNode;
    int idxSubItem;
    int iPartIdGlyph;
    int iStateIdGlyph;
    COLORREF crTextBk;
};

enum : UINT
{
    TLEDF_BUILDINEDIT = (1u << 0),// 内置编辑框
    TLEDF_DONTEDIT = (1u << 1),// 禁止编辑
    TLEDF_CUSTOMEDIT = (1u << 2),// 自定义编辑
    TLEDF_SHOULDSAVETEXT = (1u << 3),// 应保存文本
};

struct NMTLEDIT
{
    NMHDR nmhdr;
    TLNODE* pNode;
    int idx;
    int idxSubItemDisplay;
    int idxSubItem;
    RECT rc;
    UINT uFlags;
};

struct NMTLDBCLICK
{
    NMHDR nmhdr;
    TLNODE* pNode;
    int idx;
    const TLHITTEST* pHitTest;
};

struct NMTLTTGETDISPINFO
{
    NMHDR nmhdr;
    NMTTDISPINFOW* pttdi;
    TLNODE* pNode;
    int idx;
    int idxSubItemDisplay;
    POINT pt;
};

struct NMTLTTPRESHOW
{
    NMHDR nmhdr;
    TLNODE* pNode;
    int idx;
    int idxSubItemDisplay;
    CToolTip* pToolTip;
};

struct NMTLMOUSECLICK
{
    NMHDR nmhdr;
    int idx;
    UINT uMsg;
    WPARAM wParam;
    LPARAM lParam;
    const TLHITTEST* pHitTestInfo;
};

struct NMTLCOMMITEM
{
    NMHDR nmhdr;
    TLNODE* pNode;
    int idx;
    int idxSubItemDisplay;
};

enum
{
    TLEIO_COLLAPSE,
    TLEIO_EXPAND,
    TLEIO_TOGGLE,
};

struct NMTLDRAG
{
    NMHDR nmhdr;
    const TLHITTEST* pHitTestInfo;
    int idx;
    BOOL bRBtn;
    UINT uKeyFlags;
};


class CTreeList;
class CTLHeader final : public CHeaderExt
{
    friend class CTreeList;
private:
    CTreeList& m_TL;
public:
    CTLHeader(CTreeList& tl) :m_TL{ tl } {}

    LRESULT OnMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept override;
};

class CTLEditExt final : public CEditExt
{
    friend class CTreeList;
private:
    CTreeList& m_TL;
public:
    CTLEditExt(CTreeList& tl) :m_TL{ tl } {}

    LRESULT OnMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept override;
};

class CTreeList : public CWindow
{
    friend class CTLHeader;
    friend class CTLEditExt;
public:
    ECK_RTTI(CTreeList, CWindow);
    ECK_CWND_SINGLEOWNER(CTreeList);
    ECK_CWND_CREATE_CLS_HINST(WCN_TREELIST, g_hInstance);

    enum
    {
        // 工具ID
        TLI_MAIN = 10,
        // 控件ID
        IDC_HEADER = 101,	// 第一列表头
        IDC_HEADERFIXED,	// 后续列的表头
        IDC_EDIT,	// 编辑框
        IDC_SBV,	// 滚动条
        IDC_SBH,	// 滚动条
        // 定时器ID
        IDT_EDITDELAY = 10
    };
private:
    struct COL
    {
        int iLeft;		// 左边界，相对于第一列的偏移量
        int iRight;		// 右边界，相对于第一列的偏移量
        int idxActual;	// 此项对应的实际项目索引
    };
    HWND m_hParent{};					// 接收通知的窗口
    //--------控件
    CTLHeader m_Header{ *this };		// 表头
    CTLHeader m_HeaderFixed{ *this };	// 表头
    CToolTip m_ToolTip{};				// 工具提示
    CTLEditExt m_Edit{ *this };			// 编辑框
    CScrollBar m_SBV{};					// 滚动条
    CScrollBar m_SBH{};					// 滚动条
    //--------图形
    HTHEME m_hThemeTV{};				// TreeView主题
    HTHEME m_hThemeLV{};				// ItemsView主题
    HTHEME m_hThemeBT{};				// Button主题
    HFONT m_hFont{};					// 字体
    CMemoryDC m_DC{};						// 兼容DC
    COLORREF m_crBranchLine = CLR_DEFAULT;	// 分支线颜色
    COLORREF m_crBkg = CLR_DEFAULT;		// 背景颜色
    COLORREF m_crText = CLR_DEFAULT;	// 文本颜色
    //--------项目相关
    std::vector<TLNODE*> m_vItem{};		// 项目列表
    std::vector<COL> m_vCol{};			// 按显示顺序排列的列信息

    int m_idxTopItem = 0;				// 第一可见项
    int m_idxHot = -1;					// 热点项
    int m_idxFocus = -1;				// 焦点项
    int m_idxMark = -1;					// mark，范围选择用
    int m_idxSel = -1;					// 选中的项，仅用于单选模式

    int m_idxToolTip = -1;				// 工具提示项
    int m_idxToolTipSubItemDisplay = -1;// 工具提示子项

    int m_idxEditing = -1;				// 正在编辑的项
    int m_idxEditingSubItemDisplay = -1;// 正在编辑的子项

    int m_idxCheckBoxLBtnDown = -1;		// 左键在选择框内按下的项目
    //--------尺寸
    int m_cxItem = 0,					// 项目宽度
        m_cyItem = 0;					// 项目高度
    int m_cxClient = 0,					// 客户区宽度
        m_cyClient = 0;					// 客户区高度

    int m_dxContent = 0;				// 水平偏移量，总小于等于0
    int m_cyHeader = 0;					// 表头高度

    SIZE m_sizeTVGlyph{};				// 展开按钮大小
    SIZE m_sizeCheckBox{};				// 检查框大小
    //--------拖动选择
    POINT m_ptDraggingSelStart{};		// 拖动选择起点，以相对于第一项的偏移量表示
    RECT m_rcDraggingSel{};				// 拖动选择矩形，以相对于第一项的偏移量表示
    //--------图像列表
    HIMAGELIST m_hImgList = nullptr;		// 图像列表
    int m_cxImg = 0,					// 图像宽度
        m_cyImg = 0;					// 图像高度
    //--------滚动
    int m_cScrollLine = 3;				// 一次滚动行数
    int m_cCharPreScrollH = 3;			// 一次滚动的字符数
    int m_cxCharAve = 0;				// 当前字体的平均字符宽度
    int m_msDraggingSelScrollGap = 30;	// 拖动选择时自动滚动最小间隔
    int m_cyHSB = 0;					// 水平滚动条高度，用于底部悬浮滚动条
    //--------
    CStringW m_rsWatermark{};			// 水印文本
    CStringW m_rsTextBuf{};				// 文本缓冲区
    //--------内部标志
#ifdef _DEBUG
    BITBOOL m_bDbgDrawIndex : 1 = 0;			// 【调试】绘制项目索引
    BITBOOL m_bDbgDrawMarkItem : 1 = 0;			// 【调试】绘制mark项
    BITBOOL m_bDbgDrawPartRect : 1 = 0;			// 【调试】绘制部件矩形	
#endif
    BITBOOL m_bExpandBtnHot : 1 = FALSE;		// 展开按钮是否点燃
    BITBOOL m_bDraggingSel : 1 = FALSE;			// 是否处于拖动选择状态
    BITBOOL m_bFocusIndicatorVisible : 1 = Dbg;	// 焦点指示器是否可见
    BITBOOL m_bHasFocus : 1 = FALSE;			// 是否有焦点
    BITBOOL m_bWaitEditDelay : 1 = FALSE;		// 是否等待编辑
    BITBOOL m_bBuildInEditChanged : 1 = FALSE;	// 内置编辑框内容是否已改变
    BITBOOL m_bDraggingItem : 1 = FALSE;		// 正在拖动项目
    BITBOOL m_bRDragging : 1 = FALSE;			// 正在右键拖动
    //--------风格
    BITBOOL m_bFlatMode : 1 = FALSE;					// 平面列表模式
    BITBOOL m_bFlatListFilter : 1 = FALSE;				// 平面列表模式下是否有不可见项目
    BITBOOL m_bSingleSel : 1 = FALSE;					// 单选
    BITBOOL m_bDisallowBeginDragInItemSpace : 1 = FALSE;// 禁止在项目内的空白区启动拖动选择
    BITBOOL m_bBackgroundNotSolid : 1 = FALSE;			// 背景不为纯色
    BITBOOL m_bHasLines : 1 = FALSE;					// 分支线
    BITBOOL m_bDisableAutoToolTip : 1 = FALSE;			// 禁止自动显示工具提示
    BITBOOL m_bCheckBox : 1 = FALSE;					// 复选框
    BITBOOL m_bDisableHScrollWithShift : 1 = FALSE;		// 禁止Shift+滚轮水平滚动
    BITBOOL m_bDisableSelectAllWithCtrlA : 1 = FALSE;	// 禁止Ctrl+A全选
    BITBOOL m_bEditLabel : 1 = FALSE;					// 编辑标签
    BITBOOL m_b3StateCheckBox : 1 = FALSE;				// 三态复选框
    BITBOOL m_bCascadeCheck : 1 = FALSE;				// 级联选中状态
    BITBOOL m_bSplitCol0 : 1 = FALSE;					// 分离第一列
    BITBOOL m_bStickyScroll : 1 = FALSE;				// 粘滞滚动
    //--------DPI相关
    int m_iDpi = USER_DEFAULT_SCREEN_DPI;	// DPI
    ECK_DS_BEGIN(DPIS)
        ECK_DS_ENTRY(cyHeaderDef, 26)		// 默认表头高度
        ECK_DS_ENTRY(cyItemDef, 24)			// 默认项目高度
        ECK_DS_ENTRY(cxTextMargin, 4)		// 文本水平边距
        ECK_DS_ENTRY(cxCBPadding, 2)		// 复选框水平边距
        ;
    ECK_DS_END_VAR(m_Ds);
    //--------

    /// <summary>
    /// 添加虚拟项目。
    /// 函数先将pParent尾插进列表，然后递归尾插其下所有子项
    /// </summary>
    /// <param name="pParent">当前父项目</param>
    /// <param name="pNode">子项目数组</param>
    /// <param name="cChildren">pNode指向的元素数</param>
    /// <param name="iLevel">父项目层次</param>
    void AddVirtualItem(TLNODE* pParent, TLNODE** pNode, int cChildren, int iLevel) noexcept
    {
        pParent->idxLastEnd = -1;// 清除脏数据
        int idxCurrParent;
        if (!(pParent->uFlags & TLIF_INVISIBLE))
        {
            m_vItem.push_back(pParent);// 可视，直接尾插
            idxCurrParent = (iLevel == 0 ? -1 : (int)m_vItem.size() - 1);
        }
        else
        {
            // 不可视，退回到最接近的可视父项
            int idxTemp = pParent->idxParent;
            while (idxTemp >= 0)
            {
                iLevel = m_vItem[idxTemp]->iLevel;
                if (!(m_vItem[idxTemp]->uFlags & TLIF_INVISIBLE))
                    break;
                idxTemp = m_vItem[idxTemp]->idxParent;
            }
            if (idxTemp < 0)
                iLevel = 0;
            idxCurrParent = idxTemp;
        }

        pParent->iLevel = iLevel;
        const int cOld = (int)m_vItem.size();
        if (!(pParent->uFlags & TLIF_CLOSED))
        {
            int cChildrenReal = 0;

            BOOL b = FALSE;
            EckCounter(cChildren, i)
            {
                NMTLFILLCHILDREN nm{};
                nm.pParent = pNode[i];
                pNode[i]->idxParent = idxCurrParent;
                if (!(pNode[i]->uFlags & TLIF_INVISIBLE) && idxCurrParent >= 0)
                {
                    // 确定同级项中的最后一个
                    const int idx = m_vItem[idxCurrParent]->idxLastEnd;
                    if (idx >= 0)
                        m_vItem[idx]->uFlags &= ~TLIF_HASNOTSIBLING;
                    pNode[i]->uFlags |= TLIF_HASNOTSIBLING;
                    m_vItem[idxCurrParent]->idxLastEnd = (int)m_vItem.size();
                }

                FillNmhdrAndSendNotify(nm, NM_TL_FILLCHILDREN);
                AddVirtualItem(nm.pParent, nm.pChildren, nm.cChildren, iLevel + 1);
            }

            if (!(pParent->uFlags & TLIF_INVISIBLE))
            {
                if (cOld != (int)m_vItem.size())
                    pParent->uFlags |= TLIF_HASCHILDREN;
                else
                    pParent->uFlags &= ~TLIF_HASCHILDREN;
            }
        }
    }

    void PaintItem(HDC hDC, int idx, const RECT& rcPaint) noexcept
    {
        //---------------准备
        const int x = m_dxContent;
        const int y = (idx - m_idxTopItem) * m_cyItem + m_cyHeader;
        const auto e = m_vItem[idx];

        RECT rcItem{ x,y,m_cxItem + x,y + m_cyItem };
        RECT rc{ x,rcItem.top, m_vCol.front().iRight + x,rcItem.bottom };// left和Right仅用于下面一行的判断，后续将被覆盖
        const BOOL bFirstColVisible = (rc.right - rc.left > 0 && (rc.left < rcPaint.right && rc.right > rcPaint.left));

        HRGN hRgn{};
#if ECK_OPT_SUPPORT_CLASSIC_THEME
        BOOL bTextHighLight{};
#endif // ECK_OPT_SUPPORT_CLASSIC_THEME
        //---------------画分隔线
        int iStateId;
        if (IsBitSet(e->uFlags, TLIF_SELECTED) || (m_bSingleSel && m_idxSel == idx))
        {
            if (idx == m_idxHot)
                iStateId = LISS_HOTSELECTED;
            else
                if (m_bHasFocus)
                    iStateId = LISS_SELECTED;
                else
                    iStateId = LISS_SELECTEDNOTFOCUS;
        }
        else if (idx == m_idxHot)
            iStateId = LISS_HOT;
        else
            iStateId = 0;

        NMTLCUSTOMDRAW nmcd;
        nmcd.dwDrawStage = CDDS_ITEMPREPAINT;
        nmcd.hdc = hDC;
        nmcd.rc = rcItem;
        nmcd.dwItemSpec = idx;
        nmcd.uItemState = e->uFlags;

        nmcd.iStateId = iStateId;
        nmcd.iPartId = LVP_LISTITEM;
        nmcd.crText = nmcd.crBk = CLR_DEFAULT;

        nmcd.pNode = e;
        nmcd.idxSubItem = -1;
        nmcd.iPartIdGlyph = (m_bExpandBtnHot && m_idxHot == idx) ? TVP_HOTGLYPH : TVP_GLYPH;
        nmcd.iStateIdGlyph = (e->uFlags & TLIF_CLOSED) ? GLPS_CLOSED : GLPS_OPENED;
        nmcd.crTextBk = CLR_DEFAULT;
        const auto uCustom = FillNmhdrAndSendNotify(nmcd, m_hParent, NM_CUSTOMDRAW);

        if (uCustom & CDRF_SKIPDEFAULT)
            goto End;
        PaintDivider(hDC, rcItem);
        //---------------画背景
#if ECK_OPT_SUPPORT_CLASSIC_THEME
        if (!m_hThemeLV)
        {
            if (nmcd.crBk == CLR_DEFAULT)
                switch (iStateId)
                {
                case LISS_SELECTED:
                    FillRect(hDC, &rcItem, GetSysColorBrush(COLOR_HIGHLIGHT));
                    break;
                case LISS_SELECTEDNOTFOCUS:
                    FillRect(hDC, &rcItem, GetSysColorBrush(COLOR_3DFACE));
                    break;
                case LISS_HOTSELECTED:
                    if (m_bHasFocus)
                        FillRect(hDC, &rcItem, GetSysColorBrush(COLOR_HOTLIGHT));
                    else
                        FillRect(hDC, &rcItem, GetSysColorBrush(COLOR_3DFACE));
                    break;
                case LISS_HOT:// discard
                default:
                    break;
                }
            else
            {
                SetDCBrushColor(hDC, nmcd.crBk);
                FillRect(hDC, &rcItem, GetStockBrush(DC_BRUSH));
            }
        }
        else
#endif // ECK_OPT_SUPPORT_CLASSIC_THEME
        {
            BOOL bPostDrawBk{};
            if (nmcd.crBk != CLR_DEFAULT)
            {
                if (ShouldAppsUseDarkMode())
                    bPostDrawBk = TRUE;
                else
                {
                    SetDCBrushColor(hDC, nmcd.crBk);
                    FillRect(hDC, &rcItem, GetStockBrush(DC_BRUSH));
                }
            }
            if (iStateId)
                DrawThemeBackground(m_hThemeLV, hDC, LVP_LISTITEM, iStateId, &rcItem, nullptr);
            if (bPostDrawBk)
                AlphaBlendColor(hDC, rcItem, nmcd.crBk);
        }
        //---------------
        if (m_hImgList || e->idxImg >= 0 || (e->uFlags & TLIF_HASCHILDREN))// 第一列有额外图案
        {
            rc.left = x;
            rc.right = m_vCol.front().iRight + x;
            // 整个第一列都不可见，那么内容也不会绘制，不必设置剪辑区
            if (bFirstColVisible)
            {
                hRgn = CreateRectRgnIndirect(&rc);
                SelectClipRgn(hDC, hRgn);
                DeleteObject(hRgn);
            }
        }

#if ECK_OPT_SUPPORT_CLASSIC_THEME
        //---------------
        if (!m_hThemeLV &&
            (iStateId == LISS_SELECTED || iStateId == LISS_HOTSELECTED))
        {
            if (nmcd.crText == CLR_DEFAULT)
                nmcd.crText = GetSysColor(COLOR_HIGHLIGHTTEXT);
            bTextHighLight = TRUE;
        }
#endif // ECK_OPT_SUPPORT_CLASSIC_THEME
        //---------------画展开按钮
        rc.left = x + CalculateExpandButtonLeftSpace(e);
        if (!m_bFlatMode)
        {
            rc.right = rc.left + m_sizeTVGlyph.cx;
            if ((e->uFlags & TLIF_HASCHILDREN) && bFirstColVisible)
            {
                rc.top += ((m_cyItem - m_sizeTVGlyph.cy) / 2);
                rc.bottom = rc.top + m_sizeTVGlyph.cy;
#if ECK_OPT_SUPPORT_CLASSIC_THEME
                if (!m_hThemeTV)
                {
                    InflateRect(rc, -m_sizeTVGlyph.cx / 6, -m_sizeTVGlyph.cy / 6);
                    DrawPlusMinusGlyph(hDC, (e->uFlags & TLIF_CLOSED), rc,
                        GetSysColor(COLOR_GRAYTEXT),
                        (bTextHighLight ? GetSysColor(COLOR_HIGHLIGHTTEXT) : GetSysColor(COLOR_WINDOWTEXT)));
                    InflateRect(rc, m_sizeTVGlyph.cx / 6, m_sizeTVGlyph.cy / 6);
                }
                else
#endif// ECK_OPT_SUPPORT_CLASSIC_THEME
                    DrawThemeBackground(m_hThemeTV, hDC, nmcd.iPartIdGlyph, nmcd.iStateIdGlyph, &rc, nullptr);
            }
            rc.left = rc.right;
        }
        //---------------画分支线
        if (m_bHasLines && !m_bFlatMode && bFirstColVisible)
        {
            SetDCPenColor(hDC,
                m_crBranchLine == CLR_DEFAULT ? GetSysColor(COLOR_GRAYTEXT) : m_crBranchLine);
            const HGDIOBJ hOldPen = SelectObject(hDC, GetStockBrush(DC_PEN));
            int idxParent = idx;

            BOOL bLastItem = (e->uFlags & TLIF_HASNOTSIBLING);
            if ((e->uFlags & TLIF_HASCHILDREN))
                bLastItem = bLastItem && (e->uFlags & TLIF_CLOSED);
            for (int i = e->iLevel - 1; i >= 1; --i)
            {
                if (idxParent >= 0)
                {
                    const int xLine = x + (i - 1) * m_sizeTVGlyph.cx + m_sizeTVGlyph.cx / 2;
                    const auto f = m_vItem[idxParent];

                    BOOL b = (f->uFlags & TLIF_HASNOTSIBLING);
                    if (b && (f->uFlags & TLIF_HASCHILDREN))
                        if (f->uFlags & TLIF_CLOSED)
                            b = bLastItem;

                    if (bLastItem = bLastItem && b)
                    {
                        const int yBottom = rcItem.top + m_cyItem * 4 / 5;
                        MoveToEx(hDC, xLine, rcItem.top, nullptr);
                        LineTo(hDC, xLine, yBottom);
                        LineTo(hDC, xLine + m_sizeTVGlyph.cx / 2, yBottom);
                    }
                    else
                    {
                        MoveToEx(hDC, xLine, rcItem.top, nullptr);
                        LineTo(hDC, xLine, rcItem.bottom);
                    }
                    idxParent = m_vItem[idxParent]->idxParent;
                }
            }
            SelectObject(hDC, hOldPen);
        }
        //---------------画检查框
        if (m_bCheckBox || m_b3StateCheckBox)
        {
            rc.left += m_Ds.cxCBPadding;
            rc.top = rcItem.top + (m_cyItem - m_sizeCheckBox.cy) / 2;
            rc.right = rc.left + m_sizeCheckBox.cx;
            rc.bottom = rc.top + m_sizeCheckBox.cy;
            EckAssert(GetLowNBits(e->uFlags, 2) <= 2);
            if (bFirstColVisible)
#if ECK_OPT_SUPPORT_CLASSIC_THEME
                if (!m_hThemeBT)
                {
                    UINT u = DFCS_BUTTONCHECK;
                    if (e->uFlags & TLIF_DISABLECHECKBOX)
                        u |= DFCS_INACTIVE;
                    if (e->uFlags & TLIF_CHECKED)
                        u |= DFCS_CHECKED;
                    DrawFrameControl(hDC, &rc, DFC_BUTTON, u);
                }
                else
                {
#endif// ECK_OPT_SUPPORT_CLASSIC_THEME
                    if (e->uFlags & TLIF_CHECKED)
                        iStateId = ((e->uFlags & TLIF_DISABLECHECKBOX) ?
                            CBS_CHECKEDDISABLED : CBS_CHECKEDNORMAL);
                    else if (e->uFlags & TLIF_PARTIALCHECKED)
                        iStateId = ((e->uFlags & TLIF_DISABLECHECKBOX) ?
                            CBS_MIXEDDISABLED : CBS_MIXEDNORMAL);
                    else
                        iStateId = ((e->uFlags & TLIF_DISABLECHECKBOX) ?
                            CBS_UNCHECKEDDISABLED : CBS_UNCHECKEDNORMAL);
                    DrawThemeBackground(m_hThemeBT, hDC, BP_CHECKBOX, iStateId, &rc, nullptr);
#if ECK_OPT_SUPPORT_CLASSIC_THEME
                }
#endif
            rc.left = rc.right + m_Ds.cxCBPadding;
        }
        //---------------画图片
        if (m_hImgList)
        {
            if (e->idxImg >= 0 && bFirstColVisible)
            {
                EckAssert(e->idxImg < ImageList_GetImageCount(m_hImgList));
                rc.top = y + (m_cyItem - m_cyImg) / 2;
                ImageList_Draw(m_hImgList, e->idxImg, hDC, rc.left, rc.top, ILD_NORMAL);
            }
            rc.left += m_cxImg;
        }
        if (hRgn)
            SelectClipRgn(hDC, nullptr);
        //---------------画文本
        COLORREF crText;
        if (nmcd.crText != CLR_DEFAULT)
            crText = nmcd.crText;
        else if (m_crText != CLR_DEFAULT)
            crText = m_crText;
        else
            crText = PtcCurrent()->crDefText;
        SetTextColor(hDC, crText);

        if (nmcd.crTextBk != CLR_DEFAULT)
        {
            SetBkMode(hDC, OPAQUE);
            SetBkColor(hDC, nmcd.crTextBk);
        }
        else
            SetBkMode(hDC, TRANSPARENT);

        SetTextColor(hDC, m_crText == CLR_DEFAULT ? PtcCurrent()->crDefText : m_crText);

        NMTLGETDISPINFO nm;
        nm.Item.pNode = e;
        nm.Item.uMask = TLIM_TEXT;
        nm.Item.pszText = m_rsTextBuf.Data();
        nm.Item.cchText = m_rsTextBuf.Size();
        FillNmhdr(nm, NM_TL_GETDISPINFO);

        rc.left += m_Ds.cxTextMargin;
        rc.top = rcItem.top;
        rc.bottom = rcItem.bottom;
        // 按显示顺序循环
        EckCounter(m_vCol.size(), i)
        {
            rc.right = m_vCol[i].iRight + x - m_Ds.cxTextMargin;
            if (rc.left >= m_cxClient)
                break;
            if (rc.right - rc.left > 0 && (rc.left < rcPaint.right && rc.right > rcPaint.left))
            {
                nm.Item.idxSubItem = m_vCol[i].idxActual;
                nm.Item.cchText = 0;
                nm.Item.pszText = nullptr;
                SendNotify(nm, m_hParent);
#pragma warning(suppress:6387)// nm.Item.pszText可能为NULL
                DrawTextW(hDC, nm.Item.pszText, nm.Item.cchText, &rc,
                    DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX);
            }
            rc.left = rc.right + m_Ds.cxTextMargin * 2;
#ifdef _DEBUG
            if (m_bDbgDrawPartRect)
            {
                RECT rc2;
                GetSubItemLabelRect(idx, (int)i, rc2);
                SetDCBrushColor(hDC, Colorref::Pink);
                FrameRect(hDC, &rc2, GetStockBrush(DC_BRUSH));
            }
#endif
        }
        if (!(uCustom & CDRF_SKIPPOSTPAINT))
        {
            if (idx == m_idxFocus && m_bFocusIndicatorVisible && m_bHasFocus)
            {
                InflateRect(rcItem, -2, -2);
                DrawFocusRect(hDC, &rcItem);
            }
        }
        if (uCustom & CDRF_NOTIFYPOSTPAINT)
        {
            nmcd.dwDrawStage = CDDS_POSTPAINT;
            SendNotify(nmcd, m_hParent);
        }

    End:;
#ifdef _DEBUG
        const auto cr = SetTextColor(hDC, Colorref::Red);
        if (m_bDbgDrawIndex)
            DrawTextW(hDC, ToString(idx).Data(), -1, (RECT*)&rcItem,
                DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX | DT_NOCLIP);

        if (m_bDbgDrawMarkItem && idx == m_idxMark)
        {
            rcItem.left += x;
            rcItem.right = m_vCol.front().iRight + x;
            DrawTextW(hDC, L"Mark", -1, (RECT*)&rcItem,
                DT_VCENTER | DT_SINGLELINE | DT_RIGHT | DT_NOPREFIX | DT_NOCLIP);
        }
        SetTextColor(hDC, cr);

        if (m_bDbgDrawPartRect)
        {
            if (e->uFlags & TLIF_HASCHILDREN)
            {
                SetDCBrushColor(hDC, Colorref::Green);
                GetPartRect(idx, TLIP_EXPANDBTN, rc);
                FrameRect(hDC, &rc, GetStockBrush(DC_BRUSH));
            }
            if (m_bCheckBox)
            {
                SetDCBrushColor(hDC, Colorref::Aqua);
                GetPartRect(idx, TLIP_CHECKBOX, rc);
                FrameRect(hDC, &rc, GetStockBrush(DC_BRUSH));
            }
            if (m_hImgList)
            {
                SetDCBrushColor(hDC, Colorref::Purple);
                GetPartRect(idx, TLIP_ICON, rc);
                FrameRect(hDC, &rc, GetStockBrush(DC_BRUSH));
            }
        }
#endif
    }

    EckInline void PaintBackground(HDC hDC, const RECT& rc) noexcept
    {
        NMTLCUSTOMDRAW nm{};
        nm.hdc = hDC;
        nm.rc = rc;
        nm.dwDrawStage = CDDS_PREERASE;
        const auto lRet = FillNmhdrAndSendNotify(nm, m_hParent, NM_CUSTOMDRAW);
        if (lRet & CDRF_SKIPDEFAULT)
            return;
        if (m_crBkg == CLR_DEFAULT)
            SetDCBrushColor(hDC, PtcCurrent()->crDefBkg);
        else
            SetDCBrushColor(hDC, m_crBkg);
        FillRect(hDC, &rc, GetStockBrush(DC_BRUSH));
        if (lRet & CDRF_NOTIFYPOSTERASE)
        {
            nm.dwDrawStage = CDDS_POSTERASE;
            FillNmhdrAndSendNotify(nm, m_hParent, NM_CUSTOMDRAW);
        }
    }

    EckInline void PaintDivider(HDC hDC, const RECT& rc) noexcept
    {
        for (const auto& e : m_vCol)
            DrawListViewColumnDetail(m_hThemeLV, hDC, e.iRight + m_dxContent,
                rc.top, rc.bottom);
    }

    EckInline void PaintDraggingSelectRect(HDC hDC) noexcept
    {
        if (m_bDraggingSel)
        {
#if ECK_OPT_SUPPORT_CLASSIC_THEME
            if (!m_hThemeLV)
                DrawFocusRect(hDC, &m_rcDraggingSel);
            else
#endif
                DrawSelectionRect(hDC, m_rcDraggingSel);
        }
    }

    EckInline void ReCalculateTopItem() noexcept
    {
        m_idxTopItem = ScbGetPosition(SB_VERT);
    }

    EckInline void UpdateThemeInfomation() noexcept
    {
#if ECK_OPT_SUPPORT_CLASSIC_THEME
        if (!m_hThemeTV)
            m_sizeTVGlyph = { GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON) };
        else
#endif
            GetThemePartSize(m_hThemeTV, m_DC.GetDC(),
                TVP_GLYPH, GLPS_CLOSED, nullptr, TS_TRUE, &m_sizeTVGlyph);
#if ECK_OPT_SUPPORT_CLASSIC_THEME
        if (!m_hThemeBT)
            m_sizeCheckBox = { GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON) };
        else
#endif
            GetThemePartSize(m_hThemeBT, m_DC.GetDC(),
                BP_CHECKBOX, CBS_UNCHECKEDNORMAL, nullptr, TS_TRUE, &m_sizeCheckBox);
    }

    void UpdateColumnInfomation() noexcept
    {
        int x = 0;
        const int cCol = (int)m_vCol.size();
        if (!cCol)
            return;

        const auto piOrder = (int*)_malloca(sizeof(int) * cCol);
        EckCheckMem(piOrder);
        if (m_bSplitCol0)
        {
            HDITEMW hdi;
            hdi.mask = HDI_WIDTH;

            m_HeaderFixed.GetItem(0, &hdi);
            auto& e = m_vCol.front();
            e.iLeft = 0;
            e.iRight = hdi.cxy;
            e.idxActual = 0;

            m_Header.GetOrderArray(piOrder, cCol - 1);

            EckCounter(cCol - 1, i)
            {
                const int idx = piOrder[i];
                m_Header.GetItem(idx, &hdi);
                auto& e = m_vCol[i + 1];
                e.iLeft = x;
                e.iRight = x + hdi.cxy;
                e.idxActual = idx + 1;
                x += hdi.cxy;
            }
            m_cxItem = m_vCol.back().iRight;
        }
        else
        {
            m_Header.GetOrderArray(piOrder, cCol);

            HDITEMW hdi;
            hdi.mask = HDI_WIDTH;
            EckCounter(cCol, i)
            {
                const int idx = piOrder[i];
                m_Header.GetItem(idx, &hdi);
                auto& e = m_vCol[i];
                e.iLeft = x;
                e.iRight = x + hdi.cxy;
                e.idxActual = idx;
                x += hdi.cxy;
            }
            m_cxItem = m_vCol.back().iRight;
        }
        _freea(piOrder);
    }

    /// <summary>
    /// 更新滚动条
    /// </summary>
    void UpdateScrollBar() noexcept
    {
        int cxView = m_cxClient;
        int cyView = m_cyClient;
        BOOL bHSB = FALSE;
        if (cxView < m_cxItem)
        {
            bHSB = TRUE;
            if (!IsBitSet(GetStyle(), WS_HSCROLL))
                cyView -= GetSystemMetrics(SM_CYHSCROLL);
        }
        if (cyView < (int)m_vItem.size() * m_cyItem)
        {
            if (!IsBitSet(GetStyle(), WS_VSCROLL))
                cxView -= GetSystemMetrics(SM_CXVSCROLL);
            if (cxView < m_cxItem && !bHSB)
                if (!IsBitSet(GetStyle(), WS_HSCROLL))
                    cyView -= GetSystemMetrics(SM_CYHSCROLL);
        }

        SCROLLINFO si;

        if (cyView < m_cyHeader || m_vItem.empty())
        {
            si.fMask = SIF_ALL;
            si.nMin = si.nMax = si.nPos = 0;
            si.nPage = 0;
            ScbSetInfomation(SB_VERT, &si);
            ScbSetInfomation(SB_HORZ, &si);
        }

        si.fMask = SIF_POS;
        ScbGetInfomation(SB_VERT, &si);
        si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
        si.nMin = 0;
        si.nMax = (int)m_vItem.size() - 1;
        si.nPage = (cyView - m_cyHeader) / m_cyItem;
        ScbSetInfomation(SB_VERT, &si);
        ReCalculateTopItem();

        si.fMask = SIF_POS;
        ScbGetInfomation(SB_HORZ, &si);
        si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
        si.nMin = 0;
        si.nMax = (cxView >= m_cxItem) ? 0 : (m_cxItem - 1);
        si.nPage = cxView;
        ScbSetInfomation(SB_HORZ, &si);
        si.fMask = SIF_POS;
        ScbGetInfomation(SB_HORZ, &si);
        m_dxContent = -si.nPos;
    }

    void UpdateHeaderPosition() noexcept
    {
        if (m_bSplitCol0)
            m_Header.Left = m_dxContent + m_vCol.front().iRight;
        else
            m_Header.Left = m_dxContent;
    }

    void BeginDraggingSelect(UINT uMk, int xBegin, int yBegin) noexcept
    {
        EckAssert(!m_bDraggingSel);
        m_ptDraggingSelStart = { xBegin - m_dxContent,yBegin + m_idxTopItem * m_cyItem };
        m_rcDraggingSel = { m_ptDraggingSelStart.x,m_ptDraggingSelStart.y,
            m_ptDraggingSelStart.x,m_ptDraggingSelStart.y };
        m_bDraggingSel = TRUE;
        MSG msg;
        GetCursorPos(&msg.pt);
        auto ullTime = NtGetTickCount64();

        SetCapture(m_hWnd);
        while (GetCapture() == m_hWnd)// 如果捕获改变则应立即退出拖动循环
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

                case WM_MOUSEHWHEEL:
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
                    POINT pt ECK_GET_PT_LPARAM(msg.lParam);
                    const auto ullTimeNow = NtGetTickCount64();
                    //----------滚动
                    if (ullTimeNow - ullTime >= m_msDraggingSelScrollGap)
                    {
                        ullTime = ullTimeNow;
                        int xDelta = 0, yDelta = 0;
                        // 横向滚动
                        if (pt.x < 0)
                            xDelta = pt.x;
                        else if (pt.x > m_cxClient)
                            xDelta = pt.x - m_cxClient;
                        // 纵向滚动，向上舍入到整数行
                        if (pt.y < m_cyHeader)
                            yDelta = (pt.y - m_cyHeader + 1) / m_cyItem - 1;
                        else if (pt.y > m_cyClient)
                            yDelta = (pt.y - m_cyClient - 1) / m_cyItem + 1;

                        if (xDelta || yDelta)
                        {
                            SetRedraw(FALSE);
                            if (xDelta)
                                ScrollH(xDelta);
                            if (yDelta)
                                ScrollV(yDelta);
                            SetRedraw(TRUE);
                        }
                    }
                    //----------准备坐标
                    RECT rcOld = m_rcDraggingSel;
                    OffsetRect(rcOld, m_dxContent, -m_idxTopItem * m_cyItem);// 转客户坐标

                    const POINT ptStart
                    {
                        m_ptDraggingSelStart.x + m_dxContent,
                        m_ptDraggingSelStart.y - m_idxTopItem * m_cyItem
                    };

                    // 限位
                    if (pt.x < 0)
                        pt.x = 0;
                    if (pt.y < m_cyHeader)
                        pt.y = m_cyHeader;
                    if (pt.x > m_cxClient)
                        pt.x = m_cxClient;
                    if (pt.y > m_cyClient)
                        pt.y = m_cyClient;
                    m_rcDraggingSel = MakeRect(ptStart, pt);// 制矩形

                    if (EquRect(rcOld, m_rcDraggingSel))// 范围未变，退出
                    {
                        OffsetRect(m_rcDraggingSel, -m_dxContent, m_idxTopItem * m_cyItem);
                        break;
                    }
                    if (m_rcDraggingSel.right == m_rcDraggingSel.left &&
                        m_rcDraggingSel.bottom != m_rcDraggingSel.top)
                        m_rcDraggingSel.right = m_rcDraggingSel.left + 1;
                    //----------准备范围
                    int idxBegin = ItemFromY(std::min(m_rcDraggingSel.top, rcOld.top));
                    if (idxBegin < 0)
                        idxBegin = 0;
                    int idxEnd = ItemFromY(std::max(m_rcDraggingSel.bottom, rcOld.bottom));
                    if (idxEnd < 0)
                        idxEnd = (int)m_vItem.size() - 1;
                    //----------
                    int dCursorToItemMax = INT_MIN;
                    if (!(msg.wParam & (MK_CONTROL | MK_SHIFT)))
                        m_idxMark = -1;// 重置mark
                    //----------
                    RECT rcItem;
                    for (int i = idxBegin; i <= idxEnd; ++i)
                    {
                        const auto e = m_vItem[i];
                        if (!(e->uFlags & TLIF_DISABLED))
                        {
                            GetItemRect(i, rcItem);
                            const BOOL bIntersectOld = IsRectsIntersect(rcItem, rcOld);
                            const BOOL bIntersectNew = IsRectsIntersect(rcItem, m_rcDraggingSel);

                            if (msg.wParam & MK_CONTROL)
                            {
                                if (bIntersectOld != bIntersectNew)
                                    e->uFlags ^= TLIF_SELECTED;// 翻转选中状态
                            }
                            else
                            {
                                if (bIntersectOld && !bIntersectNew)
                                    e->uFlags &= ~TLIF_SELECTED;// 先前选中但是现在未选中，清除选中状态
                                else if (!bIntersectOld && bIntersectNew)
                                    e->uFlags |= TLIF_SELECTED;// 先前未选中但是现在选中，设置选中状态
                                // mark设为离光标最远的选中项（标准ListView的行为）
                                if (bIntersectNew && !(msg.wParam & (MK_CONTROL | MK_SHIFT)))
                                {
                                    const int d = Abs(pt.y - rcItem.top);
                                    if (d > dCursorToItemMax)
                                    {
                                        dCursorToItemMax = d;
                                        m_idxMark = i;
                                    }
                                }
                            }
                        }
                    }

                    Redraw();
                    UpdateWindow(m_hWnd);
                    OffsetRect(m_rcDraggingSel, -m_dxContent, m_idxTopItem * m_cyItem);
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
            {
                POINT pt{ msg.pt };
                ScreenToClient(m_hWnd, &pt);
                BOOL b = FALSE;
                SCROLLINFO si;
                si.cbSize = sizeof(si);
                si.fMask = SIF_POS | SIF_PAGE | SIF_RANGE;
                if (GetStyle() & WS_VSCROLL)
                {
                    ScbGetInfomation(SB_VERT, &si);
                    if (pt.y < m_cyHeader)
                    {
                        if (si.nPos > si.nMin)
                            b = TRUE;
                    }
                    else if (pt.y > m_cyClient)
                    {
                        if (si.nPos < si.nMax - (int)si.nPage + 1)
                            b = TRUE;
                    }
                }

                if (GetStyle() & WS_HSCROLL)
                {
                    ScbGetInfomation(SB_HORZ, &si);
                    if (pt.x < 0)
                    {
                        if (si.nPos > si.nMin)
                            b = TRUE;
                    }
                    else if (pt.x > m_cxClient)
                    {
                        if (si.nPos < si.nMax - (int)si.nPage + 1)
                            b = TRUE;
                    }
                }

                if (b)
                    SetCursorPos(msg.pt.x, msg.pt.y);// 保持触发WM_MOUSEMOVE
                else
                    WaitMessage();
            }
        }
    ExitDraggingLoop:
        ReleaseCapture();
        m_bDraggingSel = FALSE;
        OffsetRect(m_rcDraggingSel, m_dxContent, -m_idxTopItem * m_cyItem);// 转客户坐标
        Redraw(m_rcDraggingSel);
        UpdateWindow(m_hWnd);
    }

    void UpdateDCAttribute() noexcept
    {
        SetTextColor(m_DC.GetDC(), GetSysColor(COLOR_WINDOWTEXT));
        SetBkMode(m_DC.GetDC(), TRANSPARENT);
        SelectObject(m_DC.GetDC(), m_hFont);
        TEXTMETRICW tm;
        GetTextMetricsW(m_DC.GetDC(), &tm);
        m_cxCharAve = tm.tmAveCharWidth;
    }

    void UpdateSystemParameter() noexcept
    {
        SystemParametersInfoW(SPI_GETWHEELSCROLLLINES, 0, &m_cScrollLine, 0);
        SystemParametersInfoW(SPI_GETWHEELSCROLLCHARS, 0, &m_cCharPreScrollH, 0);
    }

    void CheckOldData() noexcept
    {
        const int c = (int)m_vItem.size();
        if (m_idxHot >= c)
        {
            m_idxHot = -1;
            m_bExpandBtnHot = FALSE;
        }
        if (m_idxFocus >= c)
            m_idxFocus = -1;
        if (m_idxMark >= c)
            m_idxMark = -1;
        if (m_idxSel >= c)
            m_idxSel = -1;
    }

    /// <summary>
    /// 检查项目索引数据是否在闭区间内，如果在，修改到建议的位置
    /// </summary>
    /// <param name="idxBegin">起始索引</param>
    /// <param name="idxEnd">终止索引</param>
    /// <param name="idxSuggestion">建议的索引</param>
    void CheckOldDataRange(int idxBegin, int idxEnd, int idxSuggestion) noexcept
    {
        if (m_idxHot >= idxBegin && m_idxHot <= idxEnd)
        {
            m_idxHot = -1;// 强行修改点燃项到其他位置是没有道理的
            m_bExpandBtnHot = FALSE;
        }
        if (m_idxFocus >= idxBegin && m_idxFocus <= idxEnd)
            m_idxFocus = idxSuggestion;
        if (m_idxMark >= idxBegin && m_idxMark <= idxEnd)
            m_idxMark = idxSuggestion;
        if (m_idxSel >= idxBegin && m_idxSel <= idxEnd)
            m_idxSel = idxSuggestion;
    }

    EckInline void DismissToolTip() noexcept
    {
        if (m_idxToolTip >= 0 && m_idxToolTipSubItemDisplay >= 0)
        {
            m_ToolTip.Pop();
            m_idxToolTip = m_idxToolTipSubItemDisplay = -1;
        }
    }

    void EnterEditDelay(int idx, int idxSubItemDisplay) noexcept
    {
        if (!m_bEditLabel)
            return;
        CancelEditDelay();
        m_bWaitEditDelay = TRUE;
        m_idxEditing = idx;
        m_idxEditingSubItemDisplay = idxSubItemDisplay;
        SetTimer(m_hWnd, IDT_EDITDELAY, 600, nullptr);
    }

    void CancelEditDelay() noexcept
    {
        if (!m_bEditLabel)
            return;
        if (m_bWaitEditDelay)
        {
            m_bWaitEditDelay = FALSE;
            m_idxEditing = m_idxEditingSubItemDisplay = -1;
            KillTimer(m_hWnd, IDT_EDITDELAY);
        }
    }

    void ButtonSelect(int idx, WPARAM wParam, int& idxChangeBegin, int& idxChangeEnd) noexcept
    {
        if (m_bSingleSel)
        {
            SelectItemForClick(idx, 0);
            UpdateWindow(m_hWnd);
        }
        else
        {
            if (!(wParam & (MK_CONTROL | MK_SHIFT)))// 若Ctrl和Shift都未按下，则清除所有项的选中
            {
                DeselectAll(idxChangeBegin, idxChangeEnd);
                if (idxChangeBegin >= 0)
                    RedrawItem(idxChangeBegin, idxChangeEnd);
            }
            if (idx >= 0)
            {
                if ((wParam & MK_SHIFT) && m_idxMark >= 0)
                {
                    const int idxBegin = std::min(idx, m_idxMark);
                    const int idxEnd = std::max(idx, m_idxMark);
                    SelectRangeForClick(idxBegin, idxEnd, idxChangeBegin, idxChangeEnd);
                    // Shift选择不修改mark
                    // m_idxMark = idx;
                    if (idxChangeBegin >= 0)
                        RedrawItem(idxChangeBegin, idxChangeEnd);
                }
                else if (wParam & MK_CONTROL)
                    ToggleSelectItemForClick(idx);
                else
                    SelectItemForClick(idx, TRUE);
            }
            UpdateWindow(m_hWnd);
        }
    }

    void OnLRButtonDown(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept
    {
        MSG msg{ hWnd,uMsg,wParam,lParam };
        m_ToolTip.RelayEvent(&msg);

        CancelEditDelay();
        SetFocus(hWnd);

        TLHITTEST tlht;
        tlht.pt = ECK_GET_PT_LPARAM(lParam);
        tlht.uFlags = 0;
        int idx = HitTest(tlht);

        if (tlht.iPart == TLIP_EXPANDBTN)// 命中展开按钮
            ExpandItem(idx, TLEIO_TOGGLE);// 翻转展开状态
        else if (tlht.iPart == TLIP_CHECKBOX)// 命中选择框
        {
            m_idxCheckBoxLBtnDown = idx;
            SetCapture(hWnd);
        }
        else
        {
            int idxChangeBegin = -1, idxChangeEnd = -1;
            BOOL bAlreadyDoSelect =
                (m_bSingleSel ?
                    (m_idxSel != idx) :
                    (idx >= 0 && !(m_vItem[idx]->uFlags & TLIF_SELECTED)));
            if (bAlreadyDoSelect)
                ButtonSelect(idx, wParam, idxChangeBegin, idxChangeEnd);

            const BOOL bRBtn = (uMsg == WM_RBUTTONDOWN);
            POINT pt{ tlht.pt };
            ClientToScreen(hWnd, &pt);
            if (IsMouseMovedBeforeDragging(hWnd, pt.x, pt.y))// YEILD
            {
                BOOL bAllowDragSel = FALSE;

                if (!m_bSingleSel)
                    if (idx < 0 ||
                        (bAlreadyDoSelect && (tlht.iPart == TLIP_NONE && !m_bDisallowBeginDragInItemSpace)))
                        bAllowDragSel = TRUE;

                if (bAllowDragSel)
                {
                    if (!bAlreadyDoSelect)
                        ButtonSelect(idx, wParam, idxChangeBegin, idxChangeEnd);
                    m_idxHot = -1;// 清除热点
                    BeginDraggingSelect((UINT)wParam, tlht.pt.x, tlht.pt.y);
                }
                else if (idx >= 0)
                {
                    m_bDraggingItem = TRUE;
                    m_bRDragging = bRBtn;
                    SetCapture(hWnd);
                    NMTLDRAG nm;
                    nm.pHitTestInfo = &tlht;
                    nm.bRBtn = bRBtn;
                    nm.idx = idx;
                    nm.uKeyFlags = (UINT)wParam;
                    FillNmhdrAndSendNotify(nm, NM_TL_BEGINDRAG);
                }
                else // ONLY FOR SINGLE SEL
                {
                    EckAssert(m_bSingleSel);
                    int idx = -1;
                    std::swap(m_idxSel, idx);
                    if (idx >= 0)
                        RedrawItem(idx);
                }
                UpdateWindow(hWnd);
                return;
            }

            if (!IsWindow(hWnd))// revalidate
                return;

            if (!bAlreadyDoSelect)
                ButtonSelect(idx, wParam, idxChangeBegin, idxChangeEnd);

            if (!(wParam & (MK_CONTROL | MK_SHIFT)) &&// Ctrl和Shift都未按下
                (m_bSingleSel || (idxChangeBegin == idx && idxChangeEnd == idx)))// 只有自己被选中
            {
                if ((tlht.uFlags & TLHTF_NULLTEXT ||
                    tlht.iPart == TLIP_TEXT))// 要么命中文本，要么在文本为空时命中空白
                    EnterEditDelay(idx, tlht.idxSubItemDisplay);
            }
            UpdateWindow(hWnd);
        }

        NMTLMOUSECLICK nm;
        nm.idx = idx;
        nm.pHitTestInfo = &tlht;
        nm.uMsg = uMsg;
        nm.wParam = wParam;
        nm.lParam = lParam;
        FillNmhdrAndSendNotify(nm, NM_TL_MOUSECLICK);
    }

    void OnKeyDown(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept
    {
        if (m_vItem.empty())
            return;
        CancelEditDelay();
        int idxChangeBegin, idxChangeEnd;
        const int idxOldFocus = m_idxFocus;
        int idxBottom = m_idxTopItem + (m_cyClient - m_cyHeader) / m_cyItem - 1;
        if (idxBottom >= (int)m_vItem.size())
            idxBottom = (int)m_vItem.size() - 1;
        const BOOL bCtrlPressed = GetAsyncKeyState(VK_CONTROL) & 0x8000;
        const BOOL bShiftPressed = GetAsyncKeyState(VK_SHIFT) & 0x8000;
        switch (wParam)
        {
        case VK_UP:
        {
            --m_idxFocus;
            if (m_idxFocus < 0)
                m_idxFocus = 0;
        }
        break;
        case VK_DOWN:
        {
            if (m_idxFocus >= 0)
            {
                ++m_idxFocus;
                if (m_idxFocus >= (int)m_vItem.size())
                    m_idxFocus = (int)m_vItem.size() - 1;
            }
        }
        break;
        case VK_PRIOR:
        {
            if (m_idxFocus == m_idxTopItem)
            {
                m_idxFocus -= (m_cyClient - m_cyHeader) / m_cyItem;
                if (m_idxFocus < 0)
                    m_idxFocus = 0;
            }
            else
                m_idxFocus = m_idxTopItem;
        }
        break;
        case VK_NEXT:
        {
            if (m_idxFocus == idxBottom)
            {
                m_idxFocus += (m_cyClient - m_cyHeader) / m_cyItem;
                if (m_idxFocus >= (int)m_vItem.size())
                    m_idxFocus = (int)m_vItem.size() - 1;
            }
            else
                m_idxFocus = idxBottom;
        }
        break;
        case VK_HOME:
            m_idxFocus = 0;
            break;
        case VK_END:
            m_idxFocus = (int)m_vItem.size() - 1;
            break;

        case VK_LEFT:// 如果当前焦点项目已展开，折叠之，否则移至父项目
        {
            if (m_idxFocus >= 0)
            {
                const auto e = m_vItem[m_idxFocus];
                if ((e->uFlags & TLIF_HASCHILDREN) && !(e->uFlags & TLIF_CLOSED))// 已展开
                {
                    idxChangeBegin = m_idxFocus;
                    const int idxOldTop = m_idxTopItem;
                    ExpandItem(m_idxFocus, TLEIO_TOGGLE, FALSE);
                    if (m_bSingleSel)
                        SelectItemForClick(m_idxFocus, 0);
                    else
                    {
                        int idx0, idx1;
                        DeselectAll(idx0, idx1);
                        if (idxChangeBegin > idx0)
                            idxChangeBegin = idx0;
                        SelectItemForClick(m_idxFocus, TRUE);
                    }
                    if (m_idxTopItem != idxOldTop)
                        Redraw();
                    else
                    {
                        RECT rc{ 0,GetItemY(idxChangeBegin),m_cxClient,m_cyClient };
                        Redraw(rc);
                    }
                }
                else if (e->idxParent >= 0)// 未展开，而且不是根项目
                {
                    if (m_bSingleSel)
                        SelectItemForClick(e->idxParent, 0);
                    else
                    {
                        DeselectAll(idxChangeBegin, idxChangeEnd);
                        SelectItemForClick(e->idxParent, TRUE);
                        if (idxChangeBegin >= 0)
                            RedrawItem(idxChangeBegin, idxChangeEnd);
                        RedrawItem(e->idxParent);
                    }
                }
                UpdateWindow(hWnd);
            }
        }
        return;
        case VK_RIGHT:// 如果当前焦点项目已折叠，展开之，否则移至第一个子项目
        {
            if (m_idxFocus >= 0)
            {
                const auto e = m_vItem[m_idxFocus];
                if ((e->uFlags & TLIF_HASCHILDREN) && (e->uFlags & TLIF_CLOSED))// 已折叠
                {
                    idxChangeBegin = m_idxFocus;
                    const int idxOldTop = m_idxTopItem;
                    ExpandItem(m_idxFocus, TLEIO_TOGGLE, FALSE);
                    if (m_bSingleSel)
                        SelectItemForClick(m_idxFocus, 0);
                    else
                    {
                        int idx0, idx1;
                        DeselectAll(idx0, idx1);
                        if (idxChangeBegin > idx0)
                            idxChangeBegin = idx0;
                        SelectItemForClick(m_idxFocus, TRUE);
                    }
                    if (m_idxTopItem != idxOldTop)
                        Redraw();
                    else
                    {
                        RECT rc{ 0,GetItemY(idxChangeBegin),m_cxClient,m_cyClient };
                        Redraw(rc);
                    }
                }
                else if ((e->uFlags & TLIF_HASCHILDREN))// 有子项目
                {
                    if (m_bSingleSel)
                        SelectItemForClick(m_idxFocus + 1, 0);
                    else
                    {
                        DeselectAll(idxChangeBegin, idxChangeEnd);
                        SelectItemForClick(m_idxFocus + 1, TRUE);
                        if (idxChangeBegin >= 0)
                            RedrawItem(idxChangeBegin, idxChangeEnd);
                    }
                }
                UpdateWindow(hWnd);
            }
        }
        return;

        case 'A':
        {
            if (!m_bSingleSel && (GetAsyncKeyState(VK_CONTROL) & 0x8000))
            {
                SelectRangeForClick(0, (int)m_vItem.size() - 1, idxChangeBegin, idxChangeEnd);
                if (idxChangeBegin >= 0)
                {
                    RedrawItem(idxChangeBegin, idxChangeEnd);
                    UpdateWindow(hWnd);
                }
            }
        }
        return;

        case VK_SPACE:
        {
            if (m_idxFocus >= 0)// XXX : 会被输入法占用
                if (bCtrlPressed)// 反转选中状态
                    ToggleSelectItemForClick(m_idxFocus);
                else if (bShiftPressed)// 范围选择到焦点项
                {
                    if (m_idxMark >= 0)
                    {
                        SelectRangeForClick(std::min(m_idxMark, m_idxFocus), std::max(m_idxMark, m_idxFocus),
                            idxChangeBegin, idxChangeEnd);
                        if (idxChangeBegin >= 0)
                            RedrawItem(idxChangeBegin, idxChangeEnd);
                    }
                }
                else if (m_bCheckBox)// 反转检查框状态
                    ToggleCheckItemForClick(m_idxFocus);
        }
        return;
        case VK_MULTIPLY:
            if (m_idxFocus >= 0)
                ExpandItemRecurse(m_idxFocus, TLEIO_EXPAND, TRUE);
            return;
        case VK_DIVIDE:
            if (m_idxFocus >= 0)
                ExpandItemRecurse(m_idxFocus, TLEIO_COLLAPSE, TRUE);
            return;
        case VK_ADD:
            if (bCtrlPressed)
            {
                const auto hCursorOld = SetCursor(LoadCursorW(nullptr, IDC_WAIT));
                EckCounter(m_vCol.size(), i)
                    AdjustColumnToFit((int)i);
                SetCursor(hCursorOld);
            }
            else
                ExpandItem(m_idxFocus, TLEIO_EXPAND, TRUE);
            return;
        case VK_SUBTRACT:
            ExpandItem(m_idxFocus, TLEIO_COLLAPSE, TRUE);
            return;
        }

        if (m_idxFocus >= 0)// 项目必须有效
        {
            if (!bCtrlPressed && !bShiftPressed)// Ctrl或Shift按下，不修改mark
                m_idxMark = m_idxFocus;
            if (m_idxFocus != idxOldFocus)// 已改变
            {
                if (!bCtrlPressed)// Ctrl未按下
                {
                    if (bShiftPressed)// Shift按下，执行范围选择
                    {
                        if (m_bSingleSel)// 单选不论，总是选中当前项
                            SelectItemForClick(m_idxFocus, 0);
                        else// 多选
                        {
                            if (m_idxMark >= 0)// mark有效，选择范围
                                SelectRangeForClick(std::min(m_idxFocus, m_idxMark),
                                    std::max(m_idxFocus, m_idxMark), idxChangeBegin, idxChangeEnd);
                            else// mark无效，只选中当前项
                            {
                                DeselectAll(idxChangeBegin, idxChangeEnd);
                                SelectItemForClick(m_idxFocus, TRUE);
                                if (m_idxFocus < idxChangeBegin || m_idxFocus > idxChangeEnd)
                                    RedrawItem(m_idxFocus);
                            }
                            RedrawItem(idxChangeBegin, idxChangeEnd);
                        }
                    }
                    else// Shift未按下
                    {
                        if (m_bSingleSel)
                            SelectItemForClick(m_idxFocus, 0);
                        else
                        {
                            DeselectAll(idxChangeBegin, idxChangeEnd);
                            SelectItemForClick(m_idxFocus, TRUE);
                            if (idxChangeBegin >= 0)
                                RedrawItem(idxChangeBegin, idxChangeEnd);
                            if (m_idxFocus < idxChangeBegin || m_idxFocus > idxChangeEnd)
                                RedrawItem(m_idxFocus);
                        }
                    }
                }
                else// Ctrl按下，则只移动焦点，如果必要，重画焦点项
                {
                    if (m_bFocusIndicatorVisible)
                    {
                        RedrawItem(m_idxFocus);
                        if (idxOldFocus >= 0)
                            RedrawItem(idxOldFocus);
                    }
                }

                if (m_idxFocus < m_idxTopItem || m_idxFocus > idxBottom)
                    ScrollV(m_idxFocus - idxOldFocus);// 执行滚动
                UpdateWindow(hWnd);
            }
        }
    }
public:
    ECKPROP(GetImageList, SetImageList)			HIMAGELIST ImageList;
    ECKPROP(GetDraggingSelectScrollGap,
        SetDraggingSelectScrollGap)				int DragSelScrollGap;
    ECKPROP(GetSingleSelect, SetSingleSelect)	BOOL SingleSel;
    ECKPROP(GetHasCheckBox, SetHasCheckBox)		BOOL CheckBox;
    ECKPROP(GetFlatMode, SetFlatMode)			BOOL FlatMode;
    ECKPROP(GetEditLabel, SetEditLabel)			BOOL EditLabel;
    ECKPROP(GetDisableAutoToolTip,
        SetDisableAutoToolTip)					BOOL DisableAutoToolTip;
    ECKPROP(GetBackgroundNotSolid,
        SetBackgroundNotSolid)					BOOL BkgndNotSolid;
    ECKPROP(GetHasLines, SetHasLines)			BOOL HasLines;
    ECKPROP(GetLineColor, SetLineColor)			COLORREF LineColor;
    ECKPROP(GetUseFilterInFlatMode,
        SetUseFilterInFlatMode)					BOOL UseFilterInFlatMode;
    ECKPROP(GetDisallowBeginDragInItemSpace,
        SetDisallowBeginDragInItemSpace)		BOOL DisallowBeginDragInItemSpace;
    ECKPROP(GetFocusItem, SetFocusItem)			int FocusItem;
    ECKPROP(GetMarkItem, SetMarkItem)			int MarkItem;
    ECKPROP(GetCurrentSelection, SetCurrentSelection)				int CurrSelItem;
    ECKPROP(GetItemHeight, SetItemHeight)		int ItemHeight;
    ECKPROP(GetHeaderHeight, SetHeaderHeight)	int HeaderHeight;
    ECKPROP(GetDisableHScrollWithShift,
        SetDisableHScrollWithShift)				BOOL DisableHScrollWithShift;
    ECKPROP(GetDisableSelectAllWithCtrlA,
        SetDisableSelectAllWithCtrlA)			BOOL DisableSelectAllWithCtrlA;

    LRESULT OnMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept override
    {
        switch (uMsg)
        {
        case WM_MBUTTONDOWN:
            CancelEditDelay();
            [[fallthrough]];
        case WM_MBUTTONUP:
        {
            NMTLMOUSECLICK nm;
            nm.idx = -1;
            nm.pHitTestInfo = nullptr;
            nm.uMsg = uMsg;
            nm.wParam = wParam;
            nm.lParam = lParam;
            FillNmhdrAndSendNotify(nm, NM_TL_MOUSECLICK);
        }
        [[fallthrough]];
        case WM_NCMOUSEMOVE:
        {
            MSG msg{ hWnd,uMsg,wParam,lParam };
            m_ToolTip.RelayEvent(&msg);
        }
        break;

        case WM_MOUSEMOVE:
        {
            MSG msg{ hWnd,uMsg,wParam,lParam };
            m_ToolTip.RelayEvent(&msg);

            TLHITTEST tlht;
            tlht.pt = ECK_GET_PT_LPARAM(lParam);
            tlht.uFlags = TLHTF_NOHITTEXTLABEL;
            int idx = HitTest(tlht);
            BOOL bExpBtnHot = m_bExpandBtnHot;
            m_bExpandBtnHot = (tlht.iPart == TLIP_EXPANDBTN);
            if (idx != m_idxHot)
            {
                std::swap(idx, m_idxHot);
                if (idx >= 0)
                    RedrawItem(idx);
                if (m_idxHot >= 0)
                    RedrawItem(m_idxHot);
            }
            else
            {
                if (idx >= 0 && bExpBtnHot != m_bExpandBtnHot)
                    RedrawItem(idx);
                if (m_idxHot >= 0 && bExpBtnHot != m_bExpandBtnHot)
                    RedrawItem(m_idxHot);
            }

            if (idx != m_idxToolTip || tlht.idxSubItemDisplay != m_idxToolTipSubItemDisplay)
                m_ToolTip.Pop();

            TRACKMOUSEEVENT tme;
            tme.cbSize = sizeof(tme);
            tme.dwFlags = TME_LEAVE;
            tme.hwndTrack = hWnd;
            TrackMouseEvent(&tme);
        }
        return 0;

        case WM_NOTIFY:
        {
            const auto pnmhdr = (NMHDR*)lParam;
            if (pnmhdr->hwndFrom == m_Header.HWnd)
                switch (pnmhdr->code)
                {
                case HDN_ITEMCHANGEDW:
                    //case HDN_ITEMCHANGINGW:
                {
                    const auto p = (NMHEADERW*)lParam;
                    if (IsBitSet(p->pitem->mask, HDI_WIDTH))
                    {
                        UpdateColumnInfomation();
                        UpdateScrollBar();
                        SetWindowPos(m_Header.HWnd, nullptr, m_dxContent, 0,
                            std::max(m_cxItem, m_cxClient), m_cyHeader, SWP_NOZORDER | SWP_NOACTIVATE);
                        Redraw();
                    }
                }
                return FALSE;

                case HDN_ITEMCLICKW:
                {
                    auto nm = *(const NMHEADERW*)lParam;
                    FillNmhdrAndSendNotify(nm, NM_TL_HD_CLICK);
                }
                return 0;

                case NM_RELEASEDCAPTURE:
                    UpdateColumnInfomation();
                    Redraw();
                    return 0;

                case HDN_DIVIDERDBLCLICKW:
                {
                    const auto p = (NMHEADERW*)lParam;
                    HDITEMW hdi;
                    hdi.mask = HDI_ORDER;
                    m_Header.GetItem(p->iItem, &hdi);
                    AdjustColumnToFit(hdi.iOrder);
                }
                return 0;
                }
            else if (m_bSplitCol0 && pnmhdr->hwndFrom == m_HeaderFixed.HWnd)
            {
                switch (pnmhdr->code)
                {
                case HDN_ITEMCHANGEDW:
                    //case HDN_ITEMCHANGINGW:
                {
                    const auto p = (NMHEADERW*)lParam;
                    if (IsBitSet(p->pitem->mask, HDI_WIDTH))
                    {
                        UpdateColumnInfomation();
                        UpdateScrollBar();
                        SetWindowPos(m_HeaderFixed.HWnd, nullptr, 0, 0,
                            m_vCol.front().iRight, m_cyHeader, SWP_NOZORDER | SWP_NOACTIVATE);
                        Redraw();
                    }
                }
                return FALSE;
                }
            }
            else if (pnmhdr->hwndFrom == m_ToolTip.HWnd)
                switch (pnmhdr->code)
                {
                case TTN_GETDISPINFOW:
                {
                    auto p = (NMTTDISPINFOW*)lParam;
                    if (p->uFlags & TTF_IDISHWND)
                    {
                        if (p->hdr.idFrom != (UINT_PTR)hWnd)
                            return 0;
                    }
                    else
                    {
                        if (p->hdr.idFrom != TLI_MAIN)
                            return 0;
                    }

                    const auto uPos = GetMessagePos();
                    POINT pt ECK_GET_PT_LPARAM(uPos);
                    ScreenToClient(hWnd, &pt);

                    TLHITTEST tlht;
                    tlht.pt = pt;
                    tlht.uFlags = TLHTF_ONLYITEM;
                    const int idx = HitTest(tlht);
                    if (idx >= 0)
                        if (m_bDisableAutoToolTip)
                        {
                            NMTLTTGETDISPINFO nm;
                            nm.pttdi = p;
                            nm.pNode = (idx >= 0 ? m_vItem[idx] : nullptr);
                            nm.idx = idx;
                            nm.idxSubItemDisplay = tlht.idxSubItemDisplay;
                            nm.pt = pt;
                            if (FillNmhdrAndSendNotify(nm, NM_TL_GETDISPINFO))
                            {
                                m_idxToolTip = idx;
                                m_idxToolTipSubItemDisplay = tlht.idxSubItemDisplay;
                                return 0;
                            }
                        }
                        else
                        {
                            if (IsTextTooLong(idx, tlht.idxSubItemDisplay))
                            {
                                m_idxToolTip = idx;
                                m_idxToolTipSubItemDisplay = tlht.idxSubItemDisplay;
                                p->lpszText = (PWSTR)GetItemText(m_idxToolTip,
                                    ColumnDisplayToActual(m_idxToolTipSubItemDisplay));
                                return 0;
                            }
                        }

                    m_idxToolTip = m_idxToolTipSubItemDisplay = -1;
                }
                return 0;
                case TTN_SHOW:
                {
                    if (m_idxToolTip >= 0 && m_idxToolTipSubItemDisplay >= 0)
                    {
                        if (m_bDisableAutoToolTip)
                        {
                            NMTLTTPRESHOW nm;
                            nm.pNode = m_vItem[m_idxToolTip];
                            nm.idx = m_idxToolTip;
                            nm.idxSubItemDisplay = m_idxToolTipSubItemDisplay;
                            nm.pToolTip = &m_ToolTip;
                            return FillNmhdrAndSendNotify(nm, NM_TL_TTPRESHOW);
                        }
                        else
                        {
                            RECT rc;
                            GetItemTextRect(m_idxToolTip, m_idxToolTipSubItemDisplay, rc);
                            ClientToScreen(hWnd, &rc);
                            m_ToolTip.AdjustRect(TRUE, &rc);
                            SetWindowPos(m_ToolTip.HWnd, HWND_TOPMOST,
                                rc.left, rc.top, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE);
                        }
                    }
                }
                return TRUE;
                case TTN_POP:
                    m_idxToolTip = m_idxToolTipSubItemDisplay = -1;
                    return 0;
                }
        }
        return 0;

        case WM_MOUSELEAVE:
        {
            if (m_idxHot != -1)
            {
                int idx = -1;
                std::swap(idx, m_idxHot);
                if (idx >= 0)
                    RedrawItem(idx);
            }
        }
        return 0;

        case WM_VSCROLL:
        {
            DismissEdit();
            DismissToolTip();
            SCROLLINFO si;
            si.cbSize = sizeof(si);
            si.fMask = SIF_ALL;
            ScbGetInfomation(SB_VERT, &si);
            const int iOld = si.nPos;
            switch (LOWORD(wParam))
            {
            case SB_TOP:
                si.nPos = si.nMin;
                break;
            case SB_BOTTOM:
                si.nPos = si.nMax;
                break;
            case SB_LINEUP:
                --si.nPos;
                break;
            case SB_LINEDOWN:
                ++si.nPos;
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
            if (m_bBackgroundNotSolid)
                Redraw();
            else
            {
                RECT rc{ 0,m_cyHeader,m_cxClient,m_cyClient };
                ScrollWindowEx(hWnd, 0, (iOld - si.nPos) * m_cyItem, &rc, &rc, nullptr, nullptr, SW_INVALIDATE);
            }
            UpdateWindow(hWnd);
        }
        return 0;

        case WM_HSCROLL:
        {
            DismissEdit();
            DismissToolTip();
            SCROLLINFO si;
            si.cbSize = sizeof(si);
            si.fMask = SIF_ALL;
            ScbGetInfomation(SB_HORZ, &si);
            switch (LOWORD(wParam))
            {
            case SB_LEFT:
                si.nPos = si.nMin;
                break;
            case SB_RIGHT:
                si.nPos = si.nMax;
                break;
            case SB_LINELEFT:
                si.nPos -= m_cyItem;
                break;
            case SB_LINERIGHT:
                si.nPos += m_cyItem;
                break;
            case SB_PAGELEFT:
                si.nPos -= si.nPage;
                break;
            case SB_PAGERIGHT:
                si.nPos += si.nPage;
                break;
            case SB_THUMBTRACK:
                si.nPos = si.nTrackPos;
                break;
            }

            si.fMask = SIF_POS;
            ScbSetInfomation(SB_HORZ, &si);
            ScbGetInfomation(SB_HORZ, &si);
            m_dxContent = -si.nPos;
            UpdateHeaderPosition();
            Redraw();
        }
        return 0;

        case WM_MOUSEWHEEL:
        {
            if ((!(GetStyle() & WS_VSCROLL)) || (!m_bDisableHScrollWithShift && (wParam & MK_SHIFT)))
                goto FallToHWheel;
            const int cVisible = (m_cyClient - m_cyHeader) / m_cyItem;
            const int dLine = -GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA * m_cScrollLine;
            if (Abs(dLine) > cVisible)
                ScrollV(SignValue(dLine) * ScbGetPage(SB_VERT));
            else
                ScrollV(dLine);
            UpdateWindow(hWnd);
        }
        return 0;

        case WM_MOUSEHWHEEL:
        {
        FallToHWheel:
            const int dx = -GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA * m_cCharPreScrollH * m_cxCharAve;
            if (Abs(dx) > m_cxClient)
                ScrollH(SignValue(dx) * m_cxClient);
            else
                ScrollH(dx);
            UpdateWindow(hWnd);
        }
        return 0;

        case WM_COMMAND:
        {
            if (HIWORD(wParam) == EN_CHANGE && lParam == (LPARAM)m_Edit.HWnd)
                m_bBuildInEditChanged = TRUE;
        }
        break;

        case WM_PRINTCLIENT:
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            BeginPaint(hWnd, wParam, ps);
            const int yOrg = ps.rcPaint.top;
            if (ps.rcPaint.top < m_cyHeader)// 永远不在表头控件区域内重画
                ps.rcPaint.top = m_cyHeader;
            //------------擦除
            PaintBackground(m_DC.GetDC(), ps.rcPaint);
            if (!m_vItem.empty())
            {
                //------------准备范围
                const int idxTop = std::max(m_idxTopItem + (ps.rcPaint.top - m_cyHeader) / m_cyItem, 0L);
                const int idxBottom = std::min(m_idxTopItem + (ps.rcPaint.bottom - m_cyHeader) / m_cyItem,
                    (long)m_vItem.size() - 1);
                //------------画分隔线
                PaintDivider(m_DC.GetDC(), ps.rcPaint);
                //------------画项目
                for (int i = idxTop; i <= idxBottom; ++i)
                    PaintItem(m_DC.GetDC(), i, ps.rcPaint);

            }
            else if (!m_rsWatermark.IsEmpty())
            {
                const auto crOld = SetTextColor(m_DC.GetDC(), GetSysColor(COLOR_GRAYTEXT));
                RECT rc{ 0,m_cyHeader + m_Ds.cxTextMargin,m_cxClient,m_cyClient };
                DrawTextW(m_DC.GetDC(), m_rsWatermark.Data(), m_rsWatermark.Size(), &rc,
                    DT_WORDBREAK | DT_EDITCONTROL | DT_NOPREFIX | DT_CENTER);
                SetTextColor(m_DC.GetDC(), crOld);
            }
            //------------画拖动选择矩形
            PaintDraggingSelectRect(m_DC.GetDC());
            BitBltPs(&ps, m_DC.GetDC());
            ps.rcPaint.top = yOrg;
            EndPaint(hWnd, wParam, ps);
        }
        return 0;

        case WM_SIZE:
        {
            ECK_GET_SIZE_LPARAM(m_cxClient, m_cyClient, lParam);
            m_DC.ReSize(hWnd, m_cxClient, m_cyClient);
            UpdateDCAttribute();

            UpdateScrollBar();
            m_Header.Size = SIZE{ std::max(m_cxItem, m_cxClient),m_cyHeader };

            TTTOOLINFOW ti;
            ti.cbSize = sizeof(ti);
            ti.hwnd = hWnd;
            ti.uId = TLI_MAIN;
            ti.rect = { 0,0,m_cxClient,m_cyClient };
            m_ToolTip.NewToolRect(&ti);
        }
        return 0;

        case WM_KEYDOWN:
            OnKeyDown(hWnd, uMsg, wParam, lParam);
            break;

        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
            OnLRButtonDown(hWnd, uMsg, wParam, lParam);
            break;
        case WM_LBUTTONUP:
        {
            if (m_idxCheckBoxLBtnDown >= 0)
            {
                EckAssert(m_bDraggingItem == FALSE);
                TLHITTEST tlht;
                tlht.pt = ECK_GET_PT_LPARAM(lParam);
                tlht.uFlags = 0;
                int idx = HitTest(tlht);
                if (idx == m_idxCheckBoxLBtnDown && tlht.iPart == TLIP_CHECKBOX)
                {
                    ToggleCheckItemForClick(m_idxCheckBoxLBtnDown);
                    RedrawItem(m_idxCheckBoxLBtnDown);
                }
                m_idxCheckBoxLBtnDown = -1;
                ReleaseCapture();
            }
        }
        [[fallthrough]];
        case WM_RBUTTONUP:
        {
            if (m_bDraggingItem)
            {
                if ((m_bRDragging && (uMsg == WM_RBUTTONUP)) ||
                    (!m_bRDragging && (uMsg == WM_LBUTTONUP)))
                {
                    m_bDraggingItem = FALSE;
                    ReleaseCapture();
                    NMTLDRAG nm;
                    nm.bRBtn = m_bRDragging;
                    nm.uKeyFlags = (UINT)wParam;
                    nm.pHitTestInfo = nullptr;
                    nm.idx = -1;
                    FillNmhdrAndSendNotify(nm, NM_TL_ENDDRAG);
                }
            }

            NMTLMOUSECLICK nm;
            nm.idx = -1;
            nm.pHitTestInfo = nullptr;
            nm.uMsg = uMsg;
            nm.wParam = wParam;
            nm.lParam = lParam;
            FillNmhdrAndSendNotify(nm, NM_TL_MOUSECLICK);

            MSG msg{ hWnd,uMsg,wParam,lParam };
            m_ToolTip.RelayEvent(&msg);
        }
        break;

        case WM_LBUTTONDBLCLK:
        {
            TLHITTEST tlht;
            tlht.pt = ECK_GET_PT_LPARAM(lParam);
            tlht.uFlags = TLHTF_NOHITTEXTLABEL;
            HitTest(tlht);
            if (tlht.iPart == TLIP_EXPANDBTN || tlht.iPart == TLIP_CHECKBOX)
                PostMessage(WM_LBUTTONDOWN, wParam, lParam);// 连击修正
        }
        return 0;

        case WM_UPDATEUISTATE:
        {
            if (!(HIWORD(wParam) & UISF_HIDEFOCUS))// 焦点指示器未改变
                break;
            const BOOL bOld = m_bFocusIndicatorVisible;
            switch (LOWORD(wParam))
            {
            case UIS_SET:
                m_bFocusIndicatorVisible = FALSE;
                break;
            case UIS_CLEAR:
                m_bFocusIndicatorVisible = TRUE;
                break;
            }
            if (m_bFocusIndicatorVisible != bOld && m_idxFocus >= 0)
                RedrawItem(m_idxFocus);
        }
        break;

        case WM_TIMER:
        {
            if (wParam != IDT_EDITDELAY)
                break;
            KillTimer(hWnd, IDT_EDITDELAY);
            if (m_bWaitEditDelay && m_idxEditing >= 0)
            {
                const int idx = m_idxEditing;
                const int idxSubItemDisplay = m_idxEditingSubItemDisplay;
                CancelEditDelay();
                EnterEdit(idx, idxSubItemDisplay);
            }
        }
        return 0;

        case WM_CREATE:
        {
            m_hParent = ((CREATESTRUCTW*)lParam)->hwndParent;
            m_iDpi = GetDpi(hWnd);
            UpdateDpiSize(m_Ds, m_iDpi);
            m_cyHeader = m_Ds.cyHeaderDef;
            m_cyItem = m_Ds.cyItemDef;
            m_DC.Create(hWnd);
            UpdateDCAttribute();
            m_Header.Create(nullptr, WS_CHILD | WS_VISIBLE | HDS_FULLDRAG | HDS_BUTTONS | HDS_DRAGDROP, 0,
                0, 0, ClientWidth, m_Ds.cyHeaderDef, hWnd, IDC_HEADER);
            //m_HeaderFixed.Create(NULL, WS_CHILD | WS_VISIBLE | HDS_FULLDRAG | HDS_BUTTONS | HDS_DRAGDROP, 0,
            //	0, 0, ClientWidth, m_Ds.cyHeaderDef, hWnd, IDC_HEADERFIXED);
            SetExplorerTheme();
            m_hThemeTV = OpenThemeData(hWnd, L"TreeView");
            m_hThemeLV = OpenThemeData(nullptr, L"ItemsView::ListView");
            m_hThemeBT = OpenThemeData(hWnd, L"Button");
            UpdateThemeInfomation();
            m_bHasFocus = (GetFocus() == hWnd);
            UpdateSystemParameter();

            m_ToolTip.Create(nullptr, TTS_NOPREFIX, WS_EX_TRANSPARENT,
                0, 0, 0, 0, hWnd, nullptr);
            TTTOOLINFOW ti{ sizeof(TTTOOLINFOW) };
            ti.uFlags = TTF_TRANSPARENT;
            ti.hwnd = hWnd;
            ti.uId = TLI_MAIN;
            ti.lpszText = LPSTR_TEXTCALLBACKW;
            m_ToolTip.AddTool(&ti);

            m_SBV.Create(nullptr, WS_CHILD | (m_bSplitCol0 ? WS_VISIBLE : 0) | SBS_VERT, 0,
                0, 0, 0, 0, hWnd, IDC_SBV);
            m_SBH.Create(nullptr, WS_CHILD | (m_bSplitCol0 ? WS_VISIBLE : 0) | SBS_HORZ, 0,
                0, 0, 0, 0, hWnd, IDC_SBH);
            UpdateScrollBar();
        }
        return 0;

        case WM_SETFONT:
        {
            m_hFont = (HFONT)wParam;
            SelectObject(m_DC.GetDC(), m_hFont);
            if (lParam)
                Redraw();
        }
        return 0;

        case WM_GETFONT:
            return (LRESULT)m_hFont;

        case WM_DPICHANGED_BEFOREPARENT:
            UpdateDpiSize(m_Ds, m_iDpi = GetDpi(hWnd));
            [[fallthrough]];
        case WM_THEMECHANGED:
        {
            CloseThemeData(m_hThemeTV);
            m_hThemeTV = OpenThemeData(hWnd, L"TreeView");
            CloseThemeData(m_hThemeLV);
            m_hThemeLV = OpenThemeData(nullptr, L"ItemsView::ListView");
            CloseThemeData(m_hThemeBT);
            m_hThemeBT = OpenThemeData(hWnd, L"Button");
            UpdateThemeInfomation();
            UpdateDCAttribute();
            Redraw();
        }
        break;

        case WM_SYSCOLORCHANGE:
        {
            UpdateDCAttribute();
            Redraw();
        }
        break;

        case WM_SETFOCUS:
        {
            m_bHasFocus = TRUE;
            if (m_bSingleSel)
            {
                if (m_idxSel >= 0)
                    RedrawItem(m_idxSel);
            }
            else
                Redraw();
        }
        break;

        case WM_KILLFOCUS:
        {
            m_bHasFocus = FALSE;
            if (m_bSingleSel)
            {
                if (m_idxSel >= 0)
                    RedrawItem(m_idxSel);
            }
            else
                Redraw();
        }
        break;

        case WM_DESTROY:
        {
            CloseThemeData(m_hThemeTV);
            m_hThemeTV = nullptr;
            CloseThemeData(m_hThemeLV);
            m_hThemeLV = nullptr;
            CloseThemeData(m_hThemeBT);
            m_hThemeBT = nullptr;
            m_ToolTip.Destroy();
            m_vItem.clear();
            m_vCol.clear();
            m_rsWatermark = {};
            m_hImgList = nullptr;
            m_cxImg = m_cyImg = 0;
            m_idxTopItem = 0;
            m_idxHot = m_idxFocus = m_idxMark = m_idxSel = m_idxToolTip = m_idxToolTipSubItemDisplay =
                m_idxEditing = m_idxEditingSubItemDisplay = m_idxCheckBoxLBtnDown = -1;
        }
        break;

        case WM_SETTINGCHANGE:
            UpdateSystemParameter();
            break;
        }
        return CWindow::OnMessage(hWnd, uMsg, wParam, lParam);
    }

    /// <summary>
    /// 重新构建树关系
    /// </summary>
    EckInline void BuildTree() noexcept
    {
        m_vItem.clear();
        if (m_vCol.empty())
            return;
        if (m_bFlatMode)
        {
            NMTLFILLALLFLATITEM nm{};
            FillNmhdrAndSendNotify(nm, NM_TL_FILLALLFLATITEM);
            if (m_bFlatListFilter)
            {
                m_vItem.reserve(nm.cItem);
                EckCounter(nm.cItem, i)
                {
                    if (!(nm.pItems[i]->uFlags & TLIF_INVISIBLE))
                        m_vItem.emplace_back(nm.pItems[i]);
                }
            }
            else
            {
                m_vItem.resize(nm.cItem);
                memcpy(m_vItem.data(), nm.pItems, nm.cItem * sizeof(*nm.pItems));
            }
        }
        else
        {
            NMTLFILLCHILDREN nm{};
            nm.bQueryRoot = TRUE;
            FillNmhdrAndSendNotify(nm, NM_TL_FILLCHILDREN);
            BOOL b = FALSE;
            EckCounter(nm.cChildren, i)
            {
                const int idxCurrParent = (int)m_vItem.size();
                NMTLFILLCHILDREN nm2{};
                nm2.pParent = nm.pChildren[i];
                nm2.pParent->idxParent = -1;
                if (!(nm.pChildren[nm.cChildren - i - 1]->uFlags & TLIF_INVISIBLE) &&
                    !b)
                {
                    nm.pChildren[nm.cChildren - i - 1]->uFlags |= TLIF_HASNOTSIBLING;// 确定最后一个同级项中的最后一个
                    b = TRUE;
                }

                FillNmhdrAndSendNotify(nm2, NM_TL_FILLCHILDREN);
                AddVirtualItem(nm2.pParent, nm2.pChildren, nm2.cChildren, 1);
            }
        }
        CheckOldData();
        UpdateScrollBar();
        ReCalculateTopItem();
    }

    int ItemFromY(int y) const noexcept
    {
        int idx;
        if (y > m_cyHeader)
            idx = m_idxTopItem + (y - m_cyHeader) / m_cyItem;
        else
            idx = m_idxTopItem + (y - m_cyHeader) / m_cyItem - 1;
        if (idx >= 0 && idx < (int)m_vItem.size())
            return idx;
        else
            return -1;
    }

    int HitTest(_Inout_ TLHITTEST& tlht) const noexcept
    {
        tlht.idxSubItemDisplay = -1;
        if (m_vCol.empty() || tlht.pt.x < 0 || tlht.pt.x > m_cxItem ||
            tlht.pt.y < m_cyHeader || tlht.pt.y > m_cyClient)
            return -1;
        const int idx = ItemFromY(tlht.pt.y);
        if (idx < 0)
            return -1;
        const auto it = std::find_if(m_vCol.begin(), m_vCol.end(), [&tlht](const COL& e)
            {
                return tlht.pt.x <= e.iRight && tlht.pt.x >= e.iLeft;
            });
        if (it != m_vCol.end())
            tlht.idxSubItemDisplay = (int)std::distance(m_vCol.begin(), it);

        if (!IsBitSet(tlht.uFlags, TLHTF_ONLYITEM))
        {
            tlht.iPart = TLIP_NONE;

            const int y = GetItemY(idx);
            const auto e = m_vItem[idx];
            RECT rc;
            if (!m_bFlatMode && (e->uFlags & TLIF_HASCHILDREN))
            {
                GetPartRect(idx, TLIP_EXPANDBTN, rc);
                if (rc.right > m_vCol.front().iRight)
                    rc.right = m_vCol.front().iRight;
                if (PtInRect(&rc, tlht.pt))
                {
                    tlht.iPart = TLIP_EXPANDBTN;
                    goto EndPartTest;
                }
            }

            if (m_bCheckBox)
            {
                GetPartRect(idx, TLIP_CHECKBOX, rc);
                if (rc.right > m_vCol.front().iRight)
                    rc.right = m_vCol.front().iRight;
                if (PtInRect(&rc, tlht.pt))
                {
                    tlht.iPart = TLIP_CHECKBOX;
                    goto EndPartTest;
                }
            }

            if (m_hImgList && e->idxImg >= 0)
            {
                GetPartRect(idx, TLIP_ICON, rc);
                if (rc.right > m_vCol.front().iRight)
                    rc.right = m_vCol.front().iRight;
                if (PtInRect(&rc, tlht.pt))
                {
                    tlht.iPart = TLIP_ICON;
                    goto EndPartTest;
                }
            }

            if (tlht.idxSubItemDisplay >= 0 && !(tlht.uFlags & TLHTF_NOHITTEXTLABEL))
            {
                GetItemTextRect(idx, tlht.idxSubItemDisplay, rc);
                if (IsRectEmpty(&rc))
                    tlht.uFlags |= TLHTF_NULLTEXT;
                else if (PtInRect(&rc, tlht.pt))
                {
                    tlht.iPart = TLIP_TEXT;
                    goto EndPartTest;
                }
            }
        EndPartTest:;
        }
        return idx;
    }

    EckInline int GetItemY(int idx) const noexcept
    {
        return m_cyHeader + (idx - m_idxTopItem) * m_cyItem;
    }

    void GetPartRect(int idx, int iPart, RECT& rc) const noexcept
    {
        EckAssert(idx >= 0 && idx < (int)m_vItem.size());
        rc.left = m_dxContent + CalculateExpandButtonLeftSpace(m_vItem[idx]);
        switch (iPart)
        {
        case TLIP_EXPANDBTN:
            EckAssert(m_vItem[idx]->uFlags & TLIF_HASCHILDREN);
            rc.left = m_dxContent + CalculateExpandButtonLeftSpace(m_vItem[idx]);
            rc.top = GetItemY(idx) + (m_cyItem - m_sizeTVGlyph.cy) / 2;
            rc.right = rc.left + m_sizeTVGlyph.cx;
            rc.bottom = rc.top + m_sizeTVGlyph.cy;
            break;
        case TLIP_CHECKBOX:
            EckAssert(m_bCheckBox);
            rc.left = m_dxContent + CalculateCheckBoxIndent(m_vItem[idx]) -
                m_sizeCheckBox.cx - m_Ds.cxCBPadding;
            rc.top = GetItemY(idx) + (m_cyItem - m_sizeCheckBox.cy) / 2;
            rc.right = rc.left + m_sizeCheckBox.cx;
            rc.bottom = rc.top + m_sizeCheckBox.cy;
            break;
            break;
        case TLIP_ICON:
            EckAssert(m_hImgList);
            rc.left = m_dxContent + CalculateTotalIndent(m_vItem[idx]) - m_cxImg;
            rc.top = GetItemY(idx) + (m_cyItem - m_cyImg) / 2;
            rc.right = rc.left + m_cxImg;
            rc.bottom = rc.top + m_cyImg;
            break;
        case TLIP_TEXT:
            EckDbgPrint(L"** Warning ** 请使用GetSubItemLabelRect获取文本矩形");
            break;
        default:
            EckDbgPrint(L"** Warning ** INVALID PART TYPE!");
            break;
        }
    }

    EckInline void GetItemRect(int idx, RECT& rc) const noexcept
    {
        EckAssert(idx >= 0 && idx < (int)m_vItem.size());
        rc =
        {
            m_dxContent,
            GetItemY(idx),
            m_cxItem + m_dxContent,
            GetItemY(idx) + m_cyItem
        };
    }

    /// <summary>
    /// 取子项目矩形
    /// </summary>
    /// <param name="idx"></param>
    /// <param name="idxSubItemDisplay"></param>
    /// <param name="rc"></param>
    /// <param name="bExcludeIndent"></param>
    /// <returns></returns>
    EckInline void GetSubItemRect(int idx, int idxSubItemDisplay, RECT& rc,
        BOOL bExcludeIndent = FALSE) const noexcept
    {
        EckAssert(idx >= 0 && idx < (int)m_vItem.size());
        EckAssert(idxSubItemDisplay >= 0 && idxSubItemDisplay < (int)m_vCol.size());
        rc.top = GetItemY(idx);
        rc.bottom = rc.top + m_cyItem;
        if (!idxSubItemDisplay && bExcludeIndent)
            rc.left = CalculateTotalIndent(m_vItem[idx]) + m_dxContent;
        else
            rc.left = m_vCol[idxSubItemDisplay].iLeft + m_dxContent;
        rc.right = m_vCol[idxSubItemDisplay].iRight + m_dxContent;
    }

    /// <summary>
    /// 取子项目文本可显示矩形
    /// </summary>
    /// <param name="idx"></param>
    /// <param name="idxSubItemDisplay"></param>
    /// <param name="rc"></param>
    /// <returns></returns>
    EckInline void GetSubItemLabelRect(int idx, int idxSubItemDisplay, RECT& rc) const noexcept
    {
        EckAssert(idx >= 0 && idx < (int)m_vItem.size());
        EckAssert(idxSubItemDisplay >= 0 && idxSubItemDisplay < (int)m_vCol.size());
        rc.top = GetItemY(idx);
        rc.bottom = rc.top + m_cyItem;
        if (!idxSubItemDisplay)
            rc.left = CalculateTotalIndent(m_vItem[idx]) + m_Ds.cxTextMargin;
        else
            rc.left = m_vCol[idxSubItemDisplay].iLeft + m_dxContent + m_Ds.cxTextMargin;
        rc.right = m_vCol[idxSubItemDisplay].iRight + m_dxContent - m_Ds.cxTextMargin;
    }

    EckInline int ColumnDisplayToActual(int idxDisplay) const noexcept
    {
        EckAssert(idxDisplay >= 0 && idxDisplay < (int)m_vCol.size());
        return m_vCol[idxDisplay].idxActual;
    }

    PCWSTR GetItemText(int idx, int idxSubItem, int* pcchText = nullptr) const noexcept
    {
        NMTLGETDISPINFO nm;
        nm.Item.pNode = m_vItem[idx];
        nm.Item.uMask = TLIM_TEXT;
        nm.Item.idxSubItem = idxSubItem;
        FillNmhdrAndSendNotify(nm, NM_TL_GETDISPINFO);
        if (pcchText)
            *pcchText = nm.Item.cchText;
        return nm.Item.pszText;
    }

    BOOL IsTextTooLong(int idx, int idxSubItemDisplay) const noexcept
    {
        int cchText;
        auto pszText = GetItemText(idx, ColumnDisplayToActual(idxSubItemDisplay), &cchText);
        SIZE size;
        GetTextExtentPoint32W(m_DC.GetDC(), pszText, cchText, &size);
        const int cxCol = m_vCol[idxSubItemDisplay].iRight - m_vCol[idxSubItemDisplay].iLeft;
        if (idxSubItemDisplay)
            return size.cx > cxCol - m_Ds.cxTextMargin * 2;
        else
            return size.cx > cxCol - CalculateTotalIndent(m_vItem[idx]) - m_Ds.cxTextMargin * 2;
    }

    void GetItemTextRect(int idx, int idxSubItemDisplay, RECT& rc) const noexcept
    {
        int cchText;
        auto pszText = GetItemText(idx, ColumnDisplayToActual(idxSubItemDisplay), &cchText);
        SIZE size;
        GetTextExtentPoint32W(m_DC.GetDC(), pszText, cchText, &size);
        rc.top = GetItemY(idx);

        if (idxSubItemDisplay)
            rc.left = m_vCol[idxSubItemDisplay].iLeft + m_Ds.cxTextMargin + m_dxContent;
        else
            rc.left = CalculateTotalIndent(m_vItem[idx]) + m_Ds.cxTextMargin + m_dxContent;
        rc.right = rc.left + std::min(size.cx, (long)m_vCol[idxSubItemDisplay].iRight);
        rc.top = rc.top + (m_cyItem - size.cy) / 2;
        rc.bottom = rc.top + size.cy;
    }

    BOOL RedrawItem(int idx) const noexcept
    {
        RECT rc;
        GetItemRect(idx, rc);
        return Redraw(rc);
    }

    BOOL RedrawItem(int idxBegin, int idxEnd) const noexcept
    {
        RECT rc;
        GetItemRect(idxBegin, rc);
        rc.bottom = GetItemY(idxEnd + 1);
        return Redraw(rc);
    }

    EckInlineNdCe CTLHeader& GetHeader() noexcept { return m_Header; }

    void ScrollV(int iDeltaLine) noexcept
    {
        if (!(GetStyle() & WS_VSCROLL))
            return;
        DismissEdit();
        DismissToolTip();
        SCROLLINFO si;
        si.cbSize = sizeof(si);
        si.fMask = SIF_POS;
        ScbGetInfomation(SB_VERT, &si);
        const int iOld = si.nPos;
        si.nPos += iDeltaLine;
        ScbSetInfomation(SB_VERT, &si);
        ScbGetInfomation(SB_VERT, &si);
        ReCalculateTopItem();
        if (m_bBackgroundNotSolid)
            Redraw();
        else
        {
            RECT rc{ 0,m_cyHeader,m_cxClient,m_cyClient };
            ScrollWindowEx(m_hWnd, 0, (iOld - si.nPos) * m_cyItem, &rc, &rc, nullptr, nullptr, SW_INVALIDATE);
        }
    }

    void ScrollH(int xDelta) noexcept
    {
        if (!(GetStyle() & WS_HSCROLL))
            return;
        DismissEdit();
        DismissToolTip();
        SCROLLINFO si;
        si.cbSize = sizeof(si);
        si.fMask = SIF_POS;
        ScbGetInfomation(SB_HORZ, &si);
        si.nPos += xDelta;
        ScbSetInfomation(SB_HORZ, &si);
        ScbGetInfomation(SB_HORZ, &si);
        m_dxContent = -si.nPos;
        m_Header.Left = m_dxContent;
        Redraw();
    }

    HIMAGELIST SetImageList(HIMAGELIST hImgList) noexcept
    {
        std::swap(m_hImgList, hImgList);
        if (m_hImgList)
            ImageList_GetIconSize(m_hImgList, &m_cxImg, &m_cyImg);
        else
            m_cxImg = m_cyImg = 0;
        return hImgList;
    }

    EckInline HIMAGELIST GetImageList() const noexcept { return m_hImgList; }

    EckInlineCe void SetDraggingSelectScrollGap(int ms) noexcept { m_msDraggingSelScrollGap = ms; }
    EckInlineNdCe int GetDraggingSelectScrollGap() const noexcept { return m_msDraggingSelScrollGap; }

    void SetSingleSelect(BOOL bSingleSel) noexcept
    {
        if (m_bSingleSel == bSingleSel)
            return;
        if (m_bSingleSel)
        {
            m_bSingleSel = FALSE;
            if (m_idxSel >= 0)
            {
                m_vItem[m_idxSel]->uFlags |= TLIF_SELECTED;// 转换到多选数据
                m_idxSel = -1;
            }
        }
        else
        {
            m_bSingleSel = TRUE;
            m_idxSel = -1;
            int i = 0;
            for (; i < (int)m_vItem.size(); ++i)
            {
                if (m_vItem[i]->uFlags & TLIF_SELECTED)// 查找第一个选中项
                {
                    m_idxSel = i;// 转换到单选数据
                    break;
                }
            }

            for (; i < (int)m_vItem.size(); ++i)
                m_vItem[i]->uFlags &= ~TLIF_SELECTED;
        }
        Redraw();
    }

    EckInlineNdCe BOOL GetSingleSelect() const noexcept { return m_bSingleSel; }

    EckInlineCe void SetFlatMode(BOOL b) noexcept { m_bFlatMode = b; }
    EckInlineNdCe BOOL GetFlatMode() const noexcept { return m_bFlatMode; }

    EckInline void SetWatermarkString(PCWSTR pszText) noexcept { m_rsWatermark = pszText; }
    EckInlineNdCe const CStringW& GetWatermarkString() const noexcept { return m_rsWatermark; }

    EckInlineCe void SetBackgroundNotSolid(BOOL b) noexcept { m_bBackgroundNotSolid = b; }
    EckInlineNdCe BOOL GetBackgroundNotSolid() const noexcept { return m_bBackgroundNotSolid; }

    EckInlineCe void SetHasLines(BOOL b) noexcept { m_bHasLines = b; }
    EckInlineNdCe BOOL GetHasLines() const noexcept { return m_bHasLines; }

    EckInlineCe void SetLineColor(COLORREF cr) noexcept { m_crBranchLine = cr; }
    EckInlineNdCe COLORREF GetLineColor() const noexcept { return m_crBranchLine; }

    EckInline TLNODE* IndexToNode(int idx) noexcept
    {
        EckAssert(idx >= 0 && idx < (int)m_vItem.size());
        return m_vItem[idx];
    }

    EckInlineCe void SetUseFilterInFlatMode(BOOL b) noexcept { m_bFlatListFilter = b; }
    EckInlineNdCe BOOL GetUseFilterInFlatMode() const noexcept { return m_bFlatListFilter; }

    EckInlineCe void SetDisallowBeginDragInItemSpace(BOOL b) noexcept { m_bDisallowBeginDragInItemSpace = b; }
    EckInlineNdCe BOOL GetDisallowBeginDragInItemSpace() const noexcept { return m_bDisallowBeginDragInItemSpace; }

    void EnsureVisible(int idx) noexcept
    {
        EckAssert(idx >= 0 && idx < (int)m_vItem.size());
        if (idx < m_idxTopItem)
            ScrollV(idx - m_idxTopItem);
        else if (idx >= m_idxTopItem + (m_cyClient - m_cyHeader) / m_cyItem)
            ScrollV(idx - m_idxTopItem - (m_cyClient - m_cyHeader) / m_cyItem + 1);
    }

    void GetSelectedItems(std::vector<int>& v) const noexcept
    {
        v.clear();
        if (m_bSingleSel && m_idxSel >= 0)
            v.emplace_back(m_idxSel);
        else
            for (int i = 0; i < (int)m_vItem.size(); ++i)
                if (m_vItem[i]->uFlags & TLIF_SELECTED)
                    v.emplace_back(i);
    }

#pragma region 项目状态
    /// <summary>
    /// 反转检查框
    /// </summary>
    void ToggleCheckItem(int idx) noexcept
    {
        EckAssert(idx >= 0 && idx < (int)m_vItem.size());
        if (m_b3StateCheckBox)
        {
            EckAssert(m_bCheckBox);
            const auto uNew = (GetLowNBits(m_vItem[idx]->uFlags, 2) + 1) % 3;
            m_vItem[idx]->uFlags &= ~(TLIF_CHECKED | TLIF_PARTIALCHECKED);
            m_vItem[idx]->uFlags |= uNew;
        }
        else if (m_bCheckBox)
        {
            m_vItem[idx]->uFlags &= ~TLIF_PARTIALCHECKED;
            m_vItem[idx]->uFlags ^= TLIF_CHECKED;
        }
    }

    /// <summary>
    /// 反转检查框
    /// 【检查disable】
    /// 【发送通知】
    /// 【重画】
    /// </summary>
    void ToggleCheckItemForClick(int idx) noexcept
    {
        if (!m_bCheckBox)
            return;
        EckAssert(idx >= 0 && idx < (int)m_vItem.size());
        if (!(m_vItem[idx]->uFlags & (TLIF_DISABLECHECKBOX | TLIF_DISABLED)))
        {
            NMTLCOMMITEM nm;
            nm.idx = idx;
            nm.idxSubItemDisplay = -1;
            nm.pNode = m_vItem[idx];
            if (!FillNmhdrAndSendNotify(nm, NM_TL_ITEMCHECKING))
            {
                ToggleCheckItem(idx);
                RedrawItem(idx);
                nm.nmhdr.code = NM_TL_ITEMCHECKED;
                SendNotify(nm);
            }
        }
    }

    /// <summary>
    /// 置现行选中项
    /// 【取消其他项的选中】
    /// 【重画】
    /// </summary>
    void SetCurrentSelection(int idx) noexcept
    {
        if (m_bSingleSel)
        {
            std::swap(m_idxSel, idx);
            if (m_idxSel >= 0)
                RedrawItem(m_idxSel);
            if (idx >= 0 && idx != m_idxSel)
                RedrawItem(idx);
        }
        else
        {
            int idx0, idx1;
            DeselectAll(idx0, idx1);
            m_vItem[idx]->uFlags |= TLIF_SELECTED;
            RedrawItem(idx0, idx1);
            if (idx < idx0 || idx > idx1)
                RedrawItem(idx);
        }
    }

    EckInlineNdCe int GetCurrentSelection() const noexcept { return m_idxSel; }

    /// <summary>
    /// 置焦点项
    /// 【重画】
    /// </summary>
    /// <param name="idx"></param>
    void SetFocusItem(int idx) noexcept
    {
        EckAssert(idx < (int)m_vItem.size());
        std::swap(m_idxFocus, idx);
        if (m_bFocusIndicatorVisible)
        {
            if (idx >= 0)
                RedrawItem(idx);
            if (m_idxFocus >= 0 && idx != m_idxFocus)
                RedrawItem(m_idxFocus);
        }
    }

    EckInlineNdCe int GetFocusItem() const noexcept { return m_idxFocus; }

    EckInlineCe void SetMarkItem(int idx) noexcept
    {
        EckAssert(idx < (int)m_vItem.size());
        m_idxMark = idx;
    }
    EckInlineNdCe int GetMarkItem() const noexcept { return m_idxMark; }

    void DeselectAll(
        _Out_ int& idxChangeBegin,
        _Out_ int& idxChangeEnd) noexcept
    {
        m_idxSel = -1;
        int idx0 = -1, idx1 = -1;
        EckCounter(m_vItem.size(), i)
        {
            const auto e = m_vItem[i];
            if (e->uFlags & TLIF_SELECTED)
            {
                if (idx0 < 0)
                    idx0 = (int)i;
                idx1 = (int)i;
                m_vItem[i]->uFlags &= ~TLIF_SELECTED;
            }
        }
        idxChangeBegin = idx0;
        idxChangeEnd = idx1;
    }

    void DeselectRange(int idxBegin, int idxEnd) noexcept
    {
        if (m_bSingleSel && m_idxSel >= idxBegin && m_idxSel <= idxEnd)
        {
            m_idxSel = -1;
            return;
        }

        for (int i = idxBegin; i <= idxEnd; ++i)
            m_vItem[i]->uFlags |= TLIF_SELECTED;
    }

    int DeselectChildren(int idxParent) noexcept
    {
        EckAssert(idxParent >= 0 && idxParent < (int)m_vItem.size());

        const int iParentLevel = m_vItem[idxParent]->iLevel;
        int i = idxParent + 1;
        for (; i < (int)m_vItem.size(); ++i)
        {
            const auto e = m_vItem[i];
            if (e->iLevel <= iParentLevel)
                break;
            e->uFlags &= ~TLIF_SELECTED;
        }
        return std::max(0, i - idxParent - 1);
    }

    /// <summary>
    /// 选择范围
    /// 【检查disable】
    /// 【单选时重画】
    /// </summary>
    void SelectRangeForClick(int idxBegin, int idxEnd,
        _Out_ int& idxChangeBegin, _Out_ int& idxChangeEnd) noexcept
    {
        if (m_bSingleSel)
        {
            if (idxBegin >= 0 && (m_vItem[idxBegin]->uFlags & TLIF_DISABLED))
                idxBegin = -1;
            std::swap(m_idxSel, idxBegin);
            if (m_idxSel >= 0)
                RedrawItem(m_idxSel);
            if (idxBegin >= 0 && idxBegin != m_idxSel)
                RedrawItem(idxBegin);
            idxChangeBegin = idxChangeEnd = -1;
            return;
        }
        int i;
        int idx0 = -1, idx1 = -1;
        // 清除前面选中
        for (i = 0; i < idxBegin; ++i)
        {
            if (m_vItem[i]->uFlags & TLIF_SELECTED)
            {
                if (idx0 < 0)
                    idx0 = i;
                idx1 = i;
                m_vItem[i]->uFlags &= ~TLIF_SELECTED;
            }
        }
        // 范围选中
        for (i = idxBegin; i <= idxEnd; ++i)
        {
            if (!(m_vItem[i]->uFlags & (TLIF_DISABLED | TLIF_SELECTED)))
            {
                if (idx0 < 0)
                    idx0 = i;
                idx1 = i;
                m_vItem[i]->uFlags |= TLIF_SELECTED;
            }
        }
        // 清除后面选中
        for (i = idxEnd + 1; i < (int)m_vItem.size(); ++i)
        {
            if (m_vItem[i]->uFlags & TLIF_SELECTED)
            {
                if (idx0 < 0)
                    idx0 = i;
                idx1 = i;
                m_vItem[i]->uFlags &= ~TLIF_SELECTED;
            }
        }
        idxChangeBegin = idx0;
        idxChangeEnd = idx1;
    }

    /// <summary>
    /// 选中项目。
    /// 多选时不会清除其他项目的选中
    /// 【检查disable】
    /// 【修改焦点和mark】
    /// 【重画】
    /// </summary>
    /// <param name="idx"></param>
    /// <param name="bSel"></param>
    void SelectItemForClick(int idx, BOOL bSel) noexcept
    {
        EckAssert(idx >= 0 && idx < (int)m_vItem.size());

        if (m_bSingleSel)
        {
            const int idxOldSel = m_idxSel;
            const int idxOldFocus = m_idxFocus;
            if (idx >= 0)
            {
                m_idxFocus = idx;
                m_idxMark = idx;
            }

            if (idx >= 0 ? (!(m_vItem[idx]->uFlags & TLIF_DISABLED)) : TRUE)
            {
                m_idxSel = idx;
                if (idx >= 0)
                    RedrawItem(idx);
            }
            if (idxOldSel >= 0 && idxOldSel != m_idxSel)
                RedrawItem(idxOldSel);
            if (m_bFocusIndicatorVisible && idxOldFocus >= 0 && idxOldFocus != idx)
                RedrawItem(idxOldFocus);
        }
        else
        {
            if (idx >= 0)
            {
                const auto e = m_vItem[idx];
                if (!(e->uFlags & TLIF_DISABLED))
                    if (bSel)
                        e->uFlags |= TLIF_SELECTED;
                    else
                        e->uFlags &= ~TLIF_SELECTED;
                const auto idxOldFocus = m_idxFocus;
                m_idxFocus = idx;
                m_idxMark = idx;
                RedrawItem(idx);
                if (m_bFocusIndicatorVisible && idxOldFocus >= 0 && idxOldFocus != idx)
                    RedrawItem(idxOldFocus);
            }
            else
            {
                int idx0, idx1;
                DeselectAll(idx0, idx1);
                RedrawItem(idx0, idx1);
            }
        }
    }

    /// <summary>
    /// 反转选中状态
    /// 【检查disable】
    /// 【修改焦点和mark】
    /// 【重画】
    /// </summary>
    /// <param name="idx"></param>
    /// <returns></returns>
    void ToggleSelectItemForClick(int idx) noexcept
    {
        if (m_bSingleSel)
        {
            if (m_idxSel == idx)
                SelectItemForClick(-1, 0);
            else
                SelectItemForClick(idx, 0);
        }
        else
            SelectItemForClick(idx, !(m_vItem[idx]->uFlags & TLIF_SELECTED));
    }

    /// <summary>
    /// 折叠项目
    /// 【发送通知】
    /// </summary>
    /// <param name="idx">索引</param>
    /// <param name="iOp">操作，TLEIO_常量</param>
    void ExpandItem(int idx, int iOp, BOOL bRedraw = TRUE, BOOL bRebuildTree = TRUE) noexcept
    {
        EckAssert(idx >= 0 && idx < (int)m_vItem.size());

        const auto e = m_vItem[idx];
        switch (iOp)
        {
        case TLEIO_COLLAPSE:
            if (!(e->uFlags & TLIF_CLOSED))
                ExpandItem(idx, TLEIO_TOGGLE, bRebuildTree);
            return;
        case TLEIO_EXPAND:
            if (e->uFlags & TLIF_CLOSED)
                ExpandItem(idx, TLEIO_TOGGLE, bRebuildTree);
            return;
        case TLEIO_TOGGLE:
            break;
        default:
            EckDbgBreak();
            return;
        }

        NMTLCOMMITEM nm;
        nm.idx = idx;
        nm.idxSubItemDisplay = -1;
        nm.pNode = e;
        if (!FillNmhdrAndSendNotify(nm, NM_TL_ITEMEXPANDING))
        {
            e->uFlags ^= TLIF_CLOSED;
            if (e->uFlags & TLIF_CLOSED)// 若项目折叠，则清除所有子项的选中
            {
                const int cChildren = DeselectChildren(idx);
                CheckOldDataRange(idx + 1, idx + cChildren, idx);// 限位
            }
            nm.nmhdr.code = NM_TL_ITEMEXPANDED;
            SendNotify(nm);

            if (bRebuildTree)
            {
                const int idxTopOld = m_idxTopItem;
                BuildTree();
                if (bRedraw)
                {
                    if (idxTopOld != m_idxTopItem)
                        Redraw();
                    else
                    {
                        RECT rc{ 0,GetItemY(idx),m_cxClient,m_cyClient };
                        Redraw(rc);
                    }
                }
            }
        }
    }
private:
    void ExpandItemRecurse_Collapse(NMTLFILLCHILDREN& nmfc) noexcept
    {
        NMTLFILLCHILDREN nm{ nmfc.nmhdr };

        EckCounter(nmfc.cChildren, i)
        {
            const auto e = nmfc.pChildren[i];
            if (e->uFlags & TLIF_HASCHILDREN)
            {
                e->uFlags |= TLIF_CLOSED;
                const int cChildren = DeselectChildren(i);
                CheckOldDataRange(i + 1, i + cChildren, i);// 限位

                nm.pParent = e;
                SendNotify(nm);
                ExpandItemRecurse_Collapse(nm);
            }
        }
    }

    void ExpandItemRecurse_Expand(NMTLFILLCHILDREN& nmfc) noexcept
    {
        NMTLFILLCHILDREN nm{ nmfc.nmhdr };

        EckCounter(nmfc.cChildren, i)
        {
            const auto e = nmfc.pChildren[i];
            if (e->uFlags & TLIF_HASCHILDREN)
            {
                e->uFlags &= ~TLIF_CLOSED;
                nm.pParent = e;
                SendNotify(nm);
                ExpandItemRecurse_Expand(nm);
            }
        }
    }

    void ExpandItemRecurse_Toggle(NMTLFILLCHILDREN& nmfc) noexcept
    {
        NMTLFILLCHILDREN nm{ nmfc.nmhdr };

        EckCounter(nmfc.cChildren, i)
        {
            const auto e = nmfc.pChildren[i];
            if (e->uFlags & TLIF_HASCHILDREN)
            {
                e->uFlags ^= TLIF_CLOSED;
                if (e->uFlags & TLIF_CLOSED)
                {
                    const int cChildren = DeselectChildren(i);
                    CheckOldDataRange(i + 1, i + cChildren, i);// 限位
                }

                nm.pParent = e;
                SendNotify(nm);
                ExpandItemRecurse_Toggle(nm);
            }
        }
    }
public:
    /// <summary>
    /// 折叠项目及其直接和间接子项目
    /// 【发送通知】
    /// </summary>
    /// <param name="idx"></param>
    /// <param name="iOp"></param>
    /// <param name="bRebuildTree"></param>
    void ExpandItemRecurse(int idx, int iOp,
        BOOL bRedraw = TRUE, BOOL bRebuildTree = TRUE) noexcept
    {
        EckAssert(idx >= 0 && idx < (int)m_vItem.size());

        const auto e = m_vItem[idx];
        if (!(e->uFlags & TLIF_HASCHILDREN))
            return;

        NMTLCOMMITEM nm;
        nm.idx = idx;
        nm.idxSubItemDisplay = -iOp - 1;
        nm.pNode = e;
        if (!FillNmhdrAndSendNotify(nm, NM_TL_ITEMEXPANDING))
        {
            int i = idx + 1;
            NMTLFILLCHILDREN nmfc;
            nmfc.nmhdr = nm.nmhdr;
            nmfc.nmhdr.code = NM_TL_FILLCHILDREN;
            nmfc.bQueryRoot = FALSE;
            nmfc.cChildren = 0;
            nmfc.pChildren = nullptr;
            nmfc.pParent = e;
            switch (iOp)
            {
            case TLEIO_COLLAPSE:
                e->uFlags |= TLIF_CLOSED;
                SendNotify(nmfc);
                ExpandItemRecurse_Collapse(nmfc);
                break;
            case TLEIO_EXPAND:
                e->uFlags &= ~TLIF_CLOSED;
                SendNotify(nmfc);
                ExpandItemRecurse_Expand(nmfc);
                break;
            case TLEIO_TOGGLE:
                e->uFlags ^= TLIF_CLOSED;
                SendNotify(nmfc);
                ExpandItemRecurse_Toggle(nmfc);
                break;
            default:
                EckDbgBreak();
                return;
            }

            if (bRebuildTree)
            {
                const int idxTopOld = m_idxTopItem;
                BuildTree();
                if (bRedraw)
                {
                    if (idxTopOld != m_idxTopItem)
                        Redraw();
                    else
                    {
                        RECT rc{ 0,GetItemY(idx),m_cxClient,m_cyClient };
                        Redraw(rc);
                    }
                }
            }
        }
    }
#pragma endregion 项目状态

    void SetItemHeight(int cy) noexcept
    {
        std::swap(m_cyItem, cy);
        if (m_cyItem != cy)
            Redraw();
    }
    EckInlineNdCe int GetItemHeight() const noexcept { return m_cyItem; }

    void SetHeaderHeight(int cy) noexcept
    {
        std::swap(m_cyHeader, cy);
        if (m_cyHeader != cy)
        {
            m_Header.Height = cy;
            Redraw();
        }
    }
    EckInlineNdCe int GetHeaderHeight() const noexcept { return m_cyHeader; }

    EckInlineNdCe int GetFirstVisibleItem() const noexcept { return m_idxTopItem; }

    EckInlineNdCe int GetLastFullVisibleItem() const noexcept
    {
        return m_idxTopItem + (m_cyClient - m_cyHeader) / m_cyItem;
    }

    EckInlineNdCe int GetLastPartialVisibleItem() const noexcept
    {
        const int cView = (m_cyClient - m_cyHeader) / m_cyItem;
        if (cView * m_cyItem + m_cyHeader == m_cyClient)
            return m_idxTopItem + cView - 1;
        else
            return m_idxTopItem + cView;
    }

    EckInlineNdCe CToolTip& GetToolTip() noexcept { return m_ToolTip; }

    EckInlineCe void SetDisableAutoToolTip(BOOL b) noexcept { m_bDisableAutoToolTip = b; }
    EckInlineNdCe BOOL GetDisableAutoToolTip() const noexcept { return m_bDisableAutoToolTip; }

    /// <summary>
    /// 置检查框
    /// </summary>
    /// <param name="i">0 - 无，1 - 复选，2 - 三态</param>
    EckInlineCe void SetHasCheckBox(int i) noexcept
    {
        if (i == 0)
        {
            m_bCheckBox = FALSE;
            m_b3StateCheckBox = FALSE;
        }
        else if (i == 1)
        {
            m_bCheckBox = TRUE;
            m_b3StateCheckBox = FALSE;
        }
        else if (i == 2)
        {
            m_bCheckBox = TRUE;
            m_b3StateCheckBox = TRUE;
        }
    }

    /// <summary>
    /// 取检查框
    /// </summary>
    /// <returns>0 - 无，1 - 复选，2 - 三态</returns>
    EckInlineNdCe int GetHasCheckBox() const noexcept
    {
        if (m_bCheckBox)
            return m_b3StateCheckBox ? 2 : 1;
        else
            return 0;
    }

    EckInlineCe void SetDisableHScrollWithShift(BOOL b) noexcept { m_bDisableHScrollWithShift = b; }
    EckInlineNdCe BOOL GetDisableHScrollWithShift() const noexcept { return m_bDisableHScrollWithShift; }

    void EnterEdit(int idx, int idxSubItemDisplay) noexcept
    {
        if (!m_bEditLabel)
            return;
        EckAssert(idx >= 0 && idx < (int)m_vItem.size());
        DismissEdit();
        EnsureVisible(idx);
        NMTLEDIT nm;
        nm.idx = idx;
        nm.idxSubItemDisplay = idxSubItemDisplay;
        nm.idxSubItem = ColumnDisplayToActual(idxSubItemDisplay);
        nm.uFlags = TLEDF_BUILDINEDIT;
        GetSubItemRect(idx, idxSubItemDisplay, nm.rc, TRUE);
        FillNmhdrAndSendNotify(nm, NM_TL_PREEDIT);
        if (nm.uFlags & TLEDF_DONTEDIT)
            return;
        m_idxEditing = idx;
        m_idxEditingSubItemDisplay = idxSubItemDisplay;
        if (nm.uFlags & TLEDF_BUILDINEDIT)
        {
            m_Edit.Create(GetItemText(idx, ColumnDisplayToActual(idxSubItemDisplay)),
                WS_CHILD | ES_AUTOHSCROLL, 0,
                nm.rc.left, nm.rc.top, nm.rc.right - nm.rc.left, nm.rc.bottom - nm.rc.top, m_hWnd,
                IDC_EDIT);
            m_Edit.SetFrameType(5);
            m_Edit.SetFont(m_hFont);
            m_Edit.SelectAll();
            m_Edit.Show(SW_SHOW);
            m_bBuildInEditChanged = FALSE;
            SetFocus(m_Edit.HWnd);
        }
    }

    void DismissEdit(BOOL bNotifySave = FALSE) noexcept
    {
        if (!m_bEditLabel)
            return;
        if (m_bWaitEditDelay)
        {
            CancelEditDelay();
            return;
        }
        if (m_idxEditing < 0)
            return;
        NMTLEDIT nm;
        nm.idx = m_idxEditing;
        nm.idxSubItemDisplay = m_idxEditingSubItemDisplay;
        nm.idxSubItem = ColumnDisplayToActual(m_idxEditingSubItemDisplay);
        if (m_Edit.HWnd)
            nm.uFlags = TLEDF_BUILDINEDIT |
            ((bNotifySave && m_bBuildInEditChanged) ? TLEDF_SHOULDSAVETEXT : 0);
        else
            nm.uFlags = (bNotifySave ? TLEDF_SHOULDSAVETEXT : 0);

        FillNmhdrAndSendNotify(nm, NM_TL_POSTEDIT);
        m_idxEditing = m_idxEditingSubItemDisplay = -1;
        if (nm.uFlags & TLEDF_BUILDINEDIT)
            m_Edit.Destroy();
    }

    EckInlineNdCe CTLEditExt& GetEdit() noexcept { return m_Edit; }

    EckInlineNdCe BOOL IsEditing() const noexcept { return m_idxEditing >= 0 && !m_bWaitEditDelay; }

    EckInlineCe void SetDisableSelectAllWithCtrlA(BOOL b) noexcept { m_bDisableSelectAllWithCtrlA = b; }
    EckInlineNdCe BOOL GetDisableSelectAllWithCtrlA() const noexcept { return m_bDisableSelectAllWithCtrlA; }

    void SetEditLabel(BOOL b) noexcept
    {
        if (m_bWaitEditDelay || m_idxEditing >= 0)
        {
            EckDbgBreak();// 正在编辑时不允许启用标签编辑
            return;
        }
        m_bEditLabel = b;
    }
    EckInlineNdCe BOOL GetEditLabel() const noexcept { return m_bEditLabel; }

    /// <summary>
    /// 计算展开按钮*及其*前置空白造成的缩进宽度
    /// </summary>
    EckInlineNdCe int CalculateExpandButtonIndent(const TLNODE* e) const noexcept
    {
        return (m_bFlatMode ? 0 : (e->iLevel * m_sizeTVGlyph.cx));
    }

    /// <summary>
    /// 计算展开按钮*的*前置空白造成的缩进宽度
    /// </summary>
    EckInlineNdCe int CalculateExpandButtonLeftSpace(const TLNODE* e) const noexcept
    {
        return (m_bFlatMode ? 0 : ((e->iLevel - 1) * m_sizeTVGlyph.cx));
    }

    /// <summary>
    /// 计算展开按钮、复选框及其前置空白造成的缩进宽度
    /// </summary>
    EckInlineNdCe int CalculateCheckBoxIndent(const TLNODE* e) const noexcept
    {
        return CalculateExpandButtonIndent(e) +
            (m_bCheckBox ? (m_sizeCheckBox.cx + m_Ds.cxCBPadding * 2) : 0);
    }

    /// <summary>
    /// 计算展开按钮、复选框、图标及其前置空白造成的缩进宽度
    /// </summary>
    EckInlineNdCe int CalculateTotalIndent(const TLNODE* e) const noexcept
    {
        return CalculateCheckBoxIndent(e) + m_cxImg;
    }

    void AdjustColumnToFit(int idxSubItemDisplay) noexcept
    {
        EckAssert(idxSubItemDisplay >= 0 && idxSubItemDisplay < (int)m_vCol.size());
        if (m_vItem.empty())
            return;
        const int idxCol = ColumnDisplayToActual(idxSubItemDisplay);
        HDITEMW hdi;
        hdi.mask = HDI_WIDTH;
        hdi.cxy = 0;

        if (idxSubItemDisplay)
        {
            EckCounter(m_vItem.size(), i)
            {
                int cchText;
                const auto pszText = GetItemText((int)i, idxCol, &cchText);
                SIZE size;
                GetTextExtentPoint32W(m_DC.GetDC(), pszText, cchText, &size);
                if (size.cx > hdi.cxy)
                    hdi.cxy = size.cx;
            }
            hdi.cxy += (m_Ds.cxTextMargin * 2);
        }
        else
        {
            TLNODE* MaxLevelNode = m_vItem.front();
            EckCounter(m_vItem.size(), i)
            {
                int cchText;
                const auto pszText = GetItemText((int)i, idxCol, &cchText);
                SIZE size;
                GetTextExtentPoint32W(m_DC.GetDC(), pszText, cchText, &size);
                if (size.cx > hdi.cxy)
                    hdi.cxy = size.cx;
                if (m_vItem[i]->iLevel > MaxLevelNode->iLevel)
                    MaxLevelNode = m_vItem[i];
            }
            hdi.cxy += (CalculateTotalIndent(MaxLevelNode) + m_Ds.cxTextMargin * 2);
        }

        m_Header.SetItem(idxCol, &hdi);
        Redraw();
    }

    int GetColumnCount() const { return (int)m_vCol.size(); }

    EckInlineCe void SetTextForegroundColor(COLORREF cr) noexcept { m_crText = cr; }
    EckInlineCe void SetBackgroundColor(COLORREF cr) noexcept { m_crBkg = cr; }

    EckInline void SetTextBufferSize(int cch) noexcept { m_rsTextBuf.ReSize(cch); }
    EckInlineNdCe int GetTextBufferSize() const noexcept { return m_rsTextBuf.Size(); }
};

inline LRESULT CTLHeader::OnMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept
{
    LRESULT lResult;
    switch (uMsg)
    {
    case HDM_INSERTITEMW:
        m_TL.DismissEdit();
        lResult = __super::OnMessage(hWnd, uMsg, wParam, lParam);
        if (lResult >= 0)
        {
            m_TL.m_vCol.emplace_back();
            m_TL.UpdateColumnInfomation();
        }
        return lResult;

    case HDM_DELETEITEM:
        m_TL.DismissEdit();
        lResult = __super::OnMessage(hWnd, uMsg, wParam, lParam);
        if (lResult)
        {
            m_TL.m_vCol.pop_back();
            m_TL.UpdateColumnInfomation();
        }
        return lResult;

    case HDM_SETORDERARRAY:
        m_TL.DismissEdit();
        lResult = __super::OnMessage(hWnd, uMsg, wParam, lParam);
        if (lResult)
            m_TL.UpdateColumnInfomation();
        return lResult;
    }
    return __super::OnMessage(hWnd, uMsg, wParam, lParam);
}

inline LRESULT CTLEditExt::OnMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept
{
    switch (uMsg)
    {
    case WM_KEYDOWN:
    {
        switch (wParam)
        {
        case VK_RETURN:
            m_TL.DismissEdit(TRUE);
            break;
        case VK_ESCAPE:
            m_TL.DismissEdit();
            break;
        }
    }
    break;

    case WM_KILLFOCUS:
        m_TL.DismissEdit(TRUE);
        break;
    }

    return __super::OnMessage(hWnd, uMsg, wParam, lParam);
}
ECK_NAMESPACE_END