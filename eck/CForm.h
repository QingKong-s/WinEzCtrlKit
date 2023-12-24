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
	COLORREF m_crBk = GetSysColor(COLOR_BTNFACE);
	HBRUSH m_hbrBk = GetSysColorBrush(COLOR_BTNFACE);
	HBITMAP m_hbmBk = NULL;
	int m_cxImage = 0,
		m_cyImage = 0;
	int m_iBkImageMode = DBGIF_TOPLEFT;

	int m_cxClient = 0,
		m_cyClient = 0;

	BITBOOL m_bMoveable : 1 = TRUE;
	BITBOOL m_bFullWndImage : 1 = FALSE;
	BITBOOL m_bEscClose : 1 = FALSE;
	BITBOOL m_bTotalMove : 1 = FALSE;
public:
	ECKPROP(GetBkImage, SetBkImage)				HBITMAP		BkImage;		// 背景图片
	ECKPROP(GetBkImageMode, SetBkImageMode)		int			BkImageMode;	// 背景图片模式
	ECKPROP(GetFullWndImage, SetFullWndImage)	BOOL		FullWndImage;	// 全窗口绘制背景图片
	ECKPROP(GetMoveable, SetMoveable)			BOOL		Moveable;		// 可否移动
	ECKPROP(GetEscClose, SetEscClose)			BOOL		EscClose;		// ESC关闭
	ECKPROP(GetTotalMove, SetTotalMove)			BOOL		TotalMove;		// 随意移动
	ECKPROP(GetBkColor, SetBkColor)				COLORREF	BkColor;		// 背景颜色
	ECKPROP_R(GetBkImageSize)					SIZE		BkImageSize;	// 背景图片大小

	EckInline static ATOM RegisterWndClass() { return EzRegisterWndClass(WCN_FORM); }

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
		}
		return CWnd::OnMsg(hWnd, uMsg, wParam, lParam);
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
};
ECK_NAMESPACE_END