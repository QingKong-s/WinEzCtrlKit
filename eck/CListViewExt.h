/*
* WinEzCtrlKit Library
*
* CListViewExt.h ： 列表视图扩展
*
* Copyright(C) 2024 QingKong
*/
#pragma once

#include "CListView.h"
#include "CHeaderExt.h"
#include "CtrlGraphics.h"
#include "CSrwLock.h"
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
constexpr inline DWORD LveItemDataMagic{ 'LEID' };
#endif

// 非所有者数据模式下项目lParam指向的结构
struct LVE_ITEM_DATA
{
#ifdef _DEBUG
	DWORD dwMagic{ LveItemDataMagic };
#endif
	LVE_CELL_COLOR LineClr{};
	std::map<int, LVE_CELL_COLOR> CellClr{};
	LPARAM lParam{};
};

enum :UINT
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
};

/*
CListViewExt封装了ListView的常用扩展功能
===以下ListView的原始功能被忽略===
项目lParam - 由控件内部占用，不能由外部设置
所有者绘制 - 不应使用
边框选择 - 暂不支持

===以下功能效果仍相同但处理机制已变化===
整体的颜色设置 - 应使用Lve系列方法管理，背景颜色仍使用SetBkClr
表格线 - LVS_EX_GRIDLINES永远不会达到ListView，而是由内部接管；增加颜色配置
所有者数据 - 新增一个回调
*/
class CListViewExt :public CListView
{
public:
	using FOwnerData = LRESULT(*)(LveOd, void*, void*);
	ECK_RTTI(CListViewExt);
private:
	struct SUBITEM
	{
		CRefStrW rsText;
		LVE_CELL_COLOR Clr;
	};

	struct ITEM
	{
		CRefStrW rsText;
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
	BITBOOL m_bOwnerData : 1 = FALSE;	// 所有者数据
	BITBOOL m_bSubItemImg : 1 = FALSE;	// 显示子项图像
	BITBOOL m_bFullRowSel : 1 = FALSE;	// 整行选择
	BITBOOL m_bShowSelAlways : 1 = FALSE;	// 始终显示选择
	BITBOOL m_bBorderSelect : 1 = FALSE;	// 边框选择
	BITBOOL m_bCheckBoxes : 1 = FALSE;	// 显示复选框
	BITBOOL m_bGridLines : 1 = FALSE;	// 显示表格线
	BITBOOL m_bHideLabels : 1 = FALSE;	// 隐藏标签
	BITBOOL m_bSingleSel : 1 = FALSE;	// 单选模式
	BITBOOL m_bHasFocus : 1 = FALSE;	// 是否有焦点

	// 图形
	HTHEME m_hTheme{};		// 主题句柄
	int m_iDpi{ USER_DEFAULT_SCREEN_DPI };	// DPI
	CEzCDC m_DcAlpha{};		// DC，用作暗色下的颜色Alpha混合

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

	BYTE m_byColorAlpha{ 70 };	// 透明度
	int m_cyHeader{};			// 表头高度，0 = 默认
	size_t m_cchOdTextBuf{ 0 };	// 所有者数据缓冲区大小
	// 项目
	std::vector<LVE_ITEM_DATA*> m_vRecycleData{};	// [NOD]回收数据
	FOwnerData m_pfnOwnerData{};	// [OD]所有者数据回调函数
	void* m_pOdProcData{};			// [OD]所有者数据回调函数参数
	// 
	const ECKTHREADCTX* m_pThrCtx{};// 线程上下文

	ECK_DS_BEGIN(DPIS)
		ECK_DS_ENTRY_DYN(cxEdge, GetSystemMetrics(SM_CXEDGE))
		;
	ECK_DS_END_VAR(m_Ds);

	void GetColumnMetrics(int* px, int cCol, int dx) const
	{
		RECT rc;
		for (int i = 0; i < cCol; ++i)
		{
			m_Header.GetItemRect(i, &rc);
			px[i * 2] = dx + rc.left;
			px[i * 2 + 1] = dx + rc.right;
		}
	}

	BOOL AlphaBlendColor(HDC hDC, const RECT& rcItem, COLORREF cr)
	{
		constexpr RECT rcDst{ 0,0,1,1 };
		SetDCBrushColor(m_DcAlpha.GetDC(), cr);
		FillRect(m_DcAlpha.GetDC(), &rcDst, GetStockBrush(DC_BRUSH));
		return AlphaBlend(hDC, rcItem.left, rcItem.top, rcItem.right - rcItem.left,
			rcItem.bottom - rcItem.top,
			m_DcAlpha.GetDC(), 0, 0, 1, 1, { AC_SRC_OVER,0,m_byColorAlpha,0 });
	}

	void BitBltColor(HDC hDC, const RECT& rcItem, COLORREF cr, int* pColMetrics, int cCol)
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

	LRESULT OnItemPrePaint(NMLVCUSTOMDRAW* pnmlvcd)
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
				crText = m_pThrCtx->crDefText;

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
					rc0.top += m_Ds.cxEdge;
				break;
			case LV_VIEW_SMALLICON:
				if (!m_bHideLabels)
					rc0.left += m_Ds.cxEdge;
				break;
			case LV_VIEW_LIST:
				rc0.left += m_Ds.cxEdge;
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
						rc.left -= (sizeILState.cx + m_Ds.cxEdge);
						rc0 = { 0,0,sizeILState.cx,sizeILState.cy };
						CenterRect(rc0, rc);
					}
					else
					{
						rc0.left -= (sizeILState.cx + m_Ds.cxEdge);
						rc0.top = rc.top + (rc.bottom - rc.top - sizeILState.cy) / 2;
					}
					ImageList_Draw(hILState, idxState, hDC,
						rc0.left, rc0.top,
						ILD_NORMAL | ILD_TRANSPARENT);
				}
			}
		}
		// 画文本
#pragma warning(push)
#pragma warning(disable: 6255)// 改用_malloca
		PWSTR pTempBuf{};
		if (bExtInfo)
		{
			ie.uMask = LVE_IM_TEXT | LVE_IM_IMAGE;
			ie.idxItem = idx;
			ie.idxSubItem = 0;
			if (m_cchOdTextBuf)
			{
				pTempBuf = (PWSTR)_alloca(m_cchOdTextBuf * sizeof(WCHAR));
				ie.pszText = pTempBuf;
				ie.cchText = (int)m_cchOdTextBuf;
			}
			else
			{
				ie.pszText = nullptr;
				ie.cchText = 0;
			}
		}
		else
		{
			li.mask = LVIF_TEXT | LVIF_IMAGE;
			pTempBuf = (PWSTR)_alloca(MAX_PATH * sizeof(WCHAR));
			li.pszText = pTempBuf;
			li.cchTextMax = MAX_PATH;
			li.iItem = idx;
		}
#pragma warning(pop)

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
					if (m_cchOdTextBuf)
					{
						ie.pszText = pTempBuf;
						ie.cchText = (int)m_cchOdTextBuf;
					}
					else
					{
						ie.pszText = nullptr;
						ie.cchText = 0;
					}
					m_pfnOwnerData(LveOd::GetDispInfo, &ie, m_pOdProcData);
					li.pszText = (PWSTR)ie.pszText;
					li.iImage = ie.idxImage;
					cchText = ie.cchText;
				}
				else
					GetItem(&li);
				// 如果可能，绘制后续子项图像
				if (li.iImage >= 0 && hIL)
					if (li.iSubItem == 0)
						cxImg = m_Ds.cxEdge;
					else if (m_bSubItemImg)
					{
						RECT rcImg;
						GetSubItemRect(idx, li.iSubItem, &rcImg, LVIR_ICON);
						cxImg = rcImg.right - rcImg.left + m_Ds.cxEdge * 3;
						ImageList_Draw(hIL, li.iImage, hDC,
							rcImg.left,
							(pnmlvcd->nmcd.rc.bottom - pnmlvcd->nmcd.rc.top -
								(m_sizeIL[idxIL].cy)) / 2 + pnmlvcd->nmcd.rc.top,
							ILD_NORMAL | ILD_TRANSPARENT);
					}
					else
						cxImg = m_Ds.cxEdge * 2;
				else
					cxImg = m_Ds.cxEdge * 2;

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
			GetTileInfo(&lvti);
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
					if (m_cchOdTextBuf)
					{
						ie.pszText = pTempBuf;
						ie.cchText = (int)m_cchOdTextBuf;
					}
					else
					{
						ie.pszText = nullptr;
						ie.cchText = 0;
					}
					m_pfnOwnerData(LveOd::GetDispInfo, &ie, m_pOdProcData);
					li.pszText = (PWSTR)ie.pszText;
					li.iImage = ie.idxImage;
					cchText = ie.cchText;
				}
				else
				{
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
					SetTextColor(hDC, m_pThrCtx->crGray1);
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

	void UpdateLvExOptions(DWORD dwLvExStyle)
	{
		m_bSubItemImg = IsBitSet(dwLvExStyle, LVS_EX_SUBITEMIMAGES);
		m_bFullRowSel = IsBitSet(dwLvExStyle, LVS_EX_FULLROWSELECT);
		m_bBorderSelect = IsBitSet(dwLvExStyle, LVS_EX_BORDERSELECT);
		m_bCheckBoxes = IsBitSet(dwLvExStyle, LVS_EX_CHECKBOXES);
		//m_bGridLines = IsBitSet(dwLvExStyle, LVS_EX_GRIDLINES);// Special handled
		m_bHideLabels = IsBitSet(dwLvExStyle, LVS_EX_HIDELABELS);
	}

	void UpdateStyleOptions(DWORD dwStyle)
	{
		m_bShowSelAlways = IsBitSet(dwStyle, LVS_SHOWSELALWAYS);
		m_bOwnerData = IsBitSet(dwStyle, LVS_OWNERDATA);
		m_bSingleSel = IsBitSet(dwStyle, LVS_SINGLESEL);
	}

	void HandleThemeChange()
	{
		if (m_bAutoDarkMode)
		{
			SetBkClr(m_pThrCtx->crDefBkg);
			if (m_bAutoColorPack)
				if (ShouldAppsUseDarkMode())
					LveSetClrPack(LveDarkClr1);
				else
					LveSetClrPack(LveLightClr1);
		}
	}
public:
	ECKPROP(LveGetTextClr, LveSetTextClr)					COLORREF TextColor;
	ECKPROP(LveGetOddLineTextClr, LveSetOddLineTextClr)		COLORREF OddLineTextColor;
	ECKPROP(LveGetEvenLineTextClr, LveSetEvenLineTextClr)	COLORREF EvenLineTextColor;
	ECKPROP(LveGetOddLineBkClr, LveSetOddLineBkClr)			COLORREF OddLineBkColor;
	ECKPROP(LveGetEvenLineBkClr, LveSetEvenLineBkClr)		COLORREF EvenLineBkColor;
	ECKPROP(LveGetGridLineHClr, LveSetGridLineHClr)			COLORREF GridLineHColor;
	ECKPROP(LveGetGridLineVClr, LveSetGridLineVClr)			COLORREF GridLineVColor;
	ECKPROP(LveGetHeaderTextClr, LveSetHeaderTextClr)		COLORREF HeaderTextColor;
	ECKPROP(LveGetCustomDraw, LveSetCustomDraw)				BOOL CustomDraw;
	ECKPROP(LveGetAutoDarkMode, LveSetAutoDarkMode)			BOOL AutoDarkMode;
	ECKPROP(LveGetAlphaColorInDark, LveSetAlphaColorInDark) BOOL AlphaColorInDark;
	ECKPROP(LveGetAlphaValue, LveSetAlphaValue)				BYTE AlphaValue;
	ECKPROP(LveGetAutoColorPack, LveSetAutoColorPack)		BOOL AutoColorPack;
	ECKPROP(LveGetAddSplitterForClr, LveSetAddSplitterForClr)		BOOL AddSplitterForClr;
	ECKPROP(LveGetCtrlASelectAll, LveSetCtrlASelectAll)		BOOL CtrlASelectAll;
	ECKPROP(LveGetImplOwnerDataNotify, LveSetImplOwnerDataNotify)	BOOL ImplOwnerDataNotify;
	ECKPROP(LveGetDoNotWrapInTile, LveSetDoNotWrapInTile)	BOOL DoNotWrapInTile;
	ECKPROP(LveGetOwnerDataBufferSize, LveSetOwnerDataBufferSize)	size_t OwnerDataBufferSize;
	ECKPROP_W(LveSetHeaderHeight)							int HeaderHeight;

	ECK_CWND_SINGLEOWNER;
	~CListViewExt()
	{
		for (const auto e : m_vRecycleData)
			delete e;
		m_vRecycleData.clear();
	}

	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		switch (uMsg)
		{
		case WM_NOTIFY:
		{
			switch (((NMHDR*)lParam)->code)
			{
			case NM_CUSTOMDRAW:// for Header
			{
				const auto pnmcd = (NMCUSTOMDRAW*)lParam;
				switch (pnmcd->dwDrawStage)
				{
				case CDDS_PREPAINT:
					return CDRF_NOTIFYITEMDRAW;
				case CDDS_ITEMPREPAINT:
					SetTextColor(pnmcd->hdc, m_crHeaderText == CLR_DEFAULT ?
						m_pThrCtx->crDefText : m_crHeaderText);
					return CDRF_DODEFAULT;
				}
			}
			return CDRF_DODEFAULT;
			}
		}
		break;

		case WM_PRINTCLIENT:
		case WM_PAINT:
		{
			if (!m_bGridLines || (m_iViewType != LV_VIEW_DETAILS))
				break;
			PAINTSTRUCT ps;
			BeginPaint(hWnd, wParam, ps);
			CListView::OnMsg(hWnd, WM_PAINT, (WPARAM)ps.hdc, 0);

			RECT rcItem;
			const auto cCol = m_Header.GetItemCount();

			if (m_crGridLineH != CLR_NONE)
			{
				GetItemRect(0, &rcItem, LVIR_BOUNDS);
				const int cyItem = rcItem.bottom - rcItem.top;
				m_Header.GetItemRect(0, &rcItem);
				int y = rcItem.bottom;

				SetDCPenColor(ps.hdc, m_crGridLineH == CLR_DEFAULT ?
					m_pThrCtx->crDefText : m_crGridLineH);
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
				const int oxHeader = -GetSbPos(SB_HORZ);
				SetDCPenColor(ps.hdc, m_crGridLineV == CLR_DEFAULT ?
					m_pThrCtx->crDefText : m_crGridLineV);
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

		case WM_KEYDOWN:
			if (wParam == 'A' && !m_bSingleSel)
			{
				if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
				{
					SetItemState(-1, LVIS_SELECTED, LVIS_SELECTED);
					return 0;
				}
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
			if (m_pThrCtx)
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
			UpdateDpiSize(m_Ds, m_iDpi);
			break;

		case WM_STYLECHANGED:
			if (wParam == GWL_STYLE)
				UpdateStyleOptions(((STYLESTRUCT*)lParam)->styleNew);
			break;

		case WM_CREATE:
		{
			m_pThrCtx = GetThreadCtx();
			m_DcAlpha.Create(hWnd, 1, 1);
			const auto lResult = CListView::OnMsg(hWnd, uMsg, wParam, lParam);
			if (!lResult)
			{
				if (const auto hHeader = GetHeaderCtrlHWnd(); hHeader)
					m_Header.AttachNew(hHeader);
				m_hTheme = OpenThemeData(hWnd, L"ListView");
				m_iViewType = (int)GetView();
				UpdateStyleOptions(Style);
				UpdateLvExOptions(GetLVExtendStyle());
				m_iDpi = GetDpi(hWnd);
				m_bHasFocus = (GetFocus() == hWnd);
				UpdateDpiSize(m_Ds, m_iDpi);
				HandleThemeChange();
			}
			return lResult;
		}
		break;

		case WM_DESTROY:
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
			LveSetClrPack({});
			m_bCustomDraw = m_bAutoDarkMode = m_bAlphaClrInDark =
				m_bAutoColorPack = m_bAddSplitterForClr = m_bCtrlASelectAll = TRUE;
			m_byColorAlpha = 70;
			m_cyHeader = 0;
			m_cchOdTextBuf = 0;
			m_pfnOwnerData = nullptr;
			m_pOdProcData = nullptr;
		}
		break;

		case LVM_SETTILEVIEWINFO:
		{
			const auto lResult = CListView::OnMsg(hWnd, uMsg, wParam, lParam);
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
			const auto lResult = CListView::OnMsg(hWnd, uMsg, wParam, lParam);
			if (!m_Header.IsValid())
				if (const auto hHeader = GetHeaderCtrlHWnd(); hHeader)
					m_Header.AttachNew(hHeader);
			if (lResult == 1)
				m_iViewType = (int)wParam;
			return lResult;
		}
		break;

		case LVM_SETIMAGELIST:
		{
			const auto lResult = CListView::OnMsg(hWnd, uMsg, wParam, lParam);
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
			const auto lResult = CListView::OnMsg(hWnd, uMsg, wParam, lParam);
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
		return CListView::OnMsg(hWnd, uMsg, wParam, lParam);
	}

	LRESULT OnNotifyMsg(HWND hParent, UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bProcessed) override
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
					if (m_cchOdTextBuf)
					{
						ie.pszText = p->item.pszText;
						ie.cchText = p->item.cchTextMax;
					}
				}
				if (p->item.mask & LVIF_IMAGE)
					ie.uMask |= LVE_IM_IMAGE;
				if (p->item.mask & LVIF_STATE)
					ie.uMask |= LVE_IM_STATE;
				m_pfnOwnerData(LveOd::GetDispInfo, &ie, m_pOdProcData);
				if (ie.uMask & LVE_IM_TEXT)
				{
					if (m_cchOdTextBuf)
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
			}
		}
		break;
		}
		return CListView::OnNotifyMsg(hParent, uMsg, wParam, lParam, bProcessed);
	}

	EckInline constexpr void LveSetTextClr(COLORREF cr) { m_crDefText = cr; }
	EckInline constexpr COLORREF LveGetTextClr() const { return m_crDefText; }

	EckInline constexpr void LveSetHeaderTextClr(COLORREF cr) { m_crHeaderText = cr; }
	EckInline constexpr COLORREF LveGetHeaderTextClr() const { return m_crHeaderText; }

	EckInline constexpr void LveSetOddLineTextClr(COLORREF cr) { m_crOddLineText = cr; }
	EckInline constexpr COLORREF LveGetOddLineTextClr() const { return m_crOddLineText; }

	EckInline constexpr void LveSetEvenLineTextClr(COLORREF cr) { m_crEvenLineText = cr; }
	EckInline constexpr COLORREF LveGetEvenLineTextClr() const { return m_crEvenLineText; }

	EckInline constexpr void LveSetOddLineBkClr(COLORREF cr) { m_crOddLineBk = cr; }
	EckInline constexpr COLORREF LveGetOddLineBkClr() const { return m_crOddLineBk; }

	EckInline constexpr void LveSetEvenLineBkClr(COLORREF cr) { m_crEvenLineBk = cr; }
	EckInline constexpr COLORREF LveGetEvenLineBkClr() const { return m_crEvenLineBk; }

	EckInline constexpr void LveSetGridLineHClr(COLORREF cr) { m_crGridLineH = cr; }
	EckInline constexpr COLORREF LveGetGridLineHClr() const { return m_crGridLineH; }

	EckInline constexpr void LveSetGridLineVClr(COLORREF cr) { m_crGridLineV = cr; }
	EckInline constexpr COLORREF LveGetGridLineVClr() const { return m_crGridLineV; }

	constexpr void LveSetClrPack(const LVE_COLOR_PACK& cp)
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

	void LveSetItem(const LVE_ITEM_EXT& ie)
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

	BOOL LveGetItem(LVE_ITEM_EXT& ie) const
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
	void LveSetOwnerDataProc(FOwnerData pfn, void* pParam)
	{
		m_pfnOwnerData = pfn;
		m_pOdProcData = pParam;
	}

	void LveSetHeaderHeight(int cy)
	{
		m_cyHeader = cy;
		if (m_cyHeader > 0)
		{
			if (!m_bInstalledHeaderHook)
			{
				m_Header.GetSignal().Connect(
					[this](HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bProcessed)->LRESULT
					{
						if (uMsg == HDM_LAYOUT && m_cyHeader > 0)
						{
							bProcessed = TRUE;
							const auto lResult = m_Header.OnMsg(hWnd, uMsg, wParam, lParam);
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
		SendMsg(WM_SIZE, 0, MAKELPARAM(rc.right, rc.bottom));
	}

	CHeaderExt& LveGetHeader() { return m_Header; }

	EckInline constexpr void LveSetCustomDraw(BOOL b) { m_bCustomDraw = b; }
	EckInline constexpr BOOL LveGetCustomDraw() const { return m_bCustomDraw; }

	EckInline constexpr void LveSetAutoDarkMode(BOOL b) { m_bAutoDarkMode = b; }
	EckInline constexpr BOOL LveGetAutoDarkMode() const { return m_bAutoDarkMode; }

	EckInline constexpr void LveSetAlphaColorInDark(BOOL b) { m_bAlphaClrInDark = b; }
	EckInline constexpr BOOL LveGetAlphaColorInDark() const { return m_bAlphaClrInDark; }

	EckInline constexpr void LveSetAlphaValue(BYTE b) { m_byColorAlpha = b; }
	EckInline constexpr BYTE LveGetAlphaValue() const { return m_byColorAlpha; }

	EckInline constexpr void LveSetAutoColorPack(BOOL b) { m_bAutoColorPack = b; }
	EckInline constexpr BOOL LveGetAutoColorPack() const { return m_bAutoColorPack; }

	EckInline constexpr void LveSetAddSplitterForClr(BOOL b) { m_bAddSplitterForClr = b; }
	EckInline constexpr BOOL LveGetAddSplitterForClr() const { return m_bAddSplitterForClr; }

	EckInline constexpr void LveSetCtrlASelectAll(BOOL b) { m_bCtrlASelectAll = b; }
	EckInline constexpr BOOL LveGetCtrlASelectAll() const { return m_bCtrlASelectAll; }

	EckInline constexpr void LveSetImplOwnerDataNotify(BOOL b) { m_bImplLvOdNotify = b; }
	EckInline constexpr BOOL LveGetImplOwnerDataNotify() const { return m_bImplLvOdNotify; }

	EckInline constexpr void LveSetDoNotWrapInTile(BOOL b) { m_bDoNotWrapInTile = b; }
	EckInline constexpr BOOL LveGetDoNotWrapInTile() const { return m_bDoNotWrapInTile; }

	EckInline constexpr void LveSetOwnerDataBufferSize(size_t cch)
	{
		m_cchOdTextBuf = std::min(cch, (size_t)MAX_PATH);
	}
	EckInline constexpr size_t LveGetOwnerDataBufferSize() const { return m_cchOdTextBuf; }
};
ECK_RTTI_IMPL_BASE_INLINE(CListViewExt, CListView);
ECK_NAMESPACE_END