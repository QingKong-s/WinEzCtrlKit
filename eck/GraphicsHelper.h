#pragma once
#include "ImageHelper.h"

ECK_NAMESPACE_BEGIN
struct CEzCDC
{
	HDC m_hCDC = NULL;
	HBITMAP m_hBmp = NULL;
	HGDIOBJ m_hOld = NULL;

	CEzCDC() = default;
	CEzCDC(const CEzCDC&) = delete;
	CEzCDC(CEzCDC&& x) noexcept :m_hCDC{ x.m_hCDC }, m_hBmp{ x.m_hBmp }, m_hOld{ x.m_hOld }
	{
		x.m_hCDC = NULL;
		x.m_hBmp = NULL;
		x.m_hOld = NULL;
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
	}

	HDC Create(HWND hWnd, int cx = 0, int cy = 0)
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
		m_hBmp = CreateCompatibleBitmap(hDC, cx, cy);
		m_hOld = SelectObject(m_hCDC, m_hBmp);
		ReleaseDC(hWnd, hDC);
		return m_hCDC;
	}

	void ReSize(HWND hWnd, int cx = 0, int cy = 0)
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

		m_hBmp = CreateCompatibleBitmap(hDC, cx, cy);
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
			m_hCDC = NULL;
			m_hBmp = NULL;
			m_hOld = NULL;
		}
	}
};

struct EZD2D_PARAM
{
	HWND hWnd = NULL;

	IDXGIFactory2* pDxgiFactory = NULL;
	IDXGIDevice* pDxgiDevice = NULL;
	ID2D1Device* pD2dDevice = NULL;

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

	EckInline static EZD2D_PARAM MakeBitblt(HWND hWnd, IDXGIFactory2* pDxgiFactory,
		IDXGIDevice* pDxgiDevice, ID2D1Device* pD2dDevice, int cx, int cy)
	{
		return
		{
			hWnd,pDxgiFactory,pDxgiDevice,pD2dDevice,(UINT)cx,(UINT)cy,
			1,DXGI_SCALING_STRETCH,DXGI_SWAP_EFFECT_DISCARD,DXGI_ALPHA_MODE_IGNORE,0,
			D2D1_DEVICE_CONTEXT_OPTIONS_ENABLE_MULTITHREADED_OPTIMIZATIONS,D2D1_ALPHA_MODE_IGNORE,
			D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW
		};
	}

	EckInline static EZD2D_PARAM MakeFlip(HWND hWnd, IDXGIFactory2* pDxgiFactory,
		IDXGIDevice* pDxgiDevice, ID2D1Device* pD2dDevice, int cx, int cy)
	{
		return
		{
			hWnd,pDxgiFactory,pDxgiDevice,pD2dDevice,(UINT)cx,(UINT)cy,
			2,DXGI_SCALING_NONE,DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL,DXGI_ALPHA_MODE_IGNORE,0,
			D2D1_DEVICE_CONTEXT_OPTIONS_ENABLE_MULTITHREADED_OPTIMIZATIONS,D2D1_ALPHA_MODE_IGNORE,
			D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW
		};
	}
};

struct CEzD2D
{
	ID2D1DeviceContext* m_pDC = NULL;
	IDXGISwapChain1* m_pSwapChain = NULL;
	ID2D1Bitmap1* m_pBitmap = NULL;

	CEzD2D() = default;
	CEzD2D(const CEzD2D&) = delete;
	CEzD2D(CEzD2D&& x) noexcept
		:m_pDC{ x.m_pDC }, m_pSwapChain{ x.m_pSwapChain }, m_pBitmap{ x.m_pBitmap }
	{
		x.m_pDC = NULL;
		x.m_pSwapChain = NULL;
		x.m_pBitmap = NULL;
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
			{1, 0},
			DXGI_USAGE_RENDER_TARGET_OUTPUT,
			Param.cBuffer,
			Param.uScaling,
			Param.uSwapEffect,
			Param.uAlphaMode,
			Param.uFlags
		};

		HRESULT hr;
		if (FAILED(hr = Param.pDxgiFactory->CreateSwapChainForHwnd(Param.pDxgiDevice, Param.hWnd,
			&DxgiSwapChainDesc, NULL, NULL, &m_pSwapChain)))
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
			{DXGI_FORMAT_B8G8R8A8_UNORM,Param.uBmpAlphaMode},
			96,
			96,
			Param.uBmpOptions,
			NULL
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
		D2D1_BITMAP_OPTIONS uBmpOptions = D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW)
	{
		EckAssert(!!m_pDC && !!m_pSwapChain && !!m_pBitmap);
		m_pDC->SetTarget(NULL);
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
			{DXGI_FORMAT_B8G8R8A8_UNORM,uBmpAlphaMode},
			96,
			96,
			uBmpOptions,
			NULL
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

inline HRESULT MakePloyLinePath(const D2D1_POINT_2F* pPt, int cPt,
	ID2D1Factory* pFactory, ID2D1PathGeometry*& pPathGeo)
{
	ID2D1PathGeometry* pPathGeometry;
	HRESULT hr = pFactory->CreatePathGeometry(&pPathGeometry);
	if (!pPathGeometry)
		return hr;
	ID2D1GeometrySink* pSink;
	pPathGeometry->Open(&pSink);
	pSink->BeginFigure(*pPt, D2D1_FIGURE_BEGIN_HOLLOW);
	pSink->AddLines(pPt, cPt);
	pSink->EndFigure(D2D1_FIGURE_END_OPEN);
	hr = pSink->Close();
	if (FAILED(hr))
	{
		pSink->Release();
		pPathGeometry->Release();
		return hr;
	}
	pSink->Release();
	pPathGeo = pPathGeometry;
	return S_OK;
}

/// <summary>
/// 计算繁花曲线各点
/// </summary>
/// <param name="vPt">点集合</param>
/// <param name="rOut">外圆半径</param>
/// <param name="rInt">内圆半径</param>
/// <param name="iOffsetPtPen">描绘点距内圆圆心的偏移</param>
/// <param name="fStep">步长</param>
template<class TVal = int, class TPt>
inline void CalcSpirographPoint(std::vector<TPt>& vPt, TVal rOut, TVal rInt, TVal iOffsetPtPen, float fStep = 0.1f)
{
	vPt.clear();
	float t = 0.f;
	const float k = (float)rInt / (float)rOut;
	const float l = (float)iOffsetPtPen / (float)rInt;
	const float tEnd = 2.f * PiF * (float)rInt / (float)Gcd((UINT)rOut, (UINT)rInt);
	const float fOneMinusK = 1 - k;
	const float fOneMinusKDivK = fOneMinusK / k;
	while (t < tEnd)
	{
#if ECKCXX20
		vPt.emplace_back(
			(TVal)(rOut * (fOneMinusK * cosf(t) + l * k * cosf(fOneMinusKDivK * t))),
			(TVal)(rOut * (fOneMinusK * sinf(t) - l * k * sinf(fOneMinusKDivK * t))));
#else
		vPt.emplace_back(TPt{
			(TVal)(rOut * (fOneMinusK * cosf(t) + l * k * cosf(fOneMinusKDivK * t))),
			(TVal)(rOut * (fOneMinusK * sinf(t) - l * k * sinf(fOneMinusKDivK * t))) });
#endif
		t += fStep;
	}
	vPt.emplace_back(vPt.front());
}

/// <summary>
/// 画繁花曲线
/// </summary>
/// <param name="hDC">设备场景</param>
/// <param name="xCenter">中心点X</param>
/// <param name="yCenter">中心点Y</param>
/// <param name="rOut">外圆半径</param>
/// <param name="rInt">内圆半径</param>
/// <param name="iOffsetPtPen">描绘点距内圆圆心的偏移</param>
/// <param name="fStep">步长</param>
/// <returns>Polyline返回值</returns>
EckInline BOOL DrawSpirograph(HDC hDC, int xCenter, int yCenter, int rOut, int rInt, int iOffsetPtPen, float fStep = 0.1f)
{
	std::vector<POINT> vPt{};
	CalcSpirographPoint(vPt, rOut, rInt, iOffsetPtPen, fStep);
	std::transform(std::execution::par_unseq, vPt.begin(), vPt.end(), vPt.begin(), [xCenter, yCenter](POINT& pt)
		{
			return POINT{ pt.x + xCenter,yCenter - pt.y };
		});
	return Polyline(hDC, vPt.data(), (int)vPt.size());
}

/// <summary>
/// 画繁花曲线
/// </summary>
/// <param name="pGraphics">图形对象</param>
/// <param name="pPen">画笔对象</param>
/// <param name="xCenter">中心点X</param>
/// <param name="yCenter">中心点Y</param>
/// <param name="rOut">外圆半径</param>
/// <param name="rInt">内圆半径</param>
/// <param name="iOffsetPtPen">描绘点距内圆圆心的偏移</param>
/// <param name="fStep">步长</param>
/// <returns>GdipDrawLines返回值</returns>
EckInline GpStatus DrawSpirograph(GpGraphics* pGraphics, GpPen* pPen, float xCenter, float yCenter, float rOut, float rInt,
	float fOffsetPtPen, float fStep = 0.1f)
{
	std::vector<GpPointF> vPt{};
	CalcSpirographPoint<float>(vPt, rOut, rInt, fOffsetPtPen, fStep);
	std::transform(std::execution::par_unseq, vPt.begin(), vPt.end(), vPt.begin(), [xCenter, yCenter](GpPointF& pt)
		{
			return GpPointF{ pt.x + xCenter,yCenter - pt.y };
		});
	return GdipDrawLines(pGraphics, pPen, vPt.data(), (int)vPt.size());
}

struct DRAW_SPIROGRAPH_D2D_PARAM
{
	ID2D1Factory* pFactory = NULL;// D2D工厂
	ID2D1RenderTarget* pRT = NULL;// D2D渲染目标
	ID2D1Brush* pBrush = NULL;// D2D画刷
	float cxStroke = 1.f;// 笔画宽度
	ID2D1StrokeStyle* pStrokeStyle = NULL;// 笔画样式
	float xCenter = 0.f;// 中心点X
	float yCenter = 0.f;// 中心点Y
	float rOut = 0.f;// 外圆半径
	float rInt = 0.f;// 内圆半径
	float fOffsetPtPen = 0.f;// 描绘点距内圆圆心的偏移
	float fStep = 0.1f;// 步长
};

/// <summary>
/// 画繁花曲线
/// </summary>
/// <param name="Info">参数</param>
/// <param name="ppPathGeometry">接收路径几何形变量的指针</param>
/// <returns>构建路径几何形时的失败信息，无法判断绘制操作成功与否，调用方应检查EndDraw返回值</returns>
inline HRESULT DrawSpirograph(const DRAW_SPIROGRAPH_D2D_PARAM& Info, ID2D1PathGeometry** ppPathGeometry = NULL)
{
	if (ppPathGeometry)
		*ppPathGeometry = NULL;
	std::vector<D2D1_POINT_2F> vPt{};
	CalcSpirographPoint<float>(vPt, Info.rOut, Info.rInt, Info.fOffsetPtPen, Info.fStep);
	std::transform(std::execution::par_unseq, vPt.begin(), vPt.end(), vPt.begin(), [&Info](D2D1_POINT_2F& pt)
		{
			return D2D1_POINT_2F{ pt.x + Info.xCenter,Info.yCenter - pt.y };
		});
	ID2D1PathGeometry* pPathGeometry;
	HRESULT hr;
	if (FAILED(hr = MakePloyLinePath(vPt.data(), (int)vPt.size(), Info.pFactory, pPathGeometry)))
		return hr;
	Info.pRT->DrawGeometry(pPathGeometry, Info.pBrush, Info.cxStroke, Info.pStrokeStyle);
	if (ppPathGeometry)
		*ppPathGeometry = pPathGeometry;
	else
		pPathGeometry->Release();
	return S_OK;
}

/// <summary>
/// 计算扭曲矩阵。
/// 计算将矩形映射到任意凸四边形的4x4矩阵
/// </summary>
/// <param name="rcOrg">矩形</param>
/// <param name="ptDistort">映射到的点，至少指向四个D2D1_POINT_2F，分别对应左上角、右上角、左下角、右下角</param>
/// <param name="MatrixResult">结果矩阵</param>
inline void CalcDistortMatrix(const D2D1_RECT_F& rcOrg,
	const D2D1_POINT_2F* ptDistort, D2D1_MATRIX_4X4_F& MatrixResult)
{
	const float cx = rcOrg.right - rcOrg.left;
	const float cy = rcOrg.bottom - rcOrg.top;

	const auto TN = DirectX::XMMatrixSet(
		1.f / cx, 0, 0, 0,
		0, 1.f / cy, 0, 0,
		0, 0, 0, 0,
		-rcOrg.left / cx, -rcOrg.top / cy, 0, 1.f);

	const DirectX::XMFLOAT4X4 MA(
		ptDistort[1].x - ptDistort[0].x, ptDistort[1].y - ptDistort[0].y, 0, 0,
		ptDistort[2].x - ptDistort[0].x, ptDistort[2].y - ptDistort[0].y, 0, 0,
		0, 0, 0, 0,
		ptDistort[0].x, ptDistort[0].y, 0, 1.f);
	const auto TA = DirectX::XMLoadFloat4x4(&MA);

	const float fDen = MA._11 * MA._22 - MA._12 * MA._21;
	const float a = (
		MA._22 * ptDistort[3].x -
		MA._21 * ptDistort[3].y +
		MA._21 * MA._42 -
		MA._22 * MA._41) / fDen;
	const float b = (
		MA._11 * ptDistort[3].y -
		MA._12 * ptDistort[3].x +
		MA._12 * MA._41 -
		MA._11 * MA._42) / fDen;
	const auto TB = DirectX::XMMatrixSet(
		a / (a + b - 1), 0, 0, a / (a + b - 1) - 1,
		0, b / (a + b - 1), 0, b / (a + b - 1) - 1,
		0, 0, 0, 0,
		0, 0, 0, 1.f);

	DirectX::XMStoreFloat4x4((DirectX::XMFLOAT4X4*)&MatrixResult, TN * TB * TA);
}

template<class TVal = int, class TPt>
/// <summary>
/// 计算蝴蝶曲线各点
/// </summary>
/// <param name="vPt">点集合</param>
/// <param name="fDeformationCoefficient">变形系数</param>
/// <param name="fScaleX">X方向缩放</param>
/// <param name="fScaleY">Y方向缩放</param>
/// <param name="fStep">步长</param>
EckInline void CalcButterflyCurvePoint(std::vector<TPt>& vPt, float fDeformationCoefficient = 4.f,
	float fScaleX = 100.f, float fScaleY = 100.f, float fStep = 0.01f)
{
	vPt.clear();
	float t = 0.f;
	while (t < PiF * 20.f)
	{
		const float f = (expf(cosf(t)) - 2.f * cosf(fDeformationCoefficient * t) - powf(sinf(t / 12.f), 5.f));
#if ECKCXX20
		vPt.emplace_back((TVal)(sinf(t) * f * fScaleX), (TVal)(cosf(t) * f * fScaleY));
#else
		vPt.emplace_back(TPt{ (TVal)(sinf(t) * f * fScaleX), (TVal)(cosf(t) * f * fScaleY) });
#endif
		t += fStep;
	}
	vPt.emplace_back(vPt.front());
}

/// <summary>
/// 画蝴蝶曲线
/// </summary>
/// <param name="hDC">设备场景</param>
/// <param name="xCenter">中心点X</param>
/// <param name="yCenter">中心点Y</param>
/// <param name="fDeformationCoefficient">变形系数</param>
/// <param name="fScaleX">X方向缩放</param>
/// <param name="fScaleY">Y方向缩放</param>
/// <param name="fStep">步长</param>
EckInline BOOL DrawButterflyCurve(HDC hDC, int xCenter, int yCenter, float fDeformationCoefficient = 4.f,
	float fScaleX = 100.f, float fScaleY = 100.f, float fStep = 0.01f)
{
	std::vector<POINT> vPt{};
	CalcButterflyCurvePoint(vPt, fDeformationCoefficient, fScaleX, fScaleY, fStep);
	std::transform(std::execution::par_unseq, vPt.begin(), vPt.end(), vPt.begin(), [xCenter, yCenter](POINT& pt)
		{
			return POINT{ pt.x + xCenter,yCenter - pt.y };
		});
	return Polyline(hDC, vPt.data(), (int)vPt.size());
}

/// <summary>
/// 画蝴蝶曲线
/// </summary>
/// <param name="pGraphics">图形对象</param>
/// <param name="pPen">画笔</param>
/// <param name="xCenter">中心点X</param>
/// <param name="yCenter">中心点Y</param>
/// <param name="fDeformationCoefficient">变形系数</param>
/// <param name="fScaleX">X方向缩放</param>
/// <param name="fScaleY">Y方向缩放</param>
/// <param name="fStep">步长</param>
/// <returns>GdipDrawLines返回值</returns>
EckInline GpStatus DrawButterflyCurve(GpGraphics* pGraphics, GpPen* pPen, int xCenter, int yCenter, float fDeformationCoefficient = 4.f,
	float fScaleX = 100.f, float fScaleY = 100.f, float fStep = 0.01f)
{
	std::vector<GpPointF> vPt{};
	CalcButterflyCurvePoint<float>(vPt, fDeformationCoefficient, fScaleX, fScaleY, fStep);
	std::transform(std::execution::par_unseq, vPt.begin(), vPt.end(), vPt.begin(), [xCenter, yCenter](GpPointF& pt)
		{
			return GpPointF{ pt.x + xCenter,yCenter - pt.y };
		});
	return GdipDrawLines(pGraphics, pPen, vPt.data(), (int)vPt.size());
}

struct DRAW_BUTTERFLYCURVE_D2D_PARAM
{
	ID2D1Factory* pFactory = NULL;// D2D工厂
	ID2D1RenderTarget* pRT = NULL;// D2D渲染目标
	ID2D1Brush* pBrush = NULL;// D2D画刷
	float cxStroke = 1.f;// 笔画宽度
	ID2D1StrokeStyle* pStrokeStyle = NULL;// 笔画样式
	float xCenter = 0.f;// 中心点X
	float yCenter = 0.f;// 中心点Y
	float fDeformationCoefficient = 4.f;// 变形系数
	float fScaleX = 100.f;// X方向缩放
	float fScaleY = 100.f;// Y方向缩放
	float fStep = 0.01f;// 步长
};

/// <summary>
/// 画蝴蝶曲线
/// </summary>
/// <param name="Info">参数</param>
/// <param name="ppPathGeometry">接收路径几何形变量的指针</param>
/// <returns>构建路径几何形时的失败信息，无法判断绘制操作成功与否，调用方应检查EndDraw返回值</returns>
inline HRESULT DrawButterflyCurve(const DRAW_BUTTERFLYCURVE_D2D_PARAM& Info,
	ID2D1PathGeometry** ppPathGeometry = NULL)
{
	if (ppPathGeometry)
		*ppPathGeometry = NULL;
	std::vector<D2D1_POINT_2F> vPt{};
	CalcButterflyCurvePoint<float>(vPt, Info.fDeformationCoefficient, Info.fScaleX, Info.fScaleY, Info.fStep);
	std::transform(std::execution::par_unseq, vPt.begin(), vPt.end(), vPt.begin(), [&Info](D2D1_POINT_2F& pt)
		{
			return D2D1_POINT_2F{ pt.x + Info.xCenter,Info.yCenter - pt.y };
		});
	ID2D1PathGeometry* pPathGeometry;
	HRESULT hr;
	if (FAILED(hr = MakePloyLinePath(vPt.data(), (int)vPt.size(), Info.pFactory, pPathGeometry)))
		return hr;
	Info.pRT->DrawGeometry(pPathGeometry, Info.pBrush, Info.cxStroke, Info.pStrokeStyle);
	if (ppPathGeometry)
		*ppPathGeometry = pPathGeometry;
	else
		pPathGeometry->Release();
	return S_OK;
}

template<class TVal = int, class TPt>
/// <summary>
/// 计算玫瑰曲线各点
/// </summary>
/// <param name="vPt">点集合</param>
/// <param name="a">花瓣长度</param>
/// <param name="n">花瓣数量参数</param>
/// <param name="fStep">步长</param>
EckInline void CalcRoseCurvePoint(std::vector<TPt>& vPt, float a = 10.f, float n = 1.f, float fStep = 0.01f)
{
	vPt.clear();
	float t = 0.f;
	float x, y;
	while (t < PiF * 2.f)
	{
		Polar2Rect(a * sinf(n * t), t, x, y);
#if ECKCXX20
		vPt.emplace_back((TVal)x, (TVal)y);
#else
		vPt.emplace_back(TPt{ (TVal)x, (TVal)y });
#endif
		t += fStep;
	}
	vPt.emplace_back(vPt.front());
}

/// <summary>
/// 画玫瑰曲线
/// </summary>
/// <param name="hDC">设备场景</param>
/// <param name="xCenter">中心点X</param>
/// <param name="yCenter">中心点Y</param>
/// <param name="a">花瓣长度</param>
/// <param name="n">花瓣数量参数</param>
/// <param name="fStep">步长</param>
/// <returns>Polyline返回值</returns>
EckInline BOOL DrawRoseCurve(HDC hDC, int xCenter, int yCenter, float a = 300.f, float n = 10.f, float fStep = 0.01f)
{
	std::vector<POINT> vPt{};
	CalcRoseCurvePoint(vPt, a, n, fStep);
	std::transform(std::execution::par_unseq, vPt.begin(), vPt.end(), vPt.begin(), [xCenter, yCenter](POINT& pt)
		{
			return POINT{ pt.x + xCenter,yCenter - pt.y };
		});
	return Polyline(hDC, vPt.data(), (int)vPt.size());
}

/// <summary>
/// 画玫瑰曲线
/// </summary>
/// <param name="pGraphics">图形对象</param>
/// <param name="pPen">画笔对象</param>
/// <param name="xCenter">中心点X</param>
/// <param name="yCenter">中心点Y</param>
/// <param name="a">花瓣长度</param>
/// <param name="n">花瓣数量参数</param>
/// <param name="fStep">步长</param>
/// <returns>GdipDrawLines返回值</returns>
EckInline GpStatus DrawRoseCurve(GpGraphics* pGraphics, GpPen* pPen, float xCenter, float yCenter,
	float a = 300.f, float n = 10.f, float fStep = 0.01f)
{
	std::vector<GpPointF> vPt{};
	CalcRoseCurvePoint<float>(vPt, a, n, fStep);
	std::transform(std::execution::par_unseq, vPt.begin(), vPt.end(), vPt.begin(), [xCenter, yCenter](GpPointF& pt)
		{
			return GpPointF{ pt.x + xCenter,yCenter - pt.y };
		});
	return GdipDrawLines(pGraphics, pPen, vPt.data(), (int)vPt.size());
}

struct DRAW_ROSECURVE_D2D_PARAM
{
	ID2D1Factory* pFactory = NULL;// D2D工厂
	ID2D1RenderTarget* pRT = NULL;// D2D渲染目标
	ID2D1Brush* pBrush = NULL;// D2D画刷
	float cxStroke = 1.f;// 笔画宽度
	ID2D1StrokeStyle* pStrokeStyle = NULL;// 笔画样式
	float xCenter = 0.f;// 中心点X
	float yCenter = 0.f;// 中心点Y
	float fDeformationCoefficient = 4.f;// 变形系数
	float a = 300.f;// 花瓣长度
	float n = 10.f;// 花瓣数量参数
	float fStep = 0.01f;// 步长
};

/// <summary>
/// 画玫瑰曲线
/// </summary>
/// <param name="Info">参数</param>
/// <param name="ppPathGeometry">接收路径几何形变量的指针</param>
/// <returns>构建路径几何形时的失败信息，无法判断绘制操作成功与否，调用方应检查EndDraw返回值</returns>
inline HRESULT DrawRoseCurve(const DRAW_ROSECURVE_D2D_PARAM& Info,
	ID2D1PathGeometry** ppPathGeometry = NULL)
{
	if (ppPathGeometry)
		*ppPathGeometry = NULL;
	std::vector<D2D1_POINT_2F> vPt{};
	CalcRoseCurvePoint<float>(vPt, Info.a, Info.n, Info.fStep);
	std::transform(std::execution::par_unseq, vPt.begin(), vPt.end(), vPt.begin(), [&Info](D2D1_POINT_2F& pt)
		{
			return D2D1_POINT_2F{ pt.x + Info.xCenter,Info.yCenter - pt.y };
		});
	ID2D1PathGeometry* pPathGeometry;
	HRESULT hr;
	if (FAILED(hr = MakePloyLinePath(vPt.data(), (int)vPt.size(), Info.pFactory, pPathGeometry)))
		return hr;
	Info.pRT->DrawGeometry(pPathGeometry, Info.pBrush, Info.cxStroke, Info.pStrokeStyle);
	if (ppPathGeometry)
		*ppPathGeometry = pPathGeometry;
	else
		pPathGeometry->Release();
	return S_OK;
}

/// <summary>
/// 计算正星形/正多边形各点
/// </summary>
/// <param name="vPt">点集合</param>
/// <param name="r">外接圆半径</param>
/// <param name="n">边数或角数</param>
/// <param name="fAngle">起始点相对X轴正方向的旋转角度</param>
/// <param name="bLinkStar">是否连接为星形</param>
template<class TVal = int, class TPt>
inline void CalcRegularStar(std::vector<TPt>& vPt, TVal r, int n, float fAngle = Deg2Rad(90.f), BOOL bLinkStar = TRUE)
{
	vPt.clear();
	const float fAngleUnit = PiF * 2.f / n;
	float fTheta = fAngle;
	TVal x, y;
	if (bLinkStar)
	{
		int i = 0;
		const int cLoop = (n % 2) ? n : (n / 2);
		EckCounterNV(cLoop)
		{
			CalcPointFromCircleAngle<TVal>(r, fTheta + fAngleUnit * i, x, y);
#if ECKCXX20
			vPt.emplace_back(x, y);
#else
			vPt.emplace_back(TPt{ x, y });
#endif
			i += 2;
			if (i >= n)
				i %= n;
		}
		vPt.emplace_back(vPt.front());

		if (n % 2 == 0)
		{
			i = 1;
			EckCounterNV(cLoop)
			{
				CalcPointFromCircleAngle<TVal>(r, fTheta + fAngleUnit * i, x, y);
#if ECKCXX20
				vPt.emplace_back(x, y);
#else
				vPt.emplace_back(TPt{ x, y });
#endif
				i += 2;
				if (i >= n)
					i %= n;
			}
			vPt.emplace_back(vPt[n / 2 + 1]);
		}
	}
	else
	{
		EckCounter(n, i)
		{
			CalcPointFromCircleAngle<TVal>(r, fTheta, x, y);
#if ECKCXX20
			vPt.emplace_back(x, y);
#else
			vPt.emplace_back(TPt{ x, y });
#endif
			fTheta += fAngleUnit;
		}
		vPt.emplace_back(vPt.front());
	}
}

/// <summary>
/// 画正星形/正多边形
/// </summary>
/// <param name="hDC">设备场景</param>
/// <param name="xCenter">中心点X</param>
/// <param name="yCenter">中心点Y</param>
/// <param name="r">外接圆半径</param>
/// <param name="n">边数或角数</param>
/// <param name="fAngle">起始点相对X轴正方向的旋转角度</param>
/// <param name="bLinkStar">是否连接为星形</param>
/// <returns>Polyline返回值</returns>
inline BOOL DrawRegularStar(HDC hDC, int xCenter, int yCenter,
	int r, int n, float fAngle = Deg2Rad(90.f), BOOL bLinkStar = TRUE)
{
	std::vector<POINT> vPt{};
	CalcRegularStar(vPt, r, n, fAngle, bLinkStar);
	std::transform(vPt.begin(), vPt.end(), vPt.begin(), [xCenter, yCenter](POINT& pt)
		{
			return POINT{ pt.x + xCenter,yCenter - pt.y };
		});

	if (n % 2 || !bLinkStar)
		return Polyline(hDC, vPt.data(), (int)vPt.size());
	else
		return Polyline(hDC, vPt.data(), (int)vPt.size() / 2) &&
		Polyline(hDC, vPt.data() + n / 2 + 1, (int)vPt.size() / 2);
}

/// <summary>
/// 画正星形/正多边形
/// </summary>
/// <param name="pGraphics">图形对象</param>
/// <param name="pPen">画笔句柄</param>
/// <param name="xCenter">中心点X</param>
/// <param name="yCenter">中心点Y</param>
/// <param name="r">外接圆半径</param>
/// <param name="n">边数或角数</param>
/// <param name="fAngle">起始点相对X轴正方向的旋转角度</param>
/// <param name="bLinkStar">是否连接为星形</param>
/// <returns>GdipDrawLines返回值</returns>
inline GpStatus DrawRegularStar(GpGraphics* pGraphics, GpPen* pPen, float xCenter, float yCenter,
	float r, int n, float fAngle = Deg2Rad(90.f), BOOL bLinkStar = TRUE)
{
	std::vector<GpPointF> vPt{};
	CalcRegularStar(vPt, r, n, fAngle, bLinkStar);
	std::transform(vPt.begin(), vPt.end(), vPt.begin(), [xCenter, yCenter](GpPointF& pt)
		{
			return GpPointF{ pt.x + xCenter,yCenter - pt.y };
		});

	if (n % 2 || !bLinkStar)
		return GdipDrawLines(pGraphics, pPen, vPt.data(), (int)vPt.size());
	else
	{
		GdipDrawLines(pGraphics, pPen, vPt.data(), (int)vPt.size() / 2);
		return GdipDrawLines(pGraphics, pPen, vPt.data() + n / 2 + 1, (int)vPt.size() / 2);
	}
}

struct DRAW_REGULARSTAR_D2D_PARAM
{
	ID2D1Factory* pFactory = NULL;// D2D工厂
	ID2D1RenderTarget* pRT = NULL;// D2D渲染目标
	ID2D1Brush* pBrush = NULL;// D2D画刷
	float cxStroke = 1.f;// 笔画宽度
	ID2D1StrokeStyle* pStrokeStyle = NULL;// 笔画样式
	float xCenter = 0.f;// 中心点X
	float yCenter = 0.f;// 中心点Y
	float r = 300;// 外接圆半径
	int n = 5;// 边数或角数
	float fAngle = Deg2Rad(90.f);// 起始点相对X轴正方向的旋转角度
	BOOL bLinkStar = TRUE;// 是否连接为星形
};

/// <summary>
/// 画正星形/正多边形
/// </summary>
/// <param name="Info">参数</param>
/// <param name="ppPathGeometry">接收路径几何形变量的指针</param>
/// <returns>构建路径几何形时的失败信息，无法判断绘制操作成功与否，调用方应检查EndDraw返回值</returns>
inline HRESULT DrawRegularStar(const DRAW_REGULARSTAR_D2D_PARAM& Info,
	ID2D1PathGeometry** ppPathGeometry = NULL)
{
	if (ppPathGeometry)
		*ppPathGeometry = NULL;
	std::vector<D2D1_POINT_2F> vPt{};
	CalcRegularStar(vPt, Info.r, Info.n, Info.fAngle, Info.bLinkStar);
	std::transform(vPt.begin(), vPt.end(), vPt.begin(), [&Info](D2D1_POINT_2F& pt)
		{
			return D2D1_POINT_2F{ pt.x + Info.xCenter,Info.yCenter - pt.y };
		});

	ID2D1PathGeometry* pPathGeometry;
	HRESULT hr = Info.pFactory->CreatePathGeometry(&pPathGeometry);
	if (!pPathGeometry)
		return hr;
	ID2D1GeometrySink* pSink;
	pPathGeometry->Open(&pSink);
	if (Info.n % 2 || !Info.bLinkStar)
	{
		pSink->BeginFigure(vPt.front(), D2D1_FIGURE_BEGIN_HOLLOW);
		pSink->AddLines(vPt.data(), (UINT32)vPt.size());
		pSink->EndFigure(D2D1_FIGURE_END_OPEN);
	}
	else
	{
		pSink->BeginFigure(vPt.front(), D2D1_FIGURE_BEGIN_HOLLOW);
		pSink->AddLines(vPt.data(), (UINT32)vPt.size() / 2);
		pSink->EndFigure(D2D1_FIGURE_END_OPEN);

		pSink->BeginFigure(vPt[Info.n / 2 + 1], D2D1_FIGURE_BEGIN_HOLLOW);
		pSink->AddLines(vPt.data() + Info.n / 2 + 1, (UINT32)vPt.size() / 2);
		pSink->EndFigure(D2D1_FIGURE_END_OPEN);
	}
	hr = pSink->Close();
	if (FAILED(hr))
	{
		pSink->Release();
		pPathGeometry->Release();
		return hr;
	}
	pSink->Release();
	Info.pRT->DrawGeometry(pPathGeometry, Info.pBrush, Info.cxStroke, Info.pStrokeStyle);
	if (ppPathGeometry)
		*ppPathGeometry = pPathGeometry;
	else
		pPathGeometry->Release();
	return S_OK;
}

/// <summary>
/// D2D取矩阵对称
/// </summary>
EckInline D2D1::Matrix3x2F D2dMatrixReflection(float A, float B, float C)
{
	const float fASqPlusBSq = A * A + B * B;
	const float t = -2.f * A * B / fASqPlusBSq;
	return D2D1::Matrix3x2F(
		1.f - 2.f * A * A / fASqPlusBSq, t,
		t, 1.f - 2.f * B * B / fASqPlusBSq,
		-2.f * A * C / fASqPlusBSq, -2.f * B * C / fASqPlusBSq);
}

/// <summary>
/// XFORM取矩阵平移
/// </summary>
EckInline constexpr XFORM XFORMTranslate(float dx, float dy)
{
	return
	{
		1.f,
		0.f,
		0.f,
		1.f,
		dx,
		dy
	};
}

/// <summary>
/// XFORM取矩阵旋转
/// </summary>
EckInline XFORM XFORMRotate(float fAngle)
{
	return
	{
		cosf(fAngle),
		sinf(fAngle),
		-sinf(fAngle),
		cosf(fAngle),
		0.f,
		0.f
	};
}

/// <summary>
/// XFORM取矩阵旋转
/// </summary>
EckInline XFORM XFORMRotate(float fAngle, float x, float y)
{
	/*
	|1   0   0|   |m11 m12 0|   |1   0   0|   |m11                m12                0|
	|0   1   0| * |m21 m22 0| * |0   1   0| = |m21                m22                0|
	|-dx -dy 1|   |0   0   1|   |dx  dy  1|   |-dx*m11-dy*m21+dx  -dx*m12-dy*m22+dy  1|
	应用到XFORM时m12与m21应互换位置
	*/
	return
	{
		cosf(fAngle),
		sinf(fAngle),
		-sinf(fAngle),
		cosf(fAngle),
		-x * cosf(fAngle) - y * sinf(fAngle) + x,
		x * sinf(fAngle) - y * cosf(fAngle) + y
	};
}

/// <summary>
/// XFORM取矩阵缩放
/// </summary>
EckInline constexpr XFORM XFORMScale(float xScale, float yScale)
{
	return
	{
		xScale,
		0.f,
		0.f,
		yScale,
		0.f,
		0.f
	};
}

/// <summary>
/// XFORM取矩阵缩放
/// </summary>
EckInline constexpr XFORM XFORMScale(float xScale, float yScale, float x, float y)
{
	return
	{
		xScale,
		0.f,
		0.f,
		yScale,
		-x * xScale + x,
		-y * yScale + y
	};
}

/// <summary>
/// XFORM取矩阵错切
/// </summary>
EckInline constexpr XFORM XFORMShear(float xFactor, float yFactor)
{
	return
	{
		1.f,
		xFactor,
		yFactor,
		1.f,
		0.f,
		0.f
	};
}

/// <summary>
/// XFORM取矩阵错切
/// </summary>
EckInline constexpr XFORM XFORMShear(float xFactor, float yFactor, float x, float y)
{
	return
	{
		1.f,
		xFactor,
		yFactor,
		1.f,
		-x - y * yFactor + x,
		-x * xFactor - y + y
	};
}

/// <summary>
/// XFORM取矩阵对称
/// </summary>
EckInline constexpr XFORM XFORMReflection(float A, float B, float C)
{
	const float fASqPlusBSq = A * A + B * B;
	const float t = -2.f * A * B / fASqPlusBSq;
	return
	{
		1.f - 2.f * A * A / fASqPlusBSq,
		t,
		t,
		1.f - 2.f * B * B / fASqPlusBSq,
		-2.f * A * C / fASqPlusBSq,
		-2.f * B * C / fASqPlusBSq
	};
}

/// <summary>
/// 从离散点计算贝塞尔曲线
/// </summary>
/// <param name="vPt">结果点集合</param>
/// <param name="pPt">离散点</param>
/// <param name="cPt">离散点数量</param>
/// <param name="K">张力</param>
template<class TVal = int, class TPt>
inline void CalcBezierControlPoints(std::vector<TPt>& vPt, const TPt* pPt, int cPt, float K = 1.f)
{
	auto pptMid = (TPt*)_malloca(cPt * sizeof(TPt));
	auto piLineLen = (TVal*)_malloca(cPt * sizeof(TVal));
	EckAssert(pptMid);
	EckAssert(piLineLen);
	vPt.reserve(cPt * 3 + 1);
	vPt.emplace_back(*pPt);// 起点
	vPt.emplace_back(*pPt);// 控点1
	TVal xMid1, yMid1, xMid2, yMid2, dx, dy;

	const auto [x2, y2] = pPt[1];
	const auto [x1, y1] = pPt[0];
	pptMid[0] = { (x1 + x2) / 2,(y1 + y2) / 2 };
	piLineLen[0] = CalcLineLength(x1, y1, x2, y2);
	for (int i = 1; i < cPt - 1; ++i)
	{
		const auto [x2, y2] = pPt[i + 1];
		const auto [x1, y1] = pPt[i];
		pptMid[i] = { (x1 + x2) / 2,(y1 + y2) / 2 };
		piLineLen[i] = CalcLineLength(x1, y1, x2, y2);
		const auto [x00, y00] = pptMid[i - 1];
		const auto [x01, y01] = pptMid[i];
		CalcPointFromLineScalePos<TVal>(x1, y1, x00, y00, K, xMid1, yMid1);
		CalcPointFromLineScalePos<TVal>(x1, y1, x01, y01, K, xMid2, yMid2);
		CalcPointFromLineScalePos<TVal>(x00, y00, x01, y01, (float)piLineLen[i - 1] / (float)(piLineLen[i] + piLineLen[i - 1]), dx, dy);
		dx = x1 - dx;
		dy = y1 - dy;
		vPt.emplace_back(xMid1 + dx, yMid1 + dy);// 控点2
		vPt.emplace_back(x1, y1);// 终点
		vPt.emplace_back(xMid2 + dx, yMid2 + dy);// 下一段曲线的控点1
	}
	vPt.emplace_back(pPt[cPt - 1]);// 控点2
	vPt.emplace_back(pPt[cPt - 1]);// 终点

	_freea(piLineLen);
	_freea(pptMid);
}

class CMifptpHBITMAP
{
private:
	HBITMAP m_hBitmap = NULL;
	BITMAP m_Bmp{};
public:
	using TColor = COLORREF;
	using TColorComp = BYTE;
	using TCoord = POINT;

	CMifptpHBITMAP() = default;
	explicit CMifptpHBITMAP(HBITMAP hBitmap) :m_hBitmap{ hBitmap } { GetObjectW(hBitmap, sizeof(m_Bmp), &m_Bmp); }

	EckInline constexpr static TCoord MakeCoord(int x, int y) { return { x,y }; }

	EckInline constexpr static BYTE GetColorComp(COLORREF cr, int k) { return ((BYTE*)&cr)[k]; }

	EckInline constexpr static TColor MakeColor(const TColorComp(&Comp)[4])
	{
		return RGB(Comp[0], Comp[1], Comp[2]) | (Comp[3] << 24);
	}

	CMifptpHBITMAP New(TCoord Dimension) const
	{
		BITMAPINFO bmi{};
		bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmi.bmiHeader.biWidth = Dimension.x;
		bmi.bmiHeader.biHeight = -Dimension.y;
		bmi.bmiHeader.biPlanes = 1;
		bmi.bmiHeader.biBitCount = 32;
#pragma warning (suppress:6387)// 可能为NULL
		const HBITMAP hbm = CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, NULL, NULL, 0);
		return CMifptpHBITMAP(hbm);
	}

	EckInline TColor GetPixel(TCoord pt) const
	{
		return *(((TColor*)(((BYTE*)m_Bmp.bmBits) + pt.y * m_Bmp.bmWidthBytes)) + pt.x);
	}

	EckInline void SetPixel(TCoord pt, TColor cr)
	{
		*(((TColor*)(((BYTE*)m_Bmp.bmBits) + pt.y * m_Bmp.bmWidthBytes)) + pt.x) = cr;
	}

	EckInline int GetWidth() const { return m_Bmp.bmWidth; }

	EckInline int GetHeight() const { return m_Bmp.bmHeight; }

	EckInline HBITMAP GetHBITMAP() const { return m_hBitmap; }

	EckInline void Lock() const {}
	EckInline void UnLock() const {}
};

class CMifptpGpBitmap
{
private:
	GpBitmap* m_pBitmap = NULL;
	GpBitmapData m_Data{};
	int m_cx = 0,
		m_cy = 0;
public:
	using TColor = ARGB;
	using TColorComp = BYTE;
	using TCoord = GpPoint;

	CMifptpGpBitmap() = default;
	explicit CMifptpGpBitmap(GpBitmap* pBitmap) :m_pBitmap{ pBitmap }
	{
		GdipGetImageWidth(pBitmap, (UINT*)&m_cx);
		GdipGetImageHeight(pBitmap, (UINT*)&m_cy);
	}

	EckInline constexpr static TCoord MakeCoord(int x, int y) { return { x,y }; }

	EckInline constexpr static TColorComp GetColorComp(TColor cr, int k) { return ((BYTE*)&cr)[k]; }

	EckInline constexpr static TColor MakeColor(const TColorComp(&Comp)[4])
	{
		return Comp[0] | (Comp[1] << 8) | (Comp[2] << 16) | (Comp[3] << 24);
	}

	CMifptpGpBitmap New(TCoord Dimension) const
	{
		GpBitmap* pBitmap;
		GdipCreateBitmapFromScan0(Dimension.x, Dimension.y, 0, GpPixelFormat::PixelFormat32bppPARGB, NULL, &pBitmap);
		return CMifptpGpBitmap(pBitmap);
	}

	EckInline TColor GetPixel(TCoord pt) const
	{
		return *(((TColor*)(((BYTE*)m_Data.Scan0) + pt.y * m_Data.Stride)) + pt.x);
	}

	EckInline void SetPixel(TCoord pt, TColor cr)
	{
		*(((TColor*)(((BYTE*)m_Data.Scan0) + pt.y * m_Data.Stride)) + pt.x) = cr;
	}

	EckInline int GetWidth() const { return m_cx; }

	EckInline int GetHeight() const { return m_cy; }

	EckInline GpBitmap* GetGpBitmap() const { return m_pBitmap; }

	EckInline void Lock()
	{
		const GpRect rc{ 0,0,m_cx,m_cy };
		GdipBitmapLockBits(m_pBitmap, &rc, GpImageLockMode::ImageLockModeRead | GpImageLockMode::ImageLockModeWrite,
			GpPixelFormat::PixelFormat32bppPARGB, &m_Data);
	}

	EckInline void UnLock()
	{
		GdipBitmapUnlockBits(m_pBitmap, &m_Data);
	}
};

/// <summary>
/// 生成扭曲图像。
/// 生成从原图像上的多边形区域映射到目标多边形区域的扭曲图像，函数将多边形的外接矩形与新位图的(0,0)对齐
/// </summary>
/// <param name="Bmp">输入位图处理器</param>
/// <param name="NewBmp">结果位图处理器，应自行释放相关资源</param>
/// <param name="pptSrc">源多边形区域</param>
/// <param name="pptDst">目标多边形区域</param>
/// <param name="cPt">顶点数</param>
/// <returns></returns>
template<class TBmpHandler, class TCoord>
inline BOOL MakeImageFromPolygonToPolygon(TBmpHandler& Bmp, TBmpHandler& NewBmp,
	const TCoord* pptSrc, const TCoord* pptDst, int cPt)
{
	static_assert(std::is_same_v<TCoord, typename TBmpHandler::TCoord>);
	if (cPt < 3)
		return FALSE;
	const auto [itMinY, itMaxY] = std::minmax_element(pptDst, pptDst + cPt, [](const TCoord& pt1, const TCoord& pt2)->bool
		{
			return pt1.y < pt2.y;
		});
	const auto [itMinX, itMaxX] = std::minmax_element(pptDst, pptDst + cPt, [](const TCoord& pt1, const TCoord& pt2)->bool
		{
			return pt1.x < pt2.x;
		});
	const int cyPolygon = itMaxY->y - itMinY->y + 1;
	const int cxPolygon = itMaxX->x - itMinX->x + 1;
	if (cxPolygon <= 0 || cyPolygon <= 0)
		return FALSE;

	NewBmp = Bmp.New(TBmpHandler::MakeCoord(cxPolygon, cyPolygon));
	std::vector<TCoord> vPtDst(cPt);
	EckCounter(cPt, i)
		vPtDst[i] = TCoord{ pptDst[i].x - itMinX->x,pptDst[i].y - itMinY->y };
	const int yMax = vPtDst[std::distance(pptDst, itMaxY)].y;

	struct EDGE
	{
		float x;
		float dx;
		float Rx;
		float Ry;
		float dRx;
		float dRy;
		int yMax;
		int yMin;
	};

	std::unordered_map<int, std::vector<EDGE*>> ET(cyPolygon);
	using TETIt = typename std::unordered_map<int, std::vector<EDGE*>>::iterator;
	EckCounter(cPt, i)
	{
		const auto& pt1 = vPtDst[i];
		const auto& pt2 = vPtDst[(i + 1) % cPt];
		const auto& ptSrc1 = pptSrc[i];
		const auto& ptSrc2 = pptSrc[(i + 1) % cPt];
		if (pt1.y == pt2.y)
			continue;
		auto p = new EDGE;
		int yMax, yMin;
		if (pt1.y < pt2.y)
		{
			p->x = (float)pt1.x;
			p->Rx = (float)ptSrc1.x;
			p->Ry = (float)ptSrc1.y;
			yMax = (int)pt2.y;
			yMin = (int)pt1.y;
		}
		else
		{
			p->x = (float)pt2.x;
			p->Rx = (float)ptSrc2.x;
			p->Ry = (float)ptSrc2.y;
			yMax = (int)pt1.y;
			yMin = (int)pt2.y;
		}
		p->dx = (float)(pt1.x - pt2.x) / (float)(pt1.y - pt2.y);
		p->yMax = yMax;
		p->yMin = yMin;
		p->dRx = (float)(ptSrc1.x - ptSrc2.x) / (float)(pt1.y - pt2.y);
		p->dRy = (float)(ptSrc1.y - ptSrc2.y) / (float)(pt1.y - pt2.y);
		ET[yMin].emplace_back(p);
	}

	for (auto& x : ET)
	{
		std::sort(x.second.begin(), x.second.end(), [](const EDGE* p1, const EDGE* p2)->bool
			{
				if (p1->x == p2->x)
					return p1->dx < p2->dx;
				else
					return p1->x < p2->x;
			});
	}

	std::vector<EDGE*> AEL{};
	std::vector<size_t> vNeedDel{};

	Bmp.Lock();
	NewBmp.Lock();
	for (int y = 0; y <= yMax; ++y)
	{
		if (TETIt it; (it = ET.find(y)) != ET.end())
		{
			AEL.insert(AEL.end(), it->second.begin(), it->second.end());
			std::sort(AEL.begin(), AEL.end(), [](const EDGE* p1, const EDGE* p2)->bool
				{
					if (p1->x == p2->x)
						return p1->dx < p2->dx;
					else
						return p1->x < p2->x;
				});
		}
		if (!AEL.empty())
		{
			vNeedDel.clear();
			EckCounter(AEL.size(), i)
			{
				if (y == AEL[i]->yMax)
					vNeedDel.emplace_back(i);
			}
			for (auto it = vNeedDel.rbegin(); it < vNeedDel.rend(); ++it)
				AEL.erase(AEL.begin() + *it);
			EckCounter(AEL.size() / 2, i)
			{
				const auto pL = AEL[i * 2];
				const auto pR = AEL[i * 2 + 1];
				const float dRxx = (pL->Rx - pR->Rx) / (float)(pL->x - pR->x);
				const float dRyy = (pL->Ry - pR->Ry) / (float)(pL->x - pR->x);
				float Rxx = pL->Rx;
				float Ryy = pL->Ry;
				for (int x = (int)pL->x; x <= (int)pR->x; ++x)
				{
					int x0 = (int)floorf(Rxx);
					float fRateX = Rxx - x0;
					if (x0 < 0)
					{
						x0 = 0;
						fRateX = 0.f;
					}
					else if (x0 >= Bmp.GetWidth() - 1)
					{
						x0 = Bmp.GetWidth() - 2;
						fRateX = 1.f;
					}
					int y0 = (int)floorf(Ryy);
					float fRateY = Ryy - y0;
					if (y0 < 0)
					{
						y0 = 0;
						fRateY = 0.f;
					}
					else if (y0 >= Bmp.GetHeight() - 1)
					{
						y0 = Bmp.GetHeight() - 2;
						fRateY = 1.f;
					}
					fRateX = 1.f - fRateX;
					fRateY = 1.f - fRateY;
					const typename TBmpHandler::TColor cr[4]
					{
						Bmp.GetPixel(TBmpHandler::MakeCoord(x0,y0)),
						Bmp.GetPixel(TBmpHandler::MakeCoord(x0 + 1,y0)),
						Bmp.GetPixel(TBmpHandler::MakeCoord(x0,y0 + 1)),
						Bmp.GetPixel(TBmpHandler::MakeCoord(x0 + 1,y0 + 1)),
					};

					typename TBmpHandler::TColorComp crNew[4];
					EckCounter(4, k)
					{
						crNew[k] = (typename TBmpHandler::TColorComp)(
							TBmpHandler::GetColorComp(cr[0], k) * fRateX * fRateY +
							TBmpHandler::GetColorComp(cr[1], k) * (1 - fRateX) * fRateY +
							TBmpHandler::GetColorComp(cr[2], k) * fRateX * (1 - fRateY) +
							TBmpHandler::GetColorComp(cr[3], k) * (1 - fRateX) * (1 - fRateY));
					}

					NewBmp.SetPixel(TBmpHandler::MakeCoord(x, y), TBmpHandler::MakeColor(crNew));
					Rxx += dRxx;
					Ryy += dRyy;
				}
			}

			for (auto e : AEL)
			{
				e->x += e->dx;
				e->Rx += e->dRx;
				e->Ry += e->dRy;
			}
		}
	}
	NewBmp.UnLock();
	Bmp.UnLock();
	return TRUE;
}

enum
{
	FGRF_TOPTOBOTTOM = 1,// 从上到下
	FGRF_BOTTOMTOTOP,// 从下到上
	FGRF_LEFTTORIGHT,// 从左到右
	FGRF_RIGHTTOLEFT,// 从右到左
	FGRF_TOPLEFTTOBOTTOMRIGHT,// 从左上到右下↘
	FGRF_BOTTOMRIGHTTOTOPLEFT,// 从右下到左上↖
	FGRF_BOTTOMLEFTTOTOPRIGHT,// 从左下到右上↗
	FGRF_TOPRIGHTTOBOTTOMLEFT,// 从右上到左下↙
};

/// <summary>
/// 填充渐变矩形
/// </summary>
/// <param name="hDC">设备场景</param>
/// <param name="rc">矩形</param>
/// <param name="crGradient">渐变色，至少指向3个COLORREF</param>
/// <param name="iGradientMode">渐变模式，FGRF_常量</param>
/// <returns>GradientFill的返回值</returns>
inline BOOL FillGradientRect(HDC hDC, const RECT& rc, COLORREF crGradient[], int iGradientMode)
{
	EckAssert(iGradientMode >= 1 && iGradientMode <= 8);
	const int
		cx = rc.right - rc.left,
		cy = rc.bottom - rc.top;
	if (iGradientMode >= FGRF_TOPTOBOTTOM && iGradientMode <= FGRF_RIGHTTOLEFT)
	{
		TRIVERTEX tv[4];
		COLORREF cr1, cr2, cr3;
		ULONG uMode;
		switch (iGradientMode)
		{
		case FGRF_TOPTOBOTTOM:// 从上到下
		case FGRF_BOTTOMTOTOP:// 从下到上
		{
			cr2 = crGradient[1];
			tv[0].x = 0;
			tv[0].y = 0;
			tv[1].x = cx;
			tv[1].y = cy / 2;
			tv[2].x = 0;
			tv[2].y = cy / 2;
			tv[3].x = cx;
			tv[3].y = cy;
			uMode = GRADIENT_FILL_RECT_V;
			if (iGradientMode == 1)
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
		case FGRF_LEFTTORIGHT:// 从左到右
		case FGRF_RIGHTTOLEFT:// 从右到左
		{
			cr2 = crGradient[1];
			tv[0].x = 0;
			tv[0].y = 0;
			tv[1].x = cx / 2;
			tv[1].y = cy;
			tv[2].x = cx / 2;
			tv[2].y = 0;
			tv[3].x = cx;
			tv[3].y = cy;
			uMode = GRADIENT_FILL_RECT_H;
			if (iGradientMode == 3)
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
		default:
			__assume(0);
		}

		tv[0].Red = GetRValue(cr1) << 8;
		tv[0].Green = GetGValue(cr1) << 8;
		tv[0].Blue = GetBValue(cr1) << 8;
		tv[0].Alpha = 0xFF << 8;

		tv[1].Red = GetRValue(cr2) << 8;
		tv[1].Green = GetGValue(cr2) << 8;
		tv[1].Blue = GetBValue(cr2) << 8;
		tv[1].Alpha = 0xFF << 8;

		tv[2].Red = tv[1].Red;
		tv[2].Green = tv[1].Green;
		tv[2].Blue = tv[1].Blue;
		tv[2].Alpha = 0xFF << 8;

		tv[3].Red = GetRValue(cr3) << 8;
		tv[3].Green = GetGValue(cr3) << 8;
		tv[3].Blue = GetBValue(cr3) << 8;
		tv[3].Alpha = 0xFF << 8;

		GRADIENT_RECT gr[2];
		gr[0].UpperLeft = 0;
		gr[0].LowerRight = 1;
		gr[1].UpperLeft = 2;
		gr[1].LowerRight = 3;
		return GradientFill(hDC, tv, ARRAYSIZE(tv), &gr, ARRAYSIZE(gr), uMode);
	}
	else
	{
		TRIVERTEX tv[4];
		// 左上
		tv[0].x = 0;
		tv[0].y = 0;
		// 左下
		tv[1].x = 0;
		tv[1].y = cy;
		// 右上
		tv[2].x = cx;
		tv[2].y = 0;
		// 右下
		tv[3].x = cx;
		tv[3].y = cy;
		COLORREF cr1, cr2, cr3;

		GRADIENT_TRIANGLE gt[2];
		switch (iGradientMode)
		{
		case 5:// 左上到右下↘
		case 6:// 右下到左上↖
		{
			gt[0].Vertex1 = 0;
			gt[0].Vertex2 = 1;
			gt[0].Vertex3 = 2;
			gt[1].Vertex1 = 3;
			gt[1].Vertex2 = 1;
			gt[1].Vertex3 = 2;
			cr2 = crGradient[1];
			if (iGradientMode == 5)
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

			tv[2].Red = tv[1].Red;
			tv[2].Green = tv[1].Green;
			tv[2].Blue = tv[1].Blue;
			tv[2].Alpha = 0xFF << 8;

			tv[3].Red = GetRValue(cr3) << 8;
			tv[3].Green = GetGValue(cr3) << 8;
			tv[3].Blue = GetBValue(cr3) << 8;
			tv[3].Alpha = 0xFF << 8;
		}
		break;
		case 7:// 左下到右上↗
		case 8:// 右上到左下↙
		{
			gt[0].Vertex1 = 1;
			gt[0].Vertex2 = 0;
			gt[0].Vertex3 = 3;
			gt[1].Vertex1 = 2;
			gt[1].Vertex2 = 0;
			gt[1].Vertex3 = 3;
			cr2 = crGradient[1];
			if (iGradientMode == 7)
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

			tv[3].Red = tv[0].Red;
			tv[3].Green = tv[0].Green;
			tv[3].Blue = tv[0].Blue;
			tv[3].Alpha = 0xFF << 8;

			tv[2].Red = GetRValue(cr3) << 8;
			tv[2].Green = GetGValue(cr3) << 8;
			tv[2].Blue = GetBValue(cr3) << 8;
			tv[2].Alpha = 0xFF << 8;
		}
		break;
		default:
			__assume(0);
		}

		return GradientFill(hDC, tv, ARRAYSIZE(tv), gt, ARRAYSIZE(gt), GRADIENT_FILL_TRIANGLE);
	}
}

inline BOOL FillGradientRect(HDC hDC, const RECT& rc, COLORREF cr1, COLORREF cr2, BOOL bV)
{
	TRIVERTEX tv[2] = { 0 };
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
	gr.UpperLeft = 0;// 左上角坐标为第一个成员
	gr.LowerRight = 1;// 右下角坐标为第二个成员

	return GradientFill(hDC, tv, 2, &gr, 1, bV ? GRADIENT_FILL_RECT_V : GRADIENT_FILL_RECT_H);
}

enum
{
	DBGIF_TOPLEFT,// 左上
	DBGIF_TILE,// 平铺
	DBGIF_CENTER,// 居中
	DBGIF_STRETCH,// 缩放
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
	int iMode, BOOL bFullRgnImage)
{
	constexpr BLENDFUNCTION bf{ AC_SRC_OVER,0,255,AC_SRC_ALPHA };
	const int
		cx = rc.right - rc.left,
		cy = rc.bottom - rc.top;
	switch (iMode)
	{
	case DBGIF_TOPLEFT:// 居左上
	{
		if (bFullRgnImage)
		{
			if (!cyImage || !cxImage)
				break;
			int cxRgn, cyRgn;

			cxRgn = cy * cxImage / cyImage;
			if (cxRgn < cx)// 先尝试y对齐，看x方向是否充满窗口
			{
				cxRgn = cx;
				cyRgn = cx * cyImage / cxImage;
			}
			else
				cyRgn = cy;

			return AlphaBlend(hDC, 0, 0, cxRgn, cyRgn, hdcBitmap, 0, 0, cxImage, cyImage, bf);
		}
		else
			return AlphaBlend(hDC, 0, 0, cxImage, cyImage, hdcBitmap, 0, 0, cxImage, cyImage, bf);
	}
	break;
	case DBGIF_TILE:// 平铺
	{
		BOOL b = TRUE;
		for (int i = 0; i < (cx - 1) / cxImage + 1; ++i)
			for (int j = 0; j < (cy - 1) / cyImage + 1; ++j)
				b = AlphaBlend(hDC, i * cxImage, j * cyImage, cxImage, cyImage,
					hdcBitmap, 0, 0, cxImage, cyImage, bf) && b;
		return b;
	}
	break;
	case DBGIF_CENTER:// 居中
	{
		if (bFullRgnImage)
		{
			if (!cyImage || !cxImage)
				break;
			int cxRgn, cyRgn;
			int x, y;

			cxRgn = cy * cxImage / cyImage;
			if (cxRgn < cx)// 先尝试y对齐，看x方向是否充满窗口
			{
				cxRgn = cx;
				cyRgn = cx * cyImage / cxImage;
				x = 0;
				y = (cy - cyRgn) / 2;
			}
			else
			{
				cyRgn = cy;
				x = (cx - cxRgn) / 2;
				y = 0;
			}

			return AlphaBlend(hDC, x, y, cxRgn, cyRgn, hdcBitmap, 0, 0, cxImage, cyImage, bf);
		}
		else
			return AlphaBlend(hDC, (cx - cxImage) / 2, (cy - cyImage) / 2, cxImage, cyImage,
				hdcBitmap, 0, 0, cxImage, cyImage, bf);
	}
	break;
	case DBGIF_STRETCH:// 缩放
		return AlphaBlend(hDC, 0, 0, cx, cy, hdcBitmap, 0, 0, cxImage, cyImage, bf);
		break;
	default:
		EckDbgBreak();
		EckDbgPrintWithPos(L"无效标志");
	}
	return FALSE;
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
	int iMode, BOOL bFullRgnImage)
{
	const int
		cx = rc.right - rc.left,
		cy = rc.bottom - rc.top;
	switch (iMode)
	{
	case DBGIF_TOPLEFT:// 居左上
	{
		if (bFullRgnImage)
		{
			if (!cyImage || !cxImage)
				break;
			int cxRgn, cyRgn;

			cxRgn = cy * cxImage / cyImage;
			if (cxRgn < cx)// 先尝试y对齐，看x方向是否充满窗口
			{
				cxRgn = cx;
				cyRgn = cx * cyImage / cxImage;
			}
			else
				cyRgn = cy;

			return BitBlt(hDC, 0, 0, cxRgn, cyRgn, hdcBitmap, 0, 0, SRCCOPY);
		}
		else
			return BitBlt(hDC, 0, 0, cxImage, cyImage, hdcBitmap, 0, 0, SRCCOPY);
	}
	break;
	case DBGIF_TILE:// 平铺
	{
		BOOL b = TRUE;
		for (int i = 0; i < (cx - 1) / cxImage + 1; ++i)
			for (int j = 0; j < (cy - 1) / cyImage + 1; ++j)
				b = BitBlt(hDC, i * cxImage, j * cyImage, cxImage, cyImage,
					hdcBitmap, 0, 0, SRCCOPY) && b;
		return b;
	}
	case DBGIF_CENTER:// 居中
	{
		if (bFullRgnImage)
		{
			if (!cyImage || !cxImage)
				break;
			int cxRgn, cyRgn;
			int x, y;

			cxRgn = cy * cxImage / cyImage;
			if (cxRgn < cx)// 先尝试y对齐，看x方向是否充满窗口
			{
				cxRgn = cx;
				cyRgn = cx * cyImage / cxImage;
				x = 0;
				y = (cy - cyRgn) / 2;
			}
			else
			{
				cyRgn = cy;
				x = (cx - cxRgn) / 2;
				y = 0;
			}

			return BitBlt(hDC, x, y, cxRgn, cyRgn, hdcBitmap, 0, 0, SRCCOPY);
		}
		else
			return BitBlt(hDC, (cx - cxImage) / 2, (cy - cyImage) / 2, cxImage, cyImage,
				hdcBitmap, 0, 0, SRCCOPY);
	}
	break;
	case DBGIF_STRETCH:// 缩放
		return BitBlt(hDC, 0, 0, cx, cy, hdcBitmap, 0, 0, SRCCOPY);
		break;
	default:
		EckDbgBreak();
		EckDbgPrintWithPos(L"无效标志");
	}
	return FALSE;
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
	pBmpEffect->CopyFromBitmap(NULL, pBmp, &rcU);

	pFxBlur->SetInput(0, pBmpEffect);

	const auto iBlend = pDC->GetPrimitiveBlend();
	pDC->SetPrimitiveBlend(D2D1_PRIMITIVE_BLEND_COPY);
	pDC->DrawImage(pFxBlur, { rc.left,rc.top });
	pDC->SetPrimitiveBlend(iBlend);

	pBmpEffect->Release();
	pFxBlur->Release();
	return S_OK;
}

inline HRESULT BlurD2dDC(ID2D1DeviceContext* pDC, ID2D1Bitmap* pBmp, ID2D1Bitmap*& pBmpWork,
	const D2D1_RECT_F& rc, D2D1_POINT_2F ptDrawing, float fDeviation = 3.f)
{
	const D2D1_SIZE_U sizeNew{ (UINT32)(rc.right - rc.left), (UINT32)(rc.bottom - rc.top) };
	const auto sizeOld = pBmpWork ? pBmpWork->GetSize() : D2D1_SIZE_F{};
	HRESULT hr;

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
		D2D1::RectF(0.f, 0.f, (float)sizeNew.width, (float)sizeNew.height));

	if (sizeNew.width > sizeOld.width || sizeNew.height > sizeOld.height)
	{
		SafeRelease(pBmpWork);
		float xDpi, yDpi;
		pBmp->GetDpi(&xDpi, &yDpi);
		if (FAILED(hr = pDC->CreateBitmap(sizeNew,
			D2D1::BitmapProperties(pBmp->GetPixelFormat(), xDpi, yDpi), &pBmpWork)))
		{
			pFxBlur->Release();
			pFxCrop->Release();
			return hr;
		}
	}

	const D2D1_RECT_U rcU{ (UINT32)rc.left, (UINT32)rc.top, (UINT32)rc.right, (UINT32)rc.bottom };
	pBmpWork->CopyFromBitmap(NULL, pBmp, &rcU);

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

/// <summary>
/// DW文本布局到路径几何形
/// </summary>
/// <param name="pLayout">DW文本布局</param>
/// <param name="pRT">要在其上呈现的渲染目标</param>
/// <param name="x">起始X</param>
/// <param name="y">起始Y</param>
/// <param name="pPathGeometry">结果路径几何形</param>
/// <param name="pD2dFactory">D2D工厂，若为空则使用ECK工厂</param>
/// <returns>HRESULT</returns>
inline HRESULT GetTextLayoutPathGeometry(IDWriteTextLayout* pLayout, ID2D1RenderTarget* pRT,
	float x, float y, ID2D1PathGeometry*& pPathGeometry, ID2D1Factory* pD2dFactory = NULL)
{
	class CSinkForwarder :public ID2D1SimplifiedGeometrySink
	{
	private:
		LONG m_cRef = 1;
		ID2D1SimplifiedGeometrySink* m_pSink = NULL;
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
		ID2D1RenderTarget* m_pRT = NULL;
		ID2D1Factory* m_pFactory = NULL;
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

	if (!pD2dFactory)
		pD2dFactory = g_pD2dFactory;

	HRESULT hr;
	ID2D1PathGeometry* pPath;
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
		pPathGeometry = NULL;
	}
	else
		pPathGeometry = pPath;
	return hr;
}
ECK_NAMESPACE_END