/*
* WinEzCtrlKit Library
*
* CDrawPanel.h ： 画板
*
* Copyright(C) 2024 QingKong
*/
#pragma once
#include "CWnd.h"
#include "Utility.h"
#include "GraphicsHelper.h"

#include <dxgi1_2.h>

ECK_NAMESPACE_BEGIN
class CDrawPanel :public CWnd
{
private:
	CEzCDC m_DC{};
	GpGraphics* m_pGraphics = NULL;
	int m_cxClient = 0, m_cyClient = 0;
	HBRUSH m_hbrBK = NULL;

	void OnSize(HWND hWnd, UINT uState, int cx, int cy)
	{
		HDC hDC = GetDC(hWnd);

		HBITMAP hBitmap = CreateCompatibleBitmap(hDC, cx, cy);
		HDC hCDC = CreateCompatibleDC(hDC);
		HGDIOBJ hOld = SelectObject(hCDC, hBitmap);
		const RECT rc{ 0,0,cx,cy };
		m_DC.Create(hWnd);
		FillRect(m_DC.GetDC(), &rc, m_hbrBK);
		BitBlt(hCDC, 0, 0, std::min(cx, m_cxClient), std::min(cy, m_cyClient), m_DC.GetDC(), 0, 0, SRCCOPY);

		SelectObject(hCDC, hOld);
		DeleteDC(hCDC);

		SelectObject(m_DC.m_hCDC, m_DC.m_hOld);
		DeleteObject(m_DC.m_hBmp);

		m_DC.m_hOld = SelectObject(m_DC.m_hCDC, hBitmap);
		m_DC.m_hBmp = hBitmap;

		GdipDeleteGraphics(m_pGraphics);
		GdipCreateFromHDC(m_DC.GetDC(), &m_pGraphics);
		GdipSetSmoothingMode(m_pGraphics, Gdiplus::SmoothingModeHighQuality);

		m_cxClient = cx;
		m_cyClient = cy;
	}
public:
	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		switch (uMsg)
		{
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint(hWnd, &ps);
			BitBltPs(&ps, m_DC.GetDC());
			EndPaint(hWnd, &ps);
		}
		return 0;
		case WM_SIZE:
			return HANDLE_WM_SIZE(hWnd, wParam, lParam, OnSize);
		case WM_CREATE:
		{
			RECT rc;
			GetClientRect(hWnd, &rc);
			m_DC.Create(hWnd);
			FillRect(m_DC.GetDC(), &rc, m_hbrBK);
			GdipCreateFromHDC(m_DC.GetDC(), &m_pGraphics);
			GdipSetSmoothingMode(m_pGraphics, Gdiplus::SmoothingModeHighQuality);
		}
		return 0;
		}
		return CWnd::OnMsg(hWnd, uMsg, wParam, lParam);
	}

	ECK_CWND_CREATE;
	HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, HMENU hMenu, PCVOID pData = NULL) override
	{
		m_hWnd = IntCreate(dwExStyle, WCN_DRAWPANEL, pszText, dwStyle,
			x, y, cx, cy, hParent, hMenu, g_hInstance, this);
		return m_hWnd;
	}

	EckInline HDC GetHDC() const { return m_DC.GetDC(); }

	EckInline GpGraphics* GetGraphics() const { return m_pGraphics; }

	EckInline void SetBkBrush(HBRUSH hbr)
	{
		if (m_hbrBK)
			DeleteObject(m_hbrBK);
		m_hbrBK = hbr;
	}
};

class CDrawPanelD2D :public CWnd
{
private:
	CEzD2D m_D2D{};

	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		switch (uMsg)
		{
		case WM_PAINT:
			ValidateRect(hWnd, NULL);
			m_D2D.GetSwapChain()->Present(0, 0);
			return 0;
		case WM_SIZE:
			m_D2D.ReSize(0, LOWORD(lParam), HIWORD(lParam), 0);
			return 0;
		case WM_CREATE:
			m_D2D.Create(EZD2D_PARAM::MakeBitblt(hWnd, g_pDxgiFactory, g_pDxgiDevice, g_pD2dDevice, 0, 0));
			return 0;
		}
		return CWnd::OnMsg(hWnd, uMsg, wParam, lParam);
	}
public:
	ECK_CWND_CREATE;
	HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, HMENU hMenu, PCVOID pData = NULL) override
	{
		m_hWnd = IntCreate(dwExStyle, WCN_DRAWPANELD2D, pszText, dwStyle,
			x, y, cx, cy, hParent, hMenu, g_hInstance, this);
		return m_hWnd;
	}

	EckInline ID2D1DeviceContext* GetDC() const { return m_D2D.GetDC(); }

	EckInline IDXGISwapChain1* GetSwapChain() const { return m_D2D.GetSwapChain(); }
};
ECK_NAMESPACE_END