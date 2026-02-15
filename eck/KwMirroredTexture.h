#pragma once
#include "KwDef.h"
#include "ComPtr.h"
#include "CTrivialBuffer.h"

ECK_NAMESPACE_BEGIN
KW2D_NAMESPACE_BEGIN
// 表示一副CPU写GPU读的纹理，CPU保有权威副本，修改后上传至GPU
class CMirroredTexture
{
public:
    using TCoord = USHORT;
private:
    CTrivialBuffer<BYTE> m_Buffer{};
    ComPtr<ID3D11Texture2D> m_pTexture{};
    USHORT m_cx{}, m_cy{};
    BYTE m_cbPixel{ 4 };

    EckInlineNdCe UINT PixelCb() const noexcept { return m_cbPixel; }
public:
    // 先创建CPU缓冲区，然后才能创建GPU缓冲区

    void CpuCreate(BYTE cbPixel, BOOL bZero, TCoord cx, TCoord cy) noexcept
    {
        m_cbPixel = cbPixel;
        m_Buffer.ReSize(PixelCb() * cx * cy);
        if (bZero)
            CpuZero();
        m_cx = cx, m_cy = cy;
    }
    HRESULT GpuCreate(
        BOOL bUpload,
        DXGI_FORMAT eFormat = DXGI_FORMAT_B8G8R8A8_UNORM,
        UINT uBind = D3D11_BIND_SHADER_RESOURCE) noexcept
    {
        D3D11_TEXTURE2D_DESC Desc;
        Desc.Width = m_cx;
        Desc.Height = m_cy;
        Desc.MipLevels = 1;
        Desc.ArraySize = 1;
        Desc.Format = eFormat;
        Desc.SampleDesc.Count = 1u;
        Desc.SampleDesc.Quality = 0u;
        Desc.Usage = D3D11_USAGE_DEFAULT;
        Desc.BindFlags = uBind;
        Desc.CPUAccessFlags = 0;
        Desc.MiscFlags = 0;

        if (!bUpload)
            return g_pD3D11Device->CreateTexture2D(
                &Desc, nullptr, m_pTexture.AddrOfClear());

        const D3D11_SUBRESOURCE_DATA Data
        {
            .pSysMem = m_Buffer.Data(),
            .SysMemPitch = PixelCb() * m_cx,
            .SysMemSlicePitch = 0
        };
        return g_pD3D11Device->CreateTexture2D(
            &Desc, &Data, m_pTexture.AddrOfClear());
    }

    void CpuReSize(BOOL bZero, TCoord cx, TCoord cy) noexcept
    {
        m_Buffer.ReSize(PixelCb() * cx * cy);
        m_cx = cx, m_cy = cy;
        if (bZero)
            CpuZero();
    }
    // 重设大小，但在左上角保留原有数据
    void CpuReSizeReserve(BOOL bZeroExtended, TCoord cx, TCoord cy) noexcept
    {
        CTrivialBuffer<BYTE> NewBuffer{};
        const auto cbPixel = PixelCb();
        NewBuffer.ReSize(cbPixel * cx * cy);
        if (bZeroExtended)
            ZeroMemory(NewBuffer.Data(), NewBuffer.ByteSize());
        if (m_Buffer.Data())
        {
            const UINT xMin = std::min(m_cx, cx);
            const UINT yMin = std::min(m_cy, cy);
            for (UINT y = 0; y < yMin; ++y)
            {
                memcpy(NewBuffer.Data() + (y * cx) * cbPixel,
                    m_Buffer.Data() + (y * m_cx) * cbPixel,
                    cbPixel * xMin);
            }
        }

        m_Buffer = std::move(NewBuffer);
        m_cx = cx;
        m_cy = cy;
    }
    HRESULT GpuReSize(BOOL bUpload) noexcept
    {
        if (!m_pTexture.Get())
            return E_NOT_VALID_STATE;
        D3D11_TEXTURE2D_DESC Desc;
        m_pTexture->GetDesc(&Desc);
        if (Desc.Width == m_cx && Desc.Height == m_cy)
            return S_FALSE;
        Desc.Width = m_cx;
        Desc.Height = m_cy;
        if (!bUpload)
            return g_pD3D11Device->CreateTexture2D(
                &Desc, nullptr, m_pTexture.AddrOfClear());

        const D3D11_SUBRESOURCE_DATA Data
        {
            .pSysMem = m_Buffer.Data(),
            .SysMemPitch = PixelCb() * m_cx,
            .SysMemSlicePitch = 0
        };
        return g_pD3D11Device->CreateTexture2D(
            &Desc, &Data, m_pTexture.AddrOfClear());
    }

    template<class T>
    EckInlineNdCe T* CpuData(TCoord x, TCoord y) noexcept
    {
        EckAssert(x < m_cx && y < m_cy);
        return (T*)(m_Buffer.Data() + PixelCb() * (m_cx * y + x));
    }
    template<class T>
    EckInlineNdCe const T* CpuData(TCoord x, TCoord y) const noexcept
    {
        EckAssert(x < m_cx && y < m_cy);
        return (const T*)(m_Buffer.Data() + PixelCb() * (m_cx * y + x));
    }
    template<class T>
    EckInlineNdCe T* CpuData() noexcept { return (T*)m_Buffer.Data(); }
    template<class T>
    EckInlineNdCe const T* CpuData() const noexcept { return (const T*)m_Buffer.Data(); }

    EckInline void CpuZero() noexcept
    {
        ZeroMemory(m_Buffer.Data(), m_Buffer.ByteSize());
    }
    EckInline void CpuZero(const RectU& rc) noexcept
    {
        EckAssert(rc.right > rc.left);
        EckAssert(rc.bottom > rc.top);
        EckAssert(rc.right <= m_cx && rc.bottom <= m_cy);
        const auto cbStride = PixelCb() * (rc.right - rc.left);
        for (UINT y = rc.top; y < rc.bottom; ++y)
            ZeroMemory(CpuData<BYTE>(rc.left, y), cbStride);
    }

    void GpuUpload(ID3D11DeviceContext* pContext, const RectU& rcDirty) const noexcept
    {
        EckAssert(rcDirty.right > rcDirty.left);
        EckAssert(rcDirty.bottom > rcDirty.top);
        EckAssert(rcDirty.right <= m_cx && rcDirty.bottom <= m_cy);
        const D3D11_BOX Box
        {
            rcDirty.left,
            rcDirty.top,
            0,
            rcDirty.right,
            rcDirty.bottom,
            1
        };
        pContext->UpdateSubresource(m_pTexture.Get(), 0, &Box,
            CpuData<BYTE>(rcDirty.left, rcDirty.top), PixelCb() * m_cx, 0);
    }
    EckInline void GpuUpload(ID3D11DeviceContext* pContext) const noexcept
    {
        GpuUpload(pContext, { 0, 0, m_cx, m_cy });
    }

    EckInlineNdCe auto& CpuGetBuffer() const noexcept { return m_Buffer; }
    EckInlineNdCe auto GpuGetTexture() const noexcept { return m_pTexture.Get(); }

    EckInlineCe void CpuGetSize(_Out_ TCoord& cx, _Out_ TCoord& cy) const noexcept
    {
        cx = m_cx;
        cy = m_cy;
    }

    void GpuAttachTexture(ComPtr<ID3D11Texture2D>& pTex) noexcept
    {
        m_pTexture.Attach(pTex.Detach());
        if (m_pTexture.Get())
        {
            D3D11_TEXTURE2D_DESC Desc;
            m_pTexture->GetDesc(&Desc);
            m_cx = Desc.Width;
            m_cy = Desc.Height;
            m_Buffer.ReSize(PixelCb() * m_cx * m_cy);
        }
        else
        {
            m_cx = m_cy = 0;
            m_Buffer.Clear();
        }
    }

    void CpuBitBlit(
        CMirroredTexture& Dst,
        TCoord xDst, TCoord yDst,
        const RectU& rcSrc) const noexcept
    {
        EckAssert(xDst + (rcSrc.right - rcSrc.left) <= Dst.m_cx);
        EckAssert(yDst + (rcSrc.bottom - rcSrc.top) <= Dst.m_cy);
        EckAssert(rcSrc.right > rcSrc.left && rcSrc.bottom > rcSrc.top);
        EckAssert(rcSrc.right <= m_cx && rcSrc.bottom <= m_cy);
        for (TCoord y = 0; y < rcSrc.bottom - rcSrc.top; ++y)
        {
            memcpy(
                Dst.CpuData<BYTE>(xDst, yDst + y),
                CpuData<BYTE>(rcSrc.left, rcSrc.top + y),
                PixelCb() * (rcSrc.right - rcSrc.left));
        }
    }
};
KW2D_NAMESPACE_END
ECK_NAMESPACE_END