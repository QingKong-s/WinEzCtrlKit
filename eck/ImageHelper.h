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
ECK_NAMESPACE_END