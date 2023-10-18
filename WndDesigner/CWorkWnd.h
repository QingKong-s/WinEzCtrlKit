#pragma once
#include "CApp.h"

#include "..\eck\CBk.h"

class CWorkWnd :public eck::CBk
{
private:
	friend class CWndMain;

	HBRUSH m_hbrWorkWindow = NULL;
	eck::CEzCDC m_mdcGridPoint{};
	BOOL m_bRBtnDown = FALSE;

	EckInline HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, int nID, PCVOID pData = NULL) override
	{
		m_hWnd = CreateWindowExW(dwExStyle, WCN_BK, pszText, dwStyle,
			x, y, cx, cy, hParent, eck::i32ToP<HMENU>(nID), eck::g_hInstance, NULL);

		int cxPointGap = eck::DpiScale(8, eck::GetDpi(m_hWnd));
		RECT rc{ 0,0,cxPointGap * 4,cxPointGap * 4 };
		HDC hcdcGridPoint = m_mdcGridPoint.Create(m_hWnd, rc.right, rc.bottom);
		FillRect(hcdcGridPoint, &rc, GetSysColorBrush(COLOR_BTNFACE));
		EckCounter(rc.right / cxPointGap, i)
		{
			EckCounter(rc.bottom / cxPointGap, j)
			{
				SetPixel(hcdcGridPoint, i * cxPointGap, j * cxPointGap, eck::Colorref::Black);
			}
		}

		m_hbrWorkWindow = CreatePatternBrush(m_mdcGridPoint.GetBitmap());
		return m_hWnd;
	}

	int GetGridPointGap()
	{
		return 8;
	}
};

class CWorkWndBk :public eck::CBk
{
private:
	friend class CWndMain;

	HBRUSH m_hbrWorkWindow = NULL;
	eck::CEzCDC m_mdcGridPoint{};
	BOOL m_bRBtnDown = FALSE;

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
public:
	EckInline HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, int nID, PCVOID pData = NULL) override
	{
		m_hWnd = CreateWindowExW(dwExStyle, WCN_BK, pszText, dwStyle,
			x, y, cx, cy, hParent, eck::i32ToP<HMENU>(nID), eck::g_hInstance, NULL);
		SetWindowProc(WndProc);
		return m_hWnd;
	}
};