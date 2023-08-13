#include "CBk.h"

ECK_NAMESPACE_BEGIN
ATOM CBk::m_atomBK = 0;
ATOM CBk::RegisterWndClass(HINSTANCE hInstance)
{
	if (m_atomBK)
		return m_atomBK;
	WNDCLASSW wc{};
	wc.cbWndExtra = sizeof(void*) * 4;
	wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
	wc.hInstance = hInstance;
	wc.lpfnWndProc = DefWindowProcW;
	wc.lpszClassName = WCN_BK;
	wc.style = CS_DBLCLKS | CS_PARENTDC;
	m_atomBK = RegisterClassW(&wc);
	return m_atomBK;
}
ECK_NAMESPACE_END