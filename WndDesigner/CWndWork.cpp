#include "pch.h"
#include "CWndMain.h"
#include "Utils.h"

void CWndWork::SelBeginDragging(POINT pt)
{
    m_ptSelStart = PtAlign(pt, 8);
    m_vRect.resize(1);
    m_vRect.front() = eck::MakeRect(pt, pt);
}

void CWndWork::SelDraggingMove(POINT pt)
{
    m_vRect.front() = eck::MakeRect(m_ptSelStart, PtAlign(pt, 8));
    Host()->SrvDrawOverlayRect(HWnd, m_vRect.data(), 1);
}

void CWndWork::SelCancel()
{
    Host()->SrvClearOverlayRect();
}

LRESULT CWndWork::OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        BeginPaint(hWnd, &ps);
        SetDCBrushColor(ps.hdc, eck::GetThreadCtx()->crDefBkg);
        FillRect(ps.hdc, &ps.rcPaint, GetStockBrush(DC_BRUSH));
        //DrawGridPoint(ps.hdc, ps.rcPaint, 8, eck::GetThreadCtx()->crDefText);
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
            SelCancel();
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
    }
    return __super::OnMsg(hWnd, uMsg, wParam, lParam);
}