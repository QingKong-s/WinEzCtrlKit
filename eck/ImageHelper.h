/*
* WinEzCtrlKit Library
*
* ImageHelper.h ： 图像帮助函数
*
* Copyright(C) 2023 QingKong
*/
#pragma once
#include "Utility.h"
#include "GdiplusFlatDef.h"
#include "MathHelper.h"
#include "CStreamView.h"
#include "CRefBinStream.h"

#include <execution>
#include <unordered_map>

#include <DirectXMath.h>
#include <dxgi1_2.h>

ECK_NAMESPACE_BEGIN

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

	if (GdipCreateBitmapFromStream(pStream, &pBitmap) != GpStatus::GpOk)
	{
		pStream->LeaveRelease();
		return NULL;
	}

	if (GdipCreateHBITMAPFromBitmap(pBitmap, &hbm, 0))
	{
		GdipDisposeImage(pBitmap);
		pStream->LeaveRelease();
		return NULL;
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

	if (GdipCreateBitmapFromFile(pszFile, &pBitmap) != GpStatus::GpOk)
		return NULL;

	if (GdipCreateHBITMAPFromBitmap(pBitmap, &hbm, 0))
	{
		GdipDisposeImage(pBitmap);
		return NULL;
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
	const HDC hDC = GetDC(NULL);
	const HBITMAP hBitmap = CreateDIBSection(hDC, &bmi, 0, &pDibBits, NULL, 0);
	ReleaseDC(NULL, hDC);

	pBmp->CopyPixels(NULL, cx * 4, cx * cy * 4, (BYTE*)pDibBits);// GDI将每行位图数据对齐到DWORD
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
	auto pStream = new CStreamView(pData, cbData);

	if (GdipCreateBitmapFromStream(pStream, &pBitmap) != GpStatus::GpOk)
	{
		pStream->LeaveRelease();
		return NULL;
	}

	if (GdipCreateHICONFromBitmap(pBitmap, &hIcon))
	{
		GdipDisposeImage(pBitmap);
		pStream->LeaveRelease();
		return NULL;
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

	if (GdipCreateBitmapFromFile(pszFile, &pBitmap) != GpStatus::GpOk)
		return NULL;

	if (GdipCreateHICONFromBitmap(pBitmap, &hIcon))
	{
		GdipDisposeImage(pBitmap);
		return NULL;
	}

	GdipDisposeImage(pBitmap);
	return hIcon;
}

/// <summary>
/// 从WIC位图创建HICON
/// </summary>
/// <param name="pBmp">WIC位图</param>
/// <param name="hbmMask">掩码，若为NULL，则使用全1掩码</param>
/// <returns>图标句柄</returns>
inline HICON CreateHICON(IWICBitmap* pBmp, HBITMAP hbmMask)
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

/// <summary>
/// 创建WIC位图
/// </summary>
/// <param name="vResult">各帧位图</param>
/// <param name="pDecoder">解码器</param>
/// <param name="pWicFactory">WIC工厂</param>
/// <returns>成功返回S_OK，失败返回错误代码</returns>
inline HRESULT CreateWicBitmap(std::vector<IWICBitmap*>& vResult,
	IWICBitmapDecoder* pDecoder, IWICImagingFactory* pWicFactory)
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
			return hr;
		}

		hr = pWicFactory->CreateFormatConverter(&pConverter);
		if (FAILED(hr))
		{
			EckDbgPrintFormatMessage(hr);
			return hr;
		}

		hr = pConverter->Initialize(pFrameDecoder, GUID_WICPixelFormat32bppPBGRA,
			WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeMedianCut);
		if (FAILED(hr))
		{
			EckDbgPrintFormatMessage(hr);
			return hr;
		}

		IWICBitmap* pWicBitmap;
		hr = pWicFactory->CreateBitmapFromSource(pConverter, WICBitmapNoCache, &pWicBitmap);
		if (FAILED(hr))
		{
			EckDbgPrintFormatMessage(hr);
			return hr;
		}

		pConverter->Release();
		vResult[i] = pWicBitmap;
		pFrameDecoder->Release();
	}

	return S_OK;
}

inline HRESULT CreateWicBitmap(IWICBitmap*& pBmp, IWICBitmapDecoder* pDecoder)
{
	HRESULT hr;
	IWICBitmapFrameDecode* pFrameDecoder;
	IWICFormatConverter* pConverter;

	hr = pDecoder->GetFrame(0, &pFrameDecoder);
	if (FAILED(hr))
	{
		EckDbgPrintFormatMessage(hr);
		return hr;
	}

	hr = g_pWicFactory->CreateFormatConverter(&pConverter);
	if (FAILED(hr))
	{
		EckDbgPrintFormatMessage(hr);
		return hr;
	}

	hr = pConverter->Initialize(pFrameDecoder, GUID_WICPixelFormat32bppPBGRA,
		WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeMedianCut);
	if (FAILED(hr))
	{
		EckDbgPrintFormatMessage(hr);
		return hr;
	}

	hr = g_pWicFactory->CreateBitmapFromSource(pConverter, WICBitmapNoCache, &pBmp);
	if (FAILED(hr))
	{
		EckDbgPrintFormatMessage(hr);
		return hr;
	}

	pConverter->Release();
	pFrameDecoder->Release();
	return S_OK;
}

/// <summary>
/// 创建WIC位图解码器
/// </summary>
/// <param name="pszFile">文件名</param>
/// <param name="pDecoder">接收解码器变量引用</param>
/// <returns>成功返回S_OK，失败返回错误代码</returns>
EckInline HRESULT CreateWicBitmapDecoder(PCWSTR pszFile, IWICBitmapDecoder*& pDecoder)
{
	pDecoder = NULL;
	return g_pWicFactory->CreateDecoderFromFilename(pszFile, NULL, GENERIC_READ,
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
	pDecoder = NULL;
	return g_pWicFactory->CreateDecoderFromStream(pStream, NULL, 
		WICDecodeMetadataCacheOnDemand, &pDecoder);
}

struct ICONDIRENTRY
{
	BYTE  bWidth;          // Width, in pixels, of the image
	BYTE  bHeight;         // Height, in pixels, of the image
	BYTE  bColorCount;     // Number of colors in image (0 if >=8bpp)
	BYTE  bReserved;       // Reserved ( must be 0)
	WORD  wPlanes;         // Color Planes
	WORD  wBitCount;       // Bits per pixel
	DWORD dwBytesInRes;    // How many bytes in this resource?
	DWORD dwImageOffset;   // Where in the file is this image?
};

struct ICONDIR
{
	WORD         idReserved;   // Reserved (must be 0)
	WORD         idType;       // Resource Type (1 for icons)
	WORD         idCount;      // How many images?
	// ICONDIRENTRY idEntries[1]; // An entry for each image (idCount of 'em)
};

class CIcoFileReader
{
private:
	PCBYTE m_pData = NULL;
	const ICONDIR* m_pHeader = NULL;
	const ICONDIRENTRY* m_pEntry = NULL;
public:
	EckInline int AnalyzeData(PCBYTE pData)
	{
		m_pData = pData;
		m_pHeader = (ICONDIR*)pData;
		m_pEntry = (ICONDIRENTRY*)(pData + sizeof(ICONDIR));
		return m_pHeader->idCount;
	}

	EckInline auto GetHeader() const { return m_pHeader; }

	EckInline auto GetEntry() const { return m_pEntry; }

	EckInline PCVOID GetIconData(int idx) const
	{
		EckAssert(idx >= 0 && idx < GetIconCount());
		return m_pData + m_pEntry[idx].dwImageOffset;
	}

	EckInline DWORD GetIconDataSize(int idx) const
	{
		EckAssert(idx >= 0 && idx < GetIconCount());
		return m_pEntry[idx].dwBytesInRes;
	}

	EckInline int GetIconCount() const { return m_pHeader->idCount; }

	EckInline int FindIcon(int cx, int cy) const
	{
		EckCounter(GetIconCount(), i)
		{
			if (m_pEntry[i].bWidth == cx && m_pEntry[i].bHeight == cy)
				return i;
		}
		return -1;
	}

	EckInline HICON CreateHICON(int idx, int cx = 0, int cy = 0, UINT uFlags = 0u) const
	{
		EckAssert(idx >= 0 && idx < GetIconCount());
		return CreateIconFromResourceEx((BYTE*)GetIconData(idx), GetIconDataSize(idx),
			TRUE, 0x00030000, cx, cy, uFlags);
	}

	auto At(int idx) const { EckAssert(idx >= 0 && idx < GetIconCount()); return m_pEntry + idx; }
};

EckInline WORD GetPaletteEntryCount(HPALETTE hPalette)
{
	WORD w = 0;
	GetObjectW(hPalette, sizeof(w), &w);
	return w;
}

class CDib
{
private:
	HBITMAP m_hBitmap = NULL;
public:
	CDib() = default;
	CDib(const CDib&) = delete;
	CDib& operator=(const CDib&) = delete;

	CDib(CDib&& x) noexcept
		:m_hBitmap{ x.m_hBitmap } {}
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
	/// <param name="iTopToBottom">位图原点选项，&lt;0 - 自上而下  >=0 - 自下而上</param>
	/// <param name="hPalette">调色板句柄</param>
	/// <returns>创建的DIB节</returns>
	HBITMAP Create(HDC hDC, int iTopToBottom = 0, HPALETTE hPalette = NULL)
	{
		Destroy();
		BITMAP bmp;
		if (!GetObjectW(GetCurrentObject(hDC, OBJ_BITMAP), sizeof(bmp), &bmp))
			return NULL;
		const int cPalEntry = (hPalette ? GetPaletteEntryCount(hPalette) : 0);
		const size_t cbBmi = sizeof(BITMAPINFOHEADER) + cPalEntry * sizeof(RGBQUAD);
		auto pbmi = (BITMAPINFO*)malloc(cbBmi);
		EckAssert(pbmi);
		ZeroMemory(pbmi, cbBmi);
		pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		pbmi->bmiHeader.biWidth = bmp.bmWidth;
		pbmi->bmiHeader.biHeight = SetSign(bmp.bmHeight, (LONG)iTopToBottom);
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
		m_hBitmap = CreateDIBSection(NULL, pbmi, DIB_RGB_COLORS, NULL, NULL, 0);
		free(pbmi);
		if (!m_hBitmap)
			return NULL;
		HDC hCDC = CreateCompatibleDC(hDC);
		SelectObject(hCDC, m_hBitmap);
		if (hPalette)
		{
			SelectPalette(hCDC, hPalette, FALSE);
			RealizePalette(hCDC);
		}
		BitBlt(hCDC, 0, 0, bmp.bmWidth, bmp.bmHeight, hDC, 0, 0, SRCCOPY);
		DeleteDC(hCDC);
		return m_hBitmap;
	}

	/// <summary>
	/// 创建自数据。
	/// 从DIB或紧凑DIB创建DIB节，支持Win3.0、V4、V5位图头，注意：不能解析压缩DIB和OS/2风格的DIB；
	/// 若给定数据具有BI_JPEG或BI_PNG，则总是创建32位DIB节
	/// </summary>
	/// <param name="pData">位图数据</param>
	/// <param name="hDC">要使用调色板的DC，若该参数为NULL，则使用DIB_RGB_COLORS创建DIB节，否则使用DIB_PAL_COLORS</param>
	/// <returns>创建的DIB节</returns>
	HBITMAP Create(PCVOID pData, HDC hDC = NULL)
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
				return NULL;
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
				(hDC ? DIB_PAL_COLORS : DIB_RGB_COLORS), &pBits, NULL, 0);
			if (!m_hBitmap)
				return NULL;
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
					return NULL;
				}
				pStream->LeaveRelease();
				IWICBitmap* pBitmap;
				if (FAILED(CreateWicBitmap(pBitmap, pDecoder)))
				{
					pDecoder->Release();
					return NULL;
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
					(hDC ? DIB_PAL_COLORS : DIB_RGB_COLORS), &pBits, NULL, 0);
				free(pbmi);
				if (!m_hBitmap)
					return NULL;
#pragma warning(suppress:6001)// 未初始化内存
				memcpy(pBits, pBitsData, cbBits);
			}
			else
				return NULL;
		}
		break;
		default:
			EckDbgPrintWithPos(L"无法识别的位图格式");
			return NULL;
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
		m_hBitmap = CreateDIBSection(NULL, (const BITMAPINFO*)&bih, DIB_RGB_COLORS, NULL, NULL, 0);
		if (!m_hBitmap)
			return NULL;
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
		auto pfh = (BITMAPFILEHEADER*)rb.Data();
		auto pih = (BITMAPINFOHEADER*)(pfh + 1);
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
			HDC hCDC = CreateCompatibleDC(NULL);
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
		m_hBitmap = NULL;
	}

	EckInline HBITMAP Attach(HBITMAP hBitmap)
	{
		std::swap(m_hBitmap, hBitmap);
		return hBitmap;
	}

	[[nodiscard]] EckInline HBITMAP Detach() { return Attach(NULL); }
};

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

inline GpStatus GetGpEncoderClsid(PCWSTR pszType, CLSID& clsid)
{
	GpStatus gps;
	UINT cEncoders, cbEncoders;
	if ((gps = GdipGetImageEncodersSize(&cEncoders, &cbEncoders)) != GpStatus::GpOk)
		return gps;
	auto pEncoders = (GpImageCodecInfo*)malloc(cbEncoders);
	EckAssert(pEncoders);
	if ((gps = GdipGetImageEncoders(cEncoders, cbEncoders, pEncoders)) != GpStatus::GpOk)
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
			return GpStatus::GpOk;
		}
	}
	free(pEncoders);
	clsid = GUID_NULL;
	return GpStatus::GpUnknownImageFormat;
}

EckInline GUID GetWicEncoderGuid(ImageType iType)
{
	return c_guidWicEncoder[(int)iType];
}

inline HRESULT SaveWicBitmap(IStream* pStream, IWICBitmap* pBitmap, ImageType iType = ImageType::Png)
{
	HRESULT hr;
	GUID guid = GetWicEncoderGuid(iType);
	IWICBitmapEncoder* pEncoder;
	if (FAILED(hr = g_pWicFactory->CreateEncoder(guid, NULL, &pEncoder)))
		return hr;
	pEncoder->Initialize(pStream, WICBitmapEncoderNoCache);
	IWICBitmapFrameEncode* pFrame;
	IPropertyBag2* pPropBag;
#pragma warning(suppress:6001)// 使用未初始化的内存
	if (FAILED(hr = pEncoder->CreateNewFrame(&pFrame, &pPropBag)))
	{
		pEncoder->Release();
		return hr;
	}

	if (FAILED(hr = pFrame->Initialize(pPropBag)))
	{
		pFrame->Release();
		pEncoder->Release();
		pPropBag->Release();
		return hr;
	}
	// 尺寸
	UINT cx, cy;
	pBitmap->GetSize(&cx, &cy);
	pFrame->SetSize(cx, cy);
	// 像素格式
	WICPixelFormatGUID guidFmt;
	pBitmap->GetPixelFormat(&guidFmt);
	pFrame->SetPixelFormat(&guidFmt);
	// 分辨率
	double xDpi, yDpi;
	pBitmap->GetResolution(&xDpi, &yDpi);
	pFrame->SetResolution(xDpi, yDpi);

	const WICRect rc{ 0,0,(int)cx,(int)cy };
	IWICBitmapLock* pLock;
	if (FAILED(hr = pBitmap->Lock(&rc, WICBitmapLockRead, &pLock)))// 取位图锁
		goto Exit1;

	UINT uStride;
	pLock->GetStride(&uStride);

	APTTYPE iAptType;
	APTTYPEQUALIFIER iAptQualifier;
	(void)CoGetApartmentType(&iAptType, &iAptQualifier);
	if (iAptType == APTTYPE_MTA)// 多线程套间无法使用位图锁读取数据
	{
		const auto cbBuf = cy * uStride;
		auto pData = VirtualAlloc(NULL, cbBuf, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
		EckAssert(pData);
		if (FAILED(hr = pBitmap->CopyPixels(NULL, uStride, cbBuf, (BYTE*)pData)))
		{
			VirtualFree(pData, 0, MEM_RELEASE);
			goto Exit;
		}
		if (FAILED(hr = pFrame->WritePixels(cy, uStride, cbBuf, (BYTE*)pData)))// 写入
		{
			VirtualFree(pData, 0, MEM_RELEASE);
			goto Exit;
		}
		VirtualFree(pData, 0, MEM_RELEASE);
	}
	else
	{
		UINT cbBuf;
		WICInProcPointer pData;
		if (FAILED(hr = pLock->GetDataPointer(&cbBuf, &pData)))
			goto Exit;
#pragma warning(suppress:6387)// 可能为NULL
		if (FAILED(hr = pFrame->WritePixels(cy, uStride, cbBuf, pData)))// 写入
			goto Exit;
	}
	
	hr = pFrame->Commit();// 提交帧
	if (SUCCEEDED(hr))
		hr = pEncoder->Commit();// 提交编码器
Exit:
	pLock->Release();
Exit1:
	pFrame->Release();
	pEncoder->Release();
	pPropBag->Release();
	return hr;
}

inline CRefBin SaveWicBitmap(IWICBitmap* pBitmap, ImageType iType = ImageType::Png, HRESULT* phr = NULL)
{
	CRefBin rb{};
	auto pStream = new CRefBinStream(rb);
	auto hr = SaveWicBitmap(pStream, pBitmap, iType);
	if (phr)
		*phr = hr;
	pStream->LeaveRelease();
	return rb;
}

inline HRESULT SaveWicBitmap(PCWSTR pszFile, IWICBitmap* pBitmap, ImageType iType = ImageType::Png)
{
	HRESULT hr;
	IWICStream* pStream;
	if (FAILED(hr = g_pWicFactory->CreateStream(&pStream)))
		return hr;
	pStream->InitializeFromFilename(pszFile, GENERIC_READ | GENERIC_WRITE);
	hr = SaveWicBitmap(pStream, pBitmap, iType);
	pStream->Release();
	return hr;
}

inline GpStatus SaveGpImage(IStream* pStream, GpImage* pImage, ImageType iType = ImageType::Png)
{
	CLSID clsid;
	GpStatus gps;
	if ((gps = GetGpEncoderClsid(GetImageMime(iType), clsid)) != GpStatus::GpOk)
		return gps;
	return GdipSaveImageToStream(pImage, pStream, &clsid, NULL);
}

inline CRefBin SaveGpImage(GpImage* pImage, ImageType iType = ImageType::Png, GpStatus* pgps = NULL)
{
	CRefBin rb{};
	auto pStream = new CRefBinStream(rb);
	auto gps = SaveGpImage(pStream, pImage, iType);
	pStream->LeaveRelease();
	if (pgps)
		*pgps = gps;
	return rb;
}

inline GpStatus SaveGpImage(PCWSTR pszFile, GpImage* pImage, ImageType iType = ImageType::Png)
{
	CLSID clsid;
	GpStatus gps;
	if ((gps = GetGpEncoderClsid(GetImageMime(iType), clsid)) != GpStatus::GpOk)
		return gps;
	return GdipSaveImageToFile(pImage, pszFile, &clsid, NULL);
}
ECK_NAMESPACE_END