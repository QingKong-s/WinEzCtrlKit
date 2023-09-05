#include "CTaskGroupList.h"

#define SIZE_PADDING		4
#define SIZE_SUBTASKPADDING	8

ECK_NAMESPACE_BEGIN
SUBCLASS_MGR_INIT(CTaskGroupList, SCID_TASKGROUPLIST, CTaskGroupList::SubclassProc)
SUBCLASS_REF_MGR_INIT(CTaskGroupList, ObjRecorderRefPlaceholder, SCID_TASKGROUPLISTPARENT, CTaskGroupList::SubclassProc_Parent, ObjRecordRefStdDeleter)

LRESULT CALLBACK CTaskGroupList::SubclassProc_Parent(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
	UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	switch (uMsg)
	{
	case WM_NOTIFY:
	{
		switch (((NMHDR*)lParam)->code)
		{
		case NM_CUSTOMDRAW:
		{
			auto it = m_WndRecord.find(((NMHDR*)lParam)->hwndFrom);
			if (it == m_WndRecord.end())
				break;

			auto pnmlcd = (NMLVCUSTOMDRAW*)lParam;

			switch (pnmlcd->nmcd.dwDrawStage)
			{
			case CDDS_PREPAINT:
				return CDRF_NOTIFYITEMDRAW;
			case CDDS_ITEMPREPAINT:
			{
				auto p = it->second;
				auto& Item = p->m_Items[pnmlcd->nmcd.dwItemSpec];
				HDC hDC = pnmlcd->nmcd.hdc;
				int cxPadding = p->m_cxPadding;
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
					DrawThemeBackground(p->m_hthListView, hDC, LVP_LISTITEM, iState, &pnmlcd->nmcd.rc, NULL);
				//////////////////////画图标
				int x = cxPadding + pnmlcd->nmcd.rc.left, y = cxPadding + pnmlcd->nmcd.rc.top;
				if (Item.idxImage >= 0)
					ImageList_Draw(p->m_hImageList, Item.idxImage, hDC, x, y, ILD_NORMAL);
				x += (p->m_cxIcon + p->m_cxSubTaskPadding);
				//////////////////////画节标题
				if ((p->m_idxHot == pnmlcd->nmcd.dwItemSpec && p->m_bSectionTitleHot) ||
					(p->m_idxPressed == pnmlcd->nmcd.dwItemSpec && p->m_bSectionTitlePressed))
					iState = CPSTL_HOT;
				else
					iState = CPSTL_NORMAL;
				RECT rc{ x,pnmlcd->nmcd.rc.top,pnmlcd->nmcd.rc.right,pnmlcd->nmcd.rc.bottom };
				DrawThemeTextEx(p->m_hthControlPanel, hDC, CPANEL_SECTIONTITLELINK, iState,
					Item.rsText, Item.rsText.Size(), DT_SINGLELINE, &rc, NULL);
				//////////////////////画子任务
				rc.top += (p->m_cySectionTitle + cxPadding);
				
				EckCounter(Item.SubTasks.size(), i)
				{
					auto& sub = Item.SubTasks[i];

					if (p->m_idxPressed == pnmlcd->nmcd.dwItemSpec && p->m_idxPressedSubTask == i)
						iState = CPCL_PRESSED;
					else if (p->m_idxHot == pnmlcd->nmcd.dwItemSpec && p->m_idxHotSubTask == i)
						iState = CPCL_HOT;
					else
						iState = CPCL_NORMAL;

					DrawThemeTextEx(p->m_hthControlPanel, hDC, CPANEL_CONTENTLINK, iState,
						sub.rsText, sub.rsText.Size(), DT_SINGLELINE, &rc, NULL);

					rc.left += (sub.cx + p->m_cxSubTaskPadding * 2);
				}
			}
			return CDRF_SKIPDEFAULT;
			}
		}
		break;
		}
	}
	break;

	case WM_DESTROY:
		m_SMRef.DeleteRecord(hWnd);
		break;
	}

	return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK CTaskGroupList::SubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
	UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	auto p = (CTaskGroupList*)dwRefData;
	switch (uMsg)
	{
	case WM_MOUSEMOVE:
	{
		POINT pt = GET_PT_LPARAM(lParam);
		LRESULT lResult = DefSubclassProc(hWnd, uMsg, wParam, lParam);
		int idxHot = p->GetHotItem();
		BOOL bRedraw = FALSE, bHotChanged = FALSE;
		if (p->m_idxHot != idxHot)
			bHotChanged = TRUE;
		p->m_idxHot = idxHot;
		if (idxHot >= 0)
		{
			RECT rc;
			p->GetItemRect(idxHot, &rc);
			int idxHotSubTask = 1;
			UINT uPart = p->HitTestTask(&rc, idxHot, pt, &idxHotSubTask);
			if (uPart == TGLP_SECTIONTITLE)
			{
				if (!p->m_bSectionTitleHot)
					bRedraw = TRUE;
				p->m_bSectionTitleHot = TRUE;
			}
			else
			{
				if (p->m_bSectionTitleHot)
					bRedraw = TRUE;
				p->m_bSectionTitleHot = FALSE;
				if (uPart == TGLP_SUBTASK)
				{
					if (p->m_idxHotSubTask!= idxHotSubTask)
						bRedraw = TRUE;
					p->m_idxHotSubTask = idxHotSubTask;
				}
				else
				{
					if (p->m_idxHotSubTask != -1)
						bRedraw = TRUE;
					p->m_idxHotSubTask = -1;
				}
			}

			if (bHotChanged || bRedraw)
				p->RedrawItems(idxHot, idxHot);
		}

		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(TRACKMOUSEEVENT);
		tme.dwFlags = TME_LEAVE;
		tme.hwndTrack = hWnd;
		TrackMouseEvent(&tme);
		
		return lResult;
	}

	case WM_SIZE:
		p->m_cxClient = LOWORD(lParam);
		break;

	case WM_MOUSELEAVE:
	{
		p->m_idxHot = -1;
		p->m_idxHotSubTask = -1;
		p->m_bSectionTitleHot = FALSE;
	}
	break;

	case WM_LBUTTONDOWN:
	{
		LVHITTESTINFO lvhti;
		lvhti.pt = GET_PT_LPARAM(lParam);
		p->HitTest(&lvhti);
		p->m_idxPressed = lvhti.iItem;

		if (lvhti.iItem >= 0)
		{
			RECT rc;
			p->GetItemRect(lvhti.iItem, &rc);
			int idxSubTask = -1;
			UINT uPart = p->HitTestTask(&rc, lvhti.iItem, lvhti.pt, &idxSubTask);
			p->m_bSectionTitlePressed = (uPart == TGLP_SECTIONTITLE);
			p->m_idxPressedSubTask = idxSubTask;
			if (uPart == TGLP_SECTIONTITLE || uPart == TGLP_SUBTASK)
			{
				p->m_bLBtnDown = TRUE;
				SetCapture(hWnd);
				p->RedrawItems(lvhti.iItem, lvhti.iItem);
				return 0;
			}
		}
		else
		{
			p->m_bSectionTitlePressed = FALSE;
			p->m_idxPressedSubTask = -1;
		}
	}
	break;

	case WM_LBUTTONUP:
	{
		if (!p->m_bLBtnDown)
			break;
		p->m_bLBtnDown = FALSE;
		ReleaseCapture();

		LVHITTESTINFO lvhti;
		lvhti.pt = GET_PT_LPARAM(lParam);
		p->HitTest(&lvhti);

		if (lvhti.iItem >= 0 && lvhti.iItem == p->m_idxPressed)
		{
			RECT rc;
			p->GetItemRect(lvhti.iItem, &rc);
			int idxSubTask = -1;
			UINT uPart = p->HitTestTask(&rc, lvhti.iItem, lvhti.pt, &idxSubTask);

			if ((p->m_bSectionTitlePressed && uPart == TGLP_SECTIONTITLE) ||
				(p->m_idxPressedSubTask >= 0 && p->m_idxPressedSubTask == idxSubTask))
			{
				TGLCLICKINFO Info;
				Info.idxItem = lvhti.iItem;
				Info.uPart = uPart;
				Info.idxSubTask = idxSubTask;
				SendMessageW(p->m_hParent, p->m_uNotifyMsg, TGLNM_CLICK, (LPARAM)&Info);
			}
		}

		int idxOld = p->m_idxPressed;
		p->m_idxPressed = -1;
		p->m_bSectionTitlePressed = FALSE;
		p->m_idxHotSubTask = -1;
		p->RedrawItems(idxOld, idxOld);
	}
	return 0;

	case WM_DPICHANGED_AFTERPARENT:
	{
		int iDpi = GetDpi(hWnd);
		p->m_cxPadding = DpiScale(SIZE_PADDING, iDpi);
		p->m_cxSubTaskPadding = DpiScale(SIZE_SUBTASKPADDING, iDpi);
	}
	// fall through
	case WM_THEMECHANGED:
	{
		p->RefreshThemeRes();

		RECT rc;
		for (auto& x : p->m_Items)
		{
			GetThemeTextExtent(p->m_hthControlPanel, p->m_hCDCAuxiliary, CPANEL_SECTIONTITLELINK, CPSTL_NORMAL,
				x.rsText, x.rsText.Size(), DT_SINGLELINE, NULL, &rc);
			x.cx = rc.right - rc.left;
			for (auto& y : x.SubTasks)
			{
				GetThemeTextExtent(p->m_hthControlPanel, p->m_hCDCAuxiliary, CPANEL_CONTENTLINK, CPTL_NORMAL,
					y.rsText, y.rsText.Size(), DT_SINGLELINE, NULL, &rc);
				y.cx = rc.right - rc.left;
			}
		}

		p->RecalcColumnIdealWidth();
	}
	break;

	case WM_DESTROY:
	{
		CloseThemeData(p->m_hthControlPanel);
		CloseThemeData(p->m_hthListView);
		m_SM.RemoveSubclass(hWnd);
		m_SMRef.Release(p->m_hParent);
	}
	break;
	}
	return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

HWND CTaskGroupList::Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle, 
	int x, int y, int cx, int cy, HWND hParent, int nID, PCVOID pData)
{
	dwStyle |= (WS_CHILD | LVS_NOCOLUMNHEADER);
	dwStyle &= ~(LVS_OWNERDATA | LVS_OWNERDRAWFIXED);
	
	if (!IsBitSet(dwStyle, LVS_REPORT))
	{
		dwStyle &= ~(LVS_ICON | LVS_LIST | LVS_SMALLICON);
		dwStyle |= LVS_REPORT;
	}
	m_hParent = hParent;
	m_hWnd = CreateWindowExW(dwExStyle, WC_LISTVIEWW, pszText, dwStyle,
		x, y, cx, cy, hParent, i32ToP<HMENU>(nID), NULL, NULL);

	SetExplorerTheme();

	m_hCDCAuxiliary = CreateCompatibleDC(NULL);

	int iDpi = GetDpi(m_hWnd);
	m_cxPadding = DpiScale(SIZE_PADDING, iDpi);
	m_cxSubTaskPadding = DpiScale(SIZE_SUBTASKPADDING, iDpi);

	RefreshThemeRes();

	LVCOLUMNW lvc;
	lvc.mask = LVCF_WIDTH;
	lvc.cx = cx;
	InsertColumn(0, &lvc);

	UINT uExStyle = LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT;
	SetLVExtendStyle(uExStyle, uExStyle);

	m_SM.AddSubclass(m_hWnd, this);
	m_SMRef.AddRef(hParent, ObjRecorderRefPlaceholderVal);

	return m_hWnd;
}

int CTaskGroupList::InsertTask(PCWSTR pszText, int idx, int idxImage, TGLSUBTASK* pSubTask, int cSubTask)
{
	TASKITEM Item{ pszText,idxImage };
	SUBTASK SubTask;
	RECT rc;

	GetThemeTextExtent(m_hthControlPanel, m_hCDCAuxiliary, CPANEL_SECTIONTITLELINK, CPSTL_NORMAL,
		Item.rsText, Item.rsText.Size(), DT_SINGLELINE, NULL, &rc);
	Item.cx = rc.right - rc.left;
	EckCounter(cSubTask, i)
	{
		SubTask.rsText = pSubTask[i].pszText;
		SubTask.uFlags = pSubTask[i].uFlags;
		GetThemeTextExtent(m_hthControlPanel, m_hCDCAuxiliary, CPANEL_CONTENTLINK, CPTL_NORMAL,
			SubTask.rsText, SubTask.rsText.Size(), DT_SINGLELINE, NULL, &rc);
		SubTask.cx = rc.right - rc.left;
		Item.SubTasks.push_back(std::move(SubTask));
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

int CTaskGroupList::RecalcColumnIdealWidth()
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

UINT CTaskGroupList::HitTestTask(RECT* prcItem, int idxItem, POINT pt, int* pidxSubTask)
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

void CTaskGroupList::RefreshThemeRes()
{
	CloseThemeData(m_hthControlPanel);
	CloseThemeData(m_hthListView);
	m_hthControlPanel = OpenThemeData(m_hWnd, L"ControlPanel");
	m_hthListView = OpenThemeData(m_hWnd, L"ListView");

	RECT rc;
	GetThemeTextExtent(m_hthControlPanel, m_hCDCAuxiliary, CPANEL_SECTIONTITLELINK, CPSTL_NORMAL,
		L"bp", -1, DT_SINGLELINE, NULL, &rc);
	m_cySectionTitle = rc.bottom - rc.top;

	GetThemeTextExtent(m_hthControlPanel, m_hCDCAuxiliary, CPANEL_CONTENTLINK, CPCL_NORMAL,
		L"bp", -1, DT_SINGLELINE, NULL, &rc);
	m_cySubTask = rc.bottom - rc.top;

	EckLVSetItemHeight(m_hWnd, std::max(m_cyIcon, m_cySectionTitle + m_cySubTask + m_cxPadding * 2));
}
ECK_NAMESPACE_END