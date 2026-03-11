#pragma once
#include "CApp.h"

class CWindowMain final : public Dui::CDuiWindow
{
private:
	Dui::CButton m_BT{};
	Dui::CEdit m_EDUserName{};
	Dui::CEdit m_EDPassword{};
	eck::CLinearLayoutV m_Layout{};

	void OnDestory();
	LRESULT OnCreate();
public:
	ECK_CWND_SINGLEOWNER(CWindowMain);
	ECK_CWND_CREATE_CLS_HINST(eck::WCN_DUMMY, eck::g_hInstance);

	LRESULT OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept override;
};