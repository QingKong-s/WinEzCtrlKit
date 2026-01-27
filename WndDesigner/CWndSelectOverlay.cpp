#include "pch.h"
#include "CWndSelectOverlay.h"

void CWndSelectOverlay::ReCreatePen(COLORREF cr) noexcept
{
    DeleteObject(m_hPen);
    m_hPen = CreatePen(PS_SOLID, eck::DpiScale(1, eck::GetDpi(HWnd)), cr);
}

void CWndSelectOverlay::DrawDragBlock(HDC hDC) noexcept
{
    const auto cxyBlock = 8;
    SetDCBrushColor(hDC, GetSysColor(COLOR_HIGHLIGHT));
    SelectObject(hDC, GetStockObject(DC_BRUSH));
    SetDCPenColor(hDC, eck::PtcCurrent()->crDefText);
    SelectObject(hDC, GetStockObject(DC_PEN));
    int x, y;
    for (const auto& e : m_vRect)
    {
        x = e.left - cxyBlock;
        y = e.top - cxyBlock;
        Rectangle(hDC, x, y, x + cxyBlock, y + cxyBlock);
        x = e.left + (e.right - e.left - cxyBlock) / 2;
        Rectangle(hDC, x, y, x + cxyBlock, y + cxyBlock);
        x = e.right;
        Rectangle(hDC, x, y, x + cxyBlock, y + cxyBlock);

        x = e.left - cxyBlock;
        y = e.top + (e.bottom - e.top - cxyBlock) / 2;
        Rectangle(hDC, x, y, x + cxyBlock, y + cxyBlock);
        x = e.right;
        Rectangle(hDC, x, y, x + cxyBlock, y + cxyBlock);

        x = e.left - cxyBlock;
        y = e.bottom;
        Rectangle(hDC, x, y, x + cxyBlock, y + cxyBlock);
        x = e.left + (e.right - e.left - cxyBlock) / 2;
        Rectangle(hDC, x, y, x + cxyBlock, y + cxyBlock);
        x = e.right;
        Rectangle(hDC, x, y, x + cxyBlock, y + cxyBlock);
    }
}

LRESULT CWndSelectOverlay::OnMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
            const auto iRop2Old = SetROP2(ps.hdc, R2_MERGEPENNOT);
            const auto hOld = SelectObject(ps.hdc, m_hPen);
            SelectObject(ps.hdc, GetStockObject(NULL_BRUSH));
            for (const auto& e : m_vRect)
                Rectangle(ps.hdc, e.left + m_ox, e.top + m_oy,
                    e.right + m_ox, e.bottom + m_oy);
            SelectObject(ps.hdc, hOld);
            SetROP2(ps.hdc, iRop2Old);
            DrawDragBlock(ps.hdc);
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

    return __super::OnMessage(hWnd, uMsg, wParam, lParam);
}

void CWndSelectOverlay::SetRect(HWND hWndRef, const RECT* prc, size_t cRc) noexcept
{
    m_vRectInWorkWnd.assign(prc, prc + cRc);
    m_vRect.assign(prc, prc + cRc);
    MapWindowPoints(hWndRef, HWnd, (POINT*)m_vRect.data(), UINT(cRc * 2));
}
void CWndSelectOverlay::ClearRect() noexcept
{
    m_vRect.clear();
    m_vRectInWorkWnd.clear();
}

void CWndSelectOverlay::EndAddRect(HWND hWndRef) noexcept
{
    m_vRect.assign(m_vRectInWorkWnd.begin(), m_vRectInWorkWnd.end());
    MapWindowPoints(hWndRef, HWnd, (POINT*)m_vRect.data(), UINT(m_vRect.size() * 2));
}