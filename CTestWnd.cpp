#include "CTestWnd.h"

void CTestWnd::Test()
{
	using namespace eck;
	using namespace eck::Literals;

	
}

LRESULT CTestWnd::OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CREATE:
		m_DP.Create(nullptr, WS_CHILD | WS_VISIBLE, 0,
			0, 0, 900, 900, hWnd, 0);
		Test();
		break;
	}
	return __super::OnMsg(hWnd, uMsg, wParam, lParam);
}
