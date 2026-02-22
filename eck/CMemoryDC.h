#pragma once
#include "ECK.h"

ECK_NAMESPACE_BEGIN
class CMemoryDC
{
private:
    HDC m_hCDC{};
    HBITMAP m_hBmp{};

    static HBITMAP CreateDib(
        HDC hDC, int cx, int cy,
        _Outptr_opt_ void** ppPixel) noexcept
    {
        BITMAPINFO bmi{};
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = cx;
        bmi.bmiHeader.biHeight = -cy;
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;
        void* Dummy{};
        return CreateDIBSection(hDC, &bmi, DIB_RGB_COLORS,
            ppPixel ? ppPixel : &Dummy, nullptr, 0);
    }
public:
    CMemoryDC() = default;

    CMemoryDC(const CMemoryDC&) = delete;
    CMemoryDC& operator=(const CMemoryDC&) = delete;

    CMemoryDC(CMemoryDC&& x) noexcept
    {
        std::swap(m_hCDC, x.m_hCDC);
        std::swap(m_hBmp, x.m_hBmp);
    }
    CMemoryDC& operator=(CMemoryDC&& x) noexcept
    {
        std::swap(m_hCDC, x.m_hCDC);
        std::swap(m_hBmp, x.m_hBmp);
        return *this;
    }

    CMemoryDC(HWND hWnd, int cx = 0, int cy = 0) noexcept
    {
        FromWindow(hWnd, cx, cy);
    }

    ~CMemoryDC()
    {
        Destroy();
    }

    HDC FromDC(HDC hDC, int cx, int cy) noexcept
    {
        Destroy();
        m_hCDC = CreateCompatibleDC(hDC);
        m_hBmp = CreateCompatibleBitmap(hDC, cx, cy);
        SelectObject(m_hCDC, m_hBmp);
        return m_hCDC;
    }

    HDC FromWindow(HWND hWnd, int cx = -1, int cy = -1) noexcept
    {
        const auto hDC = ::GetDC(hWnd);
        if (cx < 0 || cy < 0)
        {
            RECT rc;
            GetClientRect(hWnd, &rc);
            cx = rc.right;
            cy = rc.bottom;
        }
        FromDC(hDC, cx, cy);
        ReleaseDC(hWnd, hDC);
        return m_hCDC;
    }

    void ReSize(HWND hWnd, int cx = -1, int cy = -1) noexcept
    {
        EckAssert(m_hCDC);
        if (cx < 0 || cy < 0)
        {
            RECT rc;
            GetClientRect(hWnd, &rc);
            cx = rc.right;
            cy = rc.bottom;
        }

        const auto hDC = ::GetDC(hWnd);
        const auto hNewBmp = CreateCompatibleBitmap(hDC, cx, cy);
        SelectObject(m_hCDC, hNewBmp);
        DeleteObject(m_hBmp);
        m_hBmp = hNewBmp;
        ReleaseDC(hWnd, hDC);
    }


    HDC DibFromDC(HDC hDC, int cx, int cy,
        _Outptr_opt_ void** ppPixel = nullptr) noexcept
    {
        Destroy();
        m_hCDC = CreateCompatibleDC(hDC);
        m_hBmp = CreateDib(hDC, cx, cy, ppPixel);
        SelectObject(m_hCDC, m_hBmp);
        return m_hCDC;
    }

    HDC DibFromWindow(HWND hWnd, int cx = -1, int cy = -1) noexcept
    {
        Destroy();
        const auto hDC = ::GetDC(hWnd);
        if (cx < 0 || cy < 0)
        {
            RECT rc;
            GetClientRect(hWnd, &rc);
            cx = rc.right;
            cy = rc.bottom;
        }
        DibFromDC(hDC, cx, cy);
        ReleaseDC(hWnd, hDC);
        return m_hCDC;
    }

    void DibReSize(HWND hWnd, int cx = 0, int cy = 0,
        _Outptr_opt_ void** ppPixel = nullptr) noexcept
    {
        EckAssert(!!m_hCDC && !!m_hBmp);
        if (cx < 0 || cy < 0)
        {
            RECT rc;
            GetClientRect(hWnd, &rc);
            cx = rc.right;
            cy = rc.bottom;
        }

        const auto hDC = ::GetDC(hWnd);
        const auto hNewBmp = CreateDib(hDC, cx, cy, ppPixel);
        SelectObject(m_hCDC, hNewBmp);
        DeleteObject(m_hBmp);
        m_hBmp = hNewBmp;
        ReleaseDC(hWnd, hDC);
    }

    EckInlineNdCe HDC GetDC() const noexcept { return m_hCDC; }
    EckInlineNdCe HBITMAP GetBitmap() const noexcept { return m_hBmp; }

    void Destroy() noexcept
    {
        if (m_hCDC)
        {
            DeleteDC(m_hCDC);
            m_hCDC = nullptr;
        }
        if (m_hBmp)
        {
            DeleteObject(m_hBmp);
            m_hBmp = nullptr;
        }
    }

    void Attach(HDC hDC, HBITMAP hBmp, BOOL bSelect = FALSE) noexcept
    {
        Destroy();
        m_hCDC = hDC;
        m_hBmp = hBmp;
        if (bSelect)
            SelectObject(m_hCDC, m_hBmp);
    }
    void Detach(_Out_ HDC& hDC, _Out_ HBITMAP& hBmp) noexcept
    {
        hDC = m_hCDC;
        hBmp = m_hBmp;
        m_hCDC = nullptr;
        m_hBmp = nullptr;
    }
};
ECK_NAMESPACE_END