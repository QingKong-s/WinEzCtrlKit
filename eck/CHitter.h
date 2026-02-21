#pragma once
#include "CWindow.h"

ECK_NAMESPACE_BEGIN
struct NMHTTSEL
{
    NMHDR nmhdr;
    POINT pt;
};

class CHitter : public CWindow
{
public:
    ECK_RTTI(CHitter, CWindow);
    ECK_CWND_SINGLEOWNER(CHitter);
    ECK_CWND_CREATE_CLS_HINST(WCN_HITTER, g_hInstance);
private:
    HCURSOR m_hcNormal{};
    HCURSOR m_hcHit{};
    HCURSOR m_hcDef{};

    BITBOOL m_bCaptured : 1 = FALSE;
public:
    LRESULT OnMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept override
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
            SetDCBrushColor(ps.hdc, PtcCurrent()->crDefBkg);
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
        return __super::OnMessage(hWnd, uMsg, wParam, lParam);
    }

    EckInlineCe void SetNormalCursor(HCURSOR hc) noexcept { m_hcNormal = hc; }
    EckInlineNdCe HCURSOR GetNormalCursor() const noexcept { return m_hcNormal; }

    EckInlineCe void SetHitCursor(HCURSOR hc) noexcept { m_hcHit = hc; }
    EckInlineNdCe HCURSOR GetHitCursor() const noexcept { return m_hcHit; }
};
ECK_NAMESPACE_END