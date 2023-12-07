/*
* WinEzCtrlKit Library
*
* CWnd.h ： 窗口基类
*
* Copyright(C) 2023 QingKong
*/
#pragma once
#include "ECK.h"
#include "WndHelper.h"
#include "CRefStr.h"

#include <optional>

#include <assert.h>

ECK_NAMESPACE_BEGIN
inline constexpr int
DATAVER_STD_1 = 1;
#pragma pack(push, ECK_CTRLDATA_ALIGN)
struct CREATEDATA_STD
{
	int iVer_Std;
	int cchText;
	DWORD dwStyle;
	DWORD dwExStyle;
	// WCHAR szText[];

	EckInline PCWSTR Text() const
	{
		if (cchText)
			return (PCWSTR)PtrSkipType(this);
		else
			return NULL;
	}
};
#pragma pack(pop)

#ifdef ECK_CTRL_DESIGN_INTERFACE
struct DESIGNDATA_WND
{
	CRefStrW rsName;
	BITBOOL bVisible : 1;
	BITBOOL bEnable : 1;
};
#endif

class CWnd
{
protected:
	HWND m_hWnd = NULL;
	WNDPROC m_pfnRealProc = DefWindowProcW;

	EckInline HWND DefAttach(HWND hWnd)
	{
		HWND hOld = m_hWnd;
		m_hWnd = hWnd;
		return hOld;
	}

	static void WndCreatingSetLong(HWND hWnd, CBT_CREATEWNDW* pcs, ECKTREADCTX* pThreadCtx)
	{
		SetWindowLongPtrW(hWnd, 0, (LONG_PTR)pThreadCtx->pCurrWnd);
	}

	EckInline HWND IntCreate(DWORD dwExStyle, PCWSTR pszClass, PCWSTR pszText, DWORD dwStyle, 
		int x, int y, int cx, int cy, HWND hParent, HMENU hMenu, HINSTANCE hInst, void* pParam,
		FWndCreating pfnCreatingProc = NULL)
	{
		BeginCbtHook(this, pfnCreatingProc);
		auto hWnd = CreateWindowExW(dwExStyle, pszClass, pszText, dwStyle,
			x, y, cx, cy, hParent, hMenu, hInst, pParam);
		EndCbtHook();
		return hWnd;
	}

	static LRESULT WndProcNotifyReflection(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		auto p = CWndFromHWND(hWnd);
		EckAssert(p);

		CWnd* pChild;
		LRESULT lResult = 0;
		switch (uMsg)
		{
		case WM_NOTIFY:
			pChild = CWndFromHWND(((NMHDR*)lParam)->hwndFrom);
			if (pChild && pChild->OnNotifyMsg(hWnd, uMsg, wParam, lParam, lResult))
				return lResult;
			break;
		case WM_HSCROLL:
		case WM_VSCROLL:
		case WM_COMMAND:
		case WM_CHARTOITEM:
		case WM_VKEYTOITEM:
		case WM_CTLCOLORMSGBOX:
		case WM_CTLCOLOREDIT:
		case WM_CTLCOLORLISTBOX:
		case WM_CTLCOLORBTN:
		case WM_CTLCOLORDLG:
		case WM_CTLCOLORSCROLLBAR:
		case WM_CTLCOLORSTATIC:
			pChild = CWndFromHWND((HWND)lParam);
			if (pChild && pChild->OnNotifyMsg(hWnd, uMsg, wParam, lParam, lResult))
				return lResult;
			break;
		case WM_DRAWITEM:
			pChild = CWndFromHWND(((DRAWITEMSTRUCT*)lParam)->hwndItem);
			if (pChild && pChild->OnNotifyMsg(hWnd, uMsg, wParam, lParam, lResult))
				return lResult;
			break;
		case WM_MEASUREITEM:
			pChild = CWndFromHWND(GetDlgItem(hWnd, ((MEASUREITEMSTRUCT*)lParam)->CtlID));
			if (pChild && pChild->OnNotifyMsg(hWnd, uMsg, wParam, lParam, lResult))
				return lResult;
			break;
		case WM_DELETEITEM:
			pChild = CWndFromHWND(((DELETEITEMSTRUCT*)lParam)->hwndItem);
			if (pChild && pChild->OnNotifyMsg(hWnd, uMsg, wParam, lParam, lResult))
				return lResult;
			break;
		case WM_COMPAREITEM:
			pChild = CWndFromHWND(((COMPAREITEMSTRUCT*)lParam)->hwndItem);
			if (pChild && pChild->OnNotifyMsg(hWnd, uMsg, wParam, lParam, lResult))
				return lResult;
			break;
		}
		return CallWindowProcW(p->m_pfnRealProc, hWnd, uMsg, wParam, lParam);
	}
public:
#ifdef ECK_CTRL_DESIGN_INTERFACE
	DESIGNDATA_WND m_DDBase{};
#endif

	CWnd()
	{

	}

	CWnd(HWND hWnd) :m_hWnd(hWnd)
	{

	}

	virtual ~CWnd()
	{

	}

	virtual HWND Attach(HWND hWnd)
	{
		std::swap(m_hWnd, hWnd);
		return hWnd;
	}

	virtual HWND Detach()
	{
		auto t = m_hWnd;
		m_hWnd = NULL;
		return t;
	}

	virtual HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, int nID, PCVOID pData = NULL)
	{
		assert(FALSE);
		return NULL;
	}

	virtual void SerializeData(CRefBin& rb)
	{
		CRefStrW rsText = GetText();
		const SIZE_T cbSize = sizeof(CREATEDATA_STD) + rsText.ByteSize();
		CMemWriter w(rb.PushBack(cbSize), cbSize);
		CREATEDATA_STD* p;
		w.SkipPointer(p);
		p->iVer_Std = DATAVER_STD_1;
		p->cchText = rsText.Size();
		p->dwStyle = GetStyle();
		p->dwExStyle = GetExStyle();

		w << rsText;
	}

	virtual BOOL PreTranslateMessage(const MSG* pMsg)
	{
		return FALSE;
	}

	/// <summary>
	/// 父窗口通知类消息映射。
	/// 父窗口接收到的通知消息将路由到本方法，一般情况下无需手动调用本方法。
	/// 路由的消息有以下四种：自定义绘制系列（WM_XxxITEM）、
	/// 标准通知系列（WM_COMMAND、WM_NOTIFY）、着色系列（WM_CTLCOLORXxx）、
	/// 滚动条系列（WM_VSCROLL、WM_HSCROLL）
	/// </summary>
	/// <param name="hParent">父窗口句柄</param>
	/// <param name="uMsg">消息</param>
	/// <param name="wParam">wParam</param>
	/// <param name="lParam">lParam</param>
	/// <param name="lResult">消息返回值，调用本方法前保证其为0，仅当本方法返回TRUE时有效</param>
	/// <returns>若返回TRUE，则不再将当前消息交由父窗口处理</returns>
	EckInline virtual BOOL OnNotifyMsg(HWND hParent, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult)
	{
		return FALSE;
	}

	EckInline static PCVOID SkipBaseData(PCVOID p)
	{
		return (PCBYTE)p +
			sizeof(CREATEDATA_STD) +
			(((const CREATEDATA_STD*)p)->cchText + 1) * sizeof(WCHAR);
	}

	HWND ReCreate(EckOptNul(DWORD, dwNewStyle), EckOptNul(DWORD, dwNewExStyle), EckOptNul(RECT, rcPos));

	/// <summary>
	/// 启用通知类消息路由
	/// </summary>
	EckInline WNDPROC EnableNotifyReflection()
	{
		auto pfnRealProc = (WNDPROC)SetLong(GWLP_WNDPROC, (LONG_PTR)WndProcNotifyReflection);
		std::swap(pfnRealProc, m_pfnRealProc);
		return pfnRealProc;
	}

	EckInline HWND GetHWND() const { return m_hWnd; }

	EckInline void FrameChanged() const
	{
		SetWindowPos(m_hWnd, NULL, 0, 0, 0, 0,
			SWP_NOZORDER | SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE | SWP_FRAMECHANGED);
	}

	EckInline void SetRedraw(BOOL bRedraw) const
	{
		SendMsg(WM_SETREDRAW, bRedraw, 0);
	}

	EckInline BOOL Redraw() const
	{
		return InvalidateRect(m_hWnd, NULL, FALSE);
	}

	EckInline BOOL Redraw(const RECT& rc) const
	{
		return InvalidateRect(m_hWnd, &rc, FALSE);
	}

	EckInline operator HWND() const
	{
		return m_hWnd;
	}

	/// <summary>
	/// 置边框类型
	/// </summary>
	/// <param name="iFrame">0 - 无边框  1 - 凹入式  2 - 凸出式  3 - 浅凹入式  4 - 镜框式  5 - 单线边框式</param>
	void SetFrameType(int iFrame) const;

	int GetFrameType() const;

	void SetScrollBar(int i) const;

	int GetScrollBar() const;

	EckInline LRESULT SendMsg(UINT uMsg, WPARAM wParam, LPARAM lParam) const
	{
		return SendMessageW(m_hWnd, uMsg, wParam, lParam);
	}

	EckInline DWORD GetStyle() const
	{
		return (DWORD)GetWindowLongPtrW(m_hWnd, GWL_STYLE);
	}

	EckInline DWORD GetExStyle() const
	{
		return (DWORD)GetWindowLongPtrW(m_hWnd, GWL_EXSTYLE);
	}

	EckInline DWORD ModifyStyle(DWORD dwNew, DWORD dwMask, int idx = GWL_STYLE) const
	{
		return ModifyWindowStyle(m_hWnd, dwNew, dwMask, idx);
	}

	EckInline DWORD SetStyle(DWORD dwStyle) const
	{
		return (DWORD)SetWindowLongPtrW(m_hWnd, GWL_STYLE, dwStyle);
	}

	EckInline DWORD SetExStyle(DWORD dwStyle) const
	{
		return (DWORD)SetWindowLongPtrW(m_hWnd, GWL_EXSTYLE, dwStyle);
	}

	EckInline CRefStrW GetText() const
	{
		CRefStrW rs;
		int cch = GetWindowTextLengthW(m_hWnd);
		if (cch)
		{
			rs.ReSize(cch);
			GetWindowTextW(m_hWnd, rs.Data(), cch + 1);
		}
		return rs;
	}

	EckInline int GetText(PWSTR pszBuf, int cchBuf) const
	{
		GetWindowTextW(m_hWnd, pszBuf, cchBuf);
	}

	EckInline BOOL SetText(PCWSTR pszText) const
	{
		return SetWindowTextW(m_hWnd, pszText);
	}

	EckInline HRESULT SetExplorerTheme() const
	{
		return SetWindowTheme(m_hWnd, L"Explorer", NULL);
	}

	EckInline BOOL Move(int x, int y, int cx, int cy, BOOL bNoActive = FALSE) const
	{
		return SetWindowPos(m_hWnd, NULL, x, y, cx, cy, SWP_NOZORDER | (bNoActive ? SWP_NOACTIVATE : 0));
	}

	EckInline BOOL Destroy()
	{
		BOOL b = DestroyWindow(m_hWnd);
		m_hWnd = NULL;
		return b;
	}

	EckInline void SetFont(HFONT hFont, BOOL bRedraw = FALSE) const
	{
		SendMsg(WM_SETFONT, (WPARAM)hFont, bRedraw);
	}

	EckInline BOOL Show(int nCmdShow) const
	{
		return ShowWindow(m_hWnd, nCmdShow);
	}

	EckInline BOOL Enable(BOOL bEnable) const
	{
		return EnableWindow(m_hWnd, bEnable);
	}

	EckInline BOOL IsVisible() const
	{
		return IsWindowVisible(m_hWnd);
	}

	EckInline LONG_PTR GetLong(int i) const
	{
		return GetWindowLongPtrW(m_hWnd, i);
	}

	EckInline LONG_PTR SetLong(int i, LONG_PTR l) const
	{
		return SetWindowLongPtrW(m_hWnd, i, l);
	}
};

class COwnWnd :public CWnd
{
protected:
	HFONT m_hFont = NULL;
	CRefStrW m_rsText{};

	EckInline void OnOwnWndMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg)
		{
		case WM_CREATE:
		{
			auto pcs = (CREATESTRUCTW*)lParam;
			m_rsText = pcs->lpszName;
		}
		break;
		case WM_SETFONT:
			m_hFont = (HFONT)wParam;
			break;
		case WM_SETTEXT:
			m_rsText = (PWSTR)lParam;
			break;
		case WM_GETTEXT:
			if (wParam > 0)
				m_rsText.CopyTo((PWSTR)lParam, (int)wParam - 1);
			break;
		}
	}
};
ECK_NAMESPACE_END