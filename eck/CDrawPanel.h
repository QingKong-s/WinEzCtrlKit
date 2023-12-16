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

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		auto p = (CDrawPanel*)GetWindowLongPtrW(hWnd, 0);
		switch (uMsg)
		{
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint(hWnd, &ps);
			BitBltPs(&ps, p->m_DC.GetDC());
			EndPaint(hWnd, &ps);
		}
		return 0;
		case WM_SIZE:
			return HANDLE_WM_SIZE(hWnd, wParam, lParam, p->OnSize);
		case WM_CREATE:
		{
			RECT rc;
			GetClientRect(hWnd, &rc);
			p->m_DC.Create(hWnd);
			FillRect(p->m_DC.GetDC(), &rc, p->m_hbrBK);
			GdipCreateFromHDC(p->m_DC.GetDC(), &p->m_pGraphics);
			GdipSetSmoothingMode(p->m_pGraphics, SmoothingModeHighQuality);
		}
		return 0;
		case WM_NCCREATE:
			p = (CDrawPanel*)((CREATESTRUCTW*)lParam)->lpCreateParams;
			SetWindowLongPtrW(hWnd, 0, (LONG_PTR)p);
			break;
		}
		return DefWindowProcW(hWnd, uMsg, wParam, lParam);
	}

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
		GdipSetSmoothingMode(m_pGraphics, SmoothingModeHighQuality);

		m_cxClient = cx;
		m_cyClient = cy;
	}
public:
	static ATOM RegisterWndClass()
	{
		WNDCLASSW wc{};
		wc.cbWndExtra = sizeof(void*);
		wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
		wc.hInstance = eck::g_hInstance;
		wc.lpfnWndProc = WndProc;
		wc.lpszClassName = WCN_DRAWPANEL;
		wc.style = CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS;
		return RegisterClassW(&wc);
	}

	EckInline HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, int nID, PCVOID pData = NULL) override
	{
		m_hWnd = IntCreate(dwExStyle, WCN_DRAWPANEL, pszText, dwStyle,
			x, y, cx, cy, hParent, i32ToP<HMENU>(nID), g_hInstance, this);
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

	LRESULT CALLBACK OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override
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
	static ATOM RegisterWndClass()
	{
		WNDCLASSW wc{};
		wc.cbWndExtra = sizeof(void*);
		wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
		wc.hInstance = eck::g_hInstance;
		wc.lpfnWndProc = DefWindowProcW;
		wc.lpszClassName = WCN_DRAWPANELD2D;
		wc.style = CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS;
		return RegisterClassW(&wc);
	}

	EckInline HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, int nID, PCVOID pData = NULL) override
	{
		m_hWnd = IntCreate(dwExStyle, WCN_DRAWPANELD2D, pszText, dwStyle,
			x, y, cx, cy, hParent, i32ToP<HMENU>(nID), g_hInstance, this);
		return m_hWnd;
	}

	EckInline ID2D1DeviceContext* GetDC() const { return m_D2D.GetDC(); }

	EckInline IDXGISwapChain1* GetSwapChain() const { return m_D2D.GetSwapChain(); }
};
ECK_NAMESPACE_END