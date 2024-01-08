/*
* WinEzCtrlKit Library
*
* CDialog.h ： 对话框
*
* Copyright(C) 2023 QingKong
*/
#pragma once
#include "CWnd.h"
#include "DlgHelper.h"

ECK_NAMESPACE_BEGIN
class CDialog :public CWnd
{
protected:
#ifdef _DEBUG
	BOOL m_bDlgProcInit = FALSE;
#endif
	BOOL m_bModal = FALSE;

	EckInline INT_PTR IntCreateModalDlg(HINSTANCE hInst, PCVOID pDlgTemplate, HWND hParent,
		LPARAM lParam = 0, FWndCreating pfnCreatingProc = NULL)
	{
		m_bModal = TRUE;
		BeginCbtHook(this, pfnCreatingProc);
		return DialogBoxIndirectParamW(hInst, (const DLGTEMPLATE*)pDlgTemplate,
			hParent, EckDlgProc, lParam);
	}

	EckInline HWND IntCreateModelessDlg(HINSTANCE hInst, PCVOID pDlgTemplate, HWND hParent,
		LPARAM lParam = 0, FWndCreating pfnCreatingProc = NULL)
	{
		m_bModal = FALSE;
		BeginCbtHook(this, pfnCreatingProc);
		const auto h = CreateDialogIndirectParamW(hInst, (const DLGTEMPLATE*)pDlgTemplate,
			hParent, EckDlgProc, lParam);
		EckAssert(m_bDlgProcInit);
		return h;
	}
public:
	static INT_PTR CALLBACK EckDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		const auto pDlg = dynamic_cast<CDialog*>(CWndFromHWND(hDlg));
		if (!pDlg)
		{
			EckDbgPrintWithPos(L"** 警告 **  CDialog指针为NULL");
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
		case WM_COMMAND:
			if (HIWORD(wParam) == BN_CLICKED && lParam)
				switch (LOWORD(wParam))
				{
				case IDOK:
					OnOk((HWND)lParam);
					break;
				case IDCANCEL:
					OnCancel((HWND)lParam);
					break;
				}
			break;
		}

		return CWnd::OnMsg(hWnd, uMsg, wParam, lParam);
	}

	EckInline virtual BOOL OnInitDialog(HWND hDlg, HWND hFocus, LPARAM lParam) { return TRUE; }

	EckInline virtual HWND CreateDlg(HWND hParent, void* pData = NULL) { throw CCreateDlgOnPureCDialogException{}; }

	EckInline virtual INT_PTR DlgBox(HWND hParent, void* pData = NULL) { throw CDlgBoxOnPureCDialogException{}; }

	EckInline virtual BOOL EndDlg(INT_PTR nResult) 
	{ 
		if (m_bModal)
			return EndDialog(m_hWnd, nResult);
		else
			return Destroy();
	}

	EckInline virtual void OnOk(HWND hCtrl) {}

	EckInline virtual void OnCancel(HWND hCtrl) {}
};

enum :UINT
{
	DLGNCF_CENTERSCREEN = (1u << 0),
	DLGNCF_CENTERPARENT = (1u << 1),
};

class CDialogNew :public CDialog
{
public:
	constexpr static LONG_PTR OcbPtr1 = DLGWINDOWEXTRA;
	constexpr static LONG_PTR OcbPtr2 = DLGWINDOWEXTRA + sizeof(void*);
protected:
	HWND m_hTop = NULL;
	INT_PTR m_iResult = 0;

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
		m_hTop = NULL;
	}

	void MsgLoop()
	{
		MSG msg;
		while (GetMessageW(&msg, NULL, 0, 0))
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

	INT_PTR IntCreateModalDlg(DWORD dwExStyle, PCWSTR pszClass, PCWSTR pszText, DWORD dwStyle,
		int x, int y, int cx, int cy, HWND hParent, HMENU hMenu, HINSTANCE hInst, void* pParam,
		UINT uDlgFlags = 0u, FWndCreating pfnCreatingProc = NULL)
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

		POINT pt;
		if (IsBitSet(uDlgFlags, DLGNCF_CENTERPARENT))
			pt = CalcCenterWndPos(hParent, cx, cy);
		else if (IsBitSet(uDlgFlags, DLGNCF_CENTERSCREEN))
			pt = CalcCenterWndPos(NULL, cx, cy);
		else
			pt = { x,y };

		IntCreate(dwExStyle, pszClass, pszText, dwStyle,
			pt.x, pt.y, cx, cy, hOwner, hMenu, hInst, pParam, pfnCreatingProc);
		EckAssert(m_bDlgProcInit);
		const HWND hFirstCtrl = GetFirstTabStopCtrl(m_hWnd);
		if (SendMsg(WM_INITDIALOG, (WPARAM)hFirstCtrl, (LPARAM)pParam))
			SetFocus(hFirstCtrl);
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
		UINT uDlgFlags = 0u, FWndCreating pfnCreatingProc = NULL)
	{
		POINT pt;
		if (IsBitSet(uDlgFlags, DLGNCF_CENTERPARENT))
			pt = CalcCenterWndPos(hParent, cx, cy);
		else if (IsBitSet(uDlgFlags, DLGNCF_CENTERSCREEN))
			pt = CalcCenterWndPos(NULL, cx, cy);
		else
			pt = { x,y };

		IntCreate(dwExStyle, pszClass, pszText, dwStyle,
			pt.x, pt.y, cx, cy, hParent, hMenu, hInst, pParam, pfnCreatingProc);
		EckAssert(m_bDlgProcInit);
		const HWND hFirstCtrl = GetFirstTabStopCtrl(m_hWnd);
		if (SendMsg(WM_INITDIALOG, (WPARAM)hFirstCtrl, (LPARAM)pParam))
			SetFocus(hFirstCtrl);
		return m_hWnd;
	}
public:
	EckInline BOOL EndDlg(INT_PTR nResult) override
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
};
ECK_NAMESPACE_END