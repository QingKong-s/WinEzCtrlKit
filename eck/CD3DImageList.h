#pragma once
#include "CRectPackTile.h"
#include "ComPtr.h"

ECK_NAMESPACE_BEGIN
class CD3DImageList
{
public:
    constexpr static UINT InvalidId = UINT_MAX;

    using TCoord = CRectPackTile::TCoord;

    struct TEX
    {
        ComPtr<ID3D11Texture2D> pTex;
        ComPtr<ID3D11ShaderResourceView> pSrv;
    };
private:
    using TId = CRectPackTile::TId;

    CRectPackTile m_Packer;
    std::vector<TEX> m_vItem{};
    DXGI_FORMAT m_eFormat{};
public:
    CD3DImageList(
        TCoord cxTile, TCoord cyTile,
        TCoord cxPage = 512, TCoord cyPage = 256,
        DXGI_FORMAT eFormat = DXGI_FORMAT_B8G8R8A8_UNORM,
        BOOLEAN bIncPageSize = TRUE) noexcept :
        m_Packer{ TCoord(cxTile + 1), TCoord(cyTile + 1), cxPage, cyPage, 8192, 8192, bIncPageSize },
        m_eFormat{ eFormat }
    {}

    HRESULT Add(_Out_ UINT& uId) noexcept
    {
        if (!m_Packer.HasFreeTile())
        {
            TCoord cxPage, cyPage;
            m_Packer.GetCurrentPageSize(cxPage, cyPage);
            const D3D11_TEXTURE2D_DESC Desc
            {
                .Width = cxPage,
                .Height = cyPage,
                .MipLevels = 1,
                .ArraySize = 1,
                .Format = m_eFormat,
                .SampleDesc = { 1, 0 },
                .Usage = D3D11_USAGE_DEFAULT,
                .BindFlags = D3D11_BIND_SHADER_RESOURCE,
                .CPUAccessFlags = 0,
                .MiscFlags = 0
            };
            auto& e = m_vItem.emplace_back();
            HRESULT hr;
            hr = g_pD3D11Device->CreateTexture2D(&Desc, nullptr, &e.pTex);
            if (FAILED(hr))
            {
                m_vItem.pop_back();
                uId = InvalidId;
                return hr;
            }
            hr = g_pD3D11Device->CreateShaderResourceView(
                e.pTex.Get(), nullptr, &e.pSrv);
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

    // WARNING 数据应在右方和下方包含空白边界，边界大小由GetPadding()返回
    void Upload(UINT uId, ID3D11DeviceContext* pContext,
        PCVOID pData, UINT cbStride) const noexcept
    {
        TId idxPage;
        TCoord x, y;
        m_Packer.GetTilePosition(uId, idxPage, x, y);
        const auto& e = m_vItem[idxPage];

        TCoord cxTile, cyTile;
        m_Packer.GetTileSize(cxTile, cyTile);
        const D3D11_BOX Box
        {
            .left = x,
            .top = y,
            .front = 0,
            .right = UINT(x + cxTile),
            .bottom = UINT(y + cyTile),
            .back = 1
        };
        pContext->UpdateSubresource(e.pTex.Get(), 0, &Box, pData, cbStride, 0);
    }

    // 返回以像素计的空白边界大小，上传数据时应在右方和下方包含该边界
    EckInlineNdCe UINT GetPadding() const noexcept { return 1; }

    // 返回页索引
    UINT CalculateUv(
        UINT uId,
        _Out_ float& u0, _Out_ float& v0,
        _Out_ float& u1, _Out_ float& v1) const noexcept
    {
        TId idxPage;
        TCoord x, y;
        m_Packer.GetTilePosition(uId, idxPage, x, y);
        TCoord cxPage, cyPage;
        m_Packer.GetPageSize(idxPage, cxPage, cyPage);
        TCoord cxTile, cyTile;
        m_Packer.GetTileSize(cxTile, cyTile);
        u0 = (float)x / cxPage;
        v0 = (float)y / cyPage;
        u1 = float(x + cxTile - 1) / cxPage;
        v1 = float(y + cyTile - 1) / cyPage;
        return idxPage;
    }

    void GetPageTexture(UINT idxPage, TEX& Tex) const noexcept { Tex = m_vItem[idxPage]; }

    EckInlineCe void GetTileSize(_Out_ TCoord& cx, _Out_ TCoord& cy) const noexcept
    {
        m_Packer.GetTileSize(cx, cy);
        cx -= GetPadding();
        cy -= GetPadding();
    }

    EckInlineNdCe UINT GetPageCount() const noexcept { return m_Packer.GetPageCount(); }
    EckInlineNdCe UINT GetCount() const noexcept { return m_Packer.GetCount(); }
};
ECK_NAMESPACE_END