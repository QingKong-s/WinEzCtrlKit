#include "pch.h"
#include "CWndMain.h"
#include "Utils.h"

void CWndWork::SelBeginDragging(POINT pt)
{
    m_ptSelStart = m_bAlignToGrid ? PtAlign(pt, 8) : pt;
    m_vRect.resize(1);
    m_vRect.front() = eck::MakeRect(m_ptSelStart, m_ptSelStart);
    Host()->SrvDrawOverlayRect(HWnd, m_vRect.data(), 1);
}

void CWndWork::SelDraggingMove(POINT pt)
{
    m_vRect.front() = eck::MakeRect(m_ptSelStart,
        m_bAlignToGrid ? PtAlign(pt, 8) : pt);
    Host()->SrvDrawOverlayRect(HWnd, m_vRect.data(), 1);
}

void CWndWork::SelCancel()
{
    Host()->SrvClearOverlayRect();
}

LRESULT CWndWork::OnMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        BeginPaint(hWnd, &ps);
        SetDCBrushColor(ps.hdc, eck::PtcCurrent()->crDefBkg);
        FillRect(ps.hdc, &ps.rcPaint, GetStockBrush(DC_BRUSH));
        //DrawGridPoint(ps.hdc, ps.rcPaint, 8, eck::PtcCurrent()->crDefText);
        EndPaint(hWnd, &ps);
    }
    return 0;

    case WM_LBUTTONDOWN:
    {
        m_bLBtnDown = TRUE;
        SetCapture(hWnd);
        SelBeginDragging(ECK_GET_PT_LPARAM(lParam));
    }
    return 0;
    case WM_MOUSEMOVE:
    {
        if (m_bLBtnDown)
            SelDraggingMove(ECK_GET_PT_LPARAM(lParam));
    }
    return 0;
    case WM_LBUTTONUP:
    {
        if (m_bLBtnDown)
        {
            m_bLBtnDown = FALSE;
            ReleaseCapture();
            Host()->SrvEndSelect();
        }
    }
    return 0;

    case WM_CAPTURECHANGED:
    {
        m_bLBtnDown = FALSE;
    }
    break;

    case WM_CLOSE:
        return 0;
    case WM_SYSCOMMAND:
    {
        switch (wParam)
        {
        case SC_CLOSE:
        case SC_CONTEXTHELP:
        case SC_MAXIMIZE:
        case SC_MINIMIZE:
            return 0;
        }
    }
    break;

    case WM_WINDOWPOSCHANGING:
    {
        const auto pwp = (WINDOWPOS*)lParam;
        pwp->flags |= SWP_NOZORDER;
    }
    break;
    case WM_WINDOWPOSCHANGED:
    {
        const auto pwp = (WINDOWPOS*)lParam;
        if (!(pwp->flags & SWP_NOMOVE))
            Host()->SrvWorkWindowMoved(hWnd);
    }
    break;
    }
    return __super::OnMessage(hWnd, uMsg, wParam, lParam);
}