/*
* WinEzCtrlKit Library
*
* CListBoxNew.h ： 所有者数据模式的列表框
*
* Copyright(C) 2023 QingKong
*/
#pragma once
#include "CWnd.h"
#include "GdiplusFlatDef.h"
#include "CScrollBar.h"
#include "StylePainter.h"

#include <functional>

ECK_NAMESPACE_BEGIN
struct LBNEWITEM
{
	PCWSTR pszText;
	int cchText;
	int idxItem;
};


class CListBoxNew :public CWnd
{
public:
	enum NCode
	{
		GetDispInfo,
	};

	using FLbnProc = LRESULT(*)(HWND hWnd, UINT uCode, LPARAM lParam, LPARAM lRefData);
private:
	struct ITEM
	{
		CRefStrW rsText{};
		union
		{
			struct
			{
				BITBOOL bSel : 1;
			};
			UINT uFlags = 0u;
		};
		
	};

	CEzCDC m_DC{};
	HFONT m_hFont = NULL;// 不需要释放

	CWin8UxThemePainter m_StylePainter{};

	int m_cxClient = 0,
		m_cyClient = 0;

	int m_cyItem = 24;

	int m_idxSel = -1;
	int m_idxHot = -1;
	int m_idxTop = -1;
	int m_oyTop = 0;

	std::vector<ITEM> m_vItem{};

	FLbnProc m_pfnProc = NULL;
	LPARAM m_lRefData = 0;

	CScrollBarWndH m_SBH{};
	CScrollBarWndV m_SBV{};

	union
	{
		struct
		{
			BITBOOL m_FbMultiSel : 1;
		};
		UINT m_uFlags = 0u;
	};

	int m_iDpi = USER_DEFAULT_SCREEN_DPI;

	ECK_DS_BEGIN(DPIS)
		ECK_DS_ENTRY(cyItemDef, 24)
		ECK_DS_ENTRY(cxSelBorder, 1)
		;
	ECK_DS_END_VAR(m_Ds);


	static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		auto p = (CListBoxNew*)GetWindowLongPtrW(hWnd, 0);
		switch (uMsg)
		{
		case WM_MOUSELEAVE:
			return HANDLE_WM_MOUSELEAVE(hWnd, wParam, lParam, p->OnMouseLeave);
		case WM_MOUSEMOVE:
			return HANDLE_WM_MOUSEMOVE(hWnd, wParam, lParam, p->OnMouseMove);
		case WM_SIZE:
			return HANDLE_WM_SIZE(hWnd, wParam, lParam, p->OnSize);
		case WM_PAINT:
			return HANDLE_WM_PAINT(hWnd, wParam, lParam, p->OnPaint);
		case WM_VSCROLL:
			return HANDLE_WM_VSCROLL(hWnd, wParam, lParam, p->OnVScroll);
		case WM_MOUSEWHEEL:
			return HANDLE_WM_MOUSEWHEEL(hWnd, wParam, lParam, p->OnMouseWheel);
		case WM_LBUTTONDOWN:
			return HANDLE_WM_LBUTTONDOWN(hWnd, wParam, lParam, p->OnLButtonDown);
		case WM_SETFONT:
			p->m_hFont = (HFONT)wParam;
			SelectObject(p->m_DC.GetDC(), p->m_hFont);
			if (lParam)
				p->Redraw();
			return 0;
		case WM_GETFONT:
			return (LRESULT)p->m_hFont;
		case WM_NCCREATE:
		{
			p = (CListBoxNew*)((CREATESTRUCTW*)lParam)->lpCreateParams;
			SetWindowLongPtrW(hWnd, 0, (LONG_PTR)p);
		}
		break;
		case WM_CREATE:
			return HANDLE_WM_CREATE(hWnd, wParam, lParam, p->OnCreate);
		}

		return DefWindowProcW(hWnd, uMsg, wParam, lParam);
	}

	BOOL OnCreate(HWND hWnd, CREATESTRUCTW* pcs)
	{
		m_iDpi = GetDpi(hWnd);
		UpdateDpiSize(m_Ds, m_iDpi);
		m_cyItem = m_Ds.cyItemDef;

		m_DC.Create(hWnd);
		SetBkMode(m_DC.GetDC(), TRANSPARENT);

		m_SBH.Manage(eck::CWnd::ManageOp::Attach, hWnd);
		m_SBV.Manage(eck::CWnd::ManageOp::Attach, hWnd);
		return TRUE;
	}

	void OnSize(HWND hWnd, UINT uState, int cx, int cy)
	{
		m_cxClient = cx;
		m_cyClient = cy;
		m_SBV.SetPage(cy);
		m_DC.ReSize(hWnd, cx, cy);
	}

	void OnPaint(HWND hWnd)
	{
		PAINTSTRUCT ps;
		BeginPaint(hWnd, &ps);
		const int idxTop = std::max(m_idxTop + (int)ps.rcPaint.top / m_cyItem - 1, m_idxTop);
		const int idxBottom = std::min(m_idxTop + (int)ps.rcPaint.bottom / m_cyItem + 1, (int)m_vItem.size() - 1);
		RECT rc;
		GetItemRect(idxTop, rc);
		for (int i = idxTop; i <= idxBottom; ++i)
		{
			RedrawItem(i, rc);
			rc.top += m_cyItem;
			rc.bottom += m_cyItem;
		}
		BitBltPs(&ps, m_DC.GetDC());
		EndPaint(hWnd, &ps);
	}

	void OnVScroll(HWND hWnd, HWND hCtrl, UINT uCode, int iPos)
	{
		SCROLLINFO si;
		si.cbSize = sizeof(si);
		si.fMask = SIF_ALL;
		m_SBV.GetInfo(&si);
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
		m_SBV.SetInfo(&si);
		m_SBV.GetInfo(&si);
		ReCalcTopItem();
		ScrollWindow(hWnd, 0, yOld - si.nPos, NULL, NULL);
		UpdateWindow(hWnd);
	}

	void OnMouseMove(HWND hWnd, int x, int y, UINT uKeyFlags)
	{
		int idxHot = HitTest(x, y);
		if (idxHot == m_idxHot)
			return;
		std::swap(idxHot, m_idxHot);
		RECT rcItem;
		if (idxHot >= 0)
		{
			GetItemRect(idxHot, rcItem);
			Redraw(rcItem);
		}
		if (m_idxHot >= 0)
		{
			GetItemRect(m_idxHot, rcItem);
			Redraw(rcItem);
		}

		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(TRACKMOUSEEVENT);
		tme.dwFlags = TME_LEAVE;
		tme.hwndTrack = hWnd;
		TrackMouseEvent(&tme);
	}

	void OnMouseLeave(HWND hWnd)
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

	void OnLButtonDown(HWND hWnd, BOOL bDoubleClick, int x, int y, UINT uKeyFlags)
	{
		SetFocus(hWnd);
		int idx = HitTest(x, y);
		RECT rcItem;
		if (m_FbMultiSel)
		{
			
		}
		else
		{
			std::swap(m_idxSel, idx);
			if (idx >= 0)
			{
				GetItemRect(idx, rcItem);
				Redraw(rcItem);
			}
			if (m_idxSel >= 0)
			{
				GetItemRect(m_idxSel, rcItem);
				Redraw(rcItem);
			}
		}
	}

	void OnMouseWheel(HWND hWnd, int xPos, int yPos, int zDelta, UINT uKeys)
	{
		SCROLLINFO si;
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_POS;
		m_SBV.GetInfo(&si);
		const int yOld = si.nPos;
		si.nPos += (-zDelta / WHEEL_DELTA * m_cyItem * 3);
		m_SBV.SetInfo(&si);
		m_SBV.GetInfo(&si);
		ReCalcTopItem();
		ScrollWindow(hWnd, 0, yOld - si.nPos, NULL, NULL);
		UpdateWindow(hWnd);
	}


	void ReCalcTopItem()
	{
		const int ySB = m_SBV.GetPos();
		m_idxTop = ySB / m_cyItem;
		m_oyTop = m_idxTop * m_cyItem - ySB;
	}

	void RedrawItem(int idx, const RECT& rcItem)
	{
		LBNEWITEM item{};
		const HDC hCDC = m_DC.GetDC();
		int iState;

		if (m_idxHot == idx)
			if (IsItemSel(idx))
				iState = LISS_HOTSELECTED;
			else
				iState = LISS_HOT;
		else
			if (IsItemSel(idx))
				iState = LISS_SELECTED;
			else
				iState = LISS_NORMAL;

		m_StylePainter.DrawThemeBackground(hCDC, LVP_LISTITEM, iState, rcItem, NULL);

		item.idxItem = idx;
		m_pfnProc(m_hWnd, GetDispInfo, (LPARAM)&item, m_lRefData);
		if (item.pszText)
			DrawTextW(hCDC, item.pszText, item.cchText, (RECT*)&rcItem, DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX);
	}

	void ReCalcScrollBar()
	{
		SCROLLINFO si;
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_PAGE | SIF_RANGE;
		si.nMin = 0;
		si.nMax = (int)m_vItem.size() * m_cyItem;
		si.nPage = m_cyClient;
		m_SBV.SetInfo(&si);
	}
public:
	static ATOM RegisterWndClass()
	{
		WNDCLASSW wc{};
		wc.cbWndExtra = sizeof(void*);
		wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
		wc.hInstance = eck::g_hInstance;
		wc.lpfnWndProc = WndProc;
		wc.lpszClassName = WCN_LISTBOXNEW;
		wc.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
		return RegisterClassW(&wc);
	}

	EckInline HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, int nID, PCVOID pData = NULL)
	{
		m_hWnd = CreateWindowExW(dwExStyle, WCN_LISTBOXNEW, pszText, dwStyle,
			x, y, cx, cy, hParent, i32ToP<HMENU>(nID), g_hInstance, this);
		return m_hWnd;
	}

	EckInline void SetProc(FLbnProc pfn) { m_pfnProc = pfn; }

	EckInline void SetRefData(LPARAM lRefData) { m_lRefData = lRefData; }

	EckInline void SetItemCount(int cItem)
	{
		m_vItem.resize(cItem);
		ReCalcScrollBar();
		ReCalcTopItem();
	}

	int HitTest(int x, int y)
	{
		if (x < 0 || x > m_cxClient || y < 0 || y > m_cyClient)
			return -1;
		const int idx = m_idxTop + (y - m_oyTop) / m_cyItem;
		if (idx >= (int)m_vItem.size())
			return -1;
		else
			return idx;
	}

	EckInline void GetItemRect(int idx, RECT& rc)
	{
		rc = { 0,m_oyTop + (m_cyItem * (idx - m_idxTop)),m_cxClient };
		rc.bottom = rc.top + m_cyItem;
	}

	EckInline int GetCurrSel() const { return m_idxSel; }

	EckInline void GetItemState(int idx, UINT uState)
	{

	}

	EckInline BOOL IsItemSel(int idx)
	{
		if (m_FbMultiSel)
			return m_vItem[idx].bSel;
		else
			return m_idxSel == idx;
	}
};

ECK_NAMESPACE_END