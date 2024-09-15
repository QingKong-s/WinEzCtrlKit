/*
* WinEzCtrlKit Library
*
* CTaskGroupList.h ： 任务列表组
*
* Copyright(C) 2023 QingKong
*/
#pragma once
#include "CListView.h"

ECK_NAMESPACE_BEGIN
struct TGLSUBTASK
{
	PCWSTR pszText;
	UINT uFlags;
};

struct NMTGLCLICKED
{
	NMHDR nmhdr;
	int idxItem;
	UINT uPart;
	int idxSubTask;
};

enum
{
	TGLP_NONE,
	TGLP_ICON,
	TGLP_SECTIONTITLE,
	TGLP_SUBTASK
};

class CTaskGroupList :public CListView
{
private:
	const static int c_iPadding = 4;
	const static int c_iSubTaskPadding = 8;

	struct SUBTASK
	{
		CRefStrW rsText;
		UINT uFlags;
		int cx;
	};

	struct TASKITEM
	{
		CRefStrW rsText;
		int idxImage;
		std::vector<SUBTASK> SubTasks;
		int cx;
	};

	std::vector<TASKITEM> m_Items{};

	HTHEME m_hthControlPanel = nullptr;
	HTHEME m_hthListView = nullptr;
	HDC m_hCDCAuxiliary = nullptr;
	HIMAGELIST m_hImageList = nullptr;

	int m_iIdealWidth = 0;
	int m_cxClient = 0;

	int m_cxSubTaskPadding = 0;
	int m_cxPadding = 0;
	
	int m_cxIcon = 0, 
		m_cyIcon = 0;
	int m_cySectionTitle = 0;
	int m_cySubTask = 0;

	int m_idxHot = -1;
	BOOL m_bSectionTitleHot = FALSE;
	int m_idxHotSubTask = -1;

	int m_idxPressed = -1;
	BOOL m_bSectionTitlePressed = FALSE;
	int m_idxPressedSubTask = -1;

	BOOL m_bLBtnDown = FALSE;

	void RefreshThemeRes()
	{
		CloseThemeData(m_hthControlPanel);
		CloseThemeData(m_hthListView);
		m_hthControlPanel = OpenThemeData(m_hWnd, L"ControlPanel");
		m_hthListView = OpenThemeData(m_hWnd, L"ListView");

		RECT rc;
		GetThemeTextExtent(m_hthControlPanel, m_hCDCAuxiliary, CPANEL_SECTIONTITLELINK, CPSTL_NORMAL,
			L"bp", -1, DT_SINGLELINE, nullptr, &rc);
		m_cySectionTitle = rc.bottom - rc.top;

		GetThemeTextExtent(m_hthControlPanel, m_hCDCAuxiliary, CPANEL_CONTENTLINK, CPCL_NORMAL,
			L"bp", -1, DT_SINGLELINE, nullptr, &rc);
		m_cySubTask = rc.bottom - rc.top;

		eck::LVSetItemHeight(m_hWnd, std::max(m_cyIcon, m_cySectionTitle + m_cySubTask + m_cxPadding * 2));
	}
public:
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
				if (((NMHDR*)lParam)->hwndFrom!=GetHWND())
					break;

				auto pnmlcd = (NMLVCUSTOMDRAW*)lParam;

				switch (pnmlcd->nmcd.dwDrawStage)
				{
				case CDDS_PREPAINT:
					return CDRF_NOTIFYITEMDRAW;
				case CDDS_ITEMPREPAINT:
				{
					auto& Item = m_Items[pnmlcd->nmcd.dwItemSpec];
					HDC hDC = pnmlcd->nmcd.hdc;
					int cxPadding = m_cxPadding;
					//////////////////////画项目背景
					int iState;
					if (IsBitSet(pnmlcd->nmcd.uItemState, CDIS_SELECTED))
					{
						if (IsBitSet(pnmlcd->nmcd.uItemState, CDIS_HOT))
							iState = LISS_HOTSELECTED;
						else
							iState = LISS_SELECTED;
					}
					else if (IsBitSet(pnmlcd->nmcd.uItemState, CDIS_HOT))
						iState = LISS_HOT;
					else
						iState = 0;

					if (iState)
						DrawThemeBackground(m_hthListView, hDC, LVP_LISTITEM, iState, &pnmlcd->nmcd.rc, nullptr);
					//////////////////////画图标
					int x = cxPadding + pnmlcd->nmcd.rc.left, y = cxPadding + pnmlcd->nmcd.rc.top;
					if (Item.idxImage >= 0)
						ImageList_Draw(m_hImageList, Item.idxImage, hDC, x, y, ILD_NORMAL);
					x += (m_cxIcon + m_cxSubTaskPadding);
					//////////////////////画节标题
					if ((m_idxHot == pnmlcd->nmcd.dwItemSpec && m_bSectionTitleHot) ||
						(m_idxPressed == pnmlcd->nmcd.dwItemSpec && m_bSectionTitlePressed))
						iState = CPSTL_HOT;
					else
						iState = CPSTL_NORMAL;
					RECT rc{ x,pnmlcd->nmcd.rc.top,pnmlcd->nmcd.rc.right,pnmlcd->nmcd.rc.bottom };
					DrawThemeTextEx(m_hthControlPanel, hDC, CPANEL_SECTIONTITLELINK, iState,
						Item.rsText.Data(), Item.rsText.Size(), DT_SINGLELINE, &rc, nullptr);
					//////////////////////画子任务
					rc.top += (m_cySectionTitle + cxPadding);

					EckCounter(Item.SubTasks.size(), i)
					{
						auto& sub = Item.SubTasks[i];

						if (m_idxPressed == pnmlcd->nmcd.dwItemSpec && m_idxPressedSubTask == i)
							iState = CPCL_PRESSED;
						else if (m_idxHot == pnmlcd->nmcd.dwItemSpec && m_idxHotSubTask == i)
							iState = CPCL_HOT;
						else
							iState = CPCL_NORMAL;

						DrawThemeTextEx(m_hthControlPanel, hDC, CPANEL_CONTENTLINK, iState,
							sub.rsText.Data(), sub.rsText.Size(), DT_SINGLELINE, &rc, nullptr);

						rc.left += (sub.cx + m_cxSubTaskPadding * 2);
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

	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		switch (uMsg)
		{
		case WM_MOUSEMOVE:
		{
			POINT pt = ECK_GET_PT_LPARAM(lParam);
			LRESULT lResult = DefSubclassProc(hWnd, uMsg, wParam, lParam);
			int idxHot = GetHotItem();
			BOOL bRedraw = FALSE, bHotChanged = FALSE;
			if (m_idxHot != idxHot)
				bHotChanged = TRUE;
			m_idxHot = idxHot;
			if (idxHot >= 0)
			{
				RECT rc;
				GetItemRect(idxHot, &rc);
				int idxHotSubTask = 1;
				UINT uPart = HitTestTask(&rc, idxHot, pt, &idxHotSubTask);
				if (uPart == TGLP_SECTIONTITLE)
				{
					if (!m_bSectionTitleHot)
						bRedraw = TRUE;
					m_bSectionTitleHot = TRUE;
				}
				else
				{
					if (m_bSectionTitleHot)
						bRedraw = TRUE;
					m_bSectionTitleHot = FALSE;
					if (uPart == TGLP_SUBTASK)
					{
						if (m_idxHotSubTask != idxHotSubTask)
							bRedraw = TRUE;
						m_idxHotSubTask = idxHotSubTask;
					}
					else
					{
						if (m_idxHotSubTask != -1)
							bRedraw = TRUE;
						m_idxHotSubTask = -1;
					}
				}

				if (bHotChanged || bRedraw)
					RedrawItems(idxHot, idxHot);
			}

			TRACKMOUSEEVENT tme;
			tme.cbSize = sizeof(TRACKMOUSEEVENT);
			tme.dwFlags = TME_LEAVE;
			tme.hwndTrack = hWnd;
			TrackMouseEvent(&tme);

			return lResult;
		}

		case WM_SIZE:
			m_cxClient = LOWORD(lParam);
			break;

		case WM_MOUSELEAVE:
		{
			m_idxHot = -1;
			m_idxHotSubTask = -1;
			m_bSectionTitleHot = FALSE;
		}
		break;

		case WM_LBUTTONDOWN:
		{
			LVHITTESTINFO lvhti;
			lvhti.pt = ECK_GET_PT_LPARAM(lParam);
			HitTest(&lvhti);
			m_idxPressed = lvhti.iItem;

			if (lvhti.iItem >= 0)
			{
				RECT rc;
				GetItemRect(lvhti.iItem, &rc);
				int idxSubTask = -1;
				UINT uPart = HitTestTask(&rc, lvhti.iItem, lvhti.pt, &idxSubTask);
				m_bSectionTitlePressed = (uPart == TGLP_SECTIONTITLE);
				m_idxPressedSubTask = idxSubTask;
				if (uPart == TGLP_SECTIONTITLE || uPart == TGLP_SUBTASK)
				{
					m_bLBtnDown = TRUE;
					SetCapture(hWnd);
					RedrawItems(lvhti.iItem, lvhti.iItem);
					return 0;
				}
			}
			else
			{
				m_bSectionTitlePressed = FALSE;
				m_idxPressedSubTask = -1;
			}
		}
		break;

		case WM_LBUTTONUP:
		{
			if (!m_bLBtnDown)
				break;
			m_bLBtnDown = FALSE;
			ReleaseCapture();

			LVHITTESTINFO lvhti;
			lvhti.pt = ECK_GET_PT_LPARAM(lParam);
			HitTest(&lvhti);

			if (lvhti.iItem >= 0 && lvhti.iItem == m_idxPressed)
			{
				RECT rc;
				GetItemRect(lvhti.iItem, &rc);
				int idxSubTask = -1;
				UINT uPart = HitTestTask(&rc, lvhti.iItem, lvhti.pt, &idxSubTask);

				if ((m_bSectionTitlePressed && uPart == TGLP_SECTIONTITLE) ||
					(m_idxPressedSubTask >= 0 && m_idxPressedSubTask == idxSubTask))
				{
					NMTGLCLICKED nm;
					nm.idxItem = lvhti.iItem;
					nm.uPart = uPart;
					nm.idxSubTask = idxSubTask;
					FillNmhdrAndSendNotify(nm, NM_TGL_TASKCLICKED);
				}
			}

			int idxOld = m_idxPressed;
			m_idxPressed = -1;
			m_bSectionTitlePressed = FALSE;
			m_idxHotSubTask = -1;
			RedrawItems(idxOld, idxOld);
		}
		return 0;

		case WM_DPICHANGED_AFTERPARENT:
		{
			int iDpi = GetDpi(hWnd);
			m_cxPadding = DpiScale(c_iPadding, iDpi);
			m_cxSubTaskPadding = DpiScale(c_iSubTaskPadding, iDpi);
		}
		[[fallthrough]];
		case WM_THEMECHANGED:
		{
			RefreshThemeRes();

			RECT rc;
			for (auto& x : m_Items)
			{
				GetThemeTextExtent(m_hthControlPanel, m_hCDCAuxiliary, CPANEL_SECTIONTITLELINK, CPSTL_NORMAL,
					x.rsText.Data(), x.rsText.Size(), DT_SINGLELINE, nullptr, &rc);
				x.cx = rc.right - rc.left;
				for (auto& y : x.SubTasks)
				{
					GetThemeTextExtent(m_hthControlPanel, m_hCDCAuxiliary, CPANEL_CONTENTLINK, CPTL_NORMAL,
						y.rsText.Data(), y.rsText.Size(), DT_SINGLELINE, nullptr, &rc);
					y.cx = rc.right - rc.left;
				}
			}

			ReCalcColumnIdealWidth();
		}
		break;

		case WM_DESTROY:
		{
			CloseThemeData(m_hthControlPanel);
			CloseThemeData(m_hthListView);
		}
		break;
		}
		return CListView::OnMsg(hWnd, uMsg, wParam, lParam);
	}

	ECK_CWND_CREATE;
	HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, HMENU hMenu, PCVOID pData = nullptr) override
	{
		dwStyle |= (WS_CHILD | LVS_NOCOLUMNHEADER);
		dwStyle &= ~(LVS_OWNERDATA | LVS_OWNERDRAWFIXED);

		if (!IsBitSet(dwStyle, LVS_REPORT))
		{
			dwStyle &= ~(LVS_ICON | LVS_LIST | LVS_SMALLICON);
			dwStyle |= LVS_REPORT;
		}
		m_hWnd = CreateWindowExW(dwExStyle, WC_LISTVIEWW, pszText, dwStyle,
			x, y, cx, cy, hParent, hMenu, nullptr, nullptr);

		SetExplorerTheme();

		m_hCDCAuxiliary = CreateCompatibleDC(nullptr);

		int iDpi = GetDpi(m_hWnd);
		m_cxPadding = DpiScale(c_iPadding, iDpi);
		m_cxSubTaskPadding = DpiScale(c_iSubTaskPadding, iDpi);

		RefreshThemeRes();

		LVCOLUMNW lvc;
		lvc.mask = LVCF_WIDTH;
		lvc.cx = cx;
		InsertColumn(0, &lvc);

		UINT uExStyle = LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT;
		SetLVExtendStyle(uExStyle, uExStyle);
		return m_hWnd;
	}

	int InsertTask(PCWSTR pszText, int idx = -1, int idxImage = -1,
		TGLSUBTASK* pSubTask = nullptr, int cSubTask = 0)
	{
		TASKITEM Item{ pszText,idxImage };
		RECT rc;

		GetThemeTextExtent(m_hthControlPanel, m_hCDCAuxiliary, CPANEL_SECTIONTITLELINK, CPSTL_NORMAL,
			Item.rsText.Data(), Item.rsText.Size(), DT_SINGLELINE, nullptr, &rc);
		Item.cx = rc.right - rc.left;
		EckCounter(cSubTask, i)
		{
			SUBTASK SubTask;
			SubTask.rsText = pSubTask[i].pszText;
			SubTask.uFlags = pSubTask[i].uFlags;
			GetThemeTextExtent(m_hthControlPanel, m_hCDCAuxiliary, CPANEL_CONTENTLINK, CPTL_NORMAL,
				SubTask.rsText.Data(), SubTask.rsText.Size(), DT_SINGLELINE, nullptr, &rc);
			SubTask.cx = rc.right - rc.left;
			Item.SubTasks.emplace_back(std::move(SubTask));
		}

		if (idx < 0)
		{
			m_Items.push_back(std::move(Item));
			idx = (int)m_Items.size();
		}
		else
			m_Items.insert(m_Items.begin() + idx, std::move(Item));

		LVITEMW lvi{};
		InsertItem(idx, &lvi);

		return idx;
	}

	int ReCalcColumnIdealWidth()
	{
		int iMax = 0, iSum;

		for (auto& x : m_Items)
		{
			if (x.cx > iMax)
				iMax = x.cx;

			if (x.SubTasks.size())
			{
				iSum = ((int)x.SubTasks.size() - 1) * m_cxSubTaskPadding * 2;
				for (auto& y : x.SubTasks)
					iSum += y.cx;
				if (iSum > iMax)
					iMax = iSum;
			}
		}

		int cx = m_cxIcon + m_cxPadding * 3 + m_cxSubTaskPadding + iMax;
		m_iIdealWidth = cx;
		if (cx < m_cxClient)
			cx = m_cxClient;
		SetColumnWidth(0, cx);
		return m_iIdealWidth;
	}

	EckInline int GetIdealWidth()
	{
		return m_iIdealWidth;
	}

	HIMAGELIST SetTaskImageList(HIMAGELIST hImageList)
	{
		auto hOld = m_hImageList;
		m_hImageList = hImageList;
		ImageList_GetIconSize(hImageList, &m_cxIcon, &m_cyIcon);
		Redraw();
		return hOld;
	}

	UINT HitTestTask(RECT* prcItem, int idxItem, POINT pt, int* pidxSubTask = nullptr)
	{
		RECT rc;
		rc.left = prcItem->left + m_cxPadding;
		rc.top = prcItem->top + m_cxPadding;
		rc.right = rc.left + m_cxIcon;
		rc.bottom = rc.top + m_cyIcon;
		if (PtInRect(&rc, pt))
			return TGLP_ICON;

		rc.left = rc.right + m_cxSubTaskPadding;
		rc.top = prcItem->top;
		rc.right = rc.left + m_Items[idxItem].cx;
		rc.bottom = rc.top + m_cySectionTitle;
		if (PtInRect(&rc, pt))
			return TGLP_SECTIONTITLE;

		rc.top = rc.bottom + m_cxPadding;
		rc.bottom = rc.top + m_cySubTask;
		EckCounter(m_Items[idxItem].SubTasks.size(), i)
		{
			auto& x = m_Items[idxItem].SubTasks[i];
			rc.right = rc.left + x.cx;
			if (PtInRect(&rc, pt))
			{
				if (pidxSubTask)
					*pidxSubTask = (int)i;
				return TGLP_SUBTASK;
			}
			rc.left = rc.right + m_cxSubTaskPadding * 2;
		}

		return TGLP_NONE;
	}
};

ECK_NAMESPACE_END