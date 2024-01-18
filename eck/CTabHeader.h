#pragma once
#include "CUpDown.h"
ECK_DISABLE_ARITHMETIC_OVERFLOW_WARNING

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
	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		auto p = (CTabHeader*)GetWindowLongPtrW(hWnd, 0);
		switch (uMsg)
		{
		case WM_SIZE:
		{
			p->m_cxClient = LOWORD(lParam);
			p->m_cyClient = HIWORD(lParam);
			//DbBufReSize(hWnd, p->m_hCDC, p->m_hBitmap, p->m_hOldBmp, p->m_cxClient, p->m_cyClient);
			p->Paint();
		}
		return 0;

		case WM_NOTIFY:
		{
			if (((NMHDR*)lParam)->hwndFrom == p->m_UpDown.GetHWND())
			{
				if (((NMHDR*)lParam)->code == UDN_DELTAPOS)
				{
					auto pnmud = (NMUPDOWN*)lParam;
					if (pnmud->iDelta < 0)
					{
						--p->m_idxFirstVisible;
						if (p->m_idxFirstVisible < 0)
						{
							p->m_idxFirstVisible = 0;
							return TRUE;
						}

						p->Paint();
						p->Redraw();
					}
					else
					{
						int xCurrPos = 0;
						int cxTemp;
						int cVisible = 0;
						for (SIZE_T i = p->m_idxFirstVisible; i < p->m_Items.size(); ++i)
						{
							auto& x = p->m_Items[i];
							if (x.cxText < p->m_Dpis.cxDefMinItem)
								cxTemp = p->m_Dpis.cxDefMinItem;
							else
								cxTemp = x.cxText;

							xCurrPos += (cxTemp + p->m_Dpis.cxDefItemPadding);
							//if(xCurrPos>p->m_cxClient-p)
						}
					}
					return TRUE;// 阻止位置更改
				}
			}
		}
		break;

		case WM_NCCREATE:
			p = (CTabHeader*)((CREATESTRUCTW*)lParam)->lpCreateParams;
			SetWindowLongPtrW(hWnd, 0, (LONG_PTR)p);
			return TRUE;

		case WM_CREATE:
		{
			//DbBufPrepare(hWnd, p->m_hCDC, p->m_hBitmap, p->m_hOldBmp);

			p->m_iDpi = GetDpi(hWnd);
			UpdateDpiSize(p->m_Dpis, p->m_iDpi);
			p->m_UpDown.Create(NULL, WS_CHILD | UDS_HORZ, 0,
				0, 0, p->m_Dpis.cxUpDown, p->m_Dpis.cyUpDown, hWnd, IDC_UPDOWN);
		}
		return 0;

		case WM_DPICHANGED_AFTERPARENT:
		{
			p->m_iDpi = GetDpi(hWnd);
			UpdateDpiSize(p->m_Dpis, p->m_iDpi);
		}
		return 0;

		case WM_SETFONT:
		{
			p->m_hFont = (HFONT)wParam;
			if (lParam)
			{
				p->Paint();
				p->Redraw();
			}
		}
		return 0;

		case WM_DESTROY:
			//DbBufFree(p->m_hCDC, p->m_hBitmap, p->m_hOldBmp);
			return 0;

		case WM_SETREDRAW:
			p->m_bRedraw = (BOOL)wParam;
			break;
		}

		return CWnd::OnMsg(hWnd, uMsg, wParam, lParam);
	}

	ECK_CWND_CREATE;
	HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, HMENU hMenu, PCVOID pData = NULL) override
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