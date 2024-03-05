#pragma once
ECK_DISABLE_ARITHMETIC_OVERFLOW_WARNING

#include "CListView.h"
#include "CtrlGraphics.h"

ECK_NAMESPACE_BEGIN
class CListViewExt :public CListView
{
private:
	HTHEME m_hTheme = NULL;
	int m_iLVType = LV_VIEW_DETAILS;

	BITBOOL m_bOwnerData : 1 = FALSE;
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
		}
		return CListView::OnMsg(hWnd, uMsg, wParam, lParam);
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
				const auto plvnmcd = (NMLVCUSTOMDRAW*)lParam;
				const auto hDC = plvnmcd->nmcd.hdc;
				switch (plvnmcd->nmcd.dwDrawStage)
				{
				case CDDS_PREPAINT:
					return CDRF_NOTIFYITEMDRAW;
				case CDDS_ITEMPREPAINT:
				{
					//auto Header = GetHeaderCtrl();
					//const auto cCol = Header.GetItemCount();
					//auto prcCol = (RECT*)_malloca(cCol * sizeof(RECT));
					//EckCheckMem(prcCol);

					//
					//if (m_bOwnerData)
					//{
					//	RECT rc;
					//	
					//	NMLVDISPINFOW nm{};
					//	nm.item.iItem = (int)plvnmcd->nmcd.dwItemSpec;
					//	nm.item.mask = LVIF_TEXT;
					//	nm.hdr = plvnmcd->nmcd.hdr;
					//	nm.hdr.code = LVN_GETDISPINFOW;
					//	for (nm.item.iSubItem = 0; nm.item.iSubItem < cCol; ++nm.item.iSubItem)
					//	{
					//		rc.left = prcCol[nm.item.iSubItem].cx;
					//		rc.right = prcCol[nm.item.iSubItem].cy;
					//		rc.top = plvnmcd->nmcd.rc.top;
					//		rc.bottom = plvnmcd->nmcd.rc.bottom;
					//		SendNotify(nm);
					//		DrawTextW(hDC, nm.item.pszText, -1, &rc,
					//			DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX | DT_END_ELLIPSIS);
					//	}
					//}
					//_freea(prcCol);
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