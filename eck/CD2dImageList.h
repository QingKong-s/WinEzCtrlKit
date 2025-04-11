#pragma once
#include "CSelRange.h"
#include "CUnknown.h"
#include "CSrwLock.h"
#include "Utility.h"

ECK_NAMESPACE_BEGIN
class CD2DImageList : public CRefObjMultiThread<CD2DImageList>
{
	ECK_DECL_CUNK_FRIENDS;
private:
	CSrwLock m_Lk{};
	ID2D1DeviceContext* m_pDC{};
	std::vector<ID2D1Bitmap*> m_vBmp{};
	CSelRange m_FreeRange{};
	D2D1_PIXEL_FORMAT m_PixelFormat{};

	int m_cx = 0;
	int m_cy = 0;
	int m_iPadding = 10;

	int m_cImg = 0;

	int m_cImgPerPack = 50;

	LONG m_cRef{ 1 };

	EckInline constexpr D2D1_SIZE_U GetPackSize() const
	{
		return { (UINT32)m_cx, (UINT32)((m_cy + m_iPadding) * m_cImgPerPack) };
	}

	HRESULT ReAlloc(int cImg)
	{
		if (cImg <= m_cImg)
			return S_FALSE;
		const int idxBegin = (int)m_vBmp.size();
		const int cPack = (cImg + m_cImgPerPack - 1) / m_cImgPerPack;
		m_vBmp.resize(cPack);
		for (int i = idxBegin; i < (int)m_vBmp.size(); ++i)
		{
			ID2D1Bitmap* pBmp;
			if (HRESULT hr; FAILED(hr = m_pDC->CreateBitmap(GetPackSize(), nullptr, 0,
				D2D1::BitmapProperties(m_PixelFormat), &pBmp))) ECKUNLIKELY
			{
				// 若失败，回滚到调用此方法前的状态
				for (int j = idxBegin; j < i; ++j)
					m_vBmp[j]->Release();
				m_vBmp.resize(idxBegin);
				return hr;
			}
			m_vBmp[i] = pBmp;
		}
		m_cImg = cImg;
		return S_OK;
	}

	EckInline constexpr int CalcPackIndex(int idxImg) const
	{
		return idxImg / m_cImgPerPack;
	}

	EckInline constexpr int CalcImgIndexInPack(int idxImg) const
	{
		return idxImg % m_cImgPerPack;
	}

	EckInline constexpr int CalcY(int idxImgInPack) const
	{
		return idxImgInPack * (m_cy + m_iPadding);
	}

	EckInline constexpr int CalcYFromImgIndex(int idxImg) const
	{
		return CalcImgIndexInPack(idxImg) * (m_cy + m_iPadding);
	}

	void DestroyNoLock()
	{
		for (auto e : m_vBmp)
			if (e) e->Release();
		m_vBmp.clear();
		SafeRelease(m_pDC);
		m_cImg = 0;
		m_FreeRange.Clear();
	}
public:
	ECK_DISABLE_COPY_MOVE_DEF_CONS(CD2DImageList);
	CD2DImageList(int cx, int cy, int iPadding = 1, int cImgPerPack = 50)
		: m_cx{ cx }, m_cy{ cy }, m_iPadding{ iPadding }, m_cImgPerPack{ cImgPerPack } {
	}

	/// <summary>
	/// 绑定渲染目标。
	/// 绑定与之兼容的渲染目标，修改绑定前将销毁之前的资源。
	/// 指定的渲染目标必须可被QI为ID2D1DeviceContext
	/// </summary>
	/// <param name="pRT">渲染目标</param>
	/// <returns>HRESULT</returns>
	HRESULT BindRenderTarget(ID2D1RenderTarget* pRT)
	{
		CSrwWriteGuard _{ m_Lk };
		DestroyNoLock();
		m_PixelFormat = pRT->GetPixelFormat();
		if (m_PixelFormat.format == DXGI_FORMAT_UNKNOWN)
			m_PixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
		m_PixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;
		return pRT->QueryInterface(&m_pDC);
	}

	// 销毁所有数据
	EckInline void Destroy()
	{
		CSrwWriteGuard _{ m_Lk };
		DestroyNoLock();
	}

	/// <summary>
	/// 加入图像。
	/// 将图像追加到图像列表的末尾。函数在内部调用CopyFromBitmap复制图像。
	/// 该方法不会执行缩放
	/// </summary>
	/// <param name="pBitmap">图像，像素格式（包括Alpha模式）必须兼容，这是CopyFromBitmap的基本要求</param>
	/// <param name="prcSrc">源矩形，若为nullptr则使用{ 0,0,单个图像宽度,单个图像高度 }</param>
	/// <param name="bUseFreeSpace">若为TRUE，则尝试使用空闲区域，否则直接追加</param>
	/// <param name="phr">可选的接收HRESULT的变量</param>
	/// <returns>新添加的图像的索引，若失败则返回-1</returns>
	int AddImage(ID2D1Bitmap* pBitmap, const D2D1_RECT_U* prcSrc = nullptr,
		BOOL bUseFreeSpace = TRUE, HRESULT* phr = nullptr)
	{
		HRESULT hr;
		if (bUseFreeSpace)
		{
			int idxNew;
			hr = ReplaceImage(-1, pBitmap, prcSrc, &idxNew);
			if (hr != E_NOT_SUFFICIENT_BUFFER)
			{
				if (phr)
					*phr = hr;
				return idxNew;
			}
		}
		EckAssert(pBitmap);
		CSrwWriteGuard _{ m_Lk };
		hr = ReAlloc(m_cImg + 1);
		if (FAILED(hr))
		{
			if (phr)
				*phr = hr;
			return -1;
		}

		const D2D1_POINT_2U ptDst{ 0,(UINT32)CalcY(CalcImgIndexInPack(m_cImg)) };
		D2D1_RECT_U rc;
		if (!prcSrc)
		{
			rc = { 0,0, (UINT32)m_cx, (UINT32)m_cy };
			prcSrc = &rc;
		}

		const auto pBmp = m_vBmp[CalcPackIndex(m_cImg)];
		hr = pBmp->CopyFromBitmap(&ptDst, pBitmap, prcSrc);
		if (phr)
			*phr = hr;

		if (SUCCEEDED(hr))
		{
			m_FreeRange.ExcludeRange(m_cImg - 1, m_cImg - 1);
			return m_cImg - 1;
		}
		else
			return -1;
	}

	/// <summary>
	/// 绘制图像。
	/// 绘制指定索引的图像到指定的矩形。函数使用ID2D1DeviceContext::DrawBitmap绘制图像。
	/// </summary>
	/// <param name="idxImg">图像索引</param>
	/// <param name="rcDst">目标矩形</param>
	/// <param name="fAlpha">透明度</param>
	/// <param name="iInterpolationMode">插值模式</param>
	/// <param name="pPerspectiveMatrix">绘制时应用的4x4矩阵变换</param>
	/// <returns>HRESULT</returns>
	HRESULT Draw(int idxImg, const D2D1_RECT_F& rcDst, float fAlpha = 1.f,
		D2D1_INTERPOLATION_MODE iInterpolationMode = D2D1_INTERPOLATION_MODE_LINEAR,
		const D2D1_MATRIX_4X4_F* pPerspectiveMatrix = nullptr)
	{
		EckAssert(idxImg >= 0 && idxImg < m_cImg);
		CSrwWriteGuard _{ m_Lk };
		const int y = CalcYFromImgIndex(idxImg);
		const D2D1_RECT_F rcSrc
		{
			0.f,
			(float)y,
			(float)m_cx,
			float(y + m_cy)
		};

		m_pDC->DrawBitmap(m_vBmp[CalcPackIndex(idxImg)], rcDst, fAlpha,
			iInterpolationMode, &rcSrc, pPerspectiveMatrix);
		return S_OK;
	}

	/// <summary>
	/// 绘制图像。
	/// 绘制指定索引的图像到指定的矩形。函数使用ID2D1RenderTarget::DrawBitmap绘制图像。
	/// </summary>
	/// <param name="idxImg">图像索引</param>
	/// <param name="rcDst">目标矩形</param>
	/// <param name="fAlpha">透明度</param>
	/// <param name="iInterpolationMode">插值模式</param>
	/// <returns>HRESULT</returns>
	HRESULT Draw2(int idxImg, const D2D1_RECT_F& rcDst, float fAlpha = 1.f,
		D2D1_BITMAP_INTERPOLATION_MODE iInterpolationMode = D2D1_BITMAP_INTERPOLATION_MODE_LINEAR)
	{
		EckAssert(idxImg >= 0 && idxImg < m_cImg);
		CSrwWriteGuard _{ m_Lk };
		const int y = CalcYFromImgIndex(idxImg);
		const D2D1_RECT_F rcSrc
		{
			0.f,
			(float)y,
			(float)m_cx,
			float(y + m_cy)
		};

		m_pDC->DrawBitmap(m_vBmp[CalcPackIndex(idxImg)], rcDst, fAlpha,
			iInterpolationMode, &rcSrc);
		return S_OK;
	}

	/// <summary>
	/// 创建图集效果。
	/// 使用指定索引的图像所在的位图创建一个图集（Atlas）效果。
	/// </summary>
	/// <param name="idxImg">图像索引</param>
	/// <param name="pEffectAtlas">效果</param>
	/// <returns>HRESULT</returns>
	HRESULT CreateAtlas(int idxImg, _Out_ ID2D1Effect*& pFxAtlas)
	{
		EckAssert(idxImg >= 0 && idxImg < m_cImg);
		CSrwWriteGuard _{ m_Lk };
		ID2D1Effect* pFx;
		if (HRESULT hr; FAILED(hr = m_pDC->CreateEffect(CLSID_D2D1Atlas, &pFx)))
		{
			pFxAtlas = nullptr;
			return hr;
		}

		pFx->SetInput(0, m_vBmp[CalcPackIndex(idxImg)]);
		const int y = CalcYFromImgIndex(idxImg);
		pFx->SetValue(D2D1_ATLAS_PROP_INPUT_RECT,
			D2D1::Vector4F(0.0f, (float)y, (float)m_cx, (float)(y + m_cy)));

		pFxAtlas = pFx;
		return S_OK;
	}

	/// <summary>
	/// 丢弃图像。
	/// 将指定索引的图像标记为空闲
	/// </summary>
	/// <param name="idxImg">索引</param>
	/// <returns>HRESULT</returns>
	HRESULT DiscardImage(int idxImg)
	{
		EckAssert(idxImg >= 0 && idxImg < m_cImg);
		CSrwWriteGuard _{ m_Lk };
		m_FreeRange.IncludeRange(idxImg, idxImg);
		return S_OK;
	}

	/// <summary>
	/// 替换图像。
	/// 替换指定索引的图像。函数在内部调用CopyFromBitmap复制图像。
	/// 该方法不会执行缩放
	/// </summary>
	/// <param name="idxImg">索引，若为-1则自动选择第一个空闲的索引，
	/// 在此种情况下若无空闲位置则返回E_NOT_SUFFICIENT_BUFFER</param>
	/// <param name="pBitmap">源图像</param>
	/// <param name="prcSrc">源矩形，若为nullptr则使用{ 0,0,单个图像宽度,单个图像高度 }</param>
	/// <returns>HRESULT</returns>
	HRESULT ReplaceImage(int idxImg, ID2D1Bitmap* pBitmap,
		const D2D1_RECT_U* prcSrc = nullptr, _Out_opt_ int* pidxNew = nullptr)
	{
		EckAssert(idxImg < m_cImg);
		EckAssert(pBitmap);
		CSrwWriteGuard _{ m_Lk };
		if (idxImg < 0)
		{
			idxImg = m_FreeRange.GetFirstSelected();
			if (pidxNew)
				*pidxNew = idxImg;
			if (idxImg < 0)
				return E_NOT_SUFFICIENT_BUFFER;
		}
		if (pidxNew)
			*pidxNew = idxImg;

		const D2D1_POINT_2U ptDst{ 0,(UINT32)CalcYFromImgIndex(idxImg) };
		D2D1_RECT_U rc;
		if (!prcSrc)
		{
			rc = { 0,0, (UINT32)m_cx, (UINT32)m_cy };
			prcSrc = &rc;
		}

		const auto pBmp = m_vBmp[CalcPackIndex(idxImg)];
		if (HRESULT hr; FAILED(hr = pBmp->CopyFromBitmap(&ptDst, pBitmap, prcSrc)))
			return hr;
		m_FreeRange.ExcludeRange(idxImg, idxImg);
		return S_OK;
	}

	// 取图像尺寸
	EckInline D2D1_SIZE_U GetImageSize()
	{
		CSrwReadGuard _{ m_Lk };
		return { (UINT32)m_cx, (UINT32)m_cy };
	}

	void GetImageSize(int& cx, int& cy)
	{
		CSrwReadGuard _{ m_Lk };
		cx = m_cx;
		cy = m_cy;
	}

	// 取图像数量
	EckInline int GetImageCount()
	{
		CSrwReadGuard _{ m_Lk };
		return m_cImg;
	}

	// 保留空间
	EckInline void Reserve(int cImg)
	{
		CSrwWriteGuard _{ m_Lk };
		ReAlloc(cImg);
	}
};
ECK_NAMESPACE_END