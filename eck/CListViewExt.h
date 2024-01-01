#pragma once
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
			[[fallthrough]]
		case WM_CREATE:
			m_bOwnerData = IsBitSet(GetStyle(), LVS_OWNERDATA);
			m_hTheme = OpenThemeData(hWnd, L"ListView");
			break;
		}
		return CListView::OnMsg(hWnd, uMsg, wParam, lParam);
	}

	BOOL OnNotifyMsg(HWND hParent, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult) override
	{
		switch (uMsg)
		{
		case WM_NOTIFY:
		{
			switch (((NMHDR*)lParam)->code)
			{
			case NM_CUSTOMDRAW:
			{
				const auto plvnmcd = (NMLVCUSTOMDRAW*)lParam;
				const auto hDC = plvnmcd->nmcd.hdc;
				switch (plvnmcd->nmcd.dwDrawStage)
				{
				case CDDS_PREPAINT:
					lResult = CDRF_NOTIFYITEMDRAW;
					return TRUE;
				case CDDS_ITEMPREPAINT:
				{
					auto Header = GetHeaderCtrl();
					const auto cCol = Header.GetItemCount();
					auto psizeCol = (SIZE*)_malloca(cCol * sizeof(SIZE));
					DrawListViewItemBackground(m_hTheme, m_iLVType, &Header, plvnmcd, cCol, psizeCol);
					if (m_bOwnerData)
					{
						RECT rc;

						NMLVDISPINFOW nm{};
						nm.item.iItem = (int)plvnmcd->nmcd.dwItemSpec;
						nm.item.mask = LVIF_TEXT;
						nm.hdr = plvnmcd->nmcd.hdr;
						nm.hdr.code = LVN_GETDISPINFOW;
						for (nm.item.iSubItem = 0; nm.item.iSubItem < cCol; ++nm.item.iSubItem)
						{
							rc.left = psizeCol[nm.item.iSubItem].cx;
							rc.right = psizeCol[nm.item.iSubItem].cy;
							rc.top = plvnmcd->nmcd.rc.top;
							rc.bottom = plvnmcd->nmcd.rc.bottom;
							SendNotify(nm);
							DrawTextW(hDC, nm.item.pszText, -1, &rc,
								DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX | DT_END_ELLIPSIS);
						}
					}
					_freea(psizeCol);
					lResult = CDRF_SKIPDEFAULT;
				}
				return TRUE;
				}
			}
			break;
			}
		}
		break;
		}
		return FALSE;
	}
};
ECK_NAMESPACE_END