/*
* WinEzCtrlKit Library
*
* CRefBinStream.h ： 字节集流
* 
* Copyright(C) 2023 QingKong
*/
#pragma once
#include "CRefBin.h"

#include <Shlwapi.h>

ECK_NAMESPACE_BEGIN
/// <summary>
/// 字节集流类
/// 底层为CRefBinT的IStream实现
/// 本类非线程安全
/// </summary>
/// <typeparam name="TAlloc"></typeparam>
template<class TAlloc>
class CRefBinStreamT :public IStream
{
private:
	ULONG m_cRef = 1;

	CRefBinT<TAlloc>& m_rb;
	BYTE* m_pSeek = NULL;
public:
	CRefBinStreamT(CRefBinT<TAlloc>& rb) :m_rb{ rb }, m_pSeek{ rb.Data() } {}

	EckInline void SetRefBin(CRefBinT<TAlloc>& rb)
	{
		m_rb = rb;
		m_pSeek = rb.Data();
	}

	EckInline void LeaveRelease()
	{
#ifdef _DEBUG
		EckAssert(Release() == 0);
#else
		Release();
#endif
	}

	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject)
	{
		const QITAB qit[]
		{
			QITABENT(CRefBinStreamT, IStream),
			QITABENT(CRefBinStreamT, ISequentialStream),
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
		if (m_pSeek + cb > m_rb.Data() + m_rb.Size())
		{
			cb = (ULONG)(m_rb.Data() + m_rb.Size() - m_pSeek);
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
		if (!pv)
			return STG_E_INVALIDPOINTER;
		const size_t ocbOld = m_pSeek - m_rb.Data();
		if (ocbOld + cb > m_rb.Size())
		{
			m_rb.ReSizeExtra(ocbOld + cb);
			m_pSeek = m_rb.Data() + ocbOld;
		}
		memcpy(m_pSeek, pv, cb);
		m_pSeek += cb;
		if (pcbWritten)
			*pcbWritten = cb;
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER* plibNewPosition)
	{
		switch (dwOrigin)
		{
		case SEEK_SET:// 这种情况dlibMove应视为无符号
			if ((SIZE_T)dlibMove.QuadPart > m_rb.Size())
				return STG_E_INVALIDFUNCTION;
			m_pSeek = m_rb.Data() + (SIZE_T)dlibMove.QuadPart;
			if (plibNewPosition)
				plibNewPosition->QuadPart = dlibMove.QuadPart;
			return S_OK;
		case SEEK_CUR:
		{
			const auto ocbNew = (SIZE_T)((SSIZE_T)dlibMove.QuadPart + (m_pSeek - m_rb.Data()));
			if (ocbNew > m_rb.Size() || ocbNew < 0)
				return STG_E_INVALIDFUNCTION;
			m_pSeek = m_rb.Data() + ocbNew;
			if (plibNewPosition)
				plibNewPosition->QuadPart = ocbNew;
		}
		return S_OK;
		case SEEK_END:
			if (dlibMove.QuadPart < -(SSIZE_T)m_rb.Size() || dlibMove.QuadPart>0)
				return STG_E_INVALIDFUNCTION;
			m_pSeek = m_rb.Data() + m_rb.Size() + (SIZE_T)dlibMove.QuadPart;
			if (plibNewPosition)
				plibNewPosition->QuadPart = (LONGLONG)((SSIZE_T)m_rb.Size() + dlibMove.QuadPart);
			return S_OK;
		}
		return STG_E_INVALIDFUNCTION;
	}

	HRESULT STDMETHODCALLTYPE SetSize(ULARGE_INTEGER libNewSize)
	{
		const size_t ocbOld = m_pSeek - m_rb.Data();
		m_rb.ReSizeExtra((size_t)libNewSize.QuadPart);
		m_pSeek = m_rb.Data() + ocbOld;
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE CopyTo(IStream* pstm, ULARGE_INTEGER cb, ULARGE_INTEGER* pcbRead, ULARGE_INTEGER* pcbWritten)
	{
		if (!pstm)
			return STG_E_INVALIDPOINTER;
		if (m_pSeek + cb.QuadPart > m_rb.Data() + m_rb.Size())
			cb.QuadPart = m_rb.Size();

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
		pstatstg->cbSize.QuadPart = m_rb.Size();
		pstatstg->grfMode = STGM_READWRITE;
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE Clone(IStream** ppstm)
	{
		*ppstm = NULL;
		return E_NOTIMPL;
	}
};

using CRefBinStream = CRefBinStreamT<TRefBinDefAllocator>;
ECK_NAMESPACE_END