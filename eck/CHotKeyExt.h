/*
* WinEzCtrlKit Library
*
* CHotKeyExt.h ： 热键框扩展
*
* Copyright(C) 2023-2024 QingKong
*/
#pragma once
#include "CHotKey.h"
#include "Utility2.h"

ECK_NAMESPACE_BEGIN
class CHotKeyExt : public CHotKey
{
public:
	ECK_RTTI(CHotKeyExt);

	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		switch (uMsg)
		{
		case WM_PAINT:
		case WM_PRINTCLIENT:
		{
			if (!ShouldAppsUseDarkMode())
				break;
			HideCaret(hWnd);
			PAINTSTRUCT ps;
			BeginPaint(hWnd, wParam, ps);

			const auto svPlus = GetResourceStringForCurrLocale(0x0401, g_hModComCtl32);
			UINT uFunc;
			const auto vk = GetHotKey(uFunc);

			WCHAR szBuf[64];
			int pos;
			if (uFunc || vk)
			{
				if (uFunc & HOTKEYF_CONTROL)
				{
					pos = GetKeyNameTextByVk(VK_CONTROL, szBuf, ARRAYSIZE(szBuf));
					wcscat(szBuf, svPlus.data());
					pos += (int)svPlus.size();
				}
				else
				{
					pos = 0;
					szBuf[0] = 0;
				}
				if (uFunc & HOTKEYF_SHIFT)
				{
					pos += GetKeyNameTextByVk(VK_SHIFT, szBuf + pos, ARRAYSIZE(szBuf) - pos);
					wcscat(szBuf + pos, svPlus.data());
					pos += (int)svPlus.size();
				}
				if (uFunc & HOTKEYF_ALT)
				{
					pos += GetKeyNameTextByVk(VK_MENU, szBuf + pos, ARRAYSIZE(szBuf) - pos);
					wcscat(szBuf + pos, svPlus.data());
					pos += (int)svPlus.size();
				}
				pos += GetKeyNameTextByVk(vk, szBuf + pos, ARRAYSIZE(szBuf) - pos,
					(uFunc & HOTKEYF_EXT));
			}
			else
			{
				const auto svNone = GetResourceStringForCurrLocale(0x0402, g_hModComCtl32);
				wcscpy(szBuf, svNone.data());
				pos = (int)svNone.size();
			}

			const auto* const ptc = GetThreadCtx();
			const auto iDpi = GetDpi(hWnd);
			const auto cxBorder = DaGetSystemMetrics(SM_CXBORDER, iDpi);
			const auto cyBorder = DaGetSystemMetrics(SM_CYBORDER, iDpi);

			SetDCBrushColor(ps.hdc, ptc->crDefBkg);
			FillRect(ps.hdc, &ps.rcPaint, GetStockBrush(DC_BRUSH));
			const auto hOld = SelectObject(ps.hdc, GetFont());
			SetBkMode(ps.hdc, TRANSPARENT);
			if (IsWindowEnabled(hWnd))
				SetTextColor(ps.hdc, ptc->crDefText);
			else
				SetTextColor(ps.hdc, ptc->crGray1);
			TextOutW(ps.hdc, cxBorder, cyBorder, szBuf, pos);

			SIZE size;
			GetTextExtentPoint32W(ps.hdc, szBuf, pos, &size);

			if (GetFocus() == hWnd)
				SetCaretPos(size.cx + cxBorder, cyBorder);
			ShowCaret(hWnd);

			EndPaint(hWnd, wParam, ps);
		}
		return 0;
		}
		return __super::OnMsg(hWnd, uMsg, wParam, lParam);
	}
};
ECK_RTTI_IMPL_BASE_INLINE(CHotKeyExt, CHotKey);
ECK_NAMESPACE_END