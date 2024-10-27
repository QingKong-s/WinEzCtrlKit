/*
* WinEzCtrlKit Library
*
* CDialog.h ： 对话框
*
* Copyright(C) 2023-2024 QingKong
*/
#pragma once
#include "CWnd.h"
#include "DlgHelper.h"

ECK_NAMESPACE_BEGIN
class CDialog :public CWnd
{
protected:
	INT_PTR m_iResult = 0;
	HWND m_hTop = nullptr;
	COLORREF m_crBkg = CLR_DEFAULT;

#if ECKCXX20
	BITBOOL m_bUseDefBkClr : 1 = TRUE;
#	ifdef _DEBUG
	BITBOOL m_bDlgProcInit : 1 = FALSE;
#	endif
	BITBOOL m_bModal : 1 = FALSE;
	BITBOOL m_bClrDisableEdit : 1 = FALSE;
#else
	union
	{
		struct
		{
			BITBOOL m_bUseDefBkClr : 1;
#	ifdef _DEBUG
			BITBOOL m_bDlgProcInit : 1;
#	endif
			BITBOOL m_bModal : 1;
			BITBOOL m_bClrDisableEdit : 1;
		};
		BITBOOL ECKPRIV_BITBOOL_DUMMY{ 0b1 };
	};
#endif


	EckInline HWND PreModal(HWND hParent)
	{
		m_bModal = TRUE;
		return GetSafeOwner(hParent, &m_hTop);
	}

	EckInline void PostModal()
	{
		if (!m_hTop)
			return;
		m_bModal = FALSE;
		EnableWindow(m_hTop, TRUE);
		m_hTop = nullptr;
	}

	void MsgLoop()
	{
		MSG msg;
		while (GetMessageW(&msg, nullptr, 0, 0))
		{
			if (!eck::PreTranslateMessage(msg))
			{
				if (!IsDialogMessageW(m_hWnd, &msg))
				{
					TranslateMessage(&msg);
					DispatchMessageW(&msg);
				}
			}
		}
	}

	EckInline INT_PTR IntCreateModalDlg(HINSTANCE hInst, PCWSTR pszTemplate, HWND hParent,
		LPARAM lParam = 0, FWndCreating pfnCreatingProc = nullptr)
	{
		m_bModal = TRUE;

		const HWND hOwner = PreModal(hParent);
		BOOL bNeedEnableOwner;
		if (hOwner && hOwner != GetDesktopWindow() && IsWindowEnabled(hOwner))
		{
			bNeedEnableOwner = TRUE;
			EnableWindow(hOwner, FALSE);
		}
		else
			bNeedEnableOwner = FALSE;

		BeginCbtHook(this, pfnCreatingProc);
		CreateDialogParamW(hInst, pszTemplate, hParent, EckDlgProc, lParam);
		EckAssert(m_bDlgProcInit);

		MsgLoop();
		if (bNeedEnableOwner)
			EnableWindow(hOwner, TRUE);
		if (hParent)
			SetActiveWindow(hParent);
		PostModal();
		Destroy();
		return m_iResult;
	}

	EckInline HWND IntCreateModelessDlg(HINSTANCE hInst, PCWSTR pszTemplate, HWND hParent,
		LPARAM lParam = 0, FWndCreating pfnCreatingProc = nullptr)
	{
		m_bModal = FALSE;
		BeginCbtHook(this, pfnCreatingProc);
		const auto h = CreateDialogParamW(hInst, pszTemplate, hParent, EckDlgProc, lParam);
		EckAssert(m_bDlgProcInit);
		return h;
	}
public:
	ECK_RTTI(CDialog);

	static INT_PTR CALLBACK EckDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		const auto pDlg = dynamic_cast<CDialog*>(CWndFromHWND(hDlg));
		if (!pDlg)
		{
			EckDbgPrintWithPos(L"** WARNING **  CDialog指针为NULL");
			EckDbgBreak();
			return FALSE;
		}
		if (uMsg == WM_INITDIALOG)
			return pDlg->OnInitDialog(hDlg, (HWND)wParam, lParam);
		return FALSE;
	}

	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		switch (uMsg)
		{
		case WM_NCCREATE:
			SetWindowLongPtrW(hWnd, DWLP_DLGPROC, (LONG_PTR)EckDlgProc);
#ifdef _DEBUG
			m_bDlgProcInit = TRUE;
			break;
		case WM_NCDESTROY:
			m_bDlgProcInit = FALSE;
#endif // _DEBUG
			break;
		case WM_DESTROY:
			m_bModal = FALSE;
			m_iResult = 0;
			break;
		case WM_COMMAND:
		{
			if (HIWORD(wParam) == BN_CLICKED)
				switch (LOWORD(wParam))
				{
				case IDOK:
					OnOk((HWND)lParam);
					break;
				case IDCANCEL:
					OnCancel((HWND)lParam);
					break;
				}
		}
		break;
		case WM_CTLCOLORSTATIC:
			if ((!m_bClrDisableEdit && CWnd((HWND)lParam).GetClsName() == WC_EDITW))
				break;
			[[fallthrough]];
		case WM_CTLCOLORBTN:
		case WM_CTLCOLORDLG:
		case WM_CTLCOLOREDIT:
		case WM_CTLCOLORLISTBOX:
		{
			const auto* const ptc = GetThreadCtx();
			SetTextColor((HDC)wParam, ptc->crDefText);
			SetBkColor((HDC)wParam, m_crBkg != CLR_DEFAULT ? m_crBkg : ptc->crDefBkg);
			SetDCBrushColor((HDC)wParam, m_crBkg != CLR_DEFAULT ? m_crBkg : ptc->crDefBkg);
			return (LRESULT)GetStockBrush(DC_BRUSH);
		}
		break;
		}

		return CWnd::OnMsg(hWnd, uMsg, wParam, lParam);
	}

	EckInline virtual BOOL OnInitDialog(HWND hDlg, HWND hFocus, LPARAM lParam) { return TRUE; }

	EckInline virtual HWND CreateDlg(HWND hParent, void* pData = nullptr) 
	{ 
		EckDbgPrintWithPos(L"** ERROR ** 未实现方法");
		EckDbgBreak();
		abort();
	}

	EckInline virtual INT_PTR DlgBox(HWND hParent, void* pData = nullptr)
	{ 
		EckDbgPrintWithPos(L"** ERROR ** 未实现方法");
		EckDbgBreak();
		abort();
	}

	EckInline virtual BOOL EndDlg(INT_PTR nResult)
	{
		m_iResult = nResult;
		if (m_bModal)
		{
			PostQuitMessage(0);
			return TRUE;
		}
		else
			return Destroy();
	}

	EckInline virtual void OnOk(HWND hCtrl)
	{
		EndDlg(0);
	}

	EckInline virtual void OnCancel(HWND hCtrl)
	{
		EndDlg(0);
	}

	EckInline void SetBkClr(COLORREF cr)
	{
		m_crBkg = cr;
	}

	EckInline void SetAllowColorDisableEdit(BOOL b)
	{
		m_bClrDisableEdit = b;
	}

	EckInline void SetUseDefBkColor(BOOL b) { m_bUseDefBkClr = b; }
};
ECK_RTTI_IMPL_BASE_INLINE(CDialog, CWnd);

enum :UINT
{
	DLGNCF_CENTERSCREEN = (1u << 0),
	DLGNCF_CENTERPARENT = (1u << 1),
};

class CDialogNew :public CDialog
{
public:
	ECK_RTTI(CDialogNew);

	constexpr static LONG_PTR OcbPtr = DLGWINDOWEXTRA;
protected:
	INT_PTR IntCreateModalDlg(DWORD dwExStyle, PCWSTR pszClass, PCWSTR pszText, DWORD dwStyle,
		int x, int y, int cx, int cy, HWND hParent, HMENU hMenu, HINSTANCE hInst, void* pParam,
		UINT uDlgFlags = 0u, FWndCreating pfnCreatingProc = nullptr)
	{
		const HWND hOwner = PreModal(hParent);
		BOOL bNeedEnableOwner;
		if (hOwner && hOwner != GetDesktopWindow() && IsWindowEnabled(hOwner))
		{
			bNeedEnableOwner = TRUE;
			EnableWindow(hOwner, FALSE);
		}
		else
			bNeedEnableOwner = FALSE;

		IntCreateModelessDlg(dwExStyle, pszClass, pszText, dwStyle,
			x, y, cx, cy, hParent, hMenu, hInst, pParam, uDlgFlags, pfnCreatingProc);

		MsgLoop();
		if (bNeedEnableOwner)
			EnableWindow(hOwner, TRUE);
		if (hParent)
			SetActiveWindow(hParent);
		PostModal();
		Destroy();
		return m_iResult;
	}

	HWND IntCreateModelessDlg(DWORD dwExStyle, PCWSTR pszClass, PCWSTR pszText, DWORD dwStyle,
		int x, int y, int cx, int cy, HWND hParent, HMENU hMenu, HINSTANCE hInst, void* pParam,
		UINT uDlgFlags = 0u, FWndCreating pfnCreatingProc = nullptr)
	{
		POINT pt;
		if (IsBitSet(uDlgFlags, DLGNCF_CENTERPARENT))
			pt = CalcCenterWndPos(hParent, cx, cy);
		else if (IsBitSet(uDlgFlags, DLGNCF_CENTERSCREEN))
			pt = CalcCenterWndPos(nullptr, cx, cy);
		else
			pt = { x,y };

		IntCreate(dwExStyle, pszClass, pszText, dwStyle,
			pt.x, pt.y, cx, cy, hParent, hMenu, hInst, pParam, pfnCreatingProc);
		EckAssert(m_bDlgProcInit);

		if (m_hWnd)
		{
			HWND hFirstCtrl = GetNextDlgTabItem(m_hWnd, nullptr, FALSE);
			if (SendMsg(WM_INITDIALOG, (WPARAM)hFirstCtrl, (LPARAM)pParam))
			{
				hFirstCtrl = GetNextDlgTabItem(m_hWnd, nullptr, FALSE);
				SetFocus(hFirstCtrl);
			}
		}
		return m_hWnd;
	}
};
ECK_RTTI_IMPL_BASE_INLINE(CDialogNew, CDialog);
ECK_NAMESPACE_END