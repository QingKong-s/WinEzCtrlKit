/*
* WinEzCtrlKit Library
*
* MultiFrameImage.h ： 多帧图像处理
*
* Copyright(C) 2024 QingKong
*/
#pragma once
#include "ImageHelper.h"

ECK_NAMESPACE_BEGIN
struct ICONDIRENTRY
{
	BYTE bWidth;		// 宽度
	BYTE bHeight;		// 高度
	BYTE bColorCount;	// 颜色数
	BYTE bReserved;		// 保留，必须为0
	WORD wPlanes;		// 颜色平面数，必须为1
	WORD wBitCount;		// 位深度
	DWORD dwBytesInRes;	// 在资源中的字节数
	DWORD dwImageOffset;// 在映像文件中的偏移
};

struct ICONDIR
{
	WORD idReserved;	// 保留，必须为0
	WORD idType;		// 资源类型，1 = 图标
	WORD idCount;		// 图标数
	// ICONDIRENTRY idEntries[1]; // 图像数组
};

class CMultiFrameImageWic
{
public:
	enum class Disposal
	{
		Undefined,
		None,
		Background,
		Previous
	};
private:
	struct ITEM
	{
		IWICBitmapSource* pBitmap;
		int msDelay;
		int ox;
		int oy;
		int cx;
		int cy;
		Disposal eDisposalMethod;
	};
	std::vector<ITEM> m_Frames{};
	int m_cRepeat{};
	int m_cx{};
	int m_cy{};
	BOOL m_bTransparent{};
	Disposal m_eDisposalMethod{};
	WICColor m_crBkg{};
public:
	ECKPROP(GetRepeatCount, SetRepeatCount)	int RepeatCount;
	ECKPROP(GetHeight, SetHeight)			int Height;
	ECKPROP(GetWidth, SetWidth)				int Width;
	ECKPROP(GetTransparent, SetTransparent) BOOL Transparent;
	ECKPROP(GetDisposalMethod, SetDisposalMethod) Disposal DisposalMethod;
	ECKPROP_R(GetFrameCount)				int FrameCount;
	ECKPROP(GetBackgroundColor, SetBackgroundColor) WICColor BackgroundColor;

	ECK_DISABLE_COPY_MOVE_DEF_CONS(CMultiFrameImageWic);

	/// <summary>
	/// 添加帧
	/// </summary>
	/// <param name="pBitmap">WIC位图源</param>
	/// <param name="msDelay">以毫秒计的显示延迟</param>
	/// <param name="ox">X偏移</param>
	/// <param name="oy">Y偏移</param>
	/// <param name="cx">新宽度，0表示不变</param>
	/// <param name="cy">新高度，0表示不变</param>
	/// <param name="eDisposalMethod">处理方式</param>
	/// <param name="eInterpolation">缩放时使用的插值方式</param>
	/// <returns>HRESULT</returns>
	HRESULT AddFrame(IWICBitmapSource* pBitmap, int msDelay, 
		int ox = 0, int oy = 0, int cx = 0, int cy = 0,
		Disposal eDisposalMethod = Disposal::Undefined,
		WICBitmapInterpolationMode eInterpolation = WICBitmapInterpolationModeLinear)
	{
		EckAssert(cx >= 0 && cy >= 0);
		if (ox || oy || cx || cy)
		{
			// TODO: 限制范围
			HRESULT hr;
			if (cx || cy)
			{
				IWICBitmap* pNew;
				if (FAILED(hr = ScaleWicBitmap(pBitmap, pNew, cx, cy, eInterpolation)))
					return hr;
				pBitmap = pNew;
			}
			else
			{
				pBitmap->AddRef();
				pBitmap->GetSize((UINT*)&cx, (UINT*)&cy);
			}
			m_Frames.emplace_back(pBitmap, msDelay, ox, oy, cx, cy, eDisposalMethod);
		}
		else
		{
			pBitmap->AddRef();
			pBitmap->GetSize((UINT*)&cx, (UINT*)&cy);
			m_Frames.emplace_back(pBitmap, msDelay, ox, oy, cx, cy, eDisposalMethod);
		}
		return S_OK;
	}

	/// <summary>
	/// 添加帧。
	/// 添加WIC位图解码器中的所有帧
	/// </summary>
	/// <param name="pDecoder">WIC位图解码器</param>
	/// <param name="msDelay">以毫秒计的显示延迟</param>
	/// <param name="ox">X偏移</param>
	/// <param name="oy">Y偏移</param>
	/// <param name="cx">新宽度，0表示不变</param>
	/// <param name="cy">新高度，0表示不变</param>
	/// <param name="eDisposalMethod">处理方式</param>
	/// <param name="eInterpolation">缩放时使用的插值方式</param>
	/// <returns>HRESULT</returns>
	HRESULT AddFrame(IWICBitmapDecoder* pDecoder, int msDelay,
		int ox = 0, int oy = 0, int cx = 0, int cy = 0,
		Disposal eDisposalMethod = Disposal::Undefined,
		WICBitmapInterpolationMode eInterpolation = WICBitmapInterpolationModeLinear)
	{
		HRESULT hr;
		UINT cFrame;
		pDecoder->GetFrameCount(&cFrame);
		EckCounter(cFrame, i)
		{
			ComPtr<IWICBitmapFrameDecode> pFrame;
			if (FAILED(hr = pDecoder->GetFrame(i, &pFrame)))
				return hr;
			if (FAILED(hr = AddFrame(pFrame.Get(), msDelay, 
				ox, oy, cx, cy, eDisposalMethod, eInterpolation)))
				return hr;
		}
		return S_OK;
	}

	/// <summary>
	/// 添加帧
	/// </summary>
	/// <param name="pBmps">WIC位图源数组</param>
	/// <param name="cBmps">数组中的元素数</param>
	/// <param name="msDelay">以毫秒计的显示延迟</param>
	/// <param name="ox">X偏移</param>
	/// <param name="oy">Y偏移</param>
	/// <param name="cx">新宽度，0表示不变</param>
	/// <param name="cy">新高度，0表示不变</param>
	/// <param name="eDisposalMethod">处理方式</param>
	/// <param name="eInterpolation">缩放时使用的插值方式</param>
	/// <returns>HRESULT</returns>
	HRESULT AddFrame(IWICBitmapSource** pBmps, int cBmps, int msDelay,
		int ox = 0, int oy = 0, int cx = 0, int cy = 0,
		Disposal eDisposalMethod = Disposal::Undefined,
		WICBitmapInterpolationMode eInterpolation = WICBitmapInterpolationModeLinear)
	{
		HRESULT hr;
		EckCounter(cBmps, i)
		{
			if (FAILED(hr = AddFrame(pBmps[i], msDelay, 
				ox, oy, cx, cy, eDisposalMethod, eInterpolation)))
				return hr;
		}
		return S_OK;
	}

	int GetFrameCount() const { return (int)m_Frames.size(); }

	EckInline void SetRepeatCount(int c) { m_cRepeat = c; }

	EckInline int GetRepeatCount() const { return m_cRepeat; }

	EckInline void SetHeight(int h) { m_cy = h; }

	EckInline int GetHeight() const { return m_cy; }

	EckInline void SetWidth(int w) { m_cx = w; }

	EckInline int GetWidth() const { return m_cx; }

	/// <summary>
	/// 置是否透明。
	/// 若为TRUE，则将所有阿尔法值小于50（约20%）的像素视为透明
	/// </summary>
	EckInline void SetTransparent(BOOL b) { m_bTransparent = b; }

	EckInline BOOL GetTransparent() const { return m_bTransparent; }

	EckInline void SetDisposalMethod(Disposal e) { m_eDisposalMethod = e; }

	EckInline Disposal GetDisposalMethod() const { return m_eDisposalMethod; }

	EckInline WICColor GetBackgroundColor() const { return m_crBkg; }

	EckInline void SetBackgroundColor(WICColor cr) { m_crBkg = cr; }

	void Clear()
	{
		for (const auto& e : m_Frames)
			e.pBitmap->Release();
		m_Frames.clear();
		m_cRepeat = 0;
		m_cx = 0;
		m_cy = 0;
		m_bTransparent = FALSE;
		m_eDisposalMethod = Disposal::Undefined;
		m_crBkg = 0u;
	}

	/// <summary>
	/// 保存为GIF
	/// </summary>
	/// <param name="pStream">流</param>
	/// <returns>HRESULT</returns>
	HRESULT SaveAsGif(IStream* pStream)
	{
		HRESULT hr;
		PROPVARIANT Var{};
		ComPtr<IWICBitmapEncoder> pEncoder;
		g_pWicFactory->CreateEncoder(GUID_ContainerFormatGif, NULL, &pEncoder);
		if (FAILED(hr = pEncoder->Initialize(pStream, WICBitmapEncoderNoCache)))
			return hr;
		// 写循环次数
		ComPtr<IWICMetadataQueryWriter> pMdWriter;
		if (FAILED(hr = pEncoder->GetMetadataQueryWriter(&pMdWriter)))
			return hr;
		BYTE byApp[]{ 'N','E','T','S','C','A','P','E','2','.','0' };// 或ANIMEXTS1.0
		Var.vt = VT_UI1 | VT_VECTOR;
		Var.caub.cElems = sizeof(byApp);
		Var.caub.pElems = byApp;
		if (FAILED(hr = pMdWriter->SetMetadataByName(L"/appext/Application", &Var)))
			return hr;

		BYTE byData[]
		{
			0x03,0x01,
			(BYTE)(m_cRepeat),(BYTE)(m_cRepeat >> 8),
			0x00
		};
		Var.caub.cElems = sizeof(byData);
		Var.caub.pElems = byData;
		if (FAILED(hr = pMdWriter->SetMetadataByName(L"/appext/Data", &Var)))
			return hr;
		// 写宽高
		if (m_cx && m_cy)
		{
			Var.vt = VT_UI2;
			Var.uiVal = m_cx;
			if (FAILED(hr = pMdWriter->SetMetadataByName(L"/logscrdesc/Width", &Var)))
				return hr;
			Var.uiVal = m_cy;
			if (FAILED(hr = pMdWriter->SetMetadataByName(L"/logscrdesc/Height", &Var)))
				return hr;
		}
		ComPtr<IWICPalette> pPalette;
		// 写背景色
		if (m_crBkg)
		{
			g_pWicFactory->CreatePalette(&pPalette);
			pPalette->InitializeCustom(&m_crBkg, 1);
			if (FAILED(hr = pEncoder->SetPalette(pPalette.Get())))
				return hr;
			Var.vt = VT_BOOL;
			Var.boolVal = VARIANT_TRUE;
			hr = pMdWriter->SetMetadataByName(L"/logscrdesc/GlobalColorTableFlag", &Var);
			Var.vt = VT_UI1;
			Var.bVal = 0;
			hr = pMdWriter->SetMetadataByName(L"/logscrdesc/BackgroundColorIndex", &Var);
		}
		for (const auto& e : m_Frames)
		{
			ComPtr<IWICBitmapFrameEncode> pFrameEncode;
			ComPtr<IWICMetadataQueryWriter> pMdWriter;

			if (FAILED(hr = pEncoder->CreateNewFrame(&pFrameEncode, NULL)))
				return hr;
			if (FAILED(hr = pFrameEncode->Initialize(NULL)))
				return hr;
			if (FAILED(hr = pFrameEncode->GetMetadataQueryWriter(&pMdWriter)))
				return hr;
			// 写帧延迟
			Var.vt = VT_UI2;
			Var.uiVal = e.msDelay / 10;
			pMdWriter->SetMetadataByName(L"/grctlext/Delay", &Var);
			// 写偏移
			if (e.ox)
			{
				Var.uiVal = e.ox;
				pMdWriter->SetMetadataByName(L"/imgdesc/Left", &Var);
			}
			if (e.oy)
			{
				Var.uiVal = e.oy;
				pMdWriter->SetMetadataByName(L"/imgdesc/Top", &Var);
			}
			// 写擦除标志
			if (m_eDisposalMethod != Disposal::Undefined)
			{
				Var.vt = VT_UI1;
				Var.bVal = (BYTE)m_eDisposalMethod;
				pMdWriter->SetMetadataByName(L"/grctlext/Disposal", &Var);
			}
			// 写透明标志
			if (m_crBkg || m_bTransparent)
			{
				ComPtr<IWICPalette> pPalette;
				g_pWicFactory->CreatePalette(&pPalette);
				if (FAILED(hr = pPalette->InitializeFromBitmap(e.pBitmap, 256, TRUE)))
					return hr;
				if (m_bTransparent)
				{
					UINT cClr{}, cActualClr;
					pPalette->GetColorCount(&cClr);
					if (!cClr)
						return E_FAIL;
					const auto pcr = new WICColor[cClr];
					pPalette->GetColors(cClr, pcr, &cActualClr);
					const auto it = std::find(pcr, pcr + cClr, 0);
					if (it != pcr + cClr)
					{
						Var.vt = VT_UI1;
						Var.bVal = (BYTE)(it - pcr);
						pMdWriter->SetMetadataByName(L"/grctlext/TransparentColorIndex", &Var);
						Var.vt = VT_BOOL;
						Var.boolVal = VARIANT_TRUE;
						pMdWriter->SetMetadataByName(L"/grctlext/TransparencyFlag", &Var);
					}
					delete[] pcr;
				}
				ComPtr<IWICFormatConverter> pConverter;
				g_pWicFactory->CreateFormatConverter(&pConverter);
				hr = pConverter->Initialize(
					e.pBitmap,
					GUID_WICPixelFormat8bppIndexed,
					WICBitmapDitherTypeNone,
					pPalette.Get(),
					50. / 255.,
					WICBitmapPaletteTypeCustom);
				if (FAILED(hr))
					return hr;
				pFrameEncode->SetPalette(pPalette.Get());
				if (FAILED(hr = pFrameEncode->WriteSource(pConverter.Get(), NULL)))
					return hr;
			}
			else
				if (FAILED(hr = pFrameEncode->WriteSource(e.pBitmap, NULL)))
					return hr;
			if (FAILED(hr = pFrameEncode->Commit()))
				return hr;
		}
		return pEncoder->Commit();
	}

	HRESULT SaveAsApng(IStream* pStream)
	{

	}
};

class CIcoFileReader
{
private:
	PCBYTE m_pData = NULL;
	const ICONDIR* m_pHeader = NULL;
	const ICONDIRENTRY* m_pEntry = NULL;
public:
	CIcoFileReader(PCBYTE pData)
	{
		AnalyzeData(pData);
	}

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
ECK_NAMESPACE_END