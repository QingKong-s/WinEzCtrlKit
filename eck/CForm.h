#pragma once
#include "CWnd.h"

ECK_NAMESPACE_BEGIN

class CForm :public CWnd
{
private:
	COLORREF m_crWnd = GetSysColor(COLOR_BTNFACE);
	HBITMAP m_hbmWnd = NULL;
	BITBOOL m_bMoveable : 1 = TRUE;

	static ATOM m_atomForm;


	static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
public:
	static ATOM RegisterWndClass(HINSTANCE hInstance);


};

ECK_NAMESPACE_END