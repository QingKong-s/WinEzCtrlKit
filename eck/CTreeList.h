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

enum
{
	TLIP_NONE,
	TLIP_EXPANDBTN,
	TLIP_ICON,
};

enum :UINT
{
	TLHTF_SIMPLE = (1u << 0),
};

struct TLNODE
{
	UINT uFlags;
	int cChildren;
	LPARAM lParam;
	int iLevel;
};

struct TLITEM
{
	UINT uFlags;
	int cChildren;
	int idxSubItem;
	int cchText;
	PCWSTR pszText;
	TLNODE* pNode;
};

struct TLHITTEST
{
	POINT pt;
	int iPart;
	UINT uFlags;
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
	std::vector<TLNODE*> m_Item{};
	CHeader m_Header{};

	HTHEME m_hThemeTV = NULL;
	HTHEME m_hThemeLV = NULL;
	HFONT m_hFont = NULL;
	CEzCDC m_DC{};


	int m_cyItem = 0;
	int m_idxTopItem = 0;
	int m_cyHeader = 0;
	int m_cxClient = 0,
		m_cyClient = 0;
	SIZE m_sizeTVGlyph{};
	int m_idxHot = -1;
	int m_dxContent = 0;
	int m_rightCol = 0;

	BITBOOL m_bExpandBtnHot : 1 = FALSE;

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

	void PaintItem(HDC hDC, int idx)
	{
		int x = 0;
		int y = (idx - m_idxTopItem) * m_cyItem + m_cyHeader;
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

		NMLVCUSTOMDRAW nmlvcd;
		nmlvcd.nmcd.rc = { x,y,m_cxClient,y + m_cyItem };
		nmlvcd.iPartId = LVP_LISTITEM;
		nmlvcd.iStateId = iStateId;
		nmlvcd.nmcd.hdc = hDC;

		const int cCol = m_Header.GetItemCount();
		const auto prcCol = (RECT*)_malloca((cCol + 1) * sizeof(RECT));
		EckAssert(prcCol);
		DrawListViewItemBackground(m_hThemeLV, LV_VIEW_DETAILS, &m_Header, &nmlvcd, cCol, prcCol);

		NMTLGETDISPINFO nm;
		nm.Item.pNode = e;
		FillNmhdr(nm, NM_TL_GETDISPINFO);

		x += (e->iLevel * m_sizeTVGlyph.cx);
		prcCol[0].left = x;
		EckCounter(cCol, i)
		{
			nm.Item.idxSubItem = i;
			nm.Item.cchText = 0;
			nm.Item.pszText = NULL;
			SendNotify(nm);
			prcCol[i].left += m_Ds.cxTextMargin;
			prcCol[i].top = nmlvcd.nmcd.rc.top;
			prcCol[i].bottom = nmlvcd.nmcd.rc.bottom;
			DrawTextW(hDC, nm.Item.pszText, nm.Item.cchText, prcCol + i,
				DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX);
		}
		if (e->cChildren)
		{
			prcCol[0].left -= (m_sizeTVGlyph.cx + m_Ds.cxTextMargin);
			prcCol[0].right = prcCol[0].left + m_sizeTVGlyph.cx;
			prcCol[0].top += ((prcCol[0].bottom - prcCol[0].top - m_sizeTVGlyph.cy) / 2);
			prcCol[0].bottom = prcCol[0].top + m_sizeTVGlyph.cy;
			DrawThemeBackground(m_hThemeTV, hDC, 
				m_bExpandBtnHot ? TVP_HOTGLYPH : TVP_GLYPH,
				IsBitSet(e->uFlags, TLIF_CLOSED) ? GLPS_CLOSED : GLPS_OPENED, 
				prcCol, NULL);
		}
		_freea(prcCol);
	}

	void ReCalcTopItem()
	{
		m_idxTopItem = GetSbPos(SB_VERT);
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

		case WM_MOUSEWHEEL:
		{
			SCROLLINFO si;
			si.cbSize = sizeof(SCROLLINFO);
			si.fMask = SIF_POS;
			GetSbInfo(SB_VERT, &si);
			const int iOld = si.nPos;
			si.nPos += (-GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA * 3);
			SetSbInfo(SB_VERT, &si);
			GetSbInfo(SB_VERT, &si);
			ReCalcTopItem();
			RECT rc{ 0,m_cyHeader,m_cxClient,m_cyClient };
			ScrollWindowEx(hWnd, 0, (iOld - si.nPos) * m_cyItem, &rc, &rc, NULL, NULL, SW_INVALIDATE);
			UpdateWindow(hWnd);
		}
		return 0;

		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint(hWnd, &ps);
			const int idxTop = std::max(m_idxTopItem + (ps.rcPaint.top - m_cyHeader) / m_cyItem, 0L);
			const int idxBottom = std::min(m_idxTopItem + (ps.rcPaint.bottom - m_cyHeader) / m_cyItem,
				(long)m_Item.size() - 1);
			for (int i = idxTop; i <= idxBottom; ++i)
				PaintItem(m_DC.GetDC(), i);
			if (idxBottom == (int)m_Item.size() - 1)
			{
				RECT rcPadding;
				GetItemRect(idxBottom, rcPadding);
				rcPadding.top = rcPadding.bottom;
				rcPadding.left = 0;
				rcPadding.right = m_cxClient;
				rcPadding.bottom = m_cyClient;
				FillRect(m_DC.GetDC(), &rcPadding, GetSysColorBrush(COLOR_WINDOW));
			}
			BitBltPs(&ps, m_DC.GetDC());
			EndPaint(hWnd, &ps);
		}
		return 0;

		case WM_SIZE:
		{
			ECK_GET_SIZE_LPARAM(m_cxClient, m_cyClient, lParam);
			m_DC.ReSize(hWnd, m_cxClient, m_cyClient);
			SetBkMode(m_DC.GetDC(), TRANSPARENT);
			SelectObject(m_DC.GetDC(), m_hFont);

			m_Header.Size = SIZE{ m_cxClient,m_cyHeader };
		}
		return 0;

		case WM_LBUTTONDOWN:
		{
			TLHITTEST tlht;
			tlht.pt = ECK_GET_PT_LPARAM(lParam);
			tlht.uFlags = 0;
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
				for (auto& e : m_Item)
					e->uFlags &= ~TLIF_SELECTED;
				if (idx >= 0)
				{
					m_Item[idx]->uFlags |= TLIF_SELECTED;
				}
				else
				{
				}
				Redraw();
			}
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
			m_Header.InsertItem(L"HWND", -1, 360);
			m_Header.InsertItem(L"szClsName", -1, 360);
			m_Header.InsertItem(L"szText", -1, 400);
			SetExplorerTheme();
			m_hThemeTV = OpenThemeData(hWnd, L"TreeView");
			EckDbgPrintFormatMessage(GetLastError());
			m_hThemeLV = OpenThemeData(hWnd, L"ListView");

			GetThemePartSize(m_hThemeTV, m_DC.GetDC(), TVP_GLYPH, GLPS_CLOSED, NULL, TS_TRUE, &m_sizeTVGlyph);
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

		case WM_THEMECHANGED:
			CloseThemeData(m_hThemeTV);
			m_hThemeTV = OpenThemeData(hWnd, L"TreeView");
			CloseThemeData(m_hThemeLV);
			m_hThemeLV = OpenThemeData(hWnd, L"ListView");
			return 0;

		case WM_DESTROY:
		{

		}
		return 0;
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
		SCROLLINFO si;
		si.fMask = SIF_POS;
		GetSbInfo(SB_VERT, &si);

		si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
		si.nMin = 0;
		si.nMax = (int)m_Item.size() - 1;
		si.nPage = (m_cyClient - m_cyHeader) / m_cyItem;
		SetSbInfo(SB_VERT, &si);
		ReCalcTopItem();
	}

	int HitTest(TLHITTEST& tlht) const
	{
		if (tlht.pt.x < 0 || tlht.pt.x > m_cxClient ||
			tlht.pt.y < m_cyHeader || tlht.pt.y > m_cyClient)
			return -1;
		int idx = m_idxTopItem + ((tlht.pt.y - m_cyHeader) / m_cyItem);
		if (idx < (int)m_Item.size())
		{
			if (!IsBitSet(tlht.uFlags, TLHTF_SIMPLE))
			{
				const auto& e = m_Item[idx];
				const RECT rc
				{
					(e->iLevel - 1) * m_sizeTVGlyph.cx - m_dxContent,
					m_cyHeader + (idx - m_idxTopItem) * m_cyItem,
					e->iLevel * m_sizeTVGlyph.cx - m_dxContent,
					m_cyHeader + (idx - m_idxTopItem + 1) * m_cyItem
				};
				if (PtInRect(&rc, tlht.pt))
					tlht.iPart = TLIP_EXPANDBTN;
			}
			return  idx;
		}
		else
			return -1;
	}

	void GetItemRect(int idx, RECT& rc) const
	{
		EckAssert(idx >= 0 && idx < (int)m_Item.size());
		rc =
		{
			0,
			m_cyHeader + (idx - m_idxTopItem) * m_cyItem,
			m_cxClient,
			m_cyHeader + (idx - m_idxTopItem + 1) * m_cyItem
		};
	}

	BOOL RedrawItem(int idx) const
	{
		RECT rc;
		GetItemRect(idx, rc);
		return Redraw(rc);
	}
};
ECK_NAMESPACE_END