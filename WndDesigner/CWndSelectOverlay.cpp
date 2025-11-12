#include "pch.h"
#include "CWndSelectOverlay.h"

void CWndSelectOverlay::ReCreatePen(COLORREF cr) noexcept
{
    DeleteObject(m_hPen);
    m_hPen = CreatePen(PS_SOLID, eck::DpiScale(2, eck::GetDpi(HWnd)), cr);
}

LRESULT CWndSelectOverlay::OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_NCHITTEST:
        return HTTRANSPARENT;
    case WM_ERASEBKGND:
        return TRUE;

    case WM_PAINT:
    {
        if (m_vRect.empty())
            ValidateRect(hWnd, nullptr);
        else
        {
            PAINTSTRUCT ps;
            BeginPaint(hWnd, &ps);
            SetROP2(ps.hdc, R2_MERGEPENNOT);
            const auto hOld = SelectObject(ps.hdc, m_hPen);
            SelectObject(ps.hdc, GetStockObject(NULL_BRUSH));
            for (const auto& e : m_vRect)
                Rectangle(ps.hdc, e.left, e.top, e.right, e.bottom);
            SelectObject(ps.hdc, hOld);
            EndPaint(hWnd, &ps);
        }
    }
    return 0;

    case WM_WINDOWPOSCHANGING:
    {
        const auto pwp = (WINDOWPOS*)lParam;
        pwp->flags |= SWP_NOCOPYBITS;
    }
    break;

    case WM_DPICHANGED_AFTERPARENT:
    case WM_CREATE:
        ReCreatePen();
        break;

    case WM_DESTROY:
        DeleteObject(m_hPen);
        break;
    }

    return __super::OnMsg(hWnd, uMsg, wParam, lParam);
}

void CWndSelectOverlay::SetRect(HWND hWndRef, const RECT* prc, size_t cRc) noexcept
{
    m_vRect.assign(prc, prc + cRc);
    MapWindowPoints(hWndRef, HWnd, (POINT*)m_vRect.data(), cRc * 2);
}
void CWndSelectOverlay::ClearRect() noexcept
{
    m_vRect.clear();
}