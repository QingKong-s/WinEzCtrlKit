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
};

struct TLITEM
{
	UINT uFlags;
	int cChildren;
	int idxSubItem;
	int cchText;
	PCWSTR pszText;
	LPARAM lParam;
};

struct NMTLFILLCHILDREN
{
	NMHDR nmhdr;
	TLITEM ItemParent;
	LPARAM* plParam;
};

struct NMTLGETDISPINFO
{
	NMHDR nmhdr;
	TLITEM Item;
};

constexpr inline int TL_IDC_HEADER = 101;

class CTreeList :public CWnd
{
private:
	struct NODE
	{
		UINT uFlags;
		int cChildren;
		LPARAM lParam;
		int iLevel;
	};

	std::vector<NODE> m_Item{};
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
	void AddVirtualItem(const TLITEM& Parent, const LPARAM* plParam, int iLevel)
	{
		if (!IsBitSet(Parent.uFlags, TLIF_ROOT))
			m_Item.emplace_back(Parent.uFlags, Parent.cChildren, Parent.lParam, iLevel);

		EckCounter(Parent.cChildren, i)
		{
			NMTLFILLCHILDREN nm{};
			nm.ItemParent.lParam = plParam[i];
			FillNmhdrAndSendNotify(nm, NM_TL_FILLCHILDREN);
			AddVirtualItem(nm.ItemParent, nm.plParam, iLevel + 1);
		}
	}

	void PaintItem(HDC hDC, int idx)
	{
		int x = 0;
		int y = idx * m_cyItem;
		const auto& e = m_Item[idx + m_idxTopItem];

		int iStateId;
		if (IsBitSet(e.uFlags, TLIF_SELECTED))
			if (idx + m_idxTopItem == m_idxHot)
				iStateId = LISS_HOTSELECTED;
			else
				iStateId = LISS_SELECTED;
		else if (idx + m_idxTopItem == m_idxHot)
			iStateId = LISS_HOT;
		else
			iStateId = 0;

		NMLVCUSTOMDRAW nmlvcd;
		nmlvcd.nmcd.rc = { x,idx * m_cyItem,m_cxClient,(idx + 1) * m_cyItem };
		nmlvcd.iPartId = LVP_LISTITEM;
		nmlvcd.iStateId = iStateId;
		nmlvcd.nmcd.hdc = hDC;

		const int cCol = m_Header.GetItemCount();
		const auto prcCol = (RECT*)_malloca((cCol + 1) * sizeof(RECT));
		EckAssert(prcCol);
		DrawListViewItemBackground(m_hThemeLV, LV_VIEW_DETAILS, &m_Header, &nmlvcd, cCol, prcCol);

		NMTLGETDISPINFO nm;
		nm.Item.lParam = e.lParam;
		FillNmhdr(nm, NM_TL_GETDISPINFO);

		x += (e.iLevel * m_sizeTVGlyph.cx);
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
		if (e.cChildren)
		{
			prcCol[0].left -= (m_sizeTVGlyph.cx + m_Ds.cxTextMargin);
			prcCol[0].right = prcCol[0].left + m_sizeTVGlyph.cx;
			prcCol[0].top += ((prcCol[0].bottom - prcCol[0].top - m_sizeTVGlyph.cy) / 2);
			prcCol[0].bottom = prcCol[0].top + m_sizeTVGlyph.cy;
			DrawThemeBackground(m_hThemeTV, hDC, TVP_GLYPH,
				IsBitSet(e.uFlags, TLIF_CLOSED) ? GLPS_CLOSED : GLPS_OPENED, prcCol, NULL);
		}
		_freea(prcCol);
	}
public:
	ECK_CWND_SINGLEOWNER;
	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		switch (uMsg)
		{
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint(hWnd, &ps);
			const int idxTop = std::max((ps.rcPaint.top - m_cyHeader) / m_cyItem, 0L);
			const int idxBottom = std::min((ps.rcPaint.bottom - m_cyHeader) / m_cyItem + 1, (long)m_Item.size() - 1);
			for (int i = idxTop; i <= idxBottom; ++i)
			{
				PaintItem(m_DC.GetDC(), i);
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
		}
		return 0;

		case WM_CREATE:
		{
			m_iDpi = GetDpi(hWnd);
			UpdateDpiSize(m_Ds, m_iDpi);
			m_cyHeader = m_Ds.cyHeaderDef;
			m_cyItem = m_Ds.cyItemDef;
			m_DC.Create(hWnd);
			m_Header.Create(NULL, WS_CHILD | WS_VISIBLE, 0,
				0, 0, ClientWidth, m_Ds.cyHeaderDef, hWnd, TL_IDC_HEADER);
			m_Header.SetExplorerTheme();
			m_Header.InsertItem(L"Test Col 1.", -1, 800);
			m_Header.InsertItem(L"Test Col 2.", -1, 800);
			SetExplorerTheme();
			m_hThemeTV = OpenThemeData(hWnd, L"TreeView");
			m_hThemeLV = OpenThemeData(hWnd, L"ListView");

			GetThemePartSize(m_hThemeTV, m_DC.GetDC(), TVP_GLYPH, GLPS_CLOSED, NULL, TS_TRUE, &m_sizeTVGlyph);
		}
		return 0;

		case WM_SETFONT:
			m_hFont = (HFONT)wParam;
			SelectObject(m_DC.GetDC(), m_hFont);
			if (lParam)
				Redraw();
			break;

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
		nm.ItemParent.uFlags = TLIF_ROOT;
		FillNmhdrAndSendNotify(nm, NM_TL_FILLCHILDREN);
		AddVirtualItem(nm.ItemParent, nm.plParam, 0);
	}
};
ECK_NAMESPACE_END