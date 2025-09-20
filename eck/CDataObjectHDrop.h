#pragma once
#include "CUnknown.h"

ECK_NAMESPACE_BEGIN
class CDataObjectHDrop : public CUnknown<CDataObjectHDrop, IDataObject>
{
private:
	struct ITEM
	{
		FORMATETC fmt;
		STGMEDIUM stg;
	};
	std::vector<ITEM> m_vData{};

	EckInline auto FindFormat(FORMATETC* pFormatEtc)
	{
		return std::find_if(m_vData.begin(), m_vData.end(),
			[pFormatEtc](const ITEM& x)
			{
				return x.fmt.cfFormat == pFormatEtc->cfFormat &&
					x.fmt.tymed == pFormatEtc->tymed &&
					x.fmt.dwAspect == pFormatEtc->dwAspect;
			});
	}
public:
	static HRESULT CopyHGlobal(HGLOBAL hGlobalDst, HGLOBAL hGlobalSrc, SIZE_T cb = 0)
	{
		const auto pDst = GlobalLock(hGlobalDst);
		if (!pDst)
			return STG_E_MEDIUMFULL;
		const auto pSrc = GlobalLock(hGlobalSrc);
		if (!pSrc)
		{
			GlobalUnlock(hGlobalDst);
			return STG_E_MEDIUMFULL;
		}
		if (!cb)
			cb = std::min(GlobalSize(hGlobalSrc), GlobalSize(hGlobalDst));
		memcpy(pDst, pSrc, cb);
		GlobalUnlock(hGlobalSrc);
		GlobalUnlock(hGlobalDst);
		return S_OK;
	}

	static HRESULT DuplicateHGlobal(HGLOBAL hGlobalSrc, _Out_ HGLOBAL& hGlobalDst, UINT uFlags)
	{
		const auto cb = GlobalSize(hGlobalSrc);
		if (!cb)
		{
			hGlobalDst = nullptr;
			return E_UNEXPECTED;
		}
		hGlobalDst = GlobalAlloc(uFlags, cb);
		if (!hGlobalDst)
			return E_OUTOFMEMORY;
		const auto hr = CopyHGlobal(hGlobalDst, hGlobalSrc, cb);
		if (FAILED(hr))
		{
			GlobalFree(hGlobalDst);
			hGlobalDst = nullptr;
			return hr;
		}
		return S_OK;
	}
public:
	~CDataObjectHDrop()
	{
		for (auto& x : m_vData)
			ReleaseStgMedium(&x.stg);
	}

	HRESULT STDMETHODCALLTYPE GetData(
		_In_ FORMATETC* pFormatEtc, _Out_ STGMEDIUM* pStgMedium) override
	{
		*pStgMedium = {};
		if (pFormatEtc->lindex != -1)
			return DV_E_LINDEX;
		const auto it = FindFormat(pFormatEtc);
		if (it == m_vData.end())
			return DV_E_FORMATETC;
		HRESULT hr;
		switch (it->stg.tymed)
		{
		case TYMED_HGLOBAL:
		{
			if (!it->stg.hGlobal)
				return E_UNEXPECTED;
			if (FAILED(hr = DuplicateHGlobal(it->stg.hGlobal,
				pStgMedium->hGlobal, GMEM_MOVEABLE | GMEM_DDESHARE)))
				return hr;
		}
		break;
		default:
			return DV_E_TYMED;
		}
		pStgMedium->tymed = it->stg.tymed;
		if ((pStgMedium->pUnkForRelease = it->stg.pUnkForRelease))
			it->stg.pUnkForRelease->AddRef();
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE GetDataHere(
		_In_ FORMATETC* pFormatEtc, _Inout_ STGMEDIUM* pStgMedium) override
	{
		pStgMedium->pUnkForRelease = nullptr;
		if (pFormatEtc->lindex != -1)
			return DV_E_LINDEX;
		const auto it = FindFormat(pFormatEtc);
		if (it == m_vData.end())
			return DV_E_FORMATETC;
		if (it->stg.tymed != pStgMedium->tymed)
			return DV_E_TYMED;
		HRESULT hr;
		switch (it->stg.tymed)
		{
		case TYMED_HGLOBAL:
		{
			if (!it->stg.hGlobal)
				return E_UNEXPECTED;
			if (FAILED(hr = CopyHGlobal(pStgMedium->hGlobal, it->stg.hGlobal)))
				return hr;
		}
		break;
		default:
			return DV_E_TYMED;
		}
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE QueryGetData(_In_ FORMATETC* pFormatEtc) override
	{
		return (FindFormat(pFormatEtc) == m_vData.end()) ? DV_E_FORMATETC : S_OK;
	}

	HRESULT STDMETHODCALLTYPE GetCanonicalFormatEtc(
		_In_opt_ FORMATETC* pFormatEct, _Out_ FORMATETC* pFormatEtcOut) override
	{
		*pFormatEtcOut = {};
		return DATA_S_SAMEFORMATETC;
	}

	HRESULT STDMETHODCALLTYPE SetData(_In_ FORMATETC* pFormatEtc,
		_In_ STGMEDIUM* pStgMedium, BOOL fRelease) override
	{
		if (pFormatEtc->lindex != -1)
			return DV_E_LINDEX;
		if (pFormatEtc->tymed != pStgMedium->tymed)
			return DV_E_TYMED;
		HRESULT hr;
		switch (pStgMedium->tymed)
		{
		case TYMED_HGLOBAL:
		{
			if (!pStgMedium->hGlobal)
				return E_UNEXPECTED;
			auto& e = m_vData.emplace_back(*pFormatEtc, *pStgMedium);
			if (!fRelease)
			{
				if (FAILED(hr = DuplicateHGlobal(e.stg.hGlobal,
					e.stg.hGlobal, GMEM_MOVEABLE | GMEM_DDESHARE)))
				{
					m_vData.pop_back();
					return hr;
				}
			}
		}
		break;
		default:
			return DV_E_TYMED;
		}
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC** ppEnumFormatEtc)
	{
		*ppEnumFormatEtc = nullptr;
		return E_NOTIMPL;
	}

	HRESULT STDMETHODCALLTYPE DAdvise(FORMATETC* pFormatEtc, DWORD advf,
		IAdviseSink* pAdvSink, DWORD* pdwConnection)
	{
		*pdwConnection = 0;
		return E_NOTIMPL;
	}
	HRESULT STDMETHODCALLTYPE DUnadvise(DWORD dwConnection) { return E_NOTIMPL; }
	HRESULT STDMETHODCALLTYPE EnumDAdvise(IEnumSTATDATA** ppEnumAdvise)
	{
		*ppEnumAdvise = nullptr;
		return E_NOTIMPL;
	}
};
ECK_NAMESPACE_END