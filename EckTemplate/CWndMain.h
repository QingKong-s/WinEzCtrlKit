#pragma once
#include "CApp.h"

class CWndMain final : public eck::CForm
{
private:
	eck::CButton m_BT{};
	eck::CEditExt m_EDUserName{};
	eck::CEditExt m_EDPassword{};
	eck::CLinearLayoutV m_Layout{};

	HFONT m_hFont{};

	int m_iDpi{ 96 };

	void DmNewDpi(int iDpi);
	void DmUpdateFixedSize();

	void OnDestory();
	LRESULT OnCreate(HWND hWnd, CREATESTRUCT* pcs);
public:
	ECK_CWND_SINGLEOWNER(CWndMain);
	ECK_CWND_CREATE_CLS_HINST(eck::WCN_DUMMY, eck::g_hInstance);

	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;

	BOOL PreTranslateMessage(const MSG& Msg) override;
};