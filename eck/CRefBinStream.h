/*
* WinEzCtrlKit Library
*
* CRefBinStream.h ： 字节集流
*
* Copyright(C) 2023 QingKong
*/
#pragma once
#include "CRefBin.h"
#include "IMem.h"

#include <Shlwapi.h>

ECK_NAMESPACE_BEGIN
/// <summary>
/// 字节集流。
/// CRefBinT的IStream实现。
/// 非线程安全
/// </summary>
template<class TAlloc>
class CRefBinStreamT :public IStream, public IMem
{
private:
	ULONG m_cRef{ 1ul };

	CRefBinT<TAlloc>& m_rb;
	size_t m_posSeek{};
public:
	CRefBinStreamT(CRefBinT<TAlloc>& rb) :m_rb{ rb } {}

	EckInline void LeaveRelease()
	{
#ifdef _DEBUG
		EckAssert(Release() == 0);
#else
		Release();
#endif
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

	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject)
	{
		const QITAB qit[]
		{
			QITABENT(CRefBinStreamT, IStream),
			QITABENT(CRefBinStreamT, ISequentialStream),
			QITABENT(CRefBinStreamT, IMem),
			{}
		};

		return QISearch(this, qit, riid, ppvObject);
	}

	HRESULT STDMETHODCALLTYPE Read(void* pv, ULONG cb, ULONG* pcbRead)
	{
		ULONG Dummy;
		if (!pcbRead) pcbRead = &Dummy;
		if (!pv)
		{
			*pcbRead = 0;
			return STG_E_INVALIDPOINTER;
		}
		if (m_posSeek > m_rb.Size())
		{
			*pcbRead = 0;
			return S_FALSE;
		}
		HRESULT hr;
		if (m_posSeek + cb > m_rb.Size())
		{
			cb = (ULONG)(m_rb.Size() - m_posSeek);
			hr = S_FALSE;
		}
		else
			hr = S_OK;
		memmove(pv, m_rb.Data() + m_posSeek, cb);
		m_posSeek += cb;
		*pcbRead = cb;
		return hr;
	}

	HRESULT STDMETHODCALLTYPE Write(const void* pv, ULONG cb, ULONG* pcbWritten)
	{
		ULONG Dummy;
		if (!pcbWritten) pcbWritten = &Dummy;
		if (!pv)
		{
			*pcbWritten = 0;
			return STG_E_INVALIDPOINTER;
		}
		if (m_posSeek + cb > m_rb.Size())
			m_rb.ReSizeExtra(m_posSeek + cb);
		memmove(m_rb.Data() + m_posSeek, pv, cb);
		m_posSeek += cb;
		*pcbWritten = cb;
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER* plibNewPosition)
	{
		ULARGE_INTEGER Dummy;
		if (!plibNewPosition) plibNewPosition = &Dummy;
		switch (dwOrigin)
		{
		case SEEK_SET:// 这种情况dlibMove应视为无符号
			m_posSeek = (size_t)dlibMove.QuadPart;
			*plibNewPosition = ToUli(dlibMove);
			return S_OK;

		case SEEK_CUR:
		{
			const ptrdiff_t ocbNew = (ptrdiff_t)dlibMove.QuadPart + m_posSeek;
			if (ocbNew < 0)// 落在流开始之前
				return STG_E_INVALIDFUNCTION;
			m_posSeek = (size_t)ocbNew;
			plibNewPosition->QuadPart = m_posSeek;
		}
		return S_OK;

		case SEEK_END:
			if (dlibMove.QuadPart < -(ptrdiff_t)m_rb.Size())// 落在流开始之前
				return STG_E_INVALIDFUNCTION;
			m_posSeek = (size_t)m_rb.Size() + dlibMove.QuadPart;
			plibNewPosition->QuadPart = m_posSeek;
			return S_OK;
		}
		return STG_E_INVALIDFUNCTION;
	}

	HRESULT STDMETHODCALLTYPE SetSize(ULARGE_INTEGER libNewSize)
	{
		m_rb.ReSizeExtra((size_t)libNewSize.QuadPart);
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE CopyTo(IStream* pstm, ULARGE_INTEGER cb,
		ULARGE_INTEGER* pcbRead, ULARGE_INTEGER* pcbWritten)
	{
		if (!pstm)
			return STG_E_INVALIDPOINTER;
		if (m_posSeek >= m_rb.Size())
		{
			if (pcbRead)
				pcbRead->QuadPart = 0u;
			if (pcbWritten)
				pcbWritten->QuadPart = 0u;
			return S_FALSE;
		}

		ULONG cbRead;
		if (m_posSeek + cb.LowPart > m_rb.Size())
			cbRead = (ULONG)(m_rb.Size() - m_posSeek);
		else
			cbRead = cb.LowPart;
		ULONG cbWritten;
		pstm->Write(m_rb.Data() + m_posSeek, cbRead, &cbWritten);
		if (pcbRead)
			pcbRead->QuadPart = cbRead;
		if (pcbWritten)
			pcbWritten->QuadPart = cbWritten;
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE Commit(DWORD grfCommitFlags)
	{
		return S_OK;
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
		const auto p = new CRefBinStreamT(m_rb);
		p->m_posSeek = m_posSeek;
		*ppstm = p;
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE MemGetPtr(void** ppvData, SIZE_T* pcbData)
	{
		*ppvData = m_rb.Data();
		*pcbData = m_rb.Size();
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE MemLock(void** ppvData, SIZE_T* pcbData)
	{
		*ppvData = NULL;
		*pcbData = 0u;
		return E_NOTIMPL;
	}

	HRESULT STDMETHODCALLTYPE MemUnlock()
	{
		return E_NOTIMPL;
	}
};

using CRefBinStream = CRefBinStreamT<TRefBinDefAllocator>;
ECK_NAMESPACE_END