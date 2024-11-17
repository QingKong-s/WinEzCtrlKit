/*
* WinEzCtrlKit Library
*
* GraphicsHelper.h ： 图形帮助
*
* Copyright(C) 2024 QingKong
*/
#pragma once
#include "ImageHelper.h"

ECK_NAMESPACE_BEGIN
struct CEzCDC
{
	HDC m_hCDC{};
	HBITMAP m_hBmp{};
	HGDIOBJ m_hOld{};

	CEzCDC() = default;
	CEzCDC(const CEzCDC&) = delete;
	CEzCDC(CEzCDC&& x) noexcept :m_hCDC{ x.m_hCDC }, m_hBmp{ x.m_hBmp }, m_hOld{ x.m_hOld }
	{
		x.m_hCDC = nullptr;
		x.m_hBmp = nullptr;
		x.m_hOld = nullptr;
	}

	CEzCDC(HWND hWnd, int cx = 0, int cy = 0)
	{
		Create(hWnd, cx, cy);
	}

	~CEzCDC()
	{
		Destroy();
	}

	CEzCDC& operator=(const CEzCDC&) = delete;
	CEzCDC& operator=(CEzCDC&& x) noexcept
	{
		std::swap(m_hCDC, x.m_hCDC);
		std::swap(m_hBmp, x.m_hBmp);
		std::swap(m_hOld, x.m_hOld);
		return *this;
	}

	HDC Create(HDC hDC, int cx, int cy)
	{
		Destroy();
		m_hCDC = CreateCompatibleDC(hDC);
		m_hBmp = CreateCompatibleBitmap(hDC, cx, cy);
		m_hOld = SelectObject(m_hCDC, m_hBmp);
		return m_hCDC;
	}

	HDC Create(HWND hWnd, int cx = -1, int cy = -1)
	{
		HDC hDC = ::GetDC(hWnd);
		if (cx < 0 || cy < 0)
		{
			RECT rc;
			GetClientRect(hWnd, &rc);
			cx = rc.right;
			cy = rc.bottom;
		}
		Create(hDC, cx, cy);
		ReleaseDC(hWnd, hDC);
		return m_hCDC;
	}

	HDC Create32(HWND hWnd, int cx = 0, int cy = 0)
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
		m_hCDC = CreateCompatibleDC(hDC);
		CDib dib{};
		dib.Create(cx, -cy);
		m_hBmp = dib.Detach();
		m_hOld = SelectObject(m_hCDC, m_hBmp);
		ReleaseDC(hWnd, hDC);
		return m_hCDC;
	}

	void ReSize(HWND hWnd, int cx = 0, int cy = 0)
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

	void ReSize32(HWND hWnd, int cx = 0, int cy = 0)
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

		CDib dib{};
		dib.Create(cx, -cy);
		m_hBmp = dib.Detach();
		m_hOld = SelectObject(m_hCDC, m_hBmp);
		ReleaseDC(hWnd, hDC);
	}

	EckInline constexpr HDC GetDC() const { return m_hCDC; }

	EckInline constexpr HBITMAP GetBitmap() const { return m_hBmp; }

	void Destroy()
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
		IDXGIDevice* pDxgiDevice, ID2D1Device* pD2dDevice, int cx, int cy, float fDpi = 96.f)
	{
		return
		{
			hWnd,pDxgiFactory,pDxgiDevice,pD2dDevice,(UINT)cx,(UINT)cy,
			1,DXGI_SCALING_STRETCH,DXGI_SWAP_EFFECT_DISCARD,DXGI_ALPHA_MODE_IGNORE,0,
			D2D1_DEVICE_CONTEXT_OPTIONS_ENABLE_MULTITHREADED_OPTIMIZATIONS,D2D1_ALPHA_MODE_IGNORE,
			D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
			fDpi
		};
	}

	EckInline static EZD2D_PARAM MakeFlip(HWND hWnd, IDXGIFactory2* pDxgiFactory,
		IDXGIDevice* pDxgiDevice, ID2D1Device* pD2dDevice, int cx, int cy, float fDpi = 96.f)
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
		IDXGIDevice* pDxgiDevice, ID2D1Device* pD2dDevice, int cx, int cy, float fDpi = 96.f)
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
		:m_pDC{ x.m_pDC }, m_pSwapChain{ x.m_pSwapChain }, m_pBitmap{ x.m_pBitmap }
	{
		x.m_pDC = nullptr;
		x.m_pSwapChain = nullptr;
		x.m_pBitmap = nullptr;
	}

	CEzD2D(const EZD2D_PARAM& Param)
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

	HRESULT Create(const EZD2D_PARAM& Param)
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

	HRESULT	CreateComp(const EZD2D_PARAM& Param)
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
		float fDpi = 96.0f)
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

	EckInline constexpr auto GetDC() const { return m_pDC; }

	EckInline constexpr auto GetBitmap() const { return m_pBitmap; }

	EckInline constexpr auto GetSwapChain() const { return m_pSwapChain; }

	EckInline void Destroy()
	{
		SafeRelease(m_pDC);
		SafeRelease(m_pBitmap);
		SafeRelease(m_pSwapChain);
	}
};

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
	_In_reads_(3) COLORREF* crGradient, GradientMode eMode)
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
	COLORREF cr1, COLORREF cr2, BOOL bVertical)
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

enum class BkImgMode :BYTE
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
inline BOOL DrawBackgroundImage32(HDC hDC, HDC hdcBitmap, const RECT& rc, int cxImage, int cyImage,
	BkImgMode iMode, BOOL bFullRgnImage)
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
inline BOOL DrawBackgroundImage(HDC hDC, HDC hdcBitmap, const RECT& rc, int cxImage, int cyImage,
	BkImgMode iMode, BOOL bFullRgnImage)
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
inline GpStatus DrawBackgroundImage(GpGraphics* pGraphics, GpImage* pImage, const RECT& rc, int cxImage, int cyImage,
	BkImgMode iMode, BOOL bFullRgnImage)
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

inline HRESULT BlurD2dDC(ID2D1DeviceContext* pDC, ID2D1Bitmap* pBmp,
	const D2D1_RECT_F& rc, float fDeviation = 3.f)
{
	HRESULT hr;
	ID2D1Effect* pFxBlur;
	if (FAILED(hr = pDC->CreateEffect(CLSID_D2D1GaussianBlur, &pFxBlur)))
		return hr;
	pFxBlur->SetValue(D2D1_GAUSSIANBLUR_PROP_BORDER_MODE, D2D1_BORDER_MODE_HARD);
	pFxBlur->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, fDeviation);

	float xDpi, yDpi;
	pBmp->GetDpi(&xDpi, &yDpi);

	ID2D1Bitmap* pBmpEffect;
	if (FAILED(hr = pDC->CreateBitmap({ (UINT32)(rc.right - rc.left), (UINT32)(rc.bottom - rc.top) },
		D2D1::BitmapProperties(pBmp->GetPixelFormat(), xDpi, yDpi), &pBmpEffect)))
	{
		pFxBlur->Release();
		return hr;
	}
	const D2D1_RECT_U rcU{ (UINT32)rc.left, (UINT32)rc.top, (UINT32)rc.right, (UINT32)rc.bottom };
	pBmpEffect->CopyFromBitmap(nullptr, pBmp, &rcU);

	pFxBlur->SetInput(0, pBmpEffect);

	const auto iBlend = pDC->GetPrimitiveBlend();
	pDC->SetPrimitiveBlend(D2D1_PRIMITIVE_BLEND_COPY);
	pDC->DrawImage(pFxBlur, { rc.left,rc.top });
	pDC->SetPrimitiveBlend(iBlend);

	pBmpEffect->Release();
	pFxBlur->Release();
	return S_OK;
}

inline HRESULT BlurD2dDC(ID2D1DeviceContext* pDC, ID2D1Bitmap* pBmp, ID2D1Bitmap* pBmpWork,
	const D2D1_RECT_F& rc, D2D1_POINT_2F ptDrawing, float fDeviation = 3.f)
{
	HRESULT hr;

	const D2D1_RECT_U rcU{ (UINT32)rc.left, (UINT32)rc.top, (UINT32)rc.right, (UINT32)rc.bottom };
	if (FAILED(hr = pBmpWork->CopyFromBitmap(nullptr, pBmp, &rcU)))
		return hr;

	ID2D1Effect* pFxBlur;
	hr = pDC->CreateEffect(CLSID_D2D1GaussianBlur, &pFxBlur);
	if (FAILED(hr))
		return hr;
	pFxBlur->SetValue(D2D1_GAUSSIANBLUR_PROP_BORDER_MODE, D2D1_BORDER_MODE_HARD);
	pFxBlur->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, fDeviation);

	ID2D1Effect* pFxCrop;
	hr = pDC->CreateEffect(CLSID_D2D1Crop, &pFxCrop);
	if (FAILED(hr))
	{
		pFxBlur->Release();
		return hr;
	}
	pFxCrop->SetValue(D2D1_CROP_PROP_RECT,
		D2D1::RectF(0.f, 0.f, rc.right - rc.left, rc.bottom - rc.top));

	pFxCrop->SetInput(0, pBmpWork);
	pFxBlur->SetInputEffect(0, pFxCrop);

	const auto iBlend = pDC->GetPrimitiveBlend();
	pDC->SetPrimitiveBlend(D2D1_PRIMITIVE_BLEND_COPY);
	pDC->DrawImage(pFxBlur, ptDrawing);
	pDC->SetPrimitiveBlend(iBlend);

	pFxBlur->Release();
	pFxCrop->Release();
	return S_OK;
}

class CSinkForwarder :public ID2D1SimplifiedGeometrySink
{
private:
	LONG m_cRef = 1;
	ID2D1SimplifiedGeometrySink* m_pSink = nullptr;
	float m_oxCurr = 0.f, m_oyCurr = 0.f;
public:
	CSinkForwarder(ID2D1SimplifiedGeometrySink* pSink) :m_pSink{ pSink }
	{
		pSink->AddRef();
	}

	~CSinkForwarder()
	{
		m_pSink->Release();
	}

	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void** ppvObj)
	{
		const static QITAB qit[]
		{
			QITABENT(CSinkForwarder, ID2D1SimplifiedGeometrySink),
			{},
		};
		return QISearch(this, qit, iid, ppvObj);
	}

	ULONG STDMETHODCALLTYPE AddRef() { return ++m_cRef; }

	ULONG STDMETHODCALLTYPE Release()
	{
		if (m_cRef == 1)
		{
			delete this;
			return 0;
		}
		else
			return --m_cRef;
	}

	STDMETHOD_(void, SetFillMode)(D2D1_FILL_MODE fillMode)
	{
		m_pSink->SetFillMode(fillMode);
	}

	STDMETHOD_(void, SetSegmentFlags)(D2D1_PATH_SEGMENT vertexFlags)
	{
		m_pSink->SetSegmentFlags(vertexFlags);
	}

	STDMETHOD_(void, BeginFigure)(D2D1_POINT_2F startPoint, D2D1_FIGURE_BEGIN figureBegin)
	{
		startPoint.x += m_oxCurr;
		startPoint.y += m_oyCurr;
		m_pSink->BeginFigure(startPoint, figureBegin);
	}

	STDMETHOD_(void, AddLines)(CONST D2D1_POINT_2F* points, UINT32 pointsCount)
	{
		if (m_oxCurr != 0.f || m_oyCurr != 0.f)
		{
			auto p = (D2D1_POINT_2F*)_malloca(pointsCount * sizeof(D2D1_POINT_2F));
			EckAssert(p);
			memcpy(p, points, pointsCount * sizeof(D2D1_POINT_2F));
			EckCounter(pointsCount, i)
			{
				p[i].x += m_oxCurr;
				p[i].y += m_oyCurr;
			}
			m_pSink->AddLines(p, pointsCount);
			_freea(p);
		}
		else
			m_pSink->AddLines(points, pointsCount);
	}

	STDMETHOD_(void, AddBeziers)(CONST D2D1_BEZIER_SEGMENT* beziers, UINT32 beziersCount)
	{
		if (m_oxCurr != 0.f || m_oyCurr != 0.f)
		{
			auto p = (D2D1_BEZIER_SEGMENT*)_malloca(beziersCount * sizeof(D2D1_BEZIER_SEGMENT));
			EckAssert(p);
			memcpy(p, beziers, beziersCount * sizeof(D2D1_BEZIER_SEGMENT));
			EckCounter(beziersCount, i)
			{
				p[i].point1.x += m_oxCurr;
				p[i].point1.y += m_oyCurr;
				p[i].point2.x += m_oxCurr;
				p[i].point2.y += m_oyCurr;
				p[i].point3.x += m_oxCurr;
				p[i].point3.y += m_oyCurr;
			}
			m_pSink->AddBeziers(p, beziersCount);
			_freea(p);
		}
		else
			m_pSink->AddBeziers(beziers, beziersCount);
	}

	STDMETHOD_(void, EndFigure)(D2D1_FIGURE_END figureEnd)
	{
		m_pSink->EndFigure(figureEnd);
	}

	STDMETHOD(Close)()
	{
		return m_pSink->Close();
	}

	void SetOffset(float ox, float oy)
	{
		m_oxCurr = ox;
		m_oyCurr = oy;
	}
};

class CTrFetchPath : public IDWriteTextRenderer
{
private:
	ULONG m_uRef = 1;
	ID2D1RenderTarget* m_pRT = nullptr;
	ID2D1Factory* m_pFactory = nullptr;
public:
	CTrFetchPath(ID2D1RenderTarget* pRT, ID2D1Factory* pFactory)
		:m_pRT{ pRT }, m_pFactory{ pFactory }
	{
		pRT->AddRef();
		pFactory->AddRef();
	}

	~CTrFetchPath()
	{
		m_pRT->Release();
		m_pFactory->Release();
	}

	STDMETHOD(DrawGlyphRun)(
		void* pClientDrawingContext,
		FLOAT xOrgBaseline,
		FLOAT yOrgBaseline,
		DWRITE_MEASURING_MODE MeasuringMode,
		DWRITE_GLYPH_RUN const* pGlyphRun,
		DWRITE_GLYPH_RUN_DESCRIPTION const* pGlyphRunDesc,
		IUnknown* pClientDrawingEffect)
	{
		const auto pMySink = (CSinkForwarder*)pClientDrawingContext;
		pMySink->SetOffset(xOrgBaseline, yOrgBaseline);
		return pGlyphRun->fontFace->GetGlyphRunOutline(pGlyphRun->fontEmSize, pGlyphRun->glyphIndices,
			pGlyphRun->glyphAdvances, pGlyphRun->glyphOffsets, pGlyphRun->glyphCount,
			pGlyphRun->isSideways, pGlyphRun->bidiLevel, pMySink);
	}

	STDMETHOD(DrawUnderline)(void* pClientDrawingContext, FLOAT xOrgBaseline, FLOAT yOrgBaseline,
		DWRITE_UNDERLINE const* pUnderline, IUnknown* pClientDrawingEffect)
	{
		return E_NOTIMPL;
	}

	STDMETHOD(DrawStrikethrough)(void* pClientDrawingContext, FLOAT xOrgBaseline,
		FLOAT yOrgBaseline, DWRITE_STRIKETHROUGH const* strikethrough, IUnknown* pClientDrawingEffect)
	{
		return E_NOTIMPL;
	}

	STDMETHOD(DrawInlineObject)(void* pClientDrawingContext, FLOAT xOrg, FLOAT yOrg,
		IDWriteInlineObject* pInlineObject, BOOL bSideways, BOOL bRightToLeft, IUnknown* pClientDrawingEffect)
	{
		return E_NOTIMPL;
	}

	STDMETHOD(IsPixelSnappingDisabled)(void* pClientDrawingContext, BOOL* pbDisabled)
	{
		*pbDisabled = FALSE;
		return S_OK;
	}

	STDMETHOD(GetCurrentTransform)(void* pClientDrawingContext, DWRITE_MATRIX* pMatrix)
	{
		m_pRT->GetTransform((D2D1_MATRIX_3X2_F*)pMatrix);
		return S_OK;
	}

	STDMETHOD(GetPixelsPerDip)(void* pClientDrawingContext, FLOAT* pfPixelsPerDip)
	{
		float x, y;
		m_pRT->GetDpi(&x, &y);
		*pfPixelsPerDip = x / 96.f;
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void** ppvObj)
	{
		const static QITAB qit[]
		{
			QITABENT(CTrFetchPath, IDWriteTextRenderer),
			QITABENT(CTrFetchPath, IDWritePixelSnapping),
			{},
		};
		return QISearch(this, qit, iid, ppvObj);
	}

	ULONG STDMETHODCALLTYPE AddRef() { return ++m_uRef; }

	ULONG STDMETHODCALLTYPE Release()
	{
		if (--m_uRef)
			return m_uRef;
		delete this;
		return 0;
	}
};

/// <summary>
/// DW文本布局到路径几何形
/// </summary>
/// <param name="pLayout">DW文本布局</param>
/// <param name="pRT">要在其上呈现的渲染目标</param>
/// <param name="x">起始X</param>
/// <param name="y">起始Y</param>
/// <param name="pPathGeometry">结果路径几何形</param>
/// <param name="pD2dFactory">D2D工厂</param>
/// <returns>HRESULT</returns>
inline HRESULT GetTextLayoutPathGeometry(IDWriteTextLayout* pLayout, ID2D1RenderTarget* pRT,
	float x, float y, ID2D1PathGeometry1*& pPathGeometry, ID2D1Factory1* pD2dFactory = nullptr)
{
	if (!pD2dFactory)
		pD2dFactory = g_pD2dFactory;

	HRESULT hr;
	ID2D1PathGeometry1* pPath;
	ID2D1GeometrySink* pSink;
	pD2dFactory->CreatePathGeometry(&pPath);
	pPath->Open(&pSink);

	const auto pTr = new CTrFetchPath(pRT, pD2dFactory);
	const auto pMySink = new CSinkForwarder(pSink);

	hr = pLayout->Draw(pMySink, pTr, x, y);
	if (SUCCEEDED(hr))
		hr = pMySink->Close();
	pMySink->Release();
	pSink->Release();

	pTr->Release();

	if (FAILED(hr))
	{
		pPath->Release();
		pPathGeometry = nullptr;
	}
	else
		pPathGeometry = pPath;
	return hr;
}

/// <summary>
/// DW文本布局到路径几何形
/// </summary>
/// <param name="pLayout">DW文本布局数组</param>
/// <param name="cLayout">布局个数</param>
/// <param name="cyPadding">间隔</param>
/// <param name="pRT">要在其上呈现的渲染目标</param>
/// <param name="x">起始X</param>
/// <param name="y">起始Y</param>
/// <param name="pPathGeometry">结果路径几何形</param>
/// <param name="pD2dFactory">D2D工厂</param>
/// <returns>HRESULT</returns>
inline HRESULT GetTextLayoutPathGeometry(IDWriteTextLayout* const* pLayout, int cLayout, const float* cyPadding,
	ID2D1RenderTarget* pRT, float* x, float yStart, ID2D1PathGeometry*& pPathGeometry, ID2D1Factory* pD2dFactory = nullptr)
{
	if (!pD2dFactory)
		pD2dFactory = g_pD2dFactory;

	HRESULT hr = S_OK;
	ID2D1PathGeometry* pPath;
	ID2D1GeometrySink* pSink;
	pD2dFactory->CreatePathGeometry(&pPath);
	pPath->Open(&pSink);

	const auto pTr = new CTrFetchPath(pRT, pD2dFactory);
	const auto pMySink = new CSinkForwarder(pSink);

	DWRITE_TEXT_METRICS tm;
	EckCounter(cLayout, i)
	{
		if (pLayout[i])
		{
			hr = pLayout[i]->Draw(pMySink, pTr, x[i], yStart);
			if (FAILED(hr))
				break;
			pLayout[i]->GetMetrics(&tm);
			yStart += (tm.height + cyPadding[i]);
		}
		else
			yStart += cyPadding[i];
	}

	if (SUCCEEDED(hr))
		hr = pMySink->Close();
	pMySink->Release();
	pSink->Release();

	pTr->Release();

	if (FAILED(hr))
	{
		pPath->Release();
		pPathGeometry = nullptr;
	}
	else
		pPathGeometry = pPath;
	return hr;
}

struct SAVE_DC_CLIP
{
	HRGN hRgn;
};

EckInline SAVE_DC_CLIP SaveDcClip(HDC hDC)
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

EckInline BOOL RestoreDcClip(HDC hDC, SAVE_DC_CLIP sdc)
{
	const auto b = (SelectClipRgn(hDC, sdc.hRgn) == ERROR);
	if (sdc.hRgn)
		DeleteObject(sdc.hRgn);
	return b;
}

EckInline int IntersectClipRect(HDC hDC, const RECT& rc)
{
	return IntersectClipRect(hDC, rc.left, rc.top, rc.right, rc.bottom);
}

inline void DrawImageFromGrid(ID2D1RenderTarget* pRT, ID2D1Bitmap* pBmp,
	const D2D1_RECT_F& rcDst, const D2D1_RECT_F& rcSrc, const D2D1_RECT_F& rcMargins,
	D2D1_BITMAP_INTERPOLATION_MODE eInterpolationMode = D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
	float fAlpha = 1.f)
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
	float fAlpha = 1.f)
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

inline GpStatus DrawImageFromGrid(GpGraphics* pGraphics, GpImage* pImage,
	int xDst, int yDst, int cxDst, int cyDst,
	int xSrc, int ySrc, int cxSrc, int cySrc,
	const MARGINS& Margins, Gdiplus::GpImageAttributes* pIA,
	Gdiplus::Unit eUnit = Gdiplus::UnitPixel)
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
ECK_NAMESPACE_END