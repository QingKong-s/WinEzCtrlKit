/*
* WinEzCtrlKit Library
*
* CIPEditExt.h ： IP编辑框扩展
*
* Copyright(C) 2024 QingKong
*/
#pragma once
#include "CIPEdit.h"

ECK_NAMESPACE_BEGIN
class CIPEditExt : public CIPEdit
{
public:
	ECK_RTTI(CIPEditExt);
private:
	HFONT m_hFont{};
	int m_cxEdit{};
	int m_cxDot{};
	HWND m_hEdit[4]{};

	void UpdateEditMetrics(int cxClient)
	{
		const auto hDC = GetDC(HWnd);
		const auto hOld = SelectObject(hDC, m_hFont);
		GetCharWidth32W(hDC, L'.', L'.', &m_cxDot);
		SelectObject(hDC, hOld);
		ReleaseDC(HWnd, hDC);
		m_cxEdit = (cxClient - 3 - m_cxDot * 3) / 4;
	}
public:
	void AttachNew(HWND hWnd) override
	{
		__super::AttachNew(hWnd);
		UpdateEditMetrics();
	}

	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		switch (uMsg)
		{
		case WM_PAINT:
		case WM_PRINTCLIENT:
		{
			if (!ShouldAppsUseDarkMode())
				break;
			PAINTSTRUCT ps;
			BeginPaint(hWnd, wParam, ps);
			const auto* const ptc = GetThreadCtx();
			SetDCBrushColor(ps.hdc, ptc->crDefBkg);
			FillRect(ps.hdc, &ps.rcPaint, (HBRUSH)GetStockObject(DC_BRUSH));

			SetBkMode(ps.hdc, TRANSPARENT);
			SetTextColor(ps.hdc, ptc->crDefText);

			const auto hOld = SelectObject(ps.hdc, m_hFont);
			int x{ 3 + m_cxEdit };
			EckCounterNV(3)
			{
				TextOutW(ps.hdc, x, 1, L".", 1);
				x += (m_cxEdit + m_cxDot);
			}
			SelectObject(ps.hdc, hOld);

			EndPaint(hWnd, wParam, ps);
		}
		return 0;

		case WM_CTLCOLOREDIT:
		{
			const auto* const ptc = GetThreadCtx();
			SetTextColor((HDC)wParam, ptc->crDefText);
			SetBkMode((HDC)wParam, TRANSPARENT);
			SetDCBrushColor((HDC)wParam, ptc->crDefBkg);
			return (LRESULT)GetStockObject(DC_BRUSH);
		}
		break;

		// 大小变化时并不会改变编辑框尺寸。。。
		case WM_SIZE:
		{
			const auto lResult = __super::OnMsg(hWnd, uMsg, wParam, lParam);
			UpdateEditMetrics(LOWORD(lParam));
			int x{ 3 };
			EckCounter(4, i)
			{
				SetWindowPos(m_hEdit[i], nullptr,
					x, 1, m_cxEdit, HIWORD(lParam), SWP_NOZORDER | SWP_NOACTIVATE);
				x += (m_cxEdit + m_cxDot);
			}
			return lResult;
		}
		break;

		case WM_SETFONT:
		{
			const auto lResult = __super::OnMsg(hWnd, uMsg, wParam, lParam);
			m_hFont = (HFONT)wParam;
			RECT rc;
			GetClientRect(hWnd, &rc);
			UpdateEditMetrics(rc.right - rc.left);
			return lResult;
		}
		break;

		// IP编辑框忽略此消息
		case WM_GETFONT: return (LRESULT)m_hFont;

		case WM_CREATE:
		{
			const auto lResult = __super::OnMsg(hWnd, uMsg, wParam, lParam);
			HWND hEdit{};
			EckCounter(4, i)
			{
				hEdit = FindWindowExW(hWnd, hEdit, L"Edit", nullptr);
				m_hEdit[i] = hEdit;
			}
			return lResult;
		}
		}
		return __super::OnMsg(hWnd, uMsg, wParam, lParam);
	}

	void UpdateEditMetrics()
	{
		RECT rc;
		GetClientRect(HWnd, &rc);
		UpdateEditMetrics(rc.right - rc.left);
	}

	EckInline constexpr HWND GetEdit(int i) const { return m_hEdit[i]; }
};
ECK_RTTI_IMPL_BASE_INLINE(CIPEditExt, CIPEdit);
ECK_NAMESPACE_END