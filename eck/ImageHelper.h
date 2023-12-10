﻿/*
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

#include <numbers>
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

inline HRESULT CreateWicBitmap(IWICBitmap*& pBmp,
	IWICBitmapDecoder* pDecoder, IWICImagingFactory* pWicFactory)
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

	hr = pWicFactory->CreateBitmapFromSource(pConverter, WICBitmapNoCache, &pBmp);
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
/// <param name="pWicFactory">WIC工厂</param>
/// <returns>成功返回S_OK，失败返回错误代码</returns>
EckInline HRESULT CreateWicBitmapDecoder(PCWSTR pszFile,
	IWICBitmapDecoder*& pDecoder, IWICImagingFactory* pWicFactory)
{
	pDecoder = NULL;
	return pWicFactory->CreateDecoderFromFilename(pszFile, NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pDecoder);
}

/// <summary>
/// 创建WIC位图解码器
/// </summary>
/// <param name="pStream">流对象</param>
/// <param name="pDecoder">接收解码器变量引用</param>
/// <param name="pWicFactory">WIC工厂</param>
/// <returns>成功返回S_OK，失败返回错误代码</returns>
EckInline HRESULT CreateWicBitmapDecoder(IStream* pStream,
	IWICBitmapDecoder*& pDecoder, IWICImagingFactory* pWicFactory)
{
	pDecoder = NULL;
	return pWicFactory->CreateDecoderFromStream(pStream, NULL, WICDecodeMetadataCacheOnDemand, &pDecoder);
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
	const BYTE* m_pData = NULL;
	const ICONDIR* m_pHeader = NULL;
	const ICONDIRENTRY* m_pEntry = NULL;
public:
	EckInline int AnalyzeData(const BYTE* pData)
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
	DIBSECTION m_ds{};
public:
	CDib() = default;
	CDib(const CDib&) = delete;
	CDib& operator=(const CDib&) = delete;

	CDib(CDib&& x) noexcept
		:m_hBitmap{ x.m_hBitmap }, m_ds{ x.m_ds } {}
	CDib& operator=(CDib&& x) noexcept
	{
		m_hBitmap = x.m_hBitmap;
		m_ds = x.m_ds;
	}

	~CDib()
	{
		Destroy();
	}

	/// <summary>
	/// 创建自DC。
	/// 使用指定的调色板从DC中选入的位图创建一个DIB节，
	/// 函数先获取位图的BITMAP结构并检索调色板条目，
	/// 然后使用DIB_RGB_COLORS标志创建一个DIB节，
	/// 最后使用BitBlt拷贝位图数据
	/// </summary>
	/// <param name="hDC">设备场景</param>
	/// <param name="iTopToBottom">位图原点选项，<0 - 自上而下  >=0 - 自下而上</param>
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
		GetObjectW(m_hBitmap, sizeof(m_ds), &m_ds);
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
			GetObjectW(m_hBitmap, sizeof(m_ds), &m_ds);
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
				auto pStream = new IStreamView(pBitsData, cbBits);
				IWICBitmapDecoder* pDecoder;
				if (FAILED(CreateWicBitmapDecoder(pStream, pDecoder, g_pWicFactory)))
				{
					pStream->Release();
					return NULL;
				}
				pStream->Release();
				IWICBitmap* pBitmap;
				if (FAILED(CreateWicBitmap(pBitmap, pDecoder, g_pWicFactory)))
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
				memcpy(pBits, pBitsData, cbBits);
				GetObjectW(m_hBitmap, sizeof(m_ds), &m_ds);
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
		GetObjectW(m_hBitmap, sizeof(m_ds), &m_ds);
		return m_hBitmap;
	}

	EckInline HBITMAP GetHBitmap() const { return m_hBitmap; }

	EckInline SIZE_T GetBmpDataSize() const
	{
		return sizeof(BITMAPFILEHEADER) +
			sizeof(BITMAPINFOHEADER) +
			sizeof(RGBQUAD) * m_ds.dsBmih.biClrUsed +
			m_ds.dsBm.bmWidthBytes * m_ds.dsBm.bmHeight;
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
		pih->biWidth = m_ds.dsBm.bmWidth;
		pih->biHeight = m_ds.dsBm.bmHeight;
		pih->biPlanes = 1;
		pih->biBitCount = m_ds.dsBm.bmBitsPixel;
		pih->biCompression = 0;
		pih->biXPelsPerMeter = pih->biYPelsPerMeter = 0;
		pih->biClrUsed = m_ds.dsBmih.biClrUsed;
		pih->biClrImportant = m_ds.dsBmih.biClrImportant;
		// 制颜色表
		const size_t cbPalette = sizeof(RGBQUAD) * m_ds.dsBmih.biClrUsed;
		if (cbPalette)
		{
			HDC hCDC = CreateCompatibleDC(NULL);
			SelectObject(hCDC, m_hBitmap);
			GetDIBColorTable(hCDC, 0, m_ds.dsBmih.biClrUsed, (RGBQUAD*)(pih + 1));
			DeleteDC(hCDC);
		}
		// 制像素
		memcpy(((BYTE*)(pih + 1)) + cbPalette, m_ds.dsBm.bmBits,
			m_ds.dsBm.bmWidthBytes * m_ds.dsBm.bmHeight);
		return rb;
	}

	EckInline void Destroy()
	{
		DeleteObject(m_hBitmap);
		m_hBitmap = NULL;
	}

	EckInline const DIBSECTION& GetDibSectionStruct() const { return m_ds; }

	HBITMAP Attach(HBITMAP hBitmap, const DIBSECTION* pds = NULL)
	{
		std::swap(m_hBitmap, hBitmap);
		if (pds)
			m_ds = *pds;
		else
			GetObjectW(m_hBitmap, sizeof(m_ds), &m_ds);
		return hBitmap;
	}

	HBITMAP Detach()
	{
		auto t = m_hBitmap;
		m_hBitmap = NULL;
		return t;
	}
};

inline CRefBin SaveWicBitmap(IWICBitmap* pBitmap)
{
	return {};
}

inline CRefBin SaveGpBitmap(GpBitmap* pBitmap)
{

	return {};
}
ECK_NAMESPACE_END