/*
* WinEzCtrlKit Library
*
* CStreamView.h ： 流视图
* 只读IStream接口的实现，在一些情况下可以避免内存复制
*
* Copyright(C) 2023 QingKong
*/
#pragma once
#include "CRefBin.h"

#include <Shlwapi.h>

ECK_NAMESPACE_BEGIN
class CStreamView :public IStream
{
private:
	ULONG m_cRef = 1;

	PCBYTE m_pMem = NULL;
	PCBYTE m_pSeek = NULL;
	SIZE_T m_cbSize = 0u;
public:
	CStreamView() = default;
	CStreamView(PCVOID p, SIZE_T cb)
		:m_pMem{ (PCBYTE)p }, m_cbSize{ cb }, m_pSeek{ (PCBYTE)p } {}

	template<class TAlloc>
	CStreamView(const CRefBinT<TAlloc>& rb)
		: m_pMem{ rb.Data() }, m_cbSize{ rb.Size() }, m_pSeek{ rb.Data() } {}

	template<class T, class TAlloc>
	CStreamView(const std::vector<T, TAlloc>& v)
		: m_pMem{ (PCBYTE)v.data() }, m_cbSize{ v.size() * sizeof(T) }, m_pSeek{ (PCBYTE)v.data() } {}

	void SetData(PCVOID p, SIZE_T cb)
	{
		m_pMem = (PCBYTE)p;
		m_pSeek = m_pMem;
		m_cbSize = cb;
	}

	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject)
	{
		const QITAB qit[]
		{
			QITABENT(CStreamView, IStream),
			QITABENT(CStreamView, ISequentialStream),
			{}
		};

		return QISearch(this, qit, riid, ppvObject);
	}

	ULONG STDMETHODCALLTYPE AddRef(void)
	{
		EckAssert(m_cRef);
		return ++m_cRef;
	}

	ULONG STDMETHODCALLTYPE Release(void)
	{
		EckAssert(m_cRef);
		if (m_cRef == 1)
		{
			delete this;
			return 0;
		}
		return --m_cRef;
	}

	HRESULT STDMETHODCALLTYPE Read(void* pv, ULONG cb, ULONG* pcbRead)
	{
		if (!pv)
			return STG_E_INVALIDPOINTER;
		HRESULT hr = S_OK;
		if (m_pSeek + cb > m_pMem + m_cbSize)
		{
			cb = (ULONG)(m_pMem + m_cbSize - m_pSeek);
			hr = S_FALSE;
		}
		memcpy(pv, m_pSeek, cb);
		m_pSeek += cb;
		if (pcbRead)
			*pcbRead = cb;
		return hr;
	}

	HRESULT STDMETHODCALLTYPE Write(const void* pv, ULONG cb, ULONG* pcbWritten)
	{
		EckDbgBreak();
		return STG_E_CANTSAVE;
	}

	HRESULT STDMETHODCALLTYPE Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER* plibNewPosition)
	{
		switch (dwOrigin)
		{
		case SEEK_SET:// 这种情况dlibMove应视为无符号
			if ((SIZE_T)dlibMove.QuadPart > m_cbSize)
				return STG_E_INVALIDFUNCTION;
			m_pSeek = m_pMem + (SIZE_T)dlibMove.QuadPart;
			if (plibNewPosition)
				plibNewPosition->QuadPart = dlibMove.QuadPart;
			return S_OK;
		case SEEK_CUR:
		{
			const auto ocbNew = (SIZE_T)((SSIZE_T)dlibMove.QuadPart + (m_pSeek - m_pMem));
			if (ocbNew > m_cbSize || ocbNew < 0)
				return STG_E_INVALIDFUNCTION;
			m_pSeek = m_pMem + ocbNew;
			if (plibNewPosition)
				plibNewPosition->QuadPart = ocbNew;
		}
		return S_OK;
		case SEEK_END:
			if (dlibMove.QuadPart < -(SSIZE_T)m_cbSize || dlibMove.QuadPart>0)
				return STG_E_INVALIDFUNCTION;
			m_pSeek = m_pMem + m_cbSize + (SIZE_T)dlibMove.QuadPart;
			if (plibNewPosition)
				plibNewPosition->QuadPart = (LONGLONG)((SSIZE_T)m_cbSize + dlibMove.QuadPart);
			return S_OK;
		}
		return STG_E_INVALIDFUNCTION;
	}

	HRESULT STDMETHODCALLTYPE SetSize(ULARGE_INTEGER libNewSize)
	{
		EckDbgBreak();
		return STG_E_MEDIUMFULL;
	}

	HRESULT STDMETHODCALLTYPE CopyTo(IStream* pstm, ULARGE_INTEGER cb, ULARGE_INTEGER* pcbRead, ULARGE_INTEGER* pcbWritten)
	{
		if (!pstm)
			return STG_E_INVALIDPOINTER;
		if (m_pSeek + cb.QuadPart > m_pMem + m_cbSize)
			cb.QuadPart = m_cbSize;

		ULONG cbWritten;
		pstm->Write(m_pSeek, cb.LowPart, &cbWritten);
		if (pcbRead)
			pcbRead->QuadPart = cbWritten;
		if (pcbWritten)
			pcbWritten->QuadPart = cbWritten;
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE Commit(DWORD grfCommitFlags)
	{
		return E_NOTIMPL;
	}

	HRESULT STDMETHODCALLTYPE Revert(void)
	{
		return E_NOTIMPL;
	}

	HRESULT STDMETHODCALLTYPE LockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
	{
		return E_NOTIMPL;
	}

	HRESULT STDMETHODCALLTYPE UnlockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
	{
		return E_NOTIMPL;
	}

	HRESULT STDMETHODCALLTYPE Stat(STATSTG* pstatstg, DWORD grfStatFlag)
	{
		ZeroMemory(pstatstg, sizeof(STATSTG));
		pstatstg->type = STGTY_STREAM;
		pstatstg->cbSize.QuadPart = m_cbSize;
		pstatstg->grfMode = STGM_READ;
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE Clone(IStream** ppstm)
	{
		*ppstm = NULL;
		return E_NOTIMPL;
	}
};
ECK_NAMESPACE_END