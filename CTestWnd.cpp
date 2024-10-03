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
		/*m_DP.Create(nullptr, WS_CHILD | WS_VISIBLE, 0,
			0, 0, 900, 900, hWnd, 0);*/
		m_CB.Create(L"CheckButton", WS_CHILD | WS_VISIBLE|BS_AUTORADIOBUTTON, 0,
			0, 0, 100, 20, hWnd, 0);
		m_CB0.Create(L"CheckButton", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON, 0,
			0, 50, 100, 20, hWnd, 0);
		m_DDXRB.GetSignal().Connect([](CDdx& ddx)
			{
				EckDbgPrintFmt(L"DDXRB: %d", ddx.GetInt());
			}, 0);
		//m_DDXCB.Attach(&m_CB);
		
		m_DDXRB.Attach(&m_CB, &m_CB0);
		//Test();
		break;
	}
	return __super::OnMsg(hWnd, uMsg, wParam, lParam);
}
