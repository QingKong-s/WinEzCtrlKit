#include "WndMain.h"
#include "DesginerCtrl.h"

#include <unordered_set>
#include <map>
#include <stack>
#include <functional>

#include "..\ECK.h"
#include "..\WndHelper.h"
#include "..\CListBox.h"
#include "..\CComboBox.h"
#include "..\CListView.h"
#include "..\CEdit.h"
#include "..\CBk.h"
#include "..\CTab.h"
#include "..\Utility.h"
#include "..\DesignerDef.h"

#define IDC_BK_MAIN			101
#define IDC_CBB_CTRL		102
#define IDC_LB_CTRL			103
#define IDC_LV_CTRLINFO		104
#define IDC_ED_DESC			105
#define IDC_BK_WORKWND		106
#define IDC_TC_MAIN			107

int m_cxInfo, m_cxCtrlBox;
int m_iDpi = USER_DEFAULT_SCREEN_DPI;
HWND m_hMain = NULL;
HFONT m_hFont = NULL;
HFONT m_hFontCtrl = NULL;
BOOL m_bPlacingCtrl = FALSE;

HMENU m_hMenuWorkWnd = NULL;
UINT m_uCFCtrlInfo = RegisterClipboardFormatW(L"Eck.Designer.ClipBoardFormat.Ctrl");

struct TABCTX;

struct CTRLSUBCLASSCTX
{
	int idxInfo;
	TABCTX* pTabCtx;
};

struct CTRLINFO
{
	eck::CWnd* pWnd = NULL;
	int idxInfo = -1;
	std::wstring sText{};
	std::wstring sName{};
	eck::CRefBin rbData{};
	CTRLSUBCLASSCTX* pSubclassCtx;

	CTRLINFO() = default;

	CTRLINFO(CTRLINFO&& x)
	{
		pWnd = x.pWnd;
		idxInfo = x.idxInfo;
		sText = std::move(x.sText);
		rbData = std::move(x.rbData);
		pSubclassCtx = x.pSubclassCtx;
		x.pWnd = NULL;
		x.pSubclassCtx = NULL;
	}

	~CTRLINFO()
	{
		delete pWnd;
		delete pSubclassCtx;
	}
};

struct TABCTX
{
	eck::CBk* pBK = NULL;
	eck::CBk* pWorkWnd = NULL;
	std::vector<CSizer*> pMultiSelMarker{};
	std::unordered_set<HWND> SelCtrl{};
	std::unordered_map<HWND, CTRLINFO> AllCtrls{};

	~TABCTX()
	{
		delete pBK;
		delete pWorkWnd;
		for (auto x : pMultiSelMarker)
			delete x;
	}
};




__forceinline int DPI(int i)
{
	return i * m_iDpi / USER_DEFAULT_SCREEN_DPI;
}


eck::CComboBox m_CBBCtrl;
eck::CListBox m_LBCtrl;
CPropList m_LVProp;
eck::CEdit m_EDDesc;
eck::CTab m_Tab;

std::vector<TABCTX*> m_Tabs{};

struct
{
	int iPadding;
	int cyComboBox;
}
m_DS;

ATOM RegisterWDMain(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex{ sizeof(WNDCLASSEXW) };
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc_WDMain;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIconW(NULL, IDI_APPLICATION);
	wcex.hCursor = LoadCursorW(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
	wcex.lpszClassName = WCN_WDMAIN;
	return RegisterClassExW(&wcex);
}

void UpdateDpiSize(int iDpi)
{
	m_iDpi = iDpi;
	m_DS =
	{
		DPI(12),
		DPI(12),
	};
	DeleteObject(m_hFont);
	m_hFont = eck::EzFont(L"微软雅黑", 12);
}

void ShowIt()
{
	RegisterWDMain(GetModuleHandleW(NULL));
	m_hMain = CreateWindowExW(0, WCN_WDMAIN, L"ECK窗体设计器", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, 0, NULL, NULL);
	ShowWindow(m_hMain, SW_SHOW);
	UpdateWindow(m_hMain);
}

void ClearMultiSelMarker(TABCTX* pTabCtx)
{
	for (auto x : pTabCtx->pMultiSelMarker)
		delete x;
	pTabCtx->SelCtrl.clear();
	pTabCtx->pMultiSelMarker.clear();
}

void RemoveMultiSel(TABCTX* pTabCtx, HWND hCtrl)
{
	auto& aSel = pTabCtx->pMultiSelMarker;
	EckCounter(aSel.size(), i)
	{
		if (aSel[i]->GetTargetWindow() == hCtrl)
		{
			delete aSel[i];
			aSel.erase(aSel.begin() + i);
			break;
		}
	}

	pTabCtx->SelCtrl.erase(hCtrl);
}

void AddMultiSel(TABCTX* pTabCtx, HWND hCtrl)
{
	auto pSizer = new CSizer;
	pSizer->SetBlockSize(DPI(8));
	pSizer->Create(pTabCtx->pBK->GetHWND(), pTabCtx->pWorkWnd->GetHWND());
	pSizer->SetTargetWindow(hCtrl);
	pTabCtx->pMultiSelMarker.push_back(pSizer);
	pTabCtx->SelCtrl.insert(hCtrl);
}

void FillPropList(TABCTX* pTabCtx)
{
	int idxCBB = m_CBBCtrl.GetCurrSel();

	HWND hWnd = (HWND)m_CBBCtrl.GetItemData(idxCBB);
	auto it = pTabCtx->AllCtrls.find(hWnd);
	if (it == pTabCtx->AllCtrls.end())
	{
		m_LVProp.SetPropClass();
		return;
	}

	m_LVProp.SetPropClass(it->second.idxInfo, it->second.pWnd);
}

void SingleSel(TABCTX* pTabCtx, HWND hCtrl)
{
	ClearMultiSelMarker(pTabCtx);
	AddMultiSel(pTabCtx, hCtrl);
	EckCounter(m_CBBCtrl.GetCount(), i)
	{
		if ((HWND)m_CBBCtrl.GetItemData(i) == hCtrl)
		{
			m_CBBCtrl.SetCurSel(i);
			break;
		}
	}
	FillPropList(pTabCtx);
}

void FillCtrlComboBox(TABCTX* pTabCtx)
{
	m_CBBCtrl.ResetContent();
	int idx;
	for (auto& x : pTabCtx->AllCtrls)
	{
		idx = m_CBBCtrl.AddString(x.second.sName.c_str());
		m_CBBCtrl.SetItemData(idx, (LPARAM)x.second.pWnd->GetHWND());
	}
	idx = m_CBBCtrl.AddString(L"Window");
	m_CBBCtrl.SetItemData(idx, (LPARAM)pTabCtx->pWorkWnd->GetHWND());
}

EckInline BOOL IsCtrlSel(TABCTX* pTabCtx, HWND hCtrl)
{
	return pTabCtx->SelCtrl.find(hCtrl) != pTabCtx->SelCtrl.end();
}

EckInline BOOL IsCtrlParentSel(TABCTX* pTabCtx, HWND hCtrl)
{
	HWND hPrev;
	HWND hParent = GetParent(hCtrl);

	while (hParent != pTabCtx->pBK->GetHWND())
	{
		hPrev = hParent;
		hParent = GetParent(hPrev);

		if (IsCtrlSel(pTabCtx, hPrev))
			return TRUE;
	}
	return FALSE;
}


HDC m_hDC = NULL;
HPEN m_hPen = NULL;
HGDIOBJ m_hOld = NULL;

BOOL m_bCtrlLBtnDown = FALSE;
BOOL m_bCtrlRBtnDown = FALSE;

std::vector<RECT> m_rcOrg{};
std::vector<RECT> m_rcLast{};
POINT m_ptOffset{};


BOOL m_PlaceCtrl_bLBtnDown = FALSE;
POINT m_PlaceCtrl_ptStart{};
HDC m_PlaceCtrl_hDC{};
RECT m_PlaceCtrl_rcLast{};
HGDIOBJ m_PlaceCtrl_hOld = NULL;

void OnPlaceCtrlLButtonDown(TABCTX* pTabCtx, HWND hWnd, LPARAM lParam)
{
	HWND hBK = pTabCtx->pBK->GetHWND();

	m_PlaceCtrl_bLBtnDown = TRUE;
	SetCapture(hWnd);

	m_PlaceCtrl_ptStart = GET_PT_LPARAM(lParam);
	MapWindowPoints(hWnd, hBK, &m_PlaceCtrl_ptStart, 1);

	m_PlaceCtrl_hDC = GetDC(hBK);
	SetROP2(m_PlaceCtrl_hDC, R2_NOTXORPEN);
	m_PlaceCtrl_hOld = SelectObject(m_PlaceCtrl_hDC, m_hPen);
}

void OnPlaceCtrlLButtonUp(TABCTX* pTabCtx, LPARAM lParam)
{
	m_PlaceCtrl_bLBtnDown = FALSE;
	ReleaseCapture();

	m_PlaceCtrl_rcLast = {};

	SelectObject(m_PlaceCtrl_hDC, m_PlaceCtrl_hOld);
	ReleaseDC(pTabCtx->pBK->GetHWND(), m_PlaceCtrl_hDC);
	pTabCtx->pBK->Redraw();
}

void OnPlaceCtrlMouseMove(TABCTX* pTabCtx, HWND hWnd, LPARAM lParam)
{
	if (!m_PlaceCtrl_bLBtnDown)
		return;
	POINT pt = GET_PT_LPARAM(lParam);
	MapWindowPoints(hWnd, pTabCtx->pBK->GetHWND(), &pt, 1);
	RECT rc = eck::MakeRect(pt, m_PlaceCtrl_ptStart);

	if (m_PlaceCtrl_rcLast.left != 0 ||
		m_PlaceCtrl_rcLast.top != 0 ||
		m_PlaceCtrl_rcLast.right != 0 ||
		m_PlaceCtrl_rcLast.bottom != 0)
		Rectangle(m_PlaceCtrl_hDC,
			m_PlaceCtrl_rcLast.left,
			m_PlaceCtrl_rcLast.top,
			m_PlaceCtrl_rcLast.right,
			m_PlaceCtrl_rcLast.bottom);
	m_PlaceCtrl_rcLast = rc;
	Rectangle(m_PlaceCtrl_hDC, rc.left, rc.top, rc.right, rc.bottom);
}

std::wstring MakeCtrlName(int idxInfo, HWND hWnd)
{
	return std::wstring(eck::s_EckDesignAllCtrl[idxInfo].pszName) + std::to_wstring((ULONG_PTR)hWnd);
}

eck::CWnd* CreateCtrl(int idxCtrlList, BYTE* pData, TABCTX* pTabCtx, ECK_CREATE_CTRL_EXTRA_ARGS)
{
	auto pWnd = eck::s_EckDesignAllCtrl[idxCtrlList - 1].pfnCreate(pData, ECK_CREATE_CTRL_EXTRA_REALARGS);

	auto pCtx = new CTRLSUBCLASSCTX;
	ZeroMemory(pCtx, sizeof(CTRLSUBCLASSCTX));
	pCtx->idxInfo = idxCtrlList;
	pCtx->pTabCtx = pTabCtx;

	CTRLINFO CtrlInfo;
	CtrlInfo.pWnd = pWnd;
	CtrlInfo.idxInfo = idxCtrlList - 1;
	CtrlInfo.sText = pszText;
	CtrlInfo.pSubclassCtx = pCtx;
	CtrlInfo.sName = MakeCtrlName(idxCtrlList - 1, pWnd->GetHWND());

	pTabCtx->AllCtrls.insert(std::make_pair(pWnd->GetHWND(), std::move(CtrlInfo)));

	SetWindowSubclass(pWnd->GetHWND(), SubclassProc_Ctrl, SCID_DESIGN, (DWORD_PTR)pCtx);
	int idxComboBox=m_CBBCtrl.AddString(CtrlInfo.sName.c_str());
	m_CBBCtrl.SetItemData(idxComboBox, (LPARAM)pWnd->GetHWND());
	return pWnd;
}

void DestroyCtrl(TABCTX* pTabCtx,HWND hWnd)
{
	auto& Info = pTabCtx->AllCtrls[hWnd];
	RemoveWindowSubclass(hWnd, SubclassProc_Ctrl, SCID_DESIGN);
	pTabCtx->AllCtrls.erase(hWnd);
	DestroyWindow(hWnd);
	EckCounter(m_CBBCtrl.GetCount(), i)
	{
		if (m_CBBCtrl.GetItemData(i) == (LPARAM)hWnd)
		{
			m_CBBCtrl.DeleteString(i);
			break;
		}
	}
}

LRESULT CALLBACK SubclassProc_Ctrl(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
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
		if (m_bPlacingCtrl)
		{
			SetCursor(LoadCursorW(NULL, IDC_CROSS));
			return 0;
		}
		break;

	case WM_RBUTTONDOWN:
	{
		m_bCtrlRBtnDown = TRUE;
		SetCapture(hWnd);
	}
	return 0;

	case WM_RBUTTONUP:
	{
		if (!m_bCtrlRBtnDown)
			return 0;
		m_bCtrlRBtnDown = FALSE;
		ReleaseCapture();
		POINT pt = GET_PT_LPARAM(lParam);
		ClientToScreen(hWnd, &pt);
		TrackPopupMenu(m_hMenuWorkWnd, TPM_RIGHTBUTTON, pt.x, pt.y, 0, m_hMain, NULL);
	}
	return 0;

	case WM_LBUTTONDOWN:
	{
		auto pTabCtx = p->pTabCtx;
		/////////////////////////////放置控件
		if (m_bPlacingCtrl)
		{
			OnPlaceCtrlLButtonDown(pTabCtx, hWnd, lParam);
			return 0;
		}
		/////////////////////////////Ctrl键多选
		if (eck::IsBitSet(wParam, MK_CONTROL))
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
			return 0;
		}
		/////////////////////////////
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

		m_ptOffset = GET_PT_LPARAM(lParam);
		RECT* prcOrg = m_rcOrg.data();
		HWND hCurrCtrl;
		EckCounter(cCtrls, i)
		{
			hCurrCtrl = aSelSizer[i]->GetTargetWindow();
			GetWindowRect(hCurrCtrl, prcOrg + i);
			eck::ScreenToClient(hBK, prcOrg + i);
		}
	}
	return 0;

	case WM_LBUTTONUP:
	{
		POINT pt = GET_PT_LPARAM(lParam);
		HWND hBK = p->pTabCtx->pBK->GetHWND();

		if (m_bPlacingCtrl && m_PlaceCtrl_bLBtnDown)
		{
			int idxCurrClass = m_LBCtrl.GetCurrSel();
			OnPlaceCtrlLButtonUp(p->pTabCtx, lParam);
			MapWindowPoints(hWnd, hBK, &pt, 1);
			RECT rc = eck::MakeRect(pt, m_PlaceCtrl_ptStart);
			MapWindowRect(hBK, hWnd, &rc);

			auto pWnd = CreateCtrl(idxCurrClass, NULL, p->pTabCtx, eck::s_EckDesignAllCtrl[idxCurrClass - 1].pszName, WS_CHILD | WS_VISIBLE, 0,
				rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, hWnd, 0);
			SingleSel(p->pTabCtx, pWnd->GetHWND());
			return 0;
		}
		if (!m_bCtrlLBtnDown)
			return 0;
		ReleaseCapture();
		m_bCtrlLBtnDown = FALSE;

		SelectObject(m_hDC, m_hOld);
		ReleaseDC(hBK, m_hDC);

		p->pTabCtx->pBK->SetRedraw(FALSE);
		p->pTabCtx->pWorkWnd->SetRedraw(FALSE);// byd这里SetWindowPos会导致窗口疯狂重画

		RECT rc;
		auto& aSelSizer = p->pTabCtx->pMultiSelMarker;
		SIZE_T cCtrls = aSelSizer.size();
		HWND hCurrCtrl;
		int dx = pt.x - m_ptOffset.x, dy = pt.y - m_ptOffset.y;
		EckCounter(cCtrls, i)
		{
			hCurrCtrl = aSelSizer[i]->GetTargetWindow();

			if (IsCtrlParentSel(p->pTabCtx, hCurrCtrl))
				continue;

			rc = m_rcOrg[i];
			OffsetRect(&rc, dx, dy);
			MapWindowRect(hBK, GetParent(hCurrCtrl), &rc);

			SetWindowPos(hCurrCtrl, NULL, rc.left, rc.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
		}

		for (auto x : aSelSizer)
			x->MoveToTargetWindow();

		p->pTabCtx->pBK->SetRedraw(TRUE);
		p->pTabCtx->pWorkWnd->SetRedraw(TRUE);
		p->pTabCtx->pBK->Redraw();
		m_rcLast.clear();
	}
	return 0;

	case WM_MOUSEMOVE:
	{
		if (m_bPlacingCtrl)
		{
			OnPlaceCtrlMouseMove(p->pTabCtx, hWnd, lParam);
			return 0;
		}
		if (!m_bCtrlLBtnDown)
			return 0;
		POINT pt = GET_PT_LPARAM(lParam);

		RECT rc;
		auto& aSelSizer = p->pTabCtx->pMultiSelMarker;
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
	return 0;
	}

	return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK WndProc_WDBk(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		BeginPaint(hWnd, &ps);
		FillRect(ps.hdc, &ps.rcPaint, GetSysColorBrush(COLOR_APPWORKSPACE));
		EndPaint(hWnd, &ps);
	}
	return 0;
	}
	return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK WndProc_WorkWindow(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	auto p = (TABCTX*)GetWindowLongPtrW(hWnd, 0);

	BOOL s_bRBtnDown = FALSE;
	switch (uMsg)
	{
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		BeginPaint(hWnd, &ps);
		FillRect(ps.hdc, &ps.rcPaint, GetSysColorBrush(COLOR_BTNFACE));
		EndPaint(hWnd, &ps);
	}
	return 0;

	case WM_MOVE:
		for (auto x : p->pMultiSelMarker)
			x->MoveToTargetWindow();
		return 0;

	case WM_SETCURSOR:
	{
		if (m_bPlacingCtrl)
		{
			SetCursor(LoadCursorW(NULL, IDC_CROSS));
			return 0;
		}
	}
	break;

	case WM_LBUTTONDOWN:
		OnPlaceCtrlLButtonDown(p, hWnd, lParam);
		return 0;

	case WM_LBUTTONUP:
	{
		if (!m_PlaceCtrl_bLBtnDown)
			return 0;

		OnPlaceCtrlLButtonUp(p, lParam);

		if (eck::IsBitSet(wParam, MK_CONTROL))
			return 0;

		HWND hBK = p->pBK->GetHWND();
		POINT pt = GET_PT_LPARAM(lParam);
		MapWindowPoints(hWnd, hBK, &pt, 1);

		int idxCurr = m_LBCtrl.GetCurrSel();
		if (idxCurr <= 0)// 鼠标指针模式
		{
			if (pt.x - m_PlaceCtrl_ptStart.x == 0 || pt.y - m_PlaceCtrl_ptStart.y == 0)
				SingleSel(p, hWnd);// 矩形无效，选定工作窗口
			else// 矩形有效，选中矩形内的所有控件
			{
				ClearMultiSelMarker(p);
				RECT rc = eck::MakeRect(pt, m_PlaceCtrl_ptStart);
				eck::ClientToScreen(hBK, &rc);
				RECT rcCtrl;
				HWND hCtrl;
				for (auto& x : p->AllCtrls)
				{
					hCtrl = x.second.pWnd->GetHWND();
					GetWindowRect(hCtrl, &rcCtrl);
					if (eck::IsRectsIntersect(&rc, &rcCtrl))
						AddMultiSel(p, hCtrl);
				}
			}
		}
		else// 组件设计模式
		{
			RECT rc = eck::MakeRect(pt, m_PlaceCtrl_ptStart);
			MapWindowRect(hBK, hWnd, &rc);

			auto pWnd = CreateCtrl(idxCurr, NULL, p, eck::s_EckDesignAllCtrl[idxCurr - 1].pszName, WS_CHILD | WS_VISIBLE, 0,
				rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, hWnd, 0);
			SingleSel(p, pWnd->GetHWND());
		}
	}
	return 0;

	case WM_MOUSEMOVE:
		OnPlaceCtrlMouseMove(p, hWnd, lParam);
		return 0;

	case WM_CLOSE:
		return 0;

	case WM_RBUTTONDOWN:
	{
		SetCapture(hWnd);
		s_bRBtnDown = TRUE;
	}
	return 0;

	case WM_RBUTTONUP:
	{
		if (!s_bRBtnDown)
			return 0;
		ReleaseCapture();
		s_bRBtnDown = FALSE;
		POINT pt = GET_PT_LPARAM(lParam);
		ClientToScreen(hWnd, &pt);
		TrackPopupMenu(m_hMenuWorkWnd, TPM_RIGHTBUTTON, pt.x, pt.y, 0, m_hMain, NULL);
	}
	return 0;
	}
	return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

void NewTab()
{
	m_Tab.InsertItem(L"新窗体");

	auto pCtx = new TABCTX;
	pCtx->pBK = new eck::CBk;
	pCtx->pWorkWnd = new eck::CBk;

	pCtx->pBK->Create(NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | WS_CLIPSIBLINGS, 0, 0, 0, 0, 0, m_Tab, IDC_BK_MAIN);
	pCtx->pBK->SetWindowProc(WndProc_WDBk);

	pCtx->pWorkWnd->Create(NULL, WS_CHILD | WS_VISIBLE | WS_CAPTION | WS_SYSMENU | WS_CLIPSIBLINGS,
		0, m_DS.iPadding, m_DS.iPadding, DPI(400), DPI(300), pCtx->pBK->GetHWND(), IDC_BK_WORKWND);
	pCtx->pWorkWnd->SetWindowProc(WndProc_WorkWindow);
	SetWindowLongPtrW(pCtx->pWorkWnd->GetHWND(), 0, (LONG_PTR)pCtx);

	SetWindowPos(pCtx->pWorkWnd->GetHWND(), HWND_BOTTOM, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);

	m_Tabs.push_back(pCtx);

	FillCtrlComboBox(m_Tabs[m_Tab.GetCurSel()]);

	SingleSel(pCtx, pCtx->pWorkWnd->GetHWND());
}

LRESULT CALLBACK WndProc_WDMain(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CREATE:
	{
		m_iDpi = eck::GetDpi(hWnd);
		UpdateDpiSize(m_iDpi);
		m_hFontCtrl = eck::EzFont(L"微软雅黑", 9);

		m_hMenuWorkWnd = CreatePopupMenu();
		AppendMenuW(m_hMenuWorkWnd, 0, IDMI_WW_COPY, L"复制");
		AppendMenuW(m_hMenuWorkWnd, 0, IDMI_WW_CUT, L"剪切");
		AppendMenuW(m_hMenuWorkWnd, 0, IDMI_WW_PASTE, L"粘贴");
		AppendMenuW(m_hMenuWorkWnd, 0, IDMI_WW_DEL, L"删除");

		m_hPen = CreatePen(PS_SOLID, 2, eck::Colorref::Red);
		m_cxInfo = DPI(240);
		m_cxCtrlBox = DPI(180);

		m_CBBCtrl.Create(NULL, WS_VISIBLE | CBS_DROPDOWNLIST | CBS_SORT,
			0, 0, 50, 50, 0, hWnd, IDC_CBB_CTRL);
		m_CBBCtrl.SetItemHeight(m_DS.cyComboBox);

		m_LVProp.Create(NULL, WS_VISIBLE | WS_BORDER /*| LVS_NOCOLUMNHEADER*/ | LVS_REPORT,
			0, 0, 0, 0, 0, hWnd, IDC_LV_CTRLINFO);
		EckLVSetItemHeight(m_LVProp, DPI(24));
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

		m_EDDesc.SetMultiLine(TRUE);
		m_EDDesc.Create(NULL, WS_VISIBLE | ES_AUTOVSCROLL, 0, 0, 0, 0, 0, hWnd, IDC_ED_DESC);
		m_EDDesc.SetFrameType(3);

		m_LBCtrl.Create(NULL, WS_VISIBLE | WS_BORDER | LBS_NOINTEGRALHEIGHT | LBS_OWNERDRAWFIXED | LBS_NOTIFY | LBS_NODATA,
			0, 0, 0, 0, 0, hWnd, IDC_LB_CTRL);
		m_LBCtrl.SetCount(ARRAYSIZE(eck::s_EckDesignAllCtrl) + 1);

		m_Tab.Create(NULL, WS_VISIBLE | TCS_SINGLELINE, 0, 0, 0, 0, 0, hWnd, IDC_TC_MAIN);

		eck::SetFontForWndAndCtrl(hWnd, m_hFontCtrl);

		NewTab();
	}
	return 0;

	case WM_MEASUREITEM:
	{
		if (wParam != IDC_LB_CTRL)
			break;
		auto pmis = (MEASUREITEMSTRUCT*)lParam;
		pmis->itemHeight = DPI(32);
	}
	return TRUE;

	case WM_DRAWITEM:
	{
		auto pdis = (DRAWITEMSTRUCT*)lParam;
		if (pdis->hwndItem != m_LBCtrl)
			break;

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
		HGDIOBJ hOld = SelectObject(hDC, m_hFont);
		PCWSTR pszText = (pdis->itemID ? eck::s_EckDesignAllCtrl[pdis->itemID - 1].pszName : L"指针");

		DrawTextW(hDC, pszText, -1, &pdis->rcItem, DT_SINGLELINE | DT_LEFT | DT_VCENTER);
		SelectObject(hDC, hOld);
	}
	return TRUE;

	case WM_COMMAND:
	{
		UINT uCtrlID = LOWORD(wParam);
		HWND hCtrl = (HWND)lParam;
		if (hCtrl)
		{
			switch (HIWORD(wParam))
			{
			case LBN_SELCHANGE:// 与CBN_SELCHANGE相等
				switch (uCtrlID)
				{
				case IDC_LB_CTRL:
					m_bPlacingCtrl = (m_LBCtrl.GetCurrSel() != 0);
					break;
				case IDC_CBB_CTRL:
				{
					SingleSel(m_Tabs[m_Tab.GetCurSel()], (HWND)m_CBBCtrl.GetItemData(m_CBBCtrl.GetCurrSel()));
				}
				break;
				}
			}
		}
		else
		{
			int idx = m_Tab.GetCurSel();
			auto pTabCtx = m_Tabs[idx];
			switch (uCtrlID)
			{
			case IDMI_WW_COPY:
			{
				OpenClipboard(hWnd);
				EmptyClipboard();
				HWND hCtrl;
				SIZE_T cSel = pTabCtx->pMultiSelMarker.size();
				SIZE_T cb = sizeof(CBINFOCTRLHEADER);
				RECT rc;


				std::vector<RECT> aRects;// 所有有效窗口的矩形
				std::vector<VALIDSELCTRL> aValidWnds;// 所有有效窗口的描述结构
				int minLeft = INT_MAX, minTop = INT_MAX;// 最小的左边和顶边

				// 记录有效控件
				std::function<void(HWND hWnd, CTRLINFO& Info)> ProcessValidCtrl = [&](HWND hWnd, CTRLINFO& Info)
				{
					cb += ((Info.sText.size() + 1) * sizeof(WCHAR) + Info.rbData.m_cb);

					GetWindowRect(hWnd, &rc);
					if (rc.left < minLeft)
						minLeft = rc.left;
					if (rc.top < minTop)
						minTop = rc.top;

					aRects.push_back(rc);
					aValidWnds.push_back({ hWnd,0 });
					int idxCurrParent = (int)aValidWnds.size() - 1;

					HWND hTemp = GetWindow(hWnd, GW_CHILD);
					while (hTemp)
					{
						++aValidWnds[idxCurrParent].cChildren;
						ProcessValidCtrl(hTemp, pTabCtx->AllCtrls[hTemp]);
						hTemp = GetWindow(hTemp, GW_HWNDNEXT);
					}
				};

				aRects.reserve(cSel);
				aValidWnds.reserve(cSel);

				for (auto x : pTabCtx->pMultiSelMarker)
				{
					hCtrl = x->GetTargetWindow();
					auto& Info = pTabCtx->AllCtrls[hCtrl];

					if (IsCtrlParentSel(pTabCtx, hCtrl))// 如果一个控件的某上级窗口被选择，那么不应该处理
						continue;

					ProcessValidCtrl(hCtrl, Info);
				}

				cb += (sizeof(CBINFOCTRL) * aValidWnds.size());

				HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, cb);
				void* pData = GlobalLock(hGlobal);
				eck::CMemWriter w(pData);

				CBINFOCTRLHEADER Header;
				Header.iVer = CBFVER_DESIGN_CTRL_1;
				Header.cCtrls = (int)aValidWnds.size();
				w << Header;

				CBINFOCTRL CtrlInfo;
				EckCounter(aValidWnds.size(), i)
				{
					hCtrl = aValidWnds[i].hWnd;
					auto& Info = pTabCtx->AllCtrls[hCtrl];

					CtrlInfo.cchText = (int)Info.sText.size();
					CtrlInfo.cbData = (DWORD)Info.rbData.m_cb;
					CtrlInfo.dwStyle = (DWORD)GetWindowLongPtrW(hCtrl, GWL_STYLE);
					CtrlInfo.dwExStyle = (DWORD)GetWindowLongPtrW(hCtrl, GWL_EXSTYLE);
					CtrlInfo.idxInfo = Info.idxInfo;
					CtrlInfo.rc.left = aRects[i].left - minLeft;
					CtrlInfo.rc.top = aRects[i].top - minTop;
					CtrlInfo.rc.right = aRects[i].right - aRects[i].left;
					CtrlInfo.rc.bottom = aRects[i].bottom - aRects[i].top;
					CtrlInfo.cChildren = aValidWnds[i].cChildren;

					w << CtrlInfo << Info.sText << Info.rbData;
				}

				EckDbgPrintFmt(L"剪贴板数据  距离 = %I64d  分配 = %I64d", w.m_pMem - (BYTE*)pData, cb);
				GlobalUnlock(hGlobal);
				SetClipboardData(m_uCFCtrlInfo, hGlobal);
				CloseClipboard();
			}
			return 0;

			case IDMI_WW_PASTE:
			{
				OpenClipboard(hWnd);
				if (!IsClipboardFormatAvailable(m_uCFCtrlInfo))
				{
					CloseClipboard();
					return 0;
				}

				HGLOBAL hGlobal = GetClipboardData(m_uCFCtrlInfo);
				void* pData = GlobalLock(hGlobal);
				eck::CMemReader r(pData);

				auto pHeader = (CBINFOCTRLHEADER*)r.m_pMem;
				r += sizeof(CBINFOCTRLHEADER);

				CBINFOCTRL* pCtrlInfo;
				PCWSTR pszText;
				BYTE* pCtrlData;
				HWND hParent;
				eck::CWnd* pWnd;
				struct PARENTINFO
				{
					int idxParent;
					int cChildren;
					int j;
					HWND hParent;
				};
				std::stack<PARENTINFO> stackParentInfo{};

				int j = 0;
				EckCounter(pHeader->cCtrls, i)
				{
					pCtrlInfo = (CBINFOCTRL*)r.m_pMem;
					r += sizeof(CBINFOCTRL);
					pszText = (PCWSTR)r.m_pMem;
					r += ((pCtrlInfo->cchText + 1) * sizeof(WCHAR));
					pCtrlData = r.m_pMem;
					r += pCtrlInfo->cbData;

					if (stackParentInfo.size())
					{
						auto& Top = stackParentInfo.top();
						hParent = Top.hParent;
						++Top.j;
						if (Top.j == Top.cChildren)
							stackParentInfo.pop();
					}
					else
						hParent = pTabCtx->pWorkWnd->GetHWND();

					RECT rc = pCtrlInfo->rc;
					rc.right += rc.left;
					rc.bottom += rc.top;
					MapWindowRect(pTabCtx->pWorkWnd->GetHWND(), hParent, &rc);
					rc.right -= rc.left;
					rc.bottom -= rc.top;

					pWnd = CreateCtrl(pCtrlInfo->idxInfo + 1, pCtrlData, pTabCtx, pszText, pCtrlInfo->dwStyle, pCtrlInfo->dwExStyle,
						rc.left, rc.top, rc.right, rc.bottom, hParent, 0);
					if (pCtrlInfo->cChildren)
						stackParentInfo.push({ i,pCtrlInfo->cChildren,0,pWnd->GetHWND() });
				}

				GlobalUnlock(hGlobal);
				CloseClipboard();
			}
			return 0;

			case IDMI_WW_DEL:
			{
				std::function<void(HWND hWnd)> DelCtrl = [&](HWND hWnd)
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
					DestroyCtrl(pTabCtx,x);
				}

				SingleSel(pTabCtx, pTabCtx->pWorkWnd->GetHWND());
			}
			return 0;
			}
		}
	}
	break;

	case WM_NOTIFY:
	{
		auto pnmhdr = (NMHDR*)lParam;
		switch (pnmhdr->code)
		{
		case TCN_SELCHANGE:
		{
			if (pnmhdr->hwndFrom != m_Tab)
				break;
			for (auto x : m_Tabs)
				ShowWindow(x->pBK->GetHWND(), SW_HIDE);
			ShowWindow(m_Tabs[m_Tab.GetCurSel()]->pBK->GetHWND(), SW_SHOWNOACTIVATE);
		}
		return 0;
		}
	}
	break;

	case WM_SIZE:
	{
		int cxClient = LOWORD(lParam),
			cyClient = HIWORD(lParam);
		HDWP hDwp = BeginDeferWindowPos(10);
		hDwp = DeferWindowPos(hDwp, m_CBBCtrl, NULL,
			m_DS.iPadding,
			m_DS.iPadding,
			m_cxInfo,
			DPI(24),
			SWP_NOZORDER | SWP_NOACTIVATE);

		int y = m_DS.iPadding + m_DS.cyComboBox;

		int cyLVProp = cyClient - y - DPI(80) - m_DS.iPadding * 3;
		hDwp = DeferWindowPos(hDwp, m_LVProp, NULL,
			m_DS.iPadding,
			y + m_DS.iPadding,
			m_cxInfo,
			cyLVProp,
			SWP_NOZORDER);

		hDwp = DeferWindowPos(hDwp, m_EDDesc, NULL,
			m_DS.iPadding,
			y + cyLVProp + m_DS.iPadding * 2,
			m_cxInfo,
			DPI(80),
			SWP_NOZORDER);

		int cyBK = cyClient - m_DS.iPadding * 2;
		hDwp = DeferWindowPos(hDwp, m_Tab, NULL,
			m_cxInfo + DPI(6) + m_DS.iPadding,
			m_DS.iPadding,
			cxClient - m_cxInfo - m_cxCtrlBox - m_DS.iPadding * 2 - DPI(6) * 2,
			cyBK,
			SWP_NOZORDER);

		hDwp = DeferWindowPos(hDwp, m_LBCtrl, NULL,
			cxClient - m_cxCtrlBox - m_DS.iPadding,
			m_DS.iPadding,
			m_cxCtrlBox,
			cyBK,
			SWP_NOZORDER);

		EndDeferWindowPos(hDwp);

		RECT rc;
		GetWindowRect(m_Tab, &rc);
		m_Tab.AdjustRect(&rc, FALSE);
		eck::ScreenToClient(m_Tab, &rc);
		for (auto x : m_Tabs)
			SetWindowPos(x->pBK->GetHWND(), NULL, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOZORDER);
	}
	return 0;

	case WM_DPICHANGED:
	{
		int iOldDpi = m_iDpi;
		UpdateDpiSize(LOWORD(wParam));
		ImageList_Destroy(EckLVSetItemHeight(m_LVProp.GetHWND(), DPI(24)));

		eck::OnDpiChanged_Parent_PreMonV2(hWnd, m_iDpi, iOldDpi);
	}
	return 0;
	}

	return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}