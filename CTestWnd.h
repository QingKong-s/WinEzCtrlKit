#pragma once
#include "eck\CForm.h"
#include "eck\CDrawPanel.h"
#include "eck\CRegion.h"

using namespace eck;

struct CTestWnd :public eck::CForm
{
	CDrawPanel m_DP{};

	void Test();

	BOOL PreTranslateMessage(const MSG& Msg) override
	{
		return CForm::PreTranslateMessage(Msg);
	}

	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;

	ECK_CWND_CREATE;
	HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, HMENU hMenu, PCVOID pData = nullptr) override
	{
		IntCreate(dwExStyle, WCN_BK, pszText, dwStyle | WS_CLIPCHILDREN,
			x, y, cx, cy, hParent, hMenu, g_hInstance, this);
		EckDbgPrintWndMap();
		return m_hWnd;
	}
};