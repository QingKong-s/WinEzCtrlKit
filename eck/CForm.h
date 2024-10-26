/*
* WinEzCtrlKit Library
*
* CForm.h ： 通用窗体
*
* Copyright(C) 2023 QingKong
*/
#pragma once
#include "CWnd.h"
#include "GraphicsHelper.h"
#include "CMenu.h"

ECK_NAMESPACE_BEGIN
class CForm :public CWnd
{
public:
	ECK_RTTI(CForm);
	ECK_CWND_NOSINGLEOWNER(CForm);
	ECK_CWND_CREATE_CLS_HINST(WCN_FORM, g_hInstance);
private:
	struct TRAY
	{
		UINT uID;
		UINT uFlags;
		DWORD dwState;
		HICON hIcon;
		CRefStrW rsTip;
#if !ECKCXX20
		TRAY(UINT uID, UINT uFlags, DWORD dwState, HICON hIcon, PCWSTR pszTip) :
			uID(uID), uFlags(uFlags), dwState(dwState), hIcon(hIcon), rsTip(pszTip) {
		}
#endif
	};

	HBITMAP m_hbmBk = nullptr;
	int m_cxImage = 0,
		m_cyImage = 0;
	BkImgMode m_iBkImageMode = BkImgMode::TopLeft;
	COLORREF m_crBk = CLR_DEFAULT;

	int m_cxClient = 0,
		m_cyClient = 0;

	std::vector<TRAY> m_Tray{};

#if ECKCXX20
	BITBOOL m_bMoveable : 1 = TRUE;
	BITBOOL m_bFullWndImage : 1 = FALSE;
	BITBOOL m_bEscClose : 1 = FALSE;
	BITBOOL m_bTotalMove : 1 = FALSE;
	BITBOOL m_bClrDisableEdit : 1 = FALSE;
#else
	union
	{
		struct
		{
			BITBOOL m_bMoveable : 1;
			BITBOOL m_bFullWndImage : 1;
			BITBOOL m_bEscClose : 1;
			BITBOOL m_bTotalMove : 1;
			BITBOOL m_bClrDisableEdit : 1;
		};
		UINT m_byFlags = 0b1;
	};
#endif
public:
	static UINT s_uTrayMsg;
	static UINT s_uTaskbarCreatedMsg;
public:
	ECKPROP(GetBkImage, SetBkImage)				HBITMAP		BkImage;		// 背景图片
	ECKPROP(GetBkImageMode, SetBkImageMode)		BkImgMode	BkImageMode;	// 背景图片模式
	ECKPROP(GetFullWndImage, SetFullWndImage)	BOOL		FullWndImage;	// 全窗口绘制背景图片
	ECKPROP(GetMoveable, SetMoveable)			BOOL		Moveable;		// 可否移动
	ECKPROP(GetEscClose, SetEscClose)			BOOL		EscClose;		// ESC关闭
	ECKPROP(GetTotalMove, SetTotalMove)			BOOL		TotalMove;		// 随意移动
	ECKPROP(GetBkClr, SetBkClr)					COLORREF	BkColor;		// 背景颜色
	ECKPROP_R(GetBkImageSize)					SIZE		BkImageSize;	// 背景图片大小

	BOOL PreTranslateMessage(const MSG& Msg) override
	{
		switch (Msg.message)
		{
		case WM_KEYDOWN:
		{
			if (m_bEscClose && Msg.wParam == VK_ESCAPE && IsWindowEnabled(GetHWND()))
			{
				PostMessageW(GetHWND(), WM_CLOSE, 0, 0);
				return TRUE;
			}
		}
		break;
		case WM_LBUTTONDOWN:
		{
			if (m_bTotalMove && IsWindowEnabled(GetHWND()))
				if ((Msg.hwnd == GetHWND() ||
					(SendMessageW(Msg.hwnd, WM_GETDLGCODE, Msg.wParam, (LPARAM)&Msg) & DLGC_STATIC)))
				{
					PostMessageW(GetHWND(), WM_NCLBUTTONDOWN, HTCAPTION, 0);
					return TRUE;
				}
		}
		break;
		}
		return CWnd::PreTranslateMessage(Msg);
	}

	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		switch (uMsg)
		{
		case WM_CTLCOLORSTATIC:
			if (!m_bClrDisableEdit)
			{
				WCHAR szCls[ARRAYSIZE(WC_EDITW) + 1];
				if (GetClassNameW((HWND)lParam, szCls, ARRAYSIZE(szCls)) &&
					_wcsicmp(szCls, WC_EDITW) == 0)
					break;
			}
			[[fallthrough]];
		case WM_CTLCOLORBTN:
		case WM_CTLCOLORDLG:
		case WM_CTLCOLOREDIT:
		case WM_CTLCOLORLISTBOX:
		{
			const auto* const ptc = GetThreadCtx();
			SetTextColor((HDC)wParam, ptc->crDefText);
			SetBkColor((HDC)wParam, m_crBk != CLR_DEFAULT ? m_crBk : ptc->crDefBkg);
			SetDCBrushColor((HDC)wParam, m_crBk != CLR_DEFAULT ? m_crBk : ptc->crDefBkg);
			return (LRESULT)GetStockBrush(DC_BRUSH);
		}
		break;

		case WM_SIZE:
			ECK_GET_SIZE_LPARAM(m_cxClient, m_cyClient, lParam);
			return 0;

		case WM_PRINTCLIENT:
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint(hWnd, wParam, ps);
			SetDCBrushColor(ps.hdc, m_crBk == CLR_DEFAULT ?
				GetThreadCtx()->crDefBkg : m_crBk);
			FillRect(ps.hdc, &ps.rcPaint, GetStockBrush(DC_BRUSH));
			if (m_hbmBk)
			{
				HDC hCDC = CreateCompatibleDC(ps.hdc);
				SelectObject(hCDC, m_hbmBk);
				const RECT rc{ 0,0,m_cxClient,m_cyClient };
				DrawBackgroundImage32(ps.hdc, hCDC, rc, m_cxImage, m_cyImage,
					m_iBkImageMode, m_bFullWndImage);
				DeleteDC(hCDC);
			}
			EndPaint(hWnd, wParam, ps);
		}
		return 0;

		case WM_ERASEBKGND:
		{
			const auto hDC = (HDC)wParam;
			RECT rc;
			GetClipBox(hDC, &rc);
			SetDCBrushColor(hDC, m_crBk == CLR_DEFAULT ?
				GetThreadCtx()->crDefBkg : m_crBk);
			FillRect(hDC, &rc, GetStockBrush(DC_BRUSH));
		}
		return TRUE;

		case WM_DESTROY:
		{
			for (const auto& e : m_Tray)
			{
				NOTIFYICONDATAW nid;
				nid.cbSize = sizeof(nid);
				nid.hWnd = hWnd;
				nid.uID = e.uID;
				nid.uFlags = 0;
				Shell_NotifyIconW(NIM_DELETE, &nid);
			}
			m_Tray.clear();
		}
		break;
		}

		if (uMsg == s_uTrayMsg)
		{
			OnTrayNotify(LOWORD(lParam), HIWORD(lParam),
				GET_X_LPARAM(wParam), GET_Y_LPARAM(wParam));
			return 0;
		}
		else if (uMsg == s_uTaskbarCreatedMsg)
		{
			NOTIFYICONDATAW nid;
			nid.cbSize = sizeof(nid);
			nid.hWnd = hWnd;
			nid.uCallbackMessage = s_uTrayMsg;
			for (const auto& e : m_Tray)
			{
				nid.uID = e.uID;
				nid.uFlags = e.uFlags | NIF_MESSAGE;
				nid.dwState = nid.dwStateMask = e.dwState;
				nid.hIcon = e.hIcon;
				if (e.rsTip.IsEmpty())
					wcscpy_s(nid.szTip, e.rsTip.Data());
				Shell_NotifyIconW(NIM_SETVERSION, &nid);
				Shell_NotifyIconW(NIM_ADD, &nid);
			}
			return 0;
		}
		return CWnd::OnMsg(hWnd, uMsg, wParam, lParam);
	}

	virtual void OnTrayNotify(UINT uMsg, UINT uID, int x, int y)
	{

	}

	EckInline HBITMAP GetBkImage() const { return m_hbmBk; }

	EckInline void SetBkImage(HBITMAP hbmBk)
	{
		m_hbmBk = hbmBk;
		BITMAP bm;
		GetObjectW(hbmBk, sizeof(bm), &bm);
		m_cxImage = bm.bmWidth;
		m_cyImage = bm.bmHeight;
	}

	EckInline constexpr void SetBkImageMode(BkImgMode iMode) { m_iBkImageMode = iMode; }
	EckInline constexpr BkImgMode GetBkImageMode() const { return m_iBkImageMode; }

	EckInline constexpr void SetFullWndImage(BOOL b) { m_bFullWndImage = b; }
	EckInline constexpr BOOL GetFullWndImage() const { return m_bFullWndImage; }

	EckInline void SetMoveable(BOOL bMoveable)
	{
		m_bMoveable = bMoveable;
		CMenu SysMenu(GetSystemMenu(GetHWND(), FALSE));
		if (bMoveable)
			SysMenu.EnableItem(SC_MOVE, MF_GRAYED, FALSE);
		else
			SysMenu.EnableItem(SC_MOVE, MF_ENABLED, FALSE);
		(void)SysMenu.Detach();
	}
	EckInline constexpr BOOL GetMoveable() const { return m_bMoveable; }

	EckInline constexpr void SetEscClose(BOOL b) { m_bEscClose = b; }
	EckInline constexpr BOOL GetEscClose() const { return m_bEscClose; }

	EckInline constexpr void SetTotalMove(BOOL b) { m_bTotalMove = b; }
	EckInline constexpr BOOL GetTotalMove() const { return m_bTotalMove; }

	EckInline constexpr void SetBkClr(COLORREF crBk) { m_crBk = crBk; }
	EckInline constexpr COLORREF GetBkClr() const { return m_crBk; }

	EckInline constexpr SIZE GetBkImageSize() const { return { m_cxImage,m_cyImage }; }

	BOOL TrayAdd(UINT uID, HICON hIcon, PCWSTR pszTip, DWORD dwState = 0u, BOOL bShowTip = TRUE)
	{
		NOTIFYICONDATAW nid;
		nid.cbSize = sizeof(nid);
		nid.hWnd = HWnd;
		nid.uID = uID;
		nid.uFlags = NIF_MESSAGE | NIF_STATE | (bShowTip ? NIF_TIP : 0);
		nid.uCallbackMessage = s_uTrayMsg;
		nid.uVersion = NOTIFYICON_VERSION_4;
		nid.dwState = nid.dwStateMask = dwState;
		if (hIcon)
		{
			nid.uFlags |= NIF_ICON;
			nid.hIcon = hIcon;
		}
		if (pszTip)
		{
			nid.uFlags |= NIF_TIP;
			wcscpy_s(nid.szTip, pszTip);
		}

		Shell_NotifyIconW(NIM_SETVERSION, &nid);
		if (!Shell_NotifyIconW(NIM_ADD, &nid))
			return FALSE;
		m_Tray.emplace_back(uID, nid.uFlags, dwState, hIcon, pszTip);
		return TRUE;
	}

	BOOL TrayModify(UINT uID, UINT uFlags, HICON hIcon = nullptr,
		PCWSTR pszTip = nullptr, DWORD dwState = 0u)
	{
		auto it = std::find_if(m_Tray.begin(), m_Tray.end(), [uID](const TRAY& x)
			{
				return x.uID == uID;
			});
		if (it == m_Tray.end())
		{
			EckDbgPrint(L"** WARNING ** 试图修改未经内部维护的托盘图标");
			return FALSE;
		}
		EckAssert(uFlags == (uFlags & ~(NIF_ICON | NIF_TIP | NIF_STATE)));
		uFlags &= ~(NIF_ICON | NIF_TIP | NIF_STATE | NIF_SHOWTIP);
		NOTIFYICONDATAW nid;
		nid.cbSize = sizeof(nid);
		nid.hWnd = HWnd;
		nid.uID = uID;
		nid.uFlags = uFlags;
		it->uFlags = uFlags;
		if (IsBitSet(uFlags, NIF_ICON))
		{
			it->hIcon = hIcon;
			nid.hIcon = hIcon;
		}
		if (IsBitSet(uFlags, NIF_TIP))
		{
			it->rsTip = pszTip;
			wcscpy_s(nid.szTip, pszTip);
		}
		if (IsBitSet(uFlags, NIF_STATE))
		{
			it->dwState = dwState;
			nid.dwState = nid.dwStateMask = dwState;
		}

		return Shell_NotifyIconW(NIM_MODIFY, &nid);
	}

	BOOL TrayDelete(UINT uID)
	{
		auto it = std::find_if(m_Tray.begin(), m_Tray.end(), [uID](const TRAY& x)
			{
				return x.uID == uID;
			});
		if (it == m_Tray.end())
		{
			EckDbgPrint(L"** WARNING ** 试图删除未经内部维护的托盘图标");
			return FALSE;
		}
		NOTIFYICONDATAW nid;
		nid.cbSize = sizeof(nid);
		nid.hWnd = HWnd;
		nid.uID = uID;
		nid.uFlags = 0;

		if (!Shell_NotifyIconW(NIM_DELETE, &nid))
			return FALSE;
		else
		{
			m_Tray.erase(it);
			return TRUE;
		}
	}

	BOOL TraySetFocus(UINT uID)
	{
		NOTIFYICONDATAW nid;
		nid.cbSize = sizeof(nid);
		nid.hWnd = HWnd;
		nid.uID = uID;
		nid.uFlags = 0;
		return Shell_NotifyIconW(NIM_SETFOCUS, &nid);
	}

	BOOL TrayPopBalloon(UINT uID, PCWSTR pszContent, PCWSTR pszTitle = nullptr,
		HICON hBalloonIcon = nullptr, DWORD dwInfoFlags = 0u, BOOL bRealTime = FALSE)
	{
		NOTIFYICONDATAW nid;
		nid.cbSize = sizeof(nid);
		nid.hWnd = HWnd;
		nid.uID = uID;
		nid.uFlags = NIF_INFO | (bRealTime ? NIF_REALTIME : 0);

		if (pszContent)
			wcscpy_s(nid.szInfo, pszContent);
		else
			nid.szInfo[0] = L'\0';
		if (pszTitle)
			wcscpy_s(nid.szInfoTitle, pszTitle);
		else
			nid.szInfoTitle[0] = L'\0';
		nid.hBalloonIcon = hBalloonIcon;
		nid.dwInfoFlags = dwInfoFlags;
		return Shell_NotifyIconW(NIM_MODIFY, &nid);
	}
};
ECK_RTTI_IMPL_BASE_INLINE(CForm, CWnd);
inline UINT CForm::s_uTrayMsg = RegisterWindowMessageW(MSGREG_FORMTRAY);
inline UINT CForm::s_uTaskbarCreatedMsg = RegisterWindowMessageW(L"TaskbarCreated");
ECK_NAMESPACE_END