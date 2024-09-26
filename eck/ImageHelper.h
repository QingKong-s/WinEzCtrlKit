/*
* WinEzCtrlKit Library
*
* ImageHelper.h ： 图像帮助函数
*
* Copyright(C) 2023-2024 QingKong
*/
#pragma once
#include "Utility.h"
#include "CStreamView.h"
#include "CRefBinStream.h"
#include "ComPtr.h"

ECK_NAMESPACE_BEGIN
// 32bppPBGRA   D2D要求预乘
constexpr inline GUID DefWicPixelFormat
ECK_GUID(0x6fddc324, 0x4e03, 0x4bfe, 0xb1, 0x85, 0x3d, 0x77, 0x76, 0x8d, 0xc9, 0x10);

/// <summary>
/// 从字节流创建HBITMAP
/// </summary>
/// <param name="pData">字节流</param>
/// <param name="cbData">字节流尺寸</param>
/// <returns>位图句柄</returns>
inline HBITMAP CreateHBITMAP(PCVOID pData, SIZE_T cbData)
{
	HBITMAP hbm;
	GpBitmap* pBitmap;
	auto pStream = new CStreamView(pData, cbData);

	if (GdipCreateBitmapFromStream(pStream, &pBitmap) != Gdiplus::Ok)
	{
		pStream->LeaveRelease();
		return nullptr;
	}

	if (GdipCreateHBITMAPFromBitmap(pBitmap, &hbm, 0))
	{
		GdipDisposeImage(pBitmap);
		pStream->LeaveRelease();
		return nullptr;
	}

	GdipDisposeImage(pBitmap);
	pStream->LeaveRelease();
	return hbm;
}

/// <summary>
/// 从文件创建HBITMAP
/// </summary>
/// <param name="pszFile">文件名</param>
/// <returns>位图句柄</returns>
inline HBITMAP CreateHBITMAP(PCWSTR pszFile)
{
	HBITMAP hbm;
	GpBitmap* pBitmap;

	if (GdipCreateBitmapFromFile(pszFile, &pBitmap) != Gdiplus::Ok)
		return nullptr;

	if (GdipCreateHBITMAPFromBitmap(pBitmap, &hbm, 0))
	{
		GdipDisposeImage(pBitmap);
		return nullptr;
	}

	GdipDisposeImage(pBitmap);
	return hbm;
}

/// <summary>
/// 从WIC位图创建HBITMAP
/// </summary>
/// <param name="pBmp">WIC位图</param>
/// <returns>位图句柄</returns>
inline HBITMAP CreateHBITMAP(IWICBitmap* pBmp)
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
	const HDC hDC = GetDC(nullptr);
	const HBITMAP hBitmap = CreateDIBSection(hDC, &bmi, 0, &pDibBits, nullptr, 0);
	ReleaseDC(nullptr, hDC);

	pBmp->CopyPixels(nullptr, cx * 4, cx * cy * 4, (BYTE*)pDibBits);// GDI将每行位图数据对齐到DWORD
	return hBitmap;
}

/// <summary>
/// 从字节流创建HICON
/// </summary>
/// <param name="pData">字节流</param>
/// <param name="cbData">字节流尺寸</param>
/// <returns>图标句柄</returns>
inline HICON CreateHICON(PCVOID pData, SIZE_T cbData)
{
	HICON hIcon;
	GpBitmap* pBitmap;
	const auto pStream = new CStreamView(pData, cbData);

	if (GdipCreateBitmapFromStream(pStream, &pBitmap) != Gdiplus::Ok)
	{
		pStream->LeaveRelease();
		return nullptr;
	}

	if (GdipCreateHICONFromBitmap(pBitmap, &hIcon))
	{
		GdipDisposeImage(pBitmap);
		pStream->LeaveRelease();
		return nullptr;
	}

	GdipDisposeImage(pBitmap);
	pStream->LeaveRelease();
	return hIcon;
}

/// <summary>
/// 从文件创建HICON
/// </summary>
/// <param name="pszFile">文件名</param>
/// <returns>图标句柄</returns>
inline HICON CreateHICON(PCWSTR pszFile)
{
	HICON hIcon;
	GpBitmap* pBitmap;

	if (GdipCreateBitmapFromFile(pszFile, &pBitmap) != Gdiplus::Ok)
		return nullptr;

	if (GdipCreateHICONFromBitmap(pBitmap, &hIcon))
	{
		GdipDisposeImage(pBitmap);
		return nullptr;
	}

	GdipDisposeImage(pBitmap);
	return hIcon;
}

/// <summary>
/// 从WIC位图创建HICON
/// </summary>
/// <param name="pBmp">WIC位图</param>
/// <param name="hbmMask">掩码，若为nullptr，则使用全1掩码</param>
/// <returns>图标句柄</returns>
inline HICON CreateHICON(IWICBitmap* pBmp, HBITMAP hbmMask = nullptr)
{
	const HBITMAP hbmColor = CreateHBITMAP(pBmp);
	ICONINFO ii{};
	if (!hbmMask)
	{
		UINT cx, cy;
		pBmp->GetSize(&cx, &cy);
		ii.hbmMask = CreateBitmap(cx, cy, 1, 1, nullptr);
		const HDC hCDC = CreateCompatibleDC(nullptr);
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

inline HRESULT ScaleWicBitmap(IWICBitmapSource* pSrc, IWICBitmap*& pDst, int cx, int cy,
	WICBitmapInterpolationMode eInterMode = WICBitmapInterpolationModeLinear)
{
	pDst = nullptr;
	ComPtr<IWICBitmapScaler> pScaler;
	HRESULT hr;
	if (FAILED(hr = g_pWicFactory->CreateBitmapScaler(&pScaler)))
		return hr;
	if (FAILED(hr = pScaler->Initialize(pSrc, cx, cy, eInterMode)))
		return hr;
	return g_pWicFactory->CreateBitmapFromSource(pScaler.Get(),
		WICBitmapNoCache, &pDst);
}

/// <summary>
/// 创建WIC位图解码器
/// </summary>
/// <param name="pszFile">文件名</param>
/// <param name="pDecoder">接收解码器变量引用</param>
/// <returns>成功返回S_OK，失败返回错误代码</returns>
EckInline HRESULT CreateWicBitmapDecoder(PCWSTR pszFile, IWICBitmapDecoder*& pDecoder)
{
	pDecoder = nullptr;
	return g_pWicFactory->CreateDecoderFromFilename(pszFile, nullptr, GENERIC_READ,
		WICDecodeMetadataCacheOnDemand, &pDecoder);
}

/// <summary>
/// 创建WIC位图解码器
/// </summary>
/// <param name="pStream">流对象</param>
/// <param name="pDecoder">接收解码器变量引用</param>
/// <param name="pWicFactory">WIC工厂</param>
/// <returns>成功返回S_OK，失败返回错误代码</returns>
EckInline HRESULT CreateWicBitmapDecoder(IStream* pStream,
	IWICBitmapDecoder*& pDecoder)
{
	pDecoder = nullptr;
	return g_pWicFactory->CreateDecoderFromStream(pStream, nullptr,
		WICDecodeMetadataCacheOnDemand, &pDecoder);
}

/// <summary>
/// 创建WIC位图
/// </summary>
/// <param name="vResult">各帧位图，不会清空该容器</param>
/// <param name="pDecoder">解码器</param>
/// <param name="pWicFactory">WIC工厂</param>
/// <returns>成功返回S_OK，失败返回错误代码</returns>
inline HRESULT CreateWicBitmap(std::vector<IWICBitmap*>& vResult,
	IWICBitmapDecoder* pDecoder, const GUID& guidFmt = GUID_WICPixelFormat32bppBGRA)
{
	HRESULT hr;
	UINT cFrame;
	pDecoder->GetFrameCount(&cFrame);
	vResult.reserve(vResult.size() + cFrame);
	GUID guidSrcFmt;
	EckCounter(cFrame, i)
	{
		ComPtr<IWICBitmapFrameDecode> pFrameDecoder;
		if (FAILED(hr = pDecoder->GetFrame(i, &pFrameDecoder)))
			return hr;
		pFrameDecoder->GetPixelFormat(&guidSrcFmt);
		if (guidSrcFmt == guidFmt)
		{
			IWICBitmap* pBitmap;
			hr = g_pWicFactory->CreateBitmapFromSource(pFrameDecoder.Get(), WICBitmapNoCache, &pBitmap);
			if (FAILED(hr))
				return hr;
			vResult.push_back(pBitmap);
		}
		else
		{
			ComPtr<IWICFormatConverter> pConverter;
			if (FAILED(hr = g_pWicFactory->CreateFormatConverter(&pConverter)))
				return hr;
			hr = pConverter->Initialize(pFrameDecoder.Get(), guidFmt,
				WICBitmapDitherTypeNone, nullptr, 0.0f, WICBitmapPaletteTypeMedianCut);
			if (FAILED(hr))
				return hr;
			IWICBitmap* pBitmap;
			hr = g_pWicFactory->CreateBitmapFromSource(pConverter.Get(), WICBitmapNoCache, &pBitmap);
			if (FAILED(hr))
				return hr;
			vResult.push_back(pBitmap);
		}
	}
	return S_OK;
}

inline HRESULT CreateWicBitmap(IWICBitmap*& pBmp, IWICBitmapDecoder* pDecoder,
	int cxNew = -1, int cyNew = -1, const GUID& guidFmt = DefWicPixelFormat,
	WICBitmapInterpolationMode eInterMode = WICBitmapInterpolationModeLinear)
{
	HRESULT hr;
	GUID guidSrcFmt;
	ComPtr<IWICBitmapFrameDecode> pFrameDecoder;
	ComPtr<IWICFormatConverter> pConverter;
	if (FAILED(hr = pDecoder->GetFrame(0, &pFrameDecoder)))
		return hr;
	if (FAILED(hr = g_pWicFactory->CreateFormatConverter(&pConverter)))
		return hr;
	pFrameDecoder->GetPixelFormat(&guidSrcFmt);
	ComPtr<IWICBitmapSource> pSrc;
	if (guidSrcFmt == guidFmt)
		pSrc = pFrameDecoder;
	else
	{
		ComPtr<IWICFormatConverter> pConverter;
		if (FAILED(hr = g_pWicFactory->CreateFormatConverter(&pConverter)))
			return hr;
		hr = pConverter->Initialize(pFrameDecoder.Get(), guidFmt,
			WICBitmapDitherTypeNone, nullptr, 0.0f, WICBitmapPaletteTypeMedianCut);
		if (FAILED(hr))
			return hr;
		pSrc = pConverter;
	}

	if (cxNew > 0 && cyNew > 0)
		return ScaleWicBitmap(pSrc.Get(), pBmp, cxNew, cyNew, eInterMode);
	else
		return g_pWicFactory->CreateBitmapFromSource(pSrc.Get(), WICBitmapNoCache, &pBmp);
}

inline HRESULT CreateWicBitmap(IWICBitmap*& pBmp, PCWSTR psz,
	int cxNew = -1, int cyNew = -1, const GUID& guidFmt = DefWicPixelFormat,
	WICBitmapInterpolationMode eInterMode = WICBitmapInterpolationModeLinear)
{
	IWICBitmapDecoder* pDecoder;
	HRESULT hr = CreateWicBitmapDecoder(psz, pDecoder);
	if (FAILED(hr))
		return hr;
	hr = CreateWicBitmap(pBmp, pDecoder, cxNew, cyNew, guidFmt, eInterMode);
	pDecoder->Release();
	return hr;
}

inline HRESULT CreateWicBitmap(IWICBitmap*& pBmp, IStream* pStream,
	int cxNew = -1, int cyNew = -1, const GUID& guidFmt = DefWicPixelFormat,
	WICBitmapInterpolationMode eInterMode = WICBitmapInterpolationModeLinear)
{
	IWICBitmapDecoder* pDecoder;
	HRESULT hr = CreateWicBitmapDecoder(pStream, pDecoder);
	if (FAILED(hr))
		return hr;
	hr = CreateWicBitmap(pBmp, pDecoder, cxNew, cyNew, guidFmt, eInterMode);
	pDecoder->Release();
	return hr;
}

EckInline WORD GetPaletteEntryCount(HPALETTE hPalette)
{
	WORD w{};
	GetObjectW(hPalette, sizeof(w), &w);
	return w;
}

class CDib
{
private:
	HBITMAP m_hBitmap = nullptr;
public:
	ECK_DISABLE_COPY_DEF_CONS(CDib);

	CDib(CDib&& x) noexcept :m_hBitmap{ x.m_hBitmap } {}
	CDib& operator=(CDib&& x) noexcept { m_hBitmap = x.m_hBitmap; }

	~CDib() { Destroy(); }

	/// <summary>
	/// 创建自DC。
	/// 使用指定的调色板从DC中选入的位图创建一个DIB节，
	/// 函数先获取位图的BITMAP结构并检索调色板条目，
	/// 然后使用DIB_RGB_COLORS标志创建一个DIB节，
	/// 最后使用BitBlt拷贝位图数据
	/// </summary>
	/// <param name="hDC">设备场景</param>
	/// <param name="bTopToBottom">位图原点选项，TRUE = 自上而下</param>
	/// <param name="hPalette">调色板句柄</param>
	/// <param name="rc">要复制的矩形区域，若为空则复制整个位图</param>
	/// <returns>创建的DIB节</returns>
	HBITMAP Create(HDC hDC, BOOL bTopToBottom = FALSE, HPALETTE hPalette = nullptr, const RCWH& rc = {})
	{
		Destroy();
		BITMAP bmp;
		if (!GetObjectW(GetCurrentObject(hDC, OBJ_BITMAP), sizeof(bmp), &bmp))
			return nullptr;
		int x, y, cx, cy;
		if (IsRectEmpty(rc))
		{
			x = y = 0;
			cx = bmp.bmWidth;
			cy = Abs(bmp.bmHeight);
		}
		else
		{
			x = rc.x;
			y = rc.y;
			cx = rc.cx;
			cy = rc.cy;
		}
		const int cPalEntry = (hPalette ? GetPaletteEntryCount(hPalette) : 0);
		const size_t cbBmi = sizeof(BITMAPINFOHEADER) + cPalEntry * sizeof(RGBQUAD);
		auto pbmi = (BITMAPINFO*)malloc(cbBmi);
		EckAssert(pbmi);
		ZeroMemory(pbmi, cbBmi);
		pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		pbmi->bmiHeader.biWidth = cx;
		pbmi->bmiHeader.biHeight = (bTopToBottom ? -cy : cy);
		pbmi->bmiHeader.biPlanes = 1;
		pbmi->bmiHeader.biBitCount = bmp.bmBitsPixel;
		pbmi->bmiHeader.biCompression = BI_RGB;
		pbmi->bmiHeader.biClrUsed = cPalEntry;

		if (hPalette)
		{
			GetPaletteEntries(hPalette, 0, cPalEntry, (PALETTEENTRY*)(pbmi->bmiColors));
			EckCounter(cPalEntry, i)
				pbmi->bmiColors[i].rgbReserved = 0;
		}
#pragma warning(suppress:6387)
		m_hBitmap = CreateDIBSection(nullptr, pbmi, DIB_RGB_COLORS, nullptr, nullptr, 0);
		free(pbmi);
		if (!m_hBitmap)
			return nullptr;
		HDC hCDC = CreateCompatibleDC(hDC);
		SelectObject(hCDC, m_hBitmap);
		if (hPalette)
		{
			SelectPalette(hCDC, hPalette, FALSE);
			RealizePalette(hCDC);
		}
		BitBlt(hCDC, 0, 0, cx, cy, hDC, x, y, SRCCOPY);
		DeleteDC(hCDC);
		return m_hBitmap;
	}

	/// <summary>
	/// 创建自数据。
	/// 从DIB或紧凑DIB创建DIB节，支持Win3.0、V4、V5位图头，注意：不能解析压缩DIB和OS/2风格的DIB；
	/// 若给定数据具有BI_JPEG或BI_PNG，则总是创建32位DIB节
	/// </summary>
	/// <param name="pData">位图数据</param>
	/// <param name="hDC">要使用调色板的DC，若该参数为nullptr，则使用DIB_RGB_COLORS创建DIB节，否则使用DIB_PAL_COLORS</param>
	/// <returns>创建的DIB节</returns>
	HBITMAP Create(PCVOID pData, HDC hDC = nullptr)
	{
		Destroy();
		PCBYTE p = (PCBYTE)pData;
		DWORD ocbBits = 0;
		if (*((WORD*)p) == 0x4D42)
		{
			ocbBits = ((BITMAPFILEHEADER*)p)->bfOffBits;
			p += sizeof(BITMAPFILEHEADER);
		}

		PCBYTE pBitsData;
		const BITMAPV4HEADER* pV4;
		size_t cbBits;
		switch (*((DWORD*)p))
		{
		case (sizeof(BITMAPINFOHEADER)):
		{
			auto pbih = (const BITMAPINFOHEADER*)p;
			const BOOL bBitFields = (pbih->biCompression == BI_BITFIELDS);
			if (pbih->biCompression != BI_RGB && !bBitFields)
				return nullptr;
			pBitsData =
				(ocbBits ?
					(p - sizeof(BITMAPFILEHEADER) + ocbBits) :
					(p + sizeof(BITMAPINFOHEADER) /*跳过信息头*/ +
						pbih->biClrUsed * sizeof(RGBQUAD) /*跳过颜色表*/ +
						(bBitFields ? (3 * sizeof(DWORD)) : 0) /*跳过颜色掩码*/
						));
			cbBits = (pbih->biSizeImage ? pbih->biSizeImage :
				(((((pbih->biWidth * pbih->biBitCount) + 31) & ~31) >> 3) * pbih->biHeight));
			void* pBits;
			m_hBitmap = CreateDIBSection(hDC, (const BITMAPINFO*)pbih,
				(hDC ? DIB_PAL_COLORS : DIB_RGB_COLORS), &pBits, nullptr, 0);
			if (!m_hBitmap)
				return nullptr;
			memcpy(pBits, pBitsData, cbBits);
		}
		break;
		case (sizeof(BITMAPV4HEADER)):
		case (sizeof(BITMAPV5HEADER)):
		{
			pV4 = (const BITMAPV4HEADER*)p;
			pBitsData = (ocbBits ?
				(p - sizeof(BITMAPFILEHEADER) + ocbBits) :
				(p + pV4->bV4Size /*跳过信息头*/ +
					pV4->bV4ClrUsed * sizeof(RGBQUAD) /*跳过颜色表*/));
			cbBits = (pV4->bV4SizeImage ? pV4->bV4SizeImage :
				((((pV4->bV4Width * pV4->bV4BitCount) + 31) & ~31) >> 3) * pV4->bV4Height);
			if (pV4->bV4V4Compression == BI_JPEG || pV4->bV4V4Compression == BI_PNG)
			{
				auto pStream = new CStreamView(pBitsData, cbBits);
				IWICBitmapDecoder* pDecoder;
				if (FAILED(CreateWicBitmapDecoder(pStream, pDecoder)))
				{
					pStream->LeaveRelease();
					return nullptr;
				}
				pStream->LeaveRelease();
				IWICBitmap* pBitmap;
				if (FAILED(CreateWicBitmap(pBitmap, pDecoder)))
				{
					pDecoder->Release();
					return nullptr;
				}
				m_hBitmap = CreateHBITMAP(pBitmap);
				pDecoder->Release();
				pBitmap->Release();
			}
			else if (pV4->bV4V4Compression == BI_RGB || pV4->bV4V4Compression == BI_BITFIELDS)
			{
				auto pbmi = (BITMAPINFO*)malloc(
					sizeof(BITMAPINFOHEADER) +
					pV4->bV4ClrUsed * sizeof(RGBQUAD));
				EckAssert(pbmi);
				memcpy(pbmi, pV4, sizeof(BITMAPINFOHEADER));
				memcpy(pbmi + 1, p + sizeof(BITMAPV4HEADER), pV4->bV4ClrUsed * sizeof(RGBQUAD));
				void* pBits;
				m_hBitmap = CreateDIBSection(hDC, pbmi,
					(hDC ? DIB_PAL_COLORS : DIB_RGB_COLORS), &pBits, nullptr, 0);
				free(pbmi);
				if (!m_hBitmap)
					return nullptr;
#pragma warning(suppress:6001)// 未初始化内存
				memcpy(pBits, pBitsData, cbBits);
			}
			else
				return nullptr;
		}
		break;
		default:
			EckDbgPrintWithPos(L"无法识别的位图格式");
			return nullptr;
		}
		return m_hBitmap;
	}

	/// <summary>
	/// 创建空白32位DIB节
	/// </summary>
	/// <param name="cx">宽度</param>
	/// <param name="cy">高度，负值为自上而下位图</param>
	/// <returns>创建的DIB节</returns>
	HBITMAP Create(int cx, int cy)
	{
		Destroy();
		BITMAPINFOHEADER bih{};
		bih.biSize = sizeof(BITMAPINFOHEADER);
		bih.biWidth = cx;
		bih.biHeight = cy;
		bih.biPlanes = 1;
		bih.biBitCount = 32;
		bih.biCompression = BI_RGB;
#pragma warning(suppress:6387)
		m_hBitmap = CreateDIBSection(nullptr, (const BITMAPINFO*)&bih, DIB_RGB_COLORS, nullptr, nullptr, 0);
		return m_hBitmap;
	}

	EckInline HBITMAP GetHBitmap() const { return m_hBitmap; }

	EckInline SIZE_T GetBmpDataSize() const
	{
		DIBSECTION ds;
		GetObjectW(m_hBitmap, sizeof(ds), &ds);
		return sizeof(BITMAPFILEHEADER) +
			sizeof(BITMAPINFOHEADER) +
			sizeof(RGBQUAD) * ds.dsBmih.biClrUsed +
			ds.dsBm.bmWidthBytes * ds.dsBm.bmHeight;
	}

	/// <summary>
	/// 取BMP数据。
	/// 使用Windows3.0版位图头
	/// </summary>
	/// <returns>BMP数据</returns>
	CRefBin GetBmpData() const
	{
		if (!m_hBitmap)
			return {};
		DIBSECTION ds;
		GetObjectW(m_hBitmap, sizeof(ds), &ds);
		const size_t cbTotal = GetBmpDataSize();
		CRefBin rb(cbTotal);
		const auto pfh = (BITMAPFILEHEADER*)rb.Data();
		const auto pih = (BITMAPINFOHEADER*)(pfh + 1);
		// 制文件头
		pfh->bfType = 0x4D42;// BM
		pfh->bfSize = (DWORD)cbTotal;
		pfh->bfReserved1 = pfh->bfReserved2 = 0;
		pfh->bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
		// 制信息头
		pih->biSize = sizeof(BITMAPINFOHEADER);
		pih->biWidth = ds.dsBm.bmWidth;
		pih->biHeight = ds.dsBm.bmHeight;
		pih->biPlanes = 1;
		pih->biBitCount = ds.dsBm.bmBitsPixel;
		pih->biCompression = 0;
		pih->biXPelsPerMeter = pih->biYPelsPerMeter = 0;
		pih->biClrUsed = ds.dsBmih.biClrUsed;
		pih->biClrImportant = ds.dsBmih.biClrImportant;
		// 制颜色表
		const size_t cbPalette = sizeof(RGBQUAD) * ds.dsBmih.biClrUsed;
		if (cbPalette)
		{
			HDC hCDC = CreateCompatibleDC(nullptr);
			SelectObject(hCDC, m_hBitmap);
			GetDIBColorTable(hCDC, 0, ds.dsBmih.biClrUsed, (RGBQUAD*)(pih + 1));
			DeleteDC(hCDC);
		}
		// 制像素
		memcpy(((BYTE*)(pih + 1)) + cbPalette, ds.dsBm.bmBits,
			ds.dsBm.bmWidthBytes * ds.dsBm.bmHeight);
		return rb;
	}

	EckInline void Destroy()
	{
		DeleteObject(m_hBitmap);
		m_hBitmap = nullptr;
	}

	EckInline void Attach(HBITMAP hBitmap)
	{
		DeleteObject(m_hBitmap);
		m_hBitmap = hBitmap;
	}

	[[nodiscard]] EckInline HBITMAP Detach()
	{
		HBITMAP hBitmap = m_hBitmap;
		m_hBitmap = nullptr;
		return hBitmap;
	}
};

enum class SnapshotDib
{
	Dib,
	Ddb,
	DibTopToBottom,
};

inline HBITMAP Snapshot(HWND hWnd, const RCWH& rc, BOOL bCursor, SnapshotDib eDib = SnapshotDib::Ddb)
{
	if (IsRectEmpty(rc))
		GetClientRect(hWnd, (RECT*)&rc);
	if (IsRectEmpty(rc))
		return nullptr;
	const HDC hDC = GetDC(hWnd);
	CURSORINFO ci{};
	ICONINFO ii{};
	int cxCursor{}, cyCursor{};
	if (bCursor)
	{
		ci.cbSize = sizeof(ci);
		GetCursorInfo(&ci);
		if (ci.flags == CURSOR_SHOWING)
		{
			ScreenToClient(hWnd, &ci.ptScreenPos);
			GetIconInfo(ci.hCursor, &ii);
			BITMAP Bmp;
			GetObjectW(ii.hbmColor, sizeof(Bmp), &Bmp);
			cxCursor = Bmp.bmWidth;
			cyCursor = Bmp.bmHeight;
			if (ii.hbmMask)
				DeleteObject(ii.hbmMask);
			if (ii.hbmColor)
				DeleteObject(ii.hbmColor);
		}
	}
	if (eDib == SnapshotDib::Ddb)
	{
		const auto hBmp = CreateCompatibleBitmap(hDC, rc.cx, rc.cy);
		const auto hCDC = CreateCompatibleDC(hDC);
		SelectObject(hCDC, hBmp);
		BitBlt(hCDC, 0, 0, rc.cx, rc.cy, hDC, rc.x, rc.y, SRCCOPY);
		if (cxCursor)
		{
			DrawIconEx(hCDC, ci.ptScreenPos.x, ci.ptScreenPos.y,
				ci.hCursor, cxCursor, cyCursor, 0, nullptr, DI_NORMAL);
		}
		DeleteDC(hCDC);
		return hBmp;
	}
	else
	{
		CDib dib;
		dib.Create(hDC, eDib == SnapshotDib::DibTopToBottom, nullptr, rc);
		if (cxCursor)
		{
			const auto hCDC = CreateCompatibleDC(hDC);
			SelectObject(hCDC, dib.GetHBitmap());
			DrawIconEx(hCDC, ci.ptScreenPos.x, ci.ptScreenPos.y,
				ci.hCursor, cxCursor, cyCursor, 0, nullptr, DI_NORMAL);
			DeleteDC(hCDC);
		}
		return dib.Detach();
	}
}

enum class ImageType
{
	Unknown,
	Bmp,
	Gif,
	Jpeg,
	Png,
	Tiff,
	Wdp,
	Dds,

	MaxValue
};

constexpr inline PCWSTR c_szImgMime[]
{
	L"",
	L"image/bmp",
	L"image/gif",
	L"image/jpeg",
	L"image/png",
	L"image/tiff",
	L"image/vnd.ms-photo",// wdp
	L"image/vnd.ms-dds",// dds
};

const inline GUID c_guidWicEncoder[]
{
	GUID_NULL,
	GUID_ContainerFormatBmp,
	GUID_ContainerFormatGif,
	GUID_ContainerFormatJpeg,
	GUID_ContainerFormatPng,
	GUID_ContainerFormatTiff,
	GUID_ContainerFormatWmp,
	GUID_ContainerFormatDds,
};

static_assert((int)ImageType::MaxValue == ARRAYSIZE(c_szImgMime) &&
	ARRAYSIZE(c_szImgMime) == ARRAYSIZE(c_guidWicEncoder));

EckInline constexpr PCWSTR GetImageMime(ImageType iType)
{
	return c_szImgMime[(int)iType];
}

inline Gdiplus::GpStatus GetGpEncoderClsid(PCWSTR pszType, CLSID& clsid)
{
	Gdiplus::GpStatus gps;
	UINT cEncoders, cbEncoders;
	if ((gps = GdipGetImageEncodersSize(&cEncoders, &cbEncoders)) != Gdiplus::Ok)
		return gps;
	const auto pEncoders = (Gdiplus::ImageCodecInfo*)malloc(cbEncoders);
	EckAssert(pEncoders);
	if ((gps = GdipGetImageEncoders(cEncoders, cbEncoders, pEncoders)) != Gdiplus::Ok)
	{
		free(pEncoders);
		return gps;
	}
	EckCounter(cEncoders, i)
	{
#pragma warning(suppress:6385)// 正在读取无效数据
		if (!wcscmp(pEncoders[i].MimeType, pszType))
		{
			clsid = pEncoders[i].Clsid;
			free(pEncoders);
			return Gdiplus::Ok;
		}
	}
	free(pEncoders);
	clsid = GUID_NULL;
	return Gdiplus::UnknownImageFormat;
}

EckInline const GUID& GetWicEncoderGuid(ImageType iType)
{
	return c_guidWicEncoder[(int)iType];
}

inline HRESULT SaveWicBitmap(IStream* pStream, IWICBitmapSource* pBitmap, ImageType iType = ImageType::Png)
{
	HRESULT hr;
	ComPtr<IWICBitmapEncoder> pEncoder;
	ComPtr<IWICBitmapFrameEncode> pFrame;
	if (FAILED(hr = g_pWicFactory->CreateEncoder(GetWicEncoderGuid(iType), nullptr, &pEncoder)))
		return hr;
	if (FAILED(hr = pEncoder->Initialize(pStream, WICBitmapEncoderNoCache)))
		return hr;
	if (FAILED(hr = pEncoder->CreateNewFrame(&pFrame, nullptr)))
		return hr;
	if (FAILED(hr = pFrame->Initialize(nullptr)))
		return hr;
	if (FAILED(hr = pFrame->WriteSource(pBitmap, nullptr)))
		return hr;
	if (FAILED(hr = pFrame->Commit()))// 提交帧
		return hr;
	return pEncoder->Commit();// 提交编码器
}

inline CRefBin SaveWicBitmap(IWICBitmap* pBitmap, ImageType iType = ImageType::Png, HRESULT* phr = nullptr)
{
	CRefBin rb{};
	const auto pStream = new CRefBinStream(rb);
	const auto hr = SaveWicBitmap(pStream, pBitmap, iType);
	if (phr)
		*phr = hr;
	pStream->LeaveRelease();
	return rb;
}

inline HRESULT SaveWicBitmap(PCWSTR pszFile, IWICBitmap* pBitmap, ImageType iType = ImageType::Png)
{
	HRESULT hr;
	ComPtr<IWICStream> pStream;
	if (FAILED(hr = g_pWicFactory->CreateStream(&pStream)))
		return hr;
	if (FAILED(hr = pStream->InitializeFromFilename(pszFile, GENERIC_WRITE)))
		return hr;
	return SaveWicBitmap(pStream.Get(), pBitmap, iType);
}

inline Gdiplus::GpStatus SaveGpImage(IStream* pStream, Gdiplus::GpImage* pImage, ImageType iType = ImageType::Png)
{
	CLSID clsid;
	Gdiplus::GpStatus gps;
	if ((gps = GetGpEncoderClsid(GetImageMime(iType), clsid)) != Gdiplus::Ok)
		return gps;
	return GdipSaveImageToStream(pImage, pStream, &clsid, nullptr);
}

inline CRefBin SaveGpImage(Gdiplus::GpImage* pImage, ImageType iType = ImageType::Png, Gdiplus::GpStatus* pgps = nullptr)
{
	CRefBin rb{};
	const auto pStream = new CRefBinStream(rb);
	const auto gps = SaveGpImage(pStream, pImage, iType);
	pStream->LeaveRelease();
	if (pgps)
		*pgps = gps;
	return rb;
}

inline Gdiplus::GpStatus SaveGpImage(PCWSTR pszFile, Gdiplus::GpImage* pImage, ImageType iType = ImageType::Png)
{
	CLSID clsid;
	Gdiplus::GpStatus gps;
	if ((gps = GetGpEncoderClsid(GetImageMime(iType), clsid)) != Gdiplus::Ok)
		return gps;
	return GdipSaveImageToFile(pImage, pszFile, &clsid, nullptr);
}

inline HRESULT LoadD2dBitmap(PCWSTR pszFile, ID2D1RenderTarget* pRT, ID2D1Bitmap*& pD2dBitmap,
	int cxNew = -1, int cyNew = -1)
{
	pD2dBitmap = nullptr;
	HRESULT hr;
	ComPtr<IWICBitmapDecoder> pDecoder;
	if (FAILED(hr = CreateWicBitmapDecoder(pszFile, pDecoder.RefOf())))
		return hr;
	ComPtr<IWICBitmap> pBitmap;
	if (FAILED(hr = CreateWicBitmap(pBitmap.RefOf(), pDecoder.Get(), cxNew, cyNew)))
		return hr;
	return pRT->CreateBitmapFromWicBitmap(pBitmap.Get(), &pD2dBitmap);
}

inline [[nodiscard]] GpBitmap* ScaleGpBitmap(GpBitmap* pBitmap, int cxNew, int cyNew,
	Gdiplus::InterpolationMode eInterp = Gdiplus::InterpolationModeDefault)
{
	if (cxNew < 0 && cyNew < 0)
		return nullptr;
	UINT cxOrg, cyOrg;
	GdipGetImageWidth(pBitmap, &cxOrg);
	GdipGetImageHeight(pBitmap, &cyOrg);
	Gdiplus::PixelFormat PixFmt;
	GdipGetImagePixelFormat(pBitmap, &PixFmt);
	if (cxNew < 0)
		cxNew = cxOrg * cyNew / cyOrg;
	if (cyNew < 0)
		cyNew = cyOrg * cxNew / cxOrg;
	if (cxNew == cxOrg && cyNew == cyOrg)
		return nullptr;
	GpBitmap* pNewBitmap = nullptr;
	GdipCreateBitmapFromScan0(cxNew, cyNew, 0, PixFmt, nullptr, &pNewBitmap);
	GpGraphics* pGraphics = nullptr;
	GdipGetImageGraphicsContext(pNewBitmap, &pGraphics);
	GdipSetInterpolationMode(pGraphics, eInterp);
	GdipDrawImageRectRectI(pGraphics, pBitmap,
		0, 0, cxNew, cyNew,
		0, 0, cxOrg, cyOrg, Gdiplus::UnitPixel, nullptr, nullptr, nullptr);
	GdipDeleteGraphics(pGraphics);
	return pNewBitmap;
}
ECK_NAMESPACE_END