/*
* WinEzCtrlKit Library
*
* CBk.h £º Í¨ÓÃ¿Õ°×´°¿Ú
*
* Copyright(C) 2023 QingKong
*/
#pragma once
#include "CWnd.h"

ECK_NAMESPACE_BEGIN
class CBk :public CWnd
{
protected:
	static ATOM m_atomBK;
public:
	static ATOM RegisterWndClass(HINSTANCE hInstance);

	EckInline HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, int nID, PCVOID pData = NULL) override
	{
		m_hWnd = CreateWindowExW(dwExStyle, WCN_BK, pszText, dwStyle,
			x, y, cx, cy, hParent, i32ToP<HMENU>(nID), g_hInstance, NULL);
		return m_hWnd;
	}

	virtual void SetWindowProc(WNDPROC pfnWndProc)
	{
		SetWindowLongPtrW(m_hWnd, GWLP_WNDPROC, (LONG_PTR)pfnWndProc);
	}
};
ECK_NAMESPACE_END