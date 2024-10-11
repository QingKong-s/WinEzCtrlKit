/*
* WinEzCtrlKit Library
*
* CListViewExt.h ： 列表视图扩展
*
* Copyright(C) 2024 QingKong
*/
#pragma once
ECK_DISABLE_ARITHMETIC_OVERFLOW_WARNING

#include "CListView.h"
#include "CtrlGraphics.h"

ECK_NAMESPACE_BEGIN
/*
CListViewExt封装了ListView的常用扩展功能
===以下ListView的原始功能被忽略===

===以下功能效果仍相同但处理机制已变化===
整体的颜色设置
*/
class CListViewExt :public CListView
{
public:
	ECK_RTTI(CListViewExt);
private:
	struct ITEMINFO
	{
		COLORREF crText;
		COLORREF crTextBk;
		COLORREF crBk;
	};

	// ListView信息
	int m_iViewType{};		// 视图类型
	HIMAGELIST m_hIL[4]{};	// 图像列表
	SIZE m_sizeIL[4]{};		// 图像列表大小
	BITBOOL m_bOwnerData : 1 = FALSE;	// 是否为所有者数据模式
	BITBOOL m_bSubItemImg : 1 = FALSE;	// 是否显示子项图像

	// 图形
	HTHEME m_hTheme{};		// 主题句柄
	int m_iDpi{ USER_DEFAULT_SCREEN_DPI };	// DPI

	// 选项
	COLORREF m_crDefText = CLR_DEFAULT;		// 默认文本颜色
	COLORREF m_crDefBk = CLR_DEFAULT;		// 默认背景颜色
	COLORREF m_crOddLineText = CLR_DEFAULT;	// 奇数行文本颜色
	COLORREF m_crEvenLineText = CLR_DEFAULT;// 偶数行文本颜色
	COLORREF m_crOddLineBk = CLR_DEFAULT;	// 奇数行背景颜色
	COLORREF m_crEvenLineBk = CLR_DEFAULT;	// 偶数行背景颜色

	BITBOOL m_bCustomDraw : 1 = TRUE;		// 是否由控件自动绘制，保留此选项以供特殊情况使用
	BITBOOL m_bAutoDarkMode : 1 = TRUE;		// 自动切换深浅色切换，保留此选项以供特殊情况使用

	ECK_DS_BEGIN(DPIS)
		ECK_DS_ENTRY_DYN(cxEdge, GetSystemMetrics(SM_CXEDGE))
		;
	ECK_DS_END_VAR(m_Ds);

	LRESULT OnItemPrePaint(NMLVCUSTOMDRAW* pnmlvcd)
	{
		if (IsRectEmpty(pnmlvcd->nmcd.rc))
			return CDRF_DODEFAULT;
		const auto hDC = pnmlvcd->nmcd.hdc;
		const int idx = (int)pnmlvcd->nmcd.dwItemSpec;

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
				&pnmlvcd->nmcd.rc, nullptr);
		const size_t idxIL = (m_iViewType == LV_VIEW_ICON ||
			m_iViewType == LV_VIEW_TILE ||
			m_iViewType == LV_VIEW_LIST ? LVSIL_NORMAL : LVSIL_SMALL);
		const auto hIL = m_hIL[idxIL];
		const auto sizeIL = m_sizeIL[idxIL];
		RECT rc;
		LVITEMW li;
		// TODO：其他附属部件
		// 画第一个子项的图像
		if (hIL)
		{
			li.mask = LVIF_IMAGE;
			li.iItem = idx;
			li.iSubItem = 0;
			GetItem(&li);
			GetItemRect(idx, &rc, LVIR_ICON);
			RECT rc0{ 0,0,sizeIL.cx,sizeIL.cy };
			AdjustRectToFitAnother(rc0, rc);

			ImageList_Draw(hIL, li.iImage, hDC,
				rc0.left, rc0.top + (m_iViewType == LV_VIEW_ICON ? 1 : 0),
				ILD_NORMAL | ILD_TRANSPARENT);
		}
		// 画文本
		WCHAR sz[MAX_PATH];// 无论如何，ListView只显示前255个字符
		li.mask = LVIF_TEXT | LVIF_IMAGE;
		li.pszText = sz;
		li.cchTextMax = MAX_PATH;
		li.iItem = idx;

		SetBkMode(hDC, TRANSPARENT);
		SetTextColor(hDC, m_crDefText);// HACK : 增加文本背景支持

		if (m_iViewType == LV_VIEW_DETAILS)
		{
			auto Header = GetHeaderCtrl();
			const auto cCol = Header.GetItemCount();
			HDITEMW hdi;
			hdi.mask = HDI_FORMAT;
			int cxImg = 0;
			for (li.iSubItem = 0; li.iSubItem < cCol; ++li.iSubItem)
			{
				GetItem(&li);
				// 如果可能，绘制后续子项图像
				if (li.iImage >= 0 && hIL)
					if (li.iSubItem == 0)
						cxImg = m_Ds.cxEdge;
					else if (m_bSubItemImg)
					{
						RECT rcImg;
						GetSubItemRect(idx, li.iSubItem, &rcImg, LVIR_ICON);
						cxImg = rcImg.right - rcImg.left + m_Ds.cxEdge * 3;
						ImageList_Draw(hIL, li.iImage, hDC,
							rcImg.left,
							(pnmlvcd->nmcd.rc.bottom - pnmlvcd->nmcd.rc.top -
								(m_sizeIL[idxIL].cy)) / 2 + pnmlvcd->nmcd.rc.top,
							ILD_NORMAL | ILD_TRANSPARENT);
					}
					else
						cxImg = m_Ds.cxEdge * 2;
				else
					cxImg = m_Ds.cxEdge * 2;

				Header.GetItem(li.iSubItem, &hdi);
				UINT uDtFlags = DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX | DT_SINGLELINE;
				if (IsBitSet(hdi.fmt, HDF_RIGHT))
					uDtFlags |= DT_RIGHT;
				else if (IsBitSet(hdi.fmt, HDF_CENTER))
					uDtFlags |= DT_CENTER;
				GetSubItemRect(idx, li.iSubItem, &rc, LVIR_LABEL);
				rc.left += cxImg;
				DrawTextW(hDC, li.pszText, -1, &rc, uDtFlags);
			}
		}
		else
		{
			GetItem(&li);
			GetItemRect(idx, &rc, LVIR_LABEL);
			DrawTextW(hDC, li.pszText, -1, &rc,
				DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX | DT_SINGLELINE | DT_CENTER);
		}
		return CDRF_SKIPDEFAULT;
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
			case NM_CUSTOMDRAW:// for Header
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
		}
		return 0;

		case WM_DPICHANGED_BEFOREPARENT:
			m_iDpi = GetDpi(hWnd);
			UpdateDpiSize(m_Ds, m_iDpi);
			break;

		case WM_CREATE:
		{
			const auto lResult = CListView::OnMsg(hWnd, uMsg, wParam, lParam);
			if (!lResult)
			{
				m_bOwnerData = IsBitSet(GetStyle(), LVS_OWNERDATA);
				m_hTheme = OpenThemeData(hWnd, L"ListView");
				m_iViewType = (int)GetView();
				m_bSubItemImg = IsBitSet(GetLVExtendStyle(), LVS_EX_SUBITEMIMAGES);
				m_iDpi = GetDpi(hWnd);
				UpdateDpiSize(m_Ds, m_iDpi);
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
		break;

		case LVM_SETIMAGELIST:
		{
			const auto lResult = CListView::OnMsg(hWnd, uMsg, wParam, lParam);
			if (wParam >= 0 && wParam <= 3)
			{
				m_hIL[wParam] = (HIMAGELIST)lParam;
				ImageList_GetIconSize(m_hIL[wParam],
					(int*)&m_sizeIL[wParam].cx, (int*)&m_sizeIL[wParam].cy);
			}
			return lResult;
		}
		break;

		case LVM_SETEXTENDEDLISTVIEWSTYLE:
		{
			const auto lResult = CListView::OnMsg(hWnd, uMsg, wParam, lParam);
			m_bSubItemImg = IsBitSet(GetLVExtendStyle(), LVS_EX_SUBITEMIMAGES);
			return lResult;
		}
		break;
		}
		return CListView::OnMsg(hWnd, uMsg, wParam, lParam);
	}

	LRESULT OnNotifyMsg(HWND hParent, UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bProcessed) override
	{
		if (m_bCustomDraw)
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
						return OnItemPrePaint(pnmlvcd);
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
ECK_RTTI_IMPL_BASE_INLINE(CListViewExt, CListView);
ECK_NAMESPACE_END