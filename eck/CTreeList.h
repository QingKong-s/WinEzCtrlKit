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

ECK_NAMESPACE_BEGIN
// 项目标志
enum :UINT
{
	TLIF_HASNOTSIBLING = (1u << 0),
	TLIF_CLOSED = (1u << 1),
	TLIF_SELECTED = (1u << 2),
	TLIF_DISABLED = (1u << 3),
	TLIF_INVISIBLE = (1u << 4),
	TLIF_HASCHILDREN = (1u << 5),
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
	USHORT uFlags = 0u;	// TLIF_标志
	short iLevel = 0u;	// 层次，根节点为1，此后逐层+1
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

struct NMTLPREEDIT
{
	NMHDR nmhdr;
	TLNODE* pNode;
	int idx;
	int idxSubItemDisplay;
	RECT rc;
};

struct NMTLDBCLICK
{
	NMHDR nmhdr;
	TLNODE* pNode;
	int idx;
	const TLHITTEST* pHitTest;
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

	std::vector<TLNODE*> m_vItem{};
	std::vector<COL> m_vCol{};// 按显示顺序排列的列信息

	CTLHeader m_Header{ *this };

	HTHEME m_hThemeTV = NULL;
	HTHEME m_hThemeLV = NULL;
	HFONT m_hFont = NULL;
	CEzCDC m_DC{};
	COLORREF m_crBranchLine = CLR_DEFAULT;

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
	BITBOOL m_bDbgDrawIndex : 1 = 0;				// 【调试】绘制项目索引
	BITBOOL m_bDbgDrawMarkItem : 1 = 0;				// 【调试】绘制mark项
#endif
	BITBOOL m_bExpandBtnHot : 1 = FALSE;			// 展开按钮是否点燃
	BITBOOL m_bDraggingSel : 1 = FALSE;				// 是否处于拖动选择状态
#ifdef _DEBUG
	BITBOOL m_bFocusIndicatorVisible : 1 = TRUE;	// 焦点指示器是否可见
#else
	BITBOOL m_bFocusIndicatorVisible : 1 = FALSE;
#endif
	BITBOOL m_bHasFocus : 1 = FALSE;				// 是否有焦点

	BITBOOL m_bFlatMode : 1 = FALSE;					// 平面列表模式
	BITBOOL m_bFlatListFilter : 1 = FALSE;				// 平面列表模式下是否有不可见项目
	BITBOOL m_bSingleSel : 1 = FALSE;					// 单选
	BITBOOL m_bDisallowBeginDragInItemSpace : 1 = FALSE;// 禁止在项目内的空白区启动拖动选择
	BITBOOL m_bBackgroundNotSolid : 1 = FALSE;			// 背景是否不是纯色
	BITBOOL m_bHasLines : 1 = FALSE;					// 是否有分支线

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
	void AddVirtualItem(TLNODE* pParent, TLNODE** pNode, int cChildren, int iLevel)
	{
		pParent->idxLastEnd = -1;// 清除脏数据
		int idxCurrParent;
		if (!(pParent->uFlags & TLIF_INVISIBLE))
		{
			m_vItem.emplace_back(pParent);
			idxCurrParent = (iLevel == 0 ? -1 : (int)m_vItem.size() - 1);
		}
		else
		{
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

	void PaintItem(HDC hDC, int idx)
	{
		//---------------准备
		const int x = m_dxContent;
		const int y = (idx - m_idxTopItem) * m_cyItem + m_cyHeader;
		const auto e = m_vItem[idx];

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
		if (idx == m_idxFocus && m_bFocusIndicatorVisible)
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

		// 按显示顺序循环
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
		if (e->idxImg >= 0 || (e->uFlags & TLIF_HASCHILDREN))// 需要设置剪辑区
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

		if (!m_bFlatMode && (e->uFlags & TLIF_HASCHILDREN))
		{
			rc.left = x + cxIndent - m_sizeTVGlyph.cx;
			rc.right = rc.left + m_sizeTVGlyph.cx;
			rc.top += ((m_cyItem - m_sizeTVGlyph.cy) / 2);
			rc.bottom = rc.top + m_sizeTVGlyph.cy;
			DrawThemeBackground(m_hThemeTV, hDC, nmcd.iPartIdGlyph, nmcd.iStateIdGlyph, &rc, NULL);
		}
		//---------------画分支线
		if (m_bHasLines && !m_bFlatMode)
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
					if (b)
						if ((f->uFlags & TLIF_HASCHILDREN))
						{
							if (f->uFlags & TLIF_CLOSED)
								b = bLastItem;
						}

					if (bLastItem = bLastItem && b)
					{
						const int yBottom = rcItem.top + m_cyItem * 4 / 5;
						MoveToEx(hDC, xLine, rcItem.top, NULL);
						LineTo(hDC, xLine, yBottom);
						LineTo(hDC, xLine + m_sizeTVGlyph.cx / 2, yBottom);
					}
					else
					{
						MoveToEx(hDC, xLine, rcItem.top, NULL);
						LineTo(hDC, xLine, rcItem.bottom);
					}
					idxParent = m_vItem[idxParent]->idxParent;
				}
			}
			SelectObject(hDC, hOldPen);
		}

		if (hRgn)
			SelectClipRgn(hDC, NULL);
	End:;
#ifdef _DEBUG
		const auto cr = SetTextColor(hDC, Colorref::Red);
		if (m_bDbgDrawIndex)
			DrawTextW(hDC, std::to_wstring(idx).c_str(), -1, (RECT*)&rcItem,
				DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX | DT_NOCLIP);

		if (m_bDbgDrawMarkItem && idx == m_idxMark)
		{
			rcItem.left += x;
			rcItem.right = m_vCol.front().iRight + x;
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
			SetSbInfo(SB_VERT, &si);
			SetSbInfo(SB_HORZ, &si);
		}

		si.fMask = SIF_POS;
		GetSbInfo(SB_VERT, &si);
		si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
		si.nMin = 0;
		si.nMax = (int)m_vItem.size() - 1;
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
		EckAssert(!m_bDraggingSel);
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
					POINT pt ECK_GET_PT_LPARAM(msg.lParam);
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

					if (rcOld == m_rcDraggingSel)// 范围未变，退出
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
			int idxBottom = m_idxTopItem + (m_cyClient - m_cyHeader) / m_cyItem - 1;
			if (idxBottom >= (int)m_vItem.size())
				idxBottom = (int)m_vItem.size() - 1;
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
			case VK_LEFT:// 如果当前焦点项目已展开，折叠之，否则移至父项目
			{
				if (m_idxFocus >= 0)
				{
					const auto e = m_vItem[m_idxFocus];
					if ((e->uFlags & TLIF_HASCHILDREN) && !(e->uFlags & TLIF_CLOSED))// 已展开
					{
						ExpandItem(m_idxFocus, TVE_TOGGLE);
						if (m_bSingleSel)
							SelectItemForClick(m_idxFocus, 0);
						else
						{
							DeselectAll();
							SelectItemForClick(m_idxFocus, TRUE);
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
							SelectItemForClick(e->idxParent, 0);
						else
						{
							DeselectAll();
							SelectItemForClick(e->idxParent, TRUE);
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
					const auto e = m_vItem[m_idxFocus];
					if ((e->uFlags & TLIF_HASCHILDREN) && (e->uFlags & TLIF_CLOSED))// 已折叠
					{
						ExpandItem(m_idxFocus, TVE_TOGGLE);
						if (m_bSingleSel)
							SelectItemForClick(m_idxFocus, 0);
						else
						{
							DeselectAll();
							SelectItemForClick(m_idxFocus, TRUE);
						}
						RECT rc;
						GetItemRect(m_idxFocus, rc);
						rc.right = m_cxClient;
						rc.bottom = m_cyClient;
						Redraw(rc);
					}
					else if ((e->uFlags & TLIF_HASCHILDREN))// 有子项目
					{
						if (m_bSingleSel)
							SelectItemForClick(m_idxFocus + 1, 0);
						else
						{
							DeselectAll();
							SelectItemForClick(m_idxFocus + 1, TRUE);
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
								SelectItemForClick(m_idxFocus, 0);
							else// 多选
							{
								if (m_idxMark >= 0)// mark有效，选择范围
									OnlySelectRange(std::min(m_idxFocus, m_idxMark), std::max(m_idxFocus, m_idxMark));
								else// mark无效，只选中当前项
								{
									DeselectAll();
									SelectItemForClick(m_idxFocus, TRUE);
								}
								Redraw();
							}
						}
						else// Shift未按下
						{
							if (m_bSingleSel)
								SelectItemForClick(m_idxFocus, 0);
							else
							{
								DeselectAll();
								SelectItemForClick(m_idxFocus, TRUE);
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
					SelectItemForClick(idx, TRUE);
					UpdateWindow(hWnd);
				}
				else
				{
					if (!(wParam & (MK_CONTROL | MK_SHIFT)))// 若Ctrl和Shift都未按下，则清除所有项的选中
						DeselectAll();
					if (idx >= 0)
					{
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
							ToggleSelectItem(idx);
						else
							SelectItemForClick(idx, TRUE);
					}
					Redraw();
					UpdateWindow(hWnd);

					if (idx < 0 || tlht.iPart == TLIP_NONE)// 空白处
					{
						POINT pt{ tlht.pt };
						ClientToScreen(hWnd, &pt);
						if (IsMouseMovedBeforeDragging(hWnd, pt.x, pt.y))
						{
							m_idxHot = -1;// 清除热点
							if ((wParam & MK_CONTROL) && idx >= 0)// 如果按下了Ctrl那么把鼠标下的项目取消选中
								m_vItem[idx]->uFlags &= ~TLIF_SELECTED;
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
			m_bHasFocus = (GetFocus() == hWnd);
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
			Redraw();
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
	EckInline void BuildTree()
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
					nm.pChildren[nm.cChildren - i - 1]->uFlags |= TLIF_HASNOTSIBLING;
					b = TRUE;
				}

				FillNmhdrAndSendNotify(nm2, NM_TL_FILLCHILDREN);
				AddVirtualItem(nm2.pParent, nm2.pChildren, nm2.cChildren, 1);
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
		if (idx >= 0 && idx < (int)m_vItem.size())
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
			const auto e = m_vItem[idx];
			RECT rc;
			rc.left = m_dxContent + CalcIndentMinusGlyph(e);

			if (!m_bFlatMode && (e->uFlags & TLIF_HASCHILDREN))
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
		EckAssert(idx >= 0 && idx < (int)m_vItem.size());
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
		nm.Item.pNode = m_vItem[idx];
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
		m_idxSel = -1;
		for (auto e : m_vItem)
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
			m_vItem[i]->uFlags |= TLIF_SELECTED;
	}

	int DeselectChildren(int idxParent)
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
			m_vItem[i]->uFlags &= ~TLIF_SELECTED;
		for (i = idxEnd + 1; i < (int)m_vItem.size(); ++i)
			m_vItem[i]->uFlags &= ~TLIF_SELECTED;
		// 范围选中
		for (i = idxBegin; i <= idxEnd; ++i)
			if (!(m_vItem[i]->uFlags & TLIF_DISABLED))
				m_vItem[i]->uFlags |= TLIF_SELECTED;
	}

	// 选中项目，同步修改焦点和mark，多选时不会清除其他项目的选中，也不会重画
	void SelectItemForClick(int idx, BOOL bSel)
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
				m_idxFocus = idx;
				m_idxMark = idx;
			}
			else
				DeselectAll();
		}
	}

	EckInline void ToggleSelectItem(int idx)
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

	EckInline const std::vector<TLNODE*>& GetItems() const { return m_vItem; }

	EckInline TLNODE* GetItem(int idx) const
	{
		EckAssert(idx >= 0 && idx < (int)m_vItem.size());
		return m_vItem[idx];
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
		EckAssert(idx >= 0 && idx < (int)m_vItem.size());

		const auto e = m_vItem[idx];
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
		{
			const int cChildren = DeselectChildren(idx);
			CheckOldDataRange(idx + 1, idx + cChildren, idx);// 限位
		}
		NMTLTLNODEEXPANDED nm{};
		nm.pNode = e;
		FillNmhdrAndSendNotify(nm, NM_TL_NODEEXPANDED);

		BuildTree();
	}

	EckInline void SetHasLines(BOOL b) { m_bHasLines = b; }

	EckInline BOOL GetHasLines() const { return m_bHasLines; }

	EckInline void SetLineColor(COLORREF cr) { m_crBranchLine = cr; }

	EckInline COLORREF GetLineColor() const { return m_crBranchLine; }

	EckInline TLNODE* IndexToNode(int idx)
	{
		EckAssert(idx >= 0 && idx < (int)m_vItem.size());
		return m_vItem[idx];
	}

	EckInline void SetUseFilterInFlatMode(BOOL b) { m_bFlatListFilter = b; }

	EckInline BOOL GetUseFilterInFlatMode() const { return m_bFlatListFilter; }

	EckInline void SetDisallowBeginDragInItemSpace(BOOL b) { m_bDisallowBeginDragInItemSpace = b; }

	EckInline BOOL GetDisallowBeginDragInItemSpace() const { return m_bDisallowBeginDragInItemSpace; }
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