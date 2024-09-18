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
	ECK_RTTI(CBk);

	ECK_CWND_CREATE;
	HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, HMENU hMenu, PCVOID pData = nullptr) override
	{
		m_hWnd = IntCreate(dwExStyle, WCN_BK, pszText, dwStyle,
			x, y, cx, cy, hParent, hMenu, g_hInstance, nullptr);
		return m_hWnd;
	}
};
ECK_RTTI_IMPL_BASE_INLINE(CBk, CWnd);
ECK_NAMESPACE_END