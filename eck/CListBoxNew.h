/*
* WinEzCtrlKit Library
*
* CListBoxNew.h ： 所有者数据模式的列表框
*
* Copyright(C) 2023 QingKong
*/
#pragma once
#include "CWnd.h"
#include "GraphicsHelper.h"

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

struct NMLBNCUSTOMDRAW
{
	NMECKCTRLCUSTOMDRAW nmcd;
};

#pragma pack(push, ECK_CTRLDATA_ALIGN)
struct CTRLDATA_LBN
{
	int iVer = 0;


	// 
};
#pragma pack(pop)

class CListBoxNew :public CWnd
{
public:
	ECK_RTTI(CListBoxNew);
private:
	struct ITEM
	{
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
	HFONT m_hFont = nullptr;// 不需要释放

	HTHEME m_hTheme = nullptr;

	int m_cxClient = 0,
		m_cyClient = 0;

	int m_cyItem = 24;

	COLORREF m_crBkg = CLR_DEFAULT;
	COLORREF m_crText = CLR_DEFAULT;
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
	BITBOOL m_bExtendSel : 1 = FALSE;
	BITBOOL m_bAllowDrag : 1 = FALSE;

	BITBOOL m_bHasFocus : 1 = FALSE;
	BITBOOL m_bLBtnDown : 1 = FALSE;
	BITBOOL m_bRBtnDown : 1 = FALSE;
	BITBOOL m_bUserItemHeight : 1 = FALSE;
	BITBOOL m_bFocusIndicatorVisible : 1 =
#ifdef _DEBUG
		TRUE;
#else
		FALSE;
#endif
	BITBOOL m_bNmDragging : 1 = FALSE;
	BITBOOL m_bTrackComboBoxList : 1 = FALSE;
	BITBOOL m_bProtectCapture : 1 = FALSE;

	HWND m_hComboBox = nullptr;// 关联的组合框句柄，可以是任何窗口

	int m_iDpi = USER_DEFAULT_SCREEN_DPI;

	ECK_DS_BEGIN(DPIS)
		ECK_DS_ENTRY(cyItemDef, 24)
		ECK_DS_ENTRY(cxTextPadding, 3)
		;
	ECK_DS_END_VAR(m_Ds);


	BOOL OnCreate(HWND hWnd, CREATESTRUCTW* pcs)
	{
		m_iDpi = GetDpi(hWnd);
		UpdateDpiSize(m_Ds, m_iDpi);
		m_cyItem = m_Ds.cyItemDef;

		m_DC.Create(hWnd);
		SetBkMode(m_DC.GetDC(), TRANSPARENT);

		SetItemsViewTheme();
		m_hTheme = OpenThemeData(hWnd, L"ListView");
		return TRUE;
	}

	void OnPaint(HWND hWnd,WPARAM wParam)
	{
		PAINTSTRUCT ps;
		BeginPaint(hWnd, &ps);

		const auto* const ptc = GetThreadCtx();
		SetDCBrushColor(m_DC.GetDC(),
			(m_crBkg == CLR_DEFAULT) ? ptc->crDefBkg : m_crBkg);
		FillRect(m_DC.GetDC(), &ps.rcPaint, GetStockBrush(DC_BRUSH));

		const int idxTop = std::max(m_idxTop + (int)ps.rcPaint.top / m_cyItem - 1, m_idxTop);
		const int idxBottom = std::min(m_idxTop + (int)ps.rcPaint.bottom / m_cyItem + 1, 
			(int)m_vItem.size() - 1);
		if (idxTop >= 0 && idxBottom >= 0)
		{
			SetTextColor(m_DC.GetDC(),
				(m_crText == CLR_DEFAULT) ? ptc->crDefText : m_crText);
			RECT rc;
			GetItemRect(idxTop, rc);
			for (int i = idxTop; i <= idxBottom; ++i)
			{
				PaintItem(i, rc);
				rc.top += m_cyItem;
				rc.bottom += m_cyItem;
			}
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
		ScrollWindow(hWnd, 0, yOld - si.nPos, nullptr, nullptr);
		UpdateWindow(hWnd);
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
		m_bLBtnDown = TRUE;
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

				NMHDR nm;
				FillNmhdr(nm, NM_LBN_LBTNDOWN);
				SendMessageW(m_hComboBox, WM_NOTIFY, nm.idFrom, (LPARAM)&nm);
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
		const int cyDeltaLine = (m_cyItem * 3 >= m_cyClient) ? (m_cyClient * 2 / 3) : m_cyItem * 3;
		si.nPos += (-zDelta / WHEEL_DELTA * cyDeltaLine);
		SetSbInfo(SB_VERT, &si);
		GetSbInfo(SB_VERT, &si);
		ReCalcTopItem();
		ScrollWindow(hWnd, 0, yOld - si.nPos, nullptr, nullptr);
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

		NMLBNCUSTOMDRAW cd{};
		cd.nmcd.hDC = hCDC;
		cd.nmcd.iStage = NMECDS_PREDRAW;
		cd.nmcd.rcItem = rcItem;
		cd.nmcd.idx = idx;
		if (FillNmhdrAndSendNotify(cd, NM_LBN_CUSTOMDRAW) == NMECDR_SKIPDEF)
			return;

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
			DrawThemeBackground(m_hTheme, hCDC, LVP_LISTITEM, iState, &rcItem, nullptr);
		if (m_bFocusIndicatorVisible && m_idxFocus == idx)
		{
			RECT rc{ rcItem };
			InflateRect(rc, -1, -1);
			DrawFocusRect(hCDC, &rc);
		}

		cd.nmcd.iStage = NMECDS_POSTDRAWBK;
		if (SendNotify(cd) == NMECDR_SKIPDEF)
			return;

		NMLBNGETDISPINFO nm{};
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
		EckDbgPrint(L"dismiss up");
		NMHDR nm;
		FillNmhdr(nm, NM_LBN_DISMISS);
		return SendMessageW(m_hComboBox, WM_NOTIFY, nm.idFrom, (LPARAM)&nm);
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
				NMECKMOUSENOTIFY nm;
				nm.pt = ECK_GET_PT_LPARAM(lParam);
				nm.uKeyFlags = (UINT)wParam;
				FillNmhdrAndSendNotify(nm, NM_RCLICK);
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
					FillNmhdrAndSendNotify(nm, NM_LBN_ENDDRAG);
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
			FillNmhdrAndSendNotify(nm, NM_SETFOCUS);
		}
		break;

		case WM_KILLFOCUS:
		{
			m_bHasFocus = FALSE;

			NMFOCUS nm;
			nm.hWnd = (HWND)wParam;
			FillNmhdrAndSendNotify(nm, NM_KILLFOCUS);
		}
		break;

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

		case WM_THEMECHANGED:
		{
			CloseThemeData(m_hTheme);
			m_hTheme = OpenThemeData(hWnd, L"ListView");
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
			m_bHasFocus = m_bLBtnDown = m_bUserItemHeight =
				m_bNmDragging = m_bTrackComboBoxList = FALSE;
			m_hComboBox = nullptr;
		}
		break;
		}

		return CWnd::OnMsg(hWnd, uMsg, wParam, lParam);
	}

	ECK_CWND_CREATE;
	HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, HMENU hMenu, PCVOID pData = nullptr) override
	{
		return IntCreate(dwExStyle, WCN_LISTBOXNEW, pszText, dwStyle,
			x, y, cx, cy, hParent, hMenu, g_hInstance, this);
	}

	EckInline void SetItemCount(int cItem)
	{
		m_vItem.resize(cItem);
		CheckOldData();
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
		rc = { 0,GetItemY(idx),m_cxClient };
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

	EckInline void SetItemHeight(int cy)
	{
		m_cyItem = cy;
		ReCalcScrollBar();
		ReCalcTopItem();
	}

	EckInline int GetItemHeight() const { return m_cyItem; }

#pragma region 组合框交互
	EckInline constexpr void SetComboBox(HWND h) { m_hComboBox = h; }
	EckInline constexpr HWND GetComboBox() const { return m_hComboBox; }

	void CbEnterTrack()
	{
		SetCapture(HWnd);
		m_bTrackComboBoxList = TRUE;
	}

	void CbLeaveTrack()
	{
		if (m_bTrackComboBoxList)
		{
			ReleaseCapture();
			m_bTrackComboBoxList = FALSE;
		}
	}

	void CbBeginProtectCapture()
	{
		EckAssert(m_hComboBox && !m_bProtectCapture);
		m_bProtectCapture = TRUE;
	}

	void CbEndProtectCapture()
	{
		EckAssert(m_hComboBox && m_bProtectCapture);
		m_bProtectCapture = FALSE;
	}
#pragma endregion 组合框交互

	EckInline constexpr void SetMultiSel(BOOL bMultiSel) { m_bMultiSel = bMultiSel; }
	EckInline constexpr BOOL GetMultiSel() const { return m_bMultiSel; }

	EckInline constexpr void SetExtendSel(BOOL bExtendSel) { m_bExtendSel = bExtendSel; }
	EckInline constexpr BOOL GetExtendSel() const { return m_bExtendSel; }
};
ECK_RTTI_IMPL_BASE_INLINE(CListBoxNew, CWnd);
ECK_NAMESPACE_END