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

    HGDIOBJ FromDC(HDC hDC, int cx, int cy) noexcept
    {
        Destroy();
        m_hCDC = CreateCompatibleDC(hDC);
        m_hBmp = CreateCompatibleBitmap(hDC, cx, cy);
        return SelectObject(m_hCDC, m_hBmp);
    }

    HGDIOBJ FromWindow(HWND hWnd, int cx = -1, int cy = -1) noexcept
    {
        const auto hDC = ::GetDC(hWnd);
        if (cx < 0 || cy < 0)
        {
            RECT rc;
            GetClientRect(hWnd, &rc);
            cx = rc.right;
            cy = rc.bottom;
        }
        const auto hOld = FromDC(hDC, cx, cy);
        ReleaseDC(hWnd, hDC);
        return hOld;
    }

    HGDIOBJ ReSize(HWND hWnd, int cx = -1, int cy = -1) noexcept
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
        const auto hOld = SelectObject(m_hCDC, hNewBmp);
        DeleteObject(m_hBmp);
        m_hBmp = hNewBmp;
        ReleaseDC(hWnd, hDC);
        return hOld;
    }


    HGDIOBJ DibFromDC(HDC hDC, int cx, int cy,
        _Outptr_opt_ void** ppPixel = nullptr) noexcept
    {
        Destroy();
        m_hCDC = CreateCompatibleDC(hDC);
        m_hBmp = CreateDib(hDC, cx, cy, ppPixel);
        return SelectObject(m_hCDC, m_hBmp);
    }

    HGDIOBJ DibFromWindow(HWND hWnd, int cx = -1, int cy = -1,
        _Outptr_opt_ void** ppPixel = nullptr) noexcept
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
        const auto hOld = DibFromDC(hDC, cx, cy, ppPixel);
        ReleaseDC(hWnd, hDC);
        return hOld;
    }

    HGDIOBJ DibReSize(HWND hWnd, int cx = 0, int cy = 0,
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
        const auto hOld = SelectObject(m_hCDC, hNewBmp);
        DeleteObject(m_hBmp);
        m_hBmp = hNewBmp;
        ReleaseDC(hWnd, hDC);
        return hOld;
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

    HGDIOBJ Attach(HDC hDC, HBITMAP hBmp, BOOL bSelect = FALSE) noexcept
    {
        Destroy();
        m_hCDC = hDC;
        m_hBmp = hBmp;
        if (bSelect)
            return SelectObject(m_hCDC, m_hBmp);
        return nullptr;
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