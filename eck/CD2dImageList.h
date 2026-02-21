#pragma once
#include "CSelectionRange.h"
#include "CUnknown.h"
#include "CSrwLock.h"
#include "Utility.h"

ECK_NAMESPACE_BEGIN
class CD2DImageList final : public CRefObj<CD2DImageList>
{
private:
    CSrwLock m_Lk{};
    ID2D1DeviceContext* m_pDC{};
    std::vector<ID2D1Bitmap1*> m_vBmp{};
    CSelectionRange m_FreeRange{};
    D2D1_PIXEL_FORMAT m_PixelFormat{};

    float m_cxLog{}, m_cyLog{};
    float m_fPaddingLog{};
    float m_fDpi{};

    int m_cImg{};
    int m_cCapacity{};
    int m_cCapacityPerPack{};

    EckInlineNd D2D1_SIZE_U PhyGetPackSize() const
    {
        return
        {
            (UINT32)ceilf(m_cxLog * m_fDpi / 96.f),
            (UINT32)ceilf(((m_cyLog + m_fPaddingLog) * m_cCapacityPerPack) * m_fDpi / 96.f)
        };
    }

    EckInlineNdCe int CalculatePackIndex(int idxImg) const
    {
        return idxImg / m_cCapacityPerPack;
    }

    EckInlineNdCe int CalculateImageIndexInPack(int idxImg) const
    {
        return idxImg % m_cCapacityPerPack;
    }

    EckInlineNd int PhyCalculateY(int idxImgInPack) const
    {
        return (int)ceilf(LogCalculateY(idxImgInPack) * m_fDpi / 96.f);
    }

    EckInlineNd int PhyYFromImgIndex(int idxImg) const
    {
        return (int)ceilf(LogYFromImageIndex(idxImg) * m_fDpi / 96.f);
    }

    EckInlineNd D2D1_RECT_U PhyGetSingleImageRect() const
    {
        return
        {
            0,0,
            (UINT32)ceilf(m_cxLog * m_fDpi / 96.f),
            (UINT32)ceilf(m_cyLog * m_fDpi / 96.f)
        };
    }

    EckInlineNdCe float LogCalculateY(int idxImgInPack) const
    {
        return idxImgInPack * (m_cyLog + m_fPaddingLog);
    }

    EckInlineNdCe float LogYFromImageIndex(int idxImg) const
    {
        return CalculateImageIndexInPack(idxImg) * (m_cyLog + m_fPaddingLog);
    }

    HRESULT ReAllocate(int cCapacity)
    {
        HRESULT hr;
        if (cCapacity <= m_cCapacity)
            return S_FALSE;
        const int idxBegin = (int)m_vBmp.size();
        const int cPack = (cCapacity + m_cCapacityPerPack) / m_cCapacityPerPack;
        EckAssert(cPack > (int)m_vBmp.size());
        m_vBmp.resize(cPack);
        const D2D1_BITMAP_PROPERTIES1 BmpProp
        {
            m_PixelFormat, m_fDpi, m_fDpi,
            D2D1_BITMAP_OPTIONS_TARGET,
        };

        const auto Size = PhyGetPackSize();
        for (int i = idxBegin; i < (int)m_vBmp.size(); ++i)
        {
            ID2D1Bitmap1* pBmp;
            if (FAILED(hr = m_pDC->CreateBitmap(Size, nullptr, 0, BmpProp, &pBmp)))
            {
                // 若失败，回滚到调用此方法前的状态
                for (int j = idxBegin; j < i; ++j)
                    m_vBmp[j]->Release();
                m_vBmp.resize(idxBegin);
                return hr;
            }
            m_vBmp[i] = pBmp;
        }

        m_FreeRange.IncludeRange(m_cCapacity, cPack * m_cCapacityPerPack - 1);
        m_cCapacity = cPack * m_cCapacityPerPack;
        return S_OK;
    }

    void DestroyNoLock()
    {
        for (auto e : m_vBmp)
            if (e) e->Release();
        m_vBmp.clear();
        SafeRelease(m_pDC);
        m_cCapacity = 0;
        m_FreeRange.Clear();
    }
public:
    ECK_DISABLE_COPY_MOVE_DEF_CONS(CD2DImageList);
    CD2DImageList(float cx, float cy, float iPadding = 1, int cCapacityPerPack = 50)
        : m_cxLog{ cx }, m_cyLog{ cy }, m_fPaddingLog{ iPadding },
        m_cCapacityPerPack{ cCapacityPerPack } {}

    ~CD2DImageList()
    {
        CSrwWriteGuard _{ m_Lk };
        DestroyNoLock();
    }

    HRESULT BindRenderTarget(_In_ ID2D1RenderTarget* pRT)
    {
        CSrwWriteGuard _{ m_Lk };
        DestroyNoLock();
        const auto hr = pRT->QueryInterface(&m_pDC);
        if (FAILED(hr)) return hr;

        m_PixelFormat = pRT->GetPixelFormat();
        if (m_PixelFormat.format == DXGI_FORMAT_UNKNOWN)
            m_PixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
        m_PixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;
        pRT->GetDpi(&m_fDpi, &m_fDpi);
        return S_OK;
    }

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
    /// <param name="pBitmap">图像，像素格式必须兼容</param>
    /// <param name="prcSrc">源矩形，物理坐标，若为nullptr则使用{ 0,0,单个图像宽度,单个图像高度 }</param>
    /// <param name="phr">可选的接收HRESULT的变量</param>
    /// <returns>新添加的图像的索引，若失败则返回-1</returns>
    int AddImage(_In_ ID2D1Bitmap* pBitmap, const D2D1_RECT_U* prcSrc = nullptr,
        _Out_opt_ HRESULT* phr = nullptr)
    {
        HRESULT hr;
        int idxNew;
        if (SUCCEEDED(hr = ReplaceImage(-1, pBitmap, prcSrc, &idxNew)))
        {
            if (phr) *phr = S_OK;
            return idxNew;
        }
        CSrwWriteGuard _{ m_Lk };
        if (FAILED(hr = ReAllocate(m_cImg + 1)))
        {
            if (phr) *phr = hr;
            return -1;
        }
        idxNew = m_cImg++;

        const D2D1_POINT_2U ptDst{ 0,(UINT32)PhyCalculateY(CalculateImageIndexInPack(idxNew)) };
        D2D1_RECT_U rc;
        if (!prcSrc)
        {
            rc = PhyGetSingleImageRect();
            prcSrc = &rc;
        }

        const auto pBmp = m_vBmp[CalculatePackIndex(idxNew)];
        hr = pBmp->CopyFromBitmap(&ptDst, pBitmap, prcSrc);
        if (phr) *phr = hr;
        if (SUCCEEDED(hr))
        {
            m_FreeRange.ExcludeItem(idxNew);
            return idxNew;
        }
        else
        {
            --m_cImg;
            return -1;
        }
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
        const auto y = LogYFromImageIndex(idxImg);
        const D2D1_RECT_F rcSrc{ 0.f,y,m_cxLog,y + m_cyLog };
        m_pDC->DrawBitmap(m_vBmp[CalculatePackIndex(idxImg)], rcDst, fAlpha,
            iInterpolationMode, &rcSrc, pPerspectiveMatrix);
        return S_OK;
    }

    // 绘制图像。
    // 绘制指定索引的图像到指定的矩形。函数使用ID2D1RenderTarget::DrawBitmap绘制图像。
    HRESULT Draw2(int idxImg, const D2D1_RECT_F& rcDst, float fAlpha = 1.f,
        D2D1_BITMAP_INTERPOLATION_MODE iInterpolationMode = D2D1_BITMAP_INTERPOLATION_MODE_LINEAR)
    {
        EckAssert(idxImg >= 0 && idxImg < m_cImg);
        CSrwWriteGuard _{ m_Lk };
        const auto y = LogYFromImageIndex(idxImg);
        const D2D1_RECT_F rcSrc{ 0.f,y,m_cxLog,y + m_cyLog };
        m_pDC->DrawBitmap(m_vBmp[CalculatePackIndex(idxImg)], rcDst, fAlpha,
            iInterpolationMode, &rcSrc);
        return S_OK;
    }

    // 丢弃图像。将指定索引的图像标记为空闲
    HRESULT DiscardImage(int idxImg)
    {
        EckAssert(idxImg >= 0 && idxImg < m_cCapacity);
        CSrwWriteGuard _{ m_Lk };
        if (m_FreeRange.IsSelected(idxImg))
            return S_FALSE;
        m_FreeRange.IncludeRange(idxImg, idxImg);
        --m_cImg;
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
    HRESULT ReplaceImage(int idxImg, _In_ ID2D1Bitmap* pBitmap,
        const D2D1_RECT_U* prcSrc = nullptr, _Out_opt_ int* pidxNew = nullptr)
    {
        EckAssert(idxImg < m_cImg);
        CSrwWriteGuard _{ m_Lk };
        BOOL bUseSpace{};
        if (idxImg < 0)
        {
            bUseSpace = TRUE;
            idxImg = m_FreeRange.GetFirstSelected();
            if (idxImg < 0)
            {
                if (pidxNew) *pidxNew = -1;
                return E_NOT_SUFFICIENT_BUFFER;
            }
        }
        if (pidxNew) *pidxNew = idxImg;

        const D2D1_POINT_2U ptDst{ 0,(UINT32)PhyYFromImgIndex(idxImg) };
        D2D1_RECT_U rc;
        if (!prcSrc)
        {
            rc = PhyGetSingleImageRect();
            prcSrc = &rc;
        }

        const auto pBmp = m_vBmp[CalculatePackIndex(idxImg)];
        if (HRESULT hr; FAILED(hr = pBmp->CopyFromBitmap(&ptDst, pBitmap, prcSrc)))
            return hr;
        m_FreeRange.ExcludeRange(idxImg, idxImg);
        if (bUseSpace)
            ++m_cImg;
        return S_OK;
    }

    EckInline D2D1_SIZE_F GetImageSize()
    {
        CSrwReadGuard _{ m_Lk };
        return { m_cxLog,m_cyLog };
    }

    EckInline void GetImageSize(float& cx, float& cy)
    {
        CSrwReadGuard _{ m_Lk };
        cx = m_cxLog;
        cy = m_cyLog;
    }

    EckInline int GetImageCount()
    {
        CSrwReadGuard _{ m_Lk };
        return m_cImg;
    }

    EckInline int GetCapacity()
    {
        CSrwReadGuard _{ m_Lk };
        return m_cCapacity;
    }

    EckInline void Reserve(int cImg)
    {
        CSrwWriteGuard _{ m_Lk };
        ReAllocate(cImg);
    }
};
ECK_NAMESPACE_END