#pragma once
#include "CWnd.h"
#include "CComboBox.h"

ECK_NAMESPACE_BEGIN
// 颜色选择器 颜色被改变通知结构
struct NMCLPCLRCHANGED
{
	NMHDR nmhdr;
	COLORREF cr;
};

class CColorPicker :public CComboBox
{
public:
	ECK_RTTI(CColorPicker);

	constexpr static int IdxCustom = 1;
private:
	COLORREF m_crCustom = CLR_INVALID;
	COLORREF m_crCCDlgCustom[16]{};
	int m_iDpi = USER_DEFAULT_SCREEN_DPI;

	constexpr static struct CPPRESETCOLOR
	{
		COLORREF cr;
		PCWSTR pszName;
	}
	c_ColorPickerPresetClr[] =
	{
		{CLR_DEFAULT,L"默认"},
		{CLR_INVALID,L"自定义..."},
		{0x0000FF,L"红色"},
		{0x00FF00,L"绿色"},
		{0xFF0000,L"蓝色"},
		{0x00FFFF,L"黄色"},
		{0xFF00FF,L"品红"},
		{0xFFFF00,L"艳青"},
		{0x000080,L"红褐"},
		{0x008000,L"墨绿"},
		{0x008080,L"褐绿"},
		{0x800000,L"藏青"},
		{0x800080,L"紫红"},
		{0x808000,L"深青"},
		{0xC0C0C0,L"浅灰"},
		{0xC0DCC0,L"美元绿"},
		{0xF0CAA6,L"浅蓝"},
		{0x808080,L"灰色"},
		{0xA4A0A0,L"中性灰"},
		{0xF0FBFF,L"乳白"},
		{0x000000,L"黑色"},
		{0xFFFFFF,L"白色"},
		{0xFF8080,L"蓝灰"},
		{0xE03058,L"藏蓝"},
		{0x00E080,L"嫩绿"},
		{0x80E000,L"青绿"},
		{0x0060C0,L"黄褐"},
		{0xFFA8FF,L"粉红"},
		{0x00D8D8,L"嫩黄"},
		{0xECECEC,L"银白"},
		{0xFF0090,L"紫色"},
		{0xFF8800,L"天蓝"},
		{0x80A080,L"灰绿"},
		{0xC06000,L"青蓝"},
		{0x0080FF,L"橙黄"},
		{0x8050FF,L"桃红"},
		{0xC080FF,L"芙红"},
		{0x606060,L"深灰"}
	};
	constexpr static int c_iCPItemPadding = 2;
	constexpr static int c_cxCPClrBlock = 20;
public:
	LRESULT OnNotifyMsg(HWND hParent, UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bProcessed) override
	{
		switch (uMsg)
		{
		case WM_DRAWITEM:
		{
			bProcessed = TRUE;
			auto pdis = (DRAWITEMSTRUCT*)lParam;
			COLORREF cr = c_ColorPickerPresetClr[pdis->itemID].cr;
			HBRUSH hbr;
			HDC hDC = pdis->hDC;
			if (cr == CLR_DEFAULT)
				hbr = CreateHatchBrush(HS_BDIAGONAL, 0x000000);
			else if (cr == CLR_INVALID)
				hbr = CreateSolidBrush(m_crCustom);
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

			int iItemPadding = DpiScale(c_iCPItemPadding, m_iDpi);
			int cxClrBlock = DpiScale(c_cxCPClrBlock, m_iDpi);

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
		}
		return TRUE;

		case WM_COMMAND:
		{
			if (HIWORD(wParam) != CBN_SELCHANGE || (HWND)lParam != GetHWND())
				break;
			bProcessed = TRUE;
			int idxCurrSel = (int)SendMessageW((HWND)lParam, CB_GETCURSEL, 0, 0);
			COLORREF cr;
			if (idxCurrSel == IdxCustom)
			{
				CHOOSECOLORW cc{ sizeof(CHOOSECOLORW) };
				cc.hwndOwner = hParent;
				cc.lpCustColors = m_crCCDlgCustom;
				cc.Flags = CC_ANYCOLOR | CC_FULLOPEN;
				if (ChooseColorW(&cc))
				{
					m_crCustom = cc.rgbResult;
					InvalidateRect((HWND)lParam, nullptr, FALSE);
				}
				cr = m_crCustom;
			}
			else
				cr = c_ColorPickerPresetClr[idxCurrSel].cr;
			NMCLPCLRCHANGED nm;
			nm.cr = cr;
			FillNmhdrAndSendNotify(nm, NM_CLP_CLRCHANGED);
			return 0;
		}
		break;
		}
		return CWnd::OnNotifyMsg(hParent, uMsg, wParam, lParam, bProcessed);
	}

	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		switch (uMsg)
		{
		case WM_DPICHANGED_AFTERPARENT:
		{
			const int iDpiOld = m_iDpi;
			m_iDpi = GetDpi(hWnd);
			SendMessageW(hWnd, CB_SETITEMHEIGHT, -1,
				DpiScale((int)SendMessageW(hWnd, CB_GETITEMHEIGHT, 0, 0), m_iDpi, iDpiOld));
			SendMessageW(hWnd, CB_SETITEMHEIGHT, 0,
				DpiScale((int)SendMessageW(hWnd, CB_GETITEMHEIGHT, 0, 0), m_iDpi, iDpiOld));
		}
		return 0;
		}

		return CComboBox::OnMsg(hWnd, uMsg, wParam, lParam);
	}
	
	ECK_CWND_CREATE;
	HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, HMENU hMenu, PCVOID pData = nullptr) override
	{
		dwStyle |= (WS_CHILD | WS_VSCROLL | CBS_OWNERDRAWFIXED | CBS_DROPDOWNLIST);
		m_iDpi = GetDpi(hParent);
		m_hWnd = IntCreate(0, WC_COMBOBOXW, nullptr, dwStyle,
			x, y, cx, cy, hParent, hMenu, nullptr, nullptr, nullptr);
		SetRedraw(FALSE);
		InitStorage(ARRAYSIZE(c_ColorPickerPresetClr), 0);
		for (auto& x : c_ColorPickerPresetClr)
			AddString(x.pszName);
		SetCurSel(0);
		SetRedraw(TRUE);
		return m_hWnd;
	}

	COLORREF GetColor()
	{
		int idx = GetCurSel();
		if (idx == CB_ERR)
			return CLR_INVALID;
		COLORREF cr = c_ColorPickerPresetClr[idx].cr;
		if (cr == CLR_INVALID)
			return m_crCustom;
		else
			return cr;
	}

	BOOL SetColor(COLORREF cr)
	{
		if (cr == CLR_INVALID)
			return FALSE;
		for (int i = 0; i < ARRAYSIZE(c_ColorPickerPresetClr); ++i)
		{
			if (c_ColorPickerPresetClr[i].cr == cr)
			{
				SetCurSel(i);
				return TRUE;
			}
		}
		m_crCustom = cr;
		SetCurSel(1);
		return TRUE;
	}

	EckInline void SelectColor(int idx)
	{
		SendMsg(CB_SETCURSEL, idx, 0);
	}
};
ECK_RTTI_IMPL_BASE_INLINE(CColorPicker, CComboBox);
ECK_NAMESPACE_END