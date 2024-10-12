/*
* WinEzCtrlKit Library
*
* CListViewExt.h ： 列表视图扩展
*
* Copyright(C) 2024 QingKong
*/
#pragma once
ECK_DISABLE_ARITHMETIC_OVERFLOW_WARNING

#include "CListView.h"
#include "CtrlGraphics.h"

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

constexpr inline LVE_COLOR_PACK LveLightClr1{
	.crEvenLineBk = 0xF0F0F0,
};
constexpr inline LVE_COLOR_PACK LveDarkClr1{
	.crEvenLineBk = 0x202020,
};

/*
CListViewExt封装了ListView的常用扩展功能
===以下ListView的原始功能被忽略===

===以下功能效果仍相同但处理机制已变化===
整体的颜色设置 - 应使用Lve系列方法管理，背景颜色仍使用SetBkClr
表格线 - LVS_EX_GRIDLINES永远不会达到ListView，而是由内部接管；增加颜色方法
*/
class CListViewExt :public CListView
{
public:
	ECK_RTTI(CListViewExt);
private:
	struct ITEMINFO
	{
		COLORREF crText;
		COLORREF crTextBk;
		COLORREF crBk;
	};

	// ListView信息
	CHeader m_Header{};		// 表头
	HIMAGELIST m_hIL[4]{};	// 图像列表
	SIZE m_sizeIL[4]{};		// 图像列表大小
	int m_iViewType{};		// 视图类型
	BITBOOL m_bOwnerData : 1 = FALSE;	// 所有者数据
	BITBOOL m_bSubItemImg : 1 = FALSE;	// 显示子项图像
	BITBOOL m_bFullRowSel : 1 = FALSE;	// 整行选择
	BITBOOL m_bShowSelAlways : 1 = FALSE;	// 始终显示选择
	BITBOOL m_bBorderSelect : 1 = FALSE;	// 边框选择 TODO
	BITBOOL m_bCheckBoxes : 1 = FALSE;	// 显示复选框 TODO:可考虑三态复选和单选
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

	BYTE m_byColorAlpha{ 70 };		// 透明度
	// 
	const ECKTHREADCTX* m_pThrCtx{};	// 线程上下文

	ECK_DS_BEGIN(DPIS)
		ECK_DS_ENTRY_DYN(cxEdge, GetSystemMetrics(SM_CXEDGE))
		;
	ECK_DS_END_VAR(m_Ds);

	void GetColumnMetrics(int* px, int cCol) const
	{
		RECT rc;
		for (int i = 0; i < cCol; ++i)
		{
			m_Header.GetItemRect(i, &rc);
			px[i * 2] = rc.left;
			px[i * 2 + 1] = rc.right;
		}
	}

	LRESULT OnItemPrePaint(NMLVCUSTOMDRAW* pnmlvcd)
	{
		//return CDRF_DODEFAULT;
		if (!m_bCustomDraw || IsRectEmpty(pnmlvcd->nmcd.rc))
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
		int cCol;

		RECT rc, rcItem;

		LVITEMW li;
		li.mask = LVIF_IMAGE | LVIF_INDENT | LVIF_STATE;
		li.iItem = idx;
		li.iSubItem = 0;
		li.stateMask = 0xFFFFFFFF;
		GetItem(&li);

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
		m_crEvenLineBk = 0xff;
		const BOOL bFillClrPostTheme = (m_bAlphaClrInDark && ShouldAppsUseDarkMode() && iState);
		if (m_iViewType == LV_VIEW_DETAILS)
		{
			if (m_bAddSplitterForClr)
			{
				if (!pColMetrics)
				{
					cCol = m_Header.GetItemCount();
					pColMetrics = (int*)_malloca(cCol * 2 * sizeof(int));
					EckCheckMem(pColMetrics);
					GetColumnMetrics(pColMetrics, cCol);
				}
			}

			if (!bFillClrPostTheme)
				if (idx % 2)
				{
					if (m_crOddLineBk != CLR_DEFAULT)
					{
						SetDCBrushColor(hDC, m_crOddLineBk);
						FillRect(hDC, &rcItem, GetStockBrush(DC_BRUSH));
						if (m_iViewType == LV_VIEW_DETAILS)
						{
							EckCounter(cCol, i)
								if (pColMetrics[i * 2 + 1] > 0)
									DrawListViewColumnDetail(m_hTheme, hDC,
										pColMetrics[i * 2 + 1], rcItem.top, rcItem.bottom);
						}
					}
				}
				else
				{
					if (m_crEvenLineBk != CLR_DEFAULT)
					{
						SetDCBrushColor(hDC, m_crEvenLineBk);
						FillRect(hDC, &rcItem, GetStockBrush(DC_BRUSH));
						if (m_iViewType == LV_VIEW_DETAILS)
						{
							EckCounter(cCol, i)
								if (pColMetrics[i * 2 + 1] > 0)
									DrawListViewColumnDetail(m_hTheme, hDC,
										pColMetrics[i * 2 + 1], rcItem.top, rcItem.bottom);
						}
					}
				}
		}

		if (iState)
		{
			DrawThemeBackground(m_hTheme, hDC, LVP_LISTITEM, iState,
				&rcItem, nullptr);
		}

		if ((m_iViewType == LV_VIEW_DETAILS) && bFillClrPostTheme)
		{
			constexpr RECT rcDst{ 0,0,1,1 };
			if (idx % 2)
			{
				if (m_crOddLineBk != CLR_DEFAULT)
				{
					SetDCBrushColor(m_DcAlpha.GetDC(), m_crOddLineBk);
					FillRect(m_DcAlpha.GetDC(), &rcDst, GetStockBrush(DC_BRUSH));
					AlphaBlend(hDC, rcItem.left, rcItem.top, rcItem.right - rcItem.left,
						rcItem.bottom - rcItem.top,
						m_DcAlpha.GetDC(), 0, 0, 1, 1, { AC_SRC_OVER,0,m_byColorAlpha,0 });
				}
			}
			else
			{
				if (m_crEvenLineBk != CLR_DEFAULT)
				{
					SetDCBrushColor(m_DcAlpha.GetDC(), m_crEvenLineBk);
					FillRect(m_DcAlpha.GetDC(), &rcDst, GetStockBrush(DC_BRUSH));
					AlphaBlend(hDC, rcItem.left, rcItem.top, rcItem.right - rcItem.left,
						rcItem.bottom - rcItem.top,
						m_DcAlpha.GetDC(), 0, 0, 1, 1, { AC_SRC_OVER,0,m_byColorAlpha,0 });
				}
			}

		}

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
		WCHAR sz[MAX_PATH];// 无论如何，ListView只显示前255个字符
		li.mask = LVIF_TEXT | LVIF_IMAGE;
		li.pszText = sz;
		li.cchTextMax = MAX_PATH;
		li.iItem = idx;

		// HACK : 增加文本背景支持
		SetBkMode(hDC, TRANSPARENT);
		const auto crNormal = m_crDefText == CLR_DEFAULT ?
			m_pThrCtx->crDefText : m_crDefText;
		if (m_iViewType == LV_VIEW_DETAILS)
			if (idx % 2)
				if (m_crOddLineText != CLR_DEFAULT)
					SetTextColor(hDC, m_crOddLineText);
				else
					SetTextColor(hDC, crNormal);
			else
				if (m_crEvenLineText != CLR_DEFAULT)
					SetTextColor(hDC, m_crEvenLineText);
				else
					SetTextColor(hDC, crNormal);
		else
			SetTextColor(hDC, crNormal);

		if (m_iViewType == LV_VIEW_DETAILS)
		{
			const auto cCol = m_Header.GetItemCount();
			HDITEMW hdi;
			hdi.mask = HDI_FORMAT;
			int cxImg = 0;
			for (li.iSubItem = 0; li.iSubItem < cCol; ++li.iSubItem)
			{
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
				DrawTextW(hDC, li.pszText, -1, &rc, uDtFlags);
			}
		}
		else
		{
			GetItem(&li);
			GetItemRect(idx, &rc, LVIR_LABEL);
			UINT uDtFlags;

			if (m_iViewType == LV_VIEW_ICON && (bSelected || (pnmlvcd->nmcd.uItemState & CDIS_FOCUS)))
				uDtFlags = DT_CENTER | DT_NOPREFIX | DT_WORDBREAK | DT_EDITCONTROL;
			else
			{
				uDtFlags = DT_END_ELLIPSIS | DT_NOPREFIX | DT_SINGLELINE | DT_CENTER;
				if (m_iViewType == LV_VIEW_SMALLICON || m_iViewType == LV_VIEW_LIST)
					uDtFlags |= DT_VCENTER;
			}
			if (!((m_iViewType == LV_VIEW_SMALLICON || m_iViewType == LV_VIEW_ICON) && m_bHideLabels))
				DrawTextW(hDC, li.pszText, -1, &rc, uDtFlags);
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

	ECK_CWND_SINGLEOWNER;

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
		return 0;

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
					m_Header.Attach(hHeader);
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

		case LVM_SETVIEW:
		{
			const auto lResult = CListView::OnMsg(hWnd, uMsg, wParam, lParam);
			if (!m_Header.IsValid())
				if (const auto hHeader = GetHeaderCtrlHWnd(); hHeader)
					m_Header.Attach(hHeader);
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
		if (m_bCustomDraw)
			switch (uMsg)
			{
			case WM_NOTIFY:
			{
				switch (((NMHDR*)lParam)->code)
				{
				case NM_CUSTOMDRAW:
				{
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
		m_crDefText = cp.crDefText;
		m_crHeaderText = cp.crHeaderText;
		m_crOddLineText = cp.crOddLineText;
		m_crEvenLineText = cp.crEvenLineText;
		m_crOddLineBk = cp.crOddLineBk;
		m_crEvenLineBk = cp.crEvenLineBk;
		m_crGridLineH = cp.crGridLineH;
		m_crGridLineV = cp.crGridLineV;
	}
};
ECK_RTTI_IMPL_BASE_INLINE(CListViewExt, CListView);
ECK_NAMESPACE_END