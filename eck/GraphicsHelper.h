#pragma once
#include "ImageHelper.h"

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

struct CEzD2D
{
    ID2D1DeviceContext* m_pDC = nullptr;
    IDXGISwapChain1* m_pSwapChain = nullptr;
    ID2D1Bitmap1* m_pBitmap = nullptr;

    CEzD2D() = default;
    CEzD2D(const CEzD2D&) = delete;
    CEzD2D(CEzD2D&& x) noexcept
        : m_pDC{ x.m_pDC }, m_pSwapChain{ x.m_pSwapChain }, m_pBitmap{ x.m_pBitmap }
    {
        x.m_pDC = nullptr;
        x.m_pSwapChain = nullptr;
        x.m_pBitmap = nullptr;
    }

    CEzD2D(const EZD2D_PARAM& Param) noexcept
    {
        Create(Param);
    }

    ~CEzD2D()
    {
        Destroy();
    }

    CEzD2D& operator=(const CEzD2D&) = delete;
    CEzD2D& operator=(CEzD2D&& x) noexcept
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

enum class GradientMode :BYTE
{
    None,	// 无效
    T2B,	// 从上到下
    B2T,	// 从下到上
    L2R,	// 从左到右
    R2L,	// 从右到左
    TL2BR,	// 从左上到右下↘
    BR2TL,	// 从右下到左上↖
    BL2TR,	// 从左下到右上↗
    TR2BL	// 从右上到左下↙
};

/// <summary>
/// 填充渐变矩形
/// </summary>
/// <param name="hDC">设备场景</param>
/// <param name="rc">矩形</param>
/// <param name="crGradient">渐变色，至少指向3个COLORREF</param>
/// <param name="eMode">渐变模式</param>
/// <returns>GradientFill的返回值</returns>
inline BOOL FillGradientRect(HDC hDC, const RECT& rc,
    _In_reads_(3) COLORREF* crGradient, GradientMode eMode) noexcept
{
    TRIVERTEX tv[4];
    COLORREF cr1, cr2, cr3;
    if (eMode >= GradientMode::T2B && eMode <= GradientMode::R2L)
    {
        ULONG uMode;
        switch (eMode)
        {
        case GradientMode::T2B:// 从上到下
        case GradientMode::B2T:// 从下到上
        {
            cr2 = crGradient[1];
            tv[0].x = rc.left;
            tv[0].y = rc.top;
            tv[1].x = rc.right;
            tv[1].y = (rc.top + rc.bottom) / 2;

            tv[2].x = rc.left;
            tv[2].y = (rc.top + rc.bottom) / 2;
            tv[3].x = rc.right;
            tv[3].y = rc.bottom;
            uMode = GRADIENT_FILL_RECT_V;
            if (eMode == GradientMode::T2B)
            {
                cr1 = crGradient[0];
                cr3 = crGradient[2];
            }
            else
            {
                cr1 = crGradient[2];
                cr3 = crGradient[0];
            }
        }
        break;
        case GradientMode::L2R:// 从左到右
        case GradientMode::R2L:// 从右到左
        {
            cr2 = crGradient[1];
            tv[0].x = rc.left;
            tv[0].y = rc.top;
            tv[1].x = (rc.left + rc.right) / 2;
            tv[1].y = rc.bottom;

            tv[2].x = (rc.left + rc.right) / 2;
            tv[2].y = rc.top;
            tv[3].x = rc.right;
            tv[3].y = rc.bottom;
            uMode = GRADIENT_FILL_RECT_H;
            if (eMode == GradientMode::L2R)
            {
                cr1 = crGradient[0];
                cr3 = crGradient[2];
            }
            else
            {
                cr1 = crGradient[2];
                cr3 = crGradient[0];
            }
        }
        break;
        default: return FALSE;
        }

        tv[0].Red = GetRValue(cr1) << 8;
        tv[0].Green = GetGValue(cr1) << 8;
        tv[0].Blue = GetBValue(cr1) << 8;
        tv[0].Alpha = 0xFF << 8;

        tv[1].Red = GetRValue(cr2) << 8;
        tv[1].Green = GetGValue(cr2) << 8;
        tv[1].Blue = GetBValue(cr2) << 8;
        tv[1].Alpha = 0xFF << 8;

        tv[2] = tv[1];

        tv[3].Red = GetRValue(cr3) << 8;
        tv[3].Green = GetGValue(cr3) << 8;
        tv[3].Blue = GetBValue(cr3) << 8;
        tv[3].Alpha = 0xFF << 8;

        GRADIENT_RECT gr[2];
        gr[0].UpperLeft = 0;
        gr[0].LowerRight = 1;
        gr[1].UpperLeft = 2;
        gr[1].LowerRight = 3;
        return GradientFill(hDC, tv, ARRAYSIZE(tv), gr, ARRAYSIZE(gr), uMode);
    }
    else if (eMode >= GradientMode::TL2BR && eMode <= GradientMode::TR2BL)
    {
        // 左上
        tv[0].x = rc.left;
        tv[0].y = rc.top;
        // 左下
        tv[1].x = rc.left;
        tv[1].y = rc.bottom;
        // 右上
        tv[2].x = rc.right;
        tv[2].y = rc.top;
        // 右下
        tv[3].x = rc.right;
        tv[3].y = rc.bottom;

        GRADIENT_TRIANGLE gt[2];
        switch (eMode)
        {
        case GradientMode::TL2BR:// 左上到右下↘
        case GradientMode::BR2TL:// 右下到左上↖
        {
            gt[0].Vertex1 = 0;
            gt[0].Vertex2 = 1;
            gt[0].Vertex3 = 2;
            gt[1].Vertex1 = 3;
            gt[1].Vertex2 = 1;
            gt[1].Vertex3 = 2;
            cr2 = crGradient[1];
            if (eMode == GradientMode::TL2BR)
            {
                cr1 = crGradient[0];
                cr3 = crGradient[2];
            }
            else
            {
                cr1 = crGradient[2];
                cr3 = crGradient[0];
            }

            tv[0].Red = GetRValue(cr1) << 8;
            tv[0].Green = GetGValue(cr1) << 8;
            tv[0].Blue = GetBValue(cr1) << 8;
            tv[0].Alpha = 0xFF << 8;

            tv[1].Red = GetRValue(cr2) << 8;
            tv[1].Green = GetGValue(cr2) << 8;
            tv[1].Blue = GetBValue(cr2) << 8;
            tv[1].Alpha = 0xFF << 8;

            tv[2] = tv[1];

            tv[3].Red = GetRValue(cr3) << 8;
            tv[3].Green = GetGValue(cr3) << 8;
            tv[3].Blue = GetBValue(cr3) << 8;
            tv[3].Alpha = 0xFF << 8;
        }
        break;
        case GradientMode::BL2TR:// 左下到右上↗
        case GradientMode::TR2BL:// 右上到左下↙
        {
            gt[0].Vertex1 = 1;
            gt[0].Vertex2 = 0;
            gt[0].Vertex3 = 3;
            gt[1].Vertex1 = 2;
            gt[1].Vertex2 = 0;
            gt[1].Vertex3 = 3;
            cr2 = crGradient[1];
            if (eMode == GradientMode::BL2TR)
            {
                cr1 = crGradient[0];
                cr3 = crGradient[2];
            }
            else
            {
                cr1 = crGradient[2];
                cr3 = crGradient[0];
            }

            tv[0].Red = GetRValue(cr2) << 8;
            tv[0].Green = GetGValue(cr2) << 8;
            tv[0].Blue = GetBValue(cr2) << 8;
            tv[0].Alpha = 0xFF << 8;

            tv[1].Red = GetRValue(cr1) << 8;
            tv[1].Green = GetGValue(cr1) << 8;
            tv[1].Blue = GetBValue(cr1) << 8;
            tv[1].Alpha = 0xFF << 8;

            tv[3] = tv[0];

            tv[2].Red = GetRValue(cr3) << 8;
            tv[2].Green = GetGValue(cr3) << 8;
            tv[2].Blue = GetBValue(cr3) << 8;
            tv[2].Alpha = 0xFF << 8;
        }
        break;
        default: return FALSE;
        }
        return GradientFill(hDC, tv, ARRAYSIZE(tv),
            gt, ARRAYSIZE(gt), GRADIENT_FILL_TRIANGLE);
    }
    return FALSE;
}

inline BOOL FillGradientRect(HDC hDC, const RECT& rc,
    COLORREF cr1, COLORREF cr2, BOOL bVertical) noexcept
{
    TRIVERTEX tv[2];
    tv[0].x = rc.left;
    tv[0].y = rc.top;
    tv[0].Red = GetRValue(cr1) << 8;
    tv[0].Green = GetGValue(cr1) << 8;
    tv[0].Blue = GetBValue(cr1) << 8;

    tv[1].x = rc.right;
    tv[1].y = rc.bottom;
    tv[1].Red = GetRValue(cr2) << 8;
    tv[1].Green = GetGValue(cr2) << 8;
    tv[1].Blue = GetBValue(cr2) << 8;

    GRADIENT_RECT gr;
    gr.UpperLeft = 0;	// 左上角坐标为第一个成员
    gr.LowerRight = 1;	// 右下角坐标为第二个成员

    return GradientFill(hDC, tv, 2, &gr, 1,
        bVertical ? GRADIENT_FILL_RECT_V : GRADIENT_FILL_RECT_H);
}

enum class BkImgMode : BYTE
{
    TopLeft,// 左上
    Tile,	// 平铺
    Center,	// 居中
    Stretch,// 缩放
    StretchKeepAspectRatio,// 缩放保持纵横比
};

/// <summary>
/// 画背景图像。
/// 通用背景图像绘制函数，支持32位位图
/// </summary>
/// <param name="hDC">设备场景</param>
/// <param name="hdcBitmap">图像场景</param>
/// <param name="rc">矩形</param>
/// <param name="cxImage">图像宽度</param>
/// <param name="cyImage">图像高度</param>
/// <param name="iMode">模式，DBGIF_常量</param>
/// <param name="bFullRgnImage">是否尽量充满目标区域</param>
/// <returns>AlphaBlend的返回值</returns>
inline BOOL DrawBackgroundImage32(HDC hDC, HDC hdcBitmap,
    const RECT& rc, int cxImage, int cyImage,
    BkImgMode iMode, BOOL bFullRgnImage) noexcept
{
    constexpr BLENDFUNCTION bf{ AC_SRC_OVER,0,255,AC_SRC_ALPHA };
    const int
        cx = rc.right - rc.left,
        cy = rc.bottom - rc.top;
    if (cxImage <= 0 || cyImage <= 0)
    {
        BITMAP bmp;
        if (!GetObjectW(GetCurrentObject(hdcBitmap, OBJ_BITMAP), sizeof(bmp), &bmp))
            return FALSE;
        cxImage = bmp.bmWidth;
        cyImage = bmp.bmHeight;
    }

    switch (iMode)
    {
    case BkImgMode::TopLeft:// 居左上
    {
        if (bFullRgnImage)
        {
            RECT rc1{ 0,0,cxImage,cyImage };
            AdjustRectToFillAnother(rc1, rc);
            return AlphaBlend(hDC, rc.left, rc.top, rc1.right - rc1.left, rc1.bottom - rc1.top,
                hdcBitmap, 0, 0, cxImage, cyImage, bf);
        }
        else
            return AlphaBlend(hDC, rc.left, rc.top, cxImage, cyImage,
                hdcBitmap, 0, 0, cxImage, cyImage, bf);
    }
    ECK_UNREACHABLE;

    case BkImgMode::Tile:// 平铺
    {
        EckCounter(DivUpper(cx, cxImage), i)
        {
            EckCounter(DivUpper(cy, cyImage), j)
                if (!AlphaBlend(hDC, rc.left + i * cxImage, rc.top + j * cyImage, cxImage, cyImage,
                    hdcBitmap, 0, 0, cxImage, cyImage, bf))
                    return FALSE;
        }
        return TRUE;
    }
    ECK_UNREACHABLE;

    case BkImgMode::Center:// 居中
    {
        if (bFullRgnImage)
        {
            RECT rc1{ 0,0,cxImage,cyImage };
            AdjustRectToFillAnother(rc1, rc);
            return AlphaBlend(hDC, rc1.left, rc1.top, rc1.right - rc1.left, rc1.bottom - rc1.top,
                hdcBitmap, 0, 0, cxImage, cyImage, bf);
        }
        else
            return AlphaBlend(hDC, rc.left + (cx - cxImage) / 2, rc.top + (cy - cyImage) / 2, cxImage, cyImage,
                hdcBitmap, 0, 0, cxImage, cyImage, bf);
    }
    ECK_UNREACHABLE;

    case BkImgMode::Stretch:// 缩放
        return AlphaBlend(hDC, rc.left, rc.top, cx, cy, hdcBitmap, 0, 0, cxImage, cyImage, bf);

    case BkImgMode::StretchKeepAspectRatio:// 缩放保持纵横比
    {
        RECT rc1{ 0,0,cxImage,cyImage };
        AdjustRectToFitAnother(rc1, rc);
        return AlphaBlend(hDC, rc1.left, rc1.top, rc1.right - rc1.left, rc1.bottom - rc1.top,
            hdcBitmap, 0, 0, cxImage, cyImage, bf);
    }
    ECK_UNREACHABLE;
    }
    ECK_UNREACHABLE;
}

/// <summary>
/// 画背景图像。
/// 通用背景图像绘制函数
/// </summary>
/// <param name="hDC">设备场景</param>
/// <param name="hdcBitmap">图像场景</param>
/// <param name="rc">矩形</param>
/// <param name="cxImage">图像宽度</param>
/// <param name="cyImage">图像高度</param>
/// <param name="iMode">模式，DBGIF_常量</param>
/// <param name="bFullRgnImage">是否尽量充满目标区域</param>
/// <returns>BitBlt的返回值</returns>
inline BOOL DrawBackgroundImage(HDC hDC, HDC hdcBitmap,
    const RECT& rc, int cxImage, int cyImage,
    BkImgMode iMode, BOOL bFullRgnImage) noexcept
{
    const int
        cx = rc.right - rc.left,
        cy = rc.bottom - rc.top;
    if (cxImage <= 0 || cyImage <= 0)
    {
        BITMAP bmp;
        if (!GetObjectW(GetCurrentObject(hdcBitmap, OBJ_BITMAP), sizeof(bmp), &bmp))
            return FALSE;
        cxImage = bmp.bmWidth;
        cyImage = bmp.bmHeight;
    }

    switch (iMode)
    {
    case BkImgMode::TopLeft:// 居左上
    {
        if (bFullRgnImage)
        {
            RECT rc1{ 0,0,cxImage,cyImage };
            AdjustRectToFillAnother(rc1, rc);
            return BitBlt(hDC, rc.left, rc.top, rc1.right - rc1.left, rc1.bottom - rc1.top,
                hdcBitmap, 0, 0, SRCCOPY);
        }
        else
            return BitBlt(hDC, rc.left, rc.top, cxImage, cyImage, hdcBitmap, 0, 0, SRCCOPY);
    }
    ECK_UNREACHABLE;

    case BkImgMode::Tile:// 平铺
    {
        EckCounter(DivUpper(cx, cxImage), i)
        {
            EckCounter(DivUpper(cy, cyImage), j)
                if (!BitBlt(hDC, rc.left + i * cxImage, rc.top + j * cyImage, cxImage, cyImage,
                    hdcBitmap, 0, 0, SRCCOPY))
                    return FALSE;
        }
        return TRUE;
    }
    ECK_UNREACHABLE;

    case BkImgMode::Center:// 居中
    {
        if (bFullRgnImage)
        {
            RECT rc1{ 0,0,cxImage,cyImage };
            AdjustRectToFillAnother(rc1, rc);
            return BitBlt(hDC, rc1.left, rc1.top, rc1.right - rc1.left, rc1.bottom - rc1.top,
                hdcBitmap, 0, 0, SRCCOPY);
        }
        else
            return BitBlt(hDC, (cx - cxImage) / 2, (cy - cyImage) / 2, cxImage, cyImage,
                hdcBitmap, 0, 0, SRCCOPY);
    }
    ECK_UNREACHABLE;

    case BkImgMode::Stretch:// 缩放
        return BitBlt(hDC, rc.left, rc.top, cx, cy, hdcBitmap, 0, 0, SRCCOPY);

    case BkImgMode::StretchKeepAspectRatio:// 缩放保持纵横比
    {
        RECT rc1{ 0,0,cxImage,cyImage };
        AdjustRectToFitAnother(rc1, rc);
        return BitBlt(hDC, rc1.left, rc1.top, rc1.right - rc1.left, rc1.bottom - rc1.top,
            hdcBitmap, 0, 0, SRCCOPY);
    }
    ECK_UNREACHABLE;
    }
    ECK_UNREACHABLE;
}

#if !ECK_OPT_NO_GDIPLUS
/// <summary>
/// 画背景图像。
/// 通用背景图像绘制函数
/// </summary>
/// <param name="hDC">设备场景</param>
/// <param name="hdcBitmap">图像场景</param>
/// <param name="rc">矩形</param>
/// <param name="cxImage">图像宽度</param>
/// <param name="cyImage">图像高度</param>
/// <param name="iMode">模式，DBGIF_常量</param>
/// <param name="bFullRgnImage">是否尽量充满目标区域</param>
/// <returns>BitBlt的返回值</returns>
inline GpStatus DrawBackgroundImage(GpGraphics* pGraphics, GpImage* pImage,
    const RECT& rc, int cxImage, int cyImage,
    BkImgMode iMode, BOOL bFullRgnImage) noexcept
{
    const int
        cx = rc.right - rc.left,
        cy = rc.bottom - rc.top;
    if (cxImage <= 0 || cyImage <= 0)
    {
        GdipGetImageWidth(pImage, (UINT*)&cxImage);
        GdipGetImageHeight(pImage, (UINT*)&cyImage);
    }

    switch (iMode)
    {
    case BkImgMode::TopLeft:// 居左上
    {
        if (bFullRgnImage)
        {
            RECT rc1{ 0,0,cxImage,cyImage };
            AdjustRectToFillAnother(rc1, rc);
            return GdipDrawImageRectI(pGraphics, pImage,
                rc.left, rc.top, rc1.right - rc1.left, rc1.bottom - rc1.top);
        }
        else
            return GdipDrawImageRectI(pGraphics, pImage, rc.left, rc.top, cxImage, cyImage);
    }
    ECK_UNREACHABLE;

    case BkImgMode::Tile:// 平铺
    {
        GpStatus gps;
        EckCounter(DivUpper(cx, cxImage), i)
        {
            EckCounter(DivUpper(cy, cyImage), j)
                if ((gps = GdipDrawImageRectI(pGraphics, pImage,
                    i * cxImage, j * cyImage, cxImage, cyImage)) != Gdiplus::Ok)
                    return gps;
        }
        return Gdiplus::Ok;
    }
    ECK_UNREACHABLE;

    case BkImgMode::Center:// 居中
    {
        if (bFullRgnImage)
        {
            RECT rc1{ 0,0,cxImage,cyImage };
            AdjustRectToFillAnother(rc1, rc);
            return GdipDrawImageRectI(pGraphics, pImage,
                rc1.left, rc1.top, rc1.right - rc1.left, rc1.bottom - rc1.top);
        }
        else
            return GdipDrawImageRectI(pGraphics, pImage,
                rc.left + (cx - cxImage) / 2, rc.top + (cy - cyImage) / 2, cxImage, cyImage);
    }
    ECK_UNREACHABLE;

    case BkImgMode::Stretch:// 缩放
        return GdipDrawImageRectI(pGraphics, pImage, rc.left, rc.top, cx, cy);

    case BkImgMode::StretchKeepAspectRatio:// 缩放保持纵横比
    {
        RECT rc1{ 0,0,cxImage,cyImage };
        AdjustRectToFitAnother(rc1, rc);
        return GdipDrawImageRectI(pGraphics, pImage,
            rc1.left, rc1.top, rc1.right - rc1.left, rc1.bottom - rc1.top);
    }
    ECK_UNREACHABLE;
    }
    ECK_UNREACHABLE;
}
#endif // !ECK_OPT_NO_GDIPLUS

#if !ECK_OPT_NO_D2D
inline void DrawBackgroundImage(ID2D1RenderTarget* pRT, ID2D1Bitmap* pBmp,
    const D2D1_RECT_F& rc, float cxImage, float cyImage,
    BkImgMode iMode, BOOL bFullRgnImage,
    D2D1_BITMAP_INTERPOLATION_MODE eInterp = D2D1_BITMAP_INTERPOLATION_MODE_LINEAR) noexcept
{
    const float
        cx = rc.right - rc.left,
        cy = rc.bottom - rc.top;
    if (cxImage <= 0 || cyImage <= 0)
    {
        D2D1_SIZE_F sz = pBmp->GetSize();
        cxImage = sz.width;
        cyImage = sz.height;
    }

    switch (iMode)
    {
    case BkImgMode::TopLeft:// 居左上
    {
        if (bFullRgnImage)
        {
            D2D1_RECT_F rc1{ 0,0,cxImage,cyImage };
            AdjustRectToFillAnother(rc1, rc);
            pRT->DrawBitmap(pBmp, rc1, 1.f, eInterp);
        }
        else
            pRT->DrawBitmap(pBmp, rc, 1.f, eInterp);
    }
    return;

    case BkImgMode::Tile:// 平铺
    {
        EckCounter((int)ceilf(cx / cxImage), i)
        {
            EckCounter((int)ceilf(cy / cyImage), j)
                pRT->DrawBitmap(pBmp,
                    D2D1::RectF(i * cxImage, j * cyImage, (i + 1) * cxImage, (j + 1) * cyImage),
                    1.f, eInterp);
        }
    }
    return;

    case BkImgMode::Center:// 居中
    {
        if (bFullRgnImage)
        {
            D2D1_RECT_F rc1{ 0,0,cxImage,cyImage };
            AdjustRectToFillAnother(rc1, rc);
            pRT->DrawBitmap(pBmp, rc1, 1.f, eInterp);
        }
        else
            pRT->DrawBitmap(pBmp,
                D2D1::RectF(rc.left + (cx - cxImage) / 2, rc.top + (cy - cyImage) / 2,
                    rc.left + (cx + cxImage) / 2, rc.top + (cy + cyImage) / 2),
                1.f, eInterp);
    }
    return;

    case BkImgMode::Stretch:// 缩放
        pRT->DrawBitmap(pBmp, rc, 1.f, eInterp);
        return;

    case BkImgMode::StretchKeepAspectRatio:// 缩放保持纵横比
    {
        D2D1_RECT_F rc1{ 0,0,cxImage,cyImage };
        AdjustRectToFitAnother(rc1, rc);
        pRT->DrawBitmap(pBmp, rc1, 1.f, eInterp);
    }
    return;
    }
    ECK_UNREACHABLE;
}

inline void DrawBackgroundImage(ID2D1DeviceContext* pRT, ID2D1Bitmap* pBmp,
    const D2D1_RECT_F& rc, float cxImage, float cyImage,
    BkImgMode iMode, BOOL bFullRgnImage,
    D2D1_INTERPOLATION_MODE eInterp = D2D1_INTERPOLATION_MODE_LINEAR) noexcept
{
    const float
        cx = rc.right - rc.left,
        cy = rc.bottom - rc.top;
    if (cxImage <= 0 || cyImage <= 0)
    {
        D2D1_SIZE_F sz = pBmp->GetSize();
        cxImage = sz.width;
        cyImage = sz.height;
    }

    switch (iMode)
    {
    case BkImgMode::TopLeft:// 居左上
    {
        if (bFullRgnImage)
        {
            D2D1_RECT_F rc1{ 0,0,cxImage,cyImage };
            AdjustRectToFillAnother(rc1, rc);
            pRT->DrawBitmap(pBmp, rc1, 1.f, eInterp);
        }
        else
            pRT->DrawBitmap(pBmp, rc, 1.f, eInterp);
    }
    return;

    case BkImgMode::Tile:// 平铺
    {
        EckCounter((int)ceilf(cx / cxImage), i)
        {
            EckCounter((int)ceilf(cy / cyImage), j)
                pRT->DrawBitmap(pBmp,
                    D2D1::RectF(i * cxImage, j * cyImage, (i + 1) * cxImage, (j + 1) * cyImage),
                    1.f, eInterp);
        }
    }
    return;

    case BkImgMode::Center:// 居中
    {
        if (bFullRgnImage)
        {
            D2D1_RECT_F rc1{ 0,0,cxImage,cyImage };
            AdjustRectToFillAnother(rc1, rc);
            pRT->DrawBitmap(pBmp, rc1, 1.f, eInterp);
        }
        else
            pRT->DrawBitmap(pBmp,
                D2D1::RectF(rc.left + (cx - cxImage) / 2, rc.top + (cy - cyImage) / 2,
                    rc.left + (cx + cxImage) / 2, rc.top + (cy + cyImage) / 2),
                1.f, eInterp);
    }
    return;

    case BkImgMode::Stretch:// 缩放
        pRT->DrawBitmap(pBmp, rc, 1.f, eInterp);
        return;

    case BkImgMode::StretchKeepAspectRatio:// 缩放保持纵横比
    {
        D2D1_RECT_F rc1{ 0,0,cxImage,cyImage };
        AdjustRectToFitAnother(rc1, rc);
        pRT->DrawBitmap(pBmp, rc1, 1.f, eInterp);
    }
    return;
    }
    ECK_UNREACHABLE;
}
#endif // !ECK_OPT_NO_D2D

struct SAVE_DC_CLIP
{
    HRGN hRgn;
};

EckInline SAVE_DC_CLIP SaveDcClip(HDC hDC) noexcept
{
    SAVE_DC_CLIP sdc{ CreateRectRgn(0,0,1,1) };
    if (GetClipRgn(hDC, sdc.hRgn) == 1)
        return sdc;
    else
    {
        DeleteObject(sdc.hRgn);
        return {};
    }
}

EckInline BOOL RestoreDcClip(HDC hDC, SAVE_DC_CLIP sdc) noexcept
{
    const auto b = (SelectClipRgn(hDC, sdc.hRgn) == ERROR);
    if (sdc.hRgn)
        DeleteObject(sdc.hRgn);
    return b;
}

EckInline int IntersectClipRect(HDC hDC, const RECT& rc) noexcept
{
    return IntersectClipRect(hDC, rc.left, rc.top, rc.right, rc.bottom);
}

#if !ECK_OPT_NO_D2D
inline void DrawImageFromGrid(ID2D1RenderTarget* pRT, ID2D1Bitmap* pBmp,
    const D2D1_RECT_F& rcDst, const D2D1_RECT_F& rcSrc, const D2D1_RECT_F& rcMargins,
    D2D1_BITMAP_INTERPOLATION_MODE eInterpolationMode = D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
    float fAlpha = 1.f) noexcept
{
    D2D1_RECT_F rcDstTmp, rcSrcTmp;
    // 左上
    rcDstTmp = { rcDst.left, rcDst.top, rcDst.left + rcMargins.left, rcDst.top + rcMargins.top };
    rcSrcTmp = { rcSrc.left, rcSrc.top, rcSrc.left + rcMargins.left, rcSrc.top + rcMargins.top };
    pRT->DrawBitmap(pBmp, rcDstTmp, fAlpha, eInterpolationMode, rcSrcTmp);
    // 上
    rcDstTmp = { rcDst.left + rcMargins.left, rcDst.top, rcDst.right - rcMargins.right, rcDst.top + rcMargins.top };
    rcSrcTmp = { rcSrc.left + rcMargins.left, rcSrc.top, rcSrc.right - rcMargins.right, rcSrc.top + rcMargins.top };
    pRT->DrawBitmap(pBmp, rcDstTmp, fAlpha, eInterpolationMode, rcSrcTmp);
    // 右上
    rcDstTmp = { rcDst.right - rcMargins.right, rcDst.top, rcDst.right, rcDst.top + rcMargins.top };
    rcSrcTmp = { rcSrc.right - rcMargins.right, rcSrc.top, rcSrc.right, rcSrc.top + rcMargins.top };
    pRT->DrawBitmap(pBmp, rcDstTmp, fAlpha, eInterpolationMode, rcSrcTmp);
    // 左
    rcDstTmp = { rcDst.left, rcDst.top + rcMargins.top, rcDst.left + rcMargins.left, rcDst.bottom - rcMargins.bottom };
    rcSrcTmp = { rcSrc.left, rcSrc.top + rcMargins.top, rcSrc.left + rcMargins.left, rcSrc.bottom - rcMargins.bottom };
    pRT->DrawBitmap(pBmp, rcDstTmp, fAlpha, eInterpolationMode, rcSrcTmp);
    // 右
    rcDstTmp = { rcDst.right - rcMargins.right, rcDst.top + rcMargins.top, rcDst.right, rcDst.bottom - rcMargins.bottom };
    rcSrcTmp = { rcSrc.right - rcMargins.right, rcSrc.top + rcMargins.top, rcSrc.right, rcSrc.bottom - rcMargins.bottom };
    pRT->DrawBitmap(pBmp, rcDstTmp, fAlpha, eInterpolationMode, rcSrcTmp);
    // 左下
    rcDstTmp = { rcDst.left, rcDst.bottom - rcMargins.bottom, rcDst.left + rcMargins.left, rcDst.bottom };
    rcSrcTmp = { rcSrc.left, rcSrc.bottom - rcMargins.bottom, rcSrc.left + rcMargins.left, rcSrc.bottom };
    pRT->DrawBitmap(pBmp, rcDstTmp, fAlpha, eInterpolationMode, rcSrcTmp);
    // 下
    rcDstTmp = { rcDst.left + rcMargins.left, rcDst.bottom - rcMargins.bottom, rcDst.right - rcMargins.right, rcDst.bottom };
    rcSrcTmp = { rcSrc.left + rcMargins.left, rcSrc.bottom - rcMargins.bottom, rcSrc.right - rcMargins.right, rcSrc.bottom };
    pRT->DrawBitmap(pBmp, rcDstTmp, fAlpha, eInterpolationMode, rcSrcTmp);
    // 右下
    rcDstTmp = { rcDst.right - rcMargins.right, rcDst.bottom - rcMargins.bottom, rcDst.right, rcDst.bottom };
    rcSrcTmp = { rcSrc.right - rcMargins.right, rcSrc.bottom - rcMargins.bottom, rcSrc.right, rcSrc.bottom };
    pRT->DrawBitmap(pBmp, rcDstTmp, fAlpha, eInterpolationMode, rcSrcTmp);
    // 中
    rcDstTmp = { rcDst.left + rcMargins.left, rcDst.top + rcMargins.top, rcDst.right - rcMargins.right, rcDst.bottom - rcMargins.bottom };
    rcSrcTmp = { rcSrc.left + rcMargins.left, rcSrc.top + rcMargins.top, rcSrc.right - rcMargins.right, rcSrc.bottom - rcMargins.bottom };
    pRT->DrawBitmap(pBmp, rcDstTmp, fAlpha, eInterpolationMode, rcSrcTmp);
}

inline void DrawImageFromGrid(ID2D1DeviceContext* pDC, ID2D1Bitmap* pBmp,
    const D2D1_RECT_F& rcDst, const D2D1_RECT_F& rcSrc, const D2D1_RECT_F& rcMargins,
    D2D1_INTERPOLATION_MODE eInterpolationMode = D2D1_INTERPOLATION_MODE_LINEAR,
    float fAlpha = 1.f) noexcept
{
    D2D1_RECT_F rcDstTmp, rcSrcTmp;
    // 左上
    rcDstTmp = { rcDst.left, rcDst.top, rcDst.left + rcMargins.left, rcDst.top + rcMargins.top };
    rcSrcTmp = { rcSrc.left, rcSrc.top, rcSrc.left + rcMargins.left, rcSrc.top + rcMargins.top };
    pDC->DrawBitmap(pBmp, rcDstTmp, fAlpha, eInterpolationMode, rcSrcTmp);
    // 上
    rcDstTmp = { rcDst.left + rcMargins.left, rcDst.top, rcDst.right - rcMargins.right, rcDst.top + rcMargins.top };
    rcSrcTmp = { rcSrc.left + rcMargins.left, rcSrc.top, rcSrc.right - rcMargins.right, rcSrc.top + rcMargins.top };
    pDC->DrawBitmap(pBmp, rcDstTmp, fAlpha, eInterpolationMode, rcSrcTmp);
    // 右上
    rcDstTmp = { rcDst.right - rcMargins.right, rcDst.top, rcDst.right, rcDst.top + rcMargins.top };
    rcSrcTmp = { rcSrc.right - rcMargins.right, rcSrc.top, rcSrc.right, rcSrc.top + rcMargins.top };
    pDC->DrawBitmap(pBmp, rcDstTmp, fAlpha, eInterpolationMode, rcSrcTmp);
    // 左
    rcDstTmp = { rcDst.left, rcDst.top + rcMargins.top, rcDst.left + rcMargins.left, rcDst.bottom - rcMargins.bottom };
    rcSrcTmp = { rcSrc.left, rcSrc.top + rcMargins.top, rcSrc.left + rcMargins.left, rcSrc.bottom - rcMargins.bottom };
    pDC->DrawBitmap(pBmp, rcDstTmp, fAlpha, eInterpolationMode, rcSrcTmp);
    // 右
    rcDstTmp = { rcDst.right - rcMargins.right, rcDst.top + rcMargins.top, rcDst.right, rcDst.bottom - rcMargins.bottom };
    rcSrcTmp = { rcSrc.right - rcMargins.right, rcSrc.top + rcMargins.top, rcSrc.right, rcSrc.bottom - rcMargins.bottom };
    pDC->DrawBitmap(pBmp, rcDstTmp, fAlpha, eInterpolationMode, rcSrcTmp);
    // 左下
    rcDstTmp = { rcDst.left, rcDst.bottom - rcMargins.bottom, rcDst.left + rcMargins.left, rcDst.bottom };
    rcSrcTmp = { rcSrc.left, rcSrc.bottom - rcMargins.bottom, rcSrc.left + rcMargins.left, rcSrc.bottom };
    pDC->DrawBitmap(pBmp, rcDstTmp, fAlpha, eInterpolationMode, rcSrcTmp);
    // 下
    rcDstTmp = { rcDst.left + rcMargins.left, rcDst.bottom - rcMargins.bottom, rcDst.right - rcMargins.right, rcDst.bottom };
    rcSrcTmp = { rcSrc.left + rcMargins.left, rcSrc.bottom - rcMargins.bottom, rcSrc.right - rcMargins.right, rcSrc.bottom };
    pDC->DrawBitmap(pBmp, rcDstTmp, fAlpha, eInterpolationMode, rcSrcTmp);
    // 右下
    rcDstTmp = { rcDst.right - rcMargins.right, rcDst.bottom - rcMargins.bottom, rcDst.right, rcDst.bottom };
    rcSrcTmp = { rcSrc.right - rcMargins.right, rcSrc.bottom - rcMargins.bottom, rcSrc.right, rcSrc.bottom };
    pDC->DrawBitmap(pBmp, rcDstTmp, fAlpha, eInterpolationMode, rcSrcTmp);
    // 中
    rcDstTmp = { rcDst.left + rcMargins.left, rcDst.top + rcMargins.top, rcDst.right - rcMargins.right, rcDst.bottom - rcMargins.bottom };
    rcSrcTmp = { rcSrc.left + rcMargins.left, rcSrc.top + rcMargins.top, rcSrc.right - rcMargins.right, rcSrc.bottom - rcMargins.bottom };
    pDC->DrawBitmap(pBmp, rcDstTmp, fAlpha, eInterpolationMode, rcSrcTmp);
}
#endif // !ECK_OPT_NO_D2D

#if !ECK_OPT_NO_GDIPLUS
inline GpStatus DrawImageFromGrid(GpGraphics* pGraphics, GpImage* pImage,
    int xDst, int yDst, int cxDst, int cyDst,
    int xSrc, int ySrc, int cxSrc, int cySrc,
    const MARGINS& Margins, Gdiplus::GpImageAttributes* pIA,
    Gdiplus::Unit eUnit = Gdiplus::UnitPixel) noexcept
{
    // 左上
    GdipDrawImageRectRectI(pGraphics, pImage,
        xDst, yDst, Margins.cxLeftWidth, Margins.cyTopHeight,
        xSrc, ySrc, Margins.cxLeftWidth, Margins.cyTopHeight,
        eUnit, pIA, nullptr, nullptr);
    // 上
    GdipDrawImageRectRectI(pGraphics, pImage,
        xDst + Margins.cxLeftWidth, yDst, cxDst - Margins.cxRightWidth - Margins.cxLeftWidth, Margins.cyTopHeight,
        xSrc + Margins.cxLeftWidth, ySrc, cxSrc - Margins.cxRightWidth - Margins.cxLeftWidth, Margins.cyTopHeight,
        eUnit, pIA, nullptr, nullptr);
    // 右上
    GdipDrawImageRectRectI(pGraphics, pImage,
        xDst + cxDst - Margins.cxRightWidth, yDst, Margins.cxRightWidth, Margins.cyTopHeight,
        xSrc + cxSrc - Margins.cxRightWidth, ySrc, Margins.cxRightWidth, Margins.cyTopHeight,
        eUnit, pIA, nullptr, nullptr);
    // 左
    GdipDrawImageRectRectI(pGraphics, pImage,
        xDst, yDst + Margins.cyTopHeight, Margins.cxLeftWidth, cyDst - Margins.cyBottomHeight - Margins.cyTopHeight,
        xSrc, ySrc + Margins.cyTopHeight, Margins.cxLeftWidth, cySrc - Margins.cyBottomHeight - Margins.cyTopHeight,
        eUnit, pIA, nullptr, nullptr);
    // 右
    GdipDrawImageRectRectI(pGraphics, pImage,
        xDst + cxDst - Margins.cxRightWidth, yDst + Margins.cyTopHeight, Margins.cxRightWidth, cyDst - Margins.cyBottomHeight - Margins.cyTopHeight,
        xSrc + cxSrc - Margins.cxRightWidth, ySrc + Margins.cyTopHeight, Margins.cxRightWidth, cySrc - Margins.cyBottomHeight - Margins.cyTopHeight,
        eUnit, pIA, nullptr, nullptr);
    // 左下
    GdipDrawImageRectRectI(pGraphics, pImage,
        xDst, yDst + cyDst - Margins.cyBottomHeight, Margins.cxLeftWidth, Margins.cyBottomHeight,
        xSrc, ySrc + cySrc - Margins.cyBottomHeight, Margins.cxLeftWidth, Margins.cyBottomHeight,
        eUnit, pIA, nullptr, nullptr);
    // 下
    GdipDrawImageRectRectI(pGraphics, pImage,
        xDst + Margins.cxLeftWidth, yDst + cyDst - Margins.cyBottomHeight, cxDst - Margins.cxRightWidth - Margins.cxLeftWidth, Margins.cyBottomHeight,
        xSrc + Margins.cxLeftWidth, ySrc + cySrc - Margins.cyBottomHeight, cxSrc - Margins.cxRightWidth - Margins.cxLeftWidth, Margins.cyBottomHeight,
        eUnit, pIA, nullptr, nullptr);
    // 右下
    GdipDrawImageRectRectI(pGraphics, pImage,
        xDst + cxDst - Margins.cxRightWidth, yDst + cyDst - Margins.cyBottomHeight, Margins.cxRightWidth, Margins.cyBottomHeight,
        xSrc + cxSrc - Margins.cxRightWidth, ySrc + cySrc - Margins.cyBottomHeight, Margins.cxRightWidth, Margins.cyBottomHeight,
        eUnit, pIA, nullptr, nullptr);
    // 中
    return GdipDrawImageRectRectI(pGraphics, pImage,
        xDst + Margins.cxLeftWidth, yDst + Margins.cyTopHeight, cxDst - Margins.cxRightWidth - Margins.cxLeftWidth, cyDst - Margins.cyBottomHeight - Margins.cyTopHeight,
        xSrc + Margins.cxLeftWidth, ySrc + Margins.cyTopHeight, cxSrc - Margins.cxRightWidth - Margins.cxLeftWidth, cySrc - Margins.cyBottomHeight - Margins.cyTopHeight,
        eUnit, pIA, nullptr, nullptr);
}
#endif // !ECK_OPT_NO_GDIPLUS
ECK_NAMESPACE_END