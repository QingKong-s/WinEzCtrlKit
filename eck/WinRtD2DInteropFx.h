#pragma once
#include "CRefStr.h"

#include <winrt/Windows.UI.Composition.Effects.h>
#include <windows.graphics.effects.interop.h>

// Win10
#include <d2d1effects_2.h>

// 实现IGraphicsEffectD2D1Interop::GetEffectId
#define ECK_WRDIFX_IMPL_CLSID(Clsid)		\
	STDMETHODIMP GetEffectId(CLSID* pclsid) \
	{										\
		if (pclsid)							\
		{									\
			*pclsid = Clsid;				\
			return S_OK;					\
		}									\
		return E_POINTER;					\
	}

using GePropMapping = ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING;

struct WRDI_FX_PROPERTY_MAPPING
{
	PCWSTR pszName;
	UINT idx;
	GePropMapping eMapping;
};

// 实现IGraphicsEffectD2D1Interop::GetNamedPropertyMapping
#define ECK_WRDIFX_IMPL_PROPMAPPING(MappingArray)	\
	STDMETHODIMP GetNamedPropertyMapping(PCWSTR pszName, UINT* pidx, GePropMapping* peMapping)	\
	{												\
		for (const auto& e : MappingArray)			\
		{											\
			if (_wcsicmp(pszName, e.pszName) == 0)	\
			{										\
				if (pidx)							\
					*pidx = e.idx;					\
				if (peMapping)						\
					*peMapping = e.eMapping;		\
				return S_OK;						\
			}										\
		}											\
		return E_FAIL;								\
	}

// 实现IGraphicsEffectD2D1Interop::GetEffectProperties为E_NOTIMPL
#define ECK_WRDIFX_IMPL_NO_PROPMAPPING()			\
	STDMETHODIMP GetNamedPropertyMapping(PCWSTR, UINT* pu, GePropMapping* pe)	\
	{												\
		if (pu) *pu = (UINT)-1;						\
		if (pe) *pe = GePropMapping::GRAPHICS_EFFECT_PROPERTY_MAPPING_UNKNOWN;	\
		return E_NOTIMPL;							\
	}

#define ECK_WRDIFX_IMPL(Clsid)						\
	ECK_WRDIFX_IMPL_CLSID(Clsid)					\
	ECK_WRDIFX_IMPL_NO_PROPMAPPING()

#define ECK_WRDIFX_IMPL2(Clsid, MappingArray)		\
	ECK_WRDIFX_IMPL_CLSID(Clsid)					\
	ECK_WRDIFX_IMPL_PROPMAPPING(MappingArray)

class CWinRtD2DInteropFx : public winrt::implements<CWinRtD2DInteropFx,
	ABI::Windows::Graphics::Effects::IGraphicsEffect,
	ABI::Windows::Graphics::Effects::IGraphicsEffectSource,
	ABI::Windows::Graphics::Effects::IGraphicsEffectD2D1Interop>
{
private:
	struct SOURCE
	{
		UINT idx;
		ABI::Windows::Graphics::Effects::IGraphicsEffectSource* pSource;

		constexpr std::strong_ordering operator<=>(const SOURCE& x) const noexcept
		{
			return idx <=> x.idx;
		}
	};
	struct PROPERTY
	{
		UINT idx;
		ABI::Windows::Foundation::IPropertyValue* pValue;

		constexpr std::strong_ordering operator<=>(const PROPERTY& x) const noexcept
		{
			return idx <=> x.idx;
		}
	};

	eck::CRefStrW m_rsName{};
	std::vector<SOURCE> m_vSource{};// 从小到大排序
	std::vector<PROPERTY> m_vProp{};// 从小到大排序
public:
	virtual ~CWinRtD2DInteropFx()
	{
		for (auto& e : m_vSource)
			e.pSource->Release();
		for (auto& e : m_vProp)
			e.pValue->Release();
	}
	// ***IGraphicsEffect***

	STDMETHODIMP get_Name(HSTRING* phstName) override
	{
		return WindowsCreateString(m_rsName.Data(), m_rsName.Size(), phstName);
	}

	STDMETHODIMP put_Name(HSTRING hstName) override
	{
		UINT32 cch;
		const auto pBuf = WindowsGetStringRawBuffer(hstName, &cch);
		m_rsName.DupString(pBuf, (int)cch);
		return S_OK;
	}

	// ***IGraphicsEffectD2D1Interop***

	STDMETHODIMP GetSourceCount(UINT* pc) override
	{
		if (pc)
		{
			*pc = (UINT)m_vSource.size();
			return S_OK;
		}
		return E_POINTER;
	}
	STDMETHODIMP GetSource(UINT idx,
		ABI::Windows::Graphics::Effects::IGraphicsEffectSource** ppSource)
	{
		if (!ppSource)
			return E_POINTER;
		const auto it = std::lower_bound(m_vSource.begin(),
			m_vSource.end(), SOURCE{ idx });
		if (it != m_vSource.end() && it->idx == idx)
		{
			*ppSource = it->pSource;
			(*ppSource)->AddRef();
			return S_OK;
		}
		return E_FAIL;
	}
	STDMETHODIMP GetPropertyCount(UINT* pc) override
	{
		if (pc)
		{
			*pc = (UINT)m_vProp.size();
			return S_OK;
		}
		return E_POINTER;
	}
	STDMETHODIMP GetProperty(UINT idx,
		ABI::Windows::Foundation::IPropertyValue** ppValue) override
	{
		if (!ppValue)
			return E_POINTER;
		const auto it = std::lower_bound(m_vProp.begin(),
			m_vProp.end(), PROPERTY{ idx });
		if (it != m_vProp.end() && it->idx == idx)
		{
			*ppValue = it->pValue;
			(*ppValue)->AddRef();
			return S_OK;
		}
		return E_FAIL;
	}

	EckInlineNdCe auto& NameString() noexcept { return m_rsName; }

	void SetInput(UINT idx,
		ABI::Windows::Graphics::Effects::IGraphicsEffectSource* pSource)
	{
		const auto it = std::lower_bound(m_vSource.begin(),
			m_vSource.end(), SOURCE{ idx });
		pSource->AddRef();
		if (it == m_vSource.end())
			m_vSource.emplace_back(idx, pSource);
		else if (it->idx != idx)
			m_vSource.insert(it, SOURCE{ idx, pSource });
		else
		{
			it->pSource->Release();
			it->pSource = pSource;
		}
	}
	void SetInput(UINT idx,
		const winrt::Windows::Graphics::Effects::IGraphicsEffectSource& Src)
	{
		SetInput(idx, Src.as<ABI::Windows::Graphics::Effects::IGraphicsEffectSource>().get());
	}
	void SetInput(UINT idx,
		const winrt::Windows::UI::Composition::CompositionEffectSourceParameter& Src)
	{
		SetInput(idx, Src.as<winrt::Windows::Graphics::Effects::IGraphicsEffectSource>());
	}
	void SetInput(UINT idx, const winrt::hstring& hstName)
	{
		SetInput(idx, winrt::Windows::UI::Composition::
			CompositionEffectSourceParameter{ hstName });
	}
	void SetInput(UINT idx, winrt::hstring&& hstName)
	{
		SetInput(idx, winrt::Windows::UI::Composition::
			CompositionEffectSourceParameter{ std::move(hstName) });
	}

	void SetValue(UINT idx,
		const winrt::Windows::Foundation::IPropertyValue& Value)
	{
		const auto it = std::lower_bound(m_vProp.begin(),
			m_vProp.end(), PROPERTY{ idx });
		const auto pValue = Value.as<ABI::Windows::Foundation::IPropertyValue>().detach();
		if (it == m_vProp.end())
			m_vProp.emplace_back(idx, pValue);
		else if (it->idx != idx)
			m_vProp.insert(it, PROPERTY{ idx, pValue });
		else
		{
			it->pValue->Release();
			it->pValue = pValue;
		}
	}
	template<class T>
	void SetValueT(UINT idx, const T& Value)
	{
		using PV = winrt::Windows::Foundation::PropertyValue;
		using IPV = winrt::Windows::Foundation::IPropertyValue;
		if constexpr (std::is_same_v<T, bool>)
			SetValue(idx, PV::CreateBoolean(Value).as<IPV>());
		else if constexpr (std::is_same_v<T, int32_t>)
			SetValue(idx, PV::CreateInt32(Value).as<IPV>());
		else if constexpr (std::is_same_v<T, uint32_t>)
			SetValue(idx, PV::CreateUInt32(Value).as<IPV>());
		else if constexpr (std::is_same_v<T, float>)
			SetValue(idx, PV::CreateSingle(Value).as<IPV>());
		else if constexpr (std::is_same_v<T, D2D1_COLOR_F> ||
			std::is_same_v<T, D2D1_VECTOR_4F> ||
			std::is_same_v<T, D2D1_MATRIX_5X4_F> ||
			std::is_same_v<T, D2D1::Matrix5x4F>)
		{
			SetValue(idx, PV::CreateSingleArray({
			(const float*)&Value,sizeof(Value) / sizeof(float) }).as<IPV>());
		}
		else if constexpr (std::is_enum_v<T>)// 枚举类型必须指定为UINT32
			SetValue(idx, PV::CreateUInt32((UINT32)Value).as<IPV>());
		else
			SetValue(idx, PV::CreateInt32(Value).as<IPV>());
	}
};

class CWrdiFxGaussianBlur : public CWinRtD2DInteropFx
{
private:
	constexpr static WRDI_FX_PROPERTY_MAPPING Mapping[]{ {
			L"Deviation",
			D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION,
			GePropMapping::GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT
		} };
public:
	ECK_WRDIFX_IMPL2(CLSID_D2D1GaussianBlur, Mapping)
};

class CWrdiFxBorder : public CWinRtD2DInteropFx
{
public:
	ECK_WRDIFX_IMPL(CLSID_D2D1Border)
};

class CWrdiFxOpacity : public CWinRtD2DInteropFx
{
private:
	constexpr static WRDI_FX_PROPERTY_MAPPING Mapping[]{ {
			L"Opacity",
			D2D1_OPACITY_PROP_OPACITY,
			GePropMapping::GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT
		} };
public:
	ECK_WRDIFX_IMPL2(CLSID_D2D1Opacity, Mapping)
};

class CWrdiFxBlend : public CWinRtD2DInteropFx
{
public:
	ECK_WRDIFX_IMPL(CLSID_D2D1Blend)
};

class CWrdiFxFlood : public CWinRtD2DInteropFx
{
private:
	constexpr static WRDI_FX_PROPERTY_MAPPING Mapping[]{ {
			L"Color",
			D2D1_FLOOD_PROP_COLOR,
			GePropMapping::GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT
		} };
public:
	ECK_WRDIFX_IMPL2(CLSID_D2D1Flood, Mapping)
};

class CWrdiFxSaturation : public CWinRtD2DInteropFx
{
private:
	constexpr static WRDI_FX_PROPERTY_MAPPING Mapping[]{ {
			L"Saturation",
			D2D1_SATURATION_PROP_SATURATION,
			GePropMapping::GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT
		} };
public:
	ECK_WRDIFX_IMPL2(CLSID_D2D1Saturation, Mapping)
};

class CWrdiFxComposite : public CWinRtD2DInteropFx
{
public:
	ECK_WRDIFX_IMPL(CLSID_D2D1Composite)
};

class CWrdiFxColorMatrix : public CWinRtD2DInteropFx
{
public:
	ECK_WRDIFX_IMPL(CLSID_D2D1ColorMatrix)
};

class CWrdiFxTint : public CWinRtD2DInteropFx
{
private:
	constexpr static WRDI_FX_PROPERTY_MAPPING Mapping[]{ {
			L"Color",
			D2D1_TINT_PROP_COLOR,
			GePropMapping::GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT
		} };
public:
	ECK_WRDIFX_IMPL2(CLSID_D2D1Tint, Mapping)
};