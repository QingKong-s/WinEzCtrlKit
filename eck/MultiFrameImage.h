#pragma once
#include "ImageHelper.h"
#include "CStreamWalker.h"
#include "CMemWalker.h"
#include "Utility2.h"

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

enum class PngColorType :BYTE
{
	Grayscale = 0,
	Truecolor = 2,
	Indexed = 3,
	GrayscaleAlpha = 4,
	TruecolorAlpha = 6
};

struct PngIHDR
{
	BYTE Width[4];		// 宽度
	BYTE Height[4];		// 高度
	BYTE BitDepth;		// 位深度
	PngColorType ColorType;	// 颜色类型
	BYTE CompressionMethod;	// 压缩方法，仅定义0
	BYTE FilterMethod;		// 过滤方法，仅定义0
	BYTE InterlaceMethod;	// 隔行扫描，0 = 不隔行扫描，1 = Adm7隔行扫描
};

enum class PngBlendOp :BYTE
{
	Source,
	Over
};

static_assert(sizeof(PngIHDR) == 13);

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
	int m_cxAct{};
	int m_cyAct{};
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
		if (cx || cy)
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
		m_cxAct = std::max(m_cxAct, ox + cx);
		m_cyAct = std::max(m_cyAct, oy + cy);
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
		g_pWicFactory->CreateEncoder(GUID_ContainerFormatGif, nullptr, &pEncoder);
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

			if (FAILED(hr = pEncoder->CreateNewFrame(&pFrameEncode, nullptr)))
				return hr;
			if (FAILED(hr = pFrameEncode->Initialize(nullptr)))
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
				if (FAILED(hr = pFrameEncode->WriteSource(pConverter.Get(), nullptr)))
					return hr;
			}
			else
				if (FAILED(hr = pFrameEncode->WriteSource(e.pBitmap, nullptr)))
					return hr;
			if (FAILED(hr = pFrameEncode->Commit()))
				return hr;
		}
		return pEncoder->Commit();
	}

	/// <summary>
	/// 保存为APNG。
	/// </summary>
	/// <param name="pStream">流</param>
	/// <returns>HRESULT</returns>
	HRESULT SaveAsApng(IStream* pStream, PngBlendOp eBlendOp = PngBlendOp::Source,
		BOOL bInterlace = FALSE, WICPngFilterOption eFilter = WICPngFilterUnspecified)
	{
		constexpr BYTE PngSignature[]{ 0x89,0x50,0x4E,0x47,0x0D,0x0A,0x1A,0x0A };
		constexpr BYTE IdIHDR[]{ 0x49,0x48,0x44,0x52 };
		constexpr BYTE IdacTL[]{ 0x61,0x63,0x54,0x4C };
		constexpr BYTE IdfcTL[]{ 0x66,0x63,0x54,0x4C };
		constexpr BYTE IdfdAT[]{ 0x66,0x64,0x41,0x54 };
		constexpr BYTE IdIDAT[]{ 0x49,0x44,0x41,0x54 };
		constexpr BYTE IdIEND[]{ 0x49,0x45,0x4E,0x44 };
		CStreamWalker w(pStream);
		w << PngSignature;
		// 写IHDR
		BYTE byBuf[36];
		const auto posIhdr = w.GetPos();
		constexpr size_t CbIHDR = 13 + 4 + 4 + 4;
		w += CbIHDR;// 悬而未决，待后续写入
		// 写acTL
		CMemWalker wkChunk(byBuf, sizeof(byBuf));
		wkChunk << ReverseInteger(8u) << IdacTL
			<< ReverseInteger((UINT)m_Frames.size())
			<< ReverseInteger((UINT)m_cRepeat);
		w.Write(byBuf, wkChunk.GetPos())
			<< ReverseInteger(CalcCrc32(byBuf + 4, wkChunk.GetPos() - 4));
		wkChunk.MoveToBegin();
		// 写入帧数据
		UINT uSerialNum{};
		CRefBin rbPng{};
		ComPtr<CRefBinStream> pPngStream(new CRefBinStream(rbPng));
		CHAR chChunkId[4];
		UINT cbChunkData;
		BOOL bFirstFrame{ TRUE };
		BYTE* pChunkIdBegin;
		for (const auto& e : m_Frames)
		{
			BYTE byDisposalMethod;
			if (e.eDisposalMethod == Disposal::Undefined)
				byDisposalMethod =
				(m_eDisposalMethod == Disposal::Undefined) ?
				((BYTE)m_eDisposalMethod - 1) : 0;
			else
				byDisposalMethod = (BYTE)e.eDisposalMethod - 1;
			// 写fcTL
			wkChunk << ReverseInteger(26u) << IdfcTL
				<< ReverseInteger(uSerialNum++)
				<< ReverseInteger(e.cx)
				<< ReverseInteger(e.cy)
				<< ReverseInteger(e.ox)
				<< ReverseInteger(e.oy)
				<< ReverseInteger((USHORT)(e.msDelay / 10))
				<< ReverseInteger(0_us)// 1/100秒
				<< byDisposalMethod
				<< eBlendOp
				;
			w.Write(byBuf, wkChunk.GetPos())
				<< ReverseInteger(CalcCrc32(byBuf + 4, wkChunk.GetPos() - 4));
			wkChunk.MoveToBegin();
			// 写IDAT或fdAT
			pPngStream->Seek(ToLi(0), STREAM_SEEK_SET, nullptr);

			HRESULT hr;
			ComPtr<IWICBitmapEncoder> pEncoder;
			ComPtr<IWICBitmapFrameEncode> pFrame;
			if (FAILED(hr = g_pWicFactory->CreateEncoder(GUID_ContainerFormatPng, nullptr, &pEncoder)))
				return hr;
			if (FAILED(hr = pEncoder->Initialize(pPngStream.Get(), WICBitmapEncoderNoCache)))
				return hr;
			if (bInterlace || (eFilter != WICPngFilterUnspecified))
			{
				ComPtr<IPropertyBag2> pPropBag;
				if (FAILED(hr = pEncoder->CreateNewFrame(&pFrame, &pPropBag)))
					return hr;
				PROPBAG2 Prop{ .pstrName = (PWSTR)L"InterlaceOption" };
				VARIANT Var{};
				Var.vt = VT_BOOL;
				Var.bVal = (bInterlace ? VARIANT_TRUE : VARIANT_FALSE);
				pPropBag->Write(1, &Prop, &Var);
				Prop.pstrName = (PWSTR)L"FilterOption";
				Var.vt = VT_UI1;
				Var.bVal = (BYTE)eFilter;
				pPropBag->Write(1, &Prop, &Var);
				if (FAILED(hr = pFrame->Initialize(pPropBag.Get())))
					return hr;
			}
			else
			{
				if (FAILED(hr = pEncoder->CreateNewFrame(&pFrame, nullptr)))
					return hr;
				if (FAILED(hr = pFrame->Initialize(nullptr)))
					return hr;
			}
			GUID guidFmt{ GUID_WICPixelFormat32bppBGRA };
			pFrame->SetPixelFormat(&guidFmt);
			if (FAILED(hr = pFrame->WriteSource(e.pBitmap, nullptr)))
				return hr;
			if (FAILED(hr = pFrame->Commit()))
				return hr;
			if (FAILED(hr= pEncoder->Commit()))
				return hr;
			CMemWalker wt(rbPng.Data(), rbPng.Size());
			wt += 8;// 跳过PNG签名
			EckLoop()
			{
				wt.ReadRev(cbChunkData) >> chChunkId;
				if (memcmp(chChunkId, IdIHDR, 4) == 0)
				{
					if (bFirstFrame)// 复制第一帧IHDR到流首部
					{
						const auto pos = w.GetPos();
						w.GetStream()->Seek(ToLi(posIhdr), STREAM_SEEK_SET, nullptr);
						w.Write(wt.Data() - 8, CbIHDR);
						w.GetStream()->Seek(ToLi(pos), STREAM_SEEK_SET, nullptr);
					}
					wt += (cbChunkData + 4);// 跳过数据和CRC
				}
				else if (memcmp(chChunkId, IdIDAT, 4) == 0)
				{
					if (bFirstFrame)
						pChunkIdBegin = wt.Data() - 4;
					else
					{
						pChunkIdBegin = wt.Data() - 8;
						memcpy(pChunkIdBegin, IdfdAT, 4);
						memcpy(pChunkIdBegin + 4, &uSerialNum, 4);
						++uSerialNum;
						cbChunkData += 4;
					}
					w << ReverseInteger(cbChunkData);
					w.Write(pChunkIdBegin, cbChunkData + 4);
					if (bFirstFrame)
					{
						w.Write(pChunkIdBegin + cbChunkData + 4, 4);
						bFirstFrame = FALSE;
					}
					else
						w << ReverseInteger(CalcCrc32(pChunkIdBegin, cbChunkData + 4));
					break;
				}
				else if (memcmp(chChunkId, IdIEND, 4) == 0)
					break;
				else
					wt += (cbChunkData + 4);// 跳过数据和CRC
			}
		}
		// 写IEND
		w << 0u << IdIEND << ReverseInteger(CalcCrc32(IdIEND, 4));
		return S_OK;
	}

	/// <summary>
	/// 保存为TIFF.
	/// 将忽略下列属性：重复次数、是否透明、背景颜色、处理方式
	/// </summary>
	/// <param name="pStream">流</param>
	/// <returns>HRESULT</returns>
	HRESULT SaveAsTiff(IStream* pStream, float fCompressionQuality = 0.f,
		WICTiffCompressionOption eCompression = WICTiffCompressionDontCare)
	{
		HRESULT hr;
		ComPtr<IWICBitmapEncoder> pEncoder;
		g_pWicFactory->CreateEncoder(GUID_ContainerFormatTiff, nullptr, &pEncoder);
		if (FAILED(hr = pEncoder->Initialize(pStream, WICBitmapEncoderNoCache)))
			return hr;
		for (const auto& e : m_Frames)
		{
			ComPtr<IWICBitmapFrameEncode> pFrameEncode;
			if (fCompressionQuality > 0.f || eCompression != WICTiffCompressionDontCare)
			{
				ComPtr<IPropertyBag2> pPropBag;
				if (FAILED(hr = pEncoder->CreateNewFrame(&pFrameEncode, &pPropBag)))
					return hr;
				PROPBAG2 Prop{ .pstrName = (PWSTR)L"TiffCompressionMethod" };
				VARIANT Var{};
				Var.vt = VT_UI1;
				Var.bVal = eCompression;
				pPropBag->Write(1, &Prop, &Var);
				Prop.pstrName = (PWSTR)L"TiffCompressionQuality";
				Var.vt = VT_R4;
				Var.fltVal = fCompressionQuality;
				pPropBag->Write(1, &Prop, &Var);
				if (FAILED(hr = pFrameEncode->Initialize(pPropBag.Get())))
					return hr;
			}
			else
			{
				if (FAILED(hr = pEncoder->CreateNewFrame(&pFrameEncode, nullptr)))
					return hr;
				if (FAILED(hr = pFrameEncode->Initialize(nullptr)))
					return hr;
			}
			if (FAILED(hr = pFrameEncode->WriteSource(e.pBitmap, nullptr)))
				return hr;
			if (FAILED(hr = pFrameEncode->Commit()))
				return hr;
		}
		return pEncoder->Commit();
	}
};

class CIcoFileReader
{
private:
	PCBYTE m_pData = nullptr;
	const ICONDIR* m_pHeader = nullptr;
	const ICONDIRENTRY* m_pEntry = nullptr;
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