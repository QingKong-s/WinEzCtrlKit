#pragma once
#include "ECK.h"

ECK_NAMESPACE_BEGIN
struct CMemoryDC
{
    HDC m_hCDC{};
    HBITMAP m_hBmp{};
    HGDIOBJ m_hOld{};

    CMemoryDC() = default;
    CMemoryDC(const CMemoryDC&) = delete;
    CMemoryDC(CMemoryDC&& x) noexcept : m_hCDC{ x.m_hCDC }, m_hBmp{ x.m_hBmp }, m_hOld{ x.m_hOld }
    {
        x.m_hCDC = nullptr;
        x.m_hBmp = nullptr;
        x.m_hOld = nullptr;
    }

    CMemoryDC(HWND hWnd, int cx = 0, int cy = 0) noexcept
    {
        Create(hWnd, cx, cy);
    }

    ~CMemoryDC()
    {
        Destroy();
    }

    CMemoryDC& operator=(const CMemoryDC&) = delete;
    CMemoryDC& operator=(CMemoryDC&& x) noexcept
    {
        std::swap(m_hCDC, x.m_hCDC);
        std::swap(m_hBmp, x.m_hBmp);
        std::swap(m_hOld, x.m_hOld);
        return *this;
    }

    HDC CreateFromDC(HDC hDC, int cx, int cy) noexcept
    {
        Destroy();
        m_hCDC = CreateCompatibleDC(hDC);
        m_hBmp = CreateCompatibleBitmap(hDC, cx, cy);
        m_hOld = SelectObject(m_hCDC, m_hBmp);
        return m_hCDC;
    }

    HDC Create(HWND hWnd, int cx = -1, int cy = -1) noexcept
    {
        HDC hDC = ::GetDC(hWnd);
        if (cx < 0 || cy < 0)
        {
            RECT rc;
            GetClientRect(hWnd, &rc);
            cx = rc.right;
            cy = rc.bottom;
        }
        CreateFromDC(hDC, cx, cy);
        ReleaseDC(hWnd, hDC);
        return m_hCDC;
    }

    HDC CreateFromDC32(HDC hDC, int cx = 0, int cy = 0,
        _Outptr_opt_ void** ppPixel = nullptr) noexcept
    {
        Destroy();
        m_hCDC = CreateCompatibleDC(hDC);
        BITMAPINFO bmi{};
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = cx;
        bmi.bmiHeader.biHeight = -cy;
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;
        m_hBmp = CreateDIBSection(hDC, &bmi, DIB_RGB_COLORS, ppPixel, nullptr, 0);
        m_hOld = SelectObject(m_hCDC, m_hBmp);
        return m_hCDC;
    }

    HDC Create32(HWND hWnd, int cx = 0, int cy = 0) noexcept
    {
        Destroy();
        HDC hDC = ::GetDC(hWnd);
        if (cx < 0 || cy < 0)
        {
            RECT rc;
            GetClientRect(hWnd, &rc);
            cx = rc.right;
            cy = rc.bottom;
        }
        CreateFromDC32(hDC, cx, cy);
        ReleaseDC(hWnd, hDC);
        return m_hCDC;
    }

    void ReSize(HWND hWnd, int cx = 0, int cy = 0) noexcept
    {
        EckAssert(m_hCDC);
        if (cx < 0 || cy < 0)
        {
            RECT rc;
            GetClientRect(hWnd, &rc);
            cx = rc.right;
            cy = rc.bottom;
        }

        HDC hDC = ::GetDC(hWnd);
        SelectObject(m_hCDC, m_hOld);
        DeleteObject(m_hBmp);

        m_hBmp = CreateCompatibleBitmap(hDC, cx, cy);
        m_hOld = SelectObject(m_hCDC, m_hBmp);
        ReleaseDC(hWnd, hDC);
    }

    void ReSize32(HWND hWnd, int cx = 0, int cy = 0,
        _Outptr_opt_ void** ppPixel = nullptr) noexcept
    {
        EckAssert(!!m_hCDC && !!m_hBmp && !!m_hOld);
        if (cx < 0 || cy < 0)
        {
            RECT rc;
            GetClientRect(hWnd, &rc);
            cx = rc.right;
            cy = rc.bottom;
        }

        HDC hDC = ::GetDC(hWnd);
        SelectObject(m_hCDC, m_hOld);
        DeleteObject(m_hBmp);

        BITMAPINFO bmi{};
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = cx;
        bmi.bmiHeader.biHeight = -cy;
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;
        m_hBmp = CreateDIBSection(hDC, &bmi, DIB_RGB_COLORS, ppPixel, nullptr, 0);
        m_hOld = SelectObject(m_hCDC, m_hBmp);
        ReleaseDC(hWnd, hDC);
    }

    EckInlineNdCe HDC GetDC() const noexcept { return m_hCDC; }

    EckInlineNdCe HBITMAP GetBitmap() const noexcept { return m_hBmp; }

    void Destroy() noexcept
    {
        if (m_hCDC)
        {
            SelectObject(m_hCDC, m_hOld);
            DeleteObject(m_hBmp);
            DeleteDC(m_hCDC);
            m_hCDC = nullptr;
            m_hBmp = nullptr;
            m_hOld = nullptr;
        }
    }
};
ECK_NAMESPACE_END