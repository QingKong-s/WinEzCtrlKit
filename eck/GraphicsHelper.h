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
		return *this;
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

	EckInline constexpr static int GetX(TCoord c) { return c.x; }
	EckInline constexpr static int GetY(TCoord c) { return c.y; }
};

class CMifptpGpBitmap
{
private:
	GpBitmap* m_pBitmap = NULL;
	Gdiplus::BitmapData m_Data{};
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

	EckInline static TCoord MakeCoord(int x, int y) { return { x,y }; }

	EckInline constexpr static TColorComp GetColorComp(TColor cr, int k) { return ((BYTE*)&cr)[k]; }

	EckInline constexpr static TColor MakeColor(const TColorComp(&Comp)[4])
	{
		return Comp[0] | (Comp[1] << 8) | (Comp[2] << 16) | (Comp[3] << 24);
	}

	CMifptpGpBitmap New(TCoord Dimension) const
	{
		GpBitmap* pBitmap;
		GdipCreateBitmapFromScan0(Dimension.X, Dimension.Y, 0, PixelFormat32bppARGB, NULL, &pBitmap);
		return CMifptpGpBitmap(pBitmap);
	}

	EckInline TColor GetPixel(TCoord pt) const
	{
		return *(((TColor*)(((BYTE*)m_Data.Scan0) + pt.Y * m_Data.Stride)) + pt.X);
	}

	EckInline void SetPixel(TCoord pt, TColor cr)
	{
		*(((TColor*)(((BYTE*)m_Data.Scan0) + pt.Y * m_Data.Stride)) + pt.X) = cr;
	}

	EckInline int GetWidth() const { return m_cx; }

	EckInline int GetHeight() const { return m_cy; }

	EckInline GpBitmap* GetGpBitmap() const { return m_pBitmap; }

	EckInline void Lock()
	{
		const GpRect rc{ 0,0,m_cx,m_cy };
		GdipBitmapLockBits(m_pBitmap, &rc, Gdiplus::ImageLockModeRead | Gdiplus::ImageLockModeWrite,
			PixelFormat32bppPARGB, &m_Data);
	}

	EckInline void UnLock()
	{
		GdipBitmapUnlockBits(m_pBitmap, &m_Data);
	}

	EckInline static int GetX(TCoord c) { return c.X; }
	EckInline static int GetY(TCoord c) { return c.Y; }
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
			return TBmpHandler::GetY(pt1) < TBmpHandler::GetY(pt2);
		});
	const auto [itMinX, itMaxX] = std::minmax_element(pptDst, pptDst + cPt, [](const TCoord& pt1, const TCoord& pt2)->bool
		{
			return TBmpHandler::GetX(pt1) < TBmpHandler::GetX(pt2);
		});
	const int cyPolygon = TBmpHandler::GetY(*itMaxY) - TBmpHandler::GetY(*itMinY) + 1;
	const int cxPolygon = TBmpHandler::GetX(*itMaxX) - TBmpHandler::GetX(*itMinX) + 1;
	if (cxPolygon <= 0 || cyPolygon <= 0)
		return FALSE;

	NewBmp = Bmp.New(TBmpHandler::MakeCoord(cxPolygon, cyPolygon));
	std::vector<TCoord> vPtDst(cPt);
	EckCounter(cPt, i)
	{
		vPtDst[i] = TCoord
		{
			TBmpHandler::GetX(pptDst[i]) - TBmpHandler::GetX(*itMinX),
			TBmpHandler::GetY(pptDst[i]) - TBmpHandler::GetY(*itMinY)
		};
	}
	const int yMax = TBmpHandler::GetY(vPtDst[std::distance(pptDst, itMaxY)]);

	struct EDGE
	{
		float x;
		float dx;
		float Rx;
		float Ry;
		float dRx;
		float dRy;
		int yMax;
		int xPtYMax;// y最大的点的x坐标
	};

	std::unordered_map<int, std::vector<EDGE*>> ET{};
	using TETIt = typename std::unordered_map<int, std::vector<EDGE*>>::iterator;
	EckCounter(cPt, i)
	{
		const auto& pt1 = vPtDst[i];
		const auto& pt2 = vPtDst[(i + 1) % cPt];
		const auto& ptSrc1 = pptSrc[i];
		const auto& ptSrc2 = pptSrc[(i + 1) % cPt];
		if (TBmpHandler::GetY(pt1) == TBmpHandler::GetY(pt2))
			continue;
		auto p = new EDGE;
		int yMax, yMin;
		if (TBmpHandler::GetY(pt1) < TBmpHandler::GetY(pt2))
		{
			p->x = (float)TBmpHandler::GetX(pt1);
			p->Rx = (float)TBmpHandler::GetX(ptSrc1);
			p->Ry = (float)TBmpHandler::GetY(ptSrc1);
			p->xPtYMax = (int)TBmpHandler::GetX(pt2);
			yMax = (int)TBmpHandler::GetY(pt2);
			yMin = (int)TBmpHandler::GetY(pt1);
		}
		else
		{
			p->x = (float)TBmpHandler::GetX(pt2);
			p->Rx = (float)TBmpHandler::GetX(ptSrc2);
			p->Ry = (float)TBmpHandler::GetY(ptSrc2);
			p->xPtYMax = (int)TBmpHandler::GetX(pt1);
			yMax = (int)TBmpHandler::GetY(pt1);
			yMin = (int)TBmpHandler::GetY(pt2);
		}
		p->dx = (float)(TBmpHandler::GetX(pt1) - TBmpHandler::GetX(pt2)) / (float)(TBmpHandler::GetY(pt1) - TBmpHandler::GetY(pt2));
		p->yMax = yMax;
		p->dRx = (float)(TBmpHandler::GetX(ptSrc1) - TBmpHandler::GetX(ptSrc2)) / (float)(TBmpHandler::GetY(pt1) - TBmpHandler::GetY(pt2));
		p->dRy = (float)(TBmpHandler::GetY(ptSrc1) - TBmpHandler::GetY(ptSrc2)) / (float)(TBmpHandler::GetY(pt1) - TBmpHandler::GetY(pt2));
		ET[yMin].push_back(p);
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
	struct YAAINFO
	{
		float k;
		float b;
		float x0;
		float x1;
	};

	std::vector<YAAINFO> vPrevYAA[2]{};// 均为向下采样
	auto fnDoYAA = [&](int y, float k, float b, float x0, float x1, bool bSampleDirection)
		{
			for (int x = (int)x0; x <= (int)x1; ++x)
			{
				const float yReal = k * x + b;
				const float e = (bSampleDirection ? yReal - y : y - yReal);// cr[0]的权值
				if (e > 1.f)
					continue;
				const int y2 = (bSampleDirection ? y - 1 : y + 1);
				if (y2 >= 0 && y2 < NewBmp.GetHeight())
				{
					const typename TBmpHandler::TColor cr[2]
					{
						NewBmp.GetPixel(TBmpHandler::MakeCoord(x,y)),
						NewBmp.GetPixel(TBmpHandler::MakeCoord(x,y2))
					};
					typename TBmpHandler::TColorComp crNew[4];
					EckCounter(4, k)
					{
						crNew[k] = (typename TBmpHandler::TColorComp)(
							TBmpHandler::GetColorComp(cr[0], k) * (1.f - e) +
							TBmpHandler::GetColorComp(cr[1], k) * e);
					}
					NewBmp.SetPixel(TBmpHandler::MakeCoord(x, y), TBmpHandler::MakeColor(crNew));
					//NewBmp.SetPixel(TBmpHandler::MakeCoord(x, y), 0xFFFF0000);
				}
			}
		};
	size_t idxCurrPrevYAA{};
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

				const float kL = 1.f / pL->dx;
				const float kR = 1.f / pR->dx;

				const float bL = pL->yMax - kL * pL->xPtYMax;
				const float bR = pR->yMax - kR * pR->xPtYMax;

				float xL, xL1, xR, xR1;
				bool bYAAL, bYAAR, bSampleDirectionL, bSampleDirectionR;// TRUE = 下面为外部
				if (kL == 0.f || kL == INFINITY ||
					(kL <= -1.f || kL >= 1.f) ||
					y == 0)
				{
					bYAAL = false;
				}
				else
				{
					bYAAL = true;
					if (kL < 0.f)
					{
						bSampleDirectionL = FALSE;
						xL = (y - bL) / kL;
						xL1 = (y - 1 - bL) / kL;
					}
					else
					{
						bSampleDirectionL = TRUE;
						xL = (y - bL) / kL;
						xL1 = (y + 1 - bL) / kL;
					}
				}

				if (kR == 0.f || kR == INFINITY ||
					(kR <= -1.f || kR >= 1.f) ||
					y == 0)
				{
					bYAAR = false;
				}
				else
				{
					bYAAR = true;
					if (kR < 0.f)
					{
						bSampleDirectionR = TRUE;
						xR = (y + 1 - bR) / kR;
						xR1 = (y - bR) / kR;
					}
					else
					{
						bSampleDirectionR = FALSE;
						xR = (y - 1 - bR) / kR;
						xR1 = (y - bR) / kR;
					}
				}

				const int xBegin = (int)ceilf(pL->x);
				const int xEnd = (int)floorf(pR->x);
				int x = xBegin;
				BOOL bAddL{}, bAddR{}, bSmpL{}, bSmpR{};
				for (; x <= xEnd; ++x)
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
					if (bYAAL && (x >= xL && x <= xL1))
					{
						if (bSampleDirectionL)
						{
							if (!bSmpL)
							{
								bSmpL = TRUE;
								fnDoYAA(y, kL, bL, xL, xL1, true);
							}
						}
						else if (!bAddL)
						{
							bAddL = TRUE;
							vPrevYAA[idxCurrPrevYAA].emplace_back(kL, bL, xL, xL1);
						}
						goto SkipNormalSetPixel;
					}

					if (bYAAR && (x >= xR && x <= xR1))
					{
						if (bSampleDirectionR)
						{
							if (!bSmpR)
							{
								bSmpR = TRUE;
								fnDoYAA(y, kR, bR, xR, xR1, true);
							}
						}
						else if (!bAddR)
						{
							bAddR = TRUE;
							vPrevYAA[idxCurrPrevYAA].emplace_back(kR, bR, xR, xR1);
						}
						goto SkipNormalSetPixel;
					}
					NewBmp.SetPixel(TBmpHandler::MakeCoord(x, y), TBmpHandler::MakeColor(crNew));
				SkipNormalSetPixel:
					Rxx += dRxx;
					Ryy += dRyy;

					if (x == xBegin && (kL <= -1.f || kL >= 1.f))
					{
						const float e = 1.f - (x - pL->x);// cr[0]的权值
						const int x2 = x - 1;
						if (x2 >= 0)
						{
							const typename TBmpHandler::TColor cr[2]
							{
								NewBmp.GetPixel(TBmpHandler::MakeCoord(x,y)),
								NewBmp.GetPixel(TBmpHandler::MakeCoord(x2,y))
							};
							typename TBmpHandler::TColorComp crNew[4];
							EckCounter(4, k)
							{
								crNew[k] = (typename TBmpHandler::TColorComp)(
									TBmpHandler::GetColorComp(cr[0], k) * (1.f - e) +
									TBmpHandler::GetColorComp(cr[1], k) * e);
							}
							NewBmp.SetPixel(TBmpHandler::MakeCoord(x2, y), TBmpHandler::MakeColor(crNew));
						}
					}
				}

				if (--x >= 0 && (kR <= -1.f || kR >= 1.f))
				{
					const float e = 1.f - (pR->x - x);// cr[0]的权值
					const int x2 = x + 1;
					if (x2 < NewBmp.GetWidth())
					{
						const typename TBmpHandler::TColor cr[2]
						{
							NewBmp.GetPixel(TBmpHandler::MakeCoord(x,y)),
							NewBmp.GetPixel(TBmpHandler::MakeCoord(x2,y))
						};
						typename TBmpHandler::TColorComp crNew[4];
						EckCounter(4, k)
						{
							crNew[k] = (typename TBmpHandler::TColorComp)(
								TBmpHandler::GetColorComp(cr[0], k) * (1.f - e) +
								TBmpHandler::GetColorComp(cr[1], k) * e);
						}
						NewBmp.SetPixel(TBmpHandler::MakeCoord(x2, y), TBmpHandler::MakeColor(crNew));
					}
				}
			}

			for (auto e : AEL)
			{
				e->x += e->dx;
				e->Rx += e->dRx;
				e->Ry += e->dRy;
			}
		}

		idxCurrPrevYAA = (idxCurrPrevYAA + 1) % 2;
		for (const auto& e : vPrevYAA[idxCurrPrevYAA])
			fnDoYAA(y - 1, e.k, e.b, e.x0, e.x1, false);
		vPrevYAA[idxCurrPrevYAA].clear();
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

inline BOOL FillGradientRect(HDC hDC, const RECT& rc, COLORREF cr1, COLORREF cr2, BOOL bVertical)
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
	gr.UpperLeft = 0;	// 左上角坐标为第一个成员
	gr.LowerRight = 1;	// 右下角坐标为第二个成员

	return GradientFill(hDC, tv, 2, &gr, 1, bVertical ? GRADIENT_FILL_RECT_V : GRADIENT_FILL_RECT_H);
}

enum class BkImgMode
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
	float x, float y, ID2D1PathGeometry1*& pPathGeometry, ID2D1Factory1* pD2dFactory = NULL)
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
		pPathGeometry = NULL;
	}
	else
		pPathGeometry = pPath;
	return hr;
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
inline HRESULT GetTextLayoutPathGeometry(IDWriteTextLayout* const* pLayout, int cLayout, const float* cyPadding,
	ID2D1RenderTarget* pRT, float* x, float yStart, ID2D1PathGeometry*& pPathGeometry, ID2D1Factory* pD2dFactory = NULL)
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
		pPathGeometry = NULL;
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
ECK_NAMESPACE_END