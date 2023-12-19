/*
* WinEzCtrlKit Library
*
* CBk.h ： 通用空白窗口
*
* Copyright(C) 2023 QingKong
*/
#pragma once
#include "CWnd.h"

ECK_NAMESPACE_BEGIN
class CBk :public CWnd
{
public:
	static ATOM RegisterWndClass()
	{
		WNDCLASSW wc{};
		wc.cbWndExtra = sizeof(void*) * 4;
		wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
		wc.hInstance = g_hInstance;
		wc.lpfnWndProc = DefWindowProcW;
		wc.lpszClassName = WCN_BK;
		wc.style = CS_DBLCLKS | CS_PARENTDC;
		return RegisterClassW(&wc);
	}

	EckInline 
	ECK_CWND_CREATE
	{
		m_hWnd = IntCreate(dwExStyle, WCN_BK, pszText, dwStyle,
			x, y, cx, cy, hParent, hMenu, g_hInstance, NULL);
		return m_hWnd;
	}

	virtual void SetWindowProc(WNDPROC pfnWndProc)
	{
		SetWindowLongPtrW(m_hWnd, GWLP_WNDPROC, (LONG_PTR)pfnWndProc);
	}
};
ECK_NAMESPACE_END