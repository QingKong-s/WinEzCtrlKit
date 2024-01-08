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
			uID(uID), uFlags(uFlags), dwState(dwState), hIcon(hIcon), rsTip(pszTip) {}
#endif
	};
	HBRUSH m_hbrBk = GetSysColorBrush(COLOR_BTNFACE);
	HBITMAP m_hbmBk = NULL;
	int m_cxImage = 0,
		m_cyImage = 0;
	int m_iBkImageMode = DBGIF_TOPLEFT;
	COLORREF m_crBk = GetSysColor(COLOR_BTNFACE);

	int m_cxClient = 0,
		m_cyClient = 0;

	std::vector<TRAY> m_Tray{};

#if ECKCXX20
	BITBOOL m_bMoveable : 1 = TRUE;
	BITBOOL m_bFullWndImage : 1 = FALSE;
	BITBOOL m_bEscClose : 1 = FALSE;
	BITBOOL m_bTotalMove : 1 = FALSE;
#else
	union
	{
		struct
		{
			BITBOOL m_bMoveable : 1 ;
			BITBOOL m_bFullWndImage : 1 ;
			BITBOOL m_bEscClose : 1;
			BITBOOL m_bTotalMove : 1 ;
		};
		UINT ECKPRIV_PLACEHOLDER___ = 0b1;
	};
#endif
protected:
	static UINT s_uTrayMsg;
	static UINT s_uTaskbarCreatedMsg;
public:
	ECKPROP(GetBkImage, SetBkImage)				HBITMAP		BkImage;		// 背景图片
	ECKPROP(GetBkImageMode, SetBkImageMode)		int			BkImageMode;	// 背景图片模式
	ECKPROP(GetFullWndImage, SetFullWndImage)	BOOL		FullWndImage;	// 全窗口绘制背景图片
	ECKPROP(GetMoveable, SetMoveable)			BOOL		Moveable;		// 可否移动
	ECKPROP(GetEscClose, SetEscClose)			BOOL		EscClose;		// ESC关闭
	ECKPROP(GetTotalMove, SetTotalMove)			BOOL		TotalMove;		// 随意移动
	ECKPROP(GetBkColor, SetBkColor)				COLORREF	BkColor;		// 背景颜色
	ECKPROP_R(GetBkImageSize)					SIZE		BkImageSize;	// 背景图片大小

	~CForm()
	{
		DeleteObject(m_hbrBk);
	}

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
		case WM_SIZE:
			ECK_GET_SIZE_LPARAM(m_cxClient, m_cyClient, lParam);
			return 0;
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint(hWnd, &ps);
			FillRect(ps.hdc, &ps.rcPaint, m_hbrBk);
			if (m_hbmBk)
			{
				HDC hCDC = CreateCompatibleDC(ps.hdc);
				SelectObject(hCDC, m_hbmBk);
				const RECT rc{ 0,0,m_cxClient,m_cyClient };
				DrawBackgroundImage32(ps.hdc, hCDC, rc, m_cxImage, m_cyImage, m_iBkImageMode, m_bFullWndImage);
				DeleteDC(hCDC);
			}
			EndPaint(hWnd, &ps);
		}
		return 0;
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
			OnTrayNotify(LOWORD(lParam), HIWORD(lParam), GET_X_LPARAM(wParam), GET_Y_LPARAM(wParam));
			return 0;
		}
		else if (uMsg == s_uTaskbarCreatedMsg)
		{
			for (const auto& e : m_Tray)
			{
				NOTIFYICONDATAW nid;
				nid.cbSize = sizeof(nid);
				nid.hWnd = hWnd;
				nid.uID = e.uID;
				nid.uFlags = e.uFlags | NIF_MESSAGE;
				nid.uCallbackMessage = s_uTrayMsg;
				nid.dwState = nid.dwStateMask = e.dwState;
				nid.hIcon = e.hIcon;
				if (e.rsTip.Data())
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

	EckInline HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, HMENU hMenu, PCVOID pData = NULL) override
	{
		return IntCreate(dwExStyle, WCN_FORM, pszText, dwStyle,
			x, y, cx, cy, hParent, hMenu, g_hInstance, NULL);
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

	EckInline void SetBkImageMode(int iMode) { EckAssert(iMode >= 0 && iMode <= 3); m_iBkImageMode = iMode; }

	EckInline int GetBkImageMode() const { return m_iBkImageMode; }

	EckInline void SetFullWndImage(BOOL bFullWndImage) { m_bFullWndImage = bFullWndImage; }

	EckInline BOOL GetFullWndImage() const { return m_bFullWndImage; }

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

	EckInline BOOL GetMoveable() const { return m_bMoveable; }

	EckInline void SetEscClose(BOOL bEscClose) { m_bEscClose = bEscClose; }

	EckInline BOOL GetEscClose() const { return m_bEscClose; }

	EckInline void SetTotalMove(BOOL bTotalMove) { m_bTotalMove = bTotalMove; }

	EckInline BOOL GetTotalMove() const { return m_bTotalMove; }

	EckInline void SetBkColor(COLORREF crBk)
	{
		m_crBk = crBk;
		DeleteObject(m_hbrBk);
		m_hbrBk = CreateSolidBrush(crBk);
	}

	EckInline COLORREF GetBkColor() const { return m_crBk; }

	EckInline SIZE GetBkImageSize() const { return { m_cxImage,m_cyImage }; }

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

	BOOL TrayModify(UINT uID, UINT uFlags, HICON hIcon = NULL,
		PCWSTR pszTip = NULL, DWORD dwState = 0u)
	{
		auto it = std::find_if(m_Tray.begin(), m_Tray.end(), [uID](const TRAY& x)
			{
				return x.uID == uID;
			});
		if (it == m_Tray.end())
		{
			EckDbgPrint(L"试图修改未经内部维护的托盘图标");
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
			EckDbgPrint(L"试图删除未经内部维护的托盘图标");
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

	BOOL TrayPopBalloon(UINT uID, PCWSTR pszContent, PCWSTR pszTitle = NULL,
		HICON hBalloonIcon = NULL, DWORD dwInfoFlags = 0u, BOOL bRealTime = FALSE)
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
inline UINT CForm::s_uTrayMsg = RegisterWindowMessageW(MSGREG_FORMTRAY);
inline UINT CForm::s_uTaskbarCreatedMsg = RegisterWindowMessageW(L"TaskbarCreated");
ECK_NAMESPACE_END