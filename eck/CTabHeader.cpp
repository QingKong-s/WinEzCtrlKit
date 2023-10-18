#include "CTabHeader.h"

ECK_NAMESPACE_BEGIN
ATOM CTabHeader::s_Atom = 0;


LRESULT CALLBACK CTabHeader::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	auto p = (CTabHeader*)GetWindowLongPtrW(hWnd, 0);
	switch (uMsg)
	{
	case WM_SIZE:
	{
		p->m_cxClient = LOWORD(lParam);
		p->m_cyClient = HIWORD(lParam);
		DbBufReSize(hWnd, p->m_hCDC, p->m_hBitmap, p->m_hOldBmp, p->m_cxClient, p->m_cyClient);
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
					if (p->m_idxFirstVisible< 0)
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
				return TRUE;// ×èÖ¹Î»ÖÃ¸ü¸Ä
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
		DbBufPrepare(hWnd, p->m_hCDC, p->m_hBitmap, p->m_hOldBmp);

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
		DbBufFree(p->m_hCDC, p->m_hBitmap, p->m_hOldBmp);
		return 0;

	case WM_SETREDRAW:
		p->m_bRedraw = (BOOL)wParam;
		break;
	}

	return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}


ATOM CTabHeader::RegisterWndClass(HINSTANCE hInstance)
{
	if (s_Atom)
		return s_Atom;
	WNDCLASSW wc{};
	wc.cbWndExtra = sizeof(void*);
	wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
	wc.hInstance = hInstance;
	wc.lpfnWndProc = WndProc;
	wc.lpszClassName = WCN_TABHEADER;
	wc.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	s_Atom = RegisterClassW(&wc);
	return s_Atom;
}

ECK_NAMESPACE_END