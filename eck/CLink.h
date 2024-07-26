/*
* WinEzCtrlKit Library
*
* CLink.h �� ��׼���ӱ�ǩ
*
* Copyright(C) 2024 QingKong
*/
#pragma once
#include "CWnd.h"

ECK_NAMESPACE_BEGIN
class CLink :public CWnd
{
public:
	ECK_CWND_CREATE;
	HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, HMENU hMenu, PCVOID pData = NULL) override
	{
		return IntCreate(dwExStyle, WC_LINK, pszText, dwStyle,
			x, y, cx, cy, hParent, hMenu, NULL, NULL);
	}

	LRESULT OnNotifyMsg(HWND hParent, UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bProcessed) override
	{
		if (ShouldAppsUseDarkMode())
			switch (uMsg)
			{
			case WM_CTLCOLORSTATIC:
			{
				bProcessed = TRUE;
				const auto* const ptc = GetThreadCtx();
				SetTextColor((HDC)wParam, ptc->crDefText);
				SetDCBrushColor((HDC)wParam, ptc->crDefBkg);
				SetBkColor((HDC)wParam, ptc->crDefBkg);
				return (LRESULT)GetStockBrush(DC_BRUSH);
			}
			}
		return CWnd::OnNotifyMsg(hParent, uMsg, wParam, lParam, bProcessed);
	}

	EckInline int GetIdealHeight()
	{
		return (int)SendMsg(LM_GETIDEALHEIGHT, 0, 0);
	}

	EckInline int GetIdealSize(int cxMax, SIZE* psize)
	{
		return (int)SendMsg(LM_GETIDEALSIZE, cxMax, (LPARAM)psize);
	}

	EckInline BOOL GetItem(LITEM* pli)
	{
		return (BOOL)SendMsg(LM_GETITEM, 0, (LPARAM)pli);
	}

	EckInline BOOL HitTest(LHITTESTINFO* plht)
	{
		return (BOOL)SendMsg(LM_HITTEST, 0, (LPARAM)plht);
	}

	EckInline BOOL SetItem(LITEM* pli)
	{
		return (BOOL)SendMsg(LM_SETITEM, 0, (LPARAM)pli);
	}
};
ECK_NAMESPACE_END