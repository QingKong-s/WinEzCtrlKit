#pragma once
#include "EuiBase.h"

ECK_NAMESPACE_BEGIN
ECK_EUI_NAMESPACE_BEGIN
class CButton : public CElement
{
private:
    BOOL m_bLBtnDown{};
public:
    LRESULT OnEvent(UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept override
    {
        switch (uMsg)
        {
        case WM_PAINT:
        {
            PAINTINFO ps;
            BeginPaint(ps, wParam, lParam);
            const auto hDC = GetDC();

            FillRect(hDC, &ps.rcClipInClient,
                GetStockBrush(m_bLBtnDown ? BLACK_BRUSH : WHITE_BRUSH));
            auto rc{ GetRectInClient() };
            FrameRect(hDC, &rc,
                GetStockBrush(m_bLBtnDown ? WHITE_BRUSH : BLACK_BRUSH));

            DrawTextW(hDC, GetText().Data(), GetText().Size(), (RECT*)&rc,
                DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_NOCLIP);

            if (GetFocus() == this && IsShowingFocus())
            {
                InflateRect(rc, -4, -4);
                FrameRect(hDC, &rc,
                    GetStockBrush(m_bLBtnDown ? WHITE_BRUSH : BLACK_BRUSH));
                //DrawFocusRect(hDC, &rc);
            }

            EndPaint(ps);
        }
        return 0;
        case WM_LBUTTONDOWN:
        case WM_LBUTTONDBLCLK:
            SetFocus();
            SetCapture();
            m_bLBtnDown = TRUE;
            Redraw();
            break;
        case WM_LBUTTONUP:
            ReleaseCapture();
            break;
        case WM_CAPTURECHANGED:
            m_bLBtnDown = FALSE;
            Redraw();
            return 0;
        case WM_SETFOCUS:
        case WM_KILLFOCUS:
            if (IsShowingFocus())
                Redraw();
            break;
        case EWM_SHOWFOCUS:
            Redraw();
            break;
        }
        return __super::OnEvent(uMsg, wParam, lParam);
    }
};
ECK_EUI_NAMESPACE_END
ECK_NAMESPACE_END