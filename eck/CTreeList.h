/*
* WinEzCtrlKit Library
*
* CTreeList.h ： 树列表
*
* Copyright(C) 2023 QingKong
*/
#pragma once
#include "CHeader.h"
#include "GraphicsHelper.h"
#include "CtrlGraphics.h"

#include <vssym32.h>

ECK_NAMESPACE_BEGIN
// 项目标志
enum :UINT
{
	TLIF_CLOSED = (1u << 1),
	TLIF_SELECTED = (1u << 2),
	TLIF_HOTEXPANDBTN = (1u << 3),
};
// TLITEM掩码
enum :UINT
{
	TLIM_TEXT = (1u << 0),

	TLIM_ALL = TLIM_TEXT,
};
// 部件
enum
{
	TLIP_NONE,// 空白
	TLIP_EXPANDBTN,// 展开按钮
	TLIP_ICON,// 图标
	TLIP_TEXT,// 文本
};
// 命中测试标志
enum :UINT
{
	TLHTF_ONLYITEM = (1u << 0),
	TLHTF_NOHITTEXTLABEL = (1u << 1),
};
// 节点
struct TLNODE
{
	USHORT uFlags;	// TLIF_标志
	short iLevel;	// 层次，根节点为1，此后逐层+1
	int cChildren;	// 子节点数
	int idxParent;	// 父节点索引，-1 = 根节点
	int idxImg;		// 图像列表索引，-1 = 无效
};
// 项目
struct TLITEM
{
	UINT uMask;
	int idxSubItem;
	TLNODE* pNode;
	UINT uFlags;
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
	BOOL bQueryRoot;// 是否请求根节点
	union
	{
		TLNODE* pParent;	// 正在请求其子项目的父项
		int cTopItem;		// 根节点的数量
	};
	TLNODE** pChildren;	// 所有子项的数组
};

struct NMTLFILLALLFLATITEM
{
	NMHDR nmhdr;
	int cChildren;		// 项目数
	TLNODE** pChildren;	// 所有项目的数组
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

enum
{
	TLCDD_PREFILLBK,// 将要填充背景，仅使用hDC和rcItem
	TLCDD_PREPAINTITEM,// 将要绘制项目
};

enum :UINT
{
	TLCDRF_NONE = 0,
	TLCDRF_TEXTCLRCHANGED = (1u << 0),// 文本颜色已改变
	TLCDRF_SKIPDEFAULT = (1u << 1),// 终止默认绘制
	TLCDRF_BKGNDCHANGED = (1u << 2),// 背景已改变
};

struct NMTLCUSTOMDRAW
{
	NMHDR nmhdr;
	HDC hDC;
	const RECT* prcItem;
	TLNODE* pNode;
	int iDrawStage;

	int idxItem;
	int idxSubItem;

	int iStateIdItem;
	int iPartIdGlyph;
	int iStateIdGlyph;

	COLORREF crText;
	COLORREF crTextBk;
};

class CTreeList;
class CTLHeader final :public CHeader
{
private:
	CTreeList& m_TL;
public:
	CTLHeader(CTreeList& tl) :m_TL{ tl } {}

	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;
};

constexpr inline int TL_IDC_HEADER = 101;// 表头控件ID

class CTreeList :public CWnd
{
	friend class CTLHeader;
private:
	struct COL
	{
		int iLeft;
		int iRight;
		int idxActual;// 这一项对应的实际项目索引
	};

	std::vector<TLNODE*> m_Item{};
	std::vector<COL> m_vCol{};// 按显示顺序排列的列信息

	CTLHeader m_Header{ *this };

	HTHEME m_hThemeTV = NULL;
	HTHEME m_hThemeLV = NULL;
	HFONT m_hFont = NULL;
	CEzCDC m_DC{};
	COLORREF m_crBranchLine = CLR_INVALID;

	int m_idxTopItem = 0;	// 第一可见项
	int m_idxHot = -1;		// 热点项
	int m_idxFocus = -1;	// 焦点项
	int m_idxMark = -1;		// mark，范围选择用
	int m_idxSel = -1;		// 选中的项，仅用于单选模式

	int m_cxItem = 0,
		m_cyItem = 0;
	int m_cxClient = 0,
		m_cyClient = 0;
	SIZE m_sizeTVGlyph{};
	int m_dxContent = 0;
	int m_cyHeader = 0;

	POINT m_ptDraggingSelStart{};
	RECT m_rcDraggingSel{};

	HIMAGELIST m_hImgList = NULL;
	int m_cxImg = 0,
		m_cyImg = 0;

	int m_cScrollLine = 3;							// 一次滚动行数
	int m_msDraggingSelScrollGap = 30;				// 拖动选择时自动滚动最小间隔

	CRefStrW m_rsWatermark{};

#ifdef _DEBUG
	BITBOOL m_bDbgDrawIndex : 1 = 1;				// 【调试】绘制项目索引
	BITBOOL m_bDbgDrawMarkItem : 1 = 1;				// 【调试】绘制mark项
#endif
	BITBOOL m_bExpandBtnHot : 1 = FALSE;			// 展开按钮是否点燃
	BITBOOL m_bDraggingSel : 1 = FALSE;				// 是否处于拖动选择状态
#ifdef _DEBUG
	BITBOOL m_bFocusIndicatorVisible : 1 = TRUE;	// 焦点指示器是否可见
#else
	BITBOOL m_bFocusIndicatorVisible : 1 = FALSE;
#endif

	BITBOOL m_bFlatMode : 1 = FALSE;					// 平面列表模式
	BITBOOL m_bSingleSel : 1 = FALSE;					// 单选
	BITBOOL m_bDisallowBeginDragInItemSpace : 1 = FALSE;// 禁止在项目内的空白区启动拖动选择
	BITBOOL m_bBackgroundNotSolid : 1 = FALSE;			// 背景是否不是纯色

	int m_iDpi = USER_DEFAULT_SCREEN_DPI;
	ECK_DS_BEGIN(DPIS)
		ECK_DS_ENTRY(cyHeaderDef, 26)
		ECK_DS_ENTRY(cyItemDef, 24)
		ECK_DS_ENTRY(cxTextMargin, 4)
		;
	ECK_DS_END_VAR(m_Ds);


	/// <summary>
	/// 添加虚拟项目。
	/// 函数先将pParent尾插进列表，然后递归尾插其下所有子项
	/// </summary>
	/// <param name="pParent">当前父项目</param>
	/// <param name="pNode">子项目数组</param>
	/// <param name="iLevel">父项目层次</param>
	void AddVirtualItem(TLNODE* pParent, TLNODE** pNode, int iLevel)
	{
		m_Item.emplace_back(pParent);
		pParent->iLevel = iLevel;
		if (!IsBitSet(pParent->uFlags, TLIF_CLOSED))
		{
			const int idxCurrParent = (int)m_Item.size() - 1;
			EckCounter(pParent->cChildren, i)
			{
				pNode[i]->idxParent = idxCurrParent;
				NMTLFILLCHILDREN nm{};
				nm.pParent = pNode[i];
				FillNmhdrAndSendNotify(nm, NM_TL_FILLCHILDREN);
				AddVirtualItem(nm.pParent, nm.pChildren, iLevel + 1);
			}
		}
	}

	void PaintItem(HDC hDC, int idx)
	{
		//---------------准备
		const int x = m_dxContent;
		const int y = (idx - m_idxTopItem) * m_cyItem + m_cyHeader;
		const auto e = m_Item[idx];

		RECT rcItem{ x,y,m_cxItem + x,y + m_cyItem };
		const int cxIndent = CalcIndent(e);
		RECT rc
		{
			x + m_Ds.cxTextMargin + cxIndent + m_cxImg,
			rcItem.top,
			0,
			rcItem.bottom
		};

		HRGN hRgn = NULL;
		COLORREF crOldText = CLR_INVALID, crOldTextBk = CLR_INVALID;

		int iStateId;
		if (IsBitSet(e->uFlags, TLIF_SELECTED) || (m_bSingleSel && m_idxSel == idx))
			if (idx == m_idxHot)
				iStateId = LISS_HOTSELECTED;
			else
				iStateId = LISS_SELECTED;
		else if (idx == m_idxHot)
			iStateId = LISS_HOT;
		else
			iStateId = 0;

		NMTLCUSTOMDRAW nmcd;
		nmcd.hDC = hDC;
		nmcd.prcItem = &rcItem;
		nmcd.pNode = e;
		nmcd.iDrawStage = TLCDD_PREPAINTITEM;
		nmcd.idxItem = idx;
		nmcd.idxSubItem = -1;
		nmcd.iStateIdItem = iStateId;
		nmcd.iPartIdGlyph = (m_bExpandBtnHot && m_idxHot == idx) ? TVP_HOTGLYPH : TVP_GLYPH;
		nmcd.iStateIdGlyph = IsBitSet(e->uFlags, TLIF_CLOSED) ? GLPS_CLOSED : GLPS_OPENED;
		nmcd.crText = nmcd.crTextBk = CLR_INVALID;
		const UINT uCustom = (UINT)FillNmhdrAndSendNotify(nmcd, NM_TL_CUSTOMDRAW);

		if (uCustom & TLCDRF_SKIPDEFAULT)
			goto End;
		if (uCustom & TLCDRF_BKGNDCHANGED)
			PaintDivider(hDC, rcItem);
		//---------------画背景
		if (iStateId)
			DrawThemeBackground(m_hThemeLV, hDC, LVP_LISTITEM, iStateId, &rcItem, NULL);
		if (m_bFocusIndicatorVisible && idx == m_idxFocus)
		{
			InflateRect(&rcItem, -1, -1);
			DrawFocusRect(hDC, &rcItem);
			InflateRect(&rcItem, 1, 1);
		}
		//---------------画文本
		if (uCustom & TLCDRF_TEXTCLRCHANGED)
		{
			if (nmcd.crText != CLR_INVALID)
				crOldText = SetTextColor(hDC, nmcd.crText);
			if (nmcd.crTextBk != CLR_INVALID)
				crOldTextBk = SetBkColor(hDC, nmcd.crTextBk);
		}

		NMTLGETDISPINFO nm;
		nm.Item.pNode = e;
		nm.Item.uMask = TLIM_TEXT;
		FillNmhdr(nm, NM_TL_GETDISPINFO);

		// 按可见顺序循环
		EckCounter(m_vCol.size(), i)
		{
			nm.Item.idxSubItem = m_vCol[i].idxActual;
			nm.Item.cchText = 0;
			nm.Item.pszText = NULL;
			SendNotify(nm);

			rc.right = m_vCol[i].iRight + x - m_Ds.cxTextMargin;
#pragma warning(suppress:6387)// nm.Item.pszText可能为NULL
			DrawTextW(hDC, nm.Item.pszText, nm.Item.cchText, &rc,
				DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX | DT_NOCLIP);
			rc.left = rc.right + m_Ds.cxTextMargin * 2;
		}
		if (uCustom & TLCDRF_TEXTCLRCHANGED)
		{
			if (crOldText != CLR_INVALID)
				SetTextColor(hDC, crOldText);
			if (crOldTextBk != CLR_INVALID)
				SetBkColor(hDC, crOldTextBk);
		}
		//---------------画图片
		if (e->idxImg >= 0 || e->cChildren)// 需要设置剪辑区
		{
			rc.left = 0;
			rc.right = m_vCol.front().iRight;
			hRgn = CreateRectRgnIndirect(&rc);
			SelectClipRgn(hDC, hRgn);
			DeleteObject(hRgn);
		}
		if (e->idxImg >= 0)
		{
			EckAssert(e->idxImg < ImageList_GetImageCount(m_hImgList));
			RECT rcImg;
			rcImg.left = x + cxIndent;
			rcImg.top = y + (m_cyItem - m_cyImg) / 2;
			rcImg.right = rcImg.left + m_cxImg;
			rcImg.bottom = rcImg.top + m_cyImg;
			ImageList_Draw(m_hImgList, e->idxImg, hDC, rcImg.left, rcImg.top, ILD_NORMAL);
		}
		//---------------画折叠按钮
		int cLevel;
		if (!m_bFlatMode && e->cChildren)
		{
			rc.left = x + cxIndent - m_sizeTVGlyph.cx;
			rc.right = rc.left + m_sizeTVGlyph.cx;
			rc.top += ((m_cyItem - m_sizeTVGlyph.cy) / 2);
			rc.bottom = rc.top + m_sizeTVGlyph.cy;
			DrawThemeBackground(m_hThemeTV, hDC, nmcd.iPartIdGlyph, nmcd.iStateIdGlyph, &rc, NULL);
			cLevel = e->iLevel - 1;
		}
		else
			cLevel = e->iLevel;
		////---------------画分支线
		//for (int i = 0; i < cLevel; ++i)
		//{
		//	SetDCPenColor(hDC, m_crBranchLine);
		//	SelectObject(hDC, GetStockBrush(DC_PEN));
		//	const int xLine = x + i * m_sizeTVGlyph.cx + m_sizeTVGlyph.cx / 2;
		//	MoveToEx(hDC, xLine, rcItem.top, NULL);
		//	LineTo(hDC, xLine, rcItem.bottom);
		//}

		if (hRgn)
			SelectClipRgn(hDC, NULL);
	End:;
#ifdef _DEBUG
		const auto cr = SetTextColor(hDC, Colorref::Red);
		if (m_bDbgDrawIndex)
			DrawTextW(hDC, std::to_wstring(idx).c_str(), -1, (RECT*)&rcItem,
				DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX | DT_NOCLIP);

		if (m_bDbgDrawMarkItem && idx == m_idxMark && !m_vCol.empty())
		{
			rcItem.right = m_vCol.front().iRight;
			DrawTextW(hDC, L"Mark", -1, (RECT*)&rcItem,
				DT_VCENTER | DT_SINGLELINE | DT_RIGHT | DT_NOPREFIX | DT_NOCLIP);
		}
		SetTextColor(hDC, cr);
#endif
	}

	EckInline int CalcIndent(const TLNODE* e) const
	{
		return (m_bFlatMode ? m_Ds.cxTextMargin/*边缘缩进*/ : (e->iLevel * m_sizeTVGlyph.cx));
	}

	EckInline int CalcIndentMinusGlyph(const TLNODE* e) const
	{
		return (m_bFlatMode ? m_Ds.cxTextMargin/*边缘缩进*/ : ((e->iLevel - 1) * m_sizeTVGlyph.cx));
	}

	EckInline void PaintBk(HDC hDC, const RECT& rc)
	{
		NMTLCUSTOMDRAW nm{};
		nm.hDC = hDC;
		nm.prcItem = &rc;
		nm.iDrawStage = TLCDD_PREFILLBK;
		if (!(FillNmhdrAndSendNotify(nm, NM_TL_CUSTOMDRAW) & TLCDRF_SKIPDEFAULT))
			FillRect(hDC, &rc, GetSysColorBrush(COLOR_WINDOW));
	}

	EckInline void PaintDivider(HDC hDC, const RECT& rc)
	{
		for (const auto& e : m_vCol)
			DrawListViewColumnDetail(m_hThemeLV, hDC, e.iRight + m_dxContent,
				rc.top, rc.bottom);
	}

	EckInline void PaintDraggingSelRect(HDC hDC)
	{
		if (m_bDraggingSel)
			DrawSelectionRect(hDC, m_rcDraggingSel);
	}

	EckInline void ReCalcTopItem()
	{
		m_idxTopItem = GetSbPos(SB_VERT);
	}

	EckInline void UpdateThemeInfo()
	{
		GetThemePartSize(m_hThemeTV, m_DC.GetDC(), TVP_GLYPH, GLPS_CLOSED, NULL, TS_TRUE, &m_sizeTVGlyph);
	}

	void UpdateColumnInfo()
	{
		int x = 0;
		const int cCol = (int)m_vCol.size();

		const auto piOrder = (int*)_malloca(sizeof(int) * cCol);
		EckCheckMem(piOrder);
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
		_freea(piOrder);
	}

	/// <summary>
	/// 更新滚动条
	/// </summary>
	void UpdateScrollBar()
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
		if (cyView < (int)m_Item.size() * m_cyItem)
		{
			if (!IsBitSet(GetStyle(), WS_VSCROLL))
				cxView -= GetSystemMetrics(SM_CXVSCROLL);
			if (cxView < m_cxItem && !bHSB)
				if (!IsBitSet(GetStyle(), WS_HSCROLL))
					cyView -= GetSystemMetrics(SM_CYHSCROLL);
		}

		SCROLLINFO si;

		if (cyView < m_cyHeader || m_Item.empty())
		{
			si.fMask = SIF_ALL;
			si.nMin = si.nMax = si.nPos = 0;
			si.nPage = 0;
			SetSbInfo(SB_VERT, &si);
			SetSbInfo(SB_HORZ, &si);
		}

		si.fMask = SIF_POS;
		GetSbInfo(SB_VERT, &si);
		si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
		si.nMin = 0;
		si.nMax = (int)m_Item.size() - 1;
		si.nPage = (cyView - m_cyHeader) / m_cyItem;
		SetSbInfo(SB_VERT, &si);
		ReCalcTopItem();

		si.fMask = SIF_POS;
		GetSbInfo(SB_HORZ, &si);
		si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
		si.nMin = 0;
		si.nMax = (cxView >= m_cxItem) ? 0 : (m_cxItem - 1);
		si.nPage = cxView;
		SetSbInfo(SB_HORZ, &si);
		si.fMask = SIF_POS;
		GetSbInfo(SB_HORZ, &si);
		m_dxContent = -si.nPos;
	}

	void BeginDraggingSelect(UINT uMk, int xBegin, int yBegin)
	{
		m_ptDraggingSelStart = { xBegin - m_dxContent,yBegin + m_idxTopItem * m_cyItem };
		m_rcDraggingSel = { m_ptDraggingSelStart.x,m_ptDraggingSelStart.y,
			m_ptDraggingSelStart.x,m_ptDraggingSelStart.y };
		m_bDraggingSel = TRUE;
		MSG msg;
		GetCursorPos(&msg.pt);
		auto ullTime = GetTickCount64();

		SetCapture(m_hWnd);
		while (GetCapture() == m_hWnd)// 如果捕获改变则应立即退出拖动循环
		{
			if (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
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
					const POINT pt ECK_GET_PT_LPARAM(msg.lParam);
					const auto ullTimeNow = GetTickCount64();
					//----------滚动
					if (ullTimeNow - ullTime >= m_msDraggingSelScrollGap)
					{
						ullTime = ullTimeNow;
						int xDelta = 0, yDelta = 0;
						if (pt.x < 0)
							xDelta = pt.x;
						else if (pt.x > m_cxClient)
							xDelta = pt.x - m_cxClient;

						if (pt.y < 0)
							yDelta = pt.y / m_cyItem - 1;
						else if (pt.y > m_cyClient)
							yDelta = (pt.y - m_cyClient) / m_cyItem + 1;

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
					OffsetRect(&rcOld, m_dxContent, -m_idxTopItem * m_cyItem);// 转客户坐标

					const POINT ptStart
					{
						m_ptDraggingSelStart.x + m_dxContent,
						m_ptDraggingSelStart.y - m_idxTopItem * m_cyItem
					};
					m_rcDraggingSel = MakeRect(ptStart, pt);
					if (rcOld == m_rcDraggingSel)
					{
						OffsetRect(&m_rcDraggingSel, -m_dxContent, m_idxTopItem * m_cyItem);
						break;
					}
					//----------准备范围
					int idxBegin = ItemFromY(std::min(m_rcDraggingSel.top, rcOld.top));
					if (idxBegin < 0)
						idxBegin = 0;
					int idxEnd = ItemFromY(std::max(m_rcDraggingSel.bottom, rcOld.bottom));
					if (idxEnd < 0)
						idxEnd = (int)m_Item.size() - 1;
					//----------
					int dCursorToItemMax = INT_MIN;
					if (!(msg.wParam & (MK_CONTROL | MK_SHIFT)))
						m_idxMark = -1;// 重置mark
					//----------
					RECT rcItem;
					for (int i = idxBegin; i <= idxEnd; ++i)
					{
						const auto e = m_Item[i];
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
								const int d = (pt.x - rcItem.left) * (pt.x - rcItem.left) +
									(pt.y - rcItem.top) * (pt.y - rcItem.top);
								if (d > dCursorToItemMax)
								{
									dCursorToItemMax = d;
									m_idxMark = i;
								}
							}
						}
					}

					Redraw();
					UpdateWindow(m_hWnd);
					OffsetRect(&m_rcDraggingSel, -m_dxContent, m_idxTopItem * m_cyItem);
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
					GetSbInfo(SB_VERT, &si);
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
					GetSbInfo(SB_HORZ, &si);
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
		OffsetRect(&m_rcDraggingSel, m_dxContent, -m_idxTopItem * m_cyItem);// 转客户坐标
		Redraw(m_rcDraggingSel);
		UpdateWindow(m_hWnd);
	}

	void UpdateDCAttr()
	{
		SetTextColor(m_DC.GetDC(), GetSysColor(COLOR_WINDOWTEXT));
		SetBkMode(m_DC.GetDC(), TRANSPARENT);
		SelectObject(m_DC.GetDC(), m_hFont);
	}

	void CheckOldData()
	{
		const int c = (int)m_Item.size();
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
	void CheckOldDataRange(int idxBegin, int idxEnd, int idxSuggestion)
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
public:
	ECK_CWND_SINGLEOWNER;
	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		switch (uMsg)
		{
		case WM_MOUSEMOVE:
		{
			TLHITTEST tlht;
			tlht.pt = ECK_GET_PT_LPARAM(lParam);
			tlht.uFlags = TLHTF_NOHITTEXTLABEL;
			int idx = HitTest(tlht);
			BOOL bExpBtnHot = m_bExpandBtnHot;
			m_bExpandBtnHot = (tlht.iPart == TLIP_EXPANDBTN);
			//EckDbgPrint(tlht.iPart);
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
			if (pnmhdr->hwndFrom != m_Header.HWnd)
				break;
			switch (pnmhdr->code)
			{
			case HDN_ITEMCHANGEDW:
			case HDN_ITEMCHANGINGW:
			{
				const auto p = (NMHEADERW*)lParam;
				if (IsBitSet(p->pitem->mask, HDI_WIDTH))
				{
					UpdateColumnInfo();
					UpdateScrollBar();
					SetWindowPos(m_Header.HWnd, NULL, m_dxContent, 0,
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

			case HDN_BEGINDRAG:
			case HDN_ENDDRAG:
				return (((NMHEADERW*)lParam)->iItem == 0);// 不允许拖动第一列

			case NM_RELEASEDCAPTURE:
				UpdateColumnInfo();
				Redraw();
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
			SCROLLINFO si;
			si.cbSize = sizeof(si);
			si.fMask = SIF_ALL;
			GetSbInfo(SB_VERT, &si);
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
			SetSbInfo(SB_VERT, &si);
			GetSbInfo(SB_VERT, &si);
			ReCalcTopItem();
			if (m_bBackgroundNotSolid)
				Redraw();
			else
			{
				RECT rc{ 0,m_cyHeader,m_cxClient,m_cyClient };
				ScrollWindowEx(hWnd, 0, (iOld - si.nPos) * m_cyItem, &rc, &rc, NULL, NULL, SW_INVALIDATE);
			}
			UpdateWindow(hWnd);
		}
		return 0;

		case WM_HSCROLL:
		{
			SCROLLINFO si;
			si.cbSize = sizeof(si);
			si.fMask = SIF_ALL;
			GetSbInfo(SB_HORZ, &si);
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
			SetSbInfo(SB_HORZ, &si);
			GetSbInfo(SB_HORZ, &si);
			m_dxContent = -si.nPos;
			m_Header.Left = m_dxContent;
			Redraw();
		}
		return 0;

		case WM_MOUSEWHEEL:
		{
			SCROLLINFO si;
			si.cbSize = sizeof(SCROLLINFO);
			si.fMask = SIF_POS;
			GetSbInfo(SB_VERT, &si);
			const int iOld = si.nPos;
			si.nPos += (-GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA * m_cScrollLine);
			SetSbInfo(SB_VERT, &si);
			GetSbInfo(SB_VERT, &si);
			ReCalcTopItem();
			if (m_bBackgroundNotSolid)
				Redraw();
			else
			{
				RECT rc{ 0,m_cyHeader,m_cxClient,m_cyClient };
				ScrollWindowEx(hWnd, 0, (iOld - si.nPos) * m_cyItem, &rc, &rc, NULL, NULL, SW_INVALIDATE);
			}
			UpdateWindow(hWnd);
		}
		return 0;

		case WM_ERASEBKGND:
			return 0;

		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint(hWnd, &ps);
			const int yOrg = ps.rcPaint.top;
			if (ps.rcPaint.top < m_cyHeader)// 永远不在表头控件区域内重画
				ps.rcPaint.top = m_cyHeader;
			//------------擦除
			PaintBk(m_DC.GetDC(), ps.rcPaint);
			if (m_Item.size())
			{
				//------------准备范围
				const int idxTop = std::max(m_idxTopItem + (ps.rcPaint.top - m_cyHeader) / m_cyItem, 0L);
				const int idxBottom = std::min(m_idxTopItem + (ps.rcPaint.bottom - m_cyHeader) / m_cyItem,
					(long)m_Item.size() - 1);
				//------------画分隔线
				PaintDivider(m_DC.GetDC(), ps.rcPaint);
				//------------画项目
				for (int i = idxTop; i <= idxBottom; ++i)
					PaintItem(m_DC.GetDC(), i);

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
			PaintDraggingSelRect(m_DC.GetDC());
			BitBltPs(&ps, m_DC.GetDC());
			ps.rcPaint.top = yOrg;
			EndPaint(hWnd, &ps);
		}
		return 0;

		case WM_SIZE:
		{
			ECK_GET_SIZE_LPARAM(m_cxClient, m_cyClient, lParam);
			m_DC.ReSize(hWnd, m_cxClient, m_cyClient);
			UpdateDCAttr();

			UpdateScrollBar();
			m_Header.Size = SIZE{ std::max(m_cxItem, m_cxClient),m_cyHeader };
		}
		return 0;

		case WM_KEYDOWN:
		{
			const int idxOldFocus = m_idxFocus;
			int idxOldSel;
			int idxBottom = m_idxTopItem + (m_cyClient - m_cyHeader) / m_cyItem - 1;
			if (idxBottom >= (int)m_Item.size())
				idxBottom = (int)m_Item.size() - 1;
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
					if (m_idxFocus >= (int)m_Item.size())
						m_idxFocus = (int)m_Item.size() - 1;
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
					if (m_idxFocus >= (int)m_Item.size())
						m_idxFocus = (int)m_Item.size() - 1;
				}
				else
					m_idxFocus = idxBottom;
			}
			break;
			case VK_LEFT:// 如果当前焦点项目已展开，折叠之，否则移至父项目
			{
				if (m_idxFocus >= 0)
				{
					const auto e = m_Item[m_idxFocus];
					if (e->cChildren && !(e->uFlags & TLIF_CLOSED))// 已展开
					{
						ExpandItem(m_idxFocus, TVE_TOGGLE);
						if (m_bSingleSel)
							m_idxSel = m_idxFocus;
						else
						{
							DeselectAll();
							e->uFlags |= TLIF_SELECTED;
						}
						RECT rc;
						GetItemRect(m_idxFocus, rc);
						rc.right = m_cxClient;
						rc.bottom = m_cyClient;
						Redraw(rc);
					}
					else if (e->idxParent >= 0)// 未展开，而且不是根项目
					{
						if (m_bSingleSel)
						{
							idxOldSel = m_idxSel;
							m_idxSel = m_idxFocus = e->idxParent;
							if (idxOldSel != m_idxSel && idxOldSel >= 0)
								RedrawItem(idxOldSel);
							if (idxOldFocus != m_idxFocus && idxOldFocus >= 0)
								RedrawItem(idxOldFocus);
						}
						else
						{
							DeselectAll();
							m_idxFocus = e->idxParent;
							m_Item[m_idxFocus]->uFlags |= TLIF_SELECTED;
							Redraw();
						}
					}
					UpdateWindow(hWnd);
					goto SkipItemAdjust;
				}
			}
			break;
			case VK_RIGHT:// 如果当前焦点项目已折叠，展开之，否则移至第一个子项目
			{
				if (m_idxFocus >= 0)
				{
					const auto e = m_Item[m_idxFocus];
					if (e->cChildren && (e->uFlags & TLIF_CLOSED))// 已折叠
					{
						ExpandItem(m_idxFocus, TVE_TOGGLE);
						if (m_bSingleSel)
							m_idxSel = m_idxFocus;
						else
						{
							DeselectAll();
							e->uFlags |= TLIF_SELECTED;
						}
						RECT rc;
						GetItemRect(m_idxFocus, rc);
						rc.right = m_cxClient;
						rc.bottom = m_cyClient;
						Redraw(rc);
					}
					else if (e->cChildren)// 有子项目
					{
						if (m_bSingleSel)
						{
							idxOldSel = m_idxSel;
							m_idxSel = ++m_idxFocus;
							if (idxOldSel != m_idxSel && idxOldSel >= 0)
								RedrawItem(idxOldSel);
							if (/*idxOldFocus != m_idxFocus && */idxOldFocus >= 0)
								RedrawItem(idxOldFocus);
						}
						else
						{
							DeselectAll();
							++m_idxFocus;
							m_Item[m_idxFocus]->uFlags |= TLIF_SELECTED;
							Redraw();
						}
					}
					UpdateWindow(hWnd);
					goto SkipItemAdjust;
				}
			}
			break;
			}

			if (m_idxFocus >= 0)// 项目必须有效
			{
				const BOOL bCtrlPressed = GetAsyncKeyState(VK_CONTROL) & 0x8000;
				const BOOL bShiftPressed = GetAsyncKeyState(VK_SHIFT) & 0x8000;
				if (!bCtrlPressed && !bShiftPressed)// Ctrl或Shift按下，不修改mark
					m_idxMark = m_idxFocus;
				if (m_idxFocus != idxOldFocus)// 已改变
				{
					if (!bCtrlPressed)// Ctrl未按下
					{
						if (bShiftPressed)// Shift按下，执行范围选择
						{
							if (m_bSingleSel)// 单选不论，总是选中当前项
							{
								idxOldSel = m_idxSel;
								m_idxSel = m_idxFocus;
								RedrawItem(m_idxSel);
								if (idxOldSel != m_idxSel && idxOldSel >= 0)
									RedrawItem(idxOldSel);
							}
							else// 多选
							{
								if (m_idxMark >= 0)// mark有效，选择范围
									OnlySelectRange(std::min(m_idxFocus, m_idxMark), std::max(m_idxFocus, m_idxMark));
								else// mark无效，只选中当前项
								{
									DeselectAll();
									m_Item[m_idxFocus]->uFlags |= TLIF_SELECTED;
								}
								Redraw();
							}
						}
						else// Shift未按下
						{
							if (m_bSingleSel)
							{
								idxOldSel = m_idxSel;
								m_idxSel = m_idxFocus;
								RedrawItem(m_idxSel);
								if (m_bFocusIndicatorVisible && idxOldFocus >= 0)
									RedrawItem(idxOldFocus);
							}
							else
							{
								DeselectAll();
								m_Item[m_idxFocus]->uFlags |= TLIF_SELECTED;
								Redraw();
							}
						}
					}
					else// Ctrl按下，则只移动焦点，如果必要，重画焦点项
					{
						RedrawItem(m_idxFocus);
						if (m_bFocusIndicatorVisible && idxOldFocus >= 0)
							RedrawItem(idxOldFocus);
					}

					if (m_idxFocus < m_idxTopItem || m_idxFocus > idxBottom)
						ScrollV(m_idxFocus - idxOldFocus);// 执行滚动
					UpdateWindow(hWnd);
				}
			}
		SkipItemAdjust:;
		}
		break;

		case WM_LBUTTONDOWN:
		{
			SetFocus(hWnd);
			TLHITTEST tlht;
			tlht.pt = ECK_GET_PT_LPARAM(lParam);
			tlht.uFlags = 0;
			int idx = HitTest(tlht);
			if (tlht.iPart == TLIP_EXPANDBTN)// 命中展开按钮
			{
				ExpandItem(idx, TVE_TOGGLE);// 翻转展开状态
				Redraw();
			}
			else
			{
				if (m_bSingleSel)
				{
					const int idxOldSel = m_idxSel;
					const int idxOldFocus = m_idxFocus;
					m_idxSel = idx;
					if (idx >= 0)
					{
						m_idxFocus = idx;
						m_idxMark = idx;
						RedrawItem(idx);
					}
					if (idxOldSel >= 0 && idxOldSel != idx)
						RedrawItem(idxOldSel);
					if (m_bFocusIndicatorVisible && idxOldFocus >= 0 && idxOldFocus != idx)
						RedrawItem(idxOldFocus);
					UpdateWindow(hWnd);
				}
				else
				{
					if (!(wParam & (MK_CONTROL | MK_SHIFT)))// 若Ctrl和Shift都未按下，则清除所有项的选中
						for (auto e : m_Item)
							e->uFlags &= ~TLIF_SELECTED;

					const int idxOldFocus = m_idxFocus;
					if (idx >= 0)
					{
						m_idxFocus = idx;

						if ((wParam & MK_SHIFT) && m_idxMark >= 0)
						{
							const int idxBegin = std::min(idx, m_idxMark);
							const int idxEnd = std::max(idx, m_idxMark);
							OnlySelectRange(idxBegin, idxEnd);
							// Shift选择不修改mark
							// m_idxMark = idx;
							Redraw();
						}
						else if (wParam & MK_CONTROL)
						{
							m_idxMark = idx;
							m_Item[idx]->uFlags ^= TLIF_SELECTED;
							RedrawItem(idx);
							if (idxOldFocus >= 0 && idxOldFocus != idx)
								RedrawItem(idxOldFocus);
						}
						else
						{
							m_idxMark = idx;
							m_Item[idx]->uFlags |= TLIF_SELECTED;
							Redraw();
						}
					}
					UpdateWindow(hWnd);

					if (idx < 0 || tlht.iPart == TLIP_NONE)// 空白处
					{
						POINT pt{ tlht.pt };
						ClientToScreen(hWnd, &pt);
						if (IsMouseMovedBeforeDragging(hWnd, pt.x, pt.y))
						{
							m_idxHot = -1;// 清除热点
							if ((wParam & MK_CONTROL) && idx >= 0)// 如果按下了Ctrl那么把鼠标下的项目取消选中
								m_Item[idx]->uFlags &= ~TLIF_SELECTED;
							BeginDraggingSelect((UINT)wParam, tlht.pt.x, tlht.pt.y);
							return 0;
						}
					}

					Redraw();
					UpdateWindow(hWnd);
				}
			}
		}
		return 0;

		case WM_LBUTTONDBLCLK:
		{
			TLHITTEST tlht;
			tlht.pt = ECK_GET_PT_LPARAM(lParam);
			tlht.uFlags = TLHTF_NOHITTEXTLABEL;
			HitTest(tlht);
			if (tlht.iPart == TLIP_EXPANDBTN)
				PostMsg(WM_LBUTTONDOWN, wParam, lParam);// 连击修正
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

		case WM_CREATE:
		{
			m_iDpi = GetDpi(hWnd);
			UpdateDpiSize(m_Ds, m_iDpi);
			m_cyHeader = m_Ds.cyHeaderDef;
			m_cyItem = m_Ds.cyItemDef;
			m_DC.Create(hWnd);
			UpdateDCAttr();
			m_Header.Create(NULL, WS_CHILD | WS_VISIBLE | HDS_FULLDRAG | HDS_BUTTONS | HDS_DRAGDROP, 0,
				0, 0, ClientWidth, m_Ds.cyHeaderDef, hWnd, TL_IDC_HEADER);
			m_Header.SetExplorerTheme();
			SetExplorerTheme();
			m_hThemeTV = OpenThemeData(hWnd, L"TreeView");
			m_hThemeLV = OpenThemeData(hWnd, L"ListView");
			UpdateThemeInfo();
			m_crBranchLine = GetSysColor(COLOR_GRAYTEXT);

			SystemParametersInfoW(SPI_GETWHEELSCROLLLINES, 0, &m_cScrollLine, 0);

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
			m_hThemeLV = OpenThemeData(hWnd, L"ListView");
			UpdateThemeInfo();
			Redraw();
		}
		break;

		case WM_SYSCOLORCHANGE:
			SetTextColor(m_DC.GetDC(), GetSysColor(COLOR_WINDOWTEXT));
			m_crBranchLine = GetSysColor(COLOR_GRAYTEXT);
			Redraw();
			break;

		case WM_DESTROY:
		{
			CloseThemeData(m_hThemeTV);
			m_hThemeTV = NULL;
			CloseThemeData(m_hThemeLV);
			m_hThemeLV = NULL;
		}
		break;

		case WM_SETTINGCHANGE:
			if (wParam == SPI_GETWHEELSCROLLLINES)
				SystemParametersInfoW(SPI_GETWHEELSCROLLLINES, 0, &m_cScrollLine, 0);
			break;
		}
		return CWnd::OnMsg(hWnd, uMsg, wParam, lParam);
	}

	ECK_CWND_CREATE;
	HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, HMENU hMenu, PCVOID pData = NULL) override
	{
		return IntCreate(dwExStyle, WCN_TREELIST, pszText, dwStyle,
			x, y, cx, cy, hParent, hMenu, eck::g_hInstance, NULL);
	}

	/// <summary>
	/// 重新构建树关系
	/// </summary>
	void BuildTree()
	{
		m_Item.clear();

		if (m_bFlatMode)
		{
			NMTLFILLALLFLATITEM nm{};
			FillNmhdrAndSendNotify(nm, NM_TL_FILLALLFLATITEM);
			m_Item.resize(nm.cChildren);
			memcpy(m_Item.data(), nm.pChildren, nm.cChildren * sizeof(*nm.pChildren));
		}
		else
		{
			NMTLFILLCHILDREN nm{};
			nm.bQueryRoot = TRUE;
			FillNmhdrAndSendNotify(nm, NM_TL_FILLCHILDREN);
			EckCounter(nm.cTopItem, i)
			{
				const int idxCurrParent = (int)m_Item.size();
				NMTLFILLCHILDREN nm2{};
				nm2.pParent = nm.pChildren[i];
				nm2.pParent->idxParent = -1;
				FillNmhdrAndSendNotify(nm2, NM_TL_FILLCHILDREN);

				AddVirtualItem(nm2.pParent, nm2.pChildren, 1);
			}
		}

		CheckOldData();
		UpdateScrollBar();
		ReCalcTopItem();
	}

	int ItemFromY(int y) const
	{
		int idx;
		if (y > m_cyHeader)
			idx = m_idxTopItem + (y - m_cyHeader) / m_cyItem;
		else
			idx = m_idxTopItem + (y - m_cyHeader) / m_cyItem - 1;
		if (idx >= 0 && idx < (int)m_Item.size())
			return idx;
		else
			return -1;
	}

	int HitTest(TLHITTEST& tlht) const
	{
		if (m_vCol.empty() || tlht.pt.x < 0 || tlht.pt.x > m_cxItem ||
			tlht.pt.y < m_cyHeader || tlht.pt.y > m_cyClient)
			return -1;
		const int idx = ItemFromY(tlht.pt.y);
		if (idx < 0)
			return -1;

		if (!IsBitSet(tlht.uFlags, TLHTF_ONLYITEM))
		{
			tlht.iPart = TLIP_NONE;
			const auto it = std::find_if(m_vCol.begin(), m_vCol.end(), [&tlht](const COL& e)
				{
					return tlht.pt.x < e.iRight && tlht.pt.x > e.iLeft;
				});
			if (it == m_vCol.end())
				tlht.idxSubItemDisplay = -1;
			else
				tlht.idxSubItemDisplay = (int)std::distance(m_vCol.begin(), it);
			const int y = GetItemY(idx);
			const auto e = m_Item[idx];
			RECT rc;
			rc.left = m_dxContent + CalcIndentMinusGlyph(e);

			if (!m_bFlatMode && e->cChildren)
			{
				rc.right = std::min(rc.left + m_sizeTVGlyph.cx, (long)m_vCol.front().iRight);
				rc.top = y + (m_cyItem - m_sizeTVGlyph.cy) / 2;
				rc.bottom = rc.top + m_sizeTVGlyph.cy;
				if (PtInRect(&rc, tlht.pt))
				{
					tlht.iPart = TLIP_EXPANDBTN;
					goto EndPartTest;
				}
			}

			if (m_hImgList && e->idxImg >= 0)
			{
				rc.left += (m_bFlatMode ? 0 : m_sizeTVGlyph.cx);
				rc.top = y + (m_cyItem - m_cyImg) / 2;
				rc.right = std::min(rc.left + m_cxImg, (long)m_vCol.front().iRight);
				rc.bottom = rc.top + m_cyImg;
				if (PtInRect(&rc, tlht.pt))
				{
					tlht.iPart = TLIP_ICON;
					goto EndPartTest;
				}
			}

			if (tlht.idxSubItemDisplay >= 0 && !(tlht.uFlags & TLHTF_NOHITTEXTLABEL))
			{
				GetItemTextRect(idx, tlht.idxSubItemDisplay, rc);
				if (PtInRect(&rc, tlht.pt))
				{
					tlht.iPart = TLIP_TEXT;
					goto EndPartTest;
				}
			}
		EndPartTest:;
		}
		return idx;
	}

	EckInline int GetItemY(int idx) const
	{
		return m_cyHeader + (idx - m_idxTopItem) * m_cyItem;
	}

	EckInline void GetItemRect(int idx, RECT& rc) const
	{
		EckAssert(idx >= 0 && idx < (int)m_Item.size());
		rc =
		{
			m_dxContent,
			GetItemY(idx),
			m_cxItem + m_dxContent,
			GetItemY(idx) + m_cyItem
		};
	}

	void GetItemTextRect(int idx, int idxSubItemDisplay, RECT& rc) const
	{
		NMTLGETDISPINFO nm;
		nm.Item.pNode = m_Item[idx];
		nm.Item.uMask = TLIM_TEXT;
		nm.Item.idxSubItem = idxSubItemDisplay;
		FillNmhdrAndSendNotify(nm, NM_TL_GETDISPINFO);
		SIZE size;
		GetTextExtentPoint32W(m_DC.GetDC(), nm.Item.pszText, nm.Item.cchText, &size);
		rc.top = GetItemY(idx);
		rc.left = m_vCol[idxSubItemDisplay].iLeft + m_Ds.cxTextMargin + m_dxContent;
		if (!idxSubItemDisplay)
			rc.left += (nm.Item.pNode->iLevel * m_sizeTVGlyph.cx + m_cxImg);
		rc.right = rc.left + std::min(size.cx, (long)m_vCol[idxSubItemDisplay].iRight);
		rc.top = rc.top + (m_cyItem - size.cy) / 2;
		rc.bottom = rc.top + size.cy;
	}

	BOOL RedrawItem(int idx) const
	{
		RECT rc;
		GetItemRect(idx, rc);
		return Redraw(rc);
	}

	BOOL RedrawItem(int idxBegin, int idxEnd) const
	{
		RECT rc1, rc2;
		GetItemRect(idxBegin, rc1);
		GetItemRect(idxBegin, rc2);
		rc1.top = std::min(rc1.top, rc2.top);
		rc1.bottom = std::max(rc1.bottom, rc2.bottom);
		return Redraw(rc2);
	}

	EckInline CTLHeader& GetHeader() { return m_Header; }

	void ScrollV(int iDeltaLine)
	{
		if (!(GetStyle() & WS_VSCROLL))
			return;
		SCROLLINFO si;
		si.cbSize = sizeof(si);
		si.fMask = SIF_POS;
		GetSbInfo(SB_VERT, &si);
		const int iOld = si.nPos;
		si.nPos += iDeltaLine;
		SetSbInfo(SB_VERT, &si);
		GetSbInfo(SB_VERT, &si);
		ReCalcTopItem();
		RECT rc{ 0,m_cyHeader,m_cxClient,m_cyClient };
		ScrollWindowEx(m_hWnd, 0, (iOld - si.nPos) * m_cyItem, &rc, &rc, NULL, NULL, SW_INVALIDATE);
	}

	void ScrollH(int xDelta)
	{
		if (!(GetStyle() & WS_HSCROLL))
			return;
		SCROLLINFO si;
		si.cbSize = sizeof(si);
		si.fMask = SIF_POS;
		GetSbInfo(SB_HORZ, &si);
		si.nPos += xDelta;
		SetSbInfo(SB_HORZ, &si);
		GetSbInfo(SB_HORZ, &si);
		m_dxContent = -si.nPos;
		m_Header.Left = m_dxContent;
		Redraw();
	}

	HIMAGELIST SetImageList(HIMAGELIST hImgList)
	{
		std::swap(m_hImgList, hImgList);
		if (m_hImgList)
			ImageList_GetIconSize(m_hImgList, &m_cxImg, &m_cyImg);
		else
			m_cxImg = m_cyImg = 0;
		return hImgList;
	}

	EckInline HIMAGELIST GetImageList() const { return m_hImgList; }

	EckInline void SetDraggingSelectScrollGap(DWORD ms) { m_msDraggingSelScrollGap = ms; }

	EckInline int GetDraggingSelectScrollGap() const { return m_msDraggingSelScrollGap; }

	void SetSingleSelect(BOOL bSingleSel)
	{
		if (m_bSingleSel == bSingleSel)
			return;
		if (m_bSingleSel)
		{
			m_bSingleSel = FALSE;
			if (m_idxSel >= 0)
			{
				m_Item[m_idxSel]->uFlags |= TLIF_SELECTED;// 转换到多选数据
				m_idxSel = -1;
			}
		}
		else
		{
			m_bSingleSel = TRUE;
			m_idxSel = -1;
			int i = 0;
			for (; i < (int)m_Item.size(); ++i)
			{
				if (m_Item[i]->uFlags & TLIF_SELECTED)// 查找第一个选中项
				{
					m_idxSel = i;// 转换到单选数据
					break;
				}
			}

			for (; i < (int)m_Item.size(); ++i)
				m_Item[i]->uFlags &= ~TLIF_SELECTED;
		}
		Redraw();
	}

	EckInline BOOL GetSingleSelect() const { return m_bSingleSel; }

	EckInline void SetFlatMode(BOOL b) { m_bFlatMode = b; }

	EckInline BOOL GetFlatMode() const { return m_bFlatMode; }

	EckInline void SetWatermarkString(PCWSTR pszText) { m_rsWatermark = pszText; }

	EckInline const CRefStrW& GetWatermarkString() const { return m_rsWatermark; }

	EckInline void SetBackgroundNotSolid(BOOL b) { m_bBackgroundNotSolid = b; }

	EckInline BOOL GetBackgroundNotSolid() const { return m_bBackgroundNotSolid; }

	EckInline const COL* GetColumnPosInfo() const { return m_vCol.data(); }

	EckInline void DeselectAll()
	{
		EckAssert(m_bSingleSel == FALSE);
		for (auto e : m_Item)
			e->uFlags &= ~TLIF_SELECTED;
	}

	void DeselectRange(int idxBegin, int idxEnd)
	{
		if (m_bSingleSel && m_idxSel >= idxBegin && m_idxSel <= idxEnd)
		{
			m_idxSel = -1;
			return;
		}

		for (int i = idxBegin; i <= idxEnd; ++i)
			m_Item[i]->uFlags |= TLIF_SELECTED;
	}

	void DeselectChildren(int idxParent)
	{
		EckAssert(idxParent >= 0 && idxParent < (int)m_Item.size());

		int cChildren = m_Item[idxParent]->cChildren;
		for (int i = idxParent + 1; i < idxParent + 1 + cChildren; ++i)
		{
			const auto f = m_Item[i];
			if (!(f->uFlags & TLIF_CLOSED))
				cChildren += f->cChildren;
			f->uFlags &= ~TLIF_SELECTED;
		}
		CheckOldDataRange(idxParent + 1, idxParent + cChildren, idxParent);// 限位
	}

	void OnlySelectRange(int idxBegin, int idxEnd)
	{
		if (m_bSingleSel)
		{
			m_idxSel = idxBegin;
			return;
		}
		int i;
		// 清除前后选中
		for (i = 0; i < idxBegin; ++i)
			m_Item[i]->uFlags &= ~TLIF_SELECTED;
		for (i = idxEnd + 1; i < (int)m_Item.size(); ++i)
			m_Item[i]->uFlags &= ~TLIF_SELECTED;
		// 范围选中
		for (i = idxBegin; i <= idxEnd; ++i)
			m_Item[i]->uFlags |= TLIF_SELECTED;
	}

	EckInline void SelectItem(int idx, BOOL bSel)
	{
		EckAssert(idx >= 0 && idx < (int)m_Item.size());
		if (m_bSingleSel)
		{
			m_idxSel = idx;
			return;
		}
		if (bSel)
			m_Item[idx]->uFlags |= TLIF_SELECTED;
		else
			m_Item[idx]->uFlags &= ~TLIF_SELECTED;
	}

	/// <summary>
	/// 折叠项目
	/// </summary>
	/// <param name="idx">索引</param>
	/// <param name="iOp">操作，可选下列值：
	/// TVE_COLLAPSE - 折叠
	/// TVE_EXPAND - 展开
	/// TVE_TOGGLE - 反转折叠状态</param>
	void ExpandItem(int idx, int iOp)
	{
		EckAssert(idx >= 0 && idx < (int)m_Item.size());

		const auto e = m_Item[idx];
		switch (iOp)
		{
		case TVE_COLLAPSE:
			if (!(e->uFlags & TLIF_CLOSED))
				ExpandItem(idx, TVE_TOGGLE);
			return;
		case TVE_EXPAND:
			if (e->uFlags & TLIF_CLOSED)
				ExpandItem(idx, TVE_TOGGLE);
			return;
		case TVE_TOGGLE:
			break;
		default:
			EckDbgBreak();
			return;
		}

		e->uFlags ^= TLIF_CLOSED;
		if (e->uFlags & TLIF_CLOSED)// 若项目折叠，则清除所有子项的选中
			DeselectChildren(idx);

		NMTLTLNODEEXPANDED nm{};
		nm.pNode = e;
		FillNmhdrAndSendNotify(nm, NM_TL_NODEEXPANDED);
		BuildTree();
	}
};

LRESULT CTLHeader::OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lResult;
	switch (uMsg)
	{
	case HDM_INSERTITEMW:
		lResult = CHeader::OnMsg(hWnd, uMsg, wParam, lParam);
		if (lResult >= 0)
		{
			m_TL.m_vCol.emplace_back();
			m_TL.UpdateColumnInfo();
		}
		return lResult;

	case HDM_DELETEITEM:
		lResult = CHeader::OnMsg(hWnd, uMsg, wParam, lParam);
		if (lResult)
		{
			m_TL.m_vCol.pop_back();
			m_TL.UpdateColumnInfo();
		}
		return lResult;

	case HDM_SETORDERARRAY:
		lResult = CHeader::OnMsg(hWnd, uMsg, wParam, lParam);
		if (lResult)
			m_TL.UpdateColumnInfo();
		return lResult;
	}
	return CHeader::OnMsg(hWnd, uMsg, wParam, lParam);
}
ECK_NAMESPACE_END