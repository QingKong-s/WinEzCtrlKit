#include "pch.h"	

BOOL CWndMain::OnCreate(HWND hWnd, CREATESTRUCTW* pcs)
{
	m_hFontComm = eck::EzFont(L"微软雅黑", 9);
	m_iDpi = eck::GetDpi(hWnd);
	eck::UpdateDpiSize(m_Ds, m_iDpi);

	m_iDpi = eck::GetDpi(hWnd);
	UpdateDpi();

	m_hMenuWorkWnd = CreatePopupMenu();
	AppendMenuW(m_hMenuWorkWnd, 0, IDMI_WW_COPY, L"复制");
	AppendMenuW(m_hMenuWorkWnd, 0, IDMI_WW_CUT, L"剪切");
	AppendMenuW(m_hMenuWorkWnd, 0, IDMI_WW_PASTE, L"粘贴");
	AppendMenuW(m_hMenuWorkWnd, 0, IDMI_WW_DEL, L"删除");

	m_hPen = CreatePen(PS_SOLID, 2, eck::Colorref::Red);
	m_cxInfo = DPI(240);
	m_cxCtrlBox = DPI(180);

	m_TBProj.Create(nullptr, WS_CHILD| WS_VISIBLE, 0, 0, 0, 0, 0, hWnd, IDC_TC_PROJ);
	{
		m_CBBCtrl.Create(nullptr, WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | CBS_SORT,
			0, 0, 50, 50, 0, hWnd, IDC_CBB_CTRL);
		m_CBBCtrl.SetItemHeight(m_Ds.cyComboBox);

		m_LVProp.Create(nullptr, WS_CHILD | WS_VISIBLE | WS_BORDER /*| LVS_NOCOLUMNHEADER*/ | LVS_REPORT,
			0, 0, 0, 0, 0, hWnd, IDC_LV_CTRLINFO);
		eck::LVSetItemHeight(m_LVProp.HWnd, DPI(24));
		LVCOLUMNW lc;
		lc.mask = LVCF_TEXT | LVCF_WIDTH;
		lc.pszText = (PWSTR)L"属性";
		lc.cx = DPI(110);
		m_LVProp.InsertColumn(0, &lc);
		lc.pszText = (PWSTR)L"值";
		lc.cx = DPI(110);
		m_LVProp.InsertColumn(1, &lc);
		m_LVProp.SetLVExtendStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);
		m_LVProp.SetExplorerTheme();
		m_LVProp.SetNotifyMsg(CNM_PROPLISTNOTIFY);

		m_EDDesc.SetMultiLine(TRUE);
		m_EDDesc.Create(nullptr, WS_CHILD | WS_VISIBLE | ES_AUTOVSCROLL, 0, 0, 0, 0, 0, hWnd, IDC_ED_DESC);
		m_EDDesc.SetFrameType(3);
	}

	m_LBCtrl.Create(nullptr, WS_CHILD | WS_VISIBLE | WS_BORDER | LBS_NOINTEGRALHEIGHT | LBS_OWNERDRAWFIXED | LBS_NOTIFY | LBS_NODATA,
		0, 0, 0, 0, 0, hWnd, IDC_LB_CTRL);
	m_LBCtrl.SetCount(ARRAYSIZE(eck::s_EckDesignAllCtrl) + 1);

	m_Tab.Create(nullptr,WS_CHILD| WS_VISIBLE | TCS_SINGLELINE, 0, 0, 0, 0, 0, hWnd, IDC_TC_MAIN);

	eck::SetFontForWndAndCtrl(hWnd, m_hFontComm);

	NewTab();
	return TRUE;
}

void CWndMain::OnMeasureItem(HWND hWnd, MEASUREITEMSTRUCT* pmis)
{
	if (pmis->CtlID != IDC_LB_CTRL)
		return FORWARD_WM_MEASUREITEM(hWnd, pmis, DefWindowProcW);
	pmis->itemHeight = DPI(32);
}

void CWndMain::OnDrawItem(HWND hWnd, const DRAWITEMSTRUCT* pdis)
{
	if (pdis->hwndItem == m_LBCtrl.HWnd)
	{
		HDC hDC = pdis->hDC;
		if (eck::IsBitSet(pdis->itemState, ODS_SELECTED))
		{
			FillRect(hDC, &pdis->rcItem, GetSysColorBrush(COLOR_HIGHLIGHT));
			SetTextColor(hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
		}
		else
		{
			FillRect(hDC, &pdis->rcItem, GetSysColorBrush(COLOR_WINDOW));
			SetTextColor(hDC, GetSysColor(COLOR_WINDOWTEXT));
		}

		SetBkMode(hDC, TRANSPARENT);
		HGDIOBJ hOld = SelectObject(hDC, m_hFontCtrlBox);
		PCWSTR pszText = (pdis->itemID ? eck::s_EckDesignAllCtrl[pdis->itemID - 1].pszName : L"指针");

		DrawTextW(hDC, pszText, -1, (RECT*)&pdis->rcItem, DT_SINGLELINE | DT_LEFT | DT_VCENTER);
		SelectObject(hDC, hOld);
	}
}

void CWndMain::OnCommand(HWND hWnd, int idCtrl, HWND hCtrl, UINT uNotifyCode)
{
	if (hCtrl)
	{
		switch (uNotifyCode)
		{
		case LBN_SELCHANGE:// 与CBN_SELCHANGE相等
			switch (idCtrl)
			{
			case IDC_LB_CTRL:
				m_bPlacingCtrl = (m_LBCtrl.GetCurrSel() != 0);
				break;
			case IDC_CBB_CTRL:
				SingleSel(m_vTabs[m_Tab.GetCurSel()], (HWND)m_CBBCtrl.GetItemData(m_CBBCtrl.GetCurSel()));
				break;
			}
		}
	}
	else
	{
		switch (idCtrl)
		{
		case IDMI_WW_COPY:
			OnMenuCopy(hWnd);
			return;
		case IDMI_WW_PASTE:
			OnMenuPaste(hWnd);
			return;
		case IDMI_WW_DEL:
			OnMenuDel(hWnd);
			return;
		case IDMI_SAVEWNDTABLE:
			OnMenuSaveWndTable(hWnd);
			return;
		}
	}
}

void CWndMain::OnMenuCopy(HWND hWnd)
{
	int idx = m_Tab.GetCurSel();
	auto pTabCtx = m_vTabs[idx];
	OpenClipboard(hWnd);
	EmptyClipboard();
	HWND hCtrl;
	SIZE_T cSel = pTabCtx->pMultiSelMarker.size();

	CCtrlsSerializer Serializer(pTabCtx, m_iDpi);

	Serializer.Reserve((int)cSel);
	for (auto x : pTabCtx->pMultiSelMarker)
	{
		hCtrl = x->GetTargetWindow();
		auto& Info = pTabCtx->AllCtrls[hCtrl];

		if (IsCtrlParentSel(pTabCtx, hCtrl))// 如果一个控件的某上级窗口被选择，那么不应该处理
			continue;

		Serializer.AddCtrl(hCtrl);
	}

	HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, Serializer.GetDataSize());
	void* pData = GlobalLock(hGlobal);
	Serializer.Serialize(pData);
	GlobalUnlock(hGlobal);
	SetClipboardData(App->m_cfmtCtrlInfo, hGlobal);
	CloseClipboard();
}

void CWndMain::OnMenuPaste(HWND hWnd)
{
	int idx = m_Tab.GetCurSel();
	auto pTabCtx = m_vTabs[idx];
	OpenClipboard(hWnd);
	if (!IsClipboardFormatAvailable(App->m_cfmtCtrlInfo))
	{
		CloseClipboard();
		return;
	}

	HGLOBAL hGlobal = GetClipboardData(App->m_cfmtCtrlInfo);
	void* pData = GlobalLock(hGlobal);

	eck::CCtrlsDeserializer Deserializer;
	auto pHeader = Deserializer.SetOrgData(pData, GlobalSize(pData));

	Deserializer.For(
		[pTabCtx = pTabCtx, &This = *this]
		(const eck::FTCTRLDATA* p, eck::PCBYTE pCtrlData, HWND hParent)->HWND
		{
			RECT rc = p->rc;
			rc.right += rc.left;
			rc.bottom += rc.top;
			MapWindowRect(pTabCtx->pWorkWnd->GetHWND(), hParent, &rc);
			rc.right -= rc.left;
			rc.bottom -= rc.top;

			if (!hParent)
				hParent = pTabCtx->pWorkWnd->GetHWND();

			auto pWnd = This.CreateCtrl(p->idxInfo + 1, pCtrlData, pTabCtx, nullptr, 0, 0,
				rc.left, rc.top, rc.right, rc.bottom, hParent, 0);
			return pWnd->GetHWND();
		});

	GlobalUnlock(hGlobal);
	CloseClipboard();
}

void CWndMain::OnMenuDel(HWND hWnd)
{
	int idx = m_Tab.GetCurSel();
	auto pTabCtx = m_vTabs[idx];
	const std::function<void(HWND hWnd)> DelCtrl = [&](HWND hWnd)
		{
			HWND hTemp = GetWindow(hWnd, GW_CHILD);
			hTemp = GetWindow(hTemp, GW_HWNDLAST);
			while (hTemp)
			{
				DelCtrl(hTemp);
				DestroyCtrl(pTabCtx, hTemp);
				hTemp = GetWindow(hTemp, GW_HWNDPREV);
			}
		};

	std::vector<HWND> aValidWnds{};
	HWND hCtrl;
	for (auto x : pTabCtx->pMultiSelMarker)
	{
		hCtrl = x->GetTargetWindow();
		auto& Info = pTabCtx->AllCtrls[hCtrl];
		if (IsCtrlParentSel(pTabCtx, hCtrl))// 如果一个控件的某上级窗口被选择，那么不应该处理
			continue;

		aValidWnds.push_back(hCtrl);
	}

	for (auto x : aValidWnds)
	{
		DelCtrl(x);
		DestroyCtrl(pTabCtx, x);
	}

	SingleSel(pTabCtx, pTabCtx->pWorkWnd->GetHWND());
}

void CWndMain::OnMenuSaveWndTable(HWND hWnd)
{
	int idx = m_Tab.GetCurSel();
	auto pTabCtx = m_vTabs[idx];
	CCtrlsSerializer Serializer(pTabCtx, m_iDpi);
	EnumChildWindows(pTabCtx->pWorkWnd->GetHWND(), [](HWND hChild, LPARAM lParam)->BOOL
		{
			auto p = (CCtrlsSerializer*)lParam;
			p->AddCtrl(hChild);
			return TRUE;
		}, (LPARAM)&Serializer);

	eck::CRefBin rb(Serializer.GetDataSize() + sizeof(eck::FORMTABLEHEADER) + sizeof(eck::FORMTABLE_INDEXENTRY));
	eck::CMemWriter w(rb.Data(), rb.Size());

	eck::FORMTABLEHEADER* pHeader;
	w.SkipPointer(pHeader);
	pHeader->iVer = 1;
	pHeader->cForms = 1;

	eck::FORMTABLE_INDEXENTRY* pEntry;
	w.SkipPointer(pEntry);
	pEntry->cbSize = (UINT)rb.Size();
	pEntry->iID = 101;
	pEntry->uOffset = sizeof(eck::FORMTABLEHEADER) + sizeof(eck::FORMTABLE_INDEXENTRY);

	Serializer.Serialize(w);

	eck::WriteToFile(LR"(E:\Desktop\1.eckft)", rb);
}

void CWndMain::OnSize(HWND hWnd, UINT nType, int cxClient, int cyClient)
{
	HDWP hDwp = BeginDeferWindowPos(10);

	//hDwp = DeferWindowPos(hDwp, m_TBProj, NULL,
	//	m_Ds.iPadding,
	//	m_Ds.iPadding,
	//	m_cxInfo,
	//	cyClient - m_Ds.iPadding * 2,
	//	SWP_NOZORDER);
	{
		//HDWP hDwp2 = BeginDeferWindowPos(10);
		auto hDwp2 = hDwp;
		hDwp2 = DeferWindowPos(hDwp2, m_CBBCtrl.HWnd, nullptr,
			m_Ds.iPadding,
			m_Ds.iPadding,
			m_cxInfo,
			DPI(24),
			SWP_NOZORDER | SWP_NOACTIVATE);

		int y = m_Ds.iPadding + m_Ds.cyComboBox;

		int cyLVProp = cyClient - y - DPI(80) - m_Ds.iPadding * 3;
		hDwp2 = DeferWindowPos(hDwp2, m_LVProp.HWnd, nullptr,
			m_Ds.iPadding,
			y + m_Ds.iPadding,
			m_cxInfo,
			cyLVProp,
			SWP_NOZORDER);

		hDwp2 = DeferWindowPos(hDwp2, m_EDDesc.HWnd, nullptr,
			m_Ds.iPadding,
			y + cyLVProp + m_Ds.iPadding * 2,
			m_cxInfo,
			DPI(80),
			SWP_NOZORDER);

		//EndDeferWindowPos(hDwp2);
		hDwp = hDwp2;
	}

	int cyBK = cyClient - m_Ds.iPadding * 2;
	hDwp = DeferWindowPos(hDwp, m_Tab.HWnd, nullptr,
		m_cxInfo + DPI(6) + m_Ds.iPadding,
		m_Ds.iPadding,
		cxClient - m_cxInfo - m_cxCtrlBox - m_Ds.iPadding * 2 - DPI(6) * 2,
		cyBK,
		SWP_NOZORDER);

	hDwp = DeferWindowPos(hDwp, m_LBCtrl.HWnd, nullptr,
		cxClient - m_cxCtrlBox - m_Ds.iPadding,
		m_Ds.iPadding,
		m_cxCtrlBox,
		cyBK,
		SWP_NOZORDER);

	EndDeferWindowPos(hDwp);

	RECT rc;
	GetWindowRect(m_Tab.HWnd, &rc);
	m_Tab.AdjustRect(&rc, FALSE);
	eck::ScreenToClient(m_Tab.HWnd, &rc);
	for (auto x : m_vTabs)
		SetWindowPos(x->pBK->GetHWND(), nullptr, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOZORDER);
}

void CWndMain::OnDpiChanged(HWND hWnd, int iDpiX, int iDpiY, RECT* prc)
{
	int iOldDpi = m_iDpi;
	m_iDpi = iDpiX;
	UpdateDpi();
	ImageList_Destroy(eck::LVSetItemHeight(m_LVProp.GetHWND(), DPI(24)));

	//eck::MsgOnDpiChanged(hWnd, m_iDpi, iOldDpi);
}

LRESULT CWndMain::OnPropListNotify(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case PLN_PROPCHANGED:
	{
		switch (lParam)
		{
		case EckCPIDToIndex(eck::CPID_LEFT):
		case EckCPIDToIndex(eck::CPID_TOP):
		case EckCPIDToIndex(eck::CPID_WIDTH):
		case EckCPIDToIndex(eck::CPID_HEIGHT):
		{
			auto pTabCtx = m_vTabs[m_Tab.GetCurSel()];
			for (auto x : pTabCtx->pMultiSelMarker)
				x->MoveToTargetWindow();
		}
		return 0;

		case EckCPIDToIndex(eck::CPID_NAME):
		{
			auto pTabCtx = m_vTabs[m_Tab.GetCurSel()];
			auto& Sel = pTabCtx->pMultiSelMarker;
			if (Sel.size() > 1 || Sel.size() <= 0)
				return 0;
			HWND hCtrl = Sel[0]->GetTargetWindow();
			PCWSTR pszNew = m_LVProp.GetValue(EckCPIDToIndex(eck::CPID_NAME)).Vpsz;

			EckCounter(m_CBBCtrl.GetCount(), i)
			{
				if ((HWND)m_CBBCtrl.GetItemData(i) == hCtrl)
				{
					m_CBBCtrl.SetItemString(i, pszNew);
					break;
				}
			}
		}
		return 0;
		}
	}
	return 0;
	}
}

#pragma region 控件消息处理
LRESULT CWndMain::SubclassProc_Ctrl(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	auto p = (CTRLSUBCLASSCTX*)dwRefData;
	switch (uMsg)
	{
	case WM_LBUTTONDBLCLK:
	case WM_RBUTTONDBLCLK:
	case WM_KEYDOWN:
	case WM_KEYUP:
	case WM_NCLBUTTONDOWN:
		return 0;

	case WM_SETCURSOR:
		return HANDLE_WM_SETCURSOR(hWnd, wParam, lParam, p->pThis->OnCtrlSetCursor);

	case WM_RBUTTONDOWN:
		return HANDLE_WM_RBUTTONDOWN(hWnd, wParam, lParam, p->pThis->OnCtrlRButtonDown);

	case WM_RBUTTONUP:
		return HANDLE_WM_RBUTTONUP(hWnd, wParam, lParam, p->pThis->OnCtrlRButtonUp);

	case WM_MOVE:
	{
		LRESULT lResult = DefSubclassProc(hWnd, uMsg, wParam, lParam);
		HANDLE_WM_MOVE(hWnd, wParam, lParam, p->pThis->OnCtrlMove);
		return lResult;
	}

	case WM_SIZE:
	{
		LRESULT lResult = DefSubclassProc(hWnd, uMsg, wParam, lParam);
		HANDLE_WM_SIZE(hWnd, wParam, lParam, p->pThis->OnCtrlSize);
		return lResult;
	}

	case WM_LBUTTONDOWN:
		p->pThis->OnCtrlLButtonDown(hWnd, wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), p);
		return 0;

	case WM_LBUTTONUP:
		p->pThis->OnCtrlLButtonUp(hWnd, wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), p);
		return 0;

	case WM_MOUSEMOVE:
		p->pThis->OnCtrlMouseMove(hWnd, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), p);
		return 0;
	}

	return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

BOOL CWndMain::OnCtrlSetCursor(HWND hWnd, HWND hWndCursor, UINT nHitTest, UINT uMsg)
{
	if (m_bPlacingCtrl)
		SetCursor(LoadCursorW(nullptr, IDC_CROSS));
	else
		SetCursor(LoadCursorW(nullptr, IDC_ARROW));
	return TRUE;
}

void CWndMain::OnCtrlRButtonDown(HWND hWnd, BOOL bDoubleClick, int x, int y, UINT uKeyFlags)
{
	m_bCtrlRBtnDown = TRUE;
	SetCapture(hWnd);
}

void CWndMain::OnCtrlRButtonUp(HWND hWnd, int x, int y, UINT uFlags)
{
	if (!m_bCtrlRBtnDown)
		return;
	m_bCtrlRBtnDown = FALSE;
	ReleaseCapture();
	POINT pt{ x,y };
	ClientToScreen(hWnd, &pt);
	TrackPopupMenu(m_hMenuWorkWnd, TPM_RIGHTBUTTON, pt.x, pt.y, 0, m_hWnd, nullptr);
}

void CWndMain::OnCtrlMove(HWND hWnd, int x, int y)
{
	m_LVProp.GetPropFromCtrl(EckCPIDToIndex(eck::CPID_LEFT));
	m_LVProp.GetPropFromCtrl(EckCPIDToIndex(eck::CPID_TOP));
	m_LVProp.RedrawItems(EckCPIDToIndex(eck::CPID_LEFT), EckCPIDToIndex(eck::CPID_TOP));
}

void CWndMain::OnCtrlSize(HWND hWnd, UINT nType, int cx, int cy)
{
	m_LVProp.GetPropFromCtrl(EckCPIDToIndex(eck::CPID_WIDTH));
	m_LVProp.GetPropFromCtrl(EckCPIDToIndex(eck::CPID_HEIGHT));
	m_LVProp.RedrawItems(EckCPIDToIndex(eck::CPID_WIDTH), EckCPIDToIndex(eck::CPID_HEIGHT));
}

void CWndMain::OnCtrlLButtonDown(HWND hWnd, UINT uKeyFlags, int x, int y, CTRLSUBCLASSCTX* p)
{
	auto pTabCtx = p->pTabCtx;
	/////////////////////////////放置控件
	if (m_bPlacingCtrl)
	{
		m_CtrlPlacing.OnLButtonDown(pTabCtx, hWnd, MAKELPARAM(x, y));
		return;
	}
	/////////////////////////////Ctrl键多选
	if (eck::IsBitSet(uKeyFlags, MK_CONTROL))
	{
		HWND hWorkWnd = pTabCtx->pWorkWnd->GetHWND();
		if (IsCtrlSel(pTabCtx, hWnd))
		{
			RemoveMultiSel(pTabCtx, hWnd);
			if (!pTabCtx->pMultiSelMarker.size())
				SingleSel(pTabCtx, hWorkWnd);
		}
		else
		{
			AddMultiSel(pTabCtx, hWnd);
			if (IsCtrlSel(pTabCtx, hWorkWnd))
				RemoveMultiSel(pTabCtx, hWorkWnd);
		}
		return;
	}
	/////////////////////////////
	m_LVProp.ExitEditing(TRUE);
	if (!IsCtrlSel(p->pTabCtx, hWnd))
		SingleSel(p->pTabCtx, hWnd);
	HWND hBK = pTabCtx->pBK->GetHWND();
	SetCapture(hWnd);
	m_bCtrlLBtnDown = TRUE;

	m_hDC = GetDC(hBK);
	SetROP2(m_hDC, R2_NOTXORPEN);
	m_hOld = SelectObject(m_hDC, m_hPen);

	auto& aSelSizer = pTabCtx->pMultiSelMarker;
	SIZE_T cCtrls = aSelSizer.size();
	m_rcOrg.resize(cCtrls);
	m_rcLast.resize(cCtrls, {});

	m_ptOffset = { x,y };
	RECT* prcOrg = m_rcOrg.data();
	HWND hCurrCtrl;
	EckCounter(cCtrls, i)
	{
		hCurrCtrl = aSelSizer[i]->GetTargetWindow();
		GetWindowRect(hCurrCtrl, prcOrg + i);
		eck::ScreenToClient(hBK, prcOrg + i);
	}
}

void CWndMain::OnCtrlLButtonUp(HWND hWnd, UINT uKeyFlags, int x, int y, CTRLSUBCLASSCTX* p)
{
	auto pTabCtx = p->pTabCtx;
	POINT pt{ x,y };
	HWND hBK = pTabCtx->pBK->GetHWND();
	const int iUnit = pTabCtx->pWorkWnd->GetGridPointGap();

	if (m_bPlacingCtrl && m_CtrlPlacing.IsLBtnDown())
	{
		int idxCurrClass = m_LBCtrl.GetCurrSel();
		m_CtrlPlacing.OnLButtonUp(pTabCtx, MAKELPARAM(x, y));
		MapWindowPoints(hWnd, hBK, &pt, 1);
		RECT rc = eck::MakeRect(PtAlign(pt, iUnit), PtAlign(m_CtrlPlacing.GetStartPoint(), iUnit));
		MapWindowRect(hBK, hWnd, &rc);

		auto pWnd = CreateCtrl(idxCurrClass, nullptr, pTabCtx, eck::s_EckDesignAllCtrl[idxCurrClass - 1].pszName, WS_CHILD | WS_VISIBLE, 0,
			rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, hWnd, 0);
		SingleSel(pTabCtx, pWnd->GetHWND());
		return;
	}
	if (!m_bCtrlLBtnDown)
		return;
	ReleaseCapture();
	m_bCtrlLBtnDown = FALSE;

	SelectObject(m_hDC, m_hOld);
	ReleaseDC(hBK, m_hDC);

	pTabCtx->pBK->SetRedraw(FALSE);
	pTabCtx->pWorkWnd->SetRedraw(FALSE);// byd这里SetWindowPos会导致窗口疯狂重画

	RECT rc;
	auto& aSelSizer = pTabCtx->pMultiSelMarker;
	SIZE_T cCtrls = aSelSizer.size();
	HWND hCurrCtrl;
	int dx = pt.x - m_ptOffset.x, dy = pt.y - m_ptOffset.y;
	POINT ptTemp;
	EckCounter(cCtrls, i)
	{
		hCurrCtrl = aSelSizer[i]->GetTargetWindow();

		if (IsCtrlParentSel(pTabCtx, hCurrCtrl))
			continue;

		rc = m_rcOrg[i];
		OffsetRect(&rc, dx, dy);
		MapWindowRect(hBK, GetParent(hCurrCtrl), &rc);

		SetWindowPos(hCurrCtrl, nullptr, rc.left, rc.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
	}

	for (auto x : aSelSizer)
		x->MoveToTargetWindow();

	pTabCtx->pBK->SetRedraw(TRUE);
	pTabCtx->pWorkWnd->SetRedraw(TRUE);
	pTabCtx->pBK->Redraw();
	m_rcLast.clear();
}

void CWndMain::OnCtrlMouseMove(HWND hWnd, int x, int y, CTRLSUBCLASSCTX* p)
{
	auto pTabCtx = p->pTabCtx;
	if (m_CtrlPlacing.IsLBtnDown())
	{
		m_CtrlPlacing.OnMouseMove(pTabCtx, hWnd, MAKELPARAM(x, y));
		return;
	}
	if (!m_bCtrlLBtnDown)
		return;
	POINT pt{ x,y };

	RECT rc;
	auto& aSelSizer = pTabCtx->pMultiSelMarker;
	SIZE_T cCtrls = aSelSizer.size();
	HWND hCurrCtrl;
	int dx = pt.x - m_ptOffset.x, dy = pt.y - m_ptOffset.y;
	EckCounter(cCtrls, i)
	{
		hCurrCtrl = aSelSizer[i]->GetTargetWindow();

		rc = m_rcOrg[i];
		OffsetRect(&rc, dx, dy);
		if (!IsRectEmpty(&m_rcLast[i]))
			Rectangle(m_hDC, m_rcLast[i].left, m_rcLast[i].top, m_rcLast[i].right, m_rcLast[i].bottom);
		m_rcLast[i] = rc;
		Rectangle(m_hDC, rc.left, rc.top, rc.right, rc.bottom);
	}
}
#pragma endregion

void CWndMain::OnWWLButtonUp(HWND hWnd, int x, int y, UINT uKeyFlags, TABCTX* p)
{
	if (!m_CtrlPlacing.IsLBtnDown())
		return;

	m_CtrlPlacing.OnLButtonUp(p, MAKELPARAM(x, y));

	if (eck::IsBitSet(uKeyFlags, MK_CONTROL))
		return;

	HWND hBK = p->pBK->GetHWND();
	POINT pt{ x,y };
	MapWindowPoints(hWnd, hBK, &pt, 1);

	m_LVProp.ExitEditing(TRUE);

	int idxCurr = m_LBCtrl.GetCurrSel();
	POINT ptStart = m_CtrlPlacing.GetStartPoint();
	if (idxCurr <= 0)// 鼠标指针模式
	{
		if (pt.x - ptStart.x == 0 || pt.y - ptStart.y == 0)
			SingleSel(p, hWnd);// 矩形无效，选定工作窗口
		else// 矩形有效，选中矩形内的所有控件
		{
			ClearMultiSelMarker(p);
			RECT rc = eck::MakeRect(pt, ptStart);
			eck::ClientToScreen(hBK, &rc);
			RECT rcCtrl;
			HWND hCtrl;
			for (auto& x : p->AllCtrls)
			{
				hCtrl = x.second.pWnd->GetHWND();
				GetWindowRect(hCtrl, &rcCtrl);
				if (eck::IsRectsIntersect(rc, rcCtrl))
					AddMultiSel(p, hCtrl);
			}
		}
	}
	else// 组件设计模式
	{
		const int iUnit = p->pWorkWnd->GetGridPointGap();
		RECT rc = eck::MakeRect(PtAlign(pt, iUnit), PtAlign(ptStart, iUnit));
		if (IsRectEmpty(&rc))
		{
			rc.right = rc.left + eck::DpiScale(eck::s_EckDesignAllCtrl[idxCurr - 1].sizeDef.cx, m_iDpi);
			rc.bottom = rc.top + eck::DpiScale(eck::s_EckDesignAllCtrl[idxCurr - 1].sizeDef.cy, m_iDpi);
		}
		MapWindowRect(hBK, hWnd, &rc);

		auto pWnd = CreateCtrl(idxCurr, nullptr, p, eck::s_EckDesignAllCtrl[idxCurr - 1].pszName, WS_CHILD | WS_VISIBLE, 0,
			rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, hWnd, 0);
		SingleSel(p, pWnd->GetHWND());
	}
}

void CWndMain::OnWWRButtonUp(HWND hWnd, UINT uKeyFlags, int x, int y, TABCTX* p)
{
	auto pWW = p->pWorkWnd;
	if (!pWW->m_bRBtnDown)
		return;
	ReleaseCapture();
	pWW->m_bRBtnDown = FALSE;
	POINT pt{ x,y };
	ClientToScreen(hWnd, &pt);
	TrackPopupMenu(m_hMenuWorkWnd, TPM_RIGHTBUTTON, pt.x, pt.y, 0, m_hWnd, nullptr);
}








void CWndMain::NewTab(int posTab)
{
	m_Tab.InsertItem(L"新窗体", posTab);

	auto pCtx = new TABCTX;
	pCtx->pBK = new CWorkWndBk{};
	pCtx->pWorkWnd = new CWorkWnd(pCtx);
	pCtx->pThis = this;

	pCtx->pBK->Create(nullptr, WS_CHILD | WS_VISIBLE | WS_BORDER | WS_CLIPSIBLINGS, 0,
		0, 0, 0, 0, m_Tab.HWnd, IDC_BK_MAIN);

	pCtx->pWorkWnd->Create(nullptr, WS_CHILD | WS_VISIBLE | WS_CAPTION | WS_SYSMENU | WS_CLIPSIBLINGS,
		0, m_Ds.iPadding, m_Ds.iPadding, DPI(400), DPI(300), pCtx->pBK->GetHWND(), IDC_BK_WORKWND);
	SetWindowLongPtrW(pCtx->pWorkWnd->GetHWND(), 0, (LONG_PTR)pCtx);

	SetWindowPos(pCtx->pWorkWnd->GetHWND(), HWND_BOTTOM, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);

	m_vTabs.push_back(pCtx);

	FillCtrlComboBox(m_vTabs[m_Tab.GetCurSel()]);

	SingleSel(pCtx, pCtx->pWorkWnd->GetHWND());
}

ATOM CWndMain::RegisterWndClass()
{
	WNDCLASSEXW wcex{ sizeof(WNDCLASSEXW) };
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = DefWindowProcW;
	wcex.hInstance = App->GetHInstance();
	wcex.hIcon = LoadIconW(nullptr, IDI_APPLICATION);
	wcex.hCursor = LoadCursorW(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
	wcex.lpszClassName = WCN_WDMAIN;
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDM_MAIN);
	wcex.cbWndExtra = sizeof(void*);
	return RegisterClassExW(&wcex);
}

LRESULT CWndMain::OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CREATE:
		return HANDLE_WM_CREATE(hWnd, wParam, lParam, OnCreate);

	case WM_MEASUREITEM:
		return HANDLE_WM_MEASUREITEM(hWnd, wParam, lParam, OnMeasureItem);

	case WM_DRAWITEM:
		return HANDLE_WM_DRAWITEM(hWnd, wParam, lParam, OnDrawItem);

	case WM_COMMAND:
		return HANDLE_WM_COMMAND(hWnd, wParam, lParam, OnCommand);

	case WM_NOTIFY:
	{
		const auto* const pnmhdr = (NMHDR*)lParam;
		switch (pnmhdr->code)
		{
		case TCN_SELCHANGE:
		{
			if (pnmhdr->hwndFrom == m_Tab.HWnd)
			{
				for (auto x : m_vTabs)
					ShowWindow(x->pBK->GetHWND(), SW_HIDE);
				ShowWindow(m_vTabs[m_Tab.GetCurSel()]->pBK->GetHWND(), SW_SHOWNOACTIVATE);
		return 0;
			}
		}
		break;
		}
	}
	break;

	case WM_SIZE:
		return HANDLE_WM_SIZE(hWnd, wParam, lParam, OnSize);

	case WM_DPICHANGED:
		return ECK_HANDLE_WM_DPICHANGED(hWnd, wParam, lParam, OnDpiChanged);

	case CNM_PROPLISTNOTIFY:
		return OnPropListNotify(hWnd, wParam, lParam);
	}

	return __super::OnMsg(hWnd, uMsg, wParam, lParam);
}

void CCtrlPlacingEventHandler::OnLButtonDown(TABCTX* pTabCtx, HWND hWnd, LPARAM lParam)
{
	HWND hBK = pTabCtx->pBK->GetHWND();

	m_bLBtnDown = TRUE;
	SetCapture(hWnd);

	m_ptStart = ECK_GET_PT_LPARAM(lParam);
	MapWindowPoints(hWnd, hBK, &m_ptStart, 1);

	m_hDC = GetDC(hBK);
	SetROP2(m_hDC, R2_NOTXORPEN);
	m_hOld = SelectObject(m_hDC, m_hPen);
}