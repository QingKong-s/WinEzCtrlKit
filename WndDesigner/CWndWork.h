#pragma once
#include "CProject.h"

class CWndWork : public eck::CWnd
{
private:
	friend class CWndMain;

	BOOL m_bRBtnDown{};

	DsForm* m_pForm{};
public:
	CWndWork(DsForm* pForm) : m_pForm{ pForm } {}

	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;
};