#pragma once
#include "CWindow.h"
#include "CMemoryDC.h"
#include "CEasyD2D.h"

#include <dxgi1_2.h>

ECK_NAMESPACE_BEGIN
class CDrawPanel : public CWindow
{
public:
    ECK_RTTI(CDrawPanel, CWindow);
    ECK_CWND_SINGLEOWNER(CDrawPanel);
    ECK_CWND_CREATE_CLS_HINST(WCN_DRAWPANEL, g_hInstance);
private:
    CMemoryDC m_DC{};
    GpGraphics* m_pGraphics{};
    int m_cxClient{},
        m_cyClient{};
    HBRUSH m_hbrBK{};

    void OnSize(HWND hWnd, UINT uState, int cx, int cy) noexcept
    {
        HDC hDC = GetDC(hWnd);

        HBITMAP hBitmap = CreateCompatibleBitmap(hDC, cx, cy);
        HDC hCDC = CreateCompatibleDC(hDC);
        HGDIOBJ hOld = SelectObject(hCDC, hBitmap);
        const RECT rc{ 0,0,cx,cy };
        m_DC.Create(hWnd);
        FillRect(m_DC.GetDC(), &rc, m_hbrBK);
        BitBlt(hCDC, 0, 0, std::min(cx, m_cxClient), std::min(cy, m_cyClient), m_DC.GetDC(), 0, 0, SRCCOPY);

        SelectObject(hCDC, hOld);
        DeleteDC(hCDC);

        SelectObject(m_DC.m_hCDC, m_DC.m_hOld);
        DeleteObject(m_DC.m_hBmp);

        m_DC.m_hOld = SelectObject(m_DC.m_hCDC, hBitmap);
        m_DC.m_hBmp = hBitmap;

        GdipDeleteGraphics(m_pGraphics);
        GdipCreateFromHDC(m_DC.GetDC(), &m_pGraphics);
        GdipSetSmoothingMode(m_pGraphics, Gdiplus::SmoothingModeHighQuality);

        m_cxClient = cx;
        m_cyClient = cy;
    }
public:
    LRESULT OnMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept override
    {
        switch (uMsg)
        {
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            BeginPaint(hWnd, &ps);
            BitBltPs(&ps, m_DC.GetDC());
            EndPaint(hWnd, &ps);
        }
        return 0;
        case WM_SIZE:
            return HANDLE_WM_SIZE(hWnd, wParam, lParam, OnSize);
        case WM_CREATE:
        {
            RECT rc;
            GetClientRect(hWnd, &rc);
            m_DC.Create(hWnd);
            FillRect(m_DC.GetDC(), &rc, m_hbrBK);
            GdipCreateFromHDC(m_DC.GetDC(), &m_pGraphics);
            GdipSetSmoothingMode(m_pGraphics, Gdiplus::SmoothingModeHighQuality);
        }
        return 0;
        }
        return __super::OnMessage(hWnd, uMsg, wParam, lParam);
    }

    EckInlineNdCe HDC GetHDC() const noexcept { return m_DC.GetDC(); }

    EckInlineNdCe GpGraphics* GetGraphics() const noexcept { return m_pGraphics; }

    EckInline void SetBackgroundBrush(HBRUSH hbr) noexcept
    {
        if (m_hbrBK)
            DeleteObject(m_hbrBK);
        m_hbrBK = hbr;
    }
};

class CDrawPanelD2D : public CWindow
{
public:
    ECK_RTTI(CDrawPanelD2D, CWindow);
    ECK_CWND_SINGLEOWNER(CDrawPanelD2D);
    ECK_CWND_CREATE_CLS_HINST(WCN_DRAWPANEL, g_hInstance);
private:
    CEasyD2D m_D2D{};
public:
    LRESULT OnMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept override
    {
        switch (uMsg)
        {
        case WM_PAINT:
            ValidateRect(hWnd, nullptr);
            m_D2D.GetSwapChain()->Present(0, 0);
            return 0;
        case WM_SIZE:
            m_D2D.ReSize(0, LOWORD(lParam), HIWORD(lParam), 0);
            return 0;
        case WM_CREATE:
            m_D2D.Create(EZD2D_PARAM::MakeBitblt(hWnd, g_pDxgiFactory, g_pDxgiDevice, g_pD2DDevice, 0, 0));
            return 0;
        }
        return __super::OnMessage(hWnd, uMsg, wParam, lParam);
    }

    EckInlineNdCe ID2D1DeviceContext* GetDC() const noexcept { return m_D2D.GetDC(); }

    EckInlineNdCe IDXGISwapChain1* GetSwapChain() const noexcept { return m_D2D.GetSwapChain(); }
};
ECK_NAMESPACE_END