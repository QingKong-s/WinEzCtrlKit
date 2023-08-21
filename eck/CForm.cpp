#include "CForm.h"

ECK_NAMESPACE_BEGIN
ATOM CForm::m_atomForm = 0;


LRESULT CALLBACK CForm::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_PAINT:
	{


	}
	return 0;
	}

	return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

ATOM CForm::RegisterWndClass(HINSTANCE hInstance)
{
	if (m_atomForm)
		return m_atomForm;
	WNDCLASSW wc{};
	wc.cbWndExtra = sizeof(void*);
	wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
	wc.hInstance = hInstance;
	wc.lpfnWndProc = WndProc;
	wc.lpszClassName = WCN_FORM;
	wc.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	m_atomForm = RegisterClassW(&wc);
	return m_atomForm;
}


ECK_NAMESPACE_END