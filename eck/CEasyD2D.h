#pragma once
#include "Utility.h"

ECK_NAMESPACE_BEGIN
#if !ECK_OPT_NO_D2D
struct EZD2D_PARAM
{
    HWND hWnd = nullptr;

    IDXGIFactory2* pDxgiFactory = nullptr;
    IDXGIDevice* pDxgiDevice = nullptr;
    ID2D1Device* pD2dDevice = nullptr;

    UINT cx = 8;
    UINT cy = 8;
    UINT cBuffer = 1;
    DXGI_SCALING uScaling = DXGI_SCALING_NONE;
    DXGI_SWAP_EFFECT uSwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    DXGI_ALPHA_MODE uAlphaMode = DXGI_ALPHA_MODE_IGNORE;
    UINT uFlags = 0;

    D2D1_DEVICE_CONTEXT_OPTIONS uDcOptions = D2D1_DEVICE_CONTEXT_OPTIONS_ENABLE_MULTITHREADED_OPTIMIZATIONS;

    D2D1_ALPHA_MODE uBmpAlphaMode = D2D1_ALPHA_MODE_IGNORE;
    D2D1_BITMAP_OPTIONS uBmpOptions = D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW;

    float fDpi = 96.f;

    EckInline static EZD2D_PARAM MakeBitblt(HWND hWnd, IDXGIFactory2* pDxgiFactory,
        IDXGIDevice* pDxgiDevice, ID2D1Device* pD2dDevice, int cx, int cy, float fDpi = 96.f) noexcept
    {
        return
        {
            hWnd,pDxgiFactory,pDxgiDevice,pD2dDevice,(UINT)cx,(UINT)cy,
            1,DXGI_SCALING_STRETCH,DXGI_SWAP_EFFECT_SEQUENTIAL,DXGI_ALPHA_MODE_IGNORE,0,
            D2D1_DEVICE_CONTEXT_OPTIONS_ENABLE_MULTITHREADED_OPTIMIZATIONS,D2D1_ALPHA_MODE_IGNORE,
            D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
            fDpi
        };
    }

    EckInline static EZD2D_PARAM MakeFlip(HWND hWnd, IDXGIFactory2* pDxgiFactory,
        IDXGIDevice* pDxgiDevice, ID2D1Device* pD2dDevice, int cx, int cy, float fDpi = 96.f) noexcept
    {
        return
        {
            hWnd,pDxgiFactory,pDxgiDevice,pD2dDevice,(UINT)cx,(UINT)cy,
            2,DXGI_SCALING_NONE,DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL,DXGI_ALPHA_MODE_IGNORE,0,
            D2D1_DEVICE_CONTEXT_OPTIONS_ENABLE_MULTITHREADED_OPTIMIZATIONS,D2D1_ALPHA_MODE_IGNORE,
            D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
            fDpi
        };
    }

    EckInline static EZD2D_PARAM MakeComp(HWND hWnd, IDXGIFactory2* pDxgiFactory,
        IDXGIDevice* pDxgiDevice, ID2D1Device* pD2dDevice, int cx, int cy, float fDpi = 96.f) noexcept
    {
        return
        {
            hWnd,pDxgiFactory,pDxgiDevice,pD2dDevice,(UINT)cx,(UINT)cy,
            2,DXGI_SCALING_STRETCH,DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL,DXGI_ALPHA_MODE_IGNORE,0,
            D2D1_DEVICE_CONTEXT_OPTIONS_ENABLE_MULTITHREADED_OPTIMIZATIONS,D2D1_ALPHA_MODE_IGNORE,
            D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
            fDpi
        };
    }
};

struct CEasyD2D
{
    ID2D1DeviceContext* m_pDC = nullptr;
    IDXGISwapChain1* m_pSwapChain = nullptr;
    ID2D1Bitmap1* m_pBitmap = nullptr;

    CEasyD2D() = default;
    CEasyD2D(const CEasyD2D&) = delete;
    CEasyD2D(CEasyD2D&& x) noexcept
        : m_pDC{ x.m_pDC }, m_pSwapChain{ x.m_pSwapChain }, m_pBitmap{ x.m_pBitmap }
    {
        x.m_pDC = nullptr;
        x.m_pSwapChain = nullptr;
        x.m_pBitmap = nullptr;
    }

    CEasyD2D(const EZD2D_PARAM& Param) noexcept
    {
        Create(Param);
    }

    ~CEasyD2D()
    {
        Destroy();
    }

    CEasyD2D& operator=(const CEasyD2D&) = delete;
    CEasyD2D& operator=(CEasyD2D&& x) noexcept
    {
        std::swap(m_pDC, x.m_pDC);
        std::swap(m_pSwapChain, x.m_pSwapChain);
        std::swap(m_pBitmap, x.m_pBitmap);
    }

    HRESULT Create(const EZD2D_PARAM& Param) noexcept
    {
        const DXGI_SWAP_CHAIN_DESC1 DxgiSwapChainDesc
        {
            Param.cx,
            Param.cy,
            DXGI_FORMAT_B8G8R8A8_UNORM,
            FALSE,
            { 1,0 },
            DXGI_USAGE_RENDER_TARGET_OUTPUT,
            Param.cBuffer,
            Param.uScaling,
            Param.uSwapEffect,
            Param.uAlphaMode,
            Param.uFlags
        };

        HRESULT hr;
        if (FAILED(hr = Param.pDxgiFactory->CreateSwapChainForHwnd(Param.pDxgiDevice, Param.hWnd,
            &DxgiSwapChainDesc, nullptr, nullptr, &m_pSwapChain)))
        {
            EckDbgPrintFormatMessage(hr);
            EckDbgBreak();
            return hr;
        }

        if (FAILED(hr = Param.pD2dDevice->CreateDeviceContext(Param.uDcOptions, &m_pDC)))
        {
            SafeRelease(m_pSwapChain);
            EckDbgPrintFormatMessage(hr);
            EckDbgBreak();
            return hr;
        }

        IDXGISurface1* pSurface;
        if (FAILED(hr = m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pSurface))))
        {
            SafeRelease(m_pSwapChain);
            SafeRelease(m_pDC);
            EckDbgPrintFormatMessage(hr);
            EckDbgBreak();
            return hr;
        }

        const D2D1_BITMAP_PROPERTIES1 D2dBmpProp
        {
            { DXGI_FORMAT_B8G8R8A8_UNORM,Param.uBmpAlphaMode },
            Param.fDpi,
            Param.fDpi,
            Param.uBmpOptions
        };

        if (FAILED(hr = m_pDC->CreateBitmapFromDxgiSurface(pSurface, &D2dBmpProp, &m_pBitmap)))
        {
            SafeRelease(m_pSwapChain);
            SafeRelease(m_pDC);
            pSurface->Release();
            EckDbgPrintFormatMessage(hr);
            EckDbgBreak();
            return hr;
        }

        pSurface->Release();
        m_pDC->SetTarget(m_pBitmap);
        return S_OK;
    }

    HRESULT	CreateComposition(const EZD2D_PARAM& Param) noexcept
    {
        const DXGI_SWAP_CHAIN_DESC1 DxgiSwapChainDesc
        {
            Param.cx,
            Param.cy,
            DXGI_FORMAT_B8G8R8A8_UNORM,
            FALSE,
            { 1,0 },
            DXGI_USAGE_RENDER_TARGET_OUTPUT,
            Param.cBuffer,
            Param.uScaling,
            Param.uSwapEffect,
            Param.uAlphaMode,
            Param.uFlags
        };

        HRESULT hr;
        if (FAILED(hr = Param.pDxgiFactory->CreateSwapChainForComposition(Param.pDxgiDevice,
            &DxgiSwapChainDesc, nullptr, &m_pSwapChain)))
        {
            EckDbgPrintFormatMessage(hr);
            EckDbgBreak();
            return hr;
        }

        if (FAILED(hr = Param.pD2dDevice->CreateDeviceContext(Param.uDcOptions, &m_pDC)))
        {
            SafeRelease(m_pSwapChain);
            EckDbgPrintFormatMessage(hr);
            EckDbgBreak();
            return hr;
        }

        IDXGISurface1* pSurface;
        if (FAILED(hr = m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pSurface))))
        {
            SafeRelease(m_pSwapChain);
            SafeRelease(m_pDC);
            EckDbgPrintFormatMessage(hr);
            EckDbgBreak();
            return hr;
        }

        const D2D1_BITMAP_PROPERTIES1 D2dBmpProp
        {
            { DXGI_FORMAT_B8G8R8A8_UNORM,Param.uBmpAlphaMode },
            Param.fDpi,
            Param.fDpi,
            Param.uBmpOptions
        };

        if (FAILED(hr = m_pDC->CreateBitmapFromDxgiSurface(pSurface, &D2dBmpProp, &m_pBitmap)))
        {
            SafeRelease(m_pSwapChain);
            SafeRelease(m_pDC);
            pSurface->Release();
            EckDbgPrintFormatMessage(hr);
            EckDbgBreak();
            return hr;
        }

        pSurface->Release();
        m_pDC->SetTarget(m_pBitmap);
        return S_OK;
    }

    HRESULT ReSize(UINT cBuffer, int cx, int cy, UINT uSwapChainFlags,
        D2D1_ALPHA_MODE uBmpAlphaMode = D2D1_ALPHA_MODE_IGNORE,
        D2D1_BITMAP_OPTIONS uBmpOptions = D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
        float fDpi = 96.0f) noexcept
    {
        EckAssert(!!m_pDC && !!m_pSwapChain && !!m_pBitmap);
        m_pDC->SetTarget(nullptr);
        SafeRelease(m_pBitmap);

        HRESULT hr;
        if (FAILED(hr = m_pSwapChain->ResizeBuffers(cBuffer,
            std::max(8, cx), std::max(8, cy), DXGI_FORMAT_UNKNOWN, uSwapChainFlags)))
        {
            EckDbgPrintFormatMessage(hr);
            EckDbgBreak();
            return hr;
        }

        IDXGISurface1* pSurface;
        hr = m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pSurface));
        if (!pSurface)
        {
            EckDbgPrintFormatMessage(hr);
            EckDbgBreak();
            return hr;
        }

        D2D1_BITMAP_PROPERTIES1 D2dBmpProp
        {
            { DXGI_FORMAT_B8G8R8A8_UNORM,uBmpAlphaMode },
            fDpi,
            fDpi,
            uBmpOptions,
            nullptr
        };

        if (FAILED(hr = m_pDC->CreateBitmapFromDxgiSurface(pSurface, &D2dBmpProp, &m_pBitmap)))
        {
            EckDbgPrintFormatMessage(hr);
            EckDbgBreak();
            return hr;
        }

        pSurface->Release();
        m_pDC->SetTarget(m_pBitmap);
        return S_OK;
    }

    EckInlineNdCe auto GetDC() const noexcept { return m_pDC; }
    EckInlineNdCe auto GetBitmap() const noexcept { return m_pBitmap; }
    EckInlineNdCe auto GetSwapChain() const noexcept { return m_pSwapChain; }

    EckInline void Destroy() noexcept
    {
        SafeRelease(m_pDC);
        SafeRelease(m_pBitmap);
        SafeRelease(m_pSwapChain);
    }
};
#endif// !ECK_OPT_NO_D2D
ECK_NAMESPACE_END