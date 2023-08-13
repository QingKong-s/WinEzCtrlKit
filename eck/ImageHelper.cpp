#include "ImageHelper.h"

ECK_NAMESPACE_BEGIN
HBITMAP CreateHBITMAP(PCVOID pData, SIZE_T cbData)
{
	HBITMAP hbm;
	GpBitmap* pBitmap;
	IStream* pStream = SHCreateMemStream((PCBYTE)pData, (UINT)cbData);
	if (!pStream)
		return NULL;

	if (GdipCreateBitmapFromStream(pStream, &pBitmap) != Ok)
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

	if (GdipCreateBitmapFromFile(pszFile, &pBitmap) != Ok)
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

HICON CreateHICON(PCVOID pData, SIZE_T cbData)
{
	HICON hIcon;
	GpBitmap* pBitmap;
	IStream* pStream = SHCreateMemStream((PCBYTE)pData, (UINT)cbData);
	if (!pStream)
		return NULL;

	if (GdipCreateBitmapFromStream(pStream, &pBitmap) != Ok)
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

	if (GdipCreateBitmapFromFile(pszFile, &pBitmap) != Ok)
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
ECK_NAMESPACE_END