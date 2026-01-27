#pragma once
#include "CApp.h"

class CWndMain final : public Dui::CDuiWnd
{
private:
	Dui::CButton m_BT{};
	Dui::CEdit m_EDUserName{};
	Dui::CEdit m_EDPassword{};
	eck::CLinearLayoutV m_Layout{};

	void OnDestory();
	LRESULT OnCreate();
public:
	ECK_CWND_SINGLEOWNER(CWndMain);
	ECK_CWND_CREATE_CLS_HINST(eck::WCN_DUMMY, eck::g_hInstance);

	LRESULT OnMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept override;
};