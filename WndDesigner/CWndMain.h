#pragma once
#include "CApp.h"
#include "CWorkWnd.h"
#include "CSizer.h"
#include "CPropList.h"

#include <unordered_set>
#include <unordered_map>
#include <stack>
#include <functional>

#include "..\eck\WndHelper.h"
#include "..\eck\CListBox.h"
#include "..\eck\CComboBox.h"
#include "..\eck\CListView.h"
#include "..\eck\CEditExt.h"
#include "..\eck\CBk.h"
#include "..\eck\CTab.h"
#include "..\eck\Utility.h"
#include "..\eck\DesignerDef.h"
#include "..\eck\ResStruct.h"
#include "..\eck\CFormTable.h"

inline POINT PtAlign(POINT pt, int iUnit)
{
	int ii = (pt.x % iUnit) * iUnit;
	int iDelta = pt.x - ii;
	pt.x = (iDelta > iUnit / 2 ? ii + iUnit : ii);
	ii = (pt.y % iUnit) * iUnit;
	iDelta = pt.y - ii;
	pt.y = (iDelta > iUnit / 2 ? ii + iUnit : ii);
	return pt;
}

constexpr PCWSTR WCN_WDMAIN = L"Eck.WndClass.WndDesigner.Main";

struct TABCTX;
class CWndMain;

struct CTRLSUBCLASSCTX
{
	int idxInfo;
	TABCTX* pTabCtx;
	CWndMain* pThis;
};

struct WDCTRLINFO
{
	eck::CWnd* pWnd = NULL;
	int idxInfo = -1;
	CTRLSUBCLASSCTX* pSubclassCtx = NULL;

	WDCTRLINFO() = default;

	WDCTRLINFO(WDCTRLINFO&& x) noexcept
	{
		pWnd = x.pWnd;
		idxInfo = x.idxInfo;
		pSubclassCtx = x.pSubclassCtx;
		x.pWnd = NULL;
		x.pSubclassCtx = NULL;
		x.idxInfo = -1;
	}

	~WDCTRLINFO()
	{
		delete pWnd;
		delete pSubclassCtx;
	}
};

struct TABCTX
{
	CWorkWndBk* pBK = NULL;
	CWorkWnd* pWorkWnd = NULL;
	std::vector<CSizer*> pMultiSelMarker{};
	std::unordered_set<HWND> SelCtrl{};
	std::unordered_map<HWND, WDCTRLINFO> AllCtrls{};
	CWndMain* pThis;

	~TABCTX()
	{
		delete pBK;
		delete pWorkWnd;
		for (auto x : pMultiSelMarker)
			delete x;
	}
};

#define CBFVER_DESIGN_CTRL_1 1
struct VALIDSELCTRL
{
	HWND hWnd;
	int cChildren;
};

class CCtrlsSerializer
{
private:
	std::vector<RECT> m_aRects{};// 所有矩形，无DPI缩放
	std::vector<VALIDSELCTRL> m_aValidWnds{};// 所有描述结构
	std::vector<eck::CRefBin> m_aData{};// 所有数据
	int m_minLeft = INT_MAX, m_minTop = INT_MAX;// 最小的左边和顶边

	TABCTX* m_pTabCtx = NULL;

	int m_iDpi = USER_DEFAULT_SCREEN_DPI;

	SIZE_T m_cbTotal = sizeof(eck::FTCTRLDATAHEADER);

	void AddCtrlInt(HWND hWnd)
	{
		RECT rc;
		GetWindowRect(hWnd, &rc);
		eck::DpiScale(rc, USER_DEFAULT_SCREEN_DPI, m_iDpi);
		if (rc.left < m_minLeft)
			m_minLeft = rc.left;
		if (rc.top < m_minTop)
			m_minTop = rc.top;

		m_aRects.push_back(rc);
		m_aValidWnds.push_back({ hWnd,0 });

		auto& Info = m_pTabCtx->AllCtrls[hWnd];

		m_aData.emplace_back(Info.pWnd->SerializeData());
		m_cbTotal += (m_aData.back().Size() + sizeof(eck::FTCTRLDATA));
	}
public:
	CCtrlsSerializer() = default;

	CCtrlsSerializer(TABCTX* pTabCtx, int iDpi) :m_pTabCtx(pTabCtx), m_iDpi(iDpi)
	{

	}

	void Reserve(int c)
	{
		m_aRects.reserve(c);
		m_aValidWnds.reserve(c);
	}

	int GetCount()
	{
		return (int)m_aValidWnds.size();
	}

	void SetTabCtx(TABCTX* pTabCtx)
	{
		m_pTabCtx = pTabCtx;
	}

	void SetDpi(int iDpi)
	{
		m_iDpi = iDpi;
	}

	void AddCtrl(HWND hWnd)
	{
		AddCtrlInt(hWnd);

		int idxCurrParent = GetCount() - 1;
		HWND hTemp = GetWindow(hWnd, GW_CHILD);
		while (hTemp)
		{
			++m_aValidWnds[idxCurrParent].cChildren;
			AddCtrl(hTemp);
			hTemp = GetWindow(hTemp, GW_HWNDNEXT);
		}
	};

	SIZE_T GetDataSize()
	{
		return m_cbTotal;
	}

	void Serialize(PVOID pMem)
	{
		eck::CMemWriter w(pMem, m_cbTotal);

		eck::FTCTRLDATAHEADER* pHeader;
		w.SkipPointer(pHeader);
		pHeader->iVer = CBFVER_DESIGN_CTRL_1;
		pHeader->cCtrls = GetCount();

		HWND hCtrl;
		eck::FTCTRLDATA* pCtrlInfo;
		EckCounter(pHeader->cCtrls, i)
		{
			hCtrl = m_aValidWnds[i].hWnd;
			auto& Info = m_pTabCtx->AllCtrls[hCtrl];

			w.SkipPointer(pCtrlInfo);
			auto& rbData = m_aData[i];

			pCtrlInfo->cbData = (UINT)rbData.Size();
			pCtrlInfo->idxInfo = Info.idxInfo;
			pCtrlInfo->rc.left = m_aRects[i].left - m_minLeft;
			pCtrlInfo->rc.top = m_aRects[i].top - m_minTop;
			pCtrlInfo->rc.right = m_aRects[i].right - m_aRects[i].left;
			pCtrlInfo->rc.bottom = m_aRects[i].bottom - m_aRects[i].top;
			pCtrlInfo->cChildren = m_aValidWnds[i].cChildren;

			w << rbData;
		}
	}

	eck::CRefBin Serialize()
	{
		eck::CRefBin rb(m_cbTotal);
		Serialize(rb);
		return rb;
	}
};

class CCtrlPlacingEventHandler
{
	BOOL m_bLBtnDown = FALSE;
	POINT m_ptStart{};
	HDC m_hDC{};
	RECT m_rcLast{};
	HGDIOBJ m_hOld = NULL;
	HPEN m_hPen = NULL;
public:
	void UpdateDpiSize(int iDpi)
	{
		DeleteObject(m_hPen);
		m_hPen = CreatePen(PS_SOLID, eck::DpiScale(2, iDpi), eck::Colorref::Red);
	}

	BOOL IsLBtnDown()
	{
		return m_bLBtnDown;
	}

	POINT GetStartPoint()
	{
		return m_ptStart;
	}

	void OnLButtonDown(TABCTX* pTabCtx, HWND hWnd, LPARAM lParam);

	void OnLButtonUp(TABCTX* pTabCtx, LPARAM lParam)
	{
		m_bLBtnDown = FALSE;
		ReleaseCapture();

		m_rcLast = {};

		SelectObject(m_hDC, m_hOld);
		ReleaseDC(pTabCtx->pBK->GetHWND(), m_hDC);
		pTabCtx->pBK->Redraw();
	}

	void OnMouseMove(TABCTX* pTabCtx, HWND hWnd, LPARAM lParam)
	{
		if (!m_bLBtnDown)
			return;
		POINT pt = GET_PT_LPARAM(lParam);
		MapWindowPoints(hWnd, pTabCtx->pBK->GetHWND(), &pt, 1);
		RECT rc = eck::MakeRect(pt, m_ptStart);

		if (m_rcLast.left != 0 ||
			m_rcLast.top != 0 ||
			m_rcLast.right != 0 ||
			m_rcLast.bottom != 0)
			Rectangle(m_hDC,
				m_rcLast.left,
				m_rcLast.top,
				m_rcLast.right,
				m_rcLast.bottom);
		m_rcLast = rc;
		Rectangle(m_hDC, rc.left, rc.top, rc.right, rc.bottom);
	}
};

#define CNM_PROPLISTNOTIFY	(WM_USER + 1)
class CWndMain :public eck::CWnd
{
private:
	enum
	{
		IDC_BK_MAIN = 101,
		IDC_CBB_CTRL,
		IDC_LB_CTRL,
		IDC_LV_CTRLINFO,
		IDC_ED_DESC,
		IDC_BK_WORKWND,
		IDC_TC_MAIN,
		IDC_TC_PROJ,
	};

	enum
	{
		IDMI_WW_COPY = 101,
		IDMI_WW_CUT,
		IDMI_WW_PASTE,
		IDMI_WW_DEL,
	};

	eck::CComboBox m_CBBCtrl;
	eck::CListBox m_LBCtrl;
	CPropList m_LVProp;
	eck::CEditExt m_EDDesc;
	eck::CTab m_Tab;
	eck::CTab m_TBProj{};

	std::vector<TABCTX*> m_vTabs{};
	int m_idxCurrTab = -1;

	HFONT m_hFontComm = NULL;
	HFONT m_hFontCtrlBox = NULL;

	BOOL m_bPlacingCtrl = FALSE;
	CCtrlPlacingEventHandler m_CtrlPlacing{};

	HMENU m_hMenuWorkWnd = NULL;


	int m_cxInfo, m_cxCtrlBox;

	int m_iDpi = USER_DEFAULT_SCREEN_DPI;
	ECK_DS_BEGIN(DPIS)
		ECK_DS_ENTRY(iPadding, 12)
		ECK_DS_ENTRY(cyComboBox, 12)
		ECK_DS_ENTRY(sizeBlock, 8)
		;
	ECK_DS_END_VAR(m_Ds);
	EckInline int DPI(int i)
	{
		return eck::DpiScale(i, m_iDpi);
	}

	void UpdateDpi()
	{
		eck::UpdateDpiSize(m_Ds, m_iDpi);
		m_CtrlPlacing.UpdateDpiSize(m_iDpi);
	}

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	BOOL OnCreate(HWND hWnd, CREATESTRUCTW* pcs);

	void OnMeasureItem(HWND hWnd, MEASUREITEMSTRUCT* pmis);

	void OnDrawItem(HWND hWnd, const DRAWITEMSTRUCT* pdis);

	void OnCommand(HWND hWnd, int idCtrl, HWND hCtrl, UINT uNotifyCode);

	void OnMenuCopy(HWND hWnd);

	void OnMenuPaste(HWND hWnd);

	void OnMenuDel(HWND hWnd);

	void OnMenuSaveWndTable(HWND hWnd);

	LRESULT OnNotify(HWND hWnd, int idCtrl, NMHDR* pnmhdr);

	void OnSize(HWND hWnd, UINT nType, int cxClient, int cyClient);

	void OnDpiChanged(HWND hWnd, int iDpiX, int iDpiY, RECT* prc);

	LRESULT OnPropListNotify(HWND hWnd, WPARAM wParam, LPARAM lParam);


#pragma region 控件消息处理
	static LRESULT CALLBACK SubclassProc_Ctrl(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
		UINT_PTR uIdSubclass, DWORD_PTR dwRefData);

	BOOL OnCtrlSetCursor(HWND hWnd, HWND hWndCursor, UINT nHitTest, UINT uMsg);

	void OnCtrlRButtonDown(HWND hWnd, BOOL bDoubleClick, int x, int y, UINT uKeyFlags);

	void OnCtrlRButtonUp(HWND hWnd, int x, int y, UINT uFlags);

	void OnCtrlMove(HWND hWnd, int x, int y);

	void OnCtrlSize(HWND hWnd, UINT nType, int cx, int cy);

	void OnCtrlLButtonDown(HWND hWnd, UINT uKeyFlags, int x, int y, CTRLSUBCLASSCTX* p);

	void OnCtrlLButtonUp(HWND hWnd, UINT uKeyFlags, int x, int y, CTRLSUBCLASSCTX* p);

	void OnCtrlMouseMove(HWND hWnd, int x, int y, CTRLSUBCLASSCTX* p);
#pragma endregion

	static LRESULT CALLBACK WndProc_WorkWnd(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	void OnWWLButtonUp(HWND hWnd, int x, int y, UINT uKeyFlags, TABCTX* p);

	void OnWWRButtonUp(HWND hWnd, UINT uKeyFlags, int x, int y, TABCTX* p);




	void NewTab(int posTab = -1);

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
		pSizer->SetBlockSize(m_Ds.sizeBlock);
		pSizer->Create(pTabCtx->pBK->GetHWND(), pTabCtx->pWorkWnd->GetHWND());
		pSizer->SetTargetWindow(hCtrl);
		pTabCtx->pMultiSelMarker.push_back(pSizer);
		pTabCtx->SelCtrl.insert(hCtrl);
	}

	void FillPropList(TABCTX* pTabCtx)
	{
		int idxCBB = m_CBBCtrl.GetCurSel();

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
			idx = m_CBBCtrl.AddString(x.second.pWnd->m_DDBase.rsName.Data());
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

	eck::CRefStrW MakeCtrlName(int idxInfo, HWND hWnd)
	{
		return eck::CRefStrW(eck::s_EckDesignAllCtrl[idxInfo].pszName) + eck::ToStr((ULONG_PTR)hWnd);
	}

	eck::CWnd* CreateCtrl(int idxCtrlList, eck::PCBYTE pData, TABCTX* pTabCtx, ECK_CREATE_CTRL_EXTRA_ARGS)
	{
		auto pWnd = eck::s_EckDesignAllCtrl[idxCtrlList - 1].pfnCreate(pData, ECK_CREATE_CTRL_EXTRA_REALARGS);

		auto pCtx = new CTRLSUBCLASSCTX{ idxCtrlList,pTabCtx,this };

		WDCTRLINFO CtrlInfo;
		CtrlInfo.pWnd = pWnd;
		CtrlInfo.idxInfo = idxCtrlList - 1;
		CtrlInfo.pSubclassCtx = pCtx;
		CtrlInfo.pWnd->m_DDBase.rsName = MakeCtrlName(idxCtrlList - 1, pWnd->GetHWND());

		int idxComboBox = m_CBBCtrl.AddString(CtrlInfo.pWnd->m_DDBase.rsName.Data());
		m_CBBCtrl.SetItemData(idxComboBox, (LPARAM)pWnd->GetHWND());

		pTabCtx->AllCtrls.insert(std::make_pair(pWnd->GetHWND(), std::move(CtrlInfo)));
		SetWindowSubclass(pWnd->GetHWND(), SubclassProc_Ctrl, SCID_DESIGN, (DWORD_PTR)pCtx);
		return pWnd;
	}

	void DestroyCtrl(TABCTX* pTabCtx, HWND hWnd)
	{
		auto& Info = pTabCtx->AllCtrls[hWnd];
		RemoveWindowSubclass(hWnd, SubclassProc_Ctrl, SCID_DESIGN);
		DestroyWindow(hWnd);
		pTabCtx->AllCtrls.erase(hWnd);

		EckCounter(m_CBBCtrl.GetCount(), i)
		{
			if (m_CBBCtrl.GetItemData(i) == (LPARAM)hWnd)
			{
				m_CBBCtrl.DeleteString(i);
				break;
			}
		}
	}
public:
	static ATOM RegisterWndClass();

	
	ECK_CWND_CREATE;
};