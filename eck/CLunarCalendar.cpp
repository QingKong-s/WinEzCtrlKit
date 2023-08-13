#include "CLunarCalendar.h"
#include "LunarDateLib.h"


ECK_NAMESPACE_BEGIN
ATOM CLunarCalendar::m_atomLunarCalendar = 0;


ATOM CLunarCalendar::RegisterWndClass(HINSTANCE hInstance)
{
	if (m_atomLunarCalendar)
		return m_atomLunarCalendar;
	WNDCLASSW wc{};
	wc.cbWndExtra = sizeof(void*);
	wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
	wc.hInstance = hInstance;
	wc.lpfnWndProc = WndProc;
	wc.lpszClassName = WCN_LUNARCALENDAR;
	wc.style = CS_DBLCLKS;
	m_atomLunarCalendar = RegisterClassW(&wc);
	return m_atomLunarCalendar;
}

LRESULT CALLBACK CLunarCalendar::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	auto p = (CLunarCalendar*)GetWindowLongPtrW(hWnd, 0);

	switch (uMsg)
	{
	case WM_NCCREATE:
		p = (CLunarCalendar*)((CREATESTRUCTW*)lParam)->lpCreateParams;
		SetWindowLongPtrW(hWnd, 0, (LONG_PTR)p);
		return TRUE;
	case WM_CREATE:
	{
		p->m_CBMonth.Create(NULL, WS_VISIBLE, 0, 0, 0, 0, 0, hWnd, IDC_CB_Year);
		p->m_CBMonth.Create(NULL, WS_VISIBLE, 0, 0, 0, 0, 0, hWnd, IDC_CB_Year);
	}
	return 0;
	}

	return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

ECK_NAMESPACE_END