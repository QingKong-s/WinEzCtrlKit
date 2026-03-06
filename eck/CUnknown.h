#pragma once
#include "ECK.h"

ECK_NAMESPACE_BEGIN
#define ECK_DEF_COM_INHERIT(I, ...) \
    template<>                      \
    struct Inherits<I>              \
    {                               \
        using TBaseTuple = std::tuple<__VA_ARGS__>; \
    }

namespace UnknownTraits
{
    template<class I>
    struct Inherits
    {
        using TBaseTuple = std::tuple<>;
    };

    ECK_DEF_COM_INHERIT(IStream, ISequentialStream);
#if !ECK_OPT_NO_DWRITE
    ECK_DEF_COM_INHERIT(IDWriteTextRenderer, IDWritePixelSnapping);
#endif
}

template<class TThis, CcpComInterface... TInterface>
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
            using TInterface0 = std::tuple_element_t<0, std::tuple<TInterface...>>;
            *ppvObject = static_cast<TInterface0*>(this);
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
    EckInline HRESULT QiCheck(REFIID riid, void** ppvObject) noexcept
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
        EckInline HRESULT operator()(CUnknown* pThis, REFIID riid, void** ppvObject) noexcept
        {
            if constexpr (sizeof...(Is))
                return pThis->template QiCheck<Is...>(riid, ppvObject);
            else
                return E_NOINTERFACE;
        }
    };
};

// WARNING TBase必须虚析构
template<class TBase, CcpComInterface... TInterface>
class CUnknownAppend : public TBase, public TInterface...
{
public:
    STDMETHODIMP_(ULONG) AddRef() override { return TBase::AddRef(); }
    STDMETHODIMP_(ULONG) Release() override { return TBase::Release(); }
    STDMETHODIMP QueryInterface(REFIID riid, void** ppvObject) override
    {
        const auto hr = TBase::QueryInterface(riid, ppvObject);
        if (SUCCEEDED(hr))
            return hr;
        if (QiCheck<TInterface...>(riid, ppvObject) == S_OK)
            return S_OK;
        return E_NOINTERFACE;
    }
private:
    template<class I, class... Is>
    EckInline HRESULT QiCheck(REFIID riid, void** ppvObject) noexcept
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
        EckInline HRESULT operator()(CUnknownAppend* pThis, REFIID riid, void** ppvObject) noexcept
        {
            if constexpr (sizeof...(Is))
                return pThis->template QiCheck<Is...>(riid, ppvObject);
            else
                return E_NOINTERFACE;
        }
    };
};

template<class TThis>
using CRefObj = CUnknown<TThis, IUnknown>;
ECK_NAMESPACE_END