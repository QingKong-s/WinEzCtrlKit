#include "CColorPicker.h"

ECK_NAMESPACE_BEGIN
WND_RECORDER_INIT(CColorPicker)
SUBCLASS_REF_MGR_INIT(CColorPicker, CColorPicker::PARENTSCCTX*, SCID_COLORPICKERPARENT, CColorPicker::SubclassProc_Parent, ObjRecordRefStdDeleter)

ATOM CColorPicker::m_atomColorPicker = 0;
WNDPROC CColorPicker::m_pfnColorPickerDefProc = NULL;

constexpr struct CPPRESETCOLOR
{
	COLORREF cr;
	PCWSTR pszName;
}
c_ColorPickerPresetClr[] =
{
	{CLR_DEFAULT,L"Ä¬ÈÏ"},
	{CLR_INVALID,L"×Ô¶¨Òå..."},
	{0x0000FF,L"ºìÉ«"},
	{0x00FF00,L"ÂÌÉ«"},
	{0xFF0000,L"À¶É«"},
	{0x00FFFF,L"»ÆÉ«"},
	{0xFF00FF,L"Æ·ºì"},
	{0xFFFF00,L"ÑÞÇà"},
	{0x000080,L"ºìºÖ"},
	{0x008000,L"Ä«ÂÌ"},
	{0x008080,L"ºÖÂÌ"},
	{0x800000,L"²ØÇà"},
	{0x800080,L"×Ïºì"},
	{0x808000,L"ÉîÇà"},
	{0xC0C0C0,L"Ç³»Ò"},
	{0xC0DCC0,L"ÃÀÔªÂÌ"},
	{0xF0CAA6,L"Ç³À¶"},
	{0x808080,L"»ÒÉ«"},
	{0xA4A0A0,L"ÖÐÐÔ»Ò"},
	{0xF0FBFF,L"Èé°×"},
	{0x000000,L"ºÚÉ«"},
	{0xFFFFFF,L"°×É«"},
	{0xFF8080,L"À¶»Ò"},
	{0xE03058,L"²ØÀ¶"},
	{0x00E080,L"ÄÛÂÌ"},
	{0x80E000,L"ÇàÂÌ"},
	{0x0060C0,L"»ÆºÖ"},
	{0xFFA8FF,L"·Ûºì"},
	{0x00D8D8,L"ÄÛ»Æ"},
	{0xECECEC,L"Òø°×"},
	{0xFF0090,L"×ÏÉ«"},
	{0xFF8800,L"ÌìÀ¶"},
	{0x80A080,L"»ÒÂÌ"},
	{0xC06000,L"ÇàÀ¶"},
	{0x0080FF,L"³È»Æ"},
	{0x8050FF,L"ÌÒºì"},
	{0xC080FF,L"Ü½ºì"},
	{0x606060,L"Éî»Ò"}
};
#define CLPIDX_CUSTOM 1

constexpr int c_iCPItemPadding = 2;
constexpr int c_cxCPClrBlock = 20;

LRESULT CALLBACK CColorPicker::SubclassProc_Parent(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
	UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	auto pCtx = (PARENTSCCTX*)dwRefData;
	switch (uMsg)
	{
	case WM_DRAWITEM:
	{
		auto pdis = (DRAWITEMSTRUCT*)lParam;
		auto it = m_WndRecord.find(pdis->hwndItem);
		if (it==m_WndRecord.end())
			break;
		auto p = it->second;
		COLORREF cr = c_ColorPickerPresetClr[pdis->itemID].cr;
		HBRUSH hbr;
		HDC hDC = pdis->hDC;
		if (cr == CLR_DEFAULT)
			hbr = CreateHatchBrush(HS_BDIAGONAL, 0x000000);
		else if (cr == CLR_INVALID)
			hbr = CreateSolidBrush(p->m_crCustom);
		else
			hbr = CreateSolidBrush(cr);

		if (IsBitSet(pdis->itemState, ODS_SELECTED))
		{
			FillRect(hDC, &pdis->rcItem, (HBRUSH)GetSysColorBrush(COLOR_HIGHLIGHT));
			SetTextColor(hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
		}
		else
		{
			FillRect(hDC, &pdis->rcItem, (HBRUSH)GetSysColorBrush(COLOR_WINDOW));
			SetTextColor(hDC, GetSysColor(COLOR_WINDOWTEXT));
		}

		int iItemPadding = DpiScale(c_iCPItemPadding, pCtx->iDpi);
		int cxClrBlock = DpiScale(c_cxCPClrBlock, pCtx->iDpi);

		HGDIOBJ hOldBr = SelectObject(hDC, hbr);
		HGDIOBJ hOldPen = SelectObject(hDC, GetStockObject(BLACK_PEN));
		int xClrBlock = pdis->rcItem.left + iItemPadding;
		Rectangle(hDC,
			xClrBlock,
			pdis->rcItem.top + iItemPadding,
			xClrBlock + cxClrBlock,
			pdis->rcItem.bottom - iItemPadding);
		SelectObject(hDC, hOldPen);
		DeleteObject(SelectObject(hDC, hOldBr));
		SetBkMode(hDC, TRANSPARENT);

		RECT rcText = pdis->rcItem;
		rcText.left += (xClrBlock + cxClrBlock + iItemPadding);
		DrawTextW(hDC, c_ColorPickerPresetClr[pdis->itemID].pszName, -1, &rcText,
			DT_NOCLIP | DT_SINGLELINE | DT_VCENTER);
		return TRUE;
	}
	break;

	case WM_MEASUREITEM:
	{
		auto pmis = (MEASUREITEMSTRUCT*)lParam;
		auto it = m_WndRecord.find(GetDlgItem(hWnd, pmis->CtlID));
		if (it == m_WndRecord.end())
			break;
		auto p = it->second;
		pmis->itemHeight = DpiScale(20, pCtx->iDpi);
		return TRUE;
	}
	break;

	case WM_COMMAND:
	{
		if (HIWORD(wParam) != CBN_SELCHANGE)
			break;
		auto it = m_WndRecord.find((HWND)lParam);
		if (it == m_WndRecord.end())
			break;
		auto p = it->second;

		int idxCurrSel = (int)SendMessageW((HWND)lParam, CB_GETCURSEL, 0, 0);
		COLORREF cr;
		if (idxCurrSel == CLPIDX_CUSTOM)
		{
			CHOOSECOLORW cc{ sizeof(CHOOSECOLORW) };
			cc.hwndOwner = hWnd;
			cc.lpCustColors = p->m_crCCDlgCustom;
			cc.Flags = CC_ANYCOLOR | CC_FULLOPEN;
			if (ChooseColorW(&cc))
			{
				p->m_crCustom = cc.rgbResult;
				InvalidateRect((HWND)lParam, NULL, FALSE);
			}
			cr = p->m_crCustom;
		}
		else
			cr = c_ColorPickerPresetClr[idxCurrSel].cr;
		if (p->m_uNotifyMsg)
			DefSubclassProc(hWnd, p->m_uNotifyMsg, LOWORD(wParam), cr);
		return 0;
	}
	break;

	case WM_DPICHANGED:
		pCtx->iDpi = HIWORD(wParam);
		break;

	case WM_DESTROY:
		m_SMRef.DeleteRecord(hWnd);
		break;
	}

	return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK CColorPicker::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CREATE:
	{
		SetPrevDpiProp(hWnd, GetDpi(hWnd));
		auto p = (CColorPicker*)((CREATESTRUCTW*)lParam)->lpCreateParams;
		p->m_hWnd = hWnd;
		m_Recorder.AddRecord(hWnd, p);
	}
	break;

	case WM_DESTROY:
	{
		m_SMRef.Release(m_WndRecord[hWnd]->m_hParent);
		m_Recorder.DeleteRecord(hWnd);
	}
	break;

	case WM_DPICHANGED_AFTERPARENT:
	{
		auto p = m_WndRecord[hWnd];
		int iDpiNew = GetDpi(hWnd);
		int iDpiOld = GetPrevDpiProp(hWnd);
		SendMessageW(hWnd, CB_SETITEMHEIGHT, -1,
			DpiScale((int)SendMessageW(hWnd, CB_GETITEMHEIGHT, 0, 0), iDpiNew, iDpiOld));
		SendMessageW(hWnd, CB_SETITEMHEIGHT, 0,
			DpiScale((int)SendMessageW(hWnd, CB_GETITEMHEIGHT, 0, 0), iDpiNew, iDpiOld));

		SetPrevDpiProp(hWnd, iDpiNew);
	}
	return 0;
	}

	return CallWindowProcW(m_pfnColorPickerDefProc, hWnd, uMsg, wParam, lParam);
}

ATOM CColorPicker::RegisterWndClass(HINSTANCE hInstance)
{
	if (m_atomColorPicker)
		return m_atomColorPicker;
	WNDCLASSEXW wcex{ sizeof(WNDCLASSEXW) };
	GetClassInfoExW(NULL, WC_COMBOBOXW, &wcex);
	m_pfnColorPickerDefProc = wcex.lpfnWndProc;
	wcex.style &= (~CS_GLOBALCLASS);
	wcex.hInstance = hInstance;
	wcex.lpszClassName = WCN_COLORPICKER;
	wcex.lpfnWndProc = CColorPicker::WndProc;
	m_atomColorPicker = RegisterClassExW(&wcex);
	return m_atomColorPicker;
}

HWND CColorPicker::Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
	int x, int y, int cx, int cy, HWND hParent, int nID, PCVOID pData)
{
	dwStyle |= (WS_CHILD | WS_VISIBLE | WS_VSCROLL | CBS_OWNERDRAWFIXED | CBS_DROPDOWNLIST);

	m_hParent = hParent;
	m_ParentCtx.iDpi = GetDpi(hParent);
	m_SMRef.AddRef(hParent, &m_ParentCtx);
	m_hWnd = CreateWindowExW(0, WCN_COLORPICKER, NULL, dwStyle,
		x, y, cx, cy, hParent, i32ToP<HMENU>(nID), NULL, this);

	SetRedraw(FALSE);
	SendMsg(CB_INITSTORAGE, ARRAYSIZE(c_ColorPickerPresetClr), 0);
	for (auto& x : c_ColorPickerPresetClr)
		SendMsg(CB_ADDSTRING, 0, (LPARAM)x.pszName);
	SendMsg(CB_SETCURSEL, 0, 0);
	SetRedraw(TRUE);
	return m_hWnd;
}

COLORREF CColorPicker::GetColor()
{
	int idx = (int)SendMsg(CB_GETCURSEL, 0, 0);
	if (idx == CB_ERR)
		return CLR_INVALID;
	COLORREF cr = c_ColorPickerPresetClr[idx].cr;
	if (cr == CLR_INVALID)
		return m_crCustom;
	else
		return cr;
}

BOOL CColorPicker::SetColor(COLORREF cr)
{
	if (cr == CLR_INVALID)
		return FALSE;
	for (int i = 0; i < ARRAYSIZE(c_ColorPickerPresetClr); ++i)
	{
		if (c_ColorPickerPresetClr[i].cr == cr)
		{
			SendMsg(CB_SETCURSEL, i, 0);
			return TRUE;
		}
	}
	m_crCustom = cr;
	SendMsg(CB_SETCURSEL, CLPIDX_CUSTOM, 0);
	return TRUE;
}
ECK_NAMESPACE_END