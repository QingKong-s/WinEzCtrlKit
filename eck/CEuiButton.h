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
                GetStockBrush(m_bLBtnDown ? BLACK_BRUSH : GRAY_BRUSH));
            FrameRect(hDC, &ps.rcClipInClient,
                GetStockBrush(m_bLBtnDown ? GRAY_BRUSH : BLACK_BRUSH));
            EndPaint(ps);
        }
        return 0;
        case WM_LBUTTONDOWN:
        case WM_LBUTTONDBLCLK:
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
        }
        return __super::OnEvent(uMsg, wParam, lParam);
    }
};
ECK_EUI_NAMESPACE_END
ECK_NAMESPACE_END