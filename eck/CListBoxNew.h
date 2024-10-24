﻿/*
* WinEzCtrlKit Library
*
* CListBoxNew.h ： 所有者数据模式的列表框
*
* Copyright(C) 2023-2024 QingKong
*/
#pragma once
#include "CWnd.h"
#include "CtrlGraphics.h"

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

enum :UINT
{
	LBN_IF_SEL = 1u << 0,
	LBN_IF_SLIDE_SEL = 1u << 1,
};

struct NMLBNITEMCHANGED
{
	NMHDR nmhdr;
	int idx;
	UINT uFlagsNew;
	UINT uFlagsOld;
};

#pragma pack(push, ECK_CTRLDATA_ALIGN)
struct CTRLDATA_LBN
{
	int iVer = 0;


	// 
};
#pragma pack(pop)
/*
* LBN产生的通知
* 特定通知：
* NM_LBN_GETDISPINFO	请求项目显示信息
* NM_LBN_BEGINDRAG		开始拖动项目
* NM_LBN_ENDDRAG		结束拖动项目
* NM_LBN_DISMISS		组合框应隐藏列表
* NM_LBN_ITEMCHANGED	项目状态改变
* NM_LBN_ITEMSTANDBY	调用SetItemCount时发送
* 标准通知：
* NM_SETFOCUS
* NM_KILLFOCUS
* NM_RCLICK
* NM_CUSTOMDRAW
*/
class CListBoxNew :public CWnd
{
public:
	ECK_RTTI(CListBoxNew);
	ECK_CWND_SINGLEOWNER(CListBoxNew);
	ECK_CWND_CREATE_CLS_HINST(WCN_LISTBOXNEW, g_hInstance);
private:
	struct ITEM
	{
		UINT uFlags{};
	};

	std::vector<ITEM> m_vItem{};

	CRefStrW m_rsTextBuf{};

	HWND m_hComboBox{};	// 关联的组合框，可以是除自身外的任何窗口
	HWND m_hParent{};	// 接收通知的父窗口

	CEzCDC m_DC{};
	HFONT m_hFont{};

	HTHEME m_hTheme{};

	int m_cxClient{},
		m_cyClient{};

	COLORREF m_crBkg{ CLR_DEFAULT };
	COLORREF m_crText{ CLR_DEFAULT };
	int m_idxSel{ -1 };
	int m_idxHot{ -1 };
	int m_idxTop{ -1 };
	int m_idxMark{ -1 };
	int m_idxFocus{ -1 };
	int m_oyTop{};

	int m_cyItem{ 24 };
	int m_cyFont{};

	BITBOOL m_bMultiSel : 1 = FALSE;	// 多选
	BITBOOL m_bExtendSel : 1 = FALSE;	// 扩展多选
	BITBOOL m_bAllowDrag : 1 = FALSE;	// 允许拖放项目
	BITBOOL m_bAutoItemHeight : 1 = TRUE;		// 自动计算项目高度

#ifdef _DEBUG
	BITBOOL m_bDbgDrawMarkItem : 1 = 1;	// [调试]绘制标记项目
#endif
	BITBOOL m_bHasFocus : 1 = FALSE;	// 是否有焦点
	BITBOOL m_bLBtnDown : 1 = FALSE;	// 鼠标左键已按下
	BITBOOL m_bRBtnDown : 1 = FALSE;	// 鼠标右键已按下
	BITBOOL m_bFocusIndicatorVisible : 1 = Dbg;	// 焦点指示器是否可见
	BITBOOL m_bNmDragging : 1 = FALSE;			// 正在拖放项目，产生NM_LBN_BEGINDRAG时设置为TRUE
	BITBOOL m_bTrackComboBoxList : 1 = FALSE;	// 正在作为组合框的下拉列表显示
	BITBOOL m_bProtectCapture : 1 = FALSE;		// 允许其他窗口占用鼠标捕获，通常用于弹出下拉列表时显示菜单等

	int m_iDpi{ USER_DEFAULT_SCREEN_DPI };

	void UpdateFontMetrics()
	{
		TEXTMETRICW tm;
		GetTextMetricsW(m_DC.GetDC(), &tm);
		m_cyFont = tm.tmHeight;
	}

	void UpdateDefItemHeight()
	{
		EckAssert(m_bAutoItemHeight);
		m_cyItem = m_cyFont + DpiScale(MetricsExtraV, m_iDpi);
	}

	LRESULT NotifyItemChanged(int idx, UINT uOldFlags, UINT uNewFlags)
	{
		NMLBNITEMCHANGED nm;
		nm.idx = idx;
		nm.uFlagsOld = uOldFlags;
		nm.uFlagsNew = uNewFlags;
		return FillNmhdrAndSendNotify(nm, m_hParent, NM_LBN_ITEMCHANGED);
	}

	LRESULT NotifyItemChanged(int idx, UINT uOldFlags)
	{
		return NotifyItemChanged(idx, uOldFlags, m_vItem[idx].uFlags);
	}

	BOOL OnCreate(HWND hWnd, CREATESTRUCTW* pcs)
	{
		m_hParent = pcs->hwndParent;

		m_iDpi = GetDpi(hWnd);

		m_DC.Create(hWnd);
		SetBkMode(m_DC.GetDC(), TRANSPARENT);

		SetItemsViewTheme();
		m_hTheme = OpenThemeData(hWnd, L"ListView");
		return TRUE;
	}

	void OnPaint(HWND hWnd, WPARAM wParam)
	{
		const auto* const ptc = GetThreadCtx();
		PAINTSTRUCT ps;
		BeginPaint(hWnd, wParam, ps);

		LRESULT lRet;
		NMCUSTOMDRAWEXT ne;
		FillNmhdr(ne, NM_CUSTOMDRAW);
		ne.nmcd.hdc = m_DC.GetDC();
		ne.nmcd.lItemlParam = 0;
		ne.crBk = CLR_DEFAULT;
		ne.crText = CLR_DEFAULT;
		ne.iStateId = 0;
		ne.iPartId = 0;

		ne.nmcd.dwDrawStage = CDDS_PREERASE;
		ne.nmcd.rc = ps.rcPaint;
		ne.nmcd.dwItemSpec = 0;
		ne.nmcd.uItemState = 0;
		ne.nmcd.lItemlParam = 0;
		lRet = SendNotify(ne, m_hParent);
		if (!(lRet & CDRF_SKIPDEFAULT))
		{
			if (ne.crBk != CLR_DEFAULT)
				SetDCBrushColor(ne.nmcd.hdc, ne.crBk);
			else
				SetDCBrushColor(ne.nmcd.hdc,
					(m_crBkg == CLR_DEFAULT) ? ptc->crDefBkg : m_crBkg);
			FillRect(ne.nmcd.hdc, &ps.rcPaint, GetStockBrush(DC_BRUSH));
		}
		if (lRet & CDRF_NOTIFYPOSTERASE)
		{
			ne.nmcd.dwDrawStage = CDDS_POSTERASE;
			SendNotify(ne.nmcd, m_hParent);
		}

		ne.nmcd.dwDrawStage = CDDS_PREPAINT;
		lRet = SendNotify(ne.nmcd, m_hParent);
		if (!(lRet & CDRF_SKIPDEFAULT))
		{
			const auto idxTop = (DWORD)std::max(m_idxTop + (int)ps.rcPaint.top / m_cyItem - 1, m_idxTop);
			const auto idxBottom = (DWORD)std::min(m_idxTop + (int)ps.rcPaint.bottom / m_cyItem + 1,
				(int)m_vItem.size() - 1);
			if (idxTop >= 0 && idxBottom >= 0)
			{
				SetTextColor(ne.nmcd.hdc,
					(m_crText == CLR_DEFAULT) ? ptc->crDefText : m_crText);
				GetItemRect(idxTop, ne.nmcd.rc);
				for (ne.nmcd.dwItemSpec = idxTop; ne.nmcd.dwItemSpec <= idxBottom;
					++ne.nmcd.dwItemSpec)
				{
					PaintItem(ne);
					ne.nmcd.rc.top += m_cyItem;
					ne.nmcd.rc.bottom += m_cyItem;
				}
			}
		}
		if (lRet & CDRF_NOTIFYPOSTPAINT)
		{
			ne.nmcd.dwDrawStage = CDDS_POSTPAINT;
			SendNotify(ne, m_hParent);
		}
		BitBltPs(&ps, ne.nmcd.hdc);
		EndPaint(hWnd, wParam, ps);
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
		if (si.nPos != yOld)
		{
			ReCalcTopItem();
			ScrollWindowEx(hWnd, 0, yOld - si.nPos, nullptr, nullptr,
				nullptr, nullptr, SW_INVALIDATE);
			UpdateWindow(hWnd);
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
					const auto uOld = m_vItem[idx].uFlags;
					m_vItem[idx].uFlags ^= LBN_IF_SEL;
					NotifyItemChanged(idx, uOld);
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
					if (idxChangedBegin < 0 || (idx < idxChangedBegin || idx > idxChangedEnd))
						RedrawItem(idx);
				}
			}
		}
		else if (m_bMultiSel)
		{
			if (idx >= 0)
			{
				const auto uOld = m_vItem[idx].uFlags;
				m_vItem[idx].uFlags ^= LBN_IF_SEL;
				NotifyItemChanged(idx, uOld);
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
				NotifyItemChanged(m_idxSel, 0u, LBN_IF_SEL);
				NotifyItemChanged(idx, LBN_IF_SEL, 0u);
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
			auto& e = m_vItem[i];
			if (e.uFlags & LBN_IF_SEL)
			{
				if (idx0 < 0)
					idx0 = i;
				idx1 = i;
				e.uFlags &= ~LBN_IF_SEL;
				NotifyItemChanged(i, e.uFlags | LBN_IF_SEL);
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
				auto& e = m_vItem[i];
				if (!(e.uFlags & LBN_IF_SEL))
				{
					e.uFlags |= LBN_IF_SEL;
					NotifyItemChanged(i, e.uFlags & ~LBN_IF_SEL);
				}
			}
		}
		// 清除后面选中
		for (i = idxEnd + 1; i < (int)m_vItem.size(); ++i)
		{
			auto& e = m_vItem[i];
			if (e.uFlags & LBN_IF_SEL)
			{
				if (idx0 < 0)
					idx0 = i;
				idx1 = i;
				e.uFlags &= ~LBN_IF_SEL;
				NotifyItemChanged(i, e.uFlags | LBN_IF_SEL);
			}
		}
		if (idx0 >= 0)
			RedrawItem(idx0, idx1);
	}

	void BeginDraggingSelect(int idxBegin)
	{
		MSG msg;
		m_bLBtnDown = TRUE;
		SetCapture(HWnd);
		int idxOld = idxBegin;
		for (auto& e : m_vItem)
			if (e.uFlags & LBN_IF_SEL)
				e.uFlags |= LBN_IF_SLIDE_SEL;
			else
				e.uFlags &= ~LBN_IF_SLIDE_SEL;
		int idxOldSelBegin = -1,
			idxOldSelEnd = -1,
			idxOld0 = -1,
			idxOld1 = -1;
		while (GetCapture() == HWnd)// 如果捕获改变则应立即退出拖动循环
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
								{
									if (!(e.uFlags & LBN_IF_SEL))
									{
										e.uFlags |= LBN_IF_SLIDE_SEL;
										NotifyItemChanged(i, e.uFlags & ~LBN_IF_SLIDE_SEL);
									}
								}
								else
								{
									if (IsBitSet(e.uFlags, LBN_IF_SLIDE_SEL) !=
										IsBitSet(e.uFlags, LBN_IF_SEL))
									{
										const auto uOld = e.uFlags;
										e.uFlags |= ((e.uFlags & LBN_IF_SLIDE_SEL) ?
											LBN_IF_SEL : 0);
										NotifyItemChanged(i, uOld);
									}
								}
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
		if (m_hComboBox)
		{
			EckAssert(m_bTrackComboBoxList);
			RECT rc;
			GetWindowRect(hWnd, &rc);
			ScreenToClient(hWnd, &rc);
			if (!PtInRect(rc, POINT{ x,y }))// 光标在窗口外，关闭列表
			{
				NMHDR nm;
				FillNmhdr(nm, NM_LBN_DISMISS);
				SendMessageW(m_hComboBox, WM_NOTIFY, nm.idFrom, (LPARAM)&nm);
				return;
			}
			else if (rc = { 0,0,m_cxClient,m_cyClient };
				!PtInRect(rc, POINT{ x,y }))// 试图拖动滚动条
			{
				POINT ptScr{ x,y };
				ClientToScreen(hWnd, &ptScr);
				EckAssert(GetCapture() == hWnd);
				m_bLBtnDown = FALSE;
				CbBeginProtectCapture();
				ReleaseCapture();
				__super::OnMsg(hWnd, WM_NCLBUTTONDOWN,
					__super::OnMsg(hWnd, WM_NCHITTEST, 0, POINTTOPOINTS(ptScr)),
					POINTTOPOINTS(ptScr));
				SetCapture(hWnd);
				CbEndProtectCapture();
				return;
			}
			else// 通常情况
			{
				m_bLBtnDown = TRUE;

				const int idx = HitTest(x, y);
				if (idx < 0)
					return;
				POINT ptScr{ x,y };
				ClientToScreen(hWnd, &ptScr);
				SelectItemForClick(idx);
				return;
			}
		}
		else
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
		if (IsMouseMovedBeforeDragging(hWnd, ptScr.x, ptScr.y, 0))// YEILD, ReleaseCapture
		{
			if (!IsValid())// revalidate
				return;
			m_bLBtnDown = TRUE;
			SetCapture(hWnd);
			if (m_bAllowDrag)
			{
				m_bNmDragging = TRUE;
				NMLBNDRAG nm;
				nm.idx = idx;
				nm.uKeyFlags = uKeyFlags;
				FillNmhdrAndSendNotify(nm, m_hParent, NM_LBN_BEGINDRAG);
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
		UINT cScrollLines;
		SystemParametersInfoW(SPI_GETWHEELSCROLLLINES, 0, &cScrollLines, 0);
		int d = -zDelta / WHEEL_DELTA * (m_cyItem * cScrollLines);
		if (d > m_cyClient)
			d = m_cyClient * 5 / 6;
		si.nPos += d;
		SetSbInfo(SB_VERT, &si);
		GetSbInfo(SB_VERT, &si);
		if (si.nPos != yOld)
		{
			ReCalcTopItem();
			ScrollWindowEx(hWnd, 0, yOld - si.nPos, nullptr, nullptr,
				nullptr, nullptr, SW_INVALIDATE);
			UpdateWindow(hWnd);
		}
	}

	void ReCalcTopItem()
	{
		const int ySB = GetSbPos(SB_VERT);
		m_idxTop = ySB / m_cyItem;
		m_oyTop = m_idxTop * m_cyItem - ySB;
	}

	void PaintItem(NMCUSTOMDRAWEXT& ne)
	{
		const int idx = (int)ne.nmcd.dwItemSpec;
		int iState{};
		if (m_idxHot == idx)
			if (IsItemSel(idx))
				iState = LISS_HOTSELECTED;
			else
				iState = LISS_HOT;
		else
			if (IsItemSel(idx))
				iState = LISS_SELECTED;

		ne.nmcd.dwDrawStage = CDDS_ITEMPREPAINT;
		ne.crBk = CLR_DEFAULT;
		ne.crText = CLR_DEFAULT;
		ne.iStateId = iState;
		ne.iPartId = LVP_LISTITEM;
		const auto lRet = SendNotify(ne, m_hParent);
		if (!(lRet & CDRF_SKIPDEFAULT))
		{
			BOOL bFillBk{};
			if (ne.crBk != CLR_DEFAULT)
			{
				if (ShouldAppsUseDarkMode())
					bFillBk = TRUE;
				else
				{
					SetDCBrushColor(ne.nmcd.hdc, ne.crBk);
					FillRect(ne.nmcd.hdc, &ne.nmcd.rc, GetStockBrush(DC_BRUSH));
				}
			}
			if (iState)
				DrawThemeBackground(m_hTheme, ne.nmcd.hdc, LVP_LISTITEM, iState, &ne.nmcd.rc, nullptr);
			if (bFillBk)
				AlphaBlendColor(ne.nmcd.hdc, ne.nmcd.rc, ne.crBk);

			NMLBNGETDISPINFO nm{};
			nm.Item.idxItem = idx;
			FillNmhdrAndSendNotify(nm, m_hParent, NM_LBN_GETDISPINFO);
			if (nm.Item.cchText > 0)
			{
				RECT rc{ ne.nmcd.rc };
				rc.left += DaGetSystemMetrics(SM_CXEDGE, m_iDpi);
				const auto crOld = ((ne.crText == CLR_DEFAULT) ?
					CLR_INVALID : SetTextColor(ne.nmcd.hdc, ne.crText));
				DrawTextW(ne.nmcd.hdc, nm.Item.pszText, nm.Item.cchText, &rc,
					DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX | DT_NOCLIP | DT_END_ELLIPSIS);
				if (crOld != CLR_INVALID)
					SetTextColor(ne.nmcd.hdc, crOld);
			}

			if (!(lRet & CDRF_SKIPPOSTPAINT) && m_bFocusIndicatorVisible && m_idxFocus == idx)
			{
				RECT rc{ ne.nmcd.rc };
				InflateRect(rc, -2, -2);
				DrawFocusRect(ne.nmcd.hdc, &rc);
			}
		}
#ifdef _DEBUG
		if (m_bDbgDrawMarkItem && idx == m_idxMark)
		{
			const auto crOld = SetTextColor(ne.nmcd.hdc, Colorref::Red);
			DrawTextW(ne.nmcd.hdc, L"Mark", -1, (RECT*)&ne.nmcd.rc,
				DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX | DT_NOCLIP);
			SetTextColor(ne.nmcd.hdc, crOld);
		}
#endif
		if (lRet & CDRF_NOTIFYPOSTPAINT)
		{
			ne.nmcd.dwDrawStage = CDDS_ITEMPOSTPAINT;
			SendNotify(ne, m_hParent);
		}
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

	void CheckOldData()
	{
		const int c = (int)m_vItem.size();
		if (m_idxHot >= c)
			m_idxHot = -1;
		if (m_idxFocus >= c)
			m_idxFocus = -1;
		if (m_idxMark >= c)
			m_idxMark = -1;
		if (m_idxSel >= c)
			m_idxSel = -1;
	}

	LRESULT CbNotifyDismiss()
	{
		NMHDR nm;
		return FillNmhdrAndSendNotify(nm, m_hParent, NM_LBN_DISMISS);
	}
public:
	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		switch (uMsg)
		{
		case WM_MOUSELEAVE:
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
		break;

		case WM_MOUSEMOVE:
		{
			if (!m_bLBtnDown)
			{
				const POINT pt ECK_GET_PT_LPARAM(lParam);
				int idxHot = HitTest(pt.x, pt.y);
				if (idxHot == m_idxHot)
					break;
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
		break;

		case WM_SIZE:
		{
			ECK_GET_SIZE_LPARAM(m_cxClient, m_cyClient, lParam);
			SetSbPage(SB_VERT, m_cyClient);
			m_DC.ReSize(hWnd, m_cxClient, m_cyClient);
		}
		break;

		case WM_PRINTCLIENT:
		case WM_PAINT:
			OnPaint(hWnd, wParam);
			return 0;

		case WM_VSCROLL:
			return HANDLE_WM_VSCROLL(hWnd, wParam, lParam, OnVScroll);

		case WM_MOUSEWHEEL:
			HANDLE_WM_MOUSEWHEEL(hWnd, wParam, lParam, OnMouseWheel);
			break;

		case WM_RBUTTONDOWN:
		{
			if (m_hComboBox)
			{
				const RECT rcClient{ 0,0,m_cxClient,m_cyClient };
				if (!PtInRect(&rcClient, ECK_GET_PT_LPARAM(lParam)))
					break;
				m_bRBtnDown = TRUE;
				// 无需捕获鼠标
				EckAssert(m_bTrackComboBoxList);
			}
			else
			{
				m_bRBtnDown = TRUE;
				SetCapture(hWnd);
			}
		}
		break;

		case WM_RBUTTONUP:
		{
			const RECT rcClient{ 0,0,m_cxClient,m_cyClient };
			if (m_hComboBox)
			{
				if (!PtInRect(&rcClient, ECK_GET_PT_LPARAM(lParam)))// 客户区之外，可能正在右击滚动条
				{
					CbBeginProtectCapture();
					const auto lResult = __super::OnMsg(hWnd, uMsg, wParam, lParam);
					SetCapture(hWnd);
					CbEndProtectCapture();
					return lResult;
				}
			}
			if (m_bRBtnDown)
			{
				if (!m_hComboBox)
					ReleaseCapture();
				m_bRBtnDown = FALSE;
				NMMOUSENOTIFY nm;
				nm.pt = ECK_GET_PT_LPARAM(lParam);
				nm.uKeyFlags = (UINT)wParam;
				FillNmhdrAndSendNotify(nm, m_hParent, NM_RCLICK);
			}
		}
		break;

		case WM_LBUTTONDOWN:
			HANDLE_WM_LBUTTONDOWN(hWnd, wParam, lParam, OnLButtonDown);
			break;

		case WM_LBUTTONUP:
		{
			if (m_bLBtnDown)
			{
				m_bLBtnDown = FALSE;
				POINT pt ECK_GET_PT_LPARAM(lParam);
				ReleaseCapture();
				if (!m_hComboBox && m_bNmDragging)
				{
					NMLBNDRAG nm;
					nm.idx = HitTest(pt.x, pt.y);
					nm.uKeyFlags = (UINT)wParam;
					FillNmhdrAndSendNotify(nm, m_hParent, NM_LBN_ENDDRAG);
				}
			}
		}
		break;

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
					m_vItem[m_idxFocus].uFlags ^= LBN_IF_SEL;
					NotifyItemChanged(m_idxFocus, m_vItem[m_idxFocus].uFlags ^ LBN_IF_SEL);
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
		break;

		case WM_CAPTURECHANGED:
		{
			if (m_hComboBox)
			{
				m_bLBtnDown = FALSE;
				m_bRBtnDown = FALSE;
				if (!m_bProtectCapture)
					CbNotifyDismiss();
			}
			else
			{
				m_bRBtnDown = FALSE;
				if (m_bLBtnDown)
				{
					POINT pt;
					GetCursorPos(&pt);
					ScreenToClient(hWnd, &pt);
					OnMsg(hWnd, WM_LBUTTONUP, 0, POINTTOPOINTS(pt));
				}
			}
		}
		break;

		case WM_SETFOCUS:
		{
			m_bHasFocus = TRUE;

			NMFOCUS nm;
			nm.hWnd = (HWND)wParam;
			FillNmhdrAndSendNotify(nm, m_hParent, NM_SETFOCUS);
		}
		break;

		case WM_KILLFOCUS:
		{
			m_bHasFocus = FALSE;

			NMFOCUS nm;
			nm.hWnd = (HWND)wParam;
			FillNmhdrAndSendNotify(nm, m_hParent, NM_KILLFOCUS);
		}
		break;

		case WM_SETFONT:
		{
			m_hFont = (HFONT)wParam;
			SelectObject(m_DC.GetDC(), m_hFont);
			UpdateFontMetrics();
			if (m_bAutoItemHeight)
			{
				UpdateDefItemHeight();
				ReCalcScrollBar();
			}
			if (lParam)
				Redraw();
		}
		return 0;

		case WM_GETFONT:
			return (LRESULT)m_hFont;

		case WM_THEMECHANGED:
		{
			CloseThemeData(m_hTheme);
			m_hTheme = OpenThemeData(hWnd, L"ListView");
		}
		return 0;

		case WM_DPICHANGED_BEFOREPARENT:
		{
			m_iDpi = eck::GetDpi(hWnd);
			if (m_bAutoItemHeight)
			{
				UpdateDefItemHeight();
				ReCalcScrollBar();
			}
		}
		return 0;

		case WM_CREATE:
			HANDLE_WM_CREATE(hWnd, wParam, lParam, OnCreate);
			break;

		case WM_DESTROY:
		{
			CloseThemeData(m_hTheme);
			m_hTheme = nullptr;
			m_hFont = nullptr;
			m_cyItem = 24;
			m_idxSel = m_idxHot = m_idxTop = m_idxMark = m_idxFocus = -1;
			m_oyTop = 0;
			m_vItem.clear();
			m_bHasFocus = m_bLBtnDown =
				m_bNmDragging = m_bTrackComboBoxList = FALSE;
			m_hComboBox = nullptr;
		}
		break;
		}

		return CWnd::OnMsg(hWnd, uMsg, wParam, lParam);
	}

	void SetItemCount(int cItem)
	{
		m_vItem.resize(cItem);
		CheckOldData();
		ReCalcScrollBar();
		ReCalcTopItem();
		NMHDR nm;
		FillNmhdrAndSendNotify(nm, m_hParent, NM_LBN_ITEMSTANDBY);
	}

	EckInline [[nodiscard]] constexpr int GetItemCount() { return (int)m_vItem.size(); }

	[[nodiscard]] int HitTest(int x, int y)
	{
		if (x < 0 || x > m_cxClient || y < 0 || y > m_cyClient)
			return -1;
		const int idx = m_idxTop + (y - m_oyTop) / m_cyItem;
		if (idx >= (int)m_vItem.size())
			return -1;
		else
			return idx;
	}

	EckInline [[nodiscard]] int GetItemY(int idx)
	{
		return m_oyTop + (m_cyItem * (idx - m_idxTop));
	}

	EckInline void GetItemRect(int idx, RECT& rc)
	{
		rc = { 0,GetItemY(idx),m_cxClient };
		rc.bottom = rc.top + m_cyItem;
	}

	void SetCurrSel(int idx)
	{
		EckAssert(idx >= 0 && idx < GetItemCount());
		SelectItemForClick(idx);
	}

	EckInline [[nodiscard]] int GetCurrSel() const { return m_idxSel; }

	EckInline [[nodiscard]] UINT GetItemState(int idx)
	{
		return m_vItem[idx].uFlags;
	}

	EckInline [[nodiscard]] BOOL IsItemSel(int idx)
	{
		if (m_bMultiSel || m_bExtendSel)
			return !!(m_vItem[idx].uFlags & LBN_IF_SEL);
		else
			return m_idxSel == idx;
	}

	void GetSelItem(std::vector<int>& v)
	{
		v.clear();
		for (int i = 0; const auto & e : m_vItem)
		{
			if (e.uFlags & LBN_IF_SEL)
				v.emplace_back(i);
			++i;
		}
	}

	void EnsureVisible(int idx)
	{
		if (idx < 0 || idx >= GetItemCount())
			return;
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
		ScrollWindow(HWnd, 0, yOld - si.nPos, nullptr, nullptr);
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
			if (e.uFlags & LBN_IF_SEL)
			{
				if (idx0 < 0)
					idx0 = (int)i;
				idx1 = (int)i;
				e.uFlags &= ~LBN_IF_SEL;
				NotifyItemChanged(i, e.uFlags | LBN_IF_SEL);
			}
		}
		idxChangedBegin = idx0;
		idxChangedEnd = idx1;
	}

	void SetItemHeight(int cy)
	{
		m_bAutoItemHeight = FALSE;
		m_cyItem = cy;
		ReCalcScrollBar();
		ReCalcTopItem();
	}

	EckInline int GetItemHeight() const { return m_cyItem; }

#pragma region 组合框交互
	EckInline void SetComboBox(HWND h)
	{
		m_hComboBox = h;
		if (h)
			m_hParent = h;
		else
			m_hParent = GetParent(HWnd);
	}

	EckInline constexpr HWND GetComboBox() const { return m_hComboBox; }

	EckInline void CbEnterTrack()
	{
		SetCapture(HWnd);
		m_bTrackComboBoxList = TRUE;
	}

	EckInline void CbLeaveTrack()
	{
		if (m_bTrackComboBoxList)
		{
			ReleaseCapture();
			m_bTrackComboBoxList = FALSE;
		}
	}

	EckInline void CbBeginProtectCapture()
	{
		EckAssert(m_hComboBox && !m_bProtectCapture);
		m_bProtectCapture = TRUE;
	}

	EckInline void CbEndProtectCapture()
	{
		EckAssert(m_hComboBox && m_bProtectCapture);
		m_bProtectCapture = FALSE;
	}
#pragma endregion 组合框交互

	EckInline constexpr void SetMultiSel(BOOL b) { m_bMultiSel = b; }
	EckInline constexpr BOOL GetMultiSel() const { return m_bMultiSel; }

	EckInline constexpr void SetExtendSel(BOOL b) { m_bExtendSel = b; }
	EckInline constexpr BOOL GetExtendSel() const { return m_bExtendSel; }

	void SetAutoItemHeight(BOOL b)
	{
		m_bAutoItemHeight = b;
		if (b)
		{
			UpdateDefItemHeight();
			ReCalcScrollBar();
		}
	}
	EckInline constexpr BOOL GetAutoItemHeight() const { return m_bAutoItemHeight; }

	EckInline constexpr void SetNotifyParentWindow(HWND h) { m_hParent = h; }

	EckInline void SetTextBufferSize(int cch) { m_rsTextBuf.ReSize(cch); }
	EckInline constexpr int GetTextBufferSize() const { return m_rsTextBuf.Size(); }

	LRESULT RequestItem(NMLBNGETDISPINFO& nm)
	{
		EckAssert(nm.Item.idxItem >= 0 && nm.Item.idxItem < GetItemCount());
		nm.Item.pszText = m_rsTextBuf.Data();
		nm.Item.cchText = m_rsTextBuf.Size();
		return FillNmhdrAndSendNotify(nm, m_hParent, NM_LBN_GETDISPINFO);
	}
};
ECK_RTTI_IMPL_BASE_INLINE(CListBoxNew, CWnd);
ECK_NAMESPACE_END