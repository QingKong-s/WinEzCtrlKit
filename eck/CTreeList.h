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
enum :UINT
{
	TLIF_ROOT = (1u << 0),
	TLIF_CLOSED = (1u << 1),
	TLIF_SELECTED = (1u << 2),
	TLIF_HOTEXPANDBTN = (1u << 3),
};

enum :UINT
{
	TLIM_TEXT = (1u << 0),
	TLIM_IMAGE = (1u << 1),
	TLIM_CHILDREN = (1u << 2),

	TLIM_ALL = TLIM_TEXT | TLIM_IMAGE | TLIM_CHILDREN,
};

enum
{
	TLIP_NONE,// 命中空白处
	TLIP_EXPANDBTN,// 命中展开按钮
	TLIP_ICON,// 命中图标
	TLIP_TEXT,// 命中文本
};

enum :UINT
{
	TLHTF_ONLYITEM = (1u << 0),
	TLHTF_NOHITTEXTLABEL = (1u << 1),
};

struct TLNODE
{
	UINT uFlags;
	int cChildren;
	int iLevel;
	LPARAM lParam;
};

struct TLITEM
{
	UINT uMask;
	int idxSubItem;
	TLNODE* pNode;
	UINT uFlags;
	int cChildren;
	int cchText;
	int idxImg;
	PCWSTR pszText;
};

struct TLHITTEST
{
	POINT pt;
	int iPart;
	UINT uFlags;
	int idxSubItem;
};

struct NMTLFILLCHILDREN
{
	NMHDR nmhdr;
	TLNODE* pParent;
	TLNODE** pChildren;
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


constexpr inline int TL_IDC_HEADER = 101;

class CTreeList :public CWnd
{
private:
	struct COL
	{
		int iLeft;
		int iRight;
	};
	std::vector<TLNODE*> m_Item{};
	CHeader m_Header{};

	HTHEME m_hThemeTV = NULL;
	HTHEME m_hThemeLV = NULL;
	HFONT m_hFont = NULL;
	CEzCDC m_DC{};

	std::vector<COL> m_vCol{};
	int m_cxItem = 0;

	int m_cyItem = 0;
	int m_idxTopItem = 0;
	int m_cyHeader = 0;
	int m_cxClient = 0,
		m_cyClient = 0;
	SIZE m_sizeTVGlyph{};
	int m_idxHot = -1;
	int m_dxContent = 0;
	POINT m_ptDraggingSelStart{};
	RECT m_rcDraggingSel{};

	HIMAGELIST m_hImgList = NULL;
	int m_cxImg = 0,
		m_cyImg = 0;

	int m_cScrollLine = 3;
#ifdef _DEBUG
	BITBOOL m_bDbgDrawIndex : 1 = 0;
#endif
	BITBOOL m_bExpandBtnHot : 1 = FALSE;
	BITBOOL m_bDraggingSel : 1 = FALSE;

	int m_iDpi = USER_DEFAULT_SCREEN_DPI;
	ECK_DS_BEGIN(DPIS)
		ECK_DS_ENTRY(cyHeaderDef, 26)
		ECK_DS_ENTRY(cyItemDef, 24)
		ECK_DS_ENTRY(cxTextMargin, 4)
		;
	ECK_DS_END_VAR(m_Ds);


	/// <summary>
	/// 添加虚拟项目。
	/// 函数先将Parent尾插进列表，然后递归尾插其下所有子项
	/// </summary>
	/// <param name="Parent">当前父项目</param>
	/// <param name="plParam">子项目的lParam数组</param>
	void AddVirtualItem(TLNODE* pParent, TLNODE** pNode, int iLevel)
	{
		if (!IsBitSet(pParent->uFlags, TLIF_ROOT))
			m_Item.emplace_back(pParent);
		else
			pParent->iLevel = 0;

		if (!IsBitSet(pParent->uFlags, TLIF_CLOSED))
		{
			EckCounter(pParent->cChildren, i)
			{
				NMTLFILLCHILDREN nm{};
				nm.pParent = pNode[i];
				nm.pParent->iLevel = iLevel + 1;
				FillNmhdrAndSendNotify(nm, NM_TL_FILLCHILDREN);
				AddVirtualItem(nm.pParent, nm.pChildren, iLevel + 1);
			}
		}
	}

	void PaintItem(HDC hDC, int idx, BOOL bDrawDivider = FALSE, BOOL bFillBk = FALSE)
	{
		//---------------准备
		const int x = m_dxContent;
		const int y = (idx - m_idxTopItem) * m_cyItem + m_cyHeader;
		const auto& e = m_Item[idx];

		int iStateId;
		if (IsBitSet(e->uFlags, TLIF_SELECTED))
			if (idx == m_idxHot)
				iStateId = LISS_HOTSELECTED;
			else
				iStateId = LISS_SELECTED;
		else if (idx == m_idxHot)
			iStateId = LISS_HOT;
		else
			iStateId = 0;
		//---------------擦除
		RECT rcItem{ x,y,m_cxClient,y + m_cyItem };
		if (bFillBk)
			FillRect(hDC, &rcItem, GetSysColorBrush(COLOR_WINDOW));
		//---------------画列分隔线
		if (bDrawDivider)
			for (const auto& e : m_vCol)
				DrawListViewColumnDetail(m_hThemeLV, hDC, e.iRight + x, rcItem.top, rcItem.bottom);
		//---------------画背景
		rcItem.right = m_cxItem + x;
		if (iStateId)
			DrawThemeBackground(m_hThemeLV, hDC, LVP_LISTITEM, iStateId, &rcItem, NULL);
		//---------------画文本
		NMTLGETDISPINFO nm;
		nm.Item.pNode = e;
		nm.Item.uMask = TLIM_TEXT | TLIM_IMAGE;
		FillNmhdr(nm, NM_TL_GETDISPINFO);

		RECT rc{ x + m_Ds.cxTextMargin + (e->iLevel * m_sizeTVGlyph.cx) + m_cxImg,rcItem.top,0,rcItem.bottom };
		EckCounter(m_vCol.size(), i)
		{
			nm.Item.idxSubItem = i;
			nm.Item.cchText = 0;
			nm.Item.pszText = NULL;
			SendNotify(nm);

			rc.right = m_vCol[i].iRight + x;
			DrawTextW(hDC, nm.Item.pszText, nm.Item.cchText, &rc,
				DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX);
			rc.left = rc.right + m_Ds.cxTextMargin;
		}
		//---------------画图片
		HRGN hRgn = NULL;
		if (nm.Item.idxImg >= 0 || e->cChildren)
		{
			rcItem.left = 0;
			rcItem.right = m_vCol.front().iRight;
			hRgn = CreateRectRgnIndirect(&rcItem);
			SelectClipRgn(hDC, hRgn);
			DeleteObject(hRgn);
		}
		if (nm.Item.idxImg >= 0)
		{
			EckAssert(nm.Item.idxImg < ImageList_GetImageCount(m_hImgList));
			RECT rcImg;
			rcImg.left = x + (e->iLevel * m_sizeTVGlyph.cx);
			rcImg.top = y + (m_cyItem - m_cyImg) / 2;
			rcImg.right = rcImg.left + m_cxImg;
			rcImg.bottom = rcImg.top + m_cyImg;
			ImageList_Draw(m_hImgList, nm.Item.idxImg, hDC, rcImg.left, rcImg.top, ILD_NORMAL);
		}
		//---------------画折叠按钮
		if (e->cChildren)
		{
			rc.left = x + ((e->iLevel - 1) * m_sizeTVGlyph.cx);
			rc.right = rc.left + m_sizeTVGlyph.cx;
			rc.top += ((m_cyItem - m_sizeTVGlyph.cy) / 2);
			rc.bottom = rc.top + m_sizeTVGlyph.cy;
			DrawThemeBackground(m_hThemeTV, hDC,
				m_bExpandBtnHot ? TVP_HOTGLYPH : TVP_GLYPH,
				IsBitSet(e->uFlags, TLIF_CLOSED) ? GLPS_CLOSED : GLPS_OPENED,
				&rc, NULL);
		}
		if (hRgn)
			SelectClipRgn(hDC, NULL);
#ifdef _DEBUG
		if (m_bDbgDrawIndex)
		{
			const auto cr = SetTextColor(hDC, Colorref::Red);
			GetItemRect(idx, rcItem);
			DrawTextW(hDC, std::to_wstring(idx).c_str(), -1, &rcItem, DT_VCENTER | DT_SINGLELINE);
			SetTextColor(hDC, cr);
		}
#endif
	}

	void PaintBk(HDC hDC, const RECT& rc)
	{
		FillRect(hDC, &rc, GetSysColorBrush(COLOR_WINDOW));
	}

	void PaintDivider(HDC hDC, const RECT& rc)
	{
		for (const auto& e : m_vCol)
			DrawListViewColumnDetail(m_hThemeLV, hDC, e.iRight + m_dxContent,
				rc.top, rc.bottom);
	}

	void PaintDraggingSelRect(HDC hDC)
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

	void UpdateColumnInfo(int idxBegin)
	{
		int x;
		if (idxBegin == 0)
			x = 0;
		else
			x = m_vCol[idxBegin - 1].iRight;

		HDITEMW hdi;
		hdi.mask = HDI_WIDTH;
		for (int i = idxBegin; i < (int)m_vCol.size(); ++i)
		{
			m_Header.GetItem(i, &hdi);
			auto& e = m_vCol[i];
			e.iLeft = x;
			e.iRight = x + hdi.cxy;
			x += hdi.cxy;
		}
		m_cxItem = m_vCol.back().iRight;
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
		SetCapture(m_hWnd);
		m_ptDraggingSelStart = { xBegin - m_dxContent,yBegin + m_idxTopItem * m_cyItem };
		m_rcDraggingSel =
		{
			m_ptDraggingSelStart.x,
			m_ptDraggingSelStart.y,
			m_ptDraggingSelStart.x,
			m_ptDraggingSelStart.y
		};
		m_bDraggingSel = TRUE;
		RECT rcWnd;
		GetWindowRect(m_hWnd, &rcWnd);
		MSG msg;
		GetCursorPos(&msg.pt);
		while (GetCapture() == m_hWnd && IsWindow(m_hWnd))// 如果捕获改变或者当前窗口被销毁则应立即退出拖动循环
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

					int xDelta = 0, yDelta = 0;
					if (pt.x < 0)
						xDelta = pt.x;
					else if (pt.x > m_cxClient)
						xDelta = pt.x - m_cxClient;

					if (pt.y < 0)
						yDelta = pt.y / m_cyItem - 1;
					else if (pt.y > m_cyClient)
						yDelta = (pt.y - m_cyClient) / m_cyItem + 1;

					SetRedraw(FALSE);
					if (xDelta)
						ScrollH(xDelta);
					if (yDelta)
						ScrollV(yDelta);
					SetRedraw(TRUE);

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

					int idxBegin = ItemFromY(std::min(m_rcDraggingSel.top, rcOld.top));
					if (idxBegin < 0)
						idxBegin = 0;
					int idxEnd = ItemFromY(std::max(m_rcDraggingSel.bottom, rcOld.bottom));
					if (idxEnd < 0)
						idxEnd = (int)m_Item.size() - 1;
					RECT rcItem;
					//EckDbgPrintFmt(L"%d %d\n-------------", idxBegin, idxEnd);
					for (int i = idxBegin; i <= idxEnd; ++i)
					{
						GetItemRect(i, rcItem);
						const BOOL bIntersectOld = IsRectsIntersect(rcItem, rcOld);
						const BOOL bIntersectNew = IsRectsIntersect(rcItem, m_rcDraggingSel);
						//EckDbgPrintFmt(L"%d|%d %d", i, bIntersectOld, bIntersectNew);
						if (msg.wParam & MK_CONTROL)
						{
							if (bIntersectOld != bIntersectNew)
								m_Item[i]->uFlags ^= TLIF_SELECTED;// 翻转选中状态
						}
						else
						{
							if (bIntersectOld && !bIntersectNew)
								m_Item[i]->uFlags &= ~TLIF_SELECTED;// 先前选中但是现在未选中，清除选中状态
							else if (!bIntersectOld && bIntersectNew)
								m_Item[i]->uFlags |= TLIF_SELECTED;// 先前未选中但是现在选中，设置选中状态
							//else
							//{
							//	EckDbgPrintFmt(L"%d | %d %d %d %d | %d %d %d %d | %d %d %d %d", i,
							//		rcItem.left, rcItem.top, rcItem.right, rcItem.bottom,
							//		rcOld.left, rcOld.top, rcOld.right, rcOld.bottom,
							//		m_rcDraggingSel.left, m_rcDraggingSel.top, m_rcDraggingSel.right, m_rcDraggingSel.bottom
							//		);
							//}
						}
					}

					Redraw();
					UpdateWindow(m_hWnd);
					OffsetRect(&m_rcDraggingSel, -m_dxContent, m_idxTopItem * m_cyItem);
				}
				break;
				case WM_QUIT:
					PostQuitMessage(msg.wParam);// re-throw
					goto ExitDraggingLoop;
				default:
					TranslateMessage(&msg);
					DispatchMessageW(&msg);
					break;
				}
			}
			else
			{
				//if (!PtInRect(&rcWnd, msg.pt)
				//	&&
				//	((m_cxItem > m_cxClient) ||
				//		m_cyItem * (int)m_Item.size() > m_cyClient - m_cyHeader))
				//	;
				//	//SetCursorPos(msg.pt.x, msg.pt.y);// 保持触发WM_MOUSEMOVE
				//else
				WaitMessage();
			}
		}
	ExitDraggingLoop:
		ReleaseCapture();
		m_bDraggingSel = FALSE;
		Redraw(m_rcDraggingSel);
		m_rcDraggingSel = {};
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
					EckAssert(p->iItem >= 0 && p->iItem < (int)m_vCol.size());
					auto& e = m_vCol[p->iItem];
					const int iDelta = (p->pitem->cxy - (e.iRight - e.iLeft));
					e.iRight += iDelta;
					UpdateColumnInfo(p->iItem);
					UpdateScrollBar();
					SetWindowPos(m_Header.HWnd, NULL, m_dxContent, 0,
						std::max(m_cxItem, m_cxClient), m_cyHeader, SWP_NOZORDER | SWP_NOACTIVATE);
					Redraw();
				}
			}
			return FALSE;
			}
		}
		break;

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
			RECT rc{ 0,m_cyHeader,m_cxClient,m_cyClient };
			ScrollWindowEx(hWnd, 0, (iOld - si.nPos) * m_cyItem, &rc, &rc, NULL, NULL, SW_INVALIDATE);
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
			RECT rc{ 0,m_cyHeader,m_cxClient,m_cyClient };
			ScrollWindowEx(hWnd, 0, (iOld - si.nPos) * m_cyItem, &rc, &rc, NULL, NULL, SW_INVALIDATE);
			UpdateWindow(hWnd);
		}
		return 0;

		case WM_ERASEBKGND:
			return 0;

		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint(hWnd, &ps);
			//------------准备范围
			const int yOrg = ps.rcPaint.top;
			if (ps.rcPaint.top < m_cyHeader)// 永远不在表头控件区域内重画
				ps.rcPaint.top = m_cyHeader;
			const int idxTop = std::max(m_idxTopItem + (ps.rcPaint.top - m_cyHeader) / m_cyItem, 0L);
			const int idxBottom = std::min(m_idxTopItem + (ps.rcPaint.bottom - m_cyHeader) / m_cyItem,
				(long)m_Item.size() - 1);
			//------------擦除、分隔线
			PaintBk(m_DC.GetDC(), ps.rcPaint);
			PaintDivider(m_DC.GetDC(), ps.rcPaint);
			//------------画项目
			for (int i = idxTop; i <= idxBottom; ++i)
				PaintItem(m_DC.GetDC(), i);
			//------------画拖动选择矩形
			PaintDraggingSelRect(m_DC.GetDC());
			//------------
			BitBltPs(&ps, m_DC.GetDC());
			ps.rcPaint.top = yOrg;
			EndPaint(hWnd, &ps);
		}
		return 0;

		case WM_SIZE:
		{
			ECK_GET_SIZE_LPARAM(m_cxClient, m_cyClient, lParam);
			m_DC.ReSize(hWnd, m_cxClient, m_cyClient);
			SetBkMode(m_DC.GetDC(), TRANSPARENT);
			SelectObject(m_DC.GetDC(), m_hFont);

			m_Header.Size = SIZE{ std::max(m_cxItem, m_cxClient),m_cyHeader };
		}
		return 0;

		case WM_LBUTTONDOWN:
		{
			TLHITTEST tlht;
			tlht.pt = ECK_GET_PT_LPARAM(lParam);
			tlht.uFlags = TLHTF_NOHITTEXTLABEL;
			int idx = HitTest(tlht);
			if (tlht.iPart == TLIP_EXPANDBTN)
			{
				auto& e = m_Item[idx];
				if (IsBitSet(e->uFlags, TLIF_CLOSED))
					e->uFlags &= ~TLIF_CLOSED;
				else
					e->uFlags |= TLIF_CLOSED;

				NMTLTLNODEEXPANDED nm{};
				nm.pNode = e;
				FillNmhdrAndSendNotify(nm, NM_TL_NODEEXPANDED);
				BuildTree();
				Redraw();
			}
			else
			{
				if (!(wParam & (MK_CONTROL | MK_SHIFT)))// 若Ctrl和Shift都未按下，则清除所有项的选中
					for (auto& e : m_Item)
						e->uFlags &= ~TLIF_SELECTED;
				if (idx >= 0)
				{
					m_Item[idx]->uFlags |= TLIF_SELECTED;
				}
				else// 空白处
				{
					BeginDraggingSelect(wParam, tlht.pt.x, tlht.pt.y);
				}
				Redraw();
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
				PostMsg(WM_LBUTTONDOWN, wParam, lParam);
		}
		return 0;

		case WM_CREATE:
		{
			m_iDpi = GetDpi(hWnd);
			UpdateDpiSize(m_Ds, m_iDpi);
			m_cyHeader = m_Ds.cyHeaderDef;
			m_cyItem = m_Ds.cyItemDef;
			m_DC.Create(hWnd);
			m_Header.Create(NULL, WS_CHILD | WS_VISIBLE | HDS_FULLDRAG | HDS_BUTTONS, 0,
				0, 0, ClientWidth, m_Ds.cyHeaderDef, hWnd, TL_IDC_HEADER);
			m_Header.SetExplorerTheme();
			SetExplorerTheme();
			m_hThemeTV = OpenThemeData(hWnd, L"TreeView");
			m_hThemeLV = OpenThemeData(hWnd, L"ListView");
			UpdateThemeInfo();

			SystemParametersInfoW(SPI_GETWHEELSCROLLLINES, 0, &m_cScrollLine, 0);
		}
		return 0;

		case WM_SETFONT:
		{
			m_hFont = (HFONT)wParam;
			SelectObject(m_DC.GetDC(), m_hFont);
			if (lParam)
				Redraw();
		}
		break;

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
		return 0;

		case WM_DESTROY:
		{
			CloseThemeData(m_hThemeTV);
			m_hThemeTV = NULL;
			CloseThemeData(m_hThemeLV);
			m_hThemeLV = NULL;
		}
		return 0;

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
		NMTLFILLCHILDREN nm{};
		FillNmhdrAndSendNotify(nm, NM_TL_FILLCHILDREN);
		nm.pParent->uFlags |= TLIF_ROOT;
		AddVirtualItem(nm.pParent, nm.pChildren, 0);
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
				tlht.idxSubItem = -1;
			else
				tlht.idxSubItem = (int)std::distance(m_vCol.begin(), it);
			const int y = GetItemY(idx);
			const auto& e = m_Item[idx];
			RECT rc;
			rc.left = m_dxContent + (e->iLevel - 1) * m_sizeTVGlyph.cx;

			if (e->cChildren)
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

			if (m_hImgList)
			{
				rc.left += m_sizeTVGlyph.cx;
				rc.top = y + (m_cyItem - m_cyImg) / 2;
				rc.right = std::min(rc.left + m_cxImg, (long)m_vCol.front().iRight);
				rc.bottom = rc.top + m_cyImg;
				if (PtInRect(&rc, tlht.pt))
				{
					tlht.iPart = TLIP_ICON;
					goto EndPartTest;
				}
			}

			if (tlht.idxSubItem >= 0 && !(tlht.uFlags & TLHTF_NOHITTEXTLABEL))
			{
				GetItemTextRect(idx, tlht.idxSubItem, rc);
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

	void GetItemTextRect(int idx, int idxSubItem, RECT& rc) const
	{
		NMTLGETDISPINFO nm;
		nm.Item.pNode = m_Item[idx];
		nm.Item.uMask = TLIM_TEXT;
		nm.Item.idxSubItem = idxSubItem;
		FillNmhdrAndSendNotify(nm, NM_TL_GETDISPINFO);
		SIZE size;
		GetTextExtentPoint32W(m_DC.GetDC(), nm.Item.pszText, nm.Item.cchText, &size);
		rc.top = GetItemY(idx);
		rc.left = m_vCol[idxSubItem].iLeft + m_Ds.cxTextMargin;
		rc.right = rc.left + std::min(size.cx, (long)m_vCol[idxSubItem].iRight);
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
		RECT rc1, rc2, rc3;
		GetItemRect(idxBegin, rc1);
		GetItemRect(idxBegin, rc2);
		rc1.top = std::min(rc1.top, rc2.top);
		rc1.bottom = std::max(rc1.bottom, rc2.bottom);
		return Redraw(rc2);
	}

	EckInline CHeader& GetHeader() { return m_Header; }

	EckInline int InsertColumn(PCWSTR pszText, int idx = -1, int cxItem = -1,
		int idxImage = -1, int iFmt = HDF_LEFT, LPARAM lParam = 0)
	{
		idx = m_Header.InsertItem(pszText, idx, cxItem, idxImage, iFmt, lParam);
		m_vCol.emplace_back();
		UpdateColumnInfo(idx);
		return idx;
	}

	void ScrollV(int iDeltaLine)
	{
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
};
ECK_NAMESPACE_END