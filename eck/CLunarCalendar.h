#pragma once
#include "CWnd.h"
#include "CComboBox.h"

ECK_NAMESPACE_BEGIN
constexpr
int 
IDC_CB_Year = 101,
IDC_CB_Month = 102;


class CLunarCalendar :public CWnd
{
private:
	CComboBox m_CBYear{};
	CComboBox m_CBMonth{};

	static ATOM m_atomLunarCalendar;

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	void PaintUnit(RECT rc)
	{

	}
public:
	ATOM RegisterWndClass(HINSTANCE hInstance);

	HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, int nID, PCVOID pData = NULL)
	{
		dwStyle |= WS_CHILD;
		m_hWnd = CreateWindowExW(dwExStyle, WCN_LUNARCALENDAR, pszText, dwStyle,
			x, y, cx, cy, hParent, i32ToP<HMENU>(nID), g_hInstance, this);
		
		return m_hWnd;
	}
};

ECK_NAMESPACE_END