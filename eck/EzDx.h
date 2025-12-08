#pragma once
#include "ComPtr.h"

#include <d3dcompiler.h>

#define ECK_EZDX_NAMESPACE_BEGIN	namespace EzDx {
#define ECK_EZDX_NAMESPACE_END		}

ECK_NAMESPACE_BEGIN
ECK_EZDX_NAMESPACE_BEGIN
inline HRESULT CompileShader(
    _Out_ ID3DBlob*& pShaderBlob,
    _In_reads_(cchShader) PCCH pszShader,
    int cchShader,
    PCSTR pszEntryPoint,
    PCSTR pszTarget,
    UINT uFlags1 = 0u,
    UINT uFlags2 = 0u)
{
    ComPtr<ID3DBlob> pErrorBlob;
    const auto hr = D3DCompile(pszShader, cchShader, nullptr, nullptr, nullptr,
        pszEntryPoint, pszTarget, uFlags1, uFlags2, &pShaderBlob, &pErrorBlob);
    if (FAILED(hr))
    {
#ifdef _DEBUG
        EckDbgPrint(L"D3D11: Compile shader failed!");
        OutputDebugStringA((PCSTR)pErrorBlob->GetBufferPointer());
        EckDbgBreak();
#endif
    }
    return hr;
}


struct CBuffer
{
    ComPtr<ID3D11Buffer> pBuffer;

    HRESULT Create(
        size_t cb,
        D3D11_BIND_FLAG eBind,
        D3D11_USAGE eUsage,
        UINT uCPUAccessFlags = 0u,
        UINT uMiscFlags = 0u,
        PCVOID pData = nullptr,
        size_t cbStruct = 0u) noexcept
    {
        D3D11_BUFFER_DESC Desc;
        Desc.ByteWidth = (UINT)cb;
        Desc.Usage = eUsage;
        Desc.BindFlags = eBind;
        Desc.CPUAccessFlags = uCPUAccessFlags;
        Desc.MiscFlags = uMiscFlags;
        Desc.StructureByteStride = (UINT)cbStruct;
        D3D11_SUBRESOURCE_DATA InitData;
        if (pData)
        {
            InitData.pSysMem = pData;
            InitData.SysMemPitch = 0u;
            InitData.SysMemSlicePitch = 0u;
        }
        return g_pD3d11Device->CreateBuffer(
            &Desc,
            pData ? &InitData : nullptr,
            pBuffer.AddrOfClear());
    }
    HRESULT CreateConstantBuffer(size_t cb, PCVOID pData = nullptr) noexcept
    {
        return Create(cb, D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC,
            D3D11_CPU_ACCESS_WRITE, 0u, pData);
    }
    HRESULT CreateVertexBuffer(BOOL bImmutable, size_t cb, PCVOID pData = nullptr) noexcept
    {
        return Create(cb, D3D11_BIND_VERTEX_BUFFER,
            bImmutable ? D3D11_USAGE_IMMUTABLE : D3D11_USAGE_DYNAMIC,
            bImmutable ? 0 : D3D11_CPU_ACCESS_WRITE,
            0u, pData);
    }
    HRESULT CreateIndexBuffer(BOOL bImmutable, size_t cb, PCVOID pData = nullptr) noexcept
    {
        return Create(cb, D3D11_BIND_INDEX_BUFFER,
            bImmutable ? D3D11_USAGE_IMMUTABLE : D3D11_USAGE_DYNAMIC,
            bImmutable ? 0 : D3D11_CPU_ACCESS_WRITE,
            0u, pData);
    }

    EckInlineNdCe auto operator->() const noexcept { return pBuffer.Get(); }
    EckInlineNdCe auto Get() const noexcept { return pBuffer.Get(); }
    EckInlineNdCe auto AddrOf() const noexcept { return pBuffer.AddrOf(); }
};

namespace Priv
{
    template<class TClass, class TInterface>
    struct CShader
    {
        ComPtr<TInterface> pShader;

        EckInlineNdCe auto operator->() const noexcept { return pShader.Get(); }
        EckInlineNdCe auto Get() const noexcept { return pShader.Get(); }
        EckInlineNdCe auto AddrOf() const noexcept { return pShader.AddrOf(); }

        EckInline HRESULT Create(ID3DBlob* pBlob) noexcept
        {
            return static_cast<TClass*>(this)->Create(
                pBlob->GetBufferPointer(), pBlob->GetBufferSize());
        }
    };
}
struct CPixelShader : Priv::CShader<CPixelShader, ID3D11PixelShader>
{
    EckInline HRESULT Create(
        _In_reads_bytes_(cbBytecode) PCVOID pBytecode,
        size_t cbBytecode) noexcept
    {
        return g_pD3d11Device->CreatePixelShader(pBytecode,
            cbBytecode, nullptr, pShader.AddrOfClear());
    }
};
struct CVertexShader : Priv::CShader<CVertexShader, ID3D11VertexShader>
{
    EckInline HRESULT Create(
        _In_reads_bytes_(cbBytecode) PCVOID pBytecode,
        size_t cbBytecode) noexcept
    {
        return g_pD3d11Device->CreateVertexShader(pBytecode,
            cbBytecode, nullptr, pShader.AddrOfClear());
    }
};
struct CVSAndInputLayout
{
    CVertexShader Shader;
    ComPtr<ID3D11InputLayout> pInputLayout;

    HRESULT Create(
        _In_reads_bytes_(cbBytecode) PCVOID pBytecode,
        size_t cbBytecode,
        _In_reads_(cInput) const D3D11_INPUT_ELEMENT_DESC* pInput,
        size_t cInput) noexcept
    {
        HRESULT hr;
        hr = Shader.Create(pBytecode, cbBytecode);
        if (FAILED(hr)) return hr;
        hr = g_pD3d11Device->CreateInputLayout(pInput, (UINT)cInput,
            pBytecode, cbBytecode, pInputLayout.AddrOfClear());
        return hr;
    }

    EckInlineNdCe auto GetInputLayout() const noexcept { return pInputLayout.Get(); }
    EckInlineNdCe auto GetShader() const noexcept { return Shader.Get(); }
};

struct CTexture
{
    ComPtr<ID3D11Texture2D> pTexture;

    HRESULT Create(
        UINT cx, UINT cy,
        DXGI_FORMAT eFormat = DXGI_FORMAT_B8G8R8A8_UNORM,
        UINT eBind = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET,
        D3D11_USAGE eUsage = D3D11_USAGE_DEFAULT,
        UINT uCPUAccessFlags = 0u,
        UINT uMiscFlags = 0u,
        UINT uMipLevels = 1u,
        const D3D11_SUBRESOURCE_DATA* pInitData = nullptr) noexcept
    {
        D3D11_TEXTURE2D_DESC Desc;
        Desc.Width = cx;
        Desc.Height = cy;
        Desc.MipLevels = 1u;
        Desc.ArraySize = 1u;
        Desc.Format = eFormat;
        Desc.SampleDesc.Count = 1u;
        Desc.SampleDesc.Quality = 0u;
        Desc.Usage = eUsage;
        Desc.BindFlags = eBind;
        Desc.CPUAccessFlags = uCPUAccessFlags;
        Desc.MiscFlags = uMiscFlags;
        return g_pD3d11Device->CreateTexture2D(&Desc, pInitData, pTexture.AddrOfClear());
    }

    EckInlineNdCe auto operator->() const noexcept { return pTexture.Get(); }
    EckInlineNdCe auto Get() const noexcept { return pTexture.Get(); }
    EckInlineNdCe auto AddrOf() const noexcept { return pTexture.AddrOf(); }
};

struct CSampler
{
    ComPtr<ID3D11SamplerState> pSampler;

    HRESULT Create(D3D11_FILTER eFilter = D3D11_FILTER_MIN_MAG_MIP_LINEAR,
        D3D11_TEXTURE_ADDRESS_MODE eAddressU = D3D11_TEXTURE_ADDRESS_WRAP,
        D3D11_TEXTURE_ADDRESS_MODE eAddressV = D3D11_TEXTURE_ADDRESS_WRAP,
        D3D11_TEXTURE_ADDRESS_MODE eAddressW = D3D11_TEXTURE_ADDRESS_WRAP,
        D3D11_COMPARISON_FUNC eComparisonFunc = D3D11_COMPARISON_NEVER,
        _In_reads_opt_(4) const float* pBorderColor = nullptr) noexcept
    {
        D3D11_SAMPLER_DESC Desc{};
        Desc.Filter = eFilter;
        Desc.AddressU = eAddressU;
        Desc.AddressV = eAddressV;
        Desc.AddressW = eAddressW;
        Desc.ComparisonFunc = eComparisonFunc;
        Desc.MaxAnisotropy = 1u;
        Desc.MinLOD = -FLT_MAX;
        Desc.MaxLOD = FLT_MAX;
        if (pBorderColor)
            memcpy(Desc.BorderColor, pBorderColor, 4 * sizeof(float));
        return g_pD3d11Device->CreateSamplerState(&Desc, pSampler.AddrOfClear());
    }

    EckInlineNdCe auto operator->() const noexcept { return pSampler.Get(); }
    EckInlineNdCe auto Get() const noexcept { return pSampler.Get(); }
    EckInlineNdCe auto AddrOf() const noexcept { return pSampler.AddrOf(); }
};

struct CShaderResourceView
{
    ComPtr<ID3D11ShaderResourceView> pSrv;

    HRESULT Create(
        ID3D11Resource* pResource,
        DXGI_FORMAT eFormat = DXGI_FORMAT_UNKNOWN,
        D3D11_SRV_DIMENSION eDimension = D3D11_SRV_DIMENSION_TEXTURE2D,
        UINT uMipLevels = 1u) noexcept
    {
        D3D11_SHADER_RESOURCE_VIEW_DESC Desc{};
        Desc.Format = eFormat;
        Desc.ViewDimension = eDimension;
        Desc.Texture2D.MipLevels = uMipLevels;
        return g_pD3d11Device->CreateShaderResourceView(pResource, &Desc, pSrv.AddrOfClear());
    }

    EckInlineNdCe auto operator->() const noexcept { return pSrv.Get(); }
    EckInlineNdCe auto Get() const noexcept { return pSrv.Get(); }
    EckInlineNdCe auto AddrOf() const noexcept { return pSrv.AddrOf(); }
};

struct CRenderTargetView
{
    ComPtr<ID3D11RenderTargetView> pRtv;

    HRESULT Create(ID3D11Resource* pResource) noexcept
    {
        return g_pD3d11Device->CreateRenderTargetView(pResource, nullptr, pRtv.AddrOfClear());
    }

    EckInlineNdCe auto operator->() const noexcept { return pRtv.Get(); }
    EckInlineNdCe auto Get() const noexcept { return pRtv.Get(); }
    EckInlineNdCe auto AddrOf() const noexcept { return pRtv.AddrOf(); }
};

struct CDepthStencilView
{
    ComPtr<ID3D11DepthStencilView> pDsv;

    HRESULT Create(ID3D11Resource* pResource,
        const D3D11_DEPTH_STENCIL_VIEW_DESC* pDesc = nullptr) noexcept
    {
        return g_pD3d11Device->CreateDepthStencilView(pResource, nullptr, pDsv.AddrOfClear());
    }

    EckInlineNdCe auto operator->() const noexcept { return pDsv.Get(); }
    EckInlineNdCe auto Get() const noexcept { return pDsv.Get(); }
    EckInlineNdCe auto AddrOf() const noexcept { return pDsv.AddrOf(); }
};
ECK_EZDX_NAMESPACE_END
ECK_NAMESPACE_END