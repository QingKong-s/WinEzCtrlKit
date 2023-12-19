#pragma once
#include "CWnd.h"

#include <vector>
#include <string>

ECK_NAMESPACE_BEGIN
struct CHARTPIEITEM
{
	PCWSTR pszText;
	int cchText;
	float fValue;
	COLORREF crBlock;
	COLORREF crText;
};
constexpr int
CPITC_MOVETEXTPTR = -2;
class CChartPie :public CWnd
{
private:
	struct ITEM
	{
		CRefStrW rsText;
		float fValue;
		COLORREF crBlock;
		COLORREF crText;
		GpSolidFill* pBrush;
		int cxText;
	};

	struct 
	{
		int cxBlock;
		int cxPadding;
		int cyColumn;
	}
	m_DpiSize{};
	int m_iDpi = USER_DEFAULT_SCREEN_DPI;
	int m_cxClient = 0, m_cyClient = 0;
	HDC m_hCDC = NULL;
	HBITMAP m_hBitmap = NULL;
	GpGraphics* m_pGraphics = NULL;
	std::vector<ITEM> m_Items{};
	int m_cxLegend = 0;
	int m_cyTitle = 0;
	CRefStrW m_rsTitle{};
	int m_cyColumn = 20;

	static ATOM m_atomChartPie;

	void Paint(HDC hDC)
	{
		RECT rc = { 0,0,m_cxClient,m_cyClient };
		FillRect(m_hCDC, &rc, GetSysColorBrush(COLOR_BTNFACE));

		DrawTextW(m_hCDC, m_rsTitle.Data(), m_rsTitle.Size(), &rc,
			DT_CENTER | DT_TOP | DT_WORDBREAK);
		//GdipFillPie(m_pGraphics,)
	}

	void UpdateDpiSize()
	{
		m_DpiSize =
		{
			DpiScale(12, m_iDpi),
			DpiScale(4, m_iDpi),
			DpiScale(m_cyColumn, m_iDpi),
		};
	}

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
public:
	static ATOM RegisterWndClass(HINSTANCE hInstance);

	EckInline 
	ECK_CWND_CREATE
	{
		m_hWnd = CreateWindowExW(dwExStyle, WCN_CHARTPIE, pszText, dwStyle,
			x, y, cx, cy, hParent, hMenu, g_hInstance, NULL);
		return m_hWnd;
	}

	int InsertItem(CHARTPIEITEM* pItem, int idxPos = -1)
	{
		CRefStrW rs;
		rs.Reserve(24);
		rs.PushBack(pItem->pszText);
		rs.PushBack(L" ");
		rs.PushBack(std::to_wstring(pItem->fValue).c_str());
		SIZE size;
		GetTextExtentPoint32W(m_hCDC, rs.Data(), rs.Size(), &size);
		if (size.cx > m_cxLegend)
			m_cxLegend = size.cx;
		
		ITEM Item{ std::move(rs),pItem->fValue,pItem->crBlock,pItem->crText,NULL,size.cx };
		GdipCreateSolidFill(ColorrefToARGB(pItem->crBlock), &Item.pBrush);
		if (idxPos < 0)
			m_Items.push_back(std::move(Item));
		else
			m_Items.insert(m_Items.begin() + idxPos, std::move(Item));
		Paint(m_hCDC);
		Redraw();
	}

	void SetTitle(PCWSTR pszTitle)
	{
		m_rsTitle = pszTitle;
		RECT rc = { 0,0,m_cxClient,m_cyClient };
		DrawTextW(m_hCDC, m_rsTitle.Data(), m_rsTitle.Size(), &rc,
			DT_CENTER | DT_TOP | DT_WORDBREAK | DT_CALCRECT);
		m_cyTitle = rc.bottom - rc.top;
		Paint(m_hCDC);
		Redraw();
	}
};
ECK_NAMESPACE_END