/*
* WinEzCtrlKit Library
*
* CLunarCalendar.h ： 农历月历
*
* Copyright(C) 2023 QingKong
*/
#pragma once
#include "CWnd.h"
#include "CComboBox.h"

ECK_NAMESPACE_BEGIN
class CLunarCalendar :public CWnd
{
private:
	CComboBox m_CBYear{};
	CComboBox m_CBMonth{};

	void PaintUnit(RECT rc)
	{

	}
public:
	ECK_CWND_CREATE;
	HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, HMENU hMenu, PCVOID pData = nullptr) override
	{
		dwStyle |= WS_CHILD;
		m_hWnd = IntCreate(dwExStyle, WCN_LUNARCALENDAR, pszText, dwStyle,
			x, y, cx, cy, hParent, hMenu, g_hInstance, this);
		
		return m_hWnd;
	}
};
ECK_NAMESPACE_END