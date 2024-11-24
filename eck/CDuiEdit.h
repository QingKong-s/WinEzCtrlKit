/*
* WinEzCtrlKit Library
*
* CDuiEdit.h ： DUI编辑框
*
* Copyright(C) 2024 QingKong
*/
#pragma once
#include "CDuiScrollBar.h"
#include "TextSrvDef.h"

#if !ECKCXX20
#error "EckDui requires C++20"
#endif// !ECKCXX20

ECK_NAMESPACE_BEGIN
ECK_DUI_NAMESPACE_BEGIN
class CEdit;
class CEditTextHost :public CUnknownMultiThread<CEditTextHost, ITextHost2>
{
	DECL_CUNK_FRIENDS;
private:
	ULONG m_cRef{ 1 };
	BITBOOL m_bActive : 1{};
	CEdit* m_pEdit{};
public:
	CEditTextHost(CEdit* pEdit) :m_pEdit{ pEdit } {}

	STDMETHODIMP QueryInterface(REFIID riid, void** ppvObject) override
	{
		const static QITAB qit[]
		{
			{ g_pIID_ITextHost, OFFSETOFCLASS(CEditTextHost, ITextHost2) },
			{ g_pIID_ITextHost2, OFFSETOFCLASS(CEditTextHost, ITextHost2) },
			{}
		};
		return QISearch(this, qit, riid, ppvObject);
	}

	HDC TxGetDC();
	INT TxReleaseDC(HDC hdc);
	BOOL TxShowScrollBar(INT fnBar, BOOL fShow);
	BOOL TxEnableScrollBar(INT fuSBFlags, INT fuArrowflags);
	BOOL TxSetScrollRange(INT fnBar, LONG nMinPos, INT nMaxPos, BOOL fRedraw);
	BOOL TxSetScrollPos(INT fnBar, INT nPos, BOOL fRedraw);
	void TxInvalidateRect(LPCRECT prc, BOOL fMode);
	void TxViewChange(BOOL fUpdate);
	BOOL TxCreateCaret(HBITMAP hbmp, INT xWidth, INT yHeight);
	BOOL TxShowCaret(BOOL fShow);
	BOOL TxSetCaretPos(INT x, INT y);
	BOOL TxSetTimer(UINT idTimer, UINT uTimeout);
	void TxKillTimer(UINT idTimer);
	void TxScrollWindowEx(INT dx, INT dy, LPCRECT lprcScroll, LPCRECT lprcClip,
		HRGN hrgnUpdate, LPRECT lprcUpdate, UINT fuScroll);
	void TxSetCapture(BOOL fCapture);
	void TxSetFocus();
	void TxSetCursor(HCURSOR hcur, BOOL fText);
	BOOL TxScreenToClient(LPPOINT lppt);
	BOOL TxClientToScreen(LPPOINT lppt);
	HRESULT TxActivate(LONG* plOldState);
	HRESULT TxDeactivate(LONG lNewState);
	HRESULT TxGetClientRect(LPRECT prc);
	HRESULT TxGetViewInset(LPRECT prc);
	HRESULT TxGetCharFormat(const CHARFORMATW** ppCF);
	HRESULT TxGetParaFormat(const PARAFORMAT** ppPF);
	COLORREF TxGetSysColor(int nIndex);
	HRESULT TxGetBackStyle(TXTBACKSTYLE* pstyle);
	HRESULT TxGetMaxLength(DWORD* plength);
	HRESULT TxGetScrollBars(DWORD* pdwScrollBar);
	HRESULT TxGetPasswordChar(_Out_ TCHAR* pch);
	HRESULT TxGetAcceleratorPos(LONG* pcp);
	HRESULT TxGetExtent(LPSIZEL lpExtent);
	HRESULT OnTxCharFormatChange(const CHARFORMATW* pCF);
	HRESULT OnTxParaFormatChange(const PARAFORMAT* pPF);
	HRESULT TxGetPropertyBits(DWORD dwMask, DWORD* pdwBits);
	HRESULT TxNotify(DWORD iNotify, void* pv);
	HIMC TxImmGetContext();
	void TxImmReleaseContext(HIMC himc);
	HRESULT TxGetSelectionBarWidth(LONG* lSelBarWidth);

	BOOL TxIsDoubleClickPending();
	HRESULT TxGetWindow(HWND* phwnd);
	HRESULT TxSetForegroundWindow();
	HPALETTE TxGetPalette();
	HRESULT TxGetEastAsianFlags(LONG* pFlags);
	HCURSOR TxSetCursor2(HCURSOR hcur, BOOL bText);
	void TxFreeTextServicesNotification();
	HRESULT TxGetEditStyle(DWORD dwItem, DWORD* pdwData);
	HRESULT TxGetWindowStyles(DWORD* pdwStyle, DWORD* pdwExStyle);
	HRESULT TxShowDropCaret(BOOL fShow, HDC hdc, LPCRECT prc);
	HRESULT TxDestroyCaret();
	HRESULT TxGetHorzExtent(LONG* plHorzExtent);
};

class CEdit :public CElem
{
	friend class CEditTextHost;
private:
	CScrollBar m_SBH{};
	CScrollBar m_SBV{};

	CHARFORMATW m_DefCharFormat{ .cbSize = sizeof(CHARFORMATW) };
	PARAFORMAT m_DefParaFormat{ .cbSize = sizeof(PARAFORMAT) };
	WCHAR m_chPassword{ L'*' };
	RECT m_mgTextAera{};

	CEditTextHost* m_pHost{};
	ITextServices2* m_pSrv{};
public:
	LRESULT OnEvent(UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		switch (uMsg)
		{
		case WM_PAINT:
		{
			ELEMPAINTSTRU ps;
			BeginPaint(ps, wParam, lParam);

			RECT rc{ GetViewRect() };
			RECT rcClip{ MakeRect(ps.rcfClipInElem) };
			DpiScale(rc, GetWnd()->GetDpiValue());
			DpiScale(rcClip, GetWnd()->GetDpiValue());
			m_pSrv->TxDrawD2D(m_pDC, (RECTL*)&rc, &rcClip, TXTVIEW_ACTIVE);

			ECK_DUI_DBG_DRAW_FRAME;
			EndPaint(ps);
		}
		return 0;

		case WM_SETFOCUS:
			m_pSrv->OnTxUIActivate();
			m_pSrv->TxSendMessage(WM_SETFOCUS, 0, 0, nullptr);
			return 0;

		case WM_KILLFOCUS:
			m_pSrv->OnTxUIDeactivate();
			m_pSrv->TxSendMessage(WM_KILLFOCUS, 0, 0, nullptr);
			return 0;

		case WM_SETCURSOR:
			//m_pSrv->On
			//SetCursor(LoadCursorW(nullptr, IDC_IBEAM));
			//return TRUE;
			break;

		case WM_SETTEXT:
		{

		}
		return 0;

		case WM_CREATE:
		{
			m_SBV.Create(nullptr, 0, 0, 0, 0, 0, 0, this, GetWnd());
			m_SBV.GetScrollView()->SetCallBack([](int iPos, int iPrevPos, LPARAM lParam)
				{
					((CEdit*)lParam)->InvalidateRect();
				}, (LPARAM)this);
			m_SBH.Create(nullptr, 0, 0, 0, 0, 0, 0, this, GetWnd());
			m_SBH.GetScrollView()->SetCallBack([](int iPos, int iPrevPos, LPARAM lParam)
				{
					((CEdit*)lParam)->InvalidateRect();
				}, (LPARAM)this);
			m_pSrv->TxSendMessage(EM_SETTYPOGRAPHYOPTIONS,
				TO_DEFAULTCOLOREMOJI | TO_DISPLAYFONTCOLOR,
				TO_DEFAULTCOLOREMOJI | TO_DISPLAYFONTCOLOR, nullptr);
			m_pSrv->TxSetText(m_rsText.Data());
			auto rc{ GetViewRect() };
			GetWnd()->Log2Phy(rc);
			m_pSrv->OnTxInPlaceActivate(0);
			m_pSrv->OnTxUIActivate();
		}
		break;

		case WM_DESTROY:
		{
			m_pSrv->OnTxInPlaceDeactivate();
			TsiShutdownTextServices(m_pSrv);
			m_pSrv->Release();
			m_pHost->Release();
		}
		break;

		case WM_NCCREATE:
		{
			m_DefCharFormat.yHeight = 22 * 2540 / 96;
			m_DefCharFormat.dwMask = CFM_FACE | CFM_SIZE;
			m_DefCharFormat.dwEffects = CFE_AUTOCOLOR;
			EckCopyConstStringW(m_DefCharFormat.szFaceName, L"微软雅黑");

			if (TsiGetState() != TsiState::Ver41)
				TsiInit41();
			m_pHost = new CEditTextHost{ this };
			IUnknown* pUnk{};
			auto hr = TsiCreateTextServices(nullptr, m_pHost, &pUnk);
			pUnk->QueryInterface(*g_pIID_ITextServices2, (void**)&m_pSrv);
			pUnk->Release();
		}
		break;
		}

		if (uMsg >= WM_MOUSEFIRST && uMsg <= WM_MOUSELAST)
		{
			POINT pt ECK_GET_PT_LPARAM(lParam);
			ClientToElem(pt);
			//GetWnd()->Log2Phy(pt);
			DpiScale(pt, GetWnd()->GetDpiValue());
			LRESULT lResult;
			if (m_pSrv->TxSendMessage(uMsg,
				wParam, POINTTOPOINTS(pt), &lResult) == S_OK)
				return lResult;
		}
		else if (uMsg >= WM_KEYFIRST && uMsg <= WM_IME_KEYLAST)
		{
			LRESULT lResult;
			if (m_pSrv->TxSendMessage(uMsg, wParam, lParam, &lResult) == S_OK)
				return lResult;
		}
		return CElem::OnEvent(uMsg, wParam, lParam);
	}

	void ShowScrollBar(int nBar, BOOL bShow)
	{
		if (nBar == SB_BOTH)
		{
			m_SBV.SetVisible(bShow);
			m_SBH.SetVisible(bShow);
		}
		else if (nBar == SB_HORZ)
			m_SBH.SetVisible(bShow);
		else if (nBar == SB_VERT)
			m_SBV.SetVisible(bShow);
	}
};

inline HDC CEditTextHost::TxGetDC()
{
	return GetDC(m_pEdit->GetWnd()->HWnd);
}

inline INT CEditTextHost::TxReleaseDC(HDC hdc)
{
	return ReleaseDC(m_pEdit->GetWnd()->HWnd, hdc);
}

inline BOOL CEditTextHost::TxShowScrollBar(INT fnBar, BOOL fShow)
{
	m_pEdit->ShowScrollBar(fnBar, fShow);
	return TRUE;
}

inline BOOL CEditTextHost::TxEnableScrollBar(INT fuSBFlags, INT fuArrowflags)
{
	// TODO: implement scroll bar
	return 0;
}

inline BOOL CEditTextHost::TxSetScrollRange(INT fnBar, LONG nMinPos, INT nMaxPos, BOOL fRedraw)
{
	if (fnBar == SB_VERT || fnBar == SB_BOTH)
	{
		m_pEdit->m_SBV.GetScrollView()->SetRange(nMinPos, nMaxPos);
		if (fRedraw)
			m_pEdit->m_SBV.InvalidateRect();
	}
	if (fnBar == SB_HORZ || fnBar == SB_BOTH)
	{
		m_pEdit->m_SBH.GetScrollView()->SetRange(nMinPos, nMaxPos);
		if (fRedraw)
			m_pEdit->m_SBH.InvalidateRect();
	}
	return TRUE;
}

inline BOOL CEditTextHost::TxSetScrollPos(INT fnBar, INT nPos, BOOL fRedraw)
{
	if (fnBar == SB_VERT || fnBar == SB_BOTH)
	{
		m_pEdit->m_SBV.GetScrollView()->SetPos(nPos);
		if (fRedraw)
			m_pEdit->m_SBV.InvalidateRect();
	}
	if (fnBar == SB_HORZ || fnBar == SB_BOTH)
	{
		m_pEdit->m_SBH.GetScrollView()->SetPos(nPos);
		if (fRedraw)
			m_pEdit->m_SBH.InvalidateRect();
	}
	return TRUE;
}

inline void CEditTextHost::TxInvalidateRect(LPCRECT prc, BOOL fMode)
{
	//if (prc)
	//{
	//	RECT rc{ *prc };
	//	m_pEdit->GetWnd()->Phy2Log(rc);
	//	m_pEdit->ElemToClient(rc);
	//	m_pEdit->InvalidateRect(*prc);
	//}
	//else
	m_pEdit->InvalidateRect();
}

inline void CEditTextHost::TxViewChange(BOOL fUpdate) {}

inline BOOL CEditTextHost::TxCreateCaret(HBITMAP hbmp, INT xWidth, INT yHeight)
{
	// TODO: implement caret
	return 0;
}

inline BOOL CEditTextHost::TxShowCaret(BOOL fShow)
{
	return 0;
}

inline BOOL CEditTextHost::TxSetCaretPos(INT x, INT y)
{
	return 0;
}

inline BOOL CEditTextHost::TxSetTimer(UINT idTimer, UINT uTimeout)
{
	// TODO: implement timer
	return 0;
}

inline void CEditTextHost::TxKillTimer(UINT idTimer)
{
}

inline void CEditTextHost::TxScrollWindowEx(INT dx, INT dy, LPCRECT lprcScroll, LPCRECT lprcClip, HRGN hrgnUpdate, LPRECT lprcUpdate, UINT fuScroll)
{
}

inline void CEditTextHost::TxSetCapture(BOOL fCapture)
{
	if (fCapture)
		m_pEdit->SetCapture();
	else
		m_pEdit->ReleaseCapture();
}

inline void CEditTextHost::TxSetFocus()
{
	m_pEdit->SetFocus();
}

inline void CEditTextHost::TxSetCursor(HCURSOR hcur, BOOL fText)
{
	SetCursor(hcur);
}

inline BOOL CEditTextHost::TxScreenToClient(LPPOINT lppt)
{
	ScreenToClient(m_pEdit->GetWnd()->HWnd, lppt);
	m_pEdit->ClientToElem(*lppt);
	return TRUE;
}

inline BOOL CEditTextHost::TxClientToScreen(LPPOINT lppt)
{
	m_pEdit->ElemToClient(*lppt);
	ClientToScreen(m_pEdit->GetWnd()->HWnd, lppt);
	return TRUE;
}

inline HRESULT CEditTextHost::TxActivate(LONG* plOldState)
{
	*plOldState = m_bActive;
	m_bActive = TRUE;
	return S_OK;
}

inline HRESULT CEditTextHost::TxDeactivate(LONG lNewState)
{
	m_bActive = lNewState;
	return S_OK;
}

inline HRESULT CEditTextHost::TxGetClientRect(LPRECT prc)
{
	*prc = m_pEdit->GetViewRect();
	DpiScale(*prc, m_pEdit->GetWnd()->GetDpiValue());
	return S_OK;
}

inline HRESULT CEditTextHost::TxGetViewInset(LPRECT prc)
{
	*prc = m_pEdit->m_mgTextAera;
	return S_OK;
}

inline HRESULT CEditTextHost::TxGetCharFormat(const CHARFORMATW** ppCF)
{
	*ppCF = &m_pEdit->m_DefCharFormat;
	return S_OK;
}

inline HRESULT CEditTextHost::TxGetParaFormat(const PARAFORMAT** ppPF)
{
	*ppPF = &m_pEdit->m_DefParaFormat;
	return S_OK;
}

inline COLORREF CEditTextHost::TxGetSysColor(int nIndex)
{
	D2D1_COLOR_F cr;
	switch (nIndex)
	{
	case COLOR_WINDOWTEXT:
		m_pEdit->GetTheme()->GetSysColor(SysColor::Text, cr);
		return D2dColorFToColorref(cr);
	case COLOR_WINDOW:
		m_pEdit->GetTheme()->GetSysColor(SysColor::Bk, cr);
		return D2dColorFToColorref(cr);
	default:
		return GetSysColor(nIndex);
	}
}

inline HRESULT CEditTextHost::TxGetBackStyle(TXTBACKSTYLE* pstyle)
{
	*pstyle = TXTBACK_TRANSPARENT;
	return S_OK;
}

inline HRESULT CEditTextHost::TxGetMaxLength(DWORD* plength)
{
	*plength = INFINITE;
	return S_OK;
}

inline HRESULT CEditTextHost::TxGetScrollBars(DWORD* pdwScrollBar)
{
	*pdwScrollBar = WS_VSCROLL | WS_HSCROLL | ES_AUTOVSCROLL | ES_AUTOHSCROLL;
	return S_OK;
}

inline HRESULT CEditTextHost::TxGetPasswordChar(_Out_ TCHAR* pch)
{
	*pch = m_pEdit->m_chPassword;
	return S_OK;
}

inline HRESULT CEditTextHost::TxGetAcceleratorPos(LONG* pcp)
{
	*pcp = -1;
	return S_OK;
}

inline HRESULT CEditTextHost::TxGetExtent(LPSIZEL lpExtent)
{
	lpExtent->cx = m_pEdit->GetWidth() * 2540 / 96;
	lpExtent->cy = m_pEdit->GetHeight() * 2540 / 96;
	DpiScale(*lpExtent, m_pEdit->GetWnd()->GetDpiValue());
	return S_OK;
}

inline HRESULT CEditTextHost::OnTxCharFormatChange(const CHARFORMATW* pCF)
{
	return E_NOTIMPL;
}

inline HRESULT CEditTextHost::OnTxParaFormatChange(const PARAFORMAT* pPF)
{
	return E_NOTIMPL;
}

inline HRESULT CEditTextHost::TxGetPropertyBits(DWORD dwMask, DWORD* pdwBits)
{
	*pdwBits = TXTBIT_D2DDWRITE | TXTBIT_RICHTEXT | TXTBIT_MULTILINE;
	return S_OK;
}

inline HRESULT CEditTextHost::TxNotify(DWORD iNotify, void* pv)
{
	return E_NOTIMPL;
}

inline HIMC CEditTextHost::TxImmGetContext()
{
	return ImmGetContext(m_pEdit->GetWnd()->HWnd);
}

inline void CEditTextHost::TxImmReleaseContext(HIMC himc)
{
	ImmReleaseContext(m_pEdit->GetWnd()->HWnd, himc);
}

inline HRESULT CEditTextHost::TxGetSelectionBarWidth(LONG* lSelBarWidth)
{
	*lSelBarWidth = 0;
	return S_OK;
}

inline BOOL CEditTextHost::TxIsDoubleClickPending()
{
	MSG msg;
	return PeekMessageA(&msg, m_pEdit->GetWnd()->HWnd,
		WM_LBUTTONDBLCLK, WM_LBUTTONDBLCLK, PM_NOREMOVE | PM_NOYIELD);
}

inline HRESULT CEditTextHost::TxGetWindow(HWND* phwnd)
{
	*phwnd = m_pEdit->GetWnd()->HWnd;
	return S_OK;
}

inline HRESULT CEditTextHost::TxSetForegroundWindow()
{
	SetForegroundWindow(m_pEdit->GetWnd()->HWnd);
	return S_OK;
}

inline HPALETTE CEditTextHost::TxGetPalette()
{
	return nullptr;
}

inline HRESULT CEditTextHost::TxGetEastAsianFlags(LONG* pFlags)
{
	*pFlags = 0;
	return S_OK;
}

inline HCURSOR CEditTextHost::TxSetCursor2(HCURSOR hcur, BOOL bText)
{
	return SetCursor(hcur);
}

inline void CEditTextHost::TxFreeTextServicesNotification()
{
}

inline HRESULT CEditTextHost::TxGetEditStyle(DWORD dwItem, DWORD* pdwData)
{
	*pdwData = 0;
	return S_OK;
}

inline HRESULT CEditTextHost::TxGetWindowStyles(DWORD* pdwStyle, DWORD* pdwExStyle)
{
	*pdwStyle = m_pEdit->GetWnd()->GetStyle();
	*pdwExStyle = m_pEdit->GetWnd()->GetExStyle();
	return S_OK;
}

inline HRESULT CEditTextHost::TxShowDropCaret(BOOL fShow, HDC hdc, LPCRECT prc)
{
	if (fShow)
		ShowCaret(m_pEdit->GetWnd()->HWnd);
	else
		HideCaret(m_pEdit->GetWnd()->HWnd);
	return S_OK;
}

inline HRESULT CEditTextHost::TxDestroyCaret()
{
	DestroyCaret();
	return S_OK;
}

inline HRESULT CEditTextHost::TxGetHorzExtent(LONG* plHorzExtent)
{
	return E_NOTIMPL;
}
ECK_DUI_NAMESPACE_END
ECK_NAMESPACE_END