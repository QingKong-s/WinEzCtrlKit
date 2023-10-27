#include "CDialog.h"
ECK_NAMESPACE_BEGIN
ATOM CDialog::RegisterWndClass(HINSTANCE hInstance)
{
	WNDCLASSW wc{};
	wc.cbWndExtra = sizeof(void*);
	wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
	wc.hInstance = hInstance;
	wc.lpfnWndProc = DefWindowProcW;
	wc.lpszClassName = WCN_DLG;
	wc.style = CS_DBLCLKS | CS_VREDRAW | CS_HREDRAW;
	wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
	return RegisterClassW(&wc);
}
ECK_NAMESPACE_END