#pragma once
#include "eck\CWnd.h"

#define WCN_TEST L"TesttttttttttttttWndddddddddd"

using eck::PCVOID;

class CTestWnd :public eck::CWnd
{

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{

		return DefWindowProcW(hWnd, uMsg, wParam, lParam);
	}
public:
	static ATOM RegisterWndClass()
	{
		WNDCLASSW wc{};
		wc.cbWndExtra = sizeof(void*);
		wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
		wc.hInstance = eck::g_hInstance;
		wc.lpfnWndProc = WndProc;
		wc.lpszClassName = WCN_TEST;
		wc.style = CS_DBLCLKS | CS_PARENTDC;
		return RegisterClassW(&wc);
	}

	EckInline HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, int nID, PCVOID pData = NULL)
	{
		m_hWnd = IntCreate(dwExStyle, WCN_TEST, pszText, dwStyle,
			x, y, cx, cy, hParent, eck::i32ToP<HMENU>(nID), eck::g_hInstance, this);
		return m_hWnd;
	}
};