#include "ImageHelper.h"

ECK_NAMESPACE_BEGIN
HBITMAP CreateHBITMAP(PCVOID pData, SIZE_T cbData)
{
	HBITMAP hbm;
	GpBitmap* pBitmap;
	IStream* pStream = new IStreamView(pData, cbData);
	if (!pStream)
		return NULL;

	if (GdipCreateBitmapFromStream(pStream, &pBitmap) != GpStatus::GpOk)
	{
		pStream->Release();
		return NULL;
	}

	if (GdipCreateHBITMAPFromBitmap(pBitmap, &hbm, 0))
	{
		GdipDisposeImage(pBitmap);
		pStream->Release();
		return NULL;
	}

	GdipDisposeImage(pBitmap);
	pStream->Release();
	return hbm;
}

HBITMAP CreateHBITMAP(PCWSTR pszFile)
{
	HBITMAP hbm;
	GpBitmap* pBitmap;

	if (GdipCreateBitmapFromFile(pszFile, &pBitmap) != GpStatus::GpOk)
	{
		return NULL;
	}

	if (GdipCreateHBITMAPFromBitmap(pBitmap, &hbm, 0))
	{
		GdipDisposeImage(pBitmap);
		return NULL;
	}

	GdipDisposeImage(pBitmap);
	return hbm;
}

HBITMAP CreateHBITMAP(IWICBitmap* pBmp)
{
	UINT cx, cy;
	pBmp->GetSize(&cx, &cy);
	BITMAPINFO bmi{};
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = (LONG)cx;
	bmi.bmiHeader.biHeight = -(LONG)cy;// 自上而下位图
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	void* pDibBits;
	const HDC hDC = GetDC(NULL);
	const HBITMAP hBitmap = CreateDIBSection(hDC, &bmi, 0, &pDibBits, NULL, 0);
	ReleaseDC(NULL, hDC);

	pBmp->CopyPixels(NULL, cx * 4, cx * cy * 4, (BYTE*)pDibBits);// GDI将每行位图数据对齐到DWORD
	return hBitmap;
}

HICON CreateHICON(PCVOID pData, SIZE_T cbData)
{
	HICON hIcon;
	GpBitmap* pBitmap;
	IStream* pStream = new IStreamView(pData, cbData);
	if (!pStream)
		return NULL;

	if (GdipCreateBitmapFromStream(pStream, &pBitmap) != GpStatus::GpOk)
	{
		pStream->Release();
		return NULL;
	}

	if (GdipCreateHICONFromBitmap(pBitmap, &hIcon))
	{
		GdipDisposeImage(pBitmap);
		pStream->Release();
		return NULL;
	}

	GdipDisposeImage(pBitmap);
	pStream->Release();
	return hIcon;
}

HICON CreateHICON(PCWSTR pszFile)
{
	HICON hIcon;
	GpBitmap* pBitmap;

	if (GdipCreateBitmapFromFile(pszFile, &pBitmap) != GpStatus::GpOk)
	{
		return NULL;
	}

	if (GdipCreateHICONFromBitmap(pBitmap, &hIcon))
	{
		GdipDisposeImage(pBitmap);
		return NULL;
	}

	GdipDisposeImage(pBitmap);
	return hIcon;
}

HICON CreateHICON(IWICBitmap* pBmp, HBITMAP hbmMask)
{
	const HBITMAP hbmColor = CreateHBITMAP(pBmp);
	ICONINFO ii{};
	if (!hbmMask)
	{
		UINT cx, cy;
		pBmp->GetSize(&cx, &cy);
		ii.hbmMask = CreateBitmap(cx, cy, 1, 1, NULL);
		const HDC hCDC = CreateCompatibleDC(NULL);
		const HGDIOBJ hOld = SelectObject(hCDC, ii.hbmMask);
		const RECT rc{ 0,0,(long)cx,(long)cy };
		FillRect(hCDC, &rc, (HBRUSH)GetStockObject(WHITE_BRUSH));
		SelectObject(hCDC, hOld);
		DeleteDC(hCDC);
	}
	else
		ii.hbmMask = hbmMask;
	ii.fIcon = TRUE;
	ii.hbmColor = hbmColor;
	const HICON hIcon = CreateIconIndirect(&ii);
	DeleteObject(hbmColor);
	if (!hbmMask)
		DeleteObject(ii.hbmMask);
	return hIcon; 
}

static HRESULT MakePloyLinePath(const D2D1_POINT_2F* pPt, int cPt,
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

HRESULT DrawSpirograph(const DRAW_SPIROGRAPH_D2D_PARAM& Info, ID2D1PathGeometry** ppPathGeometry)
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

void CalcDistortMatrix(const D2D1_RECT_F& rcOrg, const D2D1_POINT_2F(&ptDistort)[4], D2D1_MATRIX_4X4_F& MatrixResult)
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

HRESULT DrawButterflyCurve(const DRAW_BUTTERFLYCURVE_D2D_PARAM& Info, ID2D1PathGeometry** ppPathGeometry)
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

HRESULT DrawRoseCurve(const DRAW_ROSECURVE_D2D_PARAM& Info, ID2D1PathGeometry** ppPathGeometry)
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

HRESULT EzD2D(const EZD2D_PARAM& Param, ID2D1DeviceContext*& pDC_, IDXGISwapChain1*& pSwapChain_, ID2D1Bitmap1*& pBitmap_)
{
	pDC_ = NULL;
	pSwapChain_ = NULL;
	pBitmap_ = NULL;

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

	ID2D1DeviceContext* pDC;
	IDXGISwapChain1* pSwapChain;
	ID2D1Bitmap1* pBitmap;

	HRESULT hr;
	if (FAILED(hr = Param.pDxgiFactory->CreateSwapChainForHwnd(Param.pDxgiDevice, Param.hWnd,
		&DxgiSwapChainDesc, NULL, NULL, &pSwapChain)))
	{
		EckDbgPrintFormatMessage(hr);
		EckDbgBreak();
		return hr;
	}

	if (FAILED(hr = Param.pD2dDevice->CreateDeviceContext(Param.uDcOptions, &pDC)))
	{
		pSwapChain->Release();
		EckDbgPrintFormatMessage(hr);
		EckDbgBreak();
		return hr;
	}

	IDXGISurface1* pSurface;
	if (FAILED(hr = pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pSurface))))
	{
		pSwapChain->Release();
		pDC->Release();
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

	if (FAILED(hr = pDC->CreateBitmapFromDxgiSurface(pSurface, &D2dBmpProp, &pBitmap)))
	{
		pSwapChain->Release();
		pDC->Release();
		pSurface->Release();
		EckDbgPrintFormatMessage(hr);
		EckDbgBreak();
		return hr;
	}

	pSurface->Release();
	pDC->SetTarget(pBitmap);

	pDC_ = pDC;
	pSwapChain_ = pSwapChain;
	pBitmap_ = pBitmap;
	return S_OK;
}

HRESULT EzD2dReSize(ID2D1DeviceContext* pDC, IDXGISwapChain1* pSwapChain, ID2D1Bitmap1*& pBitmap,
	UINT cBuffer, int cx, int cy, UINT uSwapChainFlags, D2D1_ALPHA_MODE uBmpAlphaMode, D2D1_BITMAP_OPTIONS uBmpOptions)
{
	pDC->SetTarget(NULL);
	pBitmap->Release();
	pBitmap = NULL;
	HRESULT hr;
	if (FAILED(hr = pSwapChain->ResizeBuffers(cBuffer, cx, cy, DXGI_FORMAT_UNKNOWN, uSwapChainFlags)))
	{
		EckDbgPrintFormatMessage(hr);
		EckDbgBreak();
		return hr;
	}

	IDXGISurface1* pSurface;
	hr = pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pSurface));
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

	if (FAILED(hr = pDC->CreateBitmapFromDxgiSurface(pSurface, &D2dBmpProp, &pBitmap)))
	{
		EckDbgPrintFormatMessage(hr);
		EckDbgBreak();
		return hr;
	}

	pSurface->Release();
	pDC->SetTarget(pBitmap);
	return S_OK;
}

HRESULT CreateWicBitmap(std::vector<IWICBitmap*>& vResult, IWICBitmapDecoder* pDecoder, IWICImagingFactory* pWicFactory)
{
	vResult.clear();
	HRESULT hr;
	IWICBitmapFrameDecode* pFrameDecoder;
	IWICFormatConverter* pConverter;

	UINT cFrame;
	pDecoder->GetFrameCount(&cFrame);
	vResult.resize(cFrame);
	EckCounter(cFrame, i)
	{
		hr = pDecoder->GetFrame(i, &pFrameDecoder);
		if (FAILED(hr))
		{
			EckDbgPrintFormatMessage(hr);
			EckDbgBreak();
			return hr;
		}

		hr = pWicFactory->CreateFormatConverter(&pConverter);
		if (FAILED(hr))
		{
			EckDbgPrintFormatMessage(hr);
			EckDbgBreak();
			return hr;
		}

		hr = pConverter->Initialize(pFrameDecoder, GUID_WICPixelFormat32bppPBGRA,
			WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeMedianCut);
		if (FAILED(hr))
		{
			EckDbgPrintFormatMessage(hr);
			EckDbgBreak();
			return hr;
		}

		IWICBitmap* pWicBitmap;
		hr = pWicFactory->CreateBitmapFromSource(pConverter, WICBitmapNoCache, &pWicBitmap);
		if (FAILED(hr))
		{
			EckDbgPrintFormatMessage(hr);
			EckDbgBreak();
			return hr;
		}

		pConverter->Release();
		vResult[i] = pWicBitmap;
		pFrameDecoder->Release();
	}

	return S_OK;
}

BOOL DrawRegularStar(HDC hDC, int xCenter, int yCenter, int r, int n, float fAngle, BOOL bLinkStar)
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

GpStatus DrawRegularStar(GpGraphics* pGraphics, GpPen* pPen, float xCenter, float yCenter,
	float r, int n, float fAngle, BOOL bLinkStar)
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

HRESULT DrawRegularStar(const DRAW_REGULARSTAR_D2D_PARAM& Info, ID2D1PathGeometry** ppPathGeometry)
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
		pSink->AddLines(vPt.data(), (UINT32)vPt.size()/2);
		pSink->EndFigure(D2D1_FIGURE_END_OPEN);

		pSink->BeginFigure(vPt[Info.n / 2 + 1], D2D1_FIGURE_BEGIN_HOLLOW);
		pSink->AddLines(vPt.data() + Info.n / 2 + 1, (UINT32)vPt.size()/2);
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

ECK_NAMESPACE_END