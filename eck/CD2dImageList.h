/*
* WinEzCtrlKit Library
*
* CD2dImageList.h ： D2D图像列表
*
* Copyright(C) 2023-2024 QingKong
*/
#pragma once
#include "Utility.h"
#include "CCriticalSection.h"

#include <set>

ECK_NAMESPACE_BEGIN
enum
{
	DILRF_ERASE = (1u << 0),// 删除图像，并调整后续图像的位置
	DILRF_STANDBY = (1u << 1),// 将此图像设为就绪状态，保留其位置
};

enum
{
	DILIF_NORMAL = 0,// 此图像处于正常状态
	DILIF_INVALID = (1u << 0),// 此图像处于就绪状态
};

class CD2dImageList
{
private:
	ID2D1DeviceContext* m_pDC = nullptr;
	std::vector<ID2D1Bitmap*> m_vBmp{};
	D2D1_PIXEL_FORMAT m_PixelFormat = D2D1::PixelFormat(DXGI_FORMAT_R8G8B8A8_UINT, D2D1_ALPHA_MODE_IGNORE);
	std::vector<UINT> m_vFlags{};
	std::set<int> m_hsStandby{};

	int m_cx = 0;
	int m_cy = 0;
	int m_iPadding = 10;

	int m_cImg = 0;

	int m_cImgPerPack = 50;

	CCriticalSection m_cs{};

	void ReAlloc(int cImg)
	{
		if (cImg <= m_cImg)
			return;
		m_vFlags.resize(cImg, DILIF_NORMAL);
		m_cImg = cImg;
		int idxBegin = (int)m_vBmp.size();
		int cPack = (cImg + m_cImgPerPack - 1) / m_cImgPerPack;
		m_vBmp.resize(cPack);
		const int idxEnd = (int)m_vBmp.size();
		auto pf = m_pDC->GetPixelFormat();
		pf.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;
		for (int i = idxBegin; i < idxEnd; i++)
		{
			ID2D1Bitmap* pBmp;
			if (FAILED(m_pDC->CreateBitmap(
				{ 
					(UINT32)m_cx, 
					(UINT32)((m_cy + m_iPadding) * m_cImgPerPack) 
				}, 
				nullptr, 
				0, 
				D2D1::BitmapProperties(pf), 
				&pBmp)))
			{

			}
			m_vBmp[i] = pBmp;
		}
	}
public:
	CD2dImageList() = default;
	CD2dImageList(int cx, int cy, int iPadding = 1, int cImgPerPack = 50)
		: m_cx(cx), m_cy(cy), m_iPadding(iPadding), m_cImgPerPack(cImgPerPack)
	{
	}

	EckInline int CalcPackIndex(int idxImg)
	{
		return idxImg / m_cImgPerPack;
	}

	EckInline int CalcImgIndexInPack(int idxImg)
	{
		return idxImg % m_cImgPerPack;
	}

	EckInline int CalcY(int idxImgInPack)
	{
		return idxImgInPack * (m_cy + m_iPadding);
	}

	EckInline int CalcYFromImgIndex(int idxImg)
	{
		return CalcImgIndexInPack(idxImg) * (m_cy + m_iPadding);
	}

	/// <summary>
	/// 绑定渲染目标。
	/// 绑定与之兼容的渲染目标，修改绑定前将销毁之前的资源。
	/// 指定的渲染目标必须可被QueryInterface为ID2D1DeviceContext。
	/// </summary>
	/// <param name="pRT">渲染目标，不会直接增加该接口的引用计数，而是对它调用QueryInterface</param>
	/// <returns>HRESULT</returns>
	HRESULT BindRenderTarget(ID2D1RenderTarget* pRT)
	{
		Destroy();
		HRESULT hr;

		D2D1_SIZE_U size{ (UINT32)m_cx, (UINT32)((m_cy + m_iPadding) * m_cImgPerPack) };
		ID2D1Bitmap* pBmp;
		auto pf = pRT->GetPixelFormat();
		if (pf.format == DXGI_FORMAT_UNKNOWN)
			pf.format = DXGI_FORMAT_B8G8R8A8_UNORM;
		pf.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;
		if (FAILED(hr = pRT->CreateBitmap(size, D2D1::BitmapProperties(pf), &pBmp)))
			return hr;

		if (FAILED(hr = pRT->QueryInterface(&m_pDC)))
			return hr;

		m_vBmp.emplace_back(pBmp);
		return S_OK;
	}

	/// <summary>
	/// 销毁所有数据
	/// </summary>
	void Destroy()
	{
		for (auto e : m_vBmp)
			if (e) e->Release();
		m_vBmp.clear();
		SafeRelease(m_pDC);
		m_cImg = 0;
	}

	/// <summary>
	/// 加入图像。
	/// 将图像追加到图像列表的末尾。函数在内部调用CopyFromBitmap复制图像。
	/// 该方法不会执行缩放。
	/// </summary>
	/// <param name="pBitmap">图像，像素格式（包括Alpha模式）必须兼容，这是CopyFromBitmap的基本要求</param>
	/// <param name="prcSrc">源矩形</param>
	/// <returns>新添加的图像的索引</returns>
	int AddImage(ID2D1Bitmap* pBitmap, const D2D1_RECT_U* prcSrc = nullptr, HRESULT* phr = nullptr)
	{
		CCsGuard _{ m_cs };
		EckAssert(pBitmap);
		HRESULT hr;
		const int idxPack = CalcPackIndex(m_cImg);
		const int idxImg = CalcImgIndexInPack(m_cImg);
		const int y = CalcY(idxImg);
		ReAlloc(m_cImg + 1);
		auto pBmp = m_vBmp[idxPack];
		const D2D1_POINT_2U ptDst{ 0,(UINT32)y };
		D2D1_RECT_U rc;
		if (!prcSrc)
		{
			rc = { 0,0, (UINT32)m_cx, (UINT32)m_cy };
			prcSrc = &rc;
		}
		int idx;
		if (FAILED(hr = pBmp->CopyFromBitmap(&ptDst, pBitmap, prcSrc)))
		{
			--m_cImg;
			idx = -1;
		}
		else
			idx = m_cImg - 1;
		if (phr)
			*phr = hr;
		return idx;
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
		D2D1_MATRIX_4X4_F* pPerspectiveMatrix = nullptr)
	{
		CCsGuard _{ m_cs };
		EckAssert(idxImg >= 0 && idxImg < m_cImg);
		const int idxPack = CalcPackIndex(idxImg);
		const int y = CalcYFromImgIndex(idxImg);
		const D2D1_RECT_F rcSrc
		{
			0.0f,
			(float)y,
			(float)m_cx,
			(float)(y + m_cy)
		};

		m_pDC->DrawBitmap(m_vBmp[idxPack], rcDst, 1.0f, 
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
		CCsGuard _{ m_cs };
		EckAssert(idxImg >= 0 && idxImg < m_cImg);
		const int idxPack = CalcPackIndex(idxImg);
		const int y = CalcYFromImgIndex(idxImg);
		const D2D1_RECT_F rcSrc
		{
			0.0f,
			(float)y,
			(float)m_cx,
			(float)(y + m_cy)
		};

		m_pDC->DrawBitmap(m_vBmp[idxPack], rcDst, 1.0f, iInterpolationMode, &rcSrc);
		return S_OK;
	}

	/// <summary>
	/// 创建图集效果。
	/// 使用指定索引的图像所在的位图创建一个图集（Atlas）效果。
	/// </summary>
	/// <param name="idxImg">图像索引</param>
	/// <param name="pEffectAtlas">效果</param>
	/// <returns>HRESULT</returns>
	HRESULT CreateAtlas(int idxImg, ID2D1Effect*& pEffectAtlas)
	{
		CCsGuard _{ m_cs };
		EckAssert(idxImg >= 0 && idxImg < m_cImg);
		pEffectAtlas = nullptr;
		HRESULT hr;
		const int idxPack = CalcPackIndex(idxImg);
		const int y = CalcYFromImgIndex(idxImg);

		ID2D1Effect* pFx;
		if (FAILED(hr = m_pDC->CreateEffect(CLSID_D2D1Atlas, &pFx)))
			return hr;

		pFx->SetInput(0, m_vBmp[idxPack]);
		pFx->SetValue(D2D1_ATLAS_PROP_INPUT_RECT, D2D1::Vector4F(0.0f, (float)y, (float)m_cx, (float)(y + m_cy)));

		pEffectAtlas = pFx;
		return S_OK;
	}

	HRESULT DiscardImage(int idxImg)
	{
		CCsGuard _{ m_cs };
		EckAssert(idxImg >= 0 && idxImg < m_cImg);
		if (m_hsStandby.find(idxImg) != m_hsStandby.end())
		{
			EckAssert((m_vFlags[idxImg] & DILIF_INVALID) == 0);
			m_vFlags[idxImg] = DILIF_INVALID;
			return S_OK;
		}
		else
		{
			EckAssert((m_vFlags[idxImg] & DILIF_INVALID) != 0);
			return S_FALSE;
		}
	}

	HRESULT ReplaceImage(int idxImg, ID2D1Bitmap* pBitmap, const D2D1_RECT_U* prcSrc = nullptr)
	{
		CCsGuard _{ m_cs };
		if (idxImg < 0)
		{
			if (!m_hsStandby.empty())
				idxImg = *m_hsStandby.begin();
			else
				return E_INVALIDARG;
		}
		EckAssert(idxImg < m_cImg);
		EckAssert(pBitmap);
		HRESULT hr;
		const int idxPack = CalcPackIndex(idxImg);
		const int y = CalcYFromImgIndex(idxImg);
		auto pBmp = m_vBmp[idxPack];
		const D2D1_POINT_2U ptDst{ 0,(UINT32)y };
		D2D1_RECT_U rc;
		if (!prcSrc)
		{
			rc = { 0,0, (UINT32)m_cx, (UINT32)m_cy };
			prcSrc = &rc;
		}
		if (FAILED(hr = pBmp->CopyFromBitmap(&ptDst, pBitmap, prcSrc)))
			return hr;
		if (m_vFlags[idxImg] & DILIF_INVALID)
		{
			m_vFlags[idxImg] = DILIF_NORMAL;
			m_hsStandby.erase(idxImg);
		}
		return S_OK;
	}

	void GetImageSize(int& cx, int& cy)
	{
		cx = m_cx;
		cy = m_cy;
	}
};
ECK_NAMESPACE_END