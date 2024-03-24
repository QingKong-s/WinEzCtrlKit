#pragma once
ECK_DISABLE_ARITHMETIC_OVERFLOW_WARNING

#include "CListView.h"
#include "CtrlGraphics.h"

ECK_NAMESPACE_BEGIN
class CListViewExt :public CListView
{
private:
	HTHEME m_hTheme = NULL;
	int m_iViewType = LV_VIEW_DETAILS;
	HIMAGELIST m_hIL = NULL;

	BITBOOL m_bOwnerData : 1 = FALSE;
	BITBOOL m_bCustomDraw : 1 = 1;

public:
	ECK_CWND_SINGLEOWNER;

	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		switch (uMsg)
		{
		case WM_THEMECHANGED:
			CloseThemeData(m_hTheme);
			m_hTheme = OpenThemeData(hWnd, L"ListView");
			return 0;
		case WM_CREATE:
			m_bOwnerData = IsBitSet(GetStyle(), LVS_OWNERDATA);
			m_hTheme = OpenThemeData(hWnd, L"ListView");
			break;
		case LVM_SETVIEW:
		{
			const auto lResult = CListView::OnMsg(hWnd, uMsg, wParam, lParam);
			if (lResult == 1)
				m_iViewType = (int)wParam;
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
				{
					const auto hDC = pnmlvcd->nmcd.hdc;
					const int idx = (int)pnmlvcd->nmcd.dwItemSpec;

					auto Header = GetHeaderCtrl();
					const auto cCol = Header.GetItemCount();

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
						ImageList_DrawEx(m_hIL, li.iImage, hDC,
							rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
							CLR_NONE, CLR_NONE, ILD_NORMAL);
					}
					
					WCHAR sz[MAX_PATH];
					li.mask = LVIF_TEXT;
					li.pszText = sz;
					li.cchTextMax = MAX_PATH;
					li.iItem = idx;

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