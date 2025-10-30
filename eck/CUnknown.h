#pragma once
#include "ECK.h"

ECK_NAMESPACE_BEGIN
#define ECK_DEF_COM_INHERIT(I, ...)	\
	template<>						\
	struct Inherits<I>				\
	{								\
		using TBaseTuple = std::tuple<__VA_ARGS__>;	\
	}

namespace UnknownTraits
{
	template<class I>
	struct Inherits
	{
		using TBaseTuple = std::tuple<>;
	};

	ECK_DEF_COM_INHERIT(IStream, ISequentialStream);
	ECK_DEF_COM_INHERIT(IDWriteTextRenderer, IDWritePixelSnapping);
}

template<class TThis, CcpIsComInterface... TInterface>
class CUnknown : public TInterface...
{
protected:
	ULONG m_cRef{ 1ul };
public:
	STDMETHODIMP_(ULONG) AddRef() override
	{
		return InterlockedIncrement(&m_cRef);
	}

	STDMETHODIMP_(ULONG) Release() override
	{
		const auto cRef = InterlockedDecrement(&m_cRef);
		if (cRef == 0)
			delete static_cast<TThis*>(this);
		return cRef;
	}

	STDMETHODIMP QueryInterface(REFIID riid, void** ppvObject) override
	{
		if (riid == IID_IUnknown)
		{
			*ppvObject = this;
			AddRef();
			return S_OK;
		}
		else if (QiCheck<TInterface...>(riid, ppvObject) == S_OK)
			return S_OK;
		else
		{
			*ppvObject = nullptr;
			return E_NOINTERFACE;
		}
	}
private:
	template<class I, class... Is>
	EckInline HRESULT QiCheck(REFIID riid, void** ppvObject)
	{
		if (riid == __uuidof(I))
		{
			*ppvObject = static_cast<I*>(this);
			AddRef();
			return S_OK;
		}
		using TTuple = UnknownTraits::Inherits<I>::TBaseTuple;
		if (QiCheckInherit<TTuple>{}(this, riid, ppvObject) == S_OK)
			return S_OK;

		if constexpr (sizeof...(Is))
			return QiCheck<Is...>(riid, ppvObject);
		else
			return E_NOINTERFACE;
	}

	template<class T>
	struct QiCheckInherit {};
	template<class... Is>
	struct QiCheckInherit<std::tuple<Is...>>
	{
		EckInline HRESULT operator()(CUnknown* pThis, REFIID riid, void** ppvObject)
		{
			if constexpr (sizeof...(Is))
				return pThis->QiCheck<Is...>(riid, ppvObject);
			else
				return E_NOINTERFACE;
		}
	};
};

template<class TThis>
using CRefObj = CUnknown<TThis, IUnknown>;
ECK_NAMESPACE_END