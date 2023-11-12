#include "CDrawPanel.h"

ECK_NAMESPACE_BEGIN
LRESULT CDrawPanel::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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

void CDrawPanel::OnSize(HWND hWnd, UINT uState, int cx, int cy)
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

ATOM CDrawPanel::RegisterWndClass()
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

LRESULT CDrawPanelD2D::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	auto p = (CDrawPanelD2D*)GetWindowLongPtrW(hWnd, 0);
	switch (uMsg)
	{
	case WM_PAINT:
		ValidateRect(hWnd, NULL);
		p->m_pSwapChain->Present(0, 0);
		return 0;
	case WM_SIZE:
		EzD2dReSize(p->m_pDC, p->m_pSwapChain, p->m_pBitmap, 0, LOWORD(lParam), HIWORD(lParam), 0);
		return 0;
	case WM_CREATE:
		EzD2D(EZD2D_PARAM::MakeBitblt(hWnd, g_pDxgiFactory, g_pDxgiDevice, g_pD2dDevice, 0, 0),
			p->m_pDC, p->m_pSwapChain, p->m_pBitmap);
		return 0;
	case WM_NCCREATE:
		p = (CDrawPanelD2D*)((CREATESTRUCTW*)lParam)->lpCreateParams;
		SetWindowLongPtrW(hWnd, 0, (LONG_PTR)p);
		break;
	}
	return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

ATOM CDrawPanelD2D::RegisterWndClass()
{
	WNDCLASSW wc{};
	wc.cbWndExtra = sizeof(void*);
	wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
	wc.hInstance = eck::g_hInstance;
	wc.lpfnWndProc = WndProc;
	wc.lpszClassName = WCN_DRAWPANELD2D;
	wc.style = CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS;
	return RegisterClassW(&wc);
}

ECK_NAMESPACE_END