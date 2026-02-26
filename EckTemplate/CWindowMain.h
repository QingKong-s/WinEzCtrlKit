#pragma once
#include "CApp.h"

class CWindowMain final : public eck::CForm
{
private:
	eck::CEditExt m_EDUserName{};

	eck::CEditExt m_EDPassword{};
    eck::CButton m_CBShowPassword{};
	eck::CLinearLayoutH m_LytPassword{};

    eck::CButton m_CBRememberPassword{};
    eck::CButton m_CBLoginAuto{};
    eck::CLinearLayoutH m_LytOption{};

	eck::CButton m_BTLogin{};
	eck::CLinearLayoutV m_Layout{};

	HFONT m_hFont{};

	int m_iDpi{ 96 };

	void DmNewDpi(int iDpi) noexcept;
	void DmUpdateFixedSize() noexcept;

	void OnDestory() noexcept;
	LRESULT OnCreate(CREATESTRUCT* pcs) noexcept;
public:
	ECK_CWND_SINGLEOWNER(CWindowMain);
	ECK_CWND_CREATE_CLS_HINST(eck::WCN_DUMMY, eck::g_hInstance);

	LRESULT OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept override;

	BOOL PreTranslateMessage(const MSG& Msg) noexcept override;
};