/*
* WinEzCtrlKit Library
*
* CMsgBoxHook.h ： 信息框暗色修复。
* 一般不手动使用此类，将在SoftModalMessageBox的挂钩中自动调用
*
* Copyright(C) 2024 QingKong
*/
#pragma once
#include "CWnd.h"

ECK_NAMESPACE_BEGIN
class CMsgBoxHook : public CWnd
{
private:
	RECT m_rcMainPanel{};
	RECT m_rcCommandEdge{};
	HICON m_hIcon{};
public:
	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		switch (uMsg)
		{
		case WM_PAINT:
		{
			if (!ShouldAppsUseDarkMode())
				break;
			const auto* const ptc = GetThreadCtx();
			PAINTSTRUCT ps;
			BeginPaint(hWnd, &ps);
			SetDCBrushColor(ps.hdc, ptc->crDefBkg);
			FillRect(ps.hdc, &m_rcMainPanel, GetStockBrush(DC_BRUSH));
			SetDCBrushColor(ps.hdc, ptc->crDefBtnFace);
			FillRect(ps.hdc, &m_rcCommandEdge, GetStockBrush(DC_BRUSH));
			EndPaint(hWnd, &ps);
		}
		return 0;

		case WM_INITDIALOG:
		{
			// Call默认过程，先执行初始化
			const auto lResult = __super::OnMsg(hWnd, uMsg, wParam, lParam);
			const auto hStatic = GetDlgItem(hWnd, 0xFFFF);
			const auto hStaticIcon = GetDlgItem(hWnd, 0x14);
			m_hIcon = (HICON)SendMessageW(hStaticIcon, STM_GETICON, 0, 0);
			// OD修正图标白底
			SetWindowLongPtrW(hStaticIcon, GWL_STYLE,
				(GetWindowLongPtrW(hStaticIcon, GWL_STYLE) & ~SS_ICON) | SS_OWNERDRAW);
			// 内部测高机制过于复杂，这里使用静态控件的高度
			RECT rcTemp;
			GetClientRect(hStatic, &rcTemp);
			const auto cyText = rcTemp.bottom;
			GetClientRect(hStaticIcon, &rcTemp);
			const auto cyIcon = rcTemp.bottom;
			// 其字符高度来源为GdiGetCharDimensions(Ex)，
			// 该函数简单地将字符高度设为tmHeight
			const auto hCDC = CreateCompatibleDC(nullptr);
			SelectObject(hCDC, (HGDIOBJ)SendMessageW(hStatic, WM_GETFONT, 0, 0));
			TEXTMETRICW tm;
			GetTextMetricsW(hCDC, &tm);
			DeleteDC(hCDC);
			// User32的计算方法，不应仿照其底边计算方式，
			// 因为其得出的数值不准确，实际上超过了客户区高度
			const int cyTextMargin = (14 * tm.tmHeight + 4) >> 3;
			GetClientRect(hWnd, &m_rcMainPanel);
			m_rcCommandEdge = m_rcMainPanel;
			m_rcMainPanel.bottom = std::max(cyText, cyIcon) + cyTextMargin * 2;
			m_rcCommandEdge.top = m_rcMainPanel.bottom;

			EnableWindowNcDarkMode(hWnd, ShouldAppsUseDarkMode());
			return lResult;
		}
		break;

		case WM_CTLCOLORBTN:
			if (ShouldAppsUseDarkMode())
			{
				const auto* const ptc = GetThreadCtx();
				SetDCBrushColor((HDC)wParam, ptc->crDefBtnFace);
				return (LRESULT)GetStockBrush(DC_BRUSH);
			}
			break;
		case WM_CTLCOLORSTATIC:
			if (ShouldAppsUseDarkMode())
			{
				const auto* const ptc = GetThreadCtx();
				SetTextColor((HDC)wParam, ptc->crDefText);
				SetBkMode((HDC)wParam, TRANSPARENT);
				SetBkColor((HDC)wParam, ptc->crDefBkg);
				SetDCBrushColor((HDC)wParam, ptc->crDefBkg);
				return (LRESULT)GetStockBrush(DC_BRUSH);
			}
			break;
		case WM_DRAWITEM:
		{
			const auto* const ptc = GetThreadCtx();
			const auto* const pdis = (DRAWITEMSTRUCT*)lParam;
			if (pdis->CtlType == ODT_STATIC && pdis->CtlID == 0x14)
			{
				DrawIconEx(pdis->hDC, pdis->rcItem.left, pdis->rcItem.top, m_hIcon,
					0, 0, 0, nullptr, DI_NORMAL);
				return TRUE;
			}
		}
		break;
		}
		return __super::OnMsg(hWnd, uMsg, wParam, lParam);
	}
};
ECK_NAMESPACE_END