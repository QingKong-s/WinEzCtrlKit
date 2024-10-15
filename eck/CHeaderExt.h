#pragma once
#include "CHeader.h"
#include "CBk.h"

#include "ECK.h"

ECK_NAMESPACE_BEGIN
class CHEDragMarker :public CBk
{
	friend class CHeaderExt;
private:

};

class CHeaderExt : public CHeader
{
private:
	struct ITEM
	{
		eck::CRefStrW rsSubText{};
		COLORREF crText{ CLR_DEFAULT };
		COLORREF crBk{ CLR_DEFAULT };
		COLORREF crTextBk{ CLR_DEFAULT };
		UINT uFmt;
	};

	BITBOOL m_bCheckBoxes : 1{};
	BITBOOL m_bAutoDarkMode : 1{ TRUE };
	BITBOOL m_bMoved : 1{};
	BITBOOL m_bDragging : 1{};
	BITBOOL m_bSplitBtnHot : 1{};
	BYTE m_byAlphaColor{ 100 };
	int m_idxHotItem{ -1 };
	int m_idxDragging{ -1 };

	COLORREF m_crText{ CLR_DEFAULT };
	COLORREF m_crBk{ CLR_DEFAULT };
	COLORREF m_crTextBk{ CLR_DEFAULT };

	CEzCDC m_DcAlpha{};

	int m_cxIL{};
	int m_cyIL{};
	HIMAGELIST m_hIL{};

	HTHEME m_hTheme{};
	HTHEME m_hThemeCB{};
	int m_cxCheckBox{};
	SIZE m_sizeSortUp{};
	SIZE m_sizeSortDown{};
	int m_cxSplitBtn{};

	int m_cxClient{};
	int m_cyClient{};

	CRefStrW m_rsTextBuf{ MAX_PATH };
	int m_cchTextBuf{ MAX_PATH };

	std::vector<ITEM> m_vData{};

	int m_iDpi{ USER_DEFAULT_SCREEN_DPI };
	const ECKTHREADCTX* m_pThrCtx{};

	ECK_DS_BEGIN(DPIS)
		ECK_DS_ENTRY_DYN(cxEdge, GetSystemMetrics(SM_CXEDGE))
		ECK_DS_ENTRY_DYN(cyMenuCheck, GetSystemMetrics(SM_CYMENUCHECK))
		;
	ECK_DS_END_VAR(m_Ds);

	LRESULT OnItemPrePaint(NMCUSTOMDRAW* pnmcd)
	{
		//return CDRF_DODEFAULT;
		const int idx = (int)pnmcd->dwItemSpec;
		if (idx < 0 || idx >(int)m_vData.size())
			return CDRF_DODEFAULT;
		const auto& e = m_vData[idx];
		const auto hDC = pnmcd->hdc;
		RECT rc{ pnmcd->rc };
		int xAvailable;

		HDITEMW hdi;
		hdi.mask = HDI_BITMAP | HDI_FORMAT | HDI_IMAGE | HDI_LPARAM |
			HDI_ORDER | HDI_TEXT | HDI_WIDTH;
		hdi.pszText = m_rsTextBuf.Data();
		hdi.cchTextMax = m_cchTextBuf;
		GetItem(idx, &hdi);

		rc.left += m_Ds.cxEdge;
		// 画背景
		int iState;
		if (pnmcd->uItemState & CDIS_SELECTED)
			iState = HIS_PRESSED;
		else if (idx == m_idxHotItem)
			iState = HIS_HOT;
		else
			iState = HIS_NORMAL;
		DrawThemeBackground(m_hTheme, hDC, HP_HEADERITEM, iState, &pnmcd->rc, nullptr);
		// 画拆分按钮
		if (hdi.fmt & HDF_SPLITBUTTON)
		{
			if (idx == m_idxHotItem)
				if (m_bSplitBtnHot)
					iState = HDDS_HOT;
				else
					iState = HDDS_SOFTHOT;
			else
				iState = HDDS_NORMAL;
			RECT rc{ pnmcd->rc };
			rc.left = rc.right - m_cxSplitBtn;
			xAvailable = rc.left;
			DrawThemeBackground(m_hTheme, hDC, HP_HEADERDROPDOWN, iState, &rc, nullptr);
		}
		else
			xAvailable = pnmcd->rc.right;
		// 画检查框
		if (m_bCheckBoxes && (hdi.fmt & HDF_CHECKBOX))
		{
			rc.right = rc.left + m_cxCheckBox;
			DrawThemeBackground(m_hThemeCB, hDC, BP_CHECKBOX,
				(hdi.fmt & HDF_CHECKED) ? CBS_CHECKEDNORMAL : CBS_UNCHECKEDNORMAL, &rc, nullptr);
			rc.left = rc.right + m_Ds.cxEdge;
		}
		// 画图像
		BITMAP bm{};
		int xRight{ xAvailable - m_Ds.cxEdge };
		BOOL bBmp = (hdi.fmt & HDF_BITMAP) && hdi.hbm;
		BOOL bImg = (hdi.fmt & HDF_IMAGE) && hdi.iImage >= 0 && m_hIL;
		if (bBmp || bImg)
		{
			const BOOL bRight = (hdi.fmt & HDF_BITMAP_ON_RIGHT);
			if (bRight)
				if (bBmp && bImg)
				{
					GetObjectW(hdi.hbm, sizeof(bm), &bm);
					xRight -= (m_cxIL + bm.bmWidth + m_Ds.cxEdge * 2);
				}
				else if (bBmp)
				{
					GetObjectW(hdi.hbm, sizeof(bm), &bm);
					xRight -= (bm.bmWidth + m_Ds.cxEdge);
				}
				else/* if (bImg)*/
					xRight -= (m_cxIL + m_Ds.cxEdge);
			else
				if (bBmp && bImg)// hbm绘制在右边，bRight对iImage生效
				{
					bImg = FALSE;
					rc.left += m_Ds.cxEdge;
					ImageList_Draw(m_hIL, hdi.iImage, hDC,
						rc.left, rc.top + (rc.bottom - rc.top - m_cyIL) / 2, ILD_NORMAL);
					rc.left += (m_cxIL + m_Ds.cxEdge);
					// 右边剩下hbm，此处应对称
					GetObjectW(hdi.hbm, sizeof(bm), &bm);
					//xRight -= (bm.bmWidth + m_Ds.cxEdge);
					xRight -= (rc.left - pnmcd->rc.left - m_Ds.cxEdge/*补回边界*/);
				}
				else if (bBmp)// bRight对hbm生效
				{
					bBmp = FALSE;
					rc.left += m_Ds.cxEdge;
					GetObjectW(hdi.hbm, sizeof(bm), &bm);
					const int y = rc.top + (rc.bottom - rc.top - bm.bmHeight) / 2;
					const auto hCDC = CreateCompatibleDC(hDC);
					SelectObject(hCDC, hdi.hbm);
					if (bm.bmBitsPixel == 32)
						AlphaBlend(hDC, rc.left, y, bm.bmWidth, bm.bmHeight,
							hCDC, 0, 0, bm.bmWidth, bm.bmHeight, BlendFuncAlpha);
					else
						BitBlt(hDC, rc.left, y, bm.bmWidth, bm.bmHeight,
							hCDC, 0, 0, SRCCOPY);
					DeleteDC(hCDC);
					rc.left += (bm.bmWidth + m_Ds.cxEdge);
				}
				else/* if (bImg)*/// bRight对iImage生效
				{
					bImg = FALSE;
					rc.left += m_Ds.cxEdge;
					ImageList_Draw(m_hIL, hdi.iImage, hDC,
						rc.left, rc.top + (rc.bottom - rc.top - m_cyIL) / 2, ILD_NORMAL);
					rc.left += (m_cxIL + m_Ds.cxEdge);
				}
		}
		// 画文本
		if (hdi.fmt & HDF_STRING)
		{
			if (e.crTextBk == CLR_DEFAULT)
				SetBkMode(hDC, TRANSPARENT);
			else
			{
				SetBkMode(hDC, OPAQUE);
				SetBkColor(hDC, e.crTextBk);
			}
			if (e.crText == CLR_DEFAULT)
				SetTextColor(hDC, m_crText == CLR_DEFAULT ? m_pThrCtx->crDefText : m_crText);
			else
				SetTextColor(hDC, e.crText);

			rc.right = xRight;
			UINT uDtFlags = DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX | DT_END_ELLIPSIS;
			if (hdi.fmt & HDF_RIGHT)
				uDtFlags |= DT_RIGHT;
			else if (hdi.fmt & HDF_CENTER)
				uDtFlags |= DT_CENTER;
			DrawTextW(hDC, hdi.pszText, -1, &rc, uDtFlags);
			rc.left = rc.right + m_Ds.cxEdge;
		}
		// 画右边图像
		if (bImg)
		{
			ImageList_Draw(m_hIL, hdi.iImage, hDC,
				rc.left, rc.top + (rc.bottom - rc.top - m_cyIL) / 2, ILD_NORMAL);
			rc.left += (m_cxIL + m_Ds.cxEdge);
		}
		if (bBmp)
		{
			const int y = rc.top + (rc.bottom - rc.top - bm.bmHeight) / 2;
			const auto hCDC = CreateCompatibleDC(hDC);
			SelectObject(hCDC, hdi.hbm);
			if (bm.bmBitsPixel == 32)
				AlphaBlend(hDC, rc.left, y, bm.bmWidth, bm.bmHeight,
					hCDC, 0, 0, bm.bmWidth, bm.bmHeight, BlendFuncAlpha);
			else
				BitBlt(hDC, rc.left, y, bm.bmWidth, bm.bmHeight,
					hCDC, 0, 0, SRCCOPY);
			DeleteDC(hCDC);
			rc.left += (bm.bmWidth + m_Ds.cxEdge);
		}
		// 画排序标记
		if (hdi.fmt & HDF_SORTUP)
		{
			rc.left = ((pnmcd->rc.right - pnmcd->rc.left) - m_sizeSortUp.cx) / 2;
			rc.top = 0;
			rc.right = rc.left + m_sizeSortUp.cx;
			rc.bottom = rc.top + m_sizeSortUp.cy;
			DrawThemeBackground(m_hTheme, hDC, HP_HEADERSORTARROW, HSAS_SORTEDUP, &rc, nullptr);
		}
		else if (hdi.fmt & HDF_SORTDOWN)
		{
			rc.left = ((pnmcd->rc.right - pnmcd->rc.left) - m_sizeSortDown.cx) / 2;
			rc.top = 0;
			rc.right = rc.left + m_sizeSortDown.cx;
			rc.bottom = rc.top + m_sizeSortDown.cy;
			DrawThemeBackground(m_hTheme, hDC, HP_HEADERSORTARROW, HSAS_SORTEDDOWN, &rc, NULL);
		}
		return CDRF_SKIPDEFAULT;
	}

	void UpdateStyleOptions(DWORD dwStyle)
	{
		m_bCheckBoxes = IsBitSet(dwStyle, HDS_CHECKBOXES);
	}

	void UpdateThemeMetrics()
	{
		SIZE size;
		const HDC hDC = GetDC(HWnd);
		GetThemePartSize(m_hThemeCB, hDC, BP_CHECKBOX, CBS_UNCHECKEDNORMAL,
			nullptr, TS_DRAW, &size);
		m_cxCheckBox = size.cx;
		GetThemePartSize(m_hTheme, hDC, HP_HEADERSORTARROW, HSAS_SORTEDUP,
			nullptr, TS_DRAW, &m_sizeSortUp);
		GetThemePartSize(m_hTheme, hDC, HP_HEADERSORTARROW, HSAS_SORTEDDOWN,
			nullptr, TS_DRAW, &m_sizeSortUp);
		GetThemePartSize(m_hTheme, hDC, HP_HEADERDROPDOWN, 0,
			nullptr, TS_TRUE, &size);
		if (size.cx > m_Ds.cyMenuCheck)// Windows uses it.
			m_cxSplitBtn = size.cx;
		else
			m_cxSplitBtn = m_Ds.cyMenuCheck;
		ReleaseDC(HWnd, hDC);
	}

	void InitForNewWindow(HWND hWnd)
	{
		m_DcAlpha.Create32(hWnd, 1, 1);
		m_pThrCtx = GetThreadCtx();
		m_hTheme = OpenThemeData(hWnd, L"Header");
		m_hThemeCB = OpenThemeData(hWnd, L"Button");
		UpdateThemeMetrics();

		UpdateStyleOptions(Style);
		m_iDpi = GetDpi(hWnd);
		UpdateDpiSize(m_Ds, m_iDpi);

		m_vData.clear();
		if (const int cCol = GetItemCount(); cCol)
			m_vData.resize(cCol);
	}

	void CleanupForDestroyWindow()
	{
		CloseThemeData(m_hTheme);
		m_hTheme = nullptr;
		CloseThemeData(m_hThemeCB);
		m_hThemeCB = nullptr;
		m_crBk = m_crText = m_crTextBk = CLR_DEFAULT;
	}
public:
	void AttachNew(HWND hWnd) override
	{
		CWnd::AttachNew(hWnd);
		InitForNewWindow(hWnd);
	}

	void DetachNew() override
	{
		CWnd::DetachNew();
		CleanupForDestroyWindow();
	}

	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		switch (uMsg)
		{
		case WM_LBUTTONDBLCLK:
			PostMsg(WM_LBUTTONDOWN, wParam, lParam);
			break;

		case WM_MOUSEMOVE:
		{
			HDHITTESTINFO hdhti;
			hdhti.pt = ECK_GET_PT_LPARAM(lParam);
			hdhti.iItem = -1;
			HitTest(&hdhti);

			if (!m_bMoved)
				m_bMoved = TRUE;

			if (m_bDragging)
			{
				if (hdhti.iItem >= 0)
				{
					RECT rc;
					CHeader::OnMsg(hWnd, HDM_GETITEMRECT, hdhti.iItem, (LPARAM)&rc);
					const int idxOld = m_idxDragging;
					if (hdhti.pt.x < rc.left + (rc.right - rc.left) / 2)
						m_idxDragging = hdhti.iItem;
					else
					{
						HDITEMW hi;
						hi.mask = HDI_ORDER;
						CHeader::OnMsg(hWnd, HDM_GETITEM, hdhti.iItem, (LPARAM)&hi);

						const int cCol = (int)CHeader::OnMsg(hWnd, HDM_GETITEMCOUNT, 0, 0);
						const auto piOrder = (int*)_malloca(cCol * sizeof(int));
						CHeader::OnMsg(hWnd, HDM_GETORDERARRAY, cCol, (LPARAM)piOrder);

						if (hi.iOrder == cCol - 1)
							m_idxDragging = cCol;
						else
							m_idxDragging = piOrder[hi.iOrder + 1];

						_freea(piOrder);
					}
					if (idxOld != m_idxDragging)
						ResetDraggingMark();
				}
			}
			else
			{
				m_idxHotItem = hdhti.iItem;
				const auto b = !!(hdhti.flags & HHT_ONDROPDOWN);
				// !!!标准控件Bug：自定义绘制时懒加载数据未正确计算，导致拆分按钮命中测试不正常
				if (b != m_bSplitBtnHot)
				{
					m_bSplitBtnHot = b;
					if (m_idxHotItem >= 0)
					{
						RECT rc;
						GetItemRect(m_idxHotItem, &rc);
						InvalidateRect(hWnd, &rc, FALSE);
					}
				}
			}

			TRACKMOUSEEVENT tme;
			tme.cbSize = sizeof(tme);
			tme.dwFlags = TME_LEAVE;
			tme.hwndTrack = hWnd;
			TrackMouseEvent(&tme);
		}
		break;

		case WM_MOUSELEAVE:
			m_idxHotItem = -1;
			m_bSplitBtnHot = FALSE;
			break;

		case WM_LBUTTONUP:
		{
			LRESULT lResult = CHeader::OnMsg(hWnd, uMsg, wParam, lParam);// 先call默认过程，因为控件内部可能要重排列

			m_idxDragging = -1;

			HDHITTESTINFO hdhti;
			hdhti.pt = ECK_GET_PT_LPARAM(lParam);
			hdhti.iItem = -1;
			CHeader::OnMsg(hWnd, HDM_HITTEST, 0, (LPARAM)&hdhti);
			m_idxHotItem = hdhti.iItem;

			if (m_bMoved)// 不知道为什么有时候不重画，手动重画一下
			{
				InvalidateRect(hWnd, NULL, FALSE);
				UpdateWindow(hWnd);
				m_bMoved = FALSE;
			}

			return lResult;
		}
		break;

		case WM_SIZE:
			m_cxClient = LOWORD(lParam);
			m_cyClient = HIWORD(lParam);
			break;

		case WM_STYLECHANGED:
		{
			const auto* const p = (STYLESTRUCT*)lParam;
			if (wParam == GWL_STYLE)
				UpdateStyleOptions(p->styleNew);
		}
		break;

		case WM_THEMECHANGED:
		{
			CloseThemeData(m_hTheme);
			m_hTheme = OpenThemeData(hWnd, L"Header");
			CloseThemeData(m_hThemeCB);
			m_hThemeCB = OpenThemeData(hWnd, L"Button");
			UpdateThemeMetrics();
		}
		break;

		case WM_DPICHANGED_BEFOREPARENT:
		{
			m_iDpi = GetDpi(hWnd);
			UpdateDpiSize(m_Ds, m_iDpi);
		}
		break;

		case WM_CREATE:
		{
			InitForNewWindow(hWnd);
		}
		break;

		case WM_DESTROY:
		{
			CleanupForDestroyWindow();
		}
		break;

		case HDM_HITTEST:
		{
			const auto lResult = CHeader::OnMsg(hWnd, uMsg, wParam, lParam);
			if (lResult >= 0)
			{
				const auto p = (HDHITTESTINFO*)lParam;
				// !!!标准控件Bug：自定义绘制时懒加载数据未正确计算，导致拆分按钮命中测试不正常
				if (m_vData[p->iItem].uFmt & HDF_SPLITBUTTON)
				{
					RECT rc;
					GetItemRect(p->iItem, &rc);
					rc.left = rc.right - m_cxSplitBtn;
					if (PtInRect(rc, p->pt))
						p->flags |= HHT_ONDROPDOWN;
				}
			}
			return lResult;
		}
		break;

		case HDM_INSERTITEMW:
		{
			const auto lResult = CHeader::OnMsg(hWnd, uMsg, wParam, lParam);
			if (lResult >= 0)
			{
				const auto it = m_vData.emplace(m_vData.begin() + lResult);
				const auto* const p = (HDITEMW*)lParam;
				if (p->mask & HDI_FORMAT)
					it->uFmt = p->fmt;
			}
			return lResult;
		}
		break;

		case HDM_SETITEMW:
		{
			const auto lResult = CHeader::OnMsg(hWnd, uMsg, wParam, lParam);
			if (lResult >= 0)
			{
				const auto* const p = (HDITEMW*)lParam;
				if (p->mask & HDI_FORMAT)
					m_vData[wParam].uFmt = p->fmt;
			}
			return lResult;
		}
		break;

		case HDM_DELETEITEM:
		{
			const auto lResult = CHeader::OnMsg(hWnd, uMsg, wParam, lParam);
			if (lResult)
				m_vData.erase(m_vData.begin() + wParam);
			return lResult;
		}
		break;

		case HDM_SETIMAGELIST:
		{
			const auto lResult = CHeader::OnMsg(hWnd, uMsg, wParam, lParam);
			if (wParam == HDSIL_NORMAL)
			{
				m_hIL = (HIMAGELIST)lParam;
				ImageList_GetIconSize(m_hIL, &m_cxIL, &m_cyIL);
			}
			return lResult;
		}
		break;
		}
		return CHeader::OnMsg(hWnd, uMsg, wParam, lParam);
	}

	LRESULT OnNotifyMsg(HWND hParent, UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bProcessed) override
	{
		switch (uMsg)
		{
		case WM_NOTIFY:
		{
			switch (((NMHDR*)lParam)->code)
			{
			case NM_CUSTOMDRAW:
			{
				bProcessed = TRUE;
				const auto pnmcd = (NMCUSTOMDRAW*)lParam;
				switch (pnmcd->dwDrawStage)
				{
				case CDDS_PREPAINT:
					return CDRF_NOTIFYITEMDRAW;
				case CDDS_ITEMPREPAINT:
					return OnItemPrePaint(pnmcd);
				}
			}
			return CDRF_DODEFAULT;
			}
		}
		break;
		}
		return CHeader::OnNotifyMsg(hParent, uMsg, wParam, lParam, bProcessed);
	}

	void ResetDraggingMark()
	{

	}
};
ECK_NAMESPACE_END