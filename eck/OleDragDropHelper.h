#pragma once
#include "ECK.h"

ECK_NAMESPACE_BEGIN
inline HRESULT CopyHGlobal(HGLOBAL hGlobalDst, HGLOBAL hGlobalSrc)
{
	void* pDst = GlobalLock(hGlobalDst);
	if (!pDst)
		return E_UNEXPECTED;

	void* pSrc = GlobalLock(hGlobalSrc);
	if (!pSrc)
	{
		GlobalUnlock(hGlobalDst);
		return E_UNEXPECTED;
	}

	memcpy(pDst, pSrc, std::min(GlobalSize(hGlobalSrc), GlobalSize(hGlobalDst)));
	GlobalUnlock(hGlobalSrc);
	GlobalUnlock(hGlobalDst);
	return S_OK;
}

inline HGLOBAL DuplicateHGlobal(HGLOBAL hGlobalSrc, HRESULT& hr, UINT uFlags)
{
	HGLOBAL hGlobalDst = GlobalAlloc(uFlags, GlobalSize(hGlobalSrc));
	if (!hGlobalDst)
	{
		hr = E_OUTOFMEMORY;
		return nullptr;
	}

	hr = CopyHGlobal(hGlobalDst, hGlobalSrc);
	if (FAILED(hr))
	{
		GlobalFree(hGlobalDst);
		return nullptr;
	}
	return hGlobalDst;
}

class CDropTarget : public IDropTarget
{
private:
	ULONG m_cRef = 1;
public:
	// IUnknown
	//HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppv);
	ULONG STDMETHODCALLTYPE AddRef() { return ++m_cRef; }

	ULONG STDMETHODCALLTYPE Release()
	{
		if (m_cRef == 1)
		{
			delete this;
			return 0;
		}
		return --m_cRef;
	}

	// IDropTarget
	//HRESULT STDMETHODCALLTYPE DragEnter(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect);
	//HRESULT STDMETHODCALLTYPE DragOver(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect);
	//HRESULT STDMETHODCALLTYPE DragLeave();
	//HRESULT STDMETHODCALLTYPE Drop(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect);
};

class CDropSource : public IDropSource
{
private:
	ULONG m_cRef = 1;
public:
	// IUnknown
	//HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void** ppvObject);
	ULONG STDMETHODCALLTYPE AddRef() { return ++m_cRef; }

	ULONG STDMETHODCALLTYPE Release()
	{
		if (m_cRef == 1)
		{
			delete this;
			return 0;
		}
		return --m_cRef;
	}

	// IDropSource
	//HRESULT STDMETHODCALLTYPE QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState);
	//HRESULT STDMETHODCALLTYPE GiveFeedback(DWORD dwEffect);
};

class CDataObject :public IDataObject
{
private:
	ULONG m_cRef = 1;
	std::vector<FORMATETC> m_vFormatEtc{};
	std::vector<STGMEDIUM> m_vStgMedium{};

	EckInline auto FindFormat(FORMATETC* pFormatEtc)
	{
		return std::find_if(m_vFormatEtc.begin(), m_vFormatEtc.end(), [pFormatEtc](const FORMATETC& x)->bool
			{
				return x.cfFormat == pFormatEtc->cfFormat &&
					x.tymed == pFormatEtc->tymed &&
					x.dwAspect == pFormatEtc->dwAspect;
			});
	}
public:
	~CDataObject()
	{
		for (auto& x : m_vStgMedium)
			ReleaseStgMedium(&x);
	}

	// IUnknown
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void** ppvObject)
	{
		const static QITAB qit[]
		{
			QITABENT(CDataObject, IDataObject),
			{}
		};

		return QISearch(this, qit, iid, ppvObject);
	}

	ULONG STDMETHODCALLTYPE AddRef() { return ++m_cRef; }

	ULONG STDMETHODCALLTYPE Release()
	{
		if (m_cRef == 1)
		{
			delete this;
			return 0;
		}
		return --m_cRef;
	}
	// IDataObject
	HRESULT STDMETHODCALLTYPE GetData(FORMATETC* pFormatEtc, STGMEDIUM* pStgMedium)
	{
		auto it = FindFormat(pFormatEtc);
		if (it == m_vFormatEtc.end())
			return DV_E_FORMATETC;
		auto itStg = m_vStgMedium.begin() + std::distance(m_vFormatEtc.begin(), it);

		switch (pFormatEtc->tymed)
		{
		case TYMED_HGLOBAL:
		{
			HGLOBAL hGlobalSrc = itStg->hGlobal;
			if (!hGlobalSrc)
				return E_UNEXPECTED;
			HRESULT hr;
			HGLOBAL hGlobal = DuplicateHGlobal(hGlobalSrc, hr, GMEM_MOVEABLE | GMEM_DDESHARE);
			if (hr != S_OK)
				return hr;
			*pStgMedium = *itStg;
			pStgMedium->hGlobal = hGlobal;
		}
		return S_OK;
		}
		return DV_E_FORMATETC;
	}

	HRESULT STDMETHODCALLTYPE GetDataHere(FORMATETC* pFormatEtc, STGMEDIUM* pStgMedium)
	{
		auto it = FindFormat(pFormatEtc);
		if (it == m_vFormatEtc.end())
			return DV_E_FORMATETC;
		auto itStg = m_vStgMedium.begin() + std::distance(m_vFormatEtc.begin(), it);
		HRESULT hr;

		switch (pFormatEtc->tymed)
		{
		case TYMED_HGLOBAL:
		{
			if (!itStg->hGlobal)
				return E_UNEXPECTED;
			hr = CopyHGlobal(pStgMedium->hGlobal, itStg->hGlobal);
		}
		return hr;
		}
		return DV_E_FORMATETC;
	}

	HRESULT STDMETHODCALLTYPE QueryGetData(FORMATETC* pFormatEtc)
	{
		return (FindFormat(pFormatEtc) == m_vFormatEtc.end()) ? DV_E_FORMATETC : S_OK;
	}

	HRESULT STDMETHODCALLTYPE GetCanonicalFormatEtc(FORMATETC* pFormatEct, FORMATETC* pFormatEtcOut)
	{
		*pFormatEtcOut = {};
		return DATA_S_SAMEFORMATETC;
	}

	HRESULT STDMETHODCALLTYPE SetData(FORMATETC* pFormatEtc, STGMEDIUM* pStgMedium, BOOL fRelease)
	{
		if (pStgMedium->tymed != TYMED_HGLOBAL)
			return DV_E_TYMED;
		if (!pStgMedium->hGlobal)
			return E_UNEXPECTED;

		HGLOBAL hGlobal;
		if (fRelease)
			hGlobal = pStgMedium->hGlobal;
		else
		{
			HRESULT hr;
			hGlobal = DuplicateHGlobal(pStgMedium->hGlobal, hr, GMEM_MOVEABLE | GMEM_DDESHARE);
			if (hr != S_OK)
				return hr;
		}

		const auto it = FindFormat(pFormatEtc);
		if (it == m_vFormatEtc.end())
		{
			m_vFormatEtc.emplace_back(*pFormatEtc);
			m_vStgMedium.emplace_back(*pStgMedium);
		}
		else
		{
			auto itStg = m_vStgMedium.begin() + std::distance(m_vFormatEtc.begin(), it);
			ReleaseStgMedium(&(*itStg));
			*itStg = *pStgMedium;
			itStg->hGlobal = hGlobal;
		}
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC** ppEnumFormatEtc)
	{
		return SHCreateStdEnumFmtEtc((UINT)m_vFormatEtc.size(), m_vFormatEtc.data(), ppEnumFormatEtc);
	}

	HRESULT STDMETHODCALLTYPE DAdvise(FORMATETC*, DWORD, IAdviseSink*, DWORD*)
	{
		return OLE_E_ADVISENOTSUPPORTED;
	}

	HRESULT STDMETHODCALLTYPE DUnadvise(DWORD)
	{
		return OLE_E_ADVISENOTSUPPORTED;
	}

	HRESULT STDMETHODCALLTYPE EnumDAdvise(IEnumSTATDATA** ppEnumAdvise)
	{
		*ppEnumAdvise = nullptr;
		return OLE_E_ADVISENOTSUPPORTED;
	}
};
ECK_NAMESPACE_END