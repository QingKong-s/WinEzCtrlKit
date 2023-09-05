#include "CTabHeader.h"

ECK_NAMESPACE_BEGIN
ATOM CTabHeader::s_Atom = 0;


LRESULT CALLBACK CTabHeader::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	auto p = (CTabHeader*)GetWindowLongPtrW(hWnd, 0);
	switch (uMsg)
	{
	case WM_NCCREATE:
		p = (CTabHeader*)((CREATESTRUCTW*)lParam)->lpCreateParams;
		SetWindowLongPtrW(hWnd, 0, (LONG_PTR)p);
		return TRUE;

	case WM_SETFONT:
		p->m_hFont = (HFONT)wParam;
		if (lParam)
			p->Redraw();
		return 0;


	}

	return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}


ATOM CTabHeader::RegisterWndClass(HINSTANCE hInstance)
{
	if (s_Atom)
		return s_Atom;
	WNDCLASSW wc{};
	wc.cbWndExtra = sizeof(void*);
	wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
	wc.hInstance = hInstance;
	wc.lpfnWndProc = WndProc;
	wc.lpszClassName = WCN_TABHEADER;
	wc.style = CS_DBLCLKS;
	s_Atom = RegisterClassW(&wc);
	return s_Atom;
}

ECK_NAMESPACE_END