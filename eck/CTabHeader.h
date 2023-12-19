#pragma once
#include "CUpDown.h"

ECK_NAMESPACE_BEGIN
enum
{
	THIM_TEXT = 1u << 0,
	THIM_PARAM = 1u << 1,
	THIM_IMAGE = 1u << 2,
	THIM_NORECALCSIZE = 1u << 3,
};

struct TABHEADERITEM
{
	UINT uMask;
	PWSTR pszText;
	int cchText;
	LPARAM lParam;
	int idxImage;
};

class CTabHeader :public CWnd
{
private:
	static constexpr int IDC_UPDOWN = 30001;
	struct ITEM
	{
		CRefStrW rsText;
		LPARAM lParam;
		int idxImage = -1;
		RECT rc;
		int cxText;
	};

	ECK_DS_BEGIN(Dpis)
		ECK_DS_ENTRY(cxUpDown, 30)
		ECK_DS_ENTRY(cyUpDown, 15)
		ECK_DS_ENTRY(cyDef, 35)
		ECK_DS_ENTRY(cxDefMinItem, 60)
		ECK_DS_ENTRY(cxTextPadding, 6)
		ECK_DS_ENTRY(cxDefItemPadding, 4);
	ECK_DS_END_VAR(m_Dpis);

	static ATOM s_Atom;

	HFONT m_hFont = NULL;
	UINT m_uNotifyMsg = 0u;
	std::vector<ITEM> m_Items{};
	HWND m_hParent = NULL;

	HDC m_hCDC = NULL;
	HBITMAP m_hBitmap = NULL;
	HGDIOBJ m_hOldBmp = NULL;

	CUpDown m_UpDown{};

	BOOL m_bMultiLine = FALSE;
	int m_iDpi = USER_DEFAULT_SCREEN_DPI;
	BOOL m_bRedraw = TRUE;

	int m_cxClient = 0,
		m_cyClient = 0;

	int m_idxFirstVisible = -1;
	int m_cxTotal = 0;

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	void Paint()
	{
		if (!m_bRedraw)
			return;
		
	}
public:
	static ATOM RegisterWndClass(HINSTANCE hInstance);

	
	ECK_CWND_CREATE
	{
		dwStyle |= WS_CHILD;
		m_hWnd = IntCreate(dwExStyle, WCN_TABHEADER, pszText, dwStyle,
			x, y, cx, cy, hParent, hMenu, g_hInstance, this);
		return m_hWnd;
	}

	int InsertItem(const TABHEADERITEM* pItem, int idx = -1)
	{
		ITEM Item{};
		if (IsBitSet(pItem->uMask, THIM_TEXT))
		{
			Item.rsText = pItem->pszText;
			SIZEL size;
			GetTextExtentPoint32W(m_hCDC, Item.rsText.Data(), Item.rsText.Size(), &size);
			Item.cxText = size.cx;
			m_cxTotal += size.cx;
		}
		if (IsBitSet(pItem->uMask, THIM_PARAM))
			Item.lParam = pItem->lParam;
		if (IsBitSet(pItem->uMask, THIM_IMAGE))
			Item.idxImage = pItem->idxImage;

		if (idx < 0)
		{
			idx = (int)m_Items.size();
			m_Items.push_back(std::move(Item));
		}
		else
			m_Items.insert(m_Items.begin() + idx, std::move(Item));

		if (!IsBitSet(pItem->uMask, THIM_NORECALCSIZE))
			ReCalcSize();

		Paint();
		Redraw();
		return idx;
	}

	void ReCalcSize()
	{
		if (m_bMultiLine)
		{

		}
		else
		{
			int xCurrPos = 0;
			int cxTemp;
			int cVisible = 0;
			for (auto& x : m_Items)
			{
				if (x.cxText < m_Dpis.cxDefMinItem)
					cxTemp = m_Dpis.cxDefMinItem;
				else
					cxTemp = x.cxText;
				x.rc = { xCurrPos,0,xCurrPos + cxTemp,m_cyClient };
				xCurrPos += (cxTemp + m_Dpis.cxDefItemPadding);
			}
		}
	}

	int HitTest(POINT pt)
	{

	}
};
ECK_NAMESPACE_END