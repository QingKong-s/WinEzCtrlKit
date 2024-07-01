/*
* WinEzCtrlKit Library
*
* MultiFrameImage.h �� ��֡ͼ����
*
* Copyright(C) 2024 QingKong
*/
#pragma once
#include "ImageHelper.h"

ECK_NAMESPACE_BEGIN
struct ICONDIRENTRY
{
	BYTE bWidth;		// ���
	BYTE bHeight;		// �߶�
	BYTE bColorCount;	// ��ɫ��
	BYTE bReserved;		// ����������Ϊ0
	WORD wPlanes;		// ��ɫƽ����������Ϊ1
	WORD wBitCount;		// λ���
	DWORD dwBytesInRes;	// ����Դ�е��ֽ���
	DWORD dwImageOffset;// ��ӳ���ļ��е�ƫ��
};

struct ICONDIR
{
	WORD idReserved;	// ����������Ϊ0
	WORD idType;		// ��Դ���ͣ�1 = ͼ��
	WORD idCount;		// ͼ����
	// ICONDIRENTRY idEntries[1]; // ͼ������
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
	/// ���֡
	/// </summary>
	/// <param name="pBitmap">WICλͼԴ</param>
	/// <param name="msDelay">�Ժ���Ƶ���ʾ�ӳ�</param>
	/// <param name="ox">Xƫ��</param>
	/// <param name="oy">Yƫ��</param>
	/// <param name="cx">�¿�ȣ�0��ʾ����</param>
	/// <param name="cy">�¸߶ȣ�0��ʾ����</param>
	/// <param name="eDisposalMethod">����ʽ</param>
	/// <param name="eInterpolation">����ʱʹ�õĲ�ֵ��ʽ</param>
	/// <returns>HRESULT</returns>
	HRESULT AddFrame(IWICBitmapSource* pBitmap, int msDelay, 
		int ox = 0, int oy = 0, int cx = 0, int cy = 0,
		Disposal eDisposalMethod = Disposal::Undefined,
		WICBitmapInterpolationMode eInterpolation = WICBitmapInterpolationModeLinear)
	{
		EckAssert(cx >= 0 && cy >= 0);
		if (ox || oy || cx || cy)
		{
			// TODO: ���Ʒ�Χ
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
	/// ���֡��
	/// ���WICλͼ�������е�����֡
	/// </summary>
	/// <param name="pDecoder">WICλͼ������</param>
	/// <param name="msDelay">�Ժ���Ƶ���ʾ�ӳ�</param>
	/// <param name="ox">Xƫ��</param>
	/// <param name="oy">Yƫ��</param>
	/// <param name="cx">�¿�ȣ�0��ʾ����</param>
	/// <param name="cy">�¸߶ȣ�0��ʾ����</param>
	/// <param name="eDisposalMethod">����ʽ</param>
	/// <param name="eInterpolation">����ʱʹ�õĲ�ֵ��ʽ</param>
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
	/// ���֡
	/// </summary>
	/// <param name="pBmps">WICλͼԴ����</param>
	/// <param name="cBmps">�����е�Ԫ����</param>
	/// <param name="msDelay">�Ժ���Ƶ���ʾ�ӳ�</param>
	/// <param name="ox">Xƫ��</param>
	/// <param name="oy">Yƫ��</param>
	/// <param name="cx">�¿�ȣ�0��ʾ����</param>
	/// <param name="cy">�¸߶ȣ�0��ʾ����</param>
	/// <param name="eDisposalMethod">����ʽ</param>
	/// <param name="eInterpolation">����ʱʹ�õĲ�ֵ��ʽ</param>
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
	/// ���Ƿ�͸����
	/// ��ΪTRUE�������а�����ֵС��50��Լ20%����������Ϊ͸��
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
	/// ����ΪGIF
	/// </summary>
	/// <param name="pStream">��</param>
	/// <returns>HRESULT</returns>
	HRESULT SaveAsGif(IStream* pStream)
	{
		HRESULT hr;
		PROPVARIANT Var{};
		ComPtr<IWICBitmapEncoder> pEncoder;
		g_pWicFactory->CreateEncoder(GUID_ContainerFormatGif, NULL, &pEncoder);
		if (FAILED(hr = pEncoder->Initialize(pStream, WICBitmapEncoderNoCache)))
			return hr;
		// дѭ������
		ComPtr<IWICMetadataQueryWriter> pMdWriter;
		if (FAILED(hr = pEncoder->GetMetadataQueryWriter(&pMdWriter)))
			return hr;
		BYTE byApp[]{ 'N','E','T','S','C','A','P','E','2','.','0' };// ��ANIMEXTS1.0
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
		// д���
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
		// д����ɫ
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
			// д֡�ӳ�
			Var.vt = VT_UI2;
			Var.uiVal = e.msDelay / 10;
			pMdWriter->SetMetadataByName(L"/grctlext/Delay", &Var);
			// дƫ��
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
			// д������־
			if (m_eDisposalMethod != Disposal::Undefined)
			{
				Var.vt = VT_UI1;
				Var.bVal = (BYTE)m_eDisposalMethod;
				pMdWriter->SetMetadataByName(L"/grctlext/Disposal", &Var);
			}
			// д͸����־
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