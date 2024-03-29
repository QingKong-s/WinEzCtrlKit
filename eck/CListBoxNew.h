﻿/*
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

#include <vssym32.h>

ECK_NAMESPACE_BEGIN
struct LBNITEM
{
	PCWSTR pszText;
	int cchText;
	int idxItem;
};

struct NMLBNGETDISPINFO
{
	NMHDR nmhdr;
	LBNITEM Item;
};

struct NMLBNDRAG
{
	NMHDR nmhdr;
	int idx;
	UINT uKeyFlags;
};


class CListBoxNew :public CWnd
{
private:
	struct ITEM
	{
		CRefStrW rsText{};
		union
		{
			struct
			{
				BITBOOL bSel : 1;
				BITBOOL bSlideSel : 1;
			};
			UINT uFlags = 0u;
		};
	};

	CEzCDC m_DC{};
	HFONT m_hFont = NULL;// 不需要释放
	HBRUSH m_hbrBkg = NULL;
	COLORREF m_crText = GetSysColor(COLOR_WINDOWTEXT);

	HTHEME m_hTheme = NULL;

	int m_cxClient = 0,
		m_cyClient = 0;

	int m_cyItem = 24;

	int m_idxSel = -1;
	int m_idxHot = -1;
	int m_idxTop = -1;
	int m_idxMark = -1;
	int m_idxFocus = -1;
	int m_oyTop = 0;

	std::vector<ITEM> m_vItem{};

#ifdef _DEBUG
	BITBOOL m_bDbgDrawMarkItem : 1 = 1;
#endif
	BITBOOL m_bMultiSel : 1 = FALSE;
	BITBOOL m_bExtendSel : 1 = 1;// FALSE;
	BITBOOL m_bAllowDrag : 1 = FALSE;

	BITBOOL m_bLBtnDown : 1 = FALSE;
	BITBOOL m_bUserItemHeight : 1 = FALSE;
	BITBOOL m_bFocusIndicatorVisible : 1 =
#ifdef _DEBUG
		TRUE;
#else
		FALSE;
#endif

	int m_iDpi = USER_DEFAULT_SCREEN_DPI;

	ECK_DS_BEGIN(DPIS)
		ECK_DS_ENTRY(cyItemDef, 24)
		ECK_DS_ENTRY(cxTextPadding, 3)
		;
	ECK_DS_END_VAR(m_Ds);


	void UpdateColor()
	{
		DeleteObject(m_hbrBkg);
		COLORREF cr;
		GetItemsViewForeBackColor(m_crText, cr);
		m_hbrBkg = CreateSolidBrush(cr);
	}

	BOOL OnCreate(HWND hWnd, CREATESTRUCTW* pcs)
	{
		m_iDpi = GetDpi(hWnd);
		UpdateDpiSize(m_Ds, m_iDpi);
		m_cyItem = m_Ds.cyItemDef;

		m_DC.Create(hWnd);
		SetBkMode(m_DC.GetDC(), TRANSPARENT);

		SetItemsViewTheme();
		m_hTheme = OpenThemeData(hWnd, L"ListView");
		UpdateColor();
		return TRUE;
	}

	void OnSize(HWND hWnd, UINT uState, int cx, int cy)
	{
		m_cxClient = cx;
		m_cyClient = cy;

		SetSbPage(SB_VERT, cy);
		m_DC.ReSize(hWnd, cx, cy);
	}

	void OnPaint(HWND hWnd)
	{
		PAINTSTRUCT ps;
		BeginPaint(hWnd, &ps);
		const int idxTop = std::max(m_idxTop + (int)ps.rcPaint.top / m_cyItem - 1, m_idxTop);
		const int idxBottom = std::min(m_idxTop + (int)ps.rcPaint.bottom / m_cyItem + 1, (int)m_vItem.size() - 1);

		FillRect(m_DC.GetDC(), &ps.rcPaint, m_hbrBkg);
		SetTextColor(m_DC.GetDC(), m_crText);

		RECT rc;
		GetItemRect(idxTop, rc);
		for (int i = idxTop; i <= idxBottom; ++i)
		{
			PaintItem(i, rc);
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
		GetSbInfo(SB_VERT, &si);
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
		SetSbInfo(SB_VERT, &si);
		GetSbInfo(SB_VERT, &si);
		ReCalcTopItem();
		ScrollWindow(hWnd, 0, yOld - si.nPos, NULL, NULL);
		UpdateWindow(hWnd);
	}

	void OnMouseMove(HWND hWnd, int x, int y, UINT uKeyFlags)
	{
		if (!m_bLBtnDown)
		{
			int idxHot = HitTest(x, y);
			if (idxHot == m_idxHot)
				return;
			std::swap(idxHot, m_idxHot);
			if (idxHot >= 0)
				RedrawItem(idxHot);
			if (m_idxHot >= 0)
				RedrawItem(m_idxHot);
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

	void SelectItemForClick(int idx)
	{
		int idxChangedBegin = -1, idxChangedEnd = -1;
		const int idxOldFocus = m_idxFocus;
		if (idx >= 0)
			m_idxFocus = idx;
		if (m_bExtendSel)
		{
			if (GetAsyncKeyState(VK_CONTROL))
			{
				if (idx >= 0)
				{
					m_idxMark = idx;
					ECKBOOLNOT(m_vItem[idx].bSel);
					RedrawItem(idx);
					if (idxOldFocus >= 0 && idxOldFocus != idx)
						RedrawItem(idxOldFocus);
				}
			}
			else if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
			{
				if (m_idxMark >= 0 && idx >= 0)
					SelectRangeForClick(std::min(m_idxMark, idx),
						std::max(m_idxMark, idx));
			}
			else
			{
				DeselectAll(idxChangedBegin, idxChangedEnd);
				if (idxChangedBegin >= 0)
					RedrawItem(idxChangedBegin, idxChangedEnd);
				if (idx >= 0)
				{
					m_idxMark = idx;
					m_vItem[idx].bSel = TRUE;
					if (idxChangedBegin < 0 || (idx < idxChangedBegin || idx > idxChangedEnd))
						RedrawItem(idx);
				}
			}
		}
		else if (m_bMultiSel)
		{
			if (idx >= 0)
			{
				ECKBOOLNOT(m_vItem[idx].bSel);
				RedrawItem(idx);
				if (idxOldFocus >= 0 && idxOldFocus != idx)
					RedrawItem(idxOldFocus);
			}
		}
		else
		{
			if (m_idxSel != idx)
			{
				std::swap(m_idxSel, idx);
				if (m_idxSel >= 0)
					RedrawItem(m_idxSel);
				if (idx >= 0)
					RedrawItem(idx);
			}
		}
	}

	void SelectRangeForClick(int idxBegin, int idxEnd)
	{
		EckAssert(m_bExtendSel);
		int i;
		int idx0 = -1, idx1 = -1;
		// 清除前面选中
		for (i = 0; i < idxBegin; ++i)
		{
			if (m_vItem[i].bSel)
			{
				if (idx0 < 0)
					idx0 = i;
				idx1 = i;
				m_vItem[i].bSel = FALSE;
			}
		}
		// 范围选中
		for (i = idxBegin; i <= idxEnd; ++i)
		{
			// if ()
			{
				if (idx0 < 0)
					idx0 = i;
				idx1 = i;
				m_vItem[i].bSel = TRUE;
			}
		}
		// 清除后面选中
		for (i = idxEnd + 1; i < (int)m_vItem.size(); ++i)
		{
			if (m_vItem[i].bSel)
			{
				if (idx0 < 0)
					idx0 = i;
				idx1 = i;
				m_vItem[i].bSel = FALSE;
			}
		}
		if (idx0 >= 0)
			RedrawItem(idx0, idx1);
	}

	void BeginDraggingSelect(int idxBegin)
	{
		MSG msg;
		SetCapture(HWnd);
		int idxOld = idxBegin;
		for (auto& e : m_vItem)
			e.bSlideSel = e.bSel;
		int idxOldSelBegin = -1,
			idxOldSelEnd = -1,
			idxOld0 = -1,
			idxOld1 = -1;
		while (GetCapture() == HWnd)// 如果捕获改变则应立即退出拖动循环
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
					//----------滚动
					int yDelta = 0;
					if (pt.y < 0)
						yDelta = pt.y;
					else if (pt.y > m_cyClient)
						yDelta = pt.y - m_cyClient;

					if (yDelta)
					{
						SetRedraw(FALSE);
						ScrollV(yDelta);
						SetRedraw(TRUE);
					}
					//----------选中
					int idxCurr = HitTest(pt.x, pt.y);
					if (idxCurr < 0)
					{
						if (pt.y < 0)
							idxCurr = m_idxTop;
						else if (pt.y > m_cyClient)
						{
							idxCurr = m_idxTop + (m_cyClient - m_oyTop) / m_cyItem;
							if (idxCurr >= GetItemCount())
								idxCurr = GetItemCount() - 1;
						}
					}

					if (m_bExtendSel)
					{
						const int
							// 闭区间
							idxSelBegin = std::min(m_idxMark, idxCurr),
							idxSelEnd = std::max(m_idxMark, idxCurr),
							// 闭区间
							idx0 = std::min({ m_idxMark,idxCurr,idxOld }),
							idx1 = std::max({ m_idxMark,idxCurr,idxOld });
						if (idxOldSelBegin != idxSelBegin && 
							idxOldSelEnd != idxSelEnd &&
							idxOld0 != idx0 && 
							idxOld1 != idx1)
						{
							for (int i = idx0; i <= idx1; ++i)
							{
								auto& e = m_vItem[i];
								if (idxSelBegin <= i && i <= idxSelEnd)
									e.bSel = TRUE;
								else
									e.bSel = e.bSlideSel;
							}
							idxOld = idxCurr;
						}
					}
					/*else if (m_bMultiSel)
					{
						// 不可能出现
					}*/
					else
						SelectItemForClick(idxCurr);
					Redraw();
					UpdateWindow(HWnd);
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
				WaitMessage();
		}
	ExitDraggingLoop:
		ReleaseCapture();
		m_bLBtnDown = FALSE;
	}

	void OnLButtonDown(HWND hWnd, BOOL bDoubleClick, int x, int y, UINT uKeyFlags)
	{
		m_bLBtnDown = TRUE;
		SetFocus(hWnd);
		const int idx = HitTest(x, y);
		if (idx < 0)
			return;
		POINT ptScr{ x,y };
		ClientToScreen(hWnd, &ptScr);
		SelectItemForClick(idx);
		/*
		* 拖动动作：
		* 模式		禁止拖放时	允许拖放时
		* ------------------------------
		* 单选		跟随选中		无
		* 多选		无			无
		* 扩展多选	范围选择		无
		*/
		if (IsMouseMovedBeforeDragging(hWnd, ptScr.x, ptScr.y, 0))// YEILD
		{
			if (!IsValid())// revalidate
				return;
			if (m_bAllowDrag)
			{
				SetCapture(hWnd);
				NMLBNDRAG nm;
				nm.idx = idx;
				nm.uKeyFlags = uKeyFlags;
				FillNmhdrAndSendNotify(nm, NM_LBN_BEGINDRAG);
			}
			else if (m_bExtendSel || (!m_bExtendSel && !m_bMultiSel))// 扩展多选或单选
				BeginDraggingSelect(idx);
		}
	}

	void OnMouseWheel(HWND hWnd, int xPos, int yPos, int zDelta, UINT uKeys)
	{
		SCROLLINFO si;
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_POS;
		GetSbInfo(SB_VERT, &si);
		const int yOld = si.nPos;
		si.nPos += (-zDelta / WHEEL_DELTA * m_cyItem * 3);
		SetSbInfo(SB_VERT, &si);
		GetSbInfo(SB_VERT, &si);
		ReCalcTopItem();
		ScrollWindow(hWnd, 0, yOld - si.nPos, NULL, NULL);
		UpdateWindow(hWnd);
	}

	void ReCalcTopItem()
	{
		const int ySB = GetSbPos(SB_VERT);
		m_idxTop = ySB / m_cyItem;
		m_oyTop = m_idxTop * m_cyItem - ySB;
	}

	void PaintItem(int idx, const RECT& rcItem)
	{
		const HDC hCDC = m_DC.GetDC();
		int iState = 0;

		if (m_idxHot == idx)
			if (IsItemSel(idx))
				iState = LISS_HOTSELECTED;
			else
				iState = LISS_HOT;
		else
			if (IsItemSel(idx))
				iState = LISS_SELECTED;
		if (iState)
			DrawThemeBackground(m_hTheme, hCDC, LVP_LISTITEM, iState, &rcItem, NULL);
		if (m_bFocusIndicatorVisible && m_idxFocus == idx)
		{
			RECT rc{ rcItem };
			InflateRect(rc, -1, -1);
			DrawFocusRect(hCDC, &rc);
		}

		NMLBNGETDISPINFO nm;
		nm.Item.idxItem = idx;
		FillNmhdrAndSendNotify(nm, NM_LBN_GETDISPINFO);
		if (nm.Item.pszText)
		{
			RECT rc{ rcItem };
			rc.left += m_Ds.cxTextPadding;
			DrawTextW(hCDC, nm.Item.pszText, nm.Item.cchText, &rc,
				DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX | DT_NOCLIP | DT_END_ELLIPSIS);
		}
#ifdef _DEBUG
		if (m_bDbgDrawMarkItem && idx == m_idxMark)
		{
			const auto dummy1 = SetTextColor(hCDC, Colorref::Red);
			DrawTextW(hCDC, L"Mark", -1, (RECT*)&rcItem, DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX | DT_NOCLIP);
			SetTextColor(hCDC, dummy1);
		}
#endif
	}

	void ReCalcScrollBar()
	{
		SCROLLINFO si;
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_PAGE | SIF_RANGE;
		si.nMin = 0;
		si.nMax = (int)m_vItem.size() * m_cyItem;
		si.nPage = m_cyClient;
		SetSbInfo(SB_VERT, &si);
	}
public:
	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		switch (uMsg)
		{
		case WM_MOUSELEAVE:
			return ECK_HANDLE_WM_MOUSELEAVE(hWnd, wParam, lParam, OnMouseLeave);
		case WM_MOUSEMOVE:
			return HANDLE_WM_MOUSEMOVE(hWnd, wParam, lParam, OnMouseMove);
		case WM_SIZE:
			return HANDLE_WM_SIZE(hWnd, wParam, lParam, OnSize);
		case WM_PAINT:
			return HANDLE_WM_PAINT(hWnd, wParam, lParam, OnPaint);
		case WM_VSCROLL:
			return HANDLE_WM_VSCROLL(hWnd, wParam, lParam, OnVScroll);
		case WM_MOUSEWHEEL:
			return HANDLE_WM_MOUSEWHEEL(hWnd, wParam, lParam, OnMouseWheel);
		case WM_LBUTTONDOWN:
			return HANDLE_WM_LBUTTONDOWN(hWnd, wParam, lParam, OnLButtonDown);
		case WM_LBUTTONUP:
		{
			if (m_bLBtnDown)
			{
				POINT pt ECK_GET_PT_LPARAM(lParam);
				ReleaseCapture();
				m_bLBtnDown = FALSE;
				NMLBNDRAG nm;
				nm.idx = HitTest(pt.x, pt.y);
				nm.uKeyFlags = (UINT)wParam;
				FillNmhdrAndSendNotify(nm, NM_LBN_ENDDRAG);
			}
		}
		return 0;
		case WM_KEYDOWN:
		{
			switch (wParam)
			{
			case VK_DOWN:
			case VK_RIGHT:
			{
				EckAssert(m_idxFocus < GetItemCount());
				if (m_idxFocus < 0)
					break;
				if (m_idxFocus == GetItemCount() - 1)
				{
					EnsureVisible(m_idxFocus);
					break;
				}
				++m_idxFocus;
				if (m_bExtendSel)
					SelectItemForClick(m_idxFocus);
				else if (m_bMultiSel)
				{
					if (m_bFocusIndicatorVisible)
						RedrawItem(m_idxFocus - 1, m_idxFocus);
				}
				else
					SelectItemForClick(m_idxFocus);
				EnsureVisible(m_idxFocus);
			}
			return 0;

			case VK_UP:
			case VK_LEFT:
			{
				EckAssert(m_idxFocus < GetItemCount());
				if (m_idxFocus < 0)
					break;
				if (m_idxFocus == 0)
				{
					EnsureVisible(m_idxFocus);
					break;
				}
				--m_idxFocus;
				if (m_bExtendSel)
					SelectItemForClick(m_idxFocus);
				else if (m_bMultiSel)
				{
					if (m_bFocusIndicatorVisible)
						RedrawItem(m_idxFocus, m_idxFocus + 1);
				}
				else
					SelectItemForClick(m_idxFocus);
				EnsureVisible(m_idxFocus);
			}
			return 0;

			case VK_SPACE:
			{
				if (m_bMultiSel && m_idxFocus >= 0)
				{
					ECKBOOLNOT(m_vItem[m_idxFocus].bSel);
					RedrawItem(m_idxFocus);
				}
			}
			return 0;

			case VK_PRIOR:
			{
				const int idxOldFocus = m_idxFocus;
				int idx = m_idxFocus;
				if (idx == m_idxTop)
				{
					idx -= ((m_cyClient - (m_cyItem + m_oyTop)) / m_cyItem);
					if (idx < 0)
						idx = 0;
				}
				else
					idx = m_idxTop;

				if (m_bExtendSel)
					SelectItemForClick(idx);
				else if (m_bMultiSel)
				{
					m_idxFocus = idx;
					if (m_idxFocus >= 0)
						RedrawItem(m_idxFocus);
					if (idxOldFocus >= 0 && idxOldFocus != m_idxFocus)
						RedrawItem(idxOldFocus);
				}
				else
					SelectItemForClick(idx);
				EnsureVisible(idx);
			}
			return 0;
			case VK_NEXT:
			{
				int idxBottom = m_idxTop + (m_cyClient - (m_cyItem + m_oyTop)) / m_cyItem;
				if (idxBottom >= GetItemCount())
					idxBottom = GetItemCount() - 1;

				const int idxOldFocus = m_idxFocus;
				int idx = m_idxFocus;
				if (idx == idxBottom)
				{
					idx += (m_cyClient / m_cyItem);
					if (idx >= GetItemCount())
						idx = GetItemCount() - 1;
				}
				else
					idx = idxBottom;

				if (m_bExtendSel)
					SelectItemForClick(idx);
				else if (m_bMultiSel)
				{
					m_idxFocus = idx;
					if (m_idxFocus >= 0)
						RedrawItem(m_idxFocus);
					if (idxOldFocus >= 0 && idxOldFocus != m_idxFocus)
						RedrawItem(idxOldFocus);
				}
				else
					SelectItemForClick(idx);
				EnsureVisible(idx);
			}
			return 0;
			}
		}
		return 0;
		case WM_SETFONT:
			m_hFont = (HFONT)wParam;
			SelectObject(m_DC.GetDC(), m_hFont);
			if (lParam)
				Redraw();
			return 0;
		case WM_GETFONT:
			return (LRESULT)m_hFont;
		case WM_THEMECHANGED:
		{
			CloseThemeData(m_hTheme);
			m_hTheme = OpenThemeData(hWnd, L"ListView");
			UpdateColor();
		}
		return 0;
		case WM_DPICHANGED_BEFOREPARENT:
		{
			m_iDpi = eck::GetDpi(hWnd);
			UpdateDpiSize(m_Ds, m_iDpi);
			if (!m_bUserItemHeight)
				m_cyItem = m_Ds.cyItemDef;
		}
		return 0;
		case WM_CREATE:
			return HANDLE_WM_CREATE(hWnd, wParam, lParam, OnCreate);
		case WM_DESTROY:
			CloseThemeData(m_hTheme);
			DeleteObject(m_hbrBkg);
			return 0;
		}

		return CWnd::OnMsg(hWnd, uMsg, wParam, lParam);
	}

	ECK_CWND_CREATE;
	HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, HMENU hMenu, PCVOID pData = NULL) override
	{
		IntCreate(dwExStyle, WCN_LISTBOXNEW, pszText, dwStyle,
			x, y, cx, cy, hParent, hMenu, g_hInstance, this);
		return m_hWnd;
	}

	EckInline void SetItemCount(int cItem)
	{
		m_vItem.resize(cItem);
		ReCalcScrollBar();
		ReCalcTopItem();
	}

	EckInline int GetItemCount() { return (int)m_vItem.size(); }

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

	EckInline int GetItemY(int idx)
	{
		return m_oyTop + (m_cyItem * (idx - m_idxTop));
	}

	EckInline void GetItemRect(int idx, RECT& rc)
	{
		rc = { 0,GetItemY(idx),m_cxClient};
		rc.bottom = rc.top + m_cyItem;
	}

	EckInline int GetCurrSel() const { return m_idxSel; }

	EckInline UINT GetItemState(int idx)
	{
		return m_vItem[idx].uFlags;
	}

	EckInline BOOL IsItemSel(int idx)
	{
		if (m_bMultiSel || m_bExtendSel)
			return m_vItem[idx].bSel;
		else
			return m_idxSel == idx;
	}

	EckInline void GetSelItem(std::vector<int>& v)
	{
		v.clear();
		for (int i = 0; const auto & e : m_vItem)
		{
			if (e.bSel)
				v.emplace_back(i);
			++i;
		}
	}

	EckInline void EnsureVisible(int idx)
	{
		RECT rc;
		GetItemRect(idx, rc);
		if (rc.bottom >= m_cyClient)
			ScrollV(rc.bottom - m_cyClient);
		else if (rc.top <= 0)
			ScrollV(rc.top);
	}

	void ScrollV(int yDelta)
	{
		SCROLLINFO si;
		si.cbSize = sizeof(si);
		si.fMask = SIF_ALL;
		GetSbInfo(SB_VERT, &si);
		const int yOld = si.nPos;
		si.nPos += yDelta;
		si.fMask = SIF_POS;
		SetSbInfo(SB_VERT, &si);
		GetSbInfo(SB_VERT, &si);
		ReCalcTopItem();
		ScrollWindow(HWnd, 0, yOld - si.nPos, NULL, NULL);
		UpdateWindow(HWnd);
	}

	void RedrawItem(int idx)
	{
		EckAssert(idx >= 0 && idx < GetItemCount());
		RECT rc;
		GetItemRect(idx, rc);
		Redraw(rc);
	}

	void RedrawItem(int idxFrom, int idxTo)
	{
		EckAssert(idxFrom >= 0 && idxFrom < GetItemCount());
		EckAssert(idxTo >= 0 && idxTo < GetItemCount());
		EckAssert(idxFrom <= idxTo);
		RECT rc;
		GetItemRect(idxFrom, rc);
		rc.bottom += (idxTo - idxFrom) * m_cyItem;
		Redraw(rc);
	}

	void DeselectAll(int& idxChangedBegin, int& idxChangedEnd)
	{
		int idx0 = -1, idx1 = -1;
		EckCounter(GetItemCount(), i)
		{
			auto& e = m_vItem[i];
			if (e.bSel)
			{
				if (idx0 < 0)
					idx0 = (int)i;
				idx1 = (int)i;
				e.bSel = FALSE;
			}
		}
		idxChangedBegin = idx0;
		idxChangedEnd = idx1;
	}
};
ECK_NAMESPACE_END