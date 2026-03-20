#pragma once
#include "CRectPackTile.h"
#include "ComPtr.h"

ECK_NAMESPACE_BEGIN
class CD2DImageList
{
public:
    constexpr static UINT InvalidId = UINT_MAX;

    using TCoord = CRectPackTile::TCoord;

    struct TEX
    {
        ComPtr<ID2D1Bitmap1> pTex;
    };
private:
    using TId = CRectPackTile::TId;

    CRectPackTile m_Packer;
    std::vector<TEX> m_vItem{};
    ComPtr<ID2D1DeviceContext> m_pDC{};
    DXGI_FORMAT m_eFormat{ DXGI_FORMAT_B8G8R8A8_UNORM };
    float m_fDpi{ 96.f };
    float m_cxTileLog{}, m_cyTileLog{};
    float m_cxPageLog{}, m_cyPageLog{};
public:
    CD2DImageList(
        float fDpi,
        float cxTile, float cyTile,
        float cxPage = 512, float cyPage = 256,
        DXGI_FORMAT eFormat = DXGI_FORMAT_B8G8R8A8_UNORM,
        BOOLEAN bIncPageSize = TRUE) noexcept :
        m_Packer{
            (TCoord)(ceilf(cxTile* fDpi / 96.f) + 1),
            (TCoord)(ceilf(cyTile* fDpi / 96.f) + 1),
            (TCoord)ceilf(cxPage),
            (TCoord)ceilf(cyPage),
            8192, 8192, bIncPageSize
        },
        m_eFormat{ eFormat },
        m_fDpi{ fDpi },
        m_cxTileLog{ cxTile }, m_cyTileLog{ cyTile },
        m_cxPageLog{ cxPage }, m_cyPageLog{ cyPage }
    {}

    HRESULT BindRenderTarget(ID2D1RenderTarget* pRT) noexcept
    {
        return pRT->QueryInterface(m_pDC.AddrOfClear());
    }

    HRESULT Add(_Out_ UINT& uId) noexcept
    {
        if (!m_Packer.HasFreeTile())
        {
            TCoord cxPage, cyPage;
            m_Packer.GetCurrentPageSize(cxPage, cyPage);

            const D2D1_BITMAP_PROPERTIES1 Prop
            {
                .pixelFormat = { m_eFormat, D2D1_ALPHA_MODE_PREMULTIPLIED },
                .dpiX = m_fDpi,
                .dpiY = m_fDpi,
            };
            auto& e = m_vItem.emplace_back();
            const auto hr = m_pDC->CreateBitmap(
                { cxPage, cyPage }, nullptr, 0, Prop, &e.pTex);
            if (FAILED(hr))
            {
                m_vItem.pop_back();
                uId = InvalidId;
                return hr;
            }
        }

        TId idxPage;
        TCoord x, y;
        uId = m_Packer.Allocate(idxPage, x, y);
        EckAssert(idxPage <= m_vItem.size());
        return S_OK;
    }

    void Discard(UINT uId) noexcept { m_Packer.Free(uId); }

    EckInlineNdCe D2D1_SIZE_U GetTilePixelSize() const noexcept
    {
        TCoord cxTile, cyTile;
        m_Packer.GetTileSize(cxTile, cyTile);
        return { cxTile - GetPadding(), cyTile - GetPadding() };
    }

    // WARNING 数据应在右方和下方包含空白边界，边界大小由GetPadding()返回
    HRESULT Upload(UINT uId, ID3D11DeviceContext* pContext,
        PCVOID pData, UINT cbStride) const noexcept
    {
        TId idxPage;
        TCoord x, y;
        m_Packer.GetTilePosition(uId, idxPage, x, y);
        const auto& e = m_vItem[idxPage];

        TCoord cxTile, cyTile;
        m_Packer.GetTileSize(cxTile, cyTile);

        const D2D1_RECT_U rc{ x, y, UINT(x + cxTile), UINT(y + cyTile) };
        return e.pTex->CopyFromMemory(&rc, pData, cbStride);
    }

    // 返回以像素计的空白边界大小，上传数据时应在右方和下方包含该边界
    EckInlineNdCe UINT GetPadding() const noexcept { return 1; }

    // 返回页索引，使用物理坐标
    constexpr UINT GetTileRectPixel(UINT uId, _Out_ D2D1_RECT_U& rc) const noexcept
    {
        TId idxPage;
        TCoord x, y;
        m_Packer.GetTilePosition(uId, idxPage, x, y);
        TCoord cxTile, cyTile;
        m_Packer.GetTileSize(cxTile, cyTile);
        rc.left = x;
        rc.top = y;
        rc.right = x + cxTile - 1;
        rc.bottom = y + cyTile - 1;
        return idxPage;
    }
    // 返回页索引，使用逻辑坐标
    constexpr UINT GetTileRectLogical(UINT uId, _Out_ D2D1_RECT_F& rc) const noexcept
    {
        D2D1_RECT_U rcPixel;
        const auto idxPage = GetTileRectPixel(uId, rcPixel);
        rc.left = rcPixel.left * 96.f / m_fDpi;
        rc.top = rcPixel.top * 96.f / m_fDpi;
        rc.right = rcPixel.right * 96.f / m_fDpi;
        rc.bottom = rcPixel.bottom * 96.f / m_fDpi;
        return idxPage;
    }

    const auto& GetPageTexture(UINT idxPage) const noexcept { return m_vItem[idxPage].pTex; }

    EckInlineCe void GetTileSize(_Out_ TCoord& cx, _Out_ TCoord& cy) const noexcept
    {
        m_Packer.GetTileSize(cx, cy);
        cx -= GetPadding();
        cy -= GetPadding();
    }

    EckInlineNdCe UINT GetPageCount() const noexcept { return m_Packer.GetPageCount(); }
    EckInlineNdCe UINT GetCount() const noexcept { return m_Packer.GetCount(); }

    void Draw(ID2D1DeviceContext* pDC, UINT uId, const D2D1_RECT_F& rcDst,
        float fOpcity = 1.f,
        D2D1_INTERPOLATION_MODE eInter = D2D1_INTERPOLATION_MODE_LINEAR) const noexcept
    {
        D2D1_RECT_F rcSrc;
        GetTileRectLogical(uId, rcSrc);
        pDC->DrawBitmap(GetPageTexture(uId).Get(),
            rcDst, fOpcity, eInter, &rcSrc);
    }
};
ECK_NAMESPACE_END