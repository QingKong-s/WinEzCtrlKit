#pragma once
ECK_DISABLE_ARITHMETIC_OVERFLOW_WARNING

#include "CListView.h"
#include "CtrlGraphics.h"

ECK_NAMESPACE_BEGIN
class CListViewExt :public CListView
{
private:
	struct ITEMINFO
	{
		COLORREF crText;
		COLORREF crTextBk;
		COLORREF crBk;
	};
	HTHEME m_hTheme = NULL;
	int m_iViewType = LV_VIEW_DETAILS;
	HIMAGELIST m_hIL = NULL;
	SIZE m_sizeILBit{};
	COLORREF m_crDefText = CLR_INVALID;

	BITBOOL m_bOwnerData : 1 = FALSE;
	BITBOOL m_bCustomDraw : 1 = 1;

	void UpdateCtrlColor()
	{
		COLORREF crBk;
		GetItemsViewForeBackColor(m_crDefText, crBk);
		SetTextClr(m_crDefText);
		SetTextBKClr(crBk);
		SetBkClr(crBk);
	}
public:
	ECK_CWND_SINGLEOWNER;

	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		switch (uMsg)
		{
		case WM_NOTIFY:
		{
			switch (((NMHDR*)lParam)->code)
			{
			case NM_CUSTOMDRAW:
			{
				const auto pnmcd = (NMCUSTOMDRAW*)lParam;
				switch (pnmcd->dwDrawStage)
				{
				case CDDS_PREPAINT:
					return CDRF_NOTIFYITEMDRAW;
				case CDDS_ITEMPREPAINT:
				{
					const HDC hDC = pnmcd->hdc;
					SetTextColor(hDC, m_crDefText);
				}
				return CDRF_DODEFAULT;
				}
			}
			return CDRF_DODEFAULT;
			}
		}
		break;
		case WM_THEMECHANGED:
		{
			CloseThemeData(m_hTheme);
			m_hTheme = OpenThemeData(hWnd, L"ListView");
			UpdateCtrlColor();
		}
		return 0;
		case WM_CREATE:
		{
			const auto lResult = CListView::OnMsg(hWnd, uMsg, wParam, lParam);
			if (!lResult)
			{
				m_bOwnerData = IsBitSet(GetStyle(), LVS_OWNERDATA);
				m_hTheme = OpenThemeData(hWnd, L"ListView");
				UpdateCtrlColor();
			}
			return lResult;
		}
		break;
		case LVM_SETVIEW:
		{
			const auto lResult = CListView::OnMsg(hWnd, uMsg, wParam, lParam);
			if (lResult == 1)
				m_iViewType = (int)wParam;
			return lResult;
		}
		
		case LVM_SETIMAGELIST:
		{
			const auto lResult = CListView::OnMsg(hWnd, uMsg, wParam, lParam);
			m_hIL = (HIMAGELIST)lParam;
			ImageList_GetIconSize(m_hIL, (int*)&m_sizeILBit.cx, (int*)&m_sizeILBit.cy);
			return lResult;
		}

		}
		return CListView::OnMsg(hWnd, uMsg, wParam, lParam);
	}

	LRESULT OnNotifyMsg(HWND hParent, UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bProcessed) override
	{
		if(m_bCustomDraw)
		switch (uMsg)
		{
		case WM_NOTIFY:
		{
			switch (((NMHDR*)lParam)->code)
			{
			case NM_CUSTOMDRAW:
			{
				bProcessed = TRUE;
				const auto pnmlvcd = (NMLVCUSTOMDRAW*)lParam;
				
				switch (pnmlvcd->nmcd.dwDrawStage)
				{
				case CDDS_PREPAINT:
					return CDRF_NOTIFYITEMDRAW;
				case CDDS_ITEMPREPAINT:
					//return CDRF_DODEFAULT;
				{
					const auto hDC = pnmlvcd->nmcd.hdc;
					const int idx = (int)pnmlvcd->nmcd.dwItemSpec;

					auto Header = GetHeaderCtrl();
					const auto cCol = Header.GetItemCount();

					if (idx == 10)
						FillRect(hDC, &pnmlvcd->nmcd.rc, CreateSolidBrush(Colorref::Red));

					int iState;
					if (GetItemState(idx, LVIS_SELECTED) == LVIS_SELECTED)// 选中
					{
						if (pnmlvcd->nmcd.uItemState & CDIS_HOT)
							iState = LISS_HOTSELECTED;
						else
							iState = LISS_SELECTED;
					}
					else if (pnmlvcd->nmcd.uItemState & CDIS_HOT)
						iState = LISS_HOT;
					else
						iState = 0;
					if (iState)
						DrawThemeBackground(m_hTheme, hDC, LVP_LISTITEM, iState,
							&pnmlvcd->nmcd.rc, NULL);
					
					RECT rc;
					LVITEMW li;
					if (m_hIL)
					{
						li.mask = LVIF_IMAGE;
						li.iItem = idx;
						li.iSubItem = 0;
						GetItem(&li);
						GetItemRect(idx, &rc, LVIR_ICON);
						RECT rc0{ 0,0,m_sizeILBit.cx,m_sizeILBit.cy };
						AdjustRectToFitAnother(rc0, rc);
						ImageList_Draw(m_hIL, li.iImage, hDC,
							rc0.left, rc0.top, ILD_NORMAL | ILD_TRANSPARENT);
					}

					WCHAR sz[MAX_PATH];
					li.mask = LVIF_TEXT;
					li.pszText = sz;
					li.cchTextMax = MAX_PATH;
					li.iItem = idx;

					SetBkMode(hDC, TRANSPARENT);
					SetTextColor(hDC, m_crDefText);// HACK : 增加文本背景支持
					HDITEMW hdi;
					hdi.mask = HDI_FORMAT;
					for (li.iSubItem = 0; li.iSubItem < cCol; ++li.iSubItem)
					{
						GetSubItemRect(idx, li.iSubItem, &rc, LVIR_LABEL);
						GetItem(&li);
						Header.GetItem(li.iSubItem, &hdi);
						UINT uDtFlags = DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX | DT_SINGLELINE;
						if (IsBitSet(hdi.fmt, HDF_RIGHT))
							uDtFlags |= DT_RIGHT;
						else if (IsBitSet(hdi.fmt, HDF_CENTER))
							uDtFlags |= DT_CENTER;
						DrawTextW(hDC, li.pszText, -1, &rc, uDtFlags);
					}
				}
				return CDRF_SKIPDEFAULT;
				}
			}
			break;
			}
		}
		break;
		}
		return CListView::OnNotifyMsg(hParent, uMsg, wParam, lParam, bProcessed);
	}
};
ECK_NAMESPACE_END