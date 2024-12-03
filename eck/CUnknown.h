/*
* WinEzCtrlKit Library
*
* CUnknown.h ： IUnknown引用计数实现
*
* Copyright(C) 2024 QingKong
*/
#pragma once
#include "ECK.h"

ECK_NAMESPACE_BEGIN
/*
* 主类必须存在ULONG类型字段m_cRef
*/

template<class TThis, class TBase = IUnknown>
struct CUnknownSingleThread : public TBase
{
	STDMETHODIMP_(ULONG) AddRef() override
	{
		return ++static_cast<TThis*>(this)->m_cRef;
	}

	STDMETHODIMP_(ULONG) Release() override
	{
		const auto cRef = --static_cast<TThis*>(this)->m_cRef;
		if (cRef == 0)
			delete static_cast<TThis*>(this);
		return cRef;
	}
};

template<class TThis, class TBase = IUnknown>
struct CUnknownMultiThread : public TBase
{
	STDMETHODIMP_(ULONG) AddRef() override
	{
		return InterlockedIncrement(&static_cast<TThis*>(this)->m_cRef);
	}

	STDMETHODIMP_(ULONG) Release() override
	{
		const auto cRef = InterlockedDecrement(&static_cast<TThis*>(this)->m_cRef);
		if (cRef == 0)
			delete static_cast<TThis*>(this);
		return cRef;
	}
};

namespace Priv
{
	template<class TUnknown>
	struct CRefObj : public TUnknown
	{
		STDMETHODIMP QueryInterface(REFIID riid, void** ppvObject) override
		{
			if (riid == IID_IUnknown)
			{
				*ppvObject = static_cast<IUnknown*>(this);
				static_cast<IUnknown*>(this)->AddRef();
				return S_OK;
			}
			*ppvObject = nullptr;
			return E_NOINTERFACE;
		}
	};
}

template<class TThis>
using CRefObjSingleThread = Priv::CRefObj<CUnknownSingleThread<TThis>>;

template<class TThis>
using CRefObjMultiThread = Priv::CRefObj<CUnknownMultiThread<TThis>>;

#define DECL_CUNK_FRIENDS				\
	template<class TThis, class TBase>	\
	friend struct CUnknownSingleThread; \
	template<class TThis, class TBase>	\
	friend struct CUnknownMultiThread;
ECK_NAMESPACE_END