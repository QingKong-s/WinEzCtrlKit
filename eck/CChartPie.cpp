#include "CChartPie.h"

ECK_NAMESPACE_BEGIN
ATOM CChartPie::m_atomChartPie = 0;

LRESULT CALLBACK CChartPie::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	auto p = (CChartPie*)GetWindowLongPtrW(hWnd, 0);
	switch (uMsg)
	{
	case WM_SIZE:
	{
		p->m_cxClient = LOWORD(lParam);
		p->m_cyClient = HIWORD(lParam);
		DeleteObject(p->m_hBitmap);
		HDC hDC = GetDC(hWnd);
		p->m_hBitmap = CreateCompatibleBitmap(hDC,p->m_cxClient, p->m_cyClient);
		SelectObject(p->m_hCDC, p->m_hBitmap);

		GdipCreateFromHDC(p->m_hCDC, &p->m_pGraphics);
		ReleaseDC(hWnd, hDC);
	}
	return 0;

	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		BeginPaint(hWnd, &ps);
		BitBltPs(&ps, p->m_hCDC);
		EndPaint(hWnd, &ps);
	}
	return 0;

	case WM_CREATE:
	{
		HDC hDC = GetDC(hWnd);
		p->m_hCDC = CreateCompatibleDC(hDC);
		p->m_hBitmap = CreateCompatibleBitmap(hDC, 8, 8);
		SelectObject(p->m_hCDC, p->m_hBitmap);

		GdipCreateFromHDC(p->m_hCDC, &p->m_pGraphics);
		ReleaseDC(hWnd, hDC);

		p->m_iDpi = GetDpi(hWnd);
		p->UpdateDpiSize();
	}
	return 0;

	case WM_NCCREATE:
		p = (CChartPie*)((CREATESTRUCTW*)lParam)->lpCreateParams;
		SetWindowLongPtrW(hWnd, 0, (LONG_PTR)p);
		return TRUE;
	}

	return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

ATOM CChartPie::RegisterWndClass(HINSTANCE hInstance)
{
	if (m_atomChartPie)
		return m_atomChartPie;
	WNDCLASSW wc{};
	wc.cbWndExtra = sizeof(void*);
	wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
	wc.hInstance = hInstance;
	wc.lpfnWndProc = WndProc;
	wc.lpszClassName = WCN_CHARTPIE;
	wc.style = CS_DBLCLKS | CS_VREDRAW | CS_HREDRAW;
	m_atomChartPie = RegisterClassW(&wc);
	return m_atomChartPie;
}

ECK_NAMESPACE_END