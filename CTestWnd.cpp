#include "CTestWnd.h"

void CTestWnd::Test()
{
	using namespace eck;
	using namespace eck::Literals;
	
}

LRESULT CTestWnd::OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	
	return __super::OnMsg(hWnd, uMsg, wParam, lParam);
}
