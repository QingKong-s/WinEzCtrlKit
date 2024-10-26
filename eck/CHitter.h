/*
* WinEzCtrlKit Library
*
* CHitter.h £º ÆÁÄ»×ø±êÑ¡È¡Æ÷
*
* Copyright(C) 2024 QingKong
*/
#pragma once
#include "CWnd.h"

ECK_NAMESPACE_BEGIN
struct NMHTTSEL
{
	NMHDR nmhdr;
	POINT pt;
};

class CHitter : public CWnd
{
public:
	ECK_RTTI(CHitter);
	ECK_CWND_SINGLEOWNER(CHitter);
	ECK_CWND_CREATE_CLS_HINST(WCN_HITTER, g_hInstance);
private:
	HCURSOR m_hcNormal{};
	HCURSOR m_hcHit{};
	HCURSOR m_hcDef{};

	BITBOOL m_bCaptured : 1 = FALSE;
public:
	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		switch (uMsg)
		{
		case WM_LBUTTONDOWN:
			m_bCaptured = TRUE;
			Redraw();
			UpdateWindow(hWnd);
			SetCapture(hWnd);
			SetCursor(m_hcHit ? m_hcHit : m_hcDef);
			return 0;

		case WM_CAPTURECHANGED:
		case WM_LBUTTONUP:
			if (m_bCaptured)
			{
				m_bCaptured = FALSE;
				Redraw();
				UpdateWindow(hWnd);
				ReleaseCapture();
				NMHTTSEL nm;
				GetCursorPos(&nm.pt);
				SetCursor(nullptr);
				FillNmhdrAndSendNotify(nm, NM_HTT_SEL);
			}
			return 0;

		case WM_SETCURSOR:
			if (m_bCaptured)
				SetCursor(m_hcHit ? m_hcHit : m_hcDef);
			else
				SetCursor(m_hcNormal ? m_hcNormal : m_hcDef);
			return TRUE;

		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint(hWnd, &ps);
			SetDCBrushColor(ps.hdc, GetThreadCtx()->crDefBkg);
			FillRect(ps.hdc, &ps.rcPaint, GetStockBrush(DC_BRUSH));
			if (!m_bCaptured)
				DrawIcon(ps.hdc, 0, 0, (HICON)(m_hcNormal ? m_hcNormal : m_hcDef));
			EndPaint(hWnd, &ps);
		}
		return 0;

		case WM_DPICHANGED_BEFOREPARENT:
		case WM_CREATE:
			m_hcDef = LoadCursorW(nullptr, IDC_CROSS);
			break;
		}
		return CWnd::OnMsg(hWnd, uMsg, wParam, lParam);
	}

	EckInline constexpr void SetNormalCursor(HCURSOR hc) { m_hcNormal = hc; }

	EckInline constexpr HCURSOR GetNormalCursor() const { return m_hcNormal; }

	EckInline constexpr void SetHitCursor(HCURSOR hc) { m_hcHit = hc; }

	EckInline constexpr HCURSOR GetHitCursor() const { return m_hcHit; }
};
ECK_RTTI_IMPL_BASE_INLINE(CHitter, CWnd);
ECK_NAMESPACE_END