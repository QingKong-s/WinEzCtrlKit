#pragma once
#include "ComPtr.h"

#include <d3dcompiler.h>

#define ECK_EZDX_NAMESPACE_BEGIN	namespace EzDx {
#define ECK_EZDX_NAMESPACE_END		}

ECK_NAMESPACE_BEGIN
ECK_EZDX_NAMESPACE_BEGIN
struct CBuffer
{
	ComPtr<ID3D11Buffer> pBuffer;

	HRESULT Create(size_t cb, D3D11_BIND_FLAG eBind,
		D3D11_USAGE eUsage, UINT uCPUAccessFlags = 0u,
		UINT uMiscFlags = 0u, void* pData = nullptr, size_t cbStruct = 0u)
	{
		D3D11_BUFFER_DESC Desc;
		Desc.ByteWidth = (UINT)cb;
		Desc.Usage = eUsage;
		Desc.BindFlags = eBind;
		Desc.CPUAccessFlags = uCPUAccessFlags;
		Desc.MiscFlags = uMiscFlags;
		Desc.StructureByteStride = cbStruct;
		D3D11_SUBRESOURCE_DATA InitData;
		if (pData)
		{
			InitData.pSysMem = pData;
			InitData.SysMemPitch = 0u;
			InitData.SysMemSlicePitch = 0u;
		}
		return g_pD3d11Device->CreateBuffer(&Desc,
			pData ? &InitData : nullptr, &pBuffer);
	}

	EckInlineNdCe auto operator->() const noexcept { return pBuffer.Get(); }
	EckInlineNdCe auto Get() const noexcept { return pBuffer.Get(); }
};

struct VS_T {};
struct PS_T {};

template<class TType>
using TShaderInterface = std::conditional_t<std::is_same_v<TType, VS_T>, ID3D11VertexShader,
	std::conditional_t<std::is_same_v<TType, PS_T>, ID3D11PixelShader,
	void>>;

template<class TType>
EckInlineNdCe PCSTR GetShaderTarget() noexcept
{
	return std::is_same_v<TType, VS_T> ? "vs_5_0" :
		std::is_same_v<TType, PS_T> ? "ps_5_0" : "";
}

template<class TType>
struct CShader
{
	static_assert(!std::is_same_v<TShaderInterface<TType>, void>);
	ComPtr<TShaderInterface<TType>> pShader;

	HRESULT Create(PCCH psShader, int cchShader, PCSTR pszEntryPoint,
		UINT uFlags1 = 0u, UINT uFlags2 = 0u, ID3DBlob** ppShaderBlob = nullptr,
		ID3DBlob** ppErrorBlob = nullptr)
	{
		HRESULT hr;
		ComPtr<ID3DBlob> pErrorBlob, pBlob;
		hr = D3DCompile(psShader, cchShader, nullptr, nullptr, nullptr,
			pszEntryPoint, GetShaderTarget<TType>(), uFlags1, uFlags2,
			&pBlob, &pErrorBlob);
		if (FAILED(hr))
		{
			if (ppErrorBlob) *ppErrorBlob = pErrorBlob.Detach();
#ifdef _DEBUG
			EckDbgPrint(L"D3D11: Compile shader failed!");
			OutputDebugStringA((PCSTR)pErrorBlob->GetBufferPointer());
			EckDbgBreak();
#endif
			return hr;
		}
		if constexpr (std::is_same_v<TType, VS_T>)
			hr = g_pD3d11Device->CreateVertexShader(pBlob->GetBufferPointer(),
				pBlob->GetBufferSize(), nullptr, &pShader);
		else if constexpr (std::is_same_v<TType, PS_T>)
			hr = g_pD3d11Device->CreatePixelShader(pBlob->GetBufferPointer(),
				pBlob->GetBufferSize(), nullptr, &pShader);
		if (ppShaderBlob) *ppShaderBlob = pBlob.Detach();
		if (ppErrorBlob) *ppErrorBlob = nullptr;
		return hr;
	}

	EckInlineNdCe auto operator->() const noexcept { return pShader.Get(); }
	EckInlineNdCe auto Get() const noexcept { return pShader.Get(); }
};

struct CVSAndInputLayout : public CShader<VS_T>
{
	ComPtr<ID3D11InputLayout> pInputLayout;

	HRESULT Create(PCCH psShader, int cchShader, PCSTR pszEntryPoint,
		const D3D11_INPUT_ELEMENT_DESC* pInput, size_t cInput,
		UINT uFlags1 = 0u, UINT uFlags2 = 0u, ID3DBlob** ppShaderBlob = nullptr,
		ID3DBlob** ppErrorBlob = nullptr)
	{
		HRESULT hr;
		ComPtr<ID3DBlob> pBlob;
		hr = __super::Create(psShader, cchShader, pszEntryPoint,
			uFlags1, uFlags2, &pBlob, ppErrorBlob);
		if (FAILED(hr)) return hr;
		return g_pD3d11Device->CreateInputLayout(pInput, (UINT)cInput,
			pBlob->GetBufferPointer(), pBlob->GetBufferSize(), &pInputLayout);
	}

	EckInlineNdCe auto GetInputLayout() const noexcept { return pInputLayout.Get(); }
	EckInlineNdCe auto GetShader() const noexcept { return pShader.Get(); }
};


struct CTexture
{
	ComPtr<ID3D11Texture2D> pTexture;

	HRESULT Create(UINT cx, UINT cy, DXGI_FORMAT eFormat = DXGI_FORMAT_B8G8R8A8_UNORM,
		UINT eBind = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET,
		D3D11_USAGE eUsage = D3D11_USAGE_DEFAULT,
		UINT uCPUAccessFlags = 0u, UINT uMiscFlags = 0u,
		UINT uMipLevels = 1u,
		const D3D11_SUBRESOURCE_DATA* pInitData = nullptr)
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
		return g_pD3d11Device->CreateTexture2D(&Desc, pInitData, &pTexture);
	}

	EckInlineNdCe auto operator->() const noexcept { return pTexture.Get(); }
	EckInlineNdCe auto Get() const noexcept { return pTexture.Get(); }
};

struct CSampler
{
	ComPtr<ID3D11SamplerState> pSampler;

	HRESULT Create(D3D11_FILTER eFilter = D3D11_FILTER_MIN_MAG_MIP_LINEAR,
		D3D11_TEXTURE_ADDRESS_MODE eAddressU = D3D11_TEXTURE_ADDRESS_WRAP,
		D3D11_TEXTURE_ADDRESS_MODE eAddressV = D3D11_TEXTURE_ADDRESS_WRAP,
		D3D11_TEXTURE_ADDRESS_MODE eAddressW = D3D11_TEXTURE_ADDRESS_WRAP,
		D3D11_COMPARISON_FUNC eComparisonFunc = D3D11_COMPARISON_NEVER)
	{
		D3D11_SAMPLER_DESC Desc{};
		Desc.Filter = eFilter;
		Desc.AddressU = eAddressU;
		Desc.AddressV = eAddressV;
		Desc.AddressW = eAddressW;
		Desc.ComparisonFunc = eComparisonFunc;
		Desc.MaxAnisotropy = 1u;
		Desc.MinLOD = 0.f;
		Desc.MaxLOD = FLT_MAX;
		return g_pD3d11Device->CreateSamplerState(&Desc, &pSampler);
	}

	EckInlineNdCe auto operator->() const noexcept { return pSampler.Get(); }
	EckInlineNdCe auto Get() const noexcept { return pSampler.Get(); }
};

struct CShaderResourceView
{
	ComPtr<ID3D11ShaderResourceView> pSrv;

	HRESULT Create(ID3D11Resource* pResource, DXGI_FORMAT eFormat = DXGI_FORMAT_UNKNOWN,
		D3D11_SRV_DIMENSION eDimension = D3D11_SRV_DIMENSION_TEXTURE2D,
		UINT uMipLevels = 1u)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC Desc{};
		Desc.Format = eFormat;
		Desc.ViewDimension = eDimension;
		Desc.Texture2D.MipLevels = uMipLevels;
		return g_pD3d11Device->CreateShaderResourceView(pResource, &Desc, &pSrv);
	}

	EckInlineNdCe auto operator->() const noexcept { return pSrv.Get(); }
	EckInlineNdCe auto Get() const noexcept { return pSrv.Get(); }
};

struct CRenderTargetView
{
	ComPtr<ID3D11RenderTargetView> pRtv;

	HRESULT Create(ID3D11Resource* pResource)
	{
		return g_pD3d11Device->CreateRenderTargetView(pResource, nullptr, &pRtv);
	}

	EckInlineNdCe auto operator->() const noexcept { return pRtv.Get(); }
	EckInlineNdCe auto Get() const noexcept { return pRtv.Get(); }
};
ECK_EZDX_NAMESPACE_END
ECK_NAMESPACE_END